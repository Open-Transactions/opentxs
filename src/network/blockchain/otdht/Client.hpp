// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <utility>

#include "internal/network/blockchain/OTDHT.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/otdht/Actor.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace otdht
{
class Acknowledgement;
class Data;
class State;
}  // namespace otdht
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::otdht
{
class Client final : public OTDHT::Actor
{
public:
    Client(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const opentxs::blockchain::node::Manager> node,
        network::zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;

    ~Client() final;

private:
    using Pending = std::pair<Message, opentxs::blockchain::block::Position>;

    static constexpr auto queue_limit_ = 5_uz;

    opentxs::blockchain::block::Position best_remote_position_;
    opentxs::blockchain::block::Position processing_position_;
    Deque<Pending> queue_;
    bool processing_;

    auto best_position() const noexcept -> opentxs::blockchain::block::Position;
    auto can_connect(const network::otdht::Data& data) const noexcept -> bool;

    auto do_work() noexcept -> bool final;
    auto drain_queue() noexcept -> void;
    auto fill_queue() noexcept -> void;
    auto process_ack(
        const Message& msg,
        const network::otdht::Acknowledgement& ack) noexcept -> void;
    auto process_data(Message&& msg, const network::otdht::Data& data) noexcept
        -> void;
    auto process_job_processed(Message&& msg) noexcept -> void final;
    auto process_state(
        const Message& msg,
        const network::otdht::State& state) noexcept -> bool;
    auto process_sync_peer(Message&& msg) noexcept -> void final;
    auto update_remote_position(
        const opentxs::blockchain::block::Position& incoming) noexcept -> void;
};
}  // namespace opentxs::network::blockchain::otdht
