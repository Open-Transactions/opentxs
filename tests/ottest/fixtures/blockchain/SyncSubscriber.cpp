// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/SyncSubscriber.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <compare>
#include <iostream>
#include <stdexcept>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/MinedBlocks.hpp"

namespace ottest
{
struct SyncSubscriber::Imp {
    SyncSubscriber& parent_;
    const ot::api::session::Client& api_;
    const MinedBlocks& cache_;
    std::atomic_int updated_;
    std::atomic_int errors_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    auto check_update(ot::network::zeromq::Message&& in) noexcept -> void
    {
        namespace bcsync = ot::network::otdht;
        const auto base = api_.Factory().BlockchainSyncMessage(in);

        try {
            const auto& data = base->asData();
            const auto& state = data.State();
            const auto& position = state.Position();
            const auto& blocks = data.Blocks();
            const auto index = updated_++;
            const auto future = cache_.get(index);
            const auto hash = future.get();

            if (bcsync::MessageType::new_block_header != base->Type()) {
                throw std::runtime_error{"invalid message"};
            }

            if (state.Chain() != test_chain_) {
                throw std::runtime_error{"wrong chain"};
            }

            if (0 == data.PreviousCfheader().size()) {
                throw std::runtime_error{"invalid previous cfheader"};
            }

            if (0 == blocks.size()) {
                throw std::runtime_error{"no block data"};
            }

            if (const auto count = blocks.size(); 1 != count) {
                const auto error =
                    ot::CString{} +
                    "Wrong number of blocks: expected 1, received " +
                    std::to_string(count).c_str();

                throw std::runtime_error{error.c_str()};
            }

            if (position.hash_ != hash) { std::runtime_error("wrong hash"); }

            ot::LogVerbose()("received sync data for block ")(position).Flush();
        } catch (const std::exception& e) {
            std::cout << e.what() << '\n';
            ++errors_;
        }
    }
    auto wait_for_counter(const bool hard = true) noexcept -> bool
    {
        const auto limit = hard ? 300s : 10s;
        auto start = ot::Clock::now();
        const auto& expected = parent_.expected_;

        while ((updated_ < expected) && ((ot::Clock::now() - start) < limit)) {
            ot::sleep(100ms);
        }

        if (false == hard) { updated_.store(expected.load()); }

        return updated_ >= expected;
    }

    Imp(SyncSubscriber& parent,
        const ot::api::session::Client& api,
        const MinedBlocks& cache)
        : parent_(parent)
        , api_(api)
        , cache_(cache)
        , updated_(0)
        , errors_(0)
        , cb_(ot::network::zeromq::ListenCallback::Factory(
              [&](auto&& in) { check_update(std::move(in)); }))
        , socket_(
              api_.Network().ZeroMQ().Context().Internal().SubscribeSocket(cb_))
    {
        if (false == socket_->Start(sync_server_push_endpoint_)) {
            throw std::runtime_error("Failed to subscribe to updates");
        }
    }
};
}  // namespace ottest

namespace ottest
{
SyncSubscriber::SyncSubscriber(
    const ot::api::session::Client& api,
    const MinedBlocks& cache)
    : expected_(0)
    , imp_(std::make_unique<Imp>(*this, api, cache))
{
}

auto SyncSubscriber::wait(const bool hard) noexcept -> bool
{
    if (imp_->wait_for_counter(hard)) {
        auto output = (0 == imp_->errors_);
        imp_->errors_ = 0;

        return output;
    }

    return false;
}

SyncSubscriber::~SyncSubscriber() = default;
}  // namespace ottest
