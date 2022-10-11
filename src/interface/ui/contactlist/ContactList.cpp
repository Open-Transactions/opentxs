// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "interface/ui/contactlist/ContactList.hpp"  // IWYU pragma: associated

#include <atomic>
#include <future>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto ContactListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::ContactList>
{
    using ReturnType = ui::implementation::ContactList;

    return std::make_unique<ReturnType>(api, nymID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ContactList::ContactList(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : ContactListList(api, nymID, cb, false)
    , Worker(api, {}, "ui::ContactList")
    , owner_contact_id_(api_.Contacts().ContactID(nymID))
{
    OT_ASSERT(false == owner_contact_id_.empty());

    process_contact(owner_contact_id_);
    init_executor({UnallocatedCString{api.Endpoints().ContactUpdate()}});
    pipeline_.Push(MakeWork(Work::init));
}

ContactList::ParsedArgs::ParsedArgs(
    const api::Session& api,
    const UnallocatedCString& purportedID,
    const UnallocatedCString& purportedPaymentCode) noexcept
    : nym_id_(extract_nymid(api, purportedID, purportedPaymentCode))
    , payment_code_(extract_paymentcode(api, purportedID, purportedPaymentCode))
{
}

auto ContactList::ParsedArgs::extract_nymid(
    const api::Session& api,
    const UnallocatedCString& purportedID,
    const UnallocatedCString& purportedPaymentCode) noexcept -> identifier::Nym
{
    auto output = identifier::Nym{};

    if (false == purportedID.empty()) {
        // Case 1: purportedID is a nym id
        output = api.Factory().NymIDFromBase58(purportedID);

        if (false == output.empty()) { return output; }

        // Case 2: purportedID is a payment code
        output = api.Factory().NymIDFromPaymentCode(purportedID);

        if (false == output.empty()) { return output; }
    }

    if (false == purportedPaymentCode.empty()) {
        // Case 3: purportedPaymentCode is a payment code
        output = api.Factory().NymIDFromPaymentCode(purportedPaymentCode);

        if (false == output.empty()) { return output; }

        // Case 4: purportedPaymentCode is a nym id
        output = api.Factory().NymIDFromBase58(purportedPaymentCode);

        if (false == output.empty()) { return output; }
    }

    // Case 5: not possible to extract a nym id

    return output;
}

auto ContactList::ParsedArgs::extract_paymentcode(
    const api::Session& api,
    const UnallocatedCString& purportedID,
    const UnallocatedCString& purportedPaymentCode) noexcept -> PaymentCode
{
    if (false == purportedPaymentCode.empty()) {
        // Case 1: purportedPaymentCode is a payment code
        auto output = api.Factory().PaymentCode(purportedPaymentCode);

        if (output.Valid()) { return output; }
    }

    if (false == purportedID.empty()) {
        // Case 2: purportedID is a payment code
        auto output = api.Factory().PaymentCode(purportedID);

        if (output.Valid()) { return output; }
    }

    // Case 3: not possible to extract a payment code

    return api.Factory().PaymentCode(UnallocatedCString{});
}

auto ContactList::AddContact(
    const UnallocatedCString& label,
    const UnallocatedCString& paymentCode,
    const UnallocatedCString& nymID) const noexcept -> UnallocatedCString
{
    auto args = ParsedArgs{api_, nymID, paymentCode};
    const auto contact =
        api_.Contacts().NewContact(label, args.nym_id_, args.payment_code_);
    const auto& id = contact->ID();
    api_.OTX().CanMessage(primary_id_, id, true);

    return id.asBase58(api_.Crypto());
}

auto ContactList::construct_row(
    const ContactListRowID& id,
    const ContactListSortKey& index,
    CustomData&) const noexcept -> RowPointer
{
    return factory::ContactListItem(*this, api_, id, index);
}

auto ContactList::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::contact: {
            process_contact(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        case Work::shutdown: {
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto ContactList::process_contact(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto& id = body.at(1);
    const auto contactID = api_.Factory().IdentifierFromHash(id.Bytes());

    OT_ASSERT(false == contactID.empty());

    process_contact(contactID);
}

auto ContactList::process_contact(const identifier::Generic& contactID) noexcept
    -> void
{
    auto name = api_.Contacts().ContactName(contactID);

    OT_ASSERT(false == name.empty());

    auto custom = CustomData{};
    add_item(
        contactID, {(contactID == owner_contact_id_), std::move(name)}, custom);
}

auto ContactList::startup() noexcept -> void
{
    const auto contacts = api_.Contacts().ContactList();
    LogVerbose()(OT_PRETTY_CLASS())("Loading ")(contacts.size())(" contacts.")
        .Flush();

    for (const auto& [id, alias] : contacts) {
        auto custom = CustomData{};
        const auto contactID = api_.Factory().IdentifierFromBase58(id);
        process_contact(contactID);
    }

    finish_startup();
}

ContactList::~ContactList()
{
    wait_for_startup();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
