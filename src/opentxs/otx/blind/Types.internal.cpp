// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/otx/blind/Types.internal.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <opentxs/protobuf/CashEnums.pb.h>
#include <functional>

#include "opentxs/otx/blind/CashType.hpp"    // IWYU pragma: keep
#include "opentxs/otx/blind/PurseType.hpp"   // IWYU pragma: keep
#include "opentxs/otx/blind/TokenState.hpp"  // IWYU pragma: keep
#include "opentxs/otx/blind/Types.hpp"

namespace opentxs::otx::blind
{
using CashTypeMap =
    frozen::unordered_map<blind::CashType, protobuf::CashType, 2>;
using CashTypeReverseMap =
    frozen::unordered_map<protobuf::CashType, blind::CashType, 2>;
using PurseTypeMap =
    frozen::unordered_map<blind::PurseType, protobuf::PurseType, 4>;
using PurseTypeReverseMap =
    frozen::unordered_map<protobuf::PurseType, blind::PurseType, 4>;
using TokenStateMap =
    frozen::unordered_map<blind::TokenState, protobuf::TokenState, 6>;
using TokenStateReverseMap =
    frozen::unordered_map<protobuf::TokenState, blind::TokenState, 6>;

auto cashtype_map() noexcept -> const CashTypeMap&;
auto pursetype_map() noexcept -> const PurseTypeMap&;
auto tokenstate_map() noexcept -> const TokenStateMap&;
}  // namespace opentxs::otx::blind

namespace opentxs::otx::blind
{
auto cashtype_map() noexcept -> const CashTypeMap&
{
    using enum CashType;
    using enum protobuf::CashType;
    static constexpr auto map = CashTypeMap{
        {Error, CASHTYPE_ERROR},
        {Lucre, CASHTYPE_LUCRE},
    };

    return map;
}

auto pursetype_map() noexcept -> const PurseTypeMap&
{
    using enum PurseType;
    using enum protobuf::PurseType;
    static constexpr auto map = PurseTypeMap{
        {Error, PURSETYPE_ERROR},
        {Request, PURSETYPE_REQUEST},
        {Issue, PURSETYPE_ISSUE},
        {Normal, PURSETYPE_NORMAL},
    };

    return map;
}

auto tokenstate_map() noexcept -> const TokenStateMap&
{
    using enum TokenState;
    using enum protobuf::TokenState;
    static constexpr auto map = TokenStateMap{
        {Error, TOKENSTATE_ERROR},
        {Blinded, TOKENSTATE_BLINDED},
        {Signed, TOKENSTATE_SIGNED},
        {Ready, TOKENSTATE_READY},
        {Spent, TOKENSTATE_SPENT},
        {Expired, TOKENSTATE_EXPIRED},
    };

    return map;
}
}  // namespace opentxs::otx::blind

namespace opentxs
{
auto translate(const otx::blind::CashType in) noexcept -> protobuf::CashType
{
    try {
        return otx::blind::cashtype_map().at(in);
    } catch (...) {
        return protobuf::CASHTYPE_ERROR;
    }
}

auto translate(const otx::blind::PurseType in) noexcept -> protobuf::PurseType
{
    try {
        return otx::blind::pursetype_map().at(in);
    } catch (...) {
        return protobuf::PURSETYPE_ERROR;
    }
}

auto translate(const otx::blind::TokenState in) noexcept -> protobuf::TokenState
{
    try {
        return otx::blind::tokenstate_map().at(in);
    } catch (...) {
        return protobuf::TOKENSTATE_ERROR;
    }
}

auto translate(const protobuf::CashType in) noexcept -> otx::blind::CashType
{
    static const auto map =
        frozen::invert_unordered_map(otx::blind::cashtype_map());
    try {
        return map.at(in);
    } catch (...) {
        return otx::blind::CashType::Error;
    }
}

auto translate(const protobuf::PurseType in) noexcept -> otx::blind::PurseType
{
    static const auto map =
        frozen::invert_unordered_map(otx::blind::pursetype_map());
    try {
        return map.at(in);
    } catch (...) {
        return otx::blind::PurseType::Error;
    }
}

auto translate(const protobuf::TokenState in) noexcept -> otx::blind::TokenState
{
    static const auto map =
        frozen::invert_unordered_map(otx::blind::tokenstate_map());
    try {
        return map.at(in);
    } catch (...) {
        return otx::blind::TokenState::Error;
    }
}
}  // namespace opentxs
