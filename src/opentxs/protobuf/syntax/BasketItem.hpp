// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class BasketItem;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
using BasketItemMap = UnallocatedMap<UnallocatedCString, std::uint64_t>;

auto version_1(const BasketItem& item, const Log& log, BasketItemMap& map)
    -> bool;
auto version_2(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_3(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_4(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_5(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_6(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_7(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_8(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_9(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_10(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_11(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_12(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_13(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_14(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_15(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_16(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_17(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_18(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_19(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
auto version_20(const BasketItem&, const Log& log, BasketItemMap&) -> bool;
}  // namespace opentxs::protobuf::inline syntax
