// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "interface/ui/payablelist/PayableList.hpp"  // IWYU pragma: associated

#include <atomic>
#include <future>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto PayableListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const UnitType& currency,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::PayableList>
{
    using ReturnType = ui::implementation::PayableList;

    return std::make_unique<ReturnType>(api, nymID, currency, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
PayableList::PayableList(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const UnitType& currency,
    const SimpleCallback& cb) noexcept
    : PayableListList(api, nymID, cb, false)
    , Worker(api, {}, "ui::PayableList")
    , owner_contact_id_()  // FIXME wtf
    , currency_(currency)
{
    init_executor(
        {UnallocatedCString{api.Endpoints().ContactUpdate()},
         UnallocatedCString{api.Endpoints().NymDownload()}});
    pipeline_.Push(MakeWork(Work::init));
}

auto PayableList::construct_row(
    const PayableListRowID& id,
    const PayableListSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::PayableListItem(
        *this,
        api_,
        id,
        index,
        extract_custom<const UnallocatedCString>(custom, 0),
        currency_);
}

auto PayableList::pipeline(const Message& in) noexcept -> void
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
        case Work::nym: {
            process_nym(in);
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

auto PayableList::process_contact(
    const PayableListRowID& id,
    const PayableListSortKey& key) noexcept -> void
{
    if (owner_contact_id_ == id) { return; }

    const auto contact = api_.Contacts().Contact(id);

    if (false == bool(contact)) {
        LogError()(OT_PRETTY_CLASS())("Error: Contact ")(
            id)(" can not be loaded.")
            .Flush();

        return;
    }

    OT_ASSERT(contact);

    auto paymentCode =
        std::make_unique<UnallocatedCString>(contact->PaymentCode(currency_));

    OT_ASSERT(paymentCode);

    if (false == paymentCode->empty()) {
        auto custom = CustomData{paymentCode.release()};
        add_item(id, key, custom);
    } else {
        LogDetail()(OT_PRETTY_CLASS())("Skipping unpayable contact ")(id)
            .Flush();
    }
}

auto PayableList::process_contact(const Message& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto& id = body.at(1);
    const auto contactID = api_.Factory().IdentifierFromHash(id.Bytes());

    OT_ASSERT(false == contactID.empty());

    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, {false, name});
}

auto PayableList::process_nym(const Message& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto& id = body.at(1);
    const auto nymID = api_.Factory().NymIDFromHash(id.Bytes());

    OT_ASSERT(false == nymID.empty());

    const auto contactID = api_.Contacts().ContactID(nymID);
    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, {false, name});
}

auto PayableList::startup() noexcept -> void
{
    const auto contacts = api_.Contacts().ContactList();
    LogDetail()(OT_PRETTY_CLASS())("Loading ")(contacts.size())(" contacts.")
        .Flush();

    for (const auto& [id, alias] : contacts) {
        process_contact(
            api_.Factory().IdentifierFromBase58(id), {false, alias});
    }

    finish_startup();
}

PayableList::~PayableList()
{
    wait_for_startup();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
