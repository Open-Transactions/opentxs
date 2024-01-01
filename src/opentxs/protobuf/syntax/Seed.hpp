// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class Seed;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(const Seed& input, const Log& log) -> bool;
auto version_2(const Seed& input, const Log& log) -> bool;
auto version_3(const Seed& input, const Log& log) -> bool;
auto version_4(const Seed& input, const Log& log) -> bool;
auto version_5(const Seed& input, const Log& log) -> bool;
auto version_6(const Seed& input, const Log& log) -> bool;
auto version_7(const Seed& input, const Log& log) -> bool;
auto version_8(const Seed& input, const Log& log) -> bool;
auto version_9(const Seed& input, const Log& log) -> bool;
auto version_10(const Seed& input, const Log& log) -> bool;
auto version_11(const Seed& input, const Log& log) -> bool;
auto version_12(const Seed& input, const Log& log) -> bool;
auto version_13(const Seed& input, const Log& log) -> bool;
auto version_14(const Seed& input, const Log& log) -> bool;
auto version_15(const Seed& input, const Log& log) -> bool;
auto version_16(const Seed& input, const Log& log) -> bool;
auto version_17(const Seed& input, const Log& log) -> bool;
auto version_18(const Seed& input, const Log& log) -> bool;
auto version_19(const Seed& input, const Log& log) -> bool;
auto version_20(const Seed& input, const Log& log) -> bool;
}  // namespace opentxs::protobuf::inline syntax
