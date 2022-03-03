// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ostream>
#include <string_view>

#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/zap/Handler.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"
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

namespace zap
{
class Callback;
}  // namespace zap
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::zap::implementation
{
class Handler final
    : virtual zap::Handler,
      zeromq::socket::implementation::Receiver<zap::Handler, zap::Request>,
      zeromq::curve::implementation::Server
{
public:
    auto Start(const std::string_view endpoint) const noexcept -> bool final
    {
        return false;
    }

    ~Handler() final;

private:
    friend zap::Handler;

    const zap::Callback& callback_;

    auto clone() const noexcept -> Handler* final
    {
        return new Handler(context_, callback_);
    }
    auto have_callback() const noexcept -> bool final { return true; }

    void init() noexcept final;
    void process_incoming(const Lock& lock, zap::Request&& message) noexcept
        final;

    Handler(
        const zeromq::Context& context,
        const zap::Callback& callback) noexcept;
    Handler() = delete;
    Handler(const Handler&) = delete;
    Handler(Handler&&) = delete;
    auto operator=(const Handler&) -> Handler& = delete;
    auto operator=(Handler&&) -> Handler& = delete;
};
}  // namespace opentxs::network::zeromq::zap::implementation
