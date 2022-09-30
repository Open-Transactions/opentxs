// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <string_view>

#include "internal/util/Timer.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::internal
{
class ShutdownSender
{
public:
    const CString endpoint_;

    auto Activated() const noexcept -> bool { return activated_; }

    auto Activate() noexcept -> void;
    auto Close() noexcept -> void;

    ShutdownSender(
        const api::network::Asio& asio,
        const network::zeromq::Context& zmq,
        std::string_view endpoint,
        std::string_view name) noexcept;
    ShutdownSender() = delete;
    ShutdownSender(const ShutdownSender&) = delete;
    ShutdownSender(ShutdownSender&&) = delete;
    auto operator=(const ShutdownSender&) -> ShutdownSender& = delete;
    auto operator=(ShutdownSender&&) -> ShutdownSender& = delete;

    ~ShutdownSender();

private:
    const CString name_;

    std::atomic_bool activated_;
    OTZMQPublishSocket socket_;
    Timer repeat_;
};
}  // namespace opentxs::internal
