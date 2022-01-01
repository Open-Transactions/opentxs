// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/rpc/AccountData.hpp"
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
class GetAccountBalance;
}  // namespace request

class AccountData;
}  // namespace rpc
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
namespace response
{
class OPENTXS_EXPORT GetAccountBalance final : public Base
{
public:
    using Data = std::pmr::vector<AccountData>;

    auto Balances() const noexcept -> const Data&;

    /// throws std::runtime_error for invalid constructor arguments
    OPENTXS_NO_EXPORT GetAccountBalance(
        const request::GetAccountBalance& request,
        Responses&& response,
        Data&& balances) noexcept(false);
    OPENTXS_NO_EXPORT GetAccountBalance(
        const proto::RPCResponse& serialized) noexcept(false);
    GetAccountBalance() noexcept;

    ~GetAccountBalance() final;

private:
    GetAccountBalance(const GetAccountBalance&) = delete;
    GetAccountBalance(GetAccountBalance&&) = delete;
    auto operator=(const GetAccountBalance&) -> GetAccountBalance& = delete;
    auto operator=(GetAccountBalance&&) -> GetAccountBalance& = delete;
};
}  // namespace response
}  // namespace rpc
}  // namespace opentxs
