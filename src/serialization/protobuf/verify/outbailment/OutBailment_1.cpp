// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/OutBailment.hpp"  // IWYU pragma: associated

#include <OutBailment.pb.h>

#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const OutBailment& input, const bool silent) -> bool
{
    CHECK_SUBOBJECT(unitid, OutbailmentAllowedIdentifier());
    CHECK_SUBOBJECT(serverid, OutbailmentAllowedIdentifier());
    CHECK_EXISTS(amount);
    CHECK_EXISTS(instructions);
    CHECK_NONE(payment);

    return true;
}

auto CheckProto_2(const OutBailment& input, const bool silent) -> bool
{

    return CheckProto_1(input, silent);
}

auto CheckProto_3(const OutBailment& input, const bool silent) -> bool
{

    return CheckProto_1(input, silent);
}

auto CheckProto_4(const OutBailment& input, const bool silent) -> bool
{

    return CheckProto_1(input, silent);
}

auto CheckProto_5(const OutBailment& input, const bool silent) -> bool
{
    CHECK_SUBOBJECT(unitid, OutbailmentAllowedIdentifier());
    CHECK_SUBOBJECT(serverid, OutbailmentAllowedIdentifier());
    CHECK_EXISTS(amount);
    CHECK_EXISTS(instructions);

    return true;
}

auto CheckProto_6(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
