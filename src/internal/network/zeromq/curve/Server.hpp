// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/socket/Socket.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::curve
{
class Server : virtual public socket::Socket
{
public:
    virtual auto SetDomain(const UnallocatedCString& domain) const noexcept
        -> bool = 0;
    virtual auto SetPrivateKey(const Secret& key) const noexcept -> bool = 0;
    virtual auto SetPrivateKey(const UnallocatedCString& z85) const noexcept
        -> bool = 0;

    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;

    ~Server() override = default;

protected:
    Server() noexcept = default;
};
}  // namespace opentxs::network::zeromq::curve
