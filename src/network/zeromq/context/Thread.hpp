// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"  // NOLINT
#include <boost/thread/thread.hpp>

#pragma GCC diagnostic pop
#include <zmq.h>
#include <atomic>
#include <future>
#include <string_view>
#include <thread>

#include "internal/network/zeromq/Thread.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Pool;
}  // namespace internal

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::context
{
class Thread final : public zeromq::internal::Thread
{
public:
    auto Alloc() const noexcept -> alloc::Resource* final;
    auto ID() const noexcept -> std::thread::id final;

    Thread(
        const unsigned int index,
        zeromq::internal::Pool& parent,
        std::string_view endpoint) noexcept;
    Thread() = delete;
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    auto operator=(const Thread&) -> Thread& = delete;
    auto operator=(Thread&&) -> Thread& = delete;

    ~Thread() final;

private:
    struct Items {
        using ItemVector = Vector<::zmq_pollitem_t>;
        using DataVector = Vector<ReceiveCallback>;

        ItemVector items_;
        DataVector data_;

        Items(alloc::Default alloc) noexcept;
        Items(Items&& rhs) noexcept;

        ~Items();
    };

    const unsigned int index_;
    zeromq::internal::Pool& parent_;
    std::atomic_bool shutdown_;
    socket::Raw control_;
    Items data_;
    CString thread_name_;
    std::promise<std::thread::id> id_promise_;
    std::shared_future<std::thread::id> id_;
    boost::thread thread_;

    auto poll() noexcept -> void;
    auto receive_message(void* socket, Message& message) noexcept -> bool;
    auto modify(Message&& message) noexcept -> void;
    auto run() noexcept -> void;
};
}  // namespace opentxs::network::zeromq::context
