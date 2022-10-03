// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/otx/consensus/Base.hpp"
#include "opentxs/otx/consensus/ManagedNumber.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace network
{
class ServerConnection;
}  // namespace network

namespace identifier
{
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind

namespace context
{
class TransactionStatement;
}  // namespace context

class Reply;
}  // namespace otx

namespace otx
{
namespace context
{
namespace internal
{
class Server;
}  // namespace internal
}  // namespace context
}  // namespace otx

namespace server
{
class Server;
}  // namespace server

class Item;
class Ledger;
class OTTransaction;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context
{
class OPENTXS_EXPORT Server : virtual public Base
{
public:
    using DeliveryResult =
        std::pair<otx::LastReplyStatus, std::shared_ptr<Message>>;
    using SendFuture = std::future<DeliveryResult>;
    using QueueResult = std::unique_ptr<SendFuture>;
    // account label, resync nym
    using ExtraArgs = std::pair<UnallocatedCString, bool>;

    virtual auto Accounts() const -> UnallocatedVector<identifier::Generic> = 0;
    virtual auto AdminPassword() const -> const UnallocatedCString& = 0;
    virtual auto AdminAttempted() const -> bool = 0;
    virtual auto FinalizeServerCommand(
        Message& command,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto HaveAdminPassword() const -> bool = 0;
    virtual auto Highest() const -> TransactionNumber = 0;
    OPENTXS_NO_EXPORT virtual auto InternalServer() const noexcept
        -> const internal::Server& = 0;
    virtual auto isAdmin() const -> bool = 0;
    virtual void Join() const = 0;
    virtual auto Purse(const identifier::UnitDefinition& id) const
        -> const otx::blind::Purse& = 0;
    virtual auto Revision() const -> std::uint64_t = 0;
    virtual auto ShouldRename(
        const UnallocatedCString& defaultName = "localhost") const -> bool = 0;
    virtual auto StaleNym() const -> bool = 0;
    virtual auto Statement(
        const OTTransaction& owner,
        const PasswordPrompt& reason) const -> std::unique_ptr<Item> = 0;
    virtual auto Statement(
        const OTTransaction& owner,
        const TransactionNumbers& adding,
        const PasswordPrompt& reason) const -> std::unique_ptr<Item> = 0;
    virtual auto Statement(
        const TransactionNumbers& adding,
        const TransactionNumbers& without,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<TransactionStatement> = 0;
    virtual auto Verify(const TransactionStatement& statement) const
        -> bool = 0;
    virtual auto VerifyTentativeNumber(const TransactionNumber& number) const
        -> bool = 0;

    virtual auto AcceptIssuedNumber(const TransactionNumber& number)
        -> bool = 0;
    virtual auto AcceptIssuedNumbers(const TransactionStatement& statement)
        -> bool = 0;
    virtual auto AddTentativeNumber(const TransactionNumber& number)
        -> bool = 0;
    virtual auto Connection() -> network::ServerConnection& = 0;
    OPENTXS_NO_EXPORT virtual auto InternalServer() noexcept
        -> internal::Server& = 0;
    virtual auto PingNotary(const PasswordPrompt& reason)
        -> client::NetworkReplyMessage = 0;
    virtual auto ProcessNotification(
        const api::session::Client& client,
        const otx::Reply& notification,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto Queue(
        const api::session::Client& client,
        std::shared_ptr<Message> message,
        const PasswordPrompt& reason,
        const ExtraArgs& args = ExtraArgs{}) -> QueueResult = 0;
    virtual auto Queue(
        const api::session::Client& client,
        std::shared_ptr<Message> message,
        std::shared_ptr<Ledger> inbox,
        std::shared_ptr<Ledger> outbox,
        UnallocatedSet<ManagedNumber>* numbers,
        const PasswordPrompt& reason,
        const ExtraArgs& args = ExtraArgs{}) -> QueueResult = 0;
    virtual auto RefreshNymbox(
        const api::session::Client& client,
        const PasswordPrompt& reason) -> QueueResult = 0;
    virtual auto RemoveTentativeNumber(const TransactionNumber& number)
        -> bool = 0;
    virtual void ResetThread() = 0;
    virtual auto Resync(const proto::Context& serialized) -> bool = 0;
    [[deprecated]] virtual auto SendMessage(
        const api::session::Client& client,
        const UnallocatedSet<ManagedNumber>& pending,
        Server& context,
        const Message& message,
        const PasswordPrompt& reason,
        const UnallocatedCString& label = "",
        const bool resync = false) -> client::NetworkReplyMessage = 0;
    virtual void SetAdminAttempted() = 0;
    virtual void SetAdminPassword(const UnallocatedCString& password) = 0;
    virtual void SetAdminSuccess() = 0;
    virtual auto SetHighest(const TransactionNumber& highest) -> bool = 0;
    virtual void SetPush(const bool enabled) = 0;
    virtual void SetRevision(const std::uint64_t revision) = 0;
    virtual auto UpdateHighest(
        const TransactionNumbers& numbers,
        TransactionNumbers& good,
        TransactionNumbers& bad) -> TransactionNumber = 0;
    virtual auto UpdateRequestNumber(const PasswordPrompt& reason)
        -> RequestNumber = 0;
    virtual auto UpdateRequestNumber(
        bool& sendStatus,
        const PasswordPrompt& reason) -> RequestNumber = 0;
    virtual auto UpdateRequestNumber(
        Message& command,
        const PasswordPrompt& reason) -> bool = 0;

    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;

    ~Server() override = default;

protected:
    Server() = default;
};
}  // namespace opentxs::otx::context
