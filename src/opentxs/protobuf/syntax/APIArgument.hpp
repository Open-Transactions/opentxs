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
class APIArgument;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(const APIArgument& input, const Log& log) -> bool;
auto version_2(const APIArgument& input, const Log& log) -> bool;
auto version_3(const APIArgument& input, const Log& log) -> bool;
auto version_4(const APIArgument& input, const Log& log) -> bool;
auto version_5(const APIArgument& input, const Log& log) -> bool;
auto version_6(const APIArgument& input, const Log& log) -> bool;
auto version_7(const APIArgument& input, const Log& log) -> bool;
auto version_8(const APIArgument& input, const Log& log) -> bool;
auto version_9(const APIArgument& input, const Log& log) -> bool;
auto version_10(const APIArgument& input, const Log& log) -> bool;
auto version_11(const APIArgument& input, const Log& log) -> bool;
auto version_12(const APIArgument& input, const Log& log) -> bool;
auto version_13(const APIArgument& input, const Log& log) -> bool;
auto version_14(const APIArgument& input, const Log& log) -> bool;
auto version_15(const APIArgument& input, const Log& log) -> bool;
auto version_16(const APIArgument& input, const Log& log) -> bool;
auto version_17(const APIArgument& input, const Log& log) -> bool;
auto version_18(const APIArgument& input, const Log& log) -> bool;
auto version_19(const APIArgument& input, const Log& log) -> bool;
auto version_20(const APIArgument& input, const Log& log) -> bool;
}  // namespace opentxs::protobuf::inline syntax
