// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/ui/ProfileSection.hpp"

#include "InternalUI.hpp"
#include "List.hpp"
#include "ProfileSectionBlank.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "Profile.hpp"

template struct std::pair<int, std::string>;

#define OT_METHOD "opentxs::ui::implementation::Profile::"

namespace opentxs
{
ui::Profile* Factory::ProfileWidget(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const Identifier& nymID)
{
    return new ui::implementation::Profile(api, publisher, nymID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const std::set<proto::ContactSectionName> Profile::allowed_types_{
    proto::CONTACTSECTION_COMMUNICATION,
    proto::CONTACTSECTION_PROFILE};

const std::map<proto::ContactSectionName, int> Profile::sort_keys_{
    {proto::CONTACTSECTION_COMMUNICATION, 0},
    {proto::CONTACTSECTION_PROFILE, 1}};

Profile::Profile(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const Identifier& nymID)
    : ProfileList(api, publisher, nymID)
    , listeners_({
          {api_.Endpoints().NymDownload(),
           new MessageProcessor<Profile>(&Profile::process_nym)},
      })
    , name_(nym_name(api_.Wallet(), nymID))
    , payment_code_()
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&Profile::startup, this));

    OT_ASSERT(startup_)
}

bool Profile::AddClaim(
    const proto::ContactSectionName section,
    const proto::ContactItemType type,
    const std::string& value,
    const bool primary,
    const bool active) const
{
    auto nym = api_.Wallet().mutable_Nym(nym_id_);

    switch (section) {
        case proto::CONTACTSECTION_SCOPE: {

            return nym.SetScope(type, value, primary);
        } break;
        case proto::CONTACTSECTION_COMMUNICATION: {
            switch (type) {
                case proto::CITEMTYPE_EMAIL: {

                    return nym.AddEmail(value, primary, active);
                } break;
                case proto::CITEMTYPE_PHONE: {

                    return nym.AddPhoneNumber(value, primary, active);
                } break;
                case proto::CITEMTYPE_OPENTXS: {

                    return nym.AddPreferredOTServer(value, primary);
                } break;
                default: {
                }
            }
        } break;
        case proto::CONTACTSECTION_PROFILE: {

            return nym.AddSocialMediaProfile(value, type, primary, active);
        } break;
        case proto::CONTACTSECTION_CONTRACT: {

            return nym.AddContract(value, type, primary, active);
        } break;
        case proto::CONTACTSECTION_PROCEDURE: {
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            return nym.AddPaymentCode(value, type, primary, active);
#endif
        } break;
        default: {
        }
    }

    Claim claim{};
    auto& [id, claimSection, claimType, claimValue, start, end, attributes] =
        claim;
    id = "";
    claimSection = section;
    claimType = type;
    claimValue = value;
    start = 0;
    end = 0;

    if (primary) { attributes.emplace(proto::CITEMATTR_PRIMARY); }

    if (primary || active) { attributes.emplace(proto::CITEMATTR_ACTIVE); }

    return nym.AddClaim(claim);
}

Profile::ItemTypeList Profile::AllowedItems(
    const proto::ContactSectionName section,
    const std::string& lang) const
{
    return ui::ProfileSection::AllowedItems(section, lang);
}

Profile::SectionTypeList Profile::AllowedSections(const std::string& lang) const
{
    SectionTypeList output{};

    for (const auto& type : allowed_types_) {
        output.emplace_back(type, proto::TranslateSectionName(type, lang));
    }

    return output;
}

bool Profile::check_type(const proto::ContactSectionName type)
{
    return 1 == allowed_types_.count(type);
}

void Profile::construct_row(
    const ProfileRowID& id,
    const ContactSortKey& index,
    const CustomData& custom) const
{
    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ProfileSectionWidget(
            *this, api_, publisher_, id, index, custom));
}

bool Profile::Delete(
    const int sectionType,
    const int type,
    const std::string& claimID) const
{
    Lock lock(lock_);
    auto& section = find_by_id(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.Delete(type, claimID);
}

const Identifier& Profile::NymID() const { return nym_id_; }

std::string Profile::DisplayName() const
{
    Lock lock(lock_);

    return name_;
}

std::string Profile::nym_name(
    const api::Wallet& wallet,
    const Identifier& nymID)
{
    for (const auto& [id, name] : wallet.NymList()) {
        if (nymID.str() == id) { return name; }
    }

    return {};
}

std::string Profile::PaymentCode() const
{
    Lock lock(lock_);

    return payment_code_;
}

void Profile::process_nym(const Nym& nym)
{
    Lock lock(lock_);
    name_ = nym.Alias();
    payment_code_ = nym.PaymentCode();
    lock.unlock();
    UpdateNotify();
    std::set<ProfileRowID> active{};

    for (const auto& section : nym.Claims()) {
        auto& type = section.first;

        if (check_type(type)) {
            CustomData custom{new opentxs::ContactSection(*section.second)};
            add_item(type, sort_key(type), custom);
            active.emplace(type);
        }
    }

    delete_inactive(active);
}

void Profile::process_nym(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto nymID = Identifier::Factory(id);

    OT_ASSERT(false == nymID->empty())

    if (nymID != nym_id_) { return; }

    const auto nym = api_.Wallet().Nym(nymID);

    OT_ASSERT(nym)

    process_nym(*nym);
}

bool Profile::SetActive(
    const int sectionType,
    const int type,
    const std::string& claimID,
    const bool active) const
{
    Lock lock(lock_);
    auto& section = find_by_id(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetActive(type, claimID, active);
}

bool Profile::SetPrimary(
    const int sectionType,
    const int type,
    const std::string& claimID,
    const bool primary) const
{
    Lock lock(lock_);
    auto& section = find_by_id(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetPrimary(type, claimID, primary);
}

bool Profile::SetValue(
    const int sectionType,
    const int type,
    const std::string& claimID,
    const std::string& value) const
{
    Lock lock(lock_);
    auto& section = find_by_id(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetValue(type, claimID, value);
}

int Profile::sort_key(const proto::ContactSectionName type)
{
    return sort_keys_.at(type);
}

void Profile::startup()
{
    LogOutput(OT_METHOD)(__FUNCTION__)(": Loading nym ")(nym_id_)(".").Flush();
    const auto nym = api_.Wallet().Nym(nym_id_);

    OT_ASSERT(nym)

    process_nym(*nym);
    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
