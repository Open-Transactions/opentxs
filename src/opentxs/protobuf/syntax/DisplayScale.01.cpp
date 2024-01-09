// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/DisplayScale.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/DisplayScale.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/ScaleRatio.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyContracts.hpp"

namespace opentxs::protobuf::inline syntax
{

auto version_1(const DisplayScale& input, const Log& log) -> bool
{
    if (!input.has_name()) { FAIL_1("missing name"); }

    OPTIONAL_SUBOBJECTS(ratios, DisplayScaleAllowedScaleRatios());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep