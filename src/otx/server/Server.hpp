// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

#include "internal/otx/Types.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/cron/OTCron.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/AsyncConst.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/server/MainFile.hpp"
#include "otx/server/Notary.hpp"
#include "otx/server/Transactor.hpp"
#include "otx/server/UserCommandProcessor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::server
{
class Server
{
public:
    auto API() const -> const api::session::Notary& { return api_; }
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
        return user_command_processor_;
    }
    auto ComputeTimeout() -> std::chrono::milliseconds
    {
        return cron_->computeTimeout();
    }
    auto Cron() -> OTCron& { return *cron_; }
    auto DropMessageToNymbox(
        const identifier::Notary& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        transactionType transactionType,
        const Message& msg) -> bool;
    auto GetMainFile() -> MainFile& { return main_file_; }
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
    auto WalletFilename() -> String& { return wallet_filename_; }

    Server(
        const opentxs::api::session::Notary& manager,
        const PasswordPrompt& reason);
    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;

    ~Server();

private:
    friend MainFile;

    const UnallocatedCString default_external_ip_ = "127.0.0.1";
    const UnallocatedCString default_bind_ip_ = "127.0.0.1";
    const UnallocatedCString default_name_ = "localhost";
    const std::uint32_t default_port_ = 7085;
    const std::uint32_t min_tcp_port_ = 1024;
    const std::uint32_t max_tcp_port_ = 63356;

    const api::session::Notary& api_;
    const PasswordPrompt& reason_;
    MainFile main_file_;
    Notary notary_;
    Transactor transactor_;
    UserCommandProcessor user_command_processor_;
    OTString wallet_filename_;
    // Used at least for whether or not to write to the PID.
    bool read_only_{false};
    // If the server wants to be shut down, it can set
    // this flag so the caller knows to do so.
    bool shutdown_flag_{false};
    // A hash of the server contract
    AsyncConst<identifier::Notary> notary_id_;
    // A hash of the public key that signed the server contract
    UnallocatedCString server_nym_id_;
    // This is the server's own contract, containing its public key and
    // connect info.
    Nym_p nym_server_;
    std::unique_ptr<OTCron> cron_;  // This is where re-occurring and expiring
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
        return server_nym_id_;
    }
    auto SetNotaryID(const identifier::Notary& notaryID) noexcept -> void;
    void SetServerNymID(const char* strNymID) { server_nym_id_ = strNymID; }

    auto SendInstrumentToNym(
        const identifier::Notary& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const Message& msg) -> bool;
};
}  // namespace opentxs::server
