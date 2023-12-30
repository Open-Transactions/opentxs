// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/messagablelist/MessagableList.hpp"  // IWYU pragma: associated

#include <atomic>
#include <future>
#include <memory>
#include <span>
#include <utility>

#include "internal/network/zeromq/Pipeline.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/otx/client/Messagability.hpp"  // IWYU pragma: keep
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
        LogError()()("Invalid message").Flush();

        LogAbort()().Abort();
    }

    const auto work = [&] {
        try {

            return body[0].as<Work>();
        } catch (...) {

            LogAbort()().Abort();
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
            LogError()()("Unhandled type").Flush();

            LogAbort()().Abort();
        }
    }
}

auto MessagableList::process_contact(
    const MessagableListRowID& id,
    const MessagableListSortKey& key) noexcept -> void
{
    if (owner_contact_id_ == id) {
        LogDetail()()("Skipping owner contact ")(id, api_.Crypto())(" (")(
            key.second)(")")
            .Flush();

        return;
    } else {
        LogDetail()()("Incoming contact ")(id, api_.Crypto())(" (")(key.second)(
            ") is not owner contact: (")(owner_contact_id_, api_.Crypto())(")")
            .Flush();
    }

    switch (api_.OTX().CanMessage(primary_id_, id, false)) {
        case otx::client::Messagability::READY:
        case otx::client::Messagability::MISSING_RECIPIENT:
        case otx::client::Messagability::UNREGISTERED: {
            LogDetail()()("Messagable contact ")(id, api_.Crypto())(" (")(
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
            LogDetail()()("Skipping non-messagable contact ")(
                id, api_.Crypto())(" (")(key.second)(")")
                .Flush();
            delete_item(id);
        }
    }
}

auto MessagableList::process_contact(const Message& message) noexcept -> void
{
    const auto body = message.Payload();

    assert_true(1 < body.size());

    const auto& id = body[1];
    const auto contactID = api_.Factory().IdentifierFromProtobuf(id.Bytes());

    assert_false(contactID.empty());

    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, {false, name});
}

auto MessagableList::process_nym(const Message& message) noexcept -> void
{
    const auto body = message.Payload();

    assert_true(1 < body.size());

    const auto& id = body[1];
    const auto nymID = api_.Factory().NymIDFromHash(id.Bytes());

    assert_false(nymID.empty());

    const auto contactID = api_.Contacts().ContactID(nymID);
    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, {false, name});
}

auto MessagableList::startup() noexcept -> void
{
    const auto contacts = api_.Contacts().ContactList();
    LogDetail()()("Loading ")(contacts.size())(" contacts.").Flush();

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
