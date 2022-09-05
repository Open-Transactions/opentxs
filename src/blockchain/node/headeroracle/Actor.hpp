// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::node::internal::HeaderOracle::Actor

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <exception>
#include <memory>

#include "blockchain/node/headeroracle/Shared.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/blockchain/node/headeroracle/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
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
class Header;
}  // namespace block

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

class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
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
    auto do_startup() noexcept -> bool;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_job_finished(Message&& in) noexcept -> void;
    auto process_report(Message&& msg) noexcept -> void;
    auto process_submit_submit_block_hash(Message&& in) noexcept -> void;
    auto process_submit_block_header(Message&& in) noexcept -> void;
    auto process_update_remote_height(Message&& in) noexcept -> void;
    auto reset_job_timer() noexcept -> void;
    auto work() noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::internal
