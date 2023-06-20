// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::node::internal::BlockOracle::Actor

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <functional>
#include <memory>
#include <span>

#include "blockchain/node/Downloader.hpp"
#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
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
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
using namespace blockoracle;
using BlockOracleActor = opentxs::Actor<BlockOracle::Actor, Job>;

class BlockOracle::Actor final : public BlockOracleActor
{
public:
    auto Init(boost::shared_ptr<Actor> me) noexcept -> void;

    Actor(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        boost::shared_ptr<Shared> shared,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend BlockOracleActor;

    using ConnectionID = network::zeromq::Envelope;
    using Requests = Map<block::Hash, Set<ConnectionID>>;
    using Notifications = Map<ConnectionID, Message>;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    boost::shared_ptr<Shared> shared_p_;
    const api::Session& api_;
    const node::Manager& node_;
    Shared& shared_;
    network::zeromq::socket::Raw& router_;
    network::zeromq::socket::Raw& tip_updated_;
    network::zeromq::socket::Raw& to_blockchain_api_;
    const blockchain::Type chain_;
    Requests requests_;
    Downloader downloader_;

    auto broadcast_tip() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto notify_requestors(
        std::span<const block::Hash> ids,
        std::span<const BlockLocation> blocks,
        allocator_type monotonic) noexcept -> void;
    auto notify_requestors(
        const block::Hash& hash,
        const BlockLocation& data,
        Notifications& out) noexcept -> void;
    auto notify_requestors(Notifications& messages) noexcept -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto process_block_ready(Message&& msg, allocator_type monotonic) noexcept
        -> void;
    auto process_header(Message&& msg) noexcept -> void;
    auto process_reorg(Message&& msg) noexcept -> void;
    auto process_report(Message&& msg) noexcept -> void;
    auto process_request_blocks(
        Message&& msg,
        allocator_type monotonic) noexcept -> void;
    auto process_submit_block(Message&& msg, allocator_type monotonic) noexcept
        -> void;
    auto queue_blocks(allocator_type monotonic) noexcept -> bool;
    auto set_tip(const block::Position& tip) noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::internal
