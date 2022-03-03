// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/interface/rpc/AccountEvent.hpp"
#include "opentxs/interface/rpc/response/Base.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::response
{
class OPENTXS_EXPORT GetAccountActivity final : public Base
{
public:
    using Events = UnallocatedVector<AccountEvent>;

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
}  // namespace opentxs::rpc::response
