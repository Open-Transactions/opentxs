// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/HDAccount.hpp"  // IWYU pragma: associated

#include <HDAccount.pb.h>

#include "internal/serialization/protobuf/verify/BlockchainAddress.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/BlockchainDeterministicAccountData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/BlockchainHDAccountData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyBlockchain.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const HDAccount& input, const bool silent) -> bool
{
    OPTIONAL_SUBOBJECT(
        deterministic, HDAccountAllowedBlockchainDeterministicAccountData());
    CHECK_SUBOBJECTS(internaladdress, HDAccountAllowedBlockchainAddress());
    CHECK_SUBOBJECTS(externaladdress, HDAccountAllowedBlockchainAddress());
    OPTIONAL_SUBOBJECT(hd, HDAccountAllowedBlockchainHDAccountData());

    return true;
}

auto CheckProto_2(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const HDAccount& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
