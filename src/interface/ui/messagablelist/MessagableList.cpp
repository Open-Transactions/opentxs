// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/messagablelist/MessagableList.hpp"  // IWYU pragma: associated

#include <atomic>
#include <future>
#include <memory>
#include <span>

#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto MessagableListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::MessagableList>
{
    using ReturnType = ui::implementation::MessagableList;

    return std::make_unique<ReturnType>(api, nymID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
MessagableList::MessagableList(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : MessagableListList(api, nymID, cb, false)
    , Worker(api, {}, "ui::MessagableList")
    , owner_contact_id_(api_.Contacts().ContactID(nymID))
{
    init_executor(
        {UnallocatedCString{api.Endpoints().ContactUpdate()},
         UnallocatedCString{api.Endpoints().NymDownload()}});
    pipeline_.Push(MakeWork(Work::init));
}

auto MessagableList::construct_row(
    const MessagableListRowID& id,
    const MessagableListSortKey& index,
    CustomData&) const noexcept -> RowPointer
{
    return factory::MessagableListItem(*this, api_, id, index);
}

auto MessagableList::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Payload();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body[0].as<Work>();
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

auto MessagableList::process_contact(
    const MessagableListRowID& id,
    const MessagableListSortKey& key) noexcept -> void
{
    if (owner_contact_id_ == id) {
        LogDetail()(OT_PRETTY_CLASS())("Skipping owner contact ")(id)(" (")(
            key.second)(")")
            .Flush();

        return;
    } else {
        LogDetail()(OT_PRETTY_CLASS())("Incoming contact ")(id)(" (")(
            key.second)(") is not owner contact: (")(owner_contact_id_)(")")
            .Flush();
    }

    switch (api_.OTX().CanMessage(primary_id_, id, false)) {
        case otx::client::Messagability::READY:
        case otx::client::Messagability::MISSING_RECIPIENT:
        case otx::client::Messagability::UNREGISTERED: {
            LogDetail()(OT_PRETTY_CLASS())("Messagable contact ")(id)(" (")(
                key.second)(")")
                .Flush();
            auto custom = CustomData{};
            add_item(id, key, custom);
        } break;
        case otx::client::Messagability::MISSING_SENDER:
        case otx::client::Messagability::INVALID_SENDER:
        case otx::client::Messagability::NO_SERVER_CLAIM:
        case otx::client::Messagability::CONTACT_LACKS_NYM:
        case otx::client::Messagability::MISSING_CONTACT:
        default: {
            LogDetail()(OT_PRETTY_CLASS())("Skipping non-messagable contact ")(
                id)(" (")(key.second)(")")
                .Flush();
            delete_item(id);
        }
    }
}

auto MessagableList::process_contact(const Message& message) noexcept -> void
{
    const auto body = message.Payload();

    OT_ASSERT(1 < body.size());

    const auto& id = body[1];
    const auto contactID = api_.Factory().IdentifierFromHash(id.Bytes());

    OT_ASSERT(false == contactID.empty());

    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, {false, name});
}

auto MessagableList::process_nym(const Message& message) noexcept -> void
{
    const auto body = message.Payload();

    OT_ASSERT(1 < body.size());

    const auto& id = body[1];
    const auto nymID = api_.Factory().NymIDFromHash(id.Bytes());

    OT_ASSERT(false == nymID.empty());

    const auto contactID = api_.Contacts().ContactID(nymID);
    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, {false, name});
}

auto MessagableList::startup() noexcept -> void
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

MessagableList::~MessagableList()
{
    wait_for_startup();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
