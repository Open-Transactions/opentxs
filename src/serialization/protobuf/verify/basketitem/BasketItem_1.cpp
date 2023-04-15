// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/BasketItem.hpp"  // IWYU pragma: associated

#include <BasketItem.pb.h>

#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(
    const BasketItem& input,
    const bool silent,
    BasketItemMap& map) -> bool
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

auto CheckProto_2(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const BasketItem& input, const bool silent, BasketItemMap&)
    -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
