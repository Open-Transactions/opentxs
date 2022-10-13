// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/curve/Server.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Pull;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQPullSocket = Pimpl<network::zeromq::socket::Pull>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
class Pull : virtual public curve::Server
{
public:
    Pull(const Pull&) = delete;
    Pull(Pull&&) = delete;
    auto operator=(const Pull&) -> Pull& = delete;
    auto operator=(Pull&&) -> Pull& = delete;

    ~Pull() override = default;

protected:
    Pull() noexcept = default;

private:
    friend OTZMQPullSocket;

    virtual auto clone() const noexcept -> Pull* = 0;
};
}  // namespace opentxs::network::zeromq::socket
