// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <thread>
#include <tuple>

#include "internal/network/zeromq/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Thread
{
public:
    virtual auto ID() const noexcept -> std::thread::id = 0;

    virtual auto Alloc() noexcept -> alloc::Resource* = 0;
    virtual auto Modify(SocketID socket, ModifyCallback cb) noexcept
        -> std::pair<bool, std::future<bool>> = 0;

    virtual ~Thread() = default;
};
}  // namespace opentxs::network::zeromq::internal
