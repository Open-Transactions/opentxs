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

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP

#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{

class OTASCIIArmor;

class PeerObject
{
public:
    static std::unique_ptr<PeerObject> Create(const std::string& message);
    static std::unique_ptr<PeerObject> Create(
        std::unique_ptr<PeerRequest>& request,
        std::unique_ptr<PeerReply>& reply);
    static std::unique_ptr<PeerObject> Create(
        std::unique_ptr<PeerRequest>& request);
    static std::unique_ptr<PeerObject> Factory(
        const ConstNym& nym,
        const proto::PeerObject& serialized);
    static std::unique_ptr<PeerObject> Factory(
        const ConstNym& recipientNym,
        const OTASCIIArmor& encrypted);

    std::unique_ptr<std::string>& Message() { return message_; }
    const std::unique_ptr<PeerRequest>& Request() const { return request_; }
    const std::unique_ptr<PeerReply>& Reply() const { return reply_; }
    proto::PeerObject Serialize() const;
    proto::PeerObjectType Type() const { return type_; }
    bool Validate() const;

    ~PeerObject() = default;

private:
    std::unique_ptr<std::string> message_;
    std::unique_ptr<PeerReply> reply_;
    std::unique_ptr<PeerRequest> request_;
    proto::PeerObjectType type_{proto::PEEROBJECT_ERROR};
    std::uint32_t version_{};

    PeerObject(const ConstNym& nym, const proto::PeerObject serialized);
    PeerObject(const std::string& message);
    PeerObject(
        std::unique_ptr<PeerRequest>& request,
        std::unique_ptr<PeerReply>& reply);
    PeerObject(std::unique_ptr<PeerRequest>& request);
    PeerObject() = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP
