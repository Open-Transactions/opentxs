// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/blind/Types.hpp"  // IWYU pragma: associated

#include <CashEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>

#include "opentxs/otx/blind/CashType.hpp"    // IWYU pragma: keep
#include "opentxs/otx/blind/PurseType.hpp"   // IWYU pragma: keep
#include "opentxs/otx/blind/TokenState.hpp"  // IWYU pragma: keep
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::otx::blind
{
using CashTypeMap = frozen::unordered_map<blind::CashType, proto::CashType, 2>;
using CashTypeReverseMap =
    frozen::unordered_map<proto::CashType, blind::CashType, 2>;
using PurseTypeMap =
    frozen::unordered_map<blind::PurseType, proto::PurseType, 4>;
using PurseTypeReverseMap =
    frozen::unordered_map<proto::PurseType, blind::PurseType, 4>;
using TokenStateMap =
    frozen::unordered_map<blind::TokenState, proto::TokenState, 6>;
using TokenStateReverseMap =
    frozen::unordered_map<proto::TokenState, blind::TokenState, 6>;

auto cashtype_map() noexcept -> const CashTypeMap&;
auto pursetype_map() noexcept -> const PurseTypeMap&;
auto tokenstate_map() noexcept -> const TokenStateMap&;
}  // namespace opentxs::otx::blind

namespace opentxs::otx::blind
{
auto cashtype_map() noexcept -> const CashTypeMap&
{
    using enum CashType;
    using enum proto::CashType;
    static constexpr auto map = CashTypeMap{
        {Error, CASHTYPE_ERROR},
        {Lucre, CASHTYPE_LUCRE},
    };

    return map;
}

auto pursetype_map() noexcept -> const PurseTypeMap&
{
    using enum PurseType;
    using enum proto::PurseType;
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
    using enum proto::TokenState;
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
auto print(otx::blind::CashType in) noexcept -> UnallocatedCString
{
    static constexpr auto map =
        frozen::make_unordered_map<otx::blind::CashType, const char*>({
            {otx::blind::CashType::Lucre, "lucre"},
        });

    try {

        return map.at(in);
    } catch (...) {

        return "invalid";
    }
}

auto supported_otx_token_types() noexcept
    -> UnallocatedSet<otx::blind::CashType>
{
    return {otx::blind::CashType::Lucre};
}

auto translate(const otx::blind::CashType in) noexcept -> proto::CashType
{
    try {
        return otx::blind::cashtype_map().at(in);
    } catch (...) {
        return proto::CASHTYPE_ERROR;
    }
}

auto translate(const otx::blind::PurseType in) noexcept -> proto::PurseType
{
    try {
        return otx::blind::pursetype_map().at(in);
    } catch (...) {
        return proto::PURSETYPE_ERROR;
    }
}

auto translate(const otx::blind::TokenState in) noexcept -> proto::TokenState
{
    try {
        return otx::blind::tokenstate_map().at(in);
    } catch (...) {
        return proto::TOKENSTATE_ERROR;
    }
}

auto translate(const proto::CashType in) noexcept -> otx::blind::CashType
{
    static const auto map =
        frozen::invert_unordered_map(otx::blind::cashtype_map());
    try {
        return map.at(in);
    } catch (...) {
        return otx::blind::CashType::Error;
    }
}

auto translate(const proto::PurseType in) noexcept -> otx::blind::PurseType
{
    static const auto map =
        frozen::invert_unordered_map(otx::blind::pursetype_map());
    try {
        return map.at(in);
    } catch (...) {
        return otx::blind::PurseType::Error;
    }
}

auto translate(const proto::TokenState in) noexcept -> otx::blind::TokenState
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
