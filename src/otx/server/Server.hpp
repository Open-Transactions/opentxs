// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/cron/OTCron.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/server/MainFile.hpp"
#include "otx/server/Notary.hpp"
#include "otx/server/Transactor.hpp"
#include "otx/server/UserCommandProcessor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Notary;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

class Data;
class OTPassword;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::server
{
class Server
{
public:
    auto API() const -> const api::session::Notary& { return manager_; }
    auto GetConnectInfo(
        AddressType& type,
        UnallocatedCString& hostname,
        std::uint32_t& port) const -> bool;
    auto GetServerID() const noexcept -> const identifier::Notary&;
    auto GetServerNym() const -> const identity::Nym&;
    auto TransportKey(Data& pubkey) const -> OTSecret;
    auto IsFlaggedForShutdown() const -> bool;

    void ActivateCron();
    auto CommandProcessor() -> UserCommandProcessor&
    {
        return userCommandProcessor_;
    }
    auto ComputeTimeout() -> std::chrono::milliseconds
    {
        return m_Cron->computeTimeout();
    }
    auto Cron() -> OTCron& { return *m_Cron; }
    auto DropMessageToNymbox(
        const identifier::Notary& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        transactionType transactionType,
        const Message& msg) -> bool;
    auto GetMainFile() -> MainFile& { return mainFile_; }
    auto GetNotary() -> Notary& { return notary_; }
    auto GetTransactor() -> Transactor& { return transactor_; }
    void Init(bool readOnly = false);
    auto LoadServerNym(const identifier::Nym& nymID) -> bool;
    void ProcessCron();
    auto SendInstrumentToNym(
        const identifier::Notary& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const OTPayment& payment,
        const char* command) -> bool;
    auto WalletFilename() -> String& { return m_strWalletFilename; }

    Server(
        const opentxs::api::session::Notary& manager,
        const PasswordPrompt& reason);

    ~Server();

private:
    friend MainFile;

    const UnallocatedCString DEFAULT_EXTERNAL_IP = "127.0.0.1";
    const UnallocatedCString DEFAULT_BIND_IP = "127.0.0.1";
    const UnallocatedCString DEFAULT_NAME = "localhost";
    const std::uint32_t DEFAULT_PORT = 7085;
    const std::uint32_t MIN_TCP_PORT = 1024;
    const std::uint32_t MAX_TCP_PORT = 63356;

    const api::session::Notary& manager_;
    const PasswordPrompt& reason_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_future_;
    std::atomic<bool> have_id_;
    MainFile mainFile_;
    Notary notary_;
    Transactor transactor_;
    UserCommandProcessor userCommandProcessor_;
    OTString m_strWalletFilename;
    // Used at least for whether or not to write to the PID.
    bool m_bReadOnly{false};
    // If the server wants to be shut down, it can set
    // this flag so the caller knows to do so.
    bool m_bShutdownFlag{false};
    // A hash of the server contract
    OTNotaryID m_notaryID;
    // A hash of the public key that signed the server contract
    UnallocatedCString m_strServerNymID;
    // This is the server's own contract, containing its public key and
    // connect info.
    Nym_p m_nymServer;
    std::unique_ptr<OTCron> m_Cron;  // This is where re-occurring and expiring
                                     // tasks go.
    OTZMQPushSocket notification_socket_;

    auto nymbox_push(const identifier::Nym& nymID, const OTTransaction& item)
        const -> network::zeromq::Message;

    void CreateMainFile(bool& mainFileExists);
    // Note: SendInstrumentToNym and SendMessageToNym CALL THIS.
    // They are higher-level, this is lower-level.
    auto DropMessageToNymbox(
        const identifier::Notary& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        transactionType transactionType,
        const Message* msg = nullptr,
        const String& messageString = String::Factory(),
        const char* command = nullptr) -> bool;
    auto parse_seed_backup(const UnallocatedCString& input) const
        -> std::pair<UnallocatedCString, UnallocatedCString>;
    auto ServerNymID() const -> const UnallocatedCString&
    {
        return m_strServerNymID;
    }
    auto SetNotaryID(const identifier::Notary& notaryID) noexcept -> void;
    void SetServerNymID(const char* strNymID) { m_strServerNymID = strNymID; }

    auto SendInstrumentToNym(
        const identifier::Notary& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const Message& msg) -> bool;

    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;
};
}  // namespace opentxs::server
