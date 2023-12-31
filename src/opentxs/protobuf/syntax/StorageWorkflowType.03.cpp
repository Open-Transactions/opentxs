// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageWorkflowType.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PaymentWorkflowEnums.pb.h>
#include <opentxs/protobuf/StorageWorkflowType.pb.h>
#include <cstdint>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_3(const StorageWorkflowType& input, const Log& log) -> bool
{
    CHECK_IDENTIFIER(workflow);

    switch (input.type()) {
        case PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE: {
            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CONVEYED:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_COMPLETED:
                case PAYMENTWORKFLOWSTATE_EXPIRED: {
                } break;
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2(
                        "invalid outgoing cheque state",
                        static_cast<std::uint32_t>(input.state()));
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE: {
            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_CONVEYED:
                case PAYMENTWORKFLOWSTATE_COMPLETED:
                case PAYMENTWORKFLOWSTATE_EXPIRED: {
                } break;
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2(
                        "invalid incoming cheque state",
                        static_cast<std::uint32_t>(input.state()));
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE: {
            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CONVEYED:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_COMPLETED:
                case PAYMENTWORKFLOWSTATE_EXPIRED: {
                } break;
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2(
                        "invalid outgoing invoice state",
                        static_cast<std::uint32_t>(input.state()));
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_CONVEYED:
                case PAYMENTWORKFLOWSTATE_COMPLETED:
                case PAYMENTWORKFLOWSTATE_EXPIRED: {
                } break;
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2(
                        "invalid incoming invoice state",
                        static_cast<std::uint32_t>(input.state()));
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER: {
            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_COMPLETED:
                case PAYMENTWORKFLOWSTATE_ABORTED: {
                } break;
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CONVEYED:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_EXPIRED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2(
                        "invalid outgoing transfer state",
                        static_cast<std::uint32_t>(input.state()));
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER: {
            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_CONVEYED:
                case PAYMENTWORKFLOWSTATE_COMPLETED: {
                } break;
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_EXPIRED:
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2(
                        "invalid incoming transfer state",
                        static_cast<std::uint32_t>(input.state()));
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_CONVEYED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_COMPLETED:
                case PAYMENTWORKFLOWSTATE_ABORTED: {
                } break;
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_EXPIRED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2(
                        "invalid internal transfer state",
                        static_cast<std::uint32_t>(input.state()));
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_OUTGOINGCASH: {
            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CONVEYED:
                case PAYMENTWORKFLOWSTATE_EXPIRED: {
                } break;
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_COMPLETED:
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2(
                        "invalid outgoing cash state",
                        static_cast<std::uint32_t>(input.state()));
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_INCOMINGCASH: {
            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_CONVEYED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_EXPIRED:
                case PAYMENTWORKFLOWSTATE_REJECTED: {
                } break;
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_COMPLETED:
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2(
                        "invalid incoming cash state",
                        static_cast<std::uint32_t>(input.state()));
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            FAIL_2("invalid type", static_cast<std::uint32_t>(input.type()));
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
