// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageItemHash.hpp"  // IWYU pragma: associated

#include <StorageItemHash.pb.h>

#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyStorage.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_2(const StorageItemHash& input, const bool silent) -> bool
{
    OPTIONAL_IDENTIFIER(item_id_base58);
    CHECK_IDENTIFIER(hash);
    CHECK_EXISTS(type);
    OPTIONAL_SUBOBJECT(id, StorageItemHashAllowedIdentifier());

    return true;
}

auto CheckProto_3(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_4(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_5(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_6(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_7(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_8(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_9(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_10(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
