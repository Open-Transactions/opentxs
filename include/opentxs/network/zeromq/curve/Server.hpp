// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/zeromq/socket/Socket.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::curve
{
class OPENTXS_EXPORT Server : virtual public socket::Socket
{
public:
    virtual auto SetDomain(const UnallocatedCString& domain) const noexcept
        -> bool = 0;
    virtual auto SetPrivateKey(const Secret& key) const noexcept -> bool = 0;
    virtual auto SetPrivateKey(const UnallocatedCString& z85) const noexcept
        -> bool = 0;

    ~Server() override = default;

protected:
    Server() noexcept = default;

private:
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;
};
}  // namespace opentxs::network::zeromq::curve
