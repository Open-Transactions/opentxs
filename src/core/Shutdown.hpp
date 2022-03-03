// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <future>

#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
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
class Context;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::internal
{
class ShutdownSender
{
public:
    const UnallocatedCString endpoint_;

    auto Activate() const noexcept -> void;

    auto Close() noexcept -> void;

    ShutdownSender(
        const network::zeromq::Context& zmq,
        const UnallocatedCString endpoint) noexcept;

    ~ShutdownSender();

private:
    OTZMQPublishSocket socket_;

    ShutdownSender() = delete;
    ShutdownSender(const ShutdownSender&) = delete;
    ShutdownSender(ShutdownSender&&) = delete;
    auto operator=(const ShutdownSender&) -> ShutdownSender& = delete;
    auto operator=(ShutdownSender&&) -> ShutdownSender& = delete;
};

class ShutdownReceiver
{
public:
    using Promise = std::promise<void>;
    using Future = std::shared_future<void>;
    using Callback = std::function<void(Promise&)>;
    using Endpoints = UnallocatedVector<UnallocatedCString>;

    Promise promise_;
    Future future_;

    auto Close() noexcept -> void;

    ShutdownReceiver(
        const network::zeromq::Context& zmq,
        const Endpoints endpoints,
        Callback cb) noexcept;

    ~ShutdownReceiver();

private:
    OTZMQListenCallback callback_;
    OTZMQSubscribeSocket socket_;

    ShutdownReceiver() = delete;
    ShutdownReceiver(const ShutdownReceiver&) = delete;
    ShutdownReceiver(ShutdownReceiver&&) = delete;
    auto operator=(const ShutdownReceiver&) -> ShutdownReceiver& = delete;
    auto operator=(ShutdownReceiver&&) -> ShutdownReceiver& = delete;
};
}  // namespace opentxs::internal
