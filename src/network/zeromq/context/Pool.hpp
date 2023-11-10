// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#include <ankerl/unordered_dense.h>
#include <boost/unordered/unordered_node_map.hpp>
#include <cs_ordered_guarded.h>
#include <cs_plain_guarded.h>
#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string_view>
#include <thread>
#include <utility>

#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/Pool.hpp"
#include "internal/network/zeromq/Thread.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "network/zeromq/context/Thread.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Gatekeeper.hpp"

#pragma once

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
}  // namespace internal
}  // namespace zeromq
}  // namespace network

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::context
{
class Pool final : public zeromq::internal::Pool
{
public:
    auto ActiveBatches(alloc::Default alloc = {}) const noexcept
        -> CString final;
    auto BelongsToThreadPool(const std::thread::id) const noexcept
        -> bool final;
    auto Parent() const noexcept -> const zeromq::Context& final
    {
        return parent_;
    }
    auto PreallocateBatch() const noexcept -> BatchID final;
    auto Thread(BatchID id) const noexcept -> zeromq::internal::Thread* final;
    auto ThreadID(BatchID id) const noexcept -> std::thread::id final;

    auto Alloc(BatchID id) noexcept -> alloc::Logging* final;
    auto DoModify(SocketID id) noexcept -> void final;
    auto GetStartArgs(BatchID id) noexcept -> ThreadStartArgs final;
    auto GetStopArgs(BatchID id) noexcept -> Set<void*> final;
    auto MakeBatch(Vector<socket::Type>&& types, std::string_view name) noexcept
        -> internal::Handle;
    auto MakeBatch(
        const BatchID id,
        Vector<socket::Type>&& types,
        std::string_view name) noexcept -> internal::Handle final;
    auto Modify(SocketID id, ModifyCallback cb) noexcept -> void;
    auto ReportShutdown(unsigned int index) noexcept -> void final;
    auto Shutdown() noexcept -> void final;
    auto Start(BatchID id, StartArgs&& sockets) noexcept
        -> zeromq::internal::Thread* final;
    auto Stop(BatchID id) noexcept -> void final;

    Pool(
        const opentxs::Options& args,
        std::shared_ptr<const Context> parent) noexcept;
    Pool() = delete;
    Pool(const Pool&) = delete;
    Pool(Pool&&) = delete;
    auto operator=(const Pool&) -> Pool& = delete;
    auto operator=(Pool&&) -> Pool& = delete;

    ~Pool() final;

private:
    using GuardedSocket = libguarded::plain_guarded<socket::Raw>;
    using ThreadNotifier = std::pair<CString, GuardedSocket>;
    using StartMap = Map<BatchID, StartArgs>;
    using StopMap = Map<BatchID, Set<void*>>;
    using ModifyMap = Map<SocketID, Vector<ModifyCallback>>;
    using Batches =
        ankerl::unordered_dense::map<BatchID, std::shared_ptr<internal::Batch>>;
    using BatchIndex = ankerl::unordered_dense::map<BatchID, Vector<SocketID>>;
    using SocketIndex = ankerl::unordered_dense::
        map<SocketID, std::pair<BatchID, socket::Raw*>>;
    using AllocatorMap = Map<BatchID, alloc::Logging>;

    struct Indices {
        BatchIndex batch_{};
        SocketIndex socket_{};

        auto clear() noexcept -> void
        {
            batch_.clear();
            socket_.clear();
        }
    };

    std::shared_ptr<const Context> parent_p_;
    const Context& parent_;
    const unsigned int count_;
    const std::filesystem::path log_dir_;
    const bool write_;
    std::atomic<unsigned int> shutdown_counter_;
    std::atomic<bool> running_;
    Gatekeeper gate_;
    boost::unordered_node_map<unsigned int, ThreadNotifier> notify_;
    boost::unordered_node_map<unsigned int, context::Thread> threads_;
    libguarded::ordered_guarded<Batches, std::shared_mutex> batches_;
    libguarded::ordered_guarded<Indices, std::shared_mutex> index_;
    libguarded::plain_guarded<StartMap> start_args_;
    libguarded::plain_guarded<StopMap> stop_args_;
    libguarded::plain_guarded<ModifyMap> modify_args_;
    mutable libguarded::plain_guarded<AllocatorMap> allocators_;

    auto allocate_next_batch() const noexcept -> BatchID;
    auto get(BatchID id) const noexcept -> const context::Thread&;

    auto get(BatchID id) noexcept -> context::Thread&;
    auto socket(BatchID id) noexcept -> GuardedSocket&;
    auto stop() noexcept -> void;
    auto stop_batch(BatchID id) noexcept -> void;
};
}  // namespace opentxs::network::zeromq::context
