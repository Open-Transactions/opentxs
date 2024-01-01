// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlindedSeriesList.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlindedSeriesList.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/StorageItemHash.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyStorage.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const BlindedSeriesList& input,
    const Log& log,
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
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
