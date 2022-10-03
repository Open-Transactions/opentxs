// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/otx/Types.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/otx/consensus/Base.hpp"
#include "opentxs/otx/consensus/Client.hpp"
#include "opentxs/otx/consensus/Server.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class NymFile;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context::internal
{
class Base : virtual public opentxs::otx::context::Base
{
public:
    virtual auto GetContract(const Lock& lock) const -> proto::Context = 0;
    auto Internal() const noexcept -> const internal::Base& final
    {
        return *this;
    }
    virtual auto Nymfile(const PasswordPrompt& reason) const
        -> std::unique_ptr<const opentxs::NymFile> = 0;
    virtual auto ValidateContext(const Lock& lock) const -> bool = 0;

    virtual auto GetLock() -> std::mutex& = 0;
    auto Internal() noexcept -> internal::Base& final { return *this; }
    virtual auto mutable_Nymfile(const PasswordPrompt& reason)
        -> Editor<opentxs::NymFile> = 0;
    virtual auto UpdateSignature(const Lock& lock, const PasswordPrompt& reason)
        -> bool = 0;

#ifdef _MSC_VER
    Base() {}
#endif  // _MSC_VER
    ~Base() override = default;
};
class Client : virtual public opentxs::otx::context::Client,
               virtual public otx::context::internal::Base
{
public:
    auto InternalClient() const noexcept -> const internal::Client& final
    {
        return *this;
    }

    auto InternalClient() noexcept -> internal::Client& final { return *this; }

#ifdef _MSC_VER
    Client() {}
#endif  // _MSC_VER
    ~Client() override = default;
};
class Server : virtual public opentxs::otx::context::Server,
               virtual public otx::context::internal::Base
{
public:
    virtual auto HaveSufficientNumbers(const MessageType reason) const
        -> bool = 0;
    virtual auto InitializeServerCommand(
        const MessageType type,
        const Armored& payload,
        const identifier::Generic& accountID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = true)
        -> std::pair<RequestNumber, std::unique_ptr<Message>> = 0;
    virtual auto InitializeServerCommand(
        const MessageType type,
        const identifier::Nym& recipientNymID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false)
        -> std::pair<RequestNumber, std::unique_ptr<Message>> = 0;
    virtual auto InitializeServerCommand(
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false)
        -> std::pair<RequestNumber, std::unique_ptr<Message>> = 0;
    auto InternalServer() const noexcept -> const internal::Server& final
    {
        return *this;
    }
    virtual auto NextTransactionNumber(const MessageType reason)
        -> ManagedNumber = 0;

    auto InternalServer() noexcept -> internal::Server& final { return *this; }
    virtual auto mutable_Purse(
        const identifier::UnitDefinition& id,
        const PasswordPrompt& reason)
        -> Editor<otx::blind::Purse, std::shared_mutex> = 0;

#ifdef _MSC_VER
    Server() {}
#endif  // _MSC_VER
    ~Server() override = default;
};
}  // namespace opentxs::otx::context::internal

namespace opentxs::factory
{
auto ClientContext(
    const api::Session& api,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server) -> otx::context::internal::Client*;
auto ClientContext(
    const api::Session& api,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server) -> otx::context::internal::Client*;
auto ManagedNumber(const TransactionNumber number, otx::context::Server&)
    -> otx::context::ManagedNumber;
auto ServerContext(
    const api::session::Client& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server,
    network::ServerConnection& connection) -> otx::context::internal::Server*;
auto ServerContext(
    const api::session::Client& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    network::ServerConnection& connection) -> otx::context::internal::Server*;
}  // namespace opentxs::factory
