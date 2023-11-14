// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/socket/Publish.hpp"  // IWYU pragma: keep

#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Sender.hpp"
#include "network/zeromq/socket/Socket.hpp"  // IWYU pragma: keep

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket::implementation
{
class Publish final : public Sender<zeromq::socket::Publish>,
                      public zeromq::curve::implementation::Server
{
public:
    Publish(const zeromq::Context& context) noexcept;
    Publish() = delete;
    Publish(const Publish&) = delete;
    Publish(Publish&&) = delete;
    auto operator=(const Publish&) -> Publish& = delete;
    auto operator=(Publish&&) -> Publish& = delete;

    ~Publish() final;

private:
    auto clone() const noexcept -> Publish* final
    {
        return new Publish(context_);
    }
};
}  // namespace opentxs::network::zeromq::socket::implementation
