// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <zmq.h>

#pragma once

#include <atomic>
#include <deque>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Thread.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/Types.hpp"
#include "util/Gatekeeper.hpp"

struct zmq_pollitem_t;

namespace opentxs
{
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
}  // namespace opentxs

namespace opentxs::network::zeromq::context
{
class Thread final : public zeromq::internal::Thread
{
public:
    auto Add(BatchID id, StartArgs&& args) noexcept -> bool;
    auto Modify(SocketID socket, ModifyCallback cb) noexcept
        -> AsyncResult final;
    auto Remove(BatchID id, std::pmr::vector<socket::Raw*>&& sockets) noexcept
        -> std::future<bool>;
    auto Shutdown() noexcept -> void;

    Thread(zeromq::internal::Pool& parent) noexcept;

    ~Thread() final;

private:
    struct Background {
        mutable std::mutex lock_{};
        std::atomic_bool running_{false};
        std::thread handle_{};
    };
    struct Items {
        using ItemVector = std::pmr::vector<zmq_pollitem_t>;
        using DataVector = std::pmr::vector<ReceiveCallback>;

        ItemVector items_{};
        DataVector data_{};

        ~Items();
    };
    struct Modification {
        SocketID socket_;
        ModifyCallback callback_;
        std::promise<bool> promise_;

        Modification(SocketID socket, ModifyCallback&& cb) noexcept
            : socket_(socket)
            , callback_(std::move(cb))
            , promise_()
        {
        }

    private:
        Modification(const Modification&) = delete;
        Modification(Modification&&) = delete;
        auto operator=(const Modification&) -> Modification& = delete;
        auto operator=(Modification&&) -> Modification& = delete;
    };
    struct Queue {
        mutable std::mutex lock_{};
        std::queue<Modification> queue_{};
    };

    zeromq::internal::Pool& parent_;
    socket::Raw null_;
    Gatekeeper gate_;
    Items poll_;
    Background thread_;
    Queue modify_;

    auto modify() noexcept -> void;
    auto join() noexcept -> void;
    auto receive_message(void* socket, Message& message) noexcept -> bool;
    auto run() noexcept -> void;
    auto wait() noexcept -> void;

    Thread() = delete;
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    auto operator=(const Thread&) -> Thread& = delete;
    auto operator=(Thread&&) -> Thread& = delete;
};
}  // namespace opentxs::network::zeromq::context
