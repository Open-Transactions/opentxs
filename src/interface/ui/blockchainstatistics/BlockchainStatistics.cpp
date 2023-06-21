// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/blockchainstatistics/BlockchainStatistics.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>  // IWYU pragma: keep
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <iterator>
#include <memory>
#include <span>
#include <utility>

#include "internal/api/network/Asio.hpp"
#include "internal/core/Factory.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto BlockchainStatisticsModel(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::BlockchainStatistics>
{
    using ReturnType = ui::implementation::BlockchainStatistics;

    return std::make_unique<ReturnType>(api, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainStatistics::BlockchainStatistics(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept
    : BlockchainStatisticsList(api, identifier::Generic{}, cb, false)
    , Worker(api, {}, "ui::BlockchainStatistics")
    , blockchain_(api.Network().Blockchain())
    , cache_()
    , timer_(api.Network().Asio().Internal().GetTimer())
{
    init_executor({
        UnallocatedCString{api.Endpoints().BlockchainBlockDownloadQueue()},
        UnallocatedCString{api.Endpoints().BlockchainNewFilter()},
        UnallocatedCString{api.Endpoints().BlockchainPeer()},
        UnallocatedCString{api.Endpoints().BlockchainPeerConnection()},
        UnallocatedCString{api.Endpoints().BlockchainReorg()},
        UnallocatedCString{api.Endpoints().BlockchainStateChange()},
        UnallocatedCString{api.Endpoints().BlockchainWalletUpdated()},
    });
    pipeline_.Push(MakeWork(Work::init));
}

auto BlockchainStatistics::construct_row(
    const BlockchainStatisticsRowID& id,
    const BlockchainStatisticsSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BlockchainStatisticsItem(*this, api_, id, index, custom);
}

auto BlockchainStatistics::custom(
    const BlockchainStatisticsRowID& chain) noexcept -> CustomData
{
    // NOTE:
    //  0: header oracle height
    //  1: filter oracle height
    //  2: connected peer count
    //  3: active peer count
    //  4: block download queue
    //  5: balance
    auto out = CustomData{};
    out.reserve(6_uz);
    const auto& data = get_cache(chain);
    const auto& [header, filter, connected, active, blocks, balance] = data;
    out.emplace_back(new blockchain::block::Height{header});
    out.emplace_back(new blockchain::block::Height{filter});
    out.emplace_back(new std::size_t{connected});
    out.emplace_back(new std::size_t{active});
    out.emplace_back(new std::size_t{blocks});
    out.emplace_back(new Amount{balance});

    return out;
}

auto BlockchainStatistics::get_cache(
    const BlockchainStatisticsRowID& chain) noexcept -> CachedData&
{
    if (auto i = cache_.find(chain); cache_.end() != i) {

        return i->second;
    } else {
        auto& data = cache_[chain];
        auto& [header, filter, connected, active, blocks, balance] = data;
        header = -1;
        filter = -1;
        connected = 0_uz;
        active = 0_uz;
        blocks = 0_uz;
        balance = 0;

        return data;
    }
}

auto BlockchainStatistics::pipeline(const Message& in) noexcept -> void
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
        case Work::shutdown: {
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        case Work::blockheader: {
            process_block_header(in);
        } break;
        case Work::activepeer: {
            process_activepeer(in);
        } break;
        case Work::reorg: {
            process_reorg(in);
        } break;
        case Work::statechange: {
            process_state(in);
        } break;
        case Work::filter: {
            process_cfilter(in);
        } break;
        case Work::block: {
            process_block(in);
        } break;
        case Work::connectedpeer: {
            process_connectedpeer(in);
        } break;
        case Work::balance: {
            process_balance(in);
        } break;
        case Work::timer: {
            process_timer(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogAbort()(OT_PRETTY_CLASS())("Unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto BlockchainStatistics::process_activepeer(const Message& in) noexcept
    -> void
{
    const auto body = in.Payload();

    OT_ASSERT(3 < body.size());

    const auto chain = body[1].as<blockchain::Type>();

    auto& [header, filter, connected, active, blocks, balance] =
        get_cache(chain);
    active = body[3].as<std::size_t>();
    process_chain(chain);
}

auto BlockchainStatistics::process_balance(const Message& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(3 < body.size());

    const auto chain = body[1].as<blockchain::Type>();
    auto& [header, filter, connected, active, blocks, balance] =
        get_cache(chain);
    balance = factory::Amount(body[3]);
    process_chain(chain);
}

auto BlockchainStatistics::process_block(const Message& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(2 < body.size());

    const auto chain = body[1].as<blockchain::Type>();
    auto& [header, filter, connected, active, blocks, balance] =
        get_cache(chain);
    blocks = body[2].as<std::size_t>();
    process_chain(chain);
}

auto BlockchainStatistics::process_block_header(const Message& in) noexcept
    -> void
{
    const auto body = in.Payload();

    OT_ASSERT(3 < body.size());

    const auto chain = body[1].as<blockchain::Type>();
    auto& [header, filter, connected, active, blocks, balance] =
        get_cache(chain);
    header = body[3].as<blockchain::block::Height>();
    process_chain(chain);
}

auto BlockchainStatistics::process_chain(
    BlockchainStatisticsRowID chain) noexcept -> void
{
    auto data = custom(chain);
    add_item(chain, UnallocatedCString{print(chain)}, data);
    // TODO List::delete_inactive should accept other container types
    delete_inactive([&] {
        auto out = UnallocatedSet<blockchain::Type>{};
        const auto in = blockchain_.EnabledChains();
        std::copy(in.begin(), in.end(), std::inserter(out, out.end()));

        return out;
    }());
}

auto BlockchainStatistics::process_cfilter(const Message& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(3 < body.size());

    const auto chain = body[1].as<blockchain::Type>();
    auto& [header, filter, connected, active, blocks, balance] =
        get_cache(chain);
    filter = body[3].as<blockchain::block::Height>();
    process_chain(chain);
}

auto BlockchainStatistics::process_connectedpeer(const Message& in) noexcept
    -> void
{
    const auto body = in.Payload();

    OT_ASSERT(2 < body.size());

    const auto chain = body[1].as<blockchain::Type>();
    auto& [header, filter, connected, active, blocks, balance] =
        get_cache(chain);
    connected = body[2].as<std::size_t>();
    process_chain(chain);
}

auto BlockchainStatistics::process_reorg(const Message& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(5 < body.size());

    const auto chain = body[1].as<blockchain::Type>();
    auto& [header, filter, connected, active, blocks, balance] =
        get_cache(chain);
    header = body[5].as<blockchain::block::Height>();
    process_chain(chain);
}

auto BlockchainStatistics::process_state(const Message& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(2 < body.size());

    process_chain(body[1].as<blockchain::Type>());
}

auto BlockchainStatistics::process_timer(const Message& in) noexcept -> void
{
    for (auto& [chain, data] : cache_) { process_chain(chain); }

    reset_timer();
}

auto BlockchainStatistics::reset_timer() noexcept -> void
{
    if (api_.GetOptions().TestMode()) { return; }

    using namespace std::literals;
    timer_.SetRelative(60s);
    timer_.Wait([this](const auto& ec) {
        if (!ec) { pipeline_.Push(MakeWork(Work::timer)); }
    });
}

auto BlockchainStatistics::startup() noexcept -> void
{
    for (const auto& chain : blockchain_.EnabledChains()) {
        process_chain(chain);
    }

    finish_startup();
    reset_timer();
}

BlockchainStatistics::~BlockchainStatistics()
{
    timer_.Cancel();
    wait_for_startup();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
