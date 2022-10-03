// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace contract
{
namespace unit
{
class Basket;
}  // namespace unit
}  // namespace contract

using OTBasketContract = SharedPimpl<contract::unit::Basket>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::unit
{
class OPENTXS_EXPORT Basket : virtual public contract::Unit
{
public:
    // account number, weight
    using Subcontract = std::pair<UnallocatedCString, std::uint64_t>;
    // unit definition id, subcontract
    using Subcontracts = UnallocatedMap<UnallocatedCString, Subcontract>;

    static auto CalculateBasketID(
        const api::Session& api,
        const proto::UnitDefinition& serialized) -> identifier::Generic;
    static auto FinalizeTemplate(
        const api::Session& api,
        const Nym_p& nym,
        proto::UnitDefinition& serialized,
        const PasswordPrompt& reason) -> bool;

    virtual auto BasketID() const -> identifier::Generic = 0;
    virtual auto Currencies() const -> const Subcontracts& = 0;
    virtual auto Weight() const -> std::uint64_t = 0;

    Basket(const Basket&) = delete;
    Basket(Basket&&) = delete;
    auto operator=(const Basket&) -> Basket& = delete;
    auto operator=(Basket&&) -> Basket& = delete;

    ~Basket() override = default;

protected:
    Basket() noexcept = default;

private:
    friend OTBasketContract;

#ifndef _WIN32
    auto clone() const noexcept -> Basket* override = 0;
#endif
};
}  // namespace opentxs::contract::unit
