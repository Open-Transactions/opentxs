// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cs_ordered_guarded.h>
#include <robin_hood.h>
#include <atomic>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <tuple>
#include <utility>

#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/Pool.hpp"
#include "internal/network/zeromq/Thread.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "network/zeromq/context/Thread.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Gatekeeper.hpp"

#pragma once

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
namespace context
{
class Thread;
}  // namespace context

namespace internal
{
class Batch;
class Handle;
class Thread;
}  // namespace internal

namespace socket
{
class Raw;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::context
{
using Batches = robin_hood::unordered_node_map<BatchID, internal::Batch>;
using BatchIndex =
    robin_hood::unordered_node_map<BatchID, UnallocatedVector<SocketID>>;
using SocketIndex =
    robin_hood::unordered_node_map<SocketID, std::pair<BatchID, socket::Raw*>>;

class Pool final : public zeromq::internal::Pool
{
public:
    auto BelongsToThreadPool(const std::thread::id) const noexcept
        -> bool final;
    auto Parent() const noexcept -> const zeromq::Context& final
    {
        return parent_;
    }
    auto Thread(BatchID id) const noexcept -> zeromq::internal::Thread* final;
    auto ThreadID(BatchID id) const noexcept -> std::thread::id final;

    auto Alloc(BatchID id) noexcept -> alloc::Resource* final;
    auto MakeBatch(Vector<socket::Type>&& types) noexcept -> internal::Handle;
    auto MakeBatch(const BatchID id, Vector<socket::Type>&& types) noexcept
        -> internal::Handle final;
    auto Modify(SocketID id, ModifyCallback cb) noexcept -> AsyncResult;
    auto DoModify(SocketID id, const ModifyCallback& cb) noexcept -> bool final;
    auto PreallocateBatch() const noexcept -> BatchID final;
    auto Shutdown() noexcept -> void;
    auto Start(BatchID id, StartArgs&& sockets) noexcept
        -> zeromq::internal::Thread*;
    auto Stop(BatchID id) noexcept -> std::future<bool>;
    auto UpdateIndex(BatchID id, StartArgs&& sockets) noexcept -> void final;
    auto UpdateIndex(BatchID id) noexcept -> void final;

    Pool(const Context& parent) noexcept;

    ~Pool() final;

private:
    const Context& parent_;
    const unsigned int count_;
    std::atomic<bool> running_;
    Gatekeeper gate_;
    robin_hood::unordered_node_map<unsigned int, context::Thread> threads_;
    libguarded::ordered_guarded<Batches, std::shared_mutex> batches_;
    libguarded::ordered_guarded<BatchIndex, std::shared_mutex> batch_index_;
    libguarded::ordered_guarded<SocketIndex, std::shared_mutex> socket_index_;

    auto get(BatchID id) const noexcept -> const context::Thread&;

    auto get(BatchID id) noexcept -> context::Thread&;
    auto stop() noexcept -> void;

    Pool() = delete;
    Pool(const Pool&) = delete;
    Pool(Pool&&) = delete;
    auto operator=(const Pool&) -> Pool& = delete;
    auto operator=(Pool&&) -> Pool& = delete;
};
}  // namespace opentxs::network::zeromq::context
