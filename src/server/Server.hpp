// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"

#include "Transactor.hpp"
#include "Notary.hpp"
#include "MainFile.hpp"
#include "UserCommandProcessor.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <tuple>

namespace opentxs::server
{
class Server
{
public:
    const api::Core& API() const;
    bool GetConnectInfo(
        proto::AddressType& type,
        std::string& hostname,
        std::uint32_t& port) const;
    const Identifier& GetServerID() const;
    const Nym& GetServerNym() const;
    std::unique_ptr<OTPassword> TransportKey(Data& pubkey) const;
    bool IsFlaggedForShutdown() const;

    void ActivateCron();
    UserCommandProcessor& CommandProcessor() { return userCommandProcessor_; }
    std::int64_t ComputeTimeout() { return m_Cron->computeTimeout(); }
    OTCron& Cron() { return *m_Cron; }
    bool DropMessageToNymbox(
        const Identifier& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        transactionType transactionType,
        const Message& msg);
    MainFile& GetMainFile() { return mainFile_; }
    Notary& GetNotary() { return notary_; }
    Transactor& GetTransactor() { return transactor_; }
    void Init(bool readOnly = false);
    bool LoadServerNym(const identifier::Nym& nymID);
    void ProcessCron();
    bool SendInstrumentToNym(
        const Identifier& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const OTPayment& payment,
        const char* command);
    String& WalletFilename() { return m_strWalletFilename; }

    ~Server();

private:
    friend api::server::implementation::Manager;
    friend MainFile;

    const std::string DEFAULT_EXTERNAL_IP = "127.0.0.1";
    const std::string DEFAULT_BIND_IP = "127.0.0.1";
    const std::string DEFAULT_NAME = "localhost";
    const std::uint32_t DEFAULT_COMMAND_PORT = 7085;
    const std::uint32_t DEFAULT_NOTIFY_PORT = 7086;
    const std::uint32_t MIN_TCP_PORT = 1024;
    const std::uint32_t MAX_TCP_PORT = 63356;

    const opentxs::api::server::Manager& manager_;
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
    OTIdentifier m_notaryID;
    // A hash of the public key that signed the server contract
    std::string m_strServerNymID;
    // This is the server's own contract, containing its public key and
    // connect info.
    ConstNym m_nymServer;
    std::unique_ptr<OTCron> m_Cron;  // This is where re-occurring and expiring
                                     // tasks go.
    OTZMQPushSocket notification_socket_;

    OTZMQMessage nymbox_push(
        const identifier::Nym& nymID,
        const OTTransaction& item) const;

    void CreateMainFile(bool& mainFileExists);
    // Note: SendInstrumentToNym and SendMessageToNym CALL THIS.
    // They are higher-level, this is lower-level.
    bool DropMessageToNymbox(
        const Identifier& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        transactionType transactionType,
        const Message* msg = nullptr,
        const String& messageString = String::Factory(),
        const char* command = nullptr);
    std::pair<std::string, std::string> parse_seed_backup(
        const std::string& input) const;
    const std::string& ServerNymID() const { return m_strServerNymID; }
    void SetNotaryID(const Identifier& notaryID) { m_notaryID = notaryID; }
    void SetServerNymID(const char* strNymID) { m_strServerNymID = strNymID; }

    bool SendInstrumentToNym(
        const Identifier& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const Message& msg);

    Server(const opentxs::api::server::Manager& manager);
    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
};
}  // namespace opentxs::server
