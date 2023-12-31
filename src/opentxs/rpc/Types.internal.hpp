// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/RPCEnums.pb.h>

#include "opentxs/rpc/Types.hpp"

namespace opentxs::rpc
{
auto translate(AccountEventType) noexcept -> protobuf::AccountEventType;
auto translate(AccountType) noexcept -> protobuf::AccountType;
auto translate(CommandType) noexcept -> protobuf::RPCCommandType;
auto translate(ContactEventType) noexcept -> protobuf::ContactEventType;
auto translate(PaymentType) noexcept -> protobuf::RPCPaymentType;
auto translate(PushType) noexcept -> protobuf::RPCPushType;
auto translate(ResponseCode) noexcept -> protobuf::RPCResponseCode;
}  // namespace opentxs::rpc

namespace opentxs::protobuf
{
auto translate(AccountEventType) noexcept -> rpc::AccountEventType;
auto translate(AccountType) noexcept -> rpc::AccountType;
auto translate(ContactEventType) noexcept -> rpc::ContactEventType;
auto translate(RPCCommandType) noexcept -> rpc::CommandType;
auto translate(RPCPaymentType) noexcept -> rpc::PaymentType;
auto translate(RPCPushType) noexcept -> rpc::PushType;
auto translate(RPCResponseCode) noexcept -> rpc::ResponseCode;
}  // namespace opentxs::protobuf
