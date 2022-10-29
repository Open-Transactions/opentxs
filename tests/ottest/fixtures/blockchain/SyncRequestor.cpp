// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/SyncRequestor.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <compare>
#include <mutex>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/MinedBlocks.hpp"

namespace ottest
{
struct SyncRequestor::Imp {
    using Buffer = ot::UnallocatedDeque<ot::network::zeromq::Message>;
    using Dir = ot::network::zeromq::socket::Direction;

    SyncRequestor& parent_;
    const ot::api::session::Client& api_;
    const MinedBlocks& cache_;
    mutable std::mutex lock_;
    std::atomic_int updated_;
    Buffer buffer_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQDealerSocket socket_;

    auto cb(ot::network::zeromq::Message&& in) noexcept -> void
    {
        auto lock = ot::Lock{lock_};
        buffer_.emplace_back(std::move(in));
        ++updated_;
    }

    Imp(SyncRequestor& parent,
        const ot::api::session::Client& api,
        const MinedBlocks& cache) noexcept
        : parent_(parent)
        , api_(api)
        , cache_(cache)
        , lock_()
        , updated_(0)
        , buffer_()
        , cb_(ot::network::zeromq::ListenCallback::Factory(
              [&](auto&& msg) { cb(std::move(msg)); }))
        , socket_(
              api.Network().ZeroMQ().Internal().DealerSocket(cb_, Dir::Connect))
    {
        socket_->Start(sync_server_main_endpoint_);
    }
};
}  // namespace ottest

namespace ottest
{
SyncRequestor::SyncRequestor(
    const ot::api::session::Client& api,
    const MinedBlocks& cache) noexcept
    : checked_(-1)
    , expected_(0)
    , imp_(std::make_unique<Imp>(*this, api, cache))
{
}

auto SyncRequestor::check(
    const ot::network::otdht::State& state,
    const ot::blockchain::block::Position& pos) const noexcept -> bool
{
    auto output{true};
    output &= (state.Chain() == test_chain_);
    output &= (state.Position() == pos);

    EXPECT_EQ(state.Chain(), test_chain_);
    EXPECT_EQ(state.Position(), pos);

    return output;
}

auto SyncRequestor::check(
    const ot::network::otdht::State& state,
    const std::size_t index) const -> bool
{
    auto pos = [&] {
        const auto handle =
            imp_->api_.Network().Blockchain().GetChain(test_chain_);

        OT_ASSERT(handle);

        const auto& chain = handle.get();
        auto header =
            chain.HeaderOracle().LoadHeader(imp_->cache_.get(index).get());

        OT_ASSERT(header);

        return header->Position();
    }();

    return check(state, pos);
}

auto SyncRequestor::check(
    const ot::network::otdht::Block& block,
    const std::size_t index) const noexcept -> bool
{
    constexpr auto filterType{ot::blockchain::cfilter::Type::ES};
    const auto handle = imp_->api_.Network().Blockchain().GetChain(test_chain_);

    OT_ASSERT(handle);

    const auto& chain = handle.get();
    const auto header =
        chain.HeaderOracle().LoadHeader(imp_->cache_.get(index).get());

    OT_ASSERT(header);

    auto headerBytes = ot::Space{};

    EXPECT_TRUE(header->Serialize(ot::writer(headerBytes)));

    const auto& pos = header->Position();
    auto output{true};
    output &= (block.Chain() == test_chain_);
    output &= (block.Height() == pos.height_);
    output &= (block.Header() == ot::reader(headerBytes));
    output &= (block.FilterType() == filterType);
    // TODO verify filter

    EXPECT_EQ(block.Chain(), test_chain_);
    EXPECT_EQ(block.Height(), pos.height_);
    EXPECT_EQ(block.Header(), ot::reader(headerBytes));
    EXPECT_EQ(block.FilterType(), filterType);

    return output;
}

auto SyncRequestor::get(const std::size_t index) const
    -> const ot::network::zeromq::Message&
{
    auto lock = ot::Lock{imp_->lock_};

    return imp_->buffer_.at(index);
}

auto SyncRequestor::request(
    const ot::blockchain::block::Position& pos) const noexcept -> bool
{
    return request(opentxs::factory::BlockchainSyncRequest([&] {
        auto out = ot::network::otdht::StateData{};
        out.emplace_back(test_chain_, pos);

        return out;
    }()));
}

auto SyncRequestor::request(
    const ot::network::otdht::Base& command) const noexcept -> bool
{
    try {
        return imp_->socket_->Send([&] {
            auto out = opentxs::network::zeromq::Message{};

            if (false == command.Serialize(out)) {
                throw std::runtime_error{"serialization error"};
            }

            return out;
        }());
    } catch (...) {
        EXPECT_TRUE(false);

        return false;
    }
}

auto SyncRequestor::wait(const bool hard) noexcept -> bool
{
    const auto limit = hard ? 300s : 10s;
    auto start = ot::Clock::now();

    while ((imp_->updated_ < expected_) &&
           ((ot::Clock::now() - start) < limit)) {
        ot::Sleep(100ms);
    }

    if (false == hard) { imp_->updated_.store(expected_.load()); }

    return imp_->updated_ >= expected_;
}

SyncRequestor::~SyncRequestor() = default;
}  // namespace ottest
