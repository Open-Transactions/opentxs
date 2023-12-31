// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/rpc/AccountData.hpp"
#include "opentxs/rpc/response/Message.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class RPCResponse;
}  // namespace protobuf

namespace rpc
{
namespace request
{
class GetAccountBalance;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::response
{
class OPENTXS_EXPORT GetAccountBalance final : public Message
{
public:
    using Data = UnallocatedVector<AccountData>;

    auto Balances() const noexcept -> const Data&;

    /// throws std::runtime_error for invalid constructor arguments
    OPENTXS_NO_EXPORT GetAccountBalance(
        const request::GetAccountBalance& request,
        Responses&& response,
        Data&& balances) noexcept(false);
    OPENTXS_NO_EXPORT GetAccountBalance(
        const protobuf::RPCResponse& serialized) noexcept(false);
    GetAccountBalance() noexcept;
    GetAccountBalance(const GetAccountBalance&) = delete;
    GetAccountBalance(GetAccountBalance&&) = delete;
    auto operator=(const GetAccountBalance&) -> GetAccountBalance& = delete;
    auto operator=(GetAccountBalance&&) -> GetAccountBalance& = delete;

    ~GetAccountBalance() final;
};
}  // namespace opentxs::rpc::response
