// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/PaymentCode.hpp"  // IWYU pragma: associated

#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/PaymentCode.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const PaymentCode& input, const bool silent) -> bool
{
    if (!input.has_key()) { FAIL_1("missing pubkey") }

    if (MIN_PLAUSIBLE_KEYSIZE > input.key().size()) { FAIL_1("invalid pubkey") }

    if (!input.has_chaincode()) { FAIL_1("missing chaincode") }

    if (MIN_PLAUSIBLE_KEYSIZE > input.chaincode().size()) {
        FAIL_1("invalid chaincode")
    }

    if (input.has_bitmessage()) {
        if (!input.has_bitmessageversion()) {
            FAIL_1("missing Bitmessage address version")
        }

        if (0xff < input.bitmessageversion()) {
            FAIL_1("invalid Bitmessage address version")
        }

        if (!input.has_bitmessagestream()) {
            FAIL_1("missing Bitmessage address stream")
        }

        if (0xff < input.bitmessagestream()) {
            FAIL_1("invalid Bitmessage address stream")
        }
    }

    return true;
}

auto CheckProto_2(const PaymentCode& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const PaymentCode& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_4(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const PaymentCode& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
