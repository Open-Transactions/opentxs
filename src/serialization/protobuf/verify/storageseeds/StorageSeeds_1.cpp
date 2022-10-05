// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageSeeds.hpp"  // IWYU pragma: associated

#include <StorageItemHash.pb.h>
#include <StorageSeeds.pb.h>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageItemHash.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyStorage.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const StorageSeeds& input, const bool silent) -> bool
{
    if (!input.has_defaultseed()) { FAIL_1("missing default seed"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.defaultseed().size()) {
        FAIL_1("invalid default seed");
    }

    for (const auto& hash : input.seed()) {
        try {
            const bool valid = Check(
                hash,
                StorageSeedsAllowedStorageItemHash().at(input.version()).first,
                StorageSeedsAllowedStorageItemHash().at(input.version()).second,
                silent);

            if (!valid) { FAIL_1("invalid has"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed storage item hash version not defined for version",
                input.version());
        }
    }

    return true;
}

auto CheckProto_2(const StorageSeeds& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const StorageSeeds& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
