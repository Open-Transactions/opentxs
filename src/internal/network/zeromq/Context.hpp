// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <optional>
#include <thread>
#include <tuple>

#include "internal/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

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
class Batch;
class Handle;
class Thread;
}  // namespace internal

namespace socket
{
class Raw;
}  // namespace socket

class Pipeline;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Context : virtual public zeromq::Context
{
public:
    virtual auto Alloc(BatchID id) const noexcept -> alloc::Resource* = 0;
    virtual auto BelongsToThreadPool(
        const std::thread::id = std::this_thread::get_id()) const noexcept
        -> bool = 0;
    auto Internal() const noexcept -> const internal::Context& final
    {
        return *this;
    }
    virtual auto MakeBatch(Vector<socket::Type>&& types) const noexcept
        -> Handle = 0;
    virtual auto MakeBatch(
        const BatchID preallocated,
        Vector<socket::Type>&& types) const noexcept -> Handle = 0;
    virtual auto Modify(SocketID id, ModifyCallback cb) const noexcept
        -> std::pair<bool, std::future<bool>> = 0;
    virtual auto PreallocateBatch() const noexcept -> BatchID = 0;
    virtual auto Pipeline(
        std::function<void(zeromq::Message&&)>&& callback,
        const EndpointArgs& subscribe = {},
        const EndpointArgs& pull = {},
        const EndpointArgs& dealer = {},
        const Vector<SocketData>& extra = {},
        const std::optional<BatchID>& preallocated = std::nullopt,
        alloc::Resource* pmr = alloc::System()) const noexcept
        -> zeromq::Pipeline = 0;
    virtual auto RawSocket(socket::Type type) const noexcept -> socket::Raw = 0;
    virtual auto Start(BatchID id, StartArgs&& sockets) const noexcept
        -> Thread* = 0;
    virtual auto Thread(BatchID id) const noexcept -> Thread* = 0;
    virtual auto ThreadID(BatchID id) const noexcept -> std::thread::id = 0;

    auto Internal() noexcept -> internal::Context& final { return *this; }

    ~Context() override = default;

private:
    friend Handle;

    virtual auto Stop(BatchID id) const noexcept -> std::future<bool> = 0;
};
}  // namespace opentxs::network::zeromq::internal
