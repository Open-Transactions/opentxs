// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/MoveFunds.hpp"  // IWYU pragma: associated

#include <MoveFunds.pb.h>
#include <RPCEnums.pb.h>

#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const MoveFunds& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(sourceaccount);
    OPTIONAL_NAME(memo);

    switch (input.type()) {
        case RPCPAYMENTTYPE_TRANSFER: {
            CHECK_IDENTIFIER(destinationaccount);
        } break;
        case RPCPAYMENTTYPE_CHEQUE:
        case RPCPAYMENTTYPE_VOUCHER:
        case RPCPAYMENTTYPE_INVOICE:
        case RPCPAYMENTTYPE_BLINDED:
        case RPCPAYMENTTYPE_ERROR:
        case RPCPAYMENTTYPE_BLOCKCHAIN: {
        } break;
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    return true;
}

auto CheckProto_2(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const MoveFunds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
