// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string>

#include "Proto.hpp"
#include "core/contract/UnitDefinition.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/contract/BasketContract.hpp"
#include "opentxs/core/contract/UnitType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/UnitDefinition.pb.h"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace display
{
class Definition;
}  // namespace display

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::contract::unit::implementation
{
class Basket final : public unit::Basket, public contract::implementation::Unit
{
public:
    auto BasketID() const -> OTIdentifier final;
    auto Currencies() const -> const Subcontracts& final
    {
        return subcontracts_;
    }
    auto Type() const -> contract::UnitType final
    {
        return contract::UnitType::Basket;
    }
    auto Weight() const -> std::uint64_t final { return weight_; }

    Basket(
        const api::Session& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& terms,
        const std::uint64_t weight,
        const core::UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement);
    Basket(
        const api::Session& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);

    ~Basket() final = default;

private:
    friend unit::Basket;

    Subcontracts subcontracts_;
    std::uint64_t weight_;

    auto BasketIDVersion(const Lock& lock) const -> proto::UnitDefinition;
    auto clone() const noexcept -> Basket* final { return new Basket(*this); }
    auto IDVersion(const Lock& lock) const -> proto::UnitDefinition final;

    Basket(const Basket&);
    Basket(Basket&&) = delete;
    auto operator=(const Basket&) -> Basket& = delete;
    auto operator=(Basket&&) -> Basket& = delete;
};
}  // namespace opentxs::contract::unit::implementation