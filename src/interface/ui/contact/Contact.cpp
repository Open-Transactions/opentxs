// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "interface/ui/contact/Contact.hpp"  // IWYU pragma: associated

#include <functional>
#include <memory>
#include <thread>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::factory
{
auto ContactModel(
    const api::session::Client& api,
    const ui::implementation::ContactPrimaryID& contactID,
    const SimpleCallback& cb) noexcept -> std::unique_ptr<ui::internal::Contact>
{
    using ReturnType = ui::implementation::Contact;

    return std::make_unique<ReturnType>(api, contactID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
const UnallocatedSet<identity::wot::claim::SectionType> Contact::allowed_types_{
    identity::wot::claim::SectionType::Communication,
    identity::wot::claim::SectionType::Profile};

const UnallocatedMap<identity::wot::claim::SectionType, int>
    Contact::sort_keys_{
        {identity::wot::claim::SectionType::Communication, 0},
        {identity::wot::claim::SectionType::Profile, 1}};

Contact::Contact(
    const api::session::Client& api,
    const Identifier& contactID,
    const SimpleCallback& cb) noexcept
    : ContactType(api, contactID, cb, false)
    , listeners_({
          {UnallocatedCString{api_.Endpoints().ContactUpdate()},
           new MessageProcessor<Contact>(&Contact::process_contact)},
      })
    , callbacks_()
    , name_(api_.Contacts().ContactName(contactID))
    , payment_code_()
{
    // NOTE nym_id_ is actually the contact id
    setup_listeners(listeners_);
    startup_ = std::make_unique<std::thread>(&Contact::startup, this);

    OT_ASSERT(startup_)
}

auto Contact::check_type(const identity::wot::claim::SectionType type) noexcept
    -> bool
{
    return 1 == allowed_types_.count(type);
}

auto Contact::ClearCallbacks() const noexcept -> void
{
    Widget::ClearCallbacks();
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = {};
}

auto Contact::construct_row(
    const ContactRowID& id,
    const ContactSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ContactSectionWidget(*this, api_, id, index, custom);
}

auto Contact::ContactID() const noexcept -> UnallocatedCString
{
    return primary_id_->str();
}

auto Contact::DisplayName() const noexcept -> UnallocatedCString
{
    auto lock = rLock{recursive_lock_};

    return name_;
}

auto Contact::PaymentCode() const noexcept -> UnallocatedCString
{
    auto lock = rLock{recursive_lock_};

    return payment_code_;
}

auto Contact::process_contact(const opentxs::Contact& contact) noexcept -> void
{
    {
        const auto name = contact.Label();
        const auto code = contact.PaymentCode();
        auto nameChanged{false};
        auto codeChanged{false};

        {
            auto lock = rLock{recursive_lock_};

            if (name_ != name) {
                name_ = name;
                nameChanged = true;
            }

            if (payment_code_ != code) {
                payment_code_ = code;
                codeChanged = true;
            }
        }

        {
            auto lock = Lock{callbacks_.lock_};
            auto& cb = callbacks_.cb_;

            if (nameChanged && cb.name_) { cb.name_(name.c_str()); }
            if (codeChanged && cb.payment_code_) {
                cb.payment_code_(code.c_str());
            }
        }

        if (nameChanged || codeChanged) { UpdateNotify(); }
    }

    auto active = UnallocatedSet<ContactRowID>{};
    const auto data = contact.Data();

    if (data) {
        for (const auto& section : *data) {
            auto& type = section.first;

            if (check_type(type)) {
                auto custom = CustomData{
                    new identity::wot::claim::Section(*section.second)};
                add_item(type, sort_key(type), custom);
                active.emplace(type);
            }
        }
    }

    delete_inactive(active);
}

auto Contact::process_contact(const Message& message) noexcept -> void
{
    wait_for_startup();

    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto& id = body.at(1);
    const auto contactID = Widget::api_.Factory().Identifier(id);

    OT_ASSERT(false == contactID->empty())

    if (contactID != primary_id_) { return; }

    const auto contact = api_.Contacts().Contact(contactID);

    OT_ASSERT(contact)

    process_contact(*contact);
}

auto Contact::SetCallbacks(Callbacks&& cb) noexcept -> void
{
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = std::move(cb);
}

auto Contact::sort_key(const identity::wot::claim::SectionType type) noexcept
    -> int
{
    return sort_keys_.at(type);
}

auto Contact::startup() noexcept -> void
{
    LogVerbose()(OT_PRETTY_CLASS())("Loading contact ")(primary_id_).Flush();
    const auto contact = api_.Contacts().Contact(primary_id_);

    OT_ASSERT(contact)

    process_contact(*contact);
    finish_startup();
}

Contact::~Contact()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
