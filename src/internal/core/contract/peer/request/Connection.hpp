// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/request/Base.hpp"
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
namespace internal
{
class Connection;
}  // namespace internal
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTConnectionRequest =
    SharedPimpl<contract::peer::request::internal::Connection>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::internal
{
class Connection : virtual public internal::Request
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
};
}  // namespace opentxs::contract::peer::request::internal
