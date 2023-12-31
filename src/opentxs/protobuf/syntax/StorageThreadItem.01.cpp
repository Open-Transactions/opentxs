// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageThreadItem.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StorageThreadItem.pb.h>

#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const StorageThreadItem& input, const Log& log) -> bool
{
    if (!input.has_id()) { FAIL_1("missing id"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) { FAIL_1("invalid id"); }

    if (0 == input.box()) { FAIL_1("invalid box"); }

    const bool hasAccount = (0 < input.account().size());
    const bool invalidAccount =
        (MIN_PLAUSIBLE_IDENTIFIER > input.account().size());

    if (hasAccount && invalidAccount) { FAIL_1("invalid account"); }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
