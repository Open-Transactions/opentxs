// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageNotary.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StorageNotary.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/BlindedSeriesList.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyStorage.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const StorageNotary& input, const Log& log) -> bool
{
    CHECK_IDENTIFIER(id);
    OPTIONAL_SUBOBJECTS_VA(
        series, StorageNotaryAllowedBlindedSeriesList(), input.id());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
