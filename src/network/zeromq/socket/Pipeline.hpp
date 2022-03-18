// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_deferred_guarded.h>
#include <zmq.h>
#include <atomic>
#include <cstddef>
#include <functional>
#include <future>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Allocated.hpp"
#include "util/Gatekeeper.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Thread;
}  // namespace internal

namespace socket
{
class Raw;
}  // namespace socket

class Context;
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class Pipeline::Imp final : virtual public internal::Pipeline,
                            public opentxs::implementation::Allocated
{
public:
    auto BatchID() const noexcept -> std::size_t;
    auto BindSubscriber(
        const std::string_view endpoint,
        std::function<Message(bool)> notify) const noexcept -> bool;
    auto Close() const noexcept -> bool;
    auto ConnectDealer(
        const std::string_view endpoint,
        std::function<Message(bool)> notify) const noexcept -> bool;
    auto ConnectionIDDealer() const noexcept -> std::size_t;
    auto ConnectionIDInternal() const noexcept -> std::size_t;
    auto ConnectionIDPull() const noexcept -> std::size_t;
    auto ConnectionIDSubscribe() const noexcept -> std::size_t;
    auto ExtraSocket(std::size_t index) const noexcept(false)
        -> const socket::Raw& final
    {
        return const_cast<Imp&>(*this).ExtraSocket(index);
    }
    auto PullFrom(const std::string_view endpoint) const noexcept -> bool;
    auto Push(zeromq::Message&& data) const noexcept -> bool;
    auto Send(zeromq::Message&& data) const noexcept -> bool;
    auto SendFromThread(zeromq::Message&& msg) noexcept -> bool final;
    auto SetCallback(Callback&& cb) const noexcept -> void final;
    auto SubscribeTo(const std::string_view endpoint) const noexcept -> bool;

    auto ExtraSocket(std::size_t index) noexcept(false) -> socket::Raw& final;

    Imp(const zeromq::Context& context,
        Callback&& callback,
        const EndpointArgs& subscribe,
        const EndpointArgs& pull,
        const EndpointArgs& dealer,
        const Vector<SocketData>& extra,
        const std::optional<zeromq::BatchID>& preallocated,
        allocator_type pmr) noexcept;
    Imp(const zeromq::Context& context,
        Callback&& callback,
        const CString internalEndpoint,
        const CString outgoingEndpoint,
        const EndpointArgs& subscribe,
        const EndpointArgs& pull,
        const EndpointArgs& dealer,
        const Vector<SocketData>& extra,
        const std::optional<zeromq::BatchID>& preallocated,
        allocator_type pmr) noexcept;

    ~Imp() final;

private:
    using GuardedSocket =
        libguarded::deferred_guarded<socket::Raw, std::shared_mutex>;

    static constexpr auto fixed_sockets_ = std::size_t{5};

    const zeromq::Context& context_;
    mutable Gatekeeper gate_;
    mutable std::atomic<bool> shutdown_;
    internal::Handle handle_;
    internal::Batch& batch_;
    socket::Raw& sub_;                   // NOTE activated by SubscribeTo()
    socket::Raw& pull_;                  // NOTE activated by PullFrom()
    socket::Raw& outgoing_;              // NOTE receives from to_dealer_
    socket::Raw& dealer_;                // NOTE activated by ConnectDealer()
    socket::Raw& internal_;              // NOTE receives from to_internal_
    mutable GuardedSocket to_dealer_;    // NOTE activated by Send()
    mutable GuardedSocket to_internal_;  // NOTE activated by Push()
    internal::Thread* thread_;

    static auto apply(
        const EndpointArgs& endpoint,
        socket::Raw& socket) noexcept -> void;

    auto bind(
        SocketID id,
        const std::string_view endpoint,
        std::function<Message(bool)> notify = {}) const noexcept
        -> std::pair<bool, std::future<bool>>;
    auto connect(
        SocketID id,
        const std::string_view endpoint,
        std::function<Message(bool)> notify = {}) const noexcept
        -> std::pair<bool, std::future<bool>>;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::network::zeromq
