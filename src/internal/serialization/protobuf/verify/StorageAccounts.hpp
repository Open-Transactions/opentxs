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
class StorageAccounts;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::proto
{
auto CheckProto_1(const StorageAccounts& input, const bool silent) -> bool;
auto CheckProto_2(const StorageAccounts&, const bool) -> bool;
auto CheckProto_3(const StorageAccounts&, const bool) -> bool;
auto CheckProto_4(const StorageAccounts&, const bool) -> bool;
auto CheckProto_5(const StorageAccounts&, const bool) -> bool;
auto CheckProto_6(const StorageAccounts&, const bool) -> bool;
auto CheckProto_7(const StorageAccounts&, const bool) -> bool;
auto CheckProto_8(const StorageAccounts&, const bool) -> bool;
auto CheckProto_9(const StorageAccounts&, const bool) -> bool;
auto CheckProto_10(const StorageAccounts&, const bool) -> bool;
auto CheckProto_11(const StorageAccounts&, const bool) -> bool;
auto CheckProto_12(const StorageAccounts&, const bool) -> bool;
auto CheckProto_13(const StorageAccounts&, const bool) -> bool;
auto CheckProto_14(const StorageAccounts&, const bool) -> bool;
auto CheckProto_15(const StorageAccounts&, const bool) -> bool;
auto CheckProto_16(const StorageAccounts&, const bool) -> bool;
auto CheckProto_17(const StorageAccounts&, const bool) -> bool;
auto CheckProto_18(const StorageAccounts&, const bool) -> bool;
auto CheckProto_19(const StorageAccounts&, const bool) -> bool;
auto CheckProto_20(const StorageAccounts&, const bool) -> bool;
}  // namespace opentxs::proto
