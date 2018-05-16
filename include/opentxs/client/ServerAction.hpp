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

#ifndef OPENTXS_CLIENT_SERVER_ACTION_HPP
#define OPENTXS_CLIENT_SERVER_ACTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <memory>
#include <string>

namespace opentxs
{
class Message;
class PeerReply;
class PeerRequest;

namespace client
{
class ServerAction
{
public:
    EXPORT virtual TransactionNumber GetTransactionNumber() const = 0;
    EXPORT virtual SendResult LastSendResult() const = 0;
    EXPORT virtual const Identifier& MessageID() const = 0;
    EXPORT virtual const std::shared_ptr<PeerRequest>& SentPeerRequest()
        const = 0;
    EXPORT virtual const std::shared_ptr<PeerReply>& SentPeerReply() const = 0;
    EXPORT virtual const std::shared_ptr<Message>& Reply() const = 0;

    EXPORT virtual std::string Run(const std::size_t totalRetries = 2) = 0;

    EXPORT virtual ~ServerAction() = default;

protected:
    ServerAction() = default;

private:
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    ServerAction& operator=(const ServerAction&) = delete;
    ServerAction& operator=(ServerAction&&) = delete;
};
}  // namespace client
}  // namespace opentxs
#endif  // OPENTXS_CLIENT_SERVER_ACTION_HPP
