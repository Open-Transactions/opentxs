// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/rpc/AccountEvent.hpp"
#include "opentxs/rpc/response/Base.hpp"

namespace opentxs
{
namespace proto
{
class RPCResponse;
}  // namespace proto

namespace rpc
{
namespace request
{
class GetAccountActivity;
}  // namespace request

class AccountEvent;
}  // namespace rpc
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
namespace response
{
class OPENTXS_EXPORT GetAccountActivity final : public Base
{
public:
    using Events = std::pmr::vector<AccountEvent>;

    auto Activity() const noexcept -> const Events&;

    /// throws std::runtime_error for invalid constructor arguments
    OPENTXS_NO_EXPORT GetAccountActivity(
        const request::GetAccountActivity& request,
        Responses&& response,
        Events&& events) noexcept(false);
    OPENTXS_NO_EXPORT GetAccountActivity(
        const proto::RPCResponse& serialized) noexcept(false);
    GetAccountActivity() noexcept;

    ~GetAccountActivity() final;

private:
    GetAccountActivity(const GetAccountActivity&) = delete;
    GetAccountActivity(GetAccountActivity&&) = delete;
    auto operator=(const GetAccountActivity&) -> GetAccountActivity& = delete;
    auto operator=(GetAccountActivity&&) -> GetAccountActivity& = delete;
};
}  // namespace response
}  // namespace rpc
}  // namespace opentxs
