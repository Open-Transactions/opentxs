// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "opentxs/Export.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network

using OTZMQListenCallback = Pimpl<network::zeromq::ListenCallback>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class OPENTXS_EXPORT ListenCallback
{
public:
    using ReceiveCallback = std::function<void(Message&&)>;

    static auto Factory(ReceiveCallback callback) -> OTZMQListenCallback;
    static auto Factory() -> OTZMQListenCallback;

    /// Deactivate will block until Process is no longer executing, unless it is
    /// called by the same thread as the one currently executing the callback.
    virtual auto Deactivate() const noexcept -> void = 0;
    virtual auto Process(Message&& message) const noexcept -> void = 0;

    virtual auto Replace(ReceiveCallback callback) noexcept -> void = 0;

    ListenCallback(const ListenCallback&) = delete;
    ListenCallback(ListenCallback&&) = default;
    auto operator=(const ListenCallback&) -> ListenCallback& = delete;
    auto operator=(ListenCallback&&) -> ListenCallback& = default;

    virtual ~ListenCallback() = default;

protected:
    ListenCallback() = default;

private:
    friend OTZMQListenCallback;

#ifdef _WIN32
public:
#endif
    virtual auto clone() const -> ListenCallback* = 0;
};
}  // namespace opentxs::network::zeromq
