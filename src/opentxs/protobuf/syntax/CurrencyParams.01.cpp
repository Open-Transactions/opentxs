// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/CurrencyParams.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/CurrencyParams.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/DisplayScale.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContracts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const CurrencyParams& input, const Log& log) -> bool
{
    if (!input.has_unit_of_account()) { FAIL_1("missing unit of account"); }

    if (!input.has_short_name()) { FAIL_1("missing short name"); }

    CHECK_SUBOBJECTS(scales, CurrencyParamsAllowedDisplayScales());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
