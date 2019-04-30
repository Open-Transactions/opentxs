// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONSENSUS_SERVERCONTEXT_HPP
#define OPENTXS_CONSENSUS_SERVERCONTEXT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/consensus/Context.hpp"

#include <future>
#include <tuple>

namespace opentxs
{
class ServerContext : virtual public Context
{
public:
    using DeliveryResult =
        std::pair<proto::LastReplyStatus, std::shared_ptr<Message>>;
    using SendFuture = std::future<DeliveryResult>;
    using QueueResult = std::unique_ptr<SendFuture>;
    // account label, resync nym
    using ExtraArgs = std::pair<std::string, bool>;

    EXPORT virtual std::vector<OTIdentifier> Accounts() const = 0;
    EXPORT virtual const std::string& AdminPassword() const = 0;
    EXPORT virtual bool AdminAttempted() const = 0;
    EXPORT virtual bool FinalizeServerCommand(Message& command) const = 0;
    EXPORT virtual bool HaveAdminPassword() const = 0;
    EXPORT virtual bool HaveSufficientNumbers(
        const MessageType reason) const = 0;
    EXPORT virtual TransactionNumber Highest() const = 0;
    EXPORT virtual bool isAdmin() const = 0;
#if OT_CASH
    EXPORT virtual std::shared_ptr<const blind::Purse> Purse(
        const identifier::UnitDefinition& id) const = 0;
#endif
    EXPORT virtual std::uint64_t Revision() const = 0;
    EXPORT virtual bool ShouldRename(
        const std::string& defaultName = "") const = 0;
    EXPORT virtual bool StaleNym() const = 0;
    EXPORT virtual std::unique_ptr<Item> Statement(
        const OTTransaction& owner) const = 0;
    EXPORT virtual std::unique_ptr<Item> Statement(
        const OTTransaction& owner,
        const TransactionNumbers& adding) const = 0;
    EXPORT virtual std::unique_ptr<TransactionStatement> Statement(
        const TransactionNumbers& adding,
        const TransactionNumbers& without) const = 0;
    EXPORT virtual bool Verify(const TransactionStatement& statement) const = 0;
    EXPORT virtual bool VerifyTentativeNumber(
        const TransactionNumber& number) const = 0;

    EXPORT virtual bool AcceptIssuedNumber(const TransactionNumber& number) = 0;
    EXPORT virtual bool AcceptIssuedNumbers(
        const TransactionStatement& statement) = 0;
    EXPORT virtual bool AddTentativeNumber(const TransactionNumber& number) = 0;
    EXPORT virtual network::ServerConnection& Connection() = 0;
    EXPORT virtual std::pair<RequestNumber, std::unique_ptr<Message>>
    InitializeServerCommand(
        const MessageType type,
        const Armored& payload,
        const Identifier& accountID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = true) = 0;
    EXPORT virtual std::pair<RequestNumber, std::unique_ptr<Message>>
    InitializeServerCommand(
        const MessageType type,
        const identifier::Nym& recipientNymID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false) = 0;
    EXPORT virtual std::pair<RequestNumber, std::unique_ptr<Message>>
    InitializeServerCommand(
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false) = 0;
    EXPORT virtual void Join() = 0;
#if OT_CASH
    EXPORT virtual Editor<blind::Purse> mutable_Purse(
        const identifier::UnitDefinition& id) = 0;
#endif
    EXPORT virtual OTManagedNumber NextTransactionNumber(
        const MessageType reason) = 0;
    EXPORT virtual NetworkReplyMessage PingNotary() = 0;
    EXPORT virtual bool ProcessNotification(
        const api::client::Manager& client,
        const otx::Reply& notification) = 0;
    EXPORT virtual QueueResult Queue(
        const api::client::Manager& client,
        std::shared_ptr<Message> message,
        const ExtraArgs& args = ExtraArgs{}) = 0;
    EXPORT virtual QueueResult Queue(
        const api::client::Manager& client,
        std::shared_ptr<Message> message,
        std::shared_ptr<Ledger> inbox,
        std::shared_ptr<Ledger> outbox,
        std::set<OTManagedNumber>* numbers,
        const ExtraArgs& args = ExtraArgs{}) = 0;
    EXPORT virtual QueueResult RefreshNymbox(
        const api::client::Manager& client) = 0;
    EXPORT virtual bool RemoveTentativeNumber(
        const TransactionNumber& number) = 0;
    EXPORT virtual void ResetThread() = 0;
    EXPORT virtual bool Resync(const proto::Context& serialized) = 0;
    [[deprecated]] EXPORT virtual NetworkReplyMessage SendMessage(
        const api::client::Manager& client,
        const std::set<OTManagedNumber>& pending,
        ServerContext& context,
        const Message& message,
        const std::string& label = "",
        const bool resync = false) = 0;
    EXPORT virtual void SetAdminAttempted() = 0;
    EXPORT virtual void SetAdminPassword(const std::string& password) = 0;
    EXPORT virtual void SetAdminSuccess() = 0;
    EXPORT virtual bool SetHighest(const TransactionNumber& highest) = 0;
    EXPORT virtual void SetPush(const bool enabled) = 0;
    EXPORT virtual void SetRevision(const std::uint64_t revision) = 0;
    EXPORT virtual TransactionNumber UpdateHighest(
        const TransactionNumbers& numbers,
        TransactionNumbers& good,
        TransactionNumbers& bad) = 0;
    EXPORT virtual RequestNumber UpdateRequestNumber() = 0;
    EXPORT virtual RequestNumber UpdateRequestNumber(bool& sendStatus) = 0;
    EXPORT virtual bool UpdateRequestNumber(Message& command) = 0;

    EXPORT virtual ~ServerContext() override = default;

protected:
    ServerContext() = default;

private:
    ServerContext(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;
    ServerContext& operator=(const ServerContext&) = delete;
    ServerContext& operator=(ServerContext&&) = delete;
};
}  // namespace opentxs
#endif
