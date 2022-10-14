// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/network/zeromq/zap/ZAP.hpp"
#include "internal/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Callback;
class Handler;
}  // namespace zap
}  // namespace zeromq
}  // namespace network

using OTZMQZAPHandler = Pimpl<network::zeromq::zap::Handler>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::zap
{
class Handler : virtual public zeromq::socket::Reply
{
public:
    static auto Factory(
        const zeromq::Context& context,
        const Callback& callback,
        const std::string_view threadname = {}) -> OTZMQZAPHandler;

    Handler(const Handler&) = delete;
    Handler(Handler&&) = delete;
    auto operator=(const Handler&) -> Handler& = delete;
    auto operator=(Handler&&) -> Handler& = delete;

    ~Handler() override = default;

protected:
    Handler() = default;

private:
    friend OTZMQZAPHandler;

#ifndef _WIN32
    auto clone() const noexcept -> Handler* override = 0;
#endif
};
}  // namespace opentxs::network::zeromq::zap
