/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_BASKET_BASKETCONTRACT_HPP
#define OPENTXS_CORE_BASKET_BASKETCONTRACT_HPP

#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Identifier.hpp"

namespace opentxs
{

class Basket;
class Nym;
class UserCommandProcessor;

class BasketContract : public UnitDefinition
{
private:
    typedef UnitDefinition ot_super;
    // account number, weight
    typedef std::pair<std::string, uint64_t> Subcontract;
    // unit definition id, subcontract
    typedef std::map<std::string, Subcontract> MapOfSubcontracts;
    friend ot_super;
    friend UserCommandProcessor;

    MapOfSubcontracts subcontracts_;
    uint64_t weight_;

    EXPORT BasketContract(
        const ConstNym& nym,
        const proto::UnitDefinition serialized);
    EXPORT BasketContract(
        const ConstNym& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const uint64_t weight);

    EXPORT proto::UnitDefinition BasketIDVersion(const Lock& lock) const;
    EXPORT proto::UnitDefinition IDVersion(const Lock& lock) const override;

public:
    EXPORT static Identifier CalculateBasketID(
        const proto::UnitDefinition& serialized);
    EXPORT static bool FinalizeTemplate(
        const ConstNym& nym,
        proto::UnitDefinition& serialized);

    EXPORT Identifier BasketID() const;
    EXPORT const MapOfSubcontracts& Currencies() const
    {
        return subcontracts_;
    }
    EXPORT proto::UnitType Type() const override
    {
        return proto::UNITTYPE_BASKET;
    }
    EXPORT uint64_t Weight() const
    {
        return weight_;
    }

    virtual ~BasketContract() = default;
};

} // namespace opentxs

#endif // OPENTXS_CORE_BASKET_BASKETCONTRACT_HPP
