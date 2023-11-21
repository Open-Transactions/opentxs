// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Pull.hpp"
#include "internal/util/Log.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class LogPrivate;  // IWYU pragma: keep
}  // namespace internal
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::internal::LogPrivate
{
public:
    LogPrivate(
        const opentxs::network::zeromq::Context& zmq,
        const UnallocatedCString endpoint);
    LogPrivate() = delete;
    LogPrivate(const LogPrivate&) = delete;
    LogPrivate(LogPrivate&&) = delete;
    auto operator=(const LogPrivate&) -> LogPrivate& = delete;
    auto operator=(LogPrivate&&) -> LogPrivate& = delete;

    ~LogPrivate();

private:
    OTZMQListenCallback callback_;
    OTZMQPullSocket socket_;
    OTZMQPublishSocket publish_socket_;
    const bool publish_;

    auto callback(opentxs::network::zeromq::Message&& message) noexcept -> void;
    auto print(
        const int level,
        const Console console,
        const std::string_view text,
        const std::string_view thread) noexcept -> void;
};
