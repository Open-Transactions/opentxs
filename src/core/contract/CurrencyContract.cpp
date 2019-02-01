// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/CurrencyContract.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{

CurrencyContract::CurrencyContract(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::UnitDefinition serialized)
    : ot_super(wallet, nym, serialized)
    , tla_(serialized.currency().tla())
    , power_(serialized.currency().power())
    , fractional_unit_name_(serialized.currency().fraction())
{
}

CurrencyContract::CurrencyContract(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const std::string& tla,
    const std::uint32_t& power,
    const std::string& fraction)
    : ot_super(wallet, nym, shortname, name, symbol, terms)
    , tla_(tla)
    , fractional_unit_name_(fraction)
{
    power_ = power;
}

proto::UnitDefinition CurrencyContract::IDVersion(const Lock& lock) const
{
    proto::UnitDefinition contract = ot_super::IDVersion(lock);
    contract.set_type(proto::UNITTYPE_CURRENCY);
    auto currency = contract.mutable_currency();
    currency->set_version(1);
    currency->set_tla(tla_);
    currency->set_power(power_);
    currency->set_fraction(fractional_unit_name_);

    return contract;
}
}  // namespace opentxs
