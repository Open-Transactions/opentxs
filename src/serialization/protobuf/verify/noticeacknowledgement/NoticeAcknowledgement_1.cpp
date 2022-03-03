// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/NoticeAcknowledgement.hpp"  // IWYU pragma: associated

#include "serialization/protobuf/NoticeAcknowledgement.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const NoticeAcknowledgement& input, const bool silent) -> bool
{
    if (!input.has_ack()) { FAIL_1("missing ack/nack") }

    return true;
}

auto CheckProto_2(const NoticeAcknowledgement& input, const bool silent) -> bool
{

    return CheckProto_1(input, silent);
}

auto CheckProto_3(const NoticeAcknowledgement& input, const bool silent) -> bool
{

    return CheckProto_1(input, silent);
}

auto CheckProto_4(const NoticeAcknowledgement& input, const bool silent) -> bool
{

    return CheckProto_1(input, silent);
}

auto CheckProto_5(const NoticeAcknowledgement& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const NoticeAcknowledgement& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const NoticeAcknowledgement& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const NoticeAcknowledgement& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const NoticeAcknowledgement& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const NoticeAcknowledgement& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
