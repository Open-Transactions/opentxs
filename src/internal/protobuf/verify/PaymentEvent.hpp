// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <map>

#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"

namespace opentxs
{
namespace proto
{
class PaymentEvent;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
auto CheckProto_1(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_2(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_3(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_4(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_5(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_6(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_7(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_8(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_9(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_10(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_11(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_12(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_13(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_14(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_15(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_16(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_17(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_18(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_19(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
auto CheckProto_20(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool;
}  // namespace proto
}  // namespace opentxs
