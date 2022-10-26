// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageBip47AddressIndex.hpp"  // IWYU pragma: associated

#include <StorageBip47AddressIndex.pb.h>

#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    CHECK_IDENTIFIER(address);
    CHECK_IDENTIFIER(channelid);

    return true;
}

auto CheckProto_2(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const StorageBip47AddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
