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
auto CheckProto_1(
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
        case proto::PAYMENTEVENTTYPE_ACCEPT: {
            OPTIONAL_IDENTIFIER(nym)
        } break;
        case proto::PAYMENTEVENTTYPE_CANCEL:
        case proto::PAYMENTEVENTTYPE_COMPLETE: {
            CHECK_EXCLUDED(nym)
        } break;
        case proto::PAYMENTEVENTTYPE_ABORT:
        case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE:
        case proto::PAYMENTEVENTTYPE_EXPIRE:
        case proto::PAYMENTEVENTTYPE_REJECT:
        case proto::PAYMENTEVENTTYPE_ERROR:
        default: {
            FAIL_1("Invalid event type")
        }
    }

    auto& counter = events[input.type()];
    ++counter;

    return true;
}
}  // namespace opentxs::proto
