// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <thread>

#include "internal/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace alloc
{
class Logging;
}  // namespace alloc

namespace network
{
namespace zeromq
{
namespace internal
{
class Handle;
class Thread;
}  // namespace internal

class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Pool
{
public:
    virtual auto ActiveBatches(alloc::Strategy alloc = {}) const noexcept
        -> CString = 0;
    virtual auto BelongsToThreadPool(const std::thread::id) const noexcept
        -> bool = 0;
    virtual auto Parent() const noexcept -> const zeromq::Context& = 0;
    virtual auto PreallocateBatch() const noexcept -> BatchID = 0;
    virtual auto Thread(BatchID id) const noexcept
        -> zeromq::internal::Thread* = 0;
    virtual auto ThreadID(BatchID id) const noexcept -> std::thread::id = 0;

    virtual auto Alloc(BatchID id) noexcept -> alloc::Logging* = 0;
    virtual auto GetStartArgs(BatchID id) noexcept -> ThreadStartArgs = 0;
    virtual auto GetStopArgs(BatchID id) noexcept -> Set<void*> = 0;
    virtual auto DoModify(SocketID id) noexcept -> void = 0;
    virtual auto MakeBatch(
        const BatchID preallocated,
        Vector<socket::Type>&& types,
        std::string_view name) noexcept -> Handle = 0;
    virtual auto ReportShutdown(unsigned int index) noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;
    virtual auto Start(BatchID id, StartArgs&& sockets) noexcept
        -> zeromq::internal::Thread* = 0;
    virtual auto Stop(BatchID id) noexcept -> void = 0;

    virtual ~Pool() = default;
};
}  // namespace opentxs::network::zeromq::internal
