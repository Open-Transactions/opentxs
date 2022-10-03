// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/interface/rpc/AccountEventType.hpp"
// IWYU pragma: no_include "opentxs/interface/rpc/AccountType.hpp"
// IWYU pragma: no_include "opentxs/interface/rpc/CommandType.hpp"
// IWYU pragma: no_include "opentxs/interface/rpc/ContactEventType.hpp"
// IWYU pragma: no_include "opentxs/interface/rpc/PaymentType.hpp"
// IWYU pragma: no_include "opentxs/interface/rpc/PushType.hpp"
// IWYU pragma: no_include "opentxs/interface/rpc/ResponseCode.hpp"

#pragma once

#include <RPCEnums.pb.h>
#include <memory>

#include "opentxs/interface/rpc/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class RPCCommand;
class RPCResponse;
}  // namespace proto

namespace rpc
{
namespace request
{
class Base;
}  // namespace request

namespace response
{
class Base;
}  // namespace response
}  // namespace rpc
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
auto translate(const rpc::AccountEventType type) noexcept
    -> proto::AccountEventType;
auto translate(const rpc::AccountType type) noexcept -> proto::AccountType;
auto translate(const rpc::CommandType type) noexcept -> proto::RPCCommandType;
auto translate(const rpc::ContactEventType type) noexcept
    -> proto::ContactEventType;
auto translate(const rpc::PaymentType type) noexcept -> proto::RPCPaymentType;
auto translate(const rpc::PushType type) noexcept -> proto::RPCPushType;
auto translate(const rpc::ResponseCode type) noexcept -> proto::RPCResponseCode;

auto translate(const proto::AccountEventType type) noexcept
    -> rpc::AccountEventType;
auto translate(const proto::AccountType type) noexcept -> rpc::AccountType;
auto translate(const proto::ContactEventType type) noexcept
    -> rpc::ContactEventType;
auto translate(const proto::RPCCommandType type) noexcept -> rpc::CommandType;
auto translate(const proto::RPCPaymentType type) noexcept -> rpc::PaymentType;
auto translate(const proto::RPCPushType type) noexcept -> rpc::PushType;
auto translate(const proto::RPCResponseCode type) noexcept -> rpc::ResponseCode;
}  // namespace opentxs

namespace opentxs::rpc::internal
{
struct RPC {
    virtual auto Process(const proto::RPCCommand& command) const
        -> proto::RPCResponse = 0;
    virtual auto Process(const request::Base& command) const
        -> std::unique_ptr<response::Base> = 0;

    virtual ~RPC() = default;
};
}  // namespace opentxs::rpc::internal
