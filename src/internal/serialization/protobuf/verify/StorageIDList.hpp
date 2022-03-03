// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class StorageIDList;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::proto
{
auto CheckProto_1(const StorageIDList& list, const bool silent) -> bool;
auto CheckProto_2(const StorageIDList&, const bool) -> bool;
auto CheckProto_3(const StorageIDList&, const bool) -> bool;
auto CheckProto_4(const StorageIDList&, const bool) -> bool;
auto CheckProto_5(const StorageIDList&, const bool) -> bool;
auto CheckProto_6(const StorageIDList&, const bool) -> bool;
auto CheckProto_7(const StorageIDList&, const bool) -> bool;
auto CheckProto_8(const StorageIDList&, const bool) -> bool;
auto CheckProto_9(const StorageIDList&, const bool) -> bool;
auto CheckProto_10(const StorageIDList&, const bool) -> bool;
auto CheckProto_11(const StorageIDList&, const bool) -> bool;
auto CheckProto_12(const StorageIDList&, const bool) -> bool;
auto CheckProto_13(const StorageIDList&, const bool) -> bool;
auto CheckProto_14(const StorageIDList&, const bool) -> bool;
auto CheckProto_15(const StorageIDList&, const bool) -> bool;
auto CheckProto_16(const StorageIDList&, const bool) -> bool;
auto CheckProto_17(const StorageIDList&, const bool) -> bool;
auto CheckProto_18(const StorageIDList&, const bool) -> bool;
auto CheckProto_19(const StorageIDList&, const bool) -> bool;
auto CheckProto_20(const StorageIDList&, const bool) -> bool;
}  // namespace opentxs::proto
