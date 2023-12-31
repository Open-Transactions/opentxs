// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/util/P0330.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class SymmetricKey;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
static constexpr auto crypto_salt_limit_ = 64_uz;

auto version_1(const SymmetricKey& key, const Log& log) -> bool;
auto version_2(const SymmetricKey&, const Log& log) -> bool;
auto version_3(const SymmetricKey&, const Log& log) -> bool;
auto version_4(const SymmetricKey&, const Log& log) -> bool;
auto version_5(const SymmetricKey&, const Log& log) -> bool;
auto version_6(const SymmetricKey&, const Log& log) -> bool;
auto version_7(const SymmetricKey&, const Log& log) -> bool;
auto version_8(const SymmetricKey&, const Log& log) -> bool;
auto version_9(const SymmetricKey&, const Log& log) -> bool;
auto version_10(const SymmetricKey&, const Log& log) -> bool;
auto version_11(const SymmetricKey&, const Log& log) -> bool;
auto version_12(const SymmetricKey&, const Log& log) -> bool;
auto version_13(const SymmetricKey&, const Log& log) -> bool;
auto version_14(const SymmetricKey&, const Log& log) -> bool;
auto version_15(const SymmetricKey&, const Log& log) -> bool;
auto version_16(const SymmetricKey&, const Log& log) -> bool;
auto version_17(const SymmetricKey&, const Log& log) -> bool;
auto version_18(const SymmetricKey&, const Log& log) -> bool;
auto version_19(const SymmetricKey&, const Log& log) -> bool;
auto version_20(const SymmetricKey&, const Log& log) -> bool;
}  // namespace opentxs::protobuf::inline syntax
