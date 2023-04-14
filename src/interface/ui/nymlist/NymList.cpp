// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/nymlist/NymList.hpp"  // IWYU pragma: associated

#include <atomic>
#include <future>
#include <memory>
#include <span>
#include <utility>

#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto NymListModel(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept -> std::unique_ptr<ui::internal::NymList>
{
    using ReturnType = ui::implementation::NymList;

    return std::make_unique<ReturnType>(api, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
NymList::NymList(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept
    : NymListList(api, identifier::Generic{}, cb, false)
    , Worker(api, {}, "ui::NymList")
{
    init_executor({
        UnallocatedCString{api.Endpoints().NymCreated()},
        UnallocatedCString{api.Endpoints().NymDownload()},
    });
    pipeline_.Push(MakeWork(Work::init));
}

auto NymList::construct_row(
    const NymListRowID& id,
    const NymListSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::NymListItem(*this, api_, id, index, custom);
}

auto NymList::load() noexcept -> void
{
    for (const auto& nym : api_.Wallet().LocalNyms()) {
        load(std::move(const_cast<identifier::Nym&&>(nym)));
    }
}

auto NymList::load(identifier::Nym&& id) noexcept -> void
{
    auto nym = api_.Wallet().Nym(id);
    auto custom = CustomData{};
    add_item(id, nym->Name(), custom);
}

auto NymList::pipeline(Message&& in) noexcept -> void
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

    if ((false == startup_complete()) && (Work::init != work)) {
        pipeline_.Push(std::move(in));

        return;
    }

    switch (work) {
        case Work::shutdown: {
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        case Work::newnym: {
            process_new_nym(std::move(in));
        } break;
        case Work::nymchanged: {
            process_nym_changed(std::move(in));
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto NymList::process_new_nym(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(1 < body.size());

    auto nymID = api_.Factory().NymIDFromHash(body[1].Bytes());

    OT_ASSERT(false == nymID.empty());

    load(std::move(nymID));
}

auto NymList::process_nym_changed(Message&& in) noexcept -> void
{
    const auto& api = api_;
    const auto body = in.Payload();

    OT_ASSERT(1 < body.size());

    auto nymID = api.Factory().NymIDFromHash(body[1].Bytes());

    OT_ASSERT(false == nymID.empty());

    if (false == api.Wallet().IsLocalNym(nymID)) { return; }

    load(std::move(nymID));
}

auto NymList::startup() noexcept -> void
{
    load();
    finish_startup();
    trigger();
}

NymList::~NymList()
{
    wait_for_startup();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
