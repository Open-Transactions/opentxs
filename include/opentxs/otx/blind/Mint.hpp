// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <ctime>

#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class UnitDefinition;
}  // namespace identifier

namespace otx
{
namespace blind
{
namespace internal
{
class Mint;
}  // namespace internal

class Mint;
}  // namespace blind
}  // namespace otx

class Amount;
class Armored;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind
{
OPENTXS_EXPORT auto swap(Mint& lhs, Mint& rhs) noexcept -> void;

class OPENTXS_EXPORT Mint
{
public:
    class Imp;

    operator bool() const noexcept;

    auto AccountID() const -> OTIdentifier;
    auto Expired() const -> bool;
    auto GetDenomination(std::int32_t nIndex) const -> Amount;
    auto GetDenominationCount() const -> std::int32_t;
    auto GetExpiration() const -> Time;
    auto GetLargestDenomination(const Amount& lAmount) const -> Amount;
    auto GetPrivate(Armored& theArmor, const Amount& lDenomination) const
        -> bool;
    auto GetPublic(Armored& theArmor, const Amount& lDenomination) const
        -> bool;
    auto GetSeries() const -> std::int32_t;
    auto GetValidFrom() const -> Time;
    auto GetValidTo() const -> Time;
    auto InstrumentDefinitionID() const -> const identifier::UnitDefinition&;
    auto Internal() const noexcept -> const otx::blind::internal::Mint&;

    auto Internal() noexcept -> otx::blind::internal::Mint&;
    OPENTXS_NO_EXPORT auto Release() noexcept -> otx::blind::internal::Mint*;
    auto swap(Mint& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Mint(Imp* imp) noexcept;
    OPENTXS_NO_EXPORT Mint(const api::Session& api) noexcept;
    Mint(Mint&& rhs) noexcept;
    auto operator=(Mint&&) noexcept -> Mint&;

    virtual ~Mint();

private:
    Imp* imp_;

    Mint(const Mint&) = delete;
    auto operator=(const Mint&) -> Mint& = delete;
};
}  // namespace opentxs::otx::blind
