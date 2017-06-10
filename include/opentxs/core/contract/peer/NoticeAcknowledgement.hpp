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

#ifndef OPENTXS_CORE_CONTRACT_PEER_NOTICEACKNOWLEDGEMENT_HPP
#define OPENTXS_CORE_CONTRACT_PEER_NOTICEACKNOWLEDGEMENT_HPP

#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/Identifier.hpp"

#include <string>

namespace opentxs
{

class NoticeAcknowledgement : public PeerReply
{
private:
    typedef PeerReply ot_super;
    friend class PeerReply;

    bool ack_{false};

    proto::PeerReply IDVersion(const Lock& lock) const override;

    NoticeAcknowledgement(
        const ConstNym& nym,
        const proto::PeerReply& serialized);
    NoticeAcknowledgement(
        const ConstNym& nym,
        const Identifier& initiator,
        const Identifier& request,
        const Identifier& server,
        const proto::PeerRequestType type,
        const bool& ack);
    NoticeAcknowledgement() = delete;

public:

    ~NoticeAcknowledgement() = default;
};
} // namespace opentxs

#endif // OPENTXS_CORE_CONTRACT_PEER_NOTICEACKNOWLEDGEMENT_HPP
