// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageWorkflowType.hpp"  // IWYU pragma: associated

#include <PaymentWorkflowEnums.pb.h>
#include <StorageWorkflowType.pb.h>
#include <cstdint>

#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_3(const StorageWorkflowType& input, const bool silent) -> bool
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

auto CheckProto_4(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const StorageWorkflowType& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
