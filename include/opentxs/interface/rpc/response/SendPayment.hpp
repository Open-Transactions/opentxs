// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/interface/rpc/response/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class SendPayment;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc::response
{
class OPENTXS_EXPORT SendPayment final : public Base
{
public:
    auto Pending() const noexcept -> const Tasks&;

    /// throws std::runtime_error for invalid constructor arguments
    OPENTXS_NO_EXPORT SendPayment(
        const request::SendPayment& request,
        Responses&& response,
        Tasks&& tasks) noexcept(false);
    OPENTXS_NO_EXPORT SendPayment(
        const proto::RPCResponse& serialized) noexcept(false);
    SendPayment() noexcept;
    SendPayment(const SendPayment&) = delete;
    SendPayment(SendPayment&&) = delete;
    auto operator=(const SendPayment&) -> SendPayment& = delete;
    auto operator=(SendPayment&&) -> SendPayment& = delete;

    ~SendPayment() final;
};
}  // namespace opentxs::rpc::response
