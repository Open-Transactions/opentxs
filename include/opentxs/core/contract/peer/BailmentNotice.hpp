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

#ifndef OPENTXS_CORE_CONTRACT_PEER_BAILMENTNOTICE_HPP
#define OPENTXS_CORE_CONTRACT_PEER_BAILMENTNOTICE_HPP

#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/Identifier.hpp"

#include <string>

namespace opentxs
{

class BailmentNotice : public PeerRequest
{
private:
    typedef PeerRequest ot_super;
    friend class PeerRequest;

    Identifier unit_;
    Identifier server_;
    std::string txid_;

    proto::PeerRequest IDVersion(const Lock& lock) const override;

    BailmentNotice(
        const ConstNym& nym,
        const proto::PeerRequest& serialized);
    BailmentNotice(
        const ConstNym& nym,
        const Identifier& recipientID,
        const Identifier& unitID,
        const Identifier& serverID,
        const std::string& txid);
    BailmentNotice() = delete;

public:

    ~BailmentNotice() = default;
};
} // namespace opentxs

#endif // OPENTXS_CORE_CONTRACT_PEER_BAILMENTNOTICE_HPP
