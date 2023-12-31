// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::CashType

#pragma once

#include <opentxs/protobuf/CashEnums.pb.h>
#include <cstdint>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class Token;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_2(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_3(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_4(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_5(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_6(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_7(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_8(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_9(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_10(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_11(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_12(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_13(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_14(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_15(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_16(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_17(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_18(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_19(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto version_20(
    const Token& input,
    const Log& log,
    const CashType expectedType,
    const UnallocatedSet<TokenState>& expectedState,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
}  // namespace opentxs::protobuf::inline syntax
