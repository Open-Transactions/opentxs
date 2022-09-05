// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <exception>
#include <memory>

#include "blockchain/node/blockoracle/blockfetcher/Shared.hpp"
#include "internal/blockchain/node/blockoracle/BlockFetcher.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block

namespace node
{
class HeaderOracle;
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class BlockFetcher::Actor final
    : public opentxs::Actor<BlockFetcher::Actor, BlockFetcherJob>
{
public:
    auto Init(boost::shared_ptr<Actor> self) noexcept -> void
    {
        signal_startup(self);
    }

    Actor(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        boost::shared_ptr<Shared> shared,
        network::zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<BlockFetcher::Actor, BlockFetcherJob>;

    using Status = Shared::Status;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    boost::shared_ptr<Shared> shared_;
    const api::Session& api_;
    const node::Manager& node_;
    const node::HeaderOracle& header_oracle_;
    network::zeromq::socket::Raw& job_ready_;
    network::zeromq::socket::Raw& tip_updated_;
    network::zeromq::socket::Raw& to_blockchain_api_;
    const blockchain::Type chain_;
    Shared::Guarded& data_;
    Timer job_available_;

    auto broadcast_tip(Shared::Data& data, const block::Position& tip) noexcept
        -> void;
    auto broadcast_tip(const block::Position& tip) noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup() noexcept -> bool;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_heartbeat(Message&& msg) noexcept -> void;
    auto process_reorg(Message&& msg) noexcept -> void;
    auto process_report(Message&& msg) noexcept -> void;
    auto publish_job_ready() noexcept -> void;
    auto update_tip(Shared::Data& data) noexcept -> void;
    auto work() noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::blockoracle
