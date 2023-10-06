// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "util/Actor.hpp"

#include <ankerl/unordered_dense.h>
#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/WorkType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Context;
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class Actor final : public opentxs::Actor<zeromq::Actor, OTZMQWorkType>
{
public:
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Init(std::shared_ptr<Actor> self) noexcept -> void
    {
        signal_startup(self);
    }

    Actor(
        const api::Context& context,
        std::string_view name,
        actor::Startup startup,
        actor::Shutdown shutdown,
        actor::Processor processor,
        actor::StateMachine statemachine,
        socket::EndpointRequests subscribe,
        socket::EndpointRequests pull,
        socket::EndpointRequests dealer,
        socket::SocketRequests extra,
        zeromq::BatchID batchID,
        std::size_t extraCount,
        allocator_type alloc) noexcept;
    Actor(
        const api::Session& session,
        std::string_view name,
        actor::Startup startup,
        actor::Shutdown shutdown,
        actor::Processor processor,
        actor::StateMachine statemachine,
        socket::EndpointRequests subscribe,
        socket::EndpointRequests pull,
        socket::EndpointRequests dealer,
        socket::SocketRequests extra,
        zeromq::BatchID batchID,
        std::size_t extraCount,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<zeromq::Actor, OTZMQWorkType>;

    using IDMap =
        ankerl::unordered_dense::pmr::map<SocketID, actor::SocketIndex>;

    static constexpr auto fixed_ = 4_uz;

    const IDMap index_;
    actor::Startup startup_;
    actor::Shutdown shutdown_;
    actor::Processor processor_;
    actor::StateMachine state_;

    auto get_index(SocketID id) const noexcept -> actor::SocketIndex;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto send(actor::SocketIndex index, std::span<Message> messages) noexcept
        -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::network::zeromq
