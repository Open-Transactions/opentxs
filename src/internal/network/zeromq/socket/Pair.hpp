// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/network/zeromq/socket/Sender.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Pair;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQPairSocket = Pimpl<network::zeromq::socket::Pair>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
class Pair : virtual public socket::Socket, virtual public Sender
{
public:
    virtual auto Endpoint() const noexcept -> std::string_view = 0;

    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    auto operator=(const Pair&) -> Pair& = delete;
    auto operator=(Pair&&) -> Pair& = delete;

    ~Pair() override = default;

protected:
    Pair() noexcept = default;

private:
    friend OTZMQPairSocket;

    virtual auto clone() const noexcept -> Pair* = 0;
};
}  // namespace opentxs::network::zeromq::socket
