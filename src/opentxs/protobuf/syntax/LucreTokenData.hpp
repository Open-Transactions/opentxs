// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::TokenState

#pragma once

#include <opentxs/protobuf/CashEnums.pb.h>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class LucreTokenData;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_2(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_3(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_4(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_5(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_6(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_7(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_8(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_9(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_10(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_11(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_12(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_13(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_14(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_15(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_16(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_17(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_18(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_19(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
auto version_20(
    const LucreTokenData& input,
    const Log& log,
    const TokenState state) -> bool;
}  // namespace opentxs::protobuf::inline syntax
