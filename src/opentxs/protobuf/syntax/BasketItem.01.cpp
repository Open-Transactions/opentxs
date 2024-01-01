// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BasketItem.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BasketItem.pb.h>

#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BasketItem& input, const Log& log, BasketItemMap& map)
    -> bool
{
    if (!input.has_weight()) { FAIL_1("missing weight"); }

    if (!input.has_unit()) { FAIL_1("missing unit"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.unit().size()) {
        FAIL_2("invalid unit", input.unit());
    }

    map[input.unit()] += 1;

    if (!input.has_account()) { FAIL_1("missing account"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.account().size()) {
        FAIL_2("invalid account", input.account());
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
