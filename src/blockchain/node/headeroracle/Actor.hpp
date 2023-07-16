// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::node::internal::HeaderOracle::Actor

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>

#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/blockchain/node/headeroracle/Types.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
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
using HeaderOracleActor =
    opentxs::Actor<HeaderOracle::Actor, headeroracle::Job>;

class HeaderOracle::Actor final : public HeaderOracleActor
{
public:
    auto Init(boost::shared_ptr<Actor> me) noexcept -> void;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

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
    friend HeaderOracleActor;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    boost::shared_ptr<Shared> shared_p_;
    const api::Session& api_;
    const node::Manager& node_;
    Shared& shared_;
    network::zeromq::socket::Raw& job_ready_;
    const blockchain::Type chain_;
    Timer job_timer_;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto process_job_finished(Message&& in) noexcept -> void;
    auto process_report(Message&& msg) noexcept -> void;
    auto process_submit_submit_block_hash(
        Message&& in,
        allocator_type monotonic) noexcept -> void;
    auto process_submit_block_header(Message&& in) noexcept -> void;
    auto process_update_remote_height(
        Message&& in,
        allocator_type monotonic) noexcept -> void;
    auto reset_job_timer() noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::internal
