// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BasketParams.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BasketParams.pb.h>
#include <utility>

#include "opentxs/protobuf/syntax/BasketItem.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContracts.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BasketParams& input, const Log& log) -> bool
{
    if (!input.has_weight()) { FAIL_1("missing weight"); }

    auto itemMap = BasketItemMap{};

    CHECK_SUBOBJECTS_VA(item, BasketParamsAllowedBasketItem(), itemMap);

    for (auto& subcurrency : itemMap) {
        if (subcurrency.second > 1) { FAIL_1("duplicate basket"); }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
