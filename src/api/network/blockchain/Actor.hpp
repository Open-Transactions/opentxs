// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>

#include "internal/api/network/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

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

namespace opentxs::api::network::blockchain
{
class Actor final : public opentxs::Actor<blockchain::Actor, BlockchainJob>
{
public:
    auto Init(boost::shared_ptr<Actor> self) noexcept -> void
    {
        signal_startup(self);
    }

    Actor(
        std::shared_ptr<const api::Session> api,
        opentxs::network::zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<blockchain::Actor, Work>;

    std::shared_ptr<const api::Session> api_p_;
    const api::Session& api_;
    opentxs::network::zeromq::socket::Raw& active_peers_;
    opentxs::network::zeromq::socket::Raw& block_available_;
    opentxs::network::zeromq::socket::Raw& block_queue_;
    opentxs::network::zeromq::socket::Raw& block_tip_;
    opentxs::network::zeromq::socket::Raw& cfilter_progress_;
    opentxs::network::zeromq::socket::Raw& cfilter_tip_;
    opentxs::network::zeromq::socket::Raw& connected_peers_;
    opentxs::network::zeromq::socket::Raw& mempool_;
    opentxs::network::zeromq::socket::Raw& reorg_;
    opentxs::network::zeromq::socket::Raw& report_status_;
    opentxs::network::zeromq::socket::Raw& sync_server_;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::api::network::blockchain
