// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/PaymentEvent.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PaymentEvent.pb.h>
#include <opentxs/protobuf/PaymentWorkflowEnums.pb.h>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyWorkflows.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const PaymentEvent& input,
    const Log& log,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    UnallocatedMap<PaymentEventType, std::size_t>& events) -> bool
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
                static_cast<std::uint32_t>(input.type()));
        }
    } catch (const std::out_of_range&) {
        FAIL_1("Invalid event type");
    }

    switch (input.method()) {
        case TRANSPORTMETHOD_OT: {
            CHECK_IDENTIFIER(transport);
        } break;
        case TRANSPORTMETHOD_NONE:
        case TRANSPORTMETHOD_OOB: {
            CHECK_EXCLUDED(transport);
        } break;
        case TRANSPORTMETHOD_ERROR:
        default: {
            FAIL_1("Invalid transport method");
        }
    }

    try {
        const bool valid =
            (1 == PaymentEventAllowedTransportMethod()
                      .at({input.version(), input.type()})
                      .count(input.method()));

        if (false == valid) {
            FAIL_1("Transport method not allowed for this version");
        }
    } catch (const std::out_of_range&) {
        FAIL_1("Invalid event type");
    }

    switch (input.type()) {
        case protobuf::PAYMENTEVENTTYPE_CREATE:
        case protobuf::PAYMENTEVENTTYPE_CONVEY:
        case protobuf::PAYMENTEVENTTYPE_ACCEPT: {
            OPTIONAL_IDENTIFIER(nym);
        } break;
        case protobuf::PAYMENTEVENTTYPE_CANCEL:
        case protobuf::PAYMENTEVENTTYPE_COMPLETE: {
            CHECK_EXCLUDED(nym);
        } break;
        case protobuf::PAYMENTEVENTTYPE_ABORT:
        case protobuf::PAYMENTEVENTTYPE_ACKNOWLEDGE:
        case protobuf::PAYMENTEVENTTYPE_EXPIRE:
        case protobuf::PAYMENTEVENTTYPE_REJECT:
        case protobuf::PAYMENTEVENTTYPE_ERROR:
        default: {
            FAIL_1("Invalid event type");
        }
    }

    auto& counter = events[input.type()];
    ++counter;

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
