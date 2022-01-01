// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/protobuf/verify/PaymentEvent.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>

#include "internal/protobuf/verify/VerifyWorkflows.hpp"
#include "serialization/protobuf/PaymentEvent.pb.h"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

#define PROTO_NAME "payment event"

namespace opentxs::proto
{
auto CheckProto_3(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::pmr::map<PaymentEventType, std::size_t>& events) -> bool
{
    try {
        const bool valid =
            (1 == PaymentWorkflowAllowedEventTypes()
                      .at({parentVersion, parent})
                      .count(input.type()));

        if (false == valid) {
            FAIL_4(
                "Invalid type. Workflow type: ",
                static_cast<std::uint32_t>(parent),
                " Event type: ",
                static_cast<std::uint32_t>(input.type()))
        }
    } catch (const std::out_of_range&) {
        FAIL_1("Invalid event type")
    }

    switch (input.method()) {
        case TRANSPORTMETHOD_OT: {
            CHECK_IDENTIFIER(transport)
        } break;
        case TRANSPORTMETHOD_NONE:
        case TRANSPORTMETHOD_OOB: {
            CHECK_EXCLUDED(transport)
        } break;
        case TRANSPORTMETHOD_ERROR:
        default: {
            FAIL_1("Invalid transport method")
        }
    }

    try {
        const bool valid =
            (1 == PaymentEventAllowedTransportMethod()
                      .at({input.version(), input.type()})
                      .count(input.method()));

        if (false == valid) {
            FAIL_1("Transport method not allowed for this version")
        }
    } catch (const std::out_of_range&) {
        FAIL_1("Invalid event type")
    }

    switch (input.type()) {
        case proto::PAYMENTEVENTTYPE_CREATE:
        case proto::PAYMENTEVENTTYPE_CONVEY:
        case proto::PAYMENTEVENTTYPE_ACCEPT:
        case proto::PAYMENTEVENTTYPE_REJECT: {
            OPTIONAL_IDENTIFIER(nym)
        } break;
        case proto::PAYMENTEVENTTYPE_CANCEL:
        case proto::PAYMENTEVENTTYPE_COMPLETE:
        case proto::PAYMENTEVENTTYPE_ABORT:
        case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE:
        case proto::PAYMENTEVENTTYPE_EXPIRE: {
            CHECK_EXCLUDED(nym)
        } break;
        case proto::PAYMENTEVENTTYPE_ERROR:
        default: {
            FAIL_1("Invalid event type")
        }
    }

    auto& counter = events[input.type()];
    ++counter;

    return true;
}

auto CheckProto_4(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const PaymentEvent& input,
    const bool silent,
    [[maybe_unused]] const std::uint32_t parentVersion,
    [[maybe_unused]] const PaymentWorkflowType parent,
    [[maybe_unused]] std::pmr::map<PaymentEventType, std::size_t>& events)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
