// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/SendPayment.hpp"  // IWYU pragma: associated

#include "serialization/protobuf/RPCEnums.pb.h"
#include "serialization/protobuf/SendPayment.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const SendPayment& input, const bool silent) -> bool
{
    switch (input.type()) {
        case RPCPAYMENTTYPE_BLOCKCHAIN: {
            OPTIONAL_IDENTIFIER(contact);
        } break;
        case RPCPAYMENTTYPE_CHEQUE:
        case RPCPAYMENTTYPE_TRANSFER:
        case RPCPAYMENTTYPE_VOUCHER:
        case RPCPAYMENTTYPE_INVOICE:
        case RPCPAYMENTTYPE_BLINDED: {
            CHECK_IDENTIFIER(contact);
        } break;
        case RPCPAYMENTTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    CHECK_IDENTIFIER(sourceaccount);
    OPTIONAL_NAME(memo);

    switch (input.type()) {
        case RPCPAYMENTTYPE_TRANSFER:
        case RPCPAYMENTTYPE_BLOCKCHAIN: {
            CHECK_IDENTIFIER(destinationaccount);
        } break;
        case RPCPAYMENTTYPE_CHEQUE:
        case RPCPAYMENTTYPE_VOUCHER:
        case RPCPAYMENTTYPE_INVOICE:
        case RPCPAYMENTTYPE_BLINDED: {
            CHECK_EXCLUDED(destinationaccount);
        } break;
        case RPCPAYMENTTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    return true;
}

auto CheckProto_2(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const SendPayment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
