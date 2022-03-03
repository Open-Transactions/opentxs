// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <tuple>

#include "internal/network/p2p/Server.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "opentxs/util/Container.hpp"
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
namespace p2p
{
class Base;
}  // namespace p2p

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
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::p2p
{
class Server::Imp
{
public:
    using Map = UnallocatedMap<
        Chain,
        std::tuple<
            UnallocatedCString,
            bool,
            std::reference_wrapper<zeromq::socket::Raw>>>;

    const api::Session& api_;
    const zeromq::Context& zmq_;
    zeromq::internal::Handle handle_;
    zeromq::internal::Batch& batch_;
    const zeromq::ListenCallback& external_callback_;
    const zeromq::ListenCallback& internal_callback_;
    zeromq::socket::Raw& sync_;
    zeromq::socket::Raw& update_;
    zeromq::socket::Raw& wallet_;
    mutable std::mutex map_lock_;
    Map map_;
    zeromq::internal::Thread* thread_;
    UnallocatedCString sync_endpoint_;
    UnallocatedCString sync_public_endpoint_;
    UnallocatedCString update_endpoint_;
    UnallocatedCString update_public_endpoint_;
    std::atomic_bool started_;
    std::atomic_bool running_;
    mutable Gatekeeper gate_;

    Imp(const api::Session& api, const zeromq::Context& zmq) noexcept;

    ~Imp();

private:
    auto process_external(zeromq::Message&& incoming) noexcept -> void;
    auto process_internal(zeromq::Message&& incoming) noexcept -> void;
    auto process_pushtx(
        zeromq::Message&& incoming,
        const p2p::Base& base) noexcept -> void;
    auto process_sync(
        zeromq::Message&& incoming,
        const p2p::Base& base) noexcept -> void;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::network::p2p
