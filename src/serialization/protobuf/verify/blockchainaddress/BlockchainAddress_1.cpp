// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/BlockchainAddress.hpp"  // IWYU pragma: associated

#include <BlockchainAddress.pb.h>
#include <Enums.pb.h>

#include "internal/serialization/protobuf/verify/AsymmetricKey.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyBlockchain.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent, true, false);
}

auto CheckProto_1(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    if (MAX_LABEL_SIZE < input.label().size()) { FAIL_1("invalid label"); }

    OPTIONAL_IDENTIFIER(contact);
    CHECK_SUBOBJECT_VA(
        key,
        BlockchainAddressAllowedAsymmetricKey(),
        hd ? CREDTYPE_HD : CREDTYPE_LEGACY,
        pubkey ? KEYMODE_PUBLIC : KEYMODE_PRIVATE,
        KEYROLE_SIGN);

    return true;
}

auto CheckProto_2(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent, true, false);
}

auto CheckProto_2(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_3(input, silent, true, false);
}

auto CheckProto_3(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_4(input, silent, true, false);
}

auto CheckProto_4(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_5(input, silent, true, false);
}

auto CheckProto_5(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_6(input, silent, true, false);
}

auto CheckProto_6(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_7(input, silent, true, false);
}

auto CheckProto_7(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_8(input, silent, true, false);
}

auto CheckProto_8(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_9(input, silent, true, false);
}

auto CheckProto_9(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_10(input, silent, true, false);
}

auto CheckProto_10(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_11(input, silent, true, false);
}

auto CheckProto_11(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_12(input, silent, true, false);
}

auto CheckProto_12(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_13(input, silent, true, false);
}

auto CheckProto_13(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_14(input, silent, true, false);
}

auto CheckProto_14(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_15(input, silent, true, false);
}

auto CheckProto_15(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_16(input, silent, true, false);
}

auto CheckProto_16(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_17(input, silent, true, false);
}

auto CheckProto_17(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_18(input, silent, true, false);
}

auto CheckProto_18(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_19(input, silent, true, false);
}

auto CheckProto_19(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const BlockchainAddress& input, const bool silent) -> bool
{
    return CheckProto_20(input, silent, true, false);
}

auto CheckProto_20(
    const BlockchainAddress& input,
    const bool silent,
    const bool pubkey,
    const bool hd) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
