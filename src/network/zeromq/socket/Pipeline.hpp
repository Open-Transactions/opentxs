// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/unordered/unordered_flat_set.hpp>
#include <cs_deferred_guarded.h>
#include <atomic>
#include <cstddef>
#include <functional>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string_view>

#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Gatekeeper.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Thread;
}  // namespace internal

class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class Pipeline::Imp final : virtual public internal::Pipeline,
                            public opentxs::pmr::Allocated
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
    auto IsExternal(std::size_t socketID) const noexcept -> bool final
    {
        return external_.contains(socketID);
    }
    auto PullFrom(const std::string_view endpoint) const noexcept -> bool;
    auto PullFromThread(std::string_view endpoint) noexcept -> bool final;
    auto Push(zeromq::Message&& data) const noexcept -> bool;
    auto Send(zeromq::Message&& data) const noexcept -> bool;
    auto SendFromThread(zeromq::Message&& msg) noexcept -> bool final;
    auto SetCallback(Callback&& cb) const noexcept -> void final;
    auto SubscribeFromThread(std::string_view endpoint) noexcept -> bool final;
    auto SubscribeTo(const std::string_view endpoint) const noexcept -> bool;

    auto ExtraSocket(std::size_t index) noexcept(false) -> socket::Raw& final;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Imp(const zeromq::Context& context,
        Callback&& callback,
        socket::EndpointRequests::span subscribe,
        socket::EndpointRequests::span pull,
        socket::EndpointRequests::span dealer,
        socket::SocketRequests::span extra,
        socket::CurveClientRequests::span curveClient,
        socket::CurveServerRequests::span curveServer,
        const std::string_view threadname,
        const std::optional<zeromq::BatchID>& preallocated,
        allocator_type alloc) noexcept;
    Imp(const zeromq::Context& context,
        Callback&& callback,
        const CString internalEndpoint,
        const CString outgoingEndpoint,
        socket::EndpointRequests::span subscribe,
        socket::EndpointRequests::span pull,
        socket::EndpointRequests::span dealer,
        socket::SocketRequests::span extra,
        socket::CurveClientRequests::span curveClient,
        socket::CurveServerRequests::span curveServer,
        const std::string_view threadname,
        const std::optional<zeromq::BatchID>& preallocated,
        allocator_type alloc) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final;

private:
    using GuardedSocket =
        libguarded::deferred_guarded<socket::Raw, std::shared_mutex>;

    static constexpr auto fixed_sockets_ = 5_uz;

    const zeromq::Context& context_;
    const std::size_t total_socket_count_;
    mutable Gatekeeper gate_;
    mutable std::atomic<bool> shutdown_;
    mutable internal::Handle handle_;
    internal::Batch& batch_;
    socket::Raw& sub_;                   // NOTE activated by SubscribeTo()
    socket::Raw& pull_;                  // NOTE activated by PullFrom()
    socket::Raw& outgoing_;              // NOTE receives from to_dealer_
    socket::Raw& dealer_;                // NOTE activated by ConnectDealer()
    socket::Raw& internal_;              // NOTE receives from to_internal_
    mutable GuardedSocket to_dealer_;    // NOTE activated by Send()
    mutable GuardedSocket to_internal_;  // NOTE activated by Push()
    internal::Thread* thread_;
    std::span<socket::Raw> extra_;
    const boost::unordered_flat_set<
        std::size_t,
        std::hash<std::size_t>,
        std::equal_to<>,
        alloc::PMR<std::size_t>>
        external_;

    static auto apply(
        const socket::EndpointRequests& endpoint,
        socket::Raw& socket,
        alloc::Strategy alloc) noexcept -> void;

    auto bind(
        SocketID id,
        const std::string_view endpoint,
        std::function<Message(bool)> notify = {}) const noexcept -> bool;
    auto connect(
        SocketID id,
        const std::string_view endpoint,
        std::function<Message(bool)> notify = {}) const noexcept -> bool;
};
}  // namespace opentxs::network::zeromq
