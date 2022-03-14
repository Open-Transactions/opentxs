// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_deferred_guarded.h>
#include <zmq.h>
#include <atomic>
#include <future>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <tuple>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Thread.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/BoostPMR.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Gatekeeper.hpp"

struct zmq_pollitem_t;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
namespace internal
{
class Pool;
class Thread;
}  // namespace internal

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

namespace opentxs::network::zeromq::context
{
class Thread final : public zeromq::internal::Thread
{
public:
    auto Add(BatchID id, StartArgs&& args) noexcept -> bool;
    auto Alloc() noexcept -> alloc::Resource* final { return &alloc_; }
    auto ID() const noexcept -> std::thread::id final
    {
        return thread_.handle_.get_id();
    }
    auto Modify(SocketID socket, ModifyCallback cb) noexcept
        -> AsyncResult final;
    auto Remove(BatchID id, UnallocatedVector<socket::Raw*>&& sockets) noexcept
        -> std::future<bool>;
    auto Shutdown() noexcept -> void;

    Thread(zeromq::internal::Pool& parent) noexcept;

    ~Thread() final;

private:
    struct Background {
        std::atomic_bool running_{false};
        std::thread handle_{};
    };
    struct Items {
        using ItemVector = UnallocatedVector<zmq_pollitem_t>;
        using DataVector = UnallocatedVector<ReceiveCallback>;

        ItemVector items_{};
        DataVector data_{};

        ~Items();
    };

    using Data = libguarded::deferred_guarded<Items, std::shared_mutex>;

    zeromq::internal::Pool& parent_;
    std::atomic_bool shutdown_;
    socket::Raw null_;
    alloc::BoostPoolSync alloc_;
    Gatekeeper gate_;
    Background thread_;
    Data data_;

    auto join() noexcept -> void;
    auto poll(Items& data) noexcept -> void;
    auto receive_message(void* socket, Message& message) noexcept -> bool;
    auto run() noexcept -> void;
    auto start() noexcept -> void;
    auto wait() noexcept -> void;

    Thread() = delete;
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    auto operator=(const Thread&) -> Thread& = delete;
    auto operator=(Thread&&) -> Thread& = delete;
};
}  // namespace opentxs::network::zeromq::context
