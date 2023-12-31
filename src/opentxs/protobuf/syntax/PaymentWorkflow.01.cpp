// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/PaymentWorkflow.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PaymentWorkflow.pb.h>
#include <opentxs/protobuf/PaymentWorkflowEnums.pb.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/InstrumentRevision.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/PaymentEvent.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyWorkflows.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

#define BAD_EVENTS(a, b)                                                       \
    FAIL_6(                                                                    \
        a,                                                                     \
        b,                                                                     \
        " Type: ",                                                             \
        std::to_string(static_cast<std::uint32_t>(input.type())),              \
        " State: ",                                                            \
        std::to_string(static_cast<std::uint32_t>(input.state())))

namespace opentxs::protobuf::inline syntax
{
auto version_1(const PaymentWorkflow& input, const Log& log) -> bool
{
    CHECK_IDENTIFIER(id);

    try {
        const bool valid =
            (1 == PaymentWorkflowAllowedState()
                      .at({input.version(), input.type()})
                      .count(input.state()));

        if (false == valid) { FAIL_2("Invalid state", __LINE__); }
    } catch (const std::out_of_range&) {
        FAIL_2("Invalid type", __LINE__);
    }

    switch (input.type()) {
        case PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            if (1 != input.source().size()) {
                FAIL_2(
                    "Incorrect number of source objects",
                    input.source().size());
            }

            if (1 != input.party().size()) {
                FAIL_2("Incorrect number of parties", input.party().size());
            }
        } break;
        case PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGCASH:
        case PAYMENTWORKFLOWTYPE_OUTGOINGCASH:
        case PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", __LINE__);
        }
    }

    CHECK_SUBOBJECTS(source, PaymentWorkflowAllowedInstrumentRevision());
    CHECK_IDENTIFIERS(party);

    auto events = UnallocatedMap<PaymentEventType, std::size_t>{};

    OPTIONAL_SUBOBJECTS_VA(
        event,
        PaymentWorkflowAllowedPaymentEvent(),
        input.version(),
        input.type(),
        events);
    CHECK_IDENTIFIERS(unit);
    CHECK_IDENTIFIERS(account);

    const auto& createEvents = events[PAYMENTEVENTTYPE_CREATE];
    const auto& conveyEvents = events[PAYMENTEVENTTYPE_CONVEY];
    const auto& cancelEvents = events[PAYMENTEVENTTYPE_CANCEL];
    const auto& acceptEvents = events[PAYMENTEVENTTYPE_ACCEPT];
    const auto& completeEvents = events[PAYMENTEVENTTYPE_COMPLETE];
    const auto accounts = input.account().size();

    switch (input.type()) {
        case PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE: {
            CHECK_IDENTIFIER(notary);

            if (1 != accounts) { FAIL_2("Wrong number of accounts", accounts); }

            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_UNSENT: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents);
                    }

                    // TODO: convey events are allowed only if they are
                    //       all failed

                    if (0 < cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents);
                    }

                    if (0 < acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents);
                    }

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents);
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_CONVEYED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents);
                    }

                    if (0 == conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents);
                    }

                    // TODO: cancel events are allowed only if they are
                    //       all failed

                    // TODO: accept events are allowed only if they are
                    //       all failed

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents);
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_CANCELLED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents);
                    }

                    // Any number of convey events are allowed

                    if (0 == cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents);
                    }

                    if (0 < acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents);
                    }

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents);
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_ACCEPTED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents);
                    }

                    if (0 == conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents);
                    }

                    // TODO: cancel events are allowed only if they are
                    //       all failed

                    if (0 == acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents);
                    }

                    // TODO: complete events are allowed only if they are
                    //       all failed
                } break;
                case PAYMENTWORKFLOWSTATE_COMPLETED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents);
                    }

                    if (0 == conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents);
                    }

                    // TODO: cancel events are allowed only if they are
                    //       all failed

                    if (0 == acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents);
                    }

                    if (0 == completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents);
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_EXPIRED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents);
                    }

                    // Any number of convey events are allowed

                    // TODO: cancel events are allowed only if they are
                    //       all failed

                    if (0 < acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents);
                    }

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents);
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2("Invalid state", __LINE__);
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            OPTIONAL_IDENTIFIER(notary);

            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_CONVEYED: {
                    if (0 != accounts) {
                        BAD_EVENTS("Wrong number of accounts", accounts);
                    }

                    if (0 < createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents);
                    }

                    if (1 != conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents);
                    }

                    if (0 < cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents);
                    }

                    // TODO: accept events are allowed only if they are
                    //       all failed

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents);
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_COMPLETED: {
                    CHECK_IDENTIFIER(notary);

                    if (1 != accounts) {
                        BAD_EVENTS("Wrong number of accounts", accounts);
                    }

                    if (0 < createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents);
                    }

                    if (1 != conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents);
                    }

                    if (0 < cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents);
                    }

                    if (0 == acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents);
                    }

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents);
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_EXPIRED: {
                    if (1 < accounts) {
                        BAD_EVENTS("Wrong number of accounts", accounts);
                    }

                    if (0 < createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents);
                    }

                    if (1 != conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents);
                    }

                    if (0 < cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents);
                    }

                    // TODO: accept events are allowed only if they are
                    //       all failed

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents);
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2("Invalid state", __LINE__);
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGCASH:
        case PAYMENTWORKFLOWTYPE_OUTGOINGCASH:
        case PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", __LINE__);
        }
    }

    switch (input.type()) {
        case PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            if (1 != input.unit().size()) { FAIL_1("Missing unit"); }
        } break;
        case PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGCASH:
        case PAYMENTWORKFLOWTYPE_OUTGOINGCASH:
        case PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", __LINE__);
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#undef BAD_EVENTS

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
