// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/BlindedSeriesList.hpp"  // IWYU pragma: associated

#include <BlindedSeriesList.pb.h>

#include "internal/serialization/protobuf/verify/StorageItemHash.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyStorage.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString& notary) -> bool
{
    CHECK_IDENTIFIER(notary);

    if (notary != input.notary()) {
        FAIL_4("Incorrect notary ", input.notary(), " expected ", notary);
    }

    CHECK_IDENTIFIER(unit);
    CHECK_SUBOBJECTS(series, BlindedSeriesListAllowedStorageItemHash());

    return true;
}

auto CheckProto_2(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const BlindedSeriesList& input,
    const bool silent,
    const UnallocatedCString&) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
