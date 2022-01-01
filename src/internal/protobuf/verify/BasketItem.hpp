// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <string>

namespace opentxs
{
namespace proto
{
class BasketItem;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
using BasketItemMap = std::pmr::map<std::string, std::uint64_t>;

auto CheckProto_1(const BasketItem& item, const bool silent, BasketItemMap& map)
    -> bool;
auto CheckProto_2(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_3(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_4(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_5(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_6(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_7(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_8(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_9(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_10(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_11(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_12(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_13(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_14(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_15(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_16(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_17(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_18(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_19(const BasketItem&, const bool, BasketItemMap&) -> bool;
auto CheckProto_20(const BasketItem&, const bool, BasketItemMap&) -> bool;
}  // namespace proto
}  // namespace opentxs
