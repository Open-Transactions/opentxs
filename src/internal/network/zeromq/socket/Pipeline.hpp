// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Pipeline
{
public:
    using Callback = std::function<void(zeromq::Message&&)>;

    virtual auto IsExternal(std::size_t socketID) const noexcept -> bool = 0;

    /**  Access and extra socket that was specified at construction time
     *
     *   \throws std::out_of_range for an invalid index
     */
    virtual auto ExtraSocket(std::size_t index) noexcept(false)
        -> socket::Raw& = 0;
    virtual auto SetCallback(Callback&& cb) const noexcept -> void = 0;

    /**  Access and extra socket that was specified at construction time
     *
     *   \throws std::out_of_range for an invalid index
     */
    virtual auto ExtraSocket(std::size_t index) const noexcept(false)
        -> const socket::Raw& = 0;
    /**  Connect the pull socket to specified endpoint
     *
     *   \warning this must only be called from inside the callback function
     *            being executed by the zmq thread pool.
     *
     *   \warning the supplied endpoint must be null terminated.
     */
    virtual auto PullFromThread(std::string_view endpoint) noexcept -> bool = 0;
    /**  Send from the dealer socket
     *
     *   \warning this must only be called from inside the callback function
     *            being executed by the zmq thread pool.
     */
    virtual auto SendFromThread(zeromq::Message&& msg) noexcept -> bool = 0;
    /**  Connect the subscribe socket to specified endpoint
     *
     *   \warning this must only be called from inside the callback function
     *            being executed by the zmq thread pool.
     *
     *   \warning the supplied endpoint must be null terminated.
     */
    virtual auto SubscribeFromThread(std::string_view endpoint) noexcept
        -> bool = 0;

    virtual ~Pipeline() = default;
};
}  // namespace opentxs::network::zeromq::internal
