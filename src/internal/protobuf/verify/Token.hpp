// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <set>
#include <string>

#include "serialization/protobuf/CashEnums.pb.h"

namespace opentxs
{
namespace proto
{
class Token;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
auto CheckProto_1(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_2(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_3(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_4(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_5(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_6(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_7(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_8(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_9(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_10(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_11(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_12(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_13(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_14(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_15(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_16(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_17(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_18(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_19(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
auto CheckProto_20(
    const Token& input,
    const bool silent,
    const CashType expectedType,
    const std::pmr::set<TokenState>& expectedState,
    std::string& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool;
}  // namespace proto
}  // namespace opentxs
