// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <chrono>
#include <exception>
#include <memory>
#include <optional>
#include <string_view>

#include "internal/network/otdht/Listener.hpp"
#include "internal/network/otdht/Node.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "network/otdht/node/Shared.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
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
namespace otdht
{
class Base;
}  // namespace otdht

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

namespace opentxs::network::otdht
{
class Listener::Actor final
    : public opentxs::Actor<Listener::Actor, ListenerJob>
{
public:
    auto Init(boost::shared_ptr<Actor> self) noexcept -> void
    {
        signal_startup(self);
    }

    Actor(
        std::shared_ptr<const api::Session> api,
        boost::shared_ptr<Node::Shared> shared,
        std::string_view routerBind,
        std::string_view routerAdvertise,
        std::string_view publishBind,
        std::string_view publishAdvertise,
        std::string_view routingID,
        std::string_view fromNode,
        zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<Listener::Actor, ListenerJob>;

    using BlockchainSockets =
        Map<opentxs::blockchain::Type, zeromq::socket::Raw&>;
    using Chains = Set<opentxs::blockchain::Type>;
    using Queue = Map<opentxs::blockchain::Type, Vector<Message>>;

    std::shared_ptr<const api::Session> api_p_;
    boost::shared_ptr<Node::Shared> shared_p_;
    const api::Session& api_;
    Node::Shared::Guarded& data_;
    zeromq::socket::Raw& external_router_;
    zeromq::socket::Raw& external_pub_;
    const CString router_public_endpoint_;
    const CString publish_public_endpoint_;
    const CString routing_id_;
    BlockchainSockets blockchain_;
    Chains active_chains_;
    Chains registered_chains_;
    Queue queue_;
    Timer registration_timer_;

    auto check_registration() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto forward_to_chain(
        opentxs::blockchain::Type chain,
        const Message& msg) noexcept -> void;
    auto forward_to_chain(
        opentxs::blockchain::Type chain,
        Message&& msg) noexcept -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto pipeline_external(const Work work, Message&& msg) noexcept -> void;
    auto pipeline_internal(const Work work, Message&& msg) noexcept -> void;
    auto process_chain_state(Message&& msg) noexcept -> void;
    auto process_external(Message&& msg) noexcept -> void;
    auto process_pushtx(Message&& msg, const otdht::Base& base) noexcept
        -> void;
    auto process_registration(Message&& msg) noexcept -> void;
    auto process_response(Message&& msg) noexcept -> void;
    auto process_sync(Message&& msg, const otdht::Base& base) noexcept -> void;
    auto process_sync_push(Message&& msg) noexcept -> void;
    auto process_sync_reply(Message&& msg) noexcept -> void;
    auto reset_registration_timer(std::chrono::microseconds interval) noexcept
        -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::network::otdht
