// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <tuple>

#include "internal/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"

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
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::internal
{
class Context : virtual public zeromq::Context
{
public:
    auto Internal() const noexcept -> const internal::Context& final
    {
        return *this;
    }
    virtual auto MakeBatch(std::pmr::vector<socket::Type>&& types)
        const noexcept -> internal::Batch& = 0;
    virtual auto Modify(SocketID id, ModifyCallback cb) const noexcept
        -> std::pair<bool, std::future<bool>> = 0;
    virtual auto Start(BatchID id, StartArgs&& sockets) const noexcept
        -> Thread* = 0;
    virtual auto Stop(BatchID id) const noexcept -> std::future<bool> = 0;

    auto Internal() noexcept -> internal::Context& final { return *this; }

    ~Context() override = default;
};
}  // namespace opentxs::network::zeromq::internal
