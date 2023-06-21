// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/socket/Push.hpp"  // IWYU pragma: keep

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Sender.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"

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
class Push final : public Sender<zeromq::socket::Push>,
                   public zeromq::curve::implementation::Client
{
public:
    Push(const zeromq::Context& context, const Direction direction) noexcept;
    Push() = delete;
    Push(const Push&) = delete;
    Push(Push&&) = delete;
    auto operator=(const Push&) -> Push& = delete;
    auto operator=(Push&&) -> Push& = delete;

    ~Push() final;

private:
    auto clone() const noexcept -> Push* final
    {
        return new Push(context_, direction_);
    }
};
}  // namespace opentxs::network::zeromq::socket::implementation
