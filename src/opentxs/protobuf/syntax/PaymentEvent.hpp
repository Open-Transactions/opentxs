// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::PaymentEventType

#pragma once

#include <opentxs/protobuf/PaymentWorkflowEnums.pb.h>
#include <cstddef>
#include <cstdint>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class PaymentEvent;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_2(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_3(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_4(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_5(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_6(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_7(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_8(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_9(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_10(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_11(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_12(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_13(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_14(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_15(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_16(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_17(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_18(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_19(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
auto version_20(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool;
}  // namespace opentxs::protobuf::inline syntax
