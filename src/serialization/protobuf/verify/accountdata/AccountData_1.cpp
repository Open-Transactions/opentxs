// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/AccountData.hpp"  // IWYU pragma: associated

#include <AccountData.pb.h>
#include <RPCEnums.pb.h>

#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const AccountData& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(id);
    OPTIONAL_NAME(label);
    CHECK_IDENTIFIER(unit);
    CHECK_IDENTIFIER(owner);
    CHECK_IDENTIFIER(issuer);
    CHECK_EXCLUDED(type);

    return true;
}

auto CheckProto_2(const AccountData& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(id);
    OPTIONAL_NAME(label);
    CHECK_IDENTIFIER(unit);
    CHECK_IDENTIFIER(owner);
    CHECK_IDENTIFIER(issuer);

    switch (input.type()) {
        case ACCOUNTTYPE_NORMAL:
        case ACCOUNTTYPE_ISSUER:
        case ACCOUNTTYPE_BLOCKCHAIN: {
        } break;
        case ACCOUNTTYPE_ERROR:
        default: {
            FAIL_1("Invalid type");
        }
    }

    return true;
}

auto CheckProto_3(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const AccountData& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
