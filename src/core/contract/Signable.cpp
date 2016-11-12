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

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs
{

Signable::Signable(const ConstNym& nym)
    : nym_(nym)
{
    Log::vOutput(0, "Signable::Signable: Signable::Signable(const ConstNym& nym)");
}

Signable::Signable(const ConstNym& nym, const std::uint32_t version)
    : nym_(nym)
    , version_(version)
{
}

Signable::Signable(
    const ConstNym& nym,
    const std::uint32_t version,
    const std::string& conditions)
        : nym_(nym)
        , version_(version)
        , conditions_(conditions)
{
}

bool Signable::UpdateSignature()
{
    if (!nym_) {
        otErr << __FUNCTION__ << ": Missing nym." << std::endl;

        return false;
    }

    return true;
}

bool Signable::VerifySignature(
    __attribute__((unused)) const proto::Signature& signature) const
{
    if (!nym_) {
        otErr << __FUNCTION__ << ": Missing nym." << std::endl;

        return false;
    }

    return true;
}
} // namespace opentxs
