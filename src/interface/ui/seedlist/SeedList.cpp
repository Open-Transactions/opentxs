// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/seedlist/SeedList.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/crypto/Bip32Child.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Seed.hpp"
#include "opentxs/crypto/SeedStyle.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto SeedListModel(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::SeedList>
{
    using ReturnType = ui::implementation::SeedList;

    return std::make_unique<ReturnType>(api, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
SeedList::SeedList(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept
    : SeedListList(api, identifier::Generic{}, cb, false)
    , Worker(api, 100ms, "ui::SeedList")
{
    init_executor({
        UnallocatedCString{api.Endpoints().SeedUpdated()},
    });
    pipeline_.Push(MakeWork(Work::init));
}

auto SeedList::construct_row(
    const SeedListRowID& id,
    const SeedListSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::SeedListItem(*this, api_, id, index, custom);
}

auto SeedList::load() noexcept -> void
{
    const auto& api = api_;

    for (auto& [id, alias] : api.Storage().SeedList()) {
        const auto seedID = api.Factory().IdentifierFromBase58(id);
        process_seed(seedID);
    }
}

auto SeedList::load_seed(
    const SeedListRowID& id,
    SeedListSortKey& name,
    crypto::SeedStyle& type) const noexcept(false) -> void
{
    const auto& api = api_;
    const auto& factory = api.Factory();
    const auto& seeds = api.Crypto().Seed();
    const auto reason = factory.PasswordPrompt("Display seed list");
    const auto seed = seeds.GetSeed(id, reason);

    if (crypto::SeedStyle::Error == seed.Type()) {
        throw std::runtime_error{"invalid seed"};
    }

    const auto sId = id.asBase58(api_.Crypto());
    name = seeds.SeedDescription(sId);
    type = seed.Type();
}

auto SeedList::pipeline(Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Payload();

    if (1 > body.size()) {
        LogAbort()(OT_PRETTY_CLASS())("Invalid message").Abort();
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
        case Work::changed_seed: {
            process_seed(std::move(in));
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogAbort()(OT_PRETTY_CLASS())("Unhandled type").Abort();
        }
    }
}

auto SeedList::process_seed(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(1 < body.size());

    const auto id = api_.Factory().IdentifierFromHash(body[1].Bytes());
    process_seed(id);
}

auto SeedList::process_seed(const identifier::Generic& id) noexcept -> void
{
    auto index = SeedListSortKey{};
    auto custom = [&] {
        auto out = CustomData{};
        out.reserve(1);
        out.emplace_back(std::make_unique<crypto::SeedStyle>().release());

        return out;
    }();
    auto& type = *reinterpret_cast<crypto::SeedStyle*>(custom.at(0));

    try {
        load_seed(id, index, type);
        add_item(id, index, custom);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return;
    }
}

auto SeedList::startup() noexcept -> void
{
    load();
    finish_startup();
    trigger();
}

SeedList::~SeedList()
{
    wait_for_startup();
    ClearCallbacks();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
