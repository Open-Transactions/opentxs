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

#ifndef OPENTXS_SERVER_REPLYMESSAGE_HPP
#define OPENTXS_SERVER_REPLYMESSAGE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <set>

namespace opentxs
{
class OTASCIIArmor;
class ClientContext;
class Message;

namespace api
{
namespace client
{
class Wallet;
}  // namespace client
}  // namespace api

namespace server
{

class Server;

class ReplyMessage
{
public:
    ReplyMessage(
        const opentxs::api::client::Wallet& wallet,
        const Identifier& notaryID,
        const Nym& signer,
        const Message& input,
        Server& server,
        const MessageType& type,
        Message& output);

    std::set<RequestNumber> Acknowledged() const;
    bool HaveContext() const;
    const bool& Init() const;
    const Message& Original() const;
    const bool& Success() const;

    ClientContext& Context();
    void ClearRequest();
    void DropToNymbox(const bool success);
    bool InitNymfileCredentials();
    bool LoadContext();
    bool LoadNym();
    Nym& Nymfile();
    void OverrideType(const String& accountID);
    void SetAccount(const String& accountID);
    void SetAcknowledgments(const ClientContext& context);
    void SetDepth(const std::int64_t depth);
    void SetInboxHash(const Identifier& hash);
    void SetInstrumentDefinitionID(const String& id);
    void SetNymboxHash(const Identifier& hash);
    void SetOutboxHash(const Identifier& hash);
    bool SetPayload(const String& payload);
    bool SetPayload(const Data& payload);
    void SetPayload(const OTASCIIArmor& payload);
    bool SetPayload2(const String& payload);
    bool SetPayload3(const String& payload);
    void SetRequestNumber(const RequestNumber number);
    void SetSuccess(const bool success);
    void SetTargetNym(const String& nymID);
    void SetTransactionNumber(const TransactionNumber& number);

    ~ReplyMessage();

private:
    const opentxs::api::client::Wallet& wallet_;
    const Nym& signer_;
    const Message& original_;
    const Identifier& notary_id_;
    Message& message_;
    Server& server_;
    Nym nymfile_;
    bool init_{false};
    bool drop_{false};
    bool drop_status_{false};
    std::shared_ptr<const Nym> sender_nym_{nullptr};
    std::unique_ptr<Editor<ClientContext>> context_{nullptr};

    void attach_request();
    void clear_request();
    bool init();
    bool init_nym();

    ReplyMessage() = delete;
    ReplyMessage(const ReplyMessage&) = delete;
    ReplyMessage(ReplyMessage&&) = delete;
    ReplyMessage& operator=(const ReplyMessage&) = delete;
    ReplyMessage& operator=(ReplyMessage&&) = delete;
};
}  // namespace server
}  // namespace opentxs

#endif  // OPENTXS_SERVER_REPLYMESSAGE_HPP
