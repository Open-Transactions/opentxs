// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
class Connection;
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTConnectionRequest = SharedPimpl<contract::peer::request::Connection>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class Connection : virtual public peer::Request
{
public:
    Connection(const Connection&) = delete;
    Connection(Connection&&) = delete;
    auto operator=(const Connection&) -> Connection& = delete;
    auto operator=(Connection&&) -> Connection& = delete;

    ~Connection() override = default;

protected:
    Connection() noexcept = default;

private:
    friend OTConnectionRequest;

#ifndef _WIN32
    auto clone() const noexcept -> Connection* override = 0;
#endif
};
}  // namespace opentxs::contract::peer::request