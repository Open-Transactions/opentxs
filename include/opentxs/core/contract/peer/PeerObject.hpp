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
    static std::unique_ptr<PeerObject> Create(
        const ConstNym& senderNym,
        const std::string& message);
    static std::unique_ptr<PeerObject> Create(
        const ConstNym& senderNym,
        const std::string& payment,
        const bool isPayment);
    static std::unique_ptr<PeerObject> Create(
        std::unique_ptr<PeerRequest>& request,
        std::unique_ptr<PeerReply>& reply);
    static std::unique_ptr<PeerObject> Create(
        std::unique_ptr<PeerRequest>& request);
    static std::unique_ptr<PeerObject> Factory(
        const ConstNym& signerNym,
        const proto::PeerObject& serialized);
    static std::unique_ptr<PeerObject> Factory(
        const ConstNym& recipientNym,
        const OTASCIIArmor& encrypted);

    const ConstNym& Nym() const { return nym_; }
    const std::unique_ptr<PeerRequest>& Request() const { return request_; }
    const std::unique_ptr<PeerReply>& Reply() const { return reply_; }
    proto::PeerObject Serialize() const;
    proto::PeerObjectType Type() const { return type_; }
    bool Validate() const;

    std::unique_ptr<std::string>& Message() { return message_; }
    std::unique_ptr<std::string>& Payment() { return payment_; }

    ~PeerObject() = default;

private:
    ConstNym nym_{nullptr};
    std::unique_ptr<std::string> message_{nullptr};
    std::unique_ptr<std::string> payment_{nullptr};
    std::unique_ptr<PeerReply> reply_{nullptr};
    std::unique_ptr<PeerRequest> request_{nullptr};
    proto::PeerObjectType type_{proto::PEEROBJECT_ERROR};
    std::uint32_t version_{0};

    PeerObject(const ConstNym& signerNym, const proto::PeerObject serialized);
    PeerObject(const ConstNym& senderNym, const std::string& message);
    PeerObject(const std::string& payment, const ConstNym& senderNym);
    PeerObject(
        std::unique_ptr<PeerRequest>& request,
        std::unique_ptr<PeerReply>& reply);
    PeerObject(std::unique_ptr<PeerRequest>& request);
    PeerObject() = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP
