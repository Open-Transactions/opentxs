// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::otx::blind::CashType

#include "internal/otx/blind/Types.hpp"  // IWYU pragma: associated

#include <CashEnums.pb.h>
#include <robin_hood.h>

#include "opentxs/otx/blind/CashType.hpp"    // IWYU pragma: keep
#include "opentxs/otx/blind/PurseType.hpp"   // IWYU pragma: keep
#include "opentxs/otx/blind/TokenState.hpp"  // IWYU pragma: keep
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Container.hpp"

namespace opentxs::otx::blind
{
using CashTypeMap =
    robin_hood::unordered_flat_map<blind::CashType, proto::CashType>;
using CashTypeReverseMap =
    robin_hood::unordered_flat_map<proto::CashType, blind::CashType>;
using PurseTypeMap =
    robin_hood::unordered_flat_map<blind::PurseType, proto::PurseType>;
using PurseTypeReverseMap =
    robin_hood::unordered_flat_map<proto::PurseType, blind::PurseType>;
using TokenStateMap =
    robin_hood::unordered_flat_map<blind::TokenState, proto::TokenState>;
using TokenStateReverseMap =
    robin_hood::unordered_flat_map<proto::TokenState, blind::TokenState>;

auto cashtype_map() noexcept -> const CashTypeMap&;
auto pursetype_map() noexcept -> const PurseTypeMap&;
auto tokenstate_map() noexcept -> const TokenStateMap&;
}  // namespace opentxs::otx::blind

namespace opentxs::otx::blind
{
auto cashtype_map() noexcept -> const CashTypeMap&
{
    static const auto map = CashTypeMap{
        {CashType::Error, proto::CASHTYPE_ERROR},
        {CashType::Lucre, proto::CASHTYPE_LUCRE},
    };

    return map;
}

auto pursetype_map() noexcept -> const PurseTypeMap&
{
    static const auto map = PurseTypeMap{
        {PurseType::Error, proto::PURSETYPE_ERROR},
        {PurseType::Request, proto::PURSETYPE_REQUEST},
        {PurseType::Issue, proto::PURSETYPE_ISSUE},
        {PurseType::Normal, proto::PURSETYPE_NORMAL},
    };

    return map;
}

auto tokenstate_map() noexcept -> const TokenStateMap&
{
    static const auto map = TokenStateMap{
        {TokenState::Error, proto::TOKENSTATE_ERROR},
        {TokenState::Blinded, proto::TOKENSTATE_BLINDED},
        {TokenState::Signed, proto::TOKENSTATE_SIGNED},
        {TokenState::Ready, proto::TOKENSTATE_READY},
        {TokenState::Spent, proto::TOKENSTATE_SPENT},
        {TokenState::Expired, proto::TOKENSTATE_EXPIRED},
    };

    return map;
}
}  // namespace opentxs::otx::blind

namespace opentxs
{
auto print(otx::blind::CashType in) noexcept -> UnallocatedCString
{
    static const auto map =
        robin_hood::unordered_flat_map<otx::blind::CashType, const char*>{
            {otx::blind::CashType::Lucre, "lucre"},
        };

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
    static const auto map = reverse_arbitrary_map<
        otx::blind::CashType,
        proto::CashType,
        otx::blind::CashTypeReverseMap>(otx::blind::cashtype_map());
    try {
        return map.at(in);
    } catch (...) {
        return otx::blind::CashType::Error;
    }
}

auto translate(const proto::PurseType in) noexcept -> otx::blind::PurseType
{
    static const auto map = reverse_arbitrary_map<
        otx::blind::PurseType,
        proto::PurseType,
        otx::blind::PurseTypeReverseMap>(otx::blind::pursetype_map());
    try {
        return map.at(in);
    } catch (...) {
        return otx::blind::PurseType::Error;
    }
}

auto translate(const proto::TokenState in) noexcept -> otx::blind::TokenState
{
    static const auto map = reverse_arbitrary_map<
        otx::blind::TokenState,
        proto::TokenState,
        otx::blind::TokenStateReverseMap>(otx::blind::tokenstate_map());
    try {
        return map.at(in);
    } catch (...) {
        return otx::blind::TokenState::Error;
    }
}
}  // namespace opentxs
