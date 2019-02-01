// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/SecurityContract.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/Proto.hpp"

#include <string>

namespace opentxs
{
SecurityContract::SecurityContract(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const proto::UnitDefinition serialized)
    : ot_super(wallet, nym, serialized)
{
}

SecurityContract::SecurityContract(
    const api::Wallet& wallet,
    const ConstNym& nym,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms)
    : ot_super(wallet, nym, shortname, name, symbol, terms)
{
}

proto::UnitDefinition SecurityContract::IDVersion(const Lock& lock) const
{
    proto::UnitDefinition contract = ot_super::IDVersion(lock);
    contract.set_type(Type());
    auto security = contract.mutable_security();
    security->set_version(1);
    security->set_type(proto::EQUITYTYPE_SHARES);

    return contract;
}
}  // namespace opentxs
