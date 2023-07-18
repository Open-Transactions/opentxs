// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Context;
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace internal
{
class Context;
}  // namespace internal

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class OPENTXS_EXPORT Context
{
public:
    virtual operator void*() const noexcept = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Context& = 0;
    virtual auto PushToEndpoint(std::string_view endpoint, Message&& message)
        const noexcept -> bool = 0;
    virtual auto SpawnActor(
        const api::Context& context,
        std::string_view name,
        actor::Startup startup = DefaultStartup(),
        actor::Shutdown shutdown = DefaultShutdown(),
        actor::Processor processor = DefaultProcessor(),
        actor::StateMachine statemachine = DefaultStateMachine(),
        socket::EndpointRequests subscribe = {},
        socket::EndpointRequests pull = {},
        socket::EndpointRequests dealer = {},
        socket::SocketRequests extra = {}) const noexcept -> BatchID = 0;
    virtual auto SpawnActor(
        const api::Session& session,
        std::string_view name,
        actor::Startup startup = DefaultStartup(),
        actor::Shutdown shutdown = DefaultShutdown(),
        actor::Processor processor = DefaultProcessor(),
        actor::StateMachine statemachine = DefaultStateMachine(),
        socket::EndpointRequests subscribe = {},
        socket::EndpointRequests pull = {},
        socket::EndpointRequests dealer = {},
        socket::SocketRequests extra = {}) const noexcept -> BatchID = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Context& = 0;

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;

    virtual ~Context() = default;

protected:
    Context() noexcept = default;
};
}  // namespace opentxs::network::zeromq
