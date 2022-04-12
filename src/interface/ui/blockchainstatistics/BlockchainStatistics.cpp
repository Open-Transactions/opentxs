// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainstatistics/BlockchainStatistics.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>  // IWYU pragma: keep
#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/core/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
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
    : BlockchainStatisticsList(api, api.Factory().Identifier(), cb, false)
    , Worker(api, {})
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
    return factory::BlockchainStatisticsItem(
        *this, Widget::api_, id, index, custom);
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

    try {
        auto& data = get_cache(chain);
        auto& [header, filter, connected, active, blocks, balance] = data;
        const auto& network = blockchain_.GetChain(chain);
        connected = network.GetPeerCount();
        active = network.GetVerifiedPeerCount();
        out.emplace_back(new blockchain::block::Height{header});
        out.emplace_back(new blockchain::block::Height{filter});
        out.emplace_back(new std::size_t{connected});
        out.emplace_back(new std::size_t{active});
        out.emplace_back(new std::size_t{blocks});
        out.emplace_back(new blockchain::Amount{balance});
    } catch (...) {
        out.emplace_back(new blockchain::block::Height{-1});
        out.emplace_back(new blockchain::block::Height{-1});
        out.emplace_back(new std::size_t{0});
        out.emplace_back(new std::size_t{0});
        out.emplace_back(new std::size_t{0});
        out.emplace_back(new blockchain::Amount{0});
    }

    return out;
}

auto BlockchainStatistics::get_cache(
    const BlockchainStatisticsRowID& chain) noexcept(false) -> CachedData&
{
    if (auto i = cache_.find(chain); cache_.end() != i) {

        return i->second;
    } else {
        auto& data = cache_[chain];
        auto& [header, filter, connected, active, blocks, balance] = data;

        try {
            const auto& network = blockchain_.GetChain(chain);
            const auto& hOracle = network.HeaderOracle();
            const auto& fOracle = network.FilterOracle();
            const auto& bOracle = network.BlockOracle();
            header = hOracle.BestChain().first;
            filter = fOracle.FilterTip(fOracle.DefaultType()).first;
            connected = network.GetPeerCount();
            active = network.GetVerifiedPeerCount();
            blocks = bOracle.DownloadQueue();
            balance = network.GetBalance().second;
        } catch (...) {
            cache_.erase(chain);

            throw std::runtime_error{"chain is not active"};
        }

        return data;
    }
}

auto BlockchainStatistics::pipeline(const Message& in) noexcept -> void
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
        case Work::shutdown: {
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        case Work::blockheader: {
            process_block_header(in);
        } break;
        case Work::activepeer: {
            process_work(in);
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
            process_work(in);
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
            LogError()(OT_PRETTY_CLASS())("Unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto BlockchainStatistics::process_balance(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    try {
        auto& data = cache_[chain];
        auto& [header, filter, connected, active, blocks, balance] = data;
        balance = factory::Amount(body.at(3));
    } catch (...) {
    }

    process_chain(chain);
}

auto BlockchainStatistics::process_block(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    try {
        auto& data = cache_[chain];
        auto& [header, filter, connected, active, blocks, balance] = data;
        blocks = body.at(2).as<std::size_t>();
    } catch (...) {
    }

    process_chain(chain);
}

auto BlockchainStatistics::process_block_header(const Message& in) noexcept
    -> void
{
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    try {
        auto& data = cache_[chain];
        auto& [header, filter, connected, active, blocks, balance] = data;
        header = body.at(3).as<blockchain::block::Height>();
    } catch (...) {
    }

    process_chain(chain);
}

auto BlockchainStatistics::process_chain(
    BlockchainStatisticsRowID chain) noexcept -> void
{
    auto data = custom(chain);
    add_item(chain, UnallocatedCString{print(chain)}, data);
    delete_inactive(blockchain_.EnabledChains());
}

auto BlockchainStatistics::process_cfilter(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    try {
        auto& data = cache_[chain];
        auto& [header, filter, connected, active, blocks, balance] = data;
        filter = body.at(3).as<blockchain::block::Height>();
    } catch (...) {
    }

    process_chain(chain);
}

auto BlockchainStatistics::process_reorg(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(5 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    try {
        auto& data = cache_[chain];
        auto& [header, filter, connected, active, blocks, balance] = data;
        header = body.at(5).as<blockchain::block::Height>();
    } catch (...) {
    }

    process_chain(chain);
}

auto BlockchainStatistics::process_state(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    process_chain(body.at(1).as<blockchain::Type>());
}

auto BlockchainStatistics::process_timer(const Message& in) noexcept -> void
{
    for (auto& [chain, data] : cache_) {
        auto& [header, filter, connected, active, blocks, balance] = data;

        try {
            balance = blockchain_.GetChain(chain).GetBalance().second;
            process_chain(chain);
        } catch (...) {
        }
    }

    reset_timer();
}

auto BlockchainStatistics::process_work(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    process_chain(body.at(1).as<blockchain::Type>());
}

auto BlockchainStatistics::reset_timer() noexcept -> void
{
    if (Widget::api_.GetOptions().TestMode()) { return; }

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
