// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/PendingBailment.hpp"  // IWYU pragma: associated

#include "serialization/protobuf/PendingBailment.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const PendingBailment& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(unitid)
    CHECK_IDENTIFIER(serverid)
    CHECK_IDENTIFIER(txid)
    CHECK_EXCLUDED(requestid)
    CHECK_EXCLUDED(amount)
    return true;
}

auto CheckProto_2(const PendingBailment& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const PendingBailment& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_4(const PendingBailment& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_5(const PendingBailment& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(unitid)
    CHECK_IDENTIFIER(serverid)
    CHECK_IDENTIFIER(txid)
    OPTIONAL_IDENTIFIER(requestid)
    CHECK_EXCLUDED(amount)
    return true;
}

auto CheckProto_6(const PendingBailment& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(unitid)
    CHECK_IDENTIFIER(serverid)
    CHECK_IDENTIFIER(txid)
    OPTIONAL_IDENTIFIER(requestid)
    return true;
}

auto CheckProto_7(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const PendingBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
