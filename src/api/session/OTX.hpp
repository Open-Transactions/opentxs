// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "internal/api/session/OTX.hpp"
#include "internal/otx/client/Client.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/util/Time.hpp"
#include "otx/client/StateMachine.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace contract
{
namespace peer
{
class Reply;
class Request;
}  // namespace peer

class Server;
}  // namespace contract

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace client
{
namespace implementation
{
class StateMachine;
}  // namespace implementation
}  // namespace client
}  // namespace otx

class OTClient;
}  // namespace opentxs

namespace opentxs::api::session::imp
{
class OTX final : virtual public internal::OTX, Lockable
{
public:
    auto AcknowledgeBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const Identifier& requestID,
        const std::string& instructions,
        const SetID setID) const -> BackgroundTask final;
    auto AcknowledgeNotice(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const bool ack,
        const SetID setID) const -> BackgroundTask final;
    auto AcknowledgeOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const std::string& details,
        const SetID setID) const -> BackgroundTask final;
    auto AcknowledgeConnection(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const SetID setID) const -> BackgroundTask final;
    auto AutoProcessInboxEnabled() const -> bool final
    {
        return auto_process_inbox_.get();
    }
    auto CanDeposit(
        const identifier::Nym& recipientNymID,
        const OTPayment& payment) const -> Depositability final;
    auto CanDeposit(
        const identifier::Nym& recipientNymID,
        const Identifier& accountID,
        const OTPayment& payment) const -> Depositability final;
    auto CanMessage(
        const identifier::Nym& senderNymID,
        const Identifier& recipientContactID,
        const bool startIntroductionServer) const -> Messagability final;
    auto CheckTransactionNumbers(
        const identifier::Nym& nym,
        const identifier::Server& serverID,
        const std::size_t quantity) const -> bool final;
    auto ContextIdle(
        const identifier::Nym& nym,
        const identifier::Server& server) const -> Finished final;
    auto DepositCheques(const identifier::Nym& nymID) const
        -> std::size_t final;
    auto DepositCheques(
        const identifier::Nym& nymID,
        const std::pmr::set<OTIdentifier>& chequeIDs) const
        -> std::size_t final;
    auto DepositPayment(
        const identifier::Nym& recipientNymID,
        const std::shared_ptr<const OTPayment>& payment) const
        -> BackgroundTask final;
    auto DepositPayment(
        const identifier::Nym& recipientNymID,
        const Identifier& accountID,
        const std::shared_ptr<const OTPayment>& payment) const
        -> BackgroundTask final;
    void DisableAutoaccept() const final;
    auto DownloadMint(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit) const -> BackgroundTask final;
    auto DownloadNym(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID) const -> BackgroundTask final;
    auto DownloadNymbox(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const -> BackgroundTask final;
    auto DownloadServerContract(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Server& contractID) const -> BackgroundTask final;
    auto DownloadUnitDefinition(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& contractID) const
        -> BackgroundTask final;
    auto FindNym(const identifier::Nym& nymID) const -> BackgroundTask final;
    auto FindNym(
        const identifier::Nym& nymID,
        const identifier::Server& serverIDHint) const -> BackgroundTask final;
    auto FindServer(const identifier::Server& serverID) const
        -> BackgroundTask final;
    auto FindUnitDefinition(const identifier::UnitDefinition& unit) const
        -> BackgroundTask final;
    auto InitiateBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const SetID setID) const -> BackgroundTask final;
    auto InitiateOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Amount amount,
        const std::string& message,
        const SetID setID) const -> BackgroundTask final;
    auto InitiateRequestConnection(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const contract::peer::ConnectionInfoType& type,
        const SetID setID) const -> BackgroundTask final;
    auto InitiateStoreSecret(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const contract::peer::SecretType& type,
        const std::string& primary,
        const std::string& secondary,
        const SetID setID) const -> BackgroundTask final;
    auto IntroductionServer() const -> const identifier::Server& final;
    auto IssueUnitDefinition(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& unitID,
        const core::UnitType advertise,
        const std::string& label) const -> BackgroundTask final;
    auto MessageContact(
        const identifier::Nym& senderNymID,
        const Identifier& contactID,
        const std::string& message,
        const SetID setID) const -> BackgroundTask final;
    auto MessageStatus(const TaskID taskID) const
        -> std::pair<ThreadStatus, MessageID> final;
    auto NotifyBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount amount,
        const SetID setID) const -> BackgroundTask final;
    auto PayContact(
        const identifier::Nym& senderNymID,
        const Identifier& contactID,
        std::shared_ptr<const OTPayment> payment) const -> BackgroundTask final;
    auto PayContactCash(
        const identifier::Nym& senderNymID,
        const Identifier& contactID,
        const Identifier& workflowID) const -> BackgroundTask final;
    auto ProcessInbox(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID) const -> BackgroundTask final;
    auto PublishServerContract(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& contractID) const -> BackgroundTask final;
    void Refresh() const final;
    auto RefreshCount() const -> std::uint64_t final;
    auto RegisterAccount(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& unitID,
        const std::string& label) const -> BackgroundTask final;
    auto RegisterNym(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const bool resync) const -> BackgroundTask final;
    auto RegisterNymPublic(
        const identifier::Nym& nymID,
        const identifier::Server& server,
        const bool setContactData,
        const bool forcePrimary,
        const bool resync) const -> BackgroundTask final;
    auto SetIntroductionServer(const contract::Server& contract) const
        -> OTServerID final;
    auto SendCheque(
        const identifier::Nym& localNymID,
        const Identifier& sourceAccountID,
        const Identifier& recipientContactID,
        const Amount value,
        const std::string& memo,
        const Time validFrom,
        const Time validTo) const -> BackgroundTask final;
    auto SendExternalTransfer(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount& value,
        const std::string& memo) const -> BackgroundTask final;
    auto SendTransfer(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount& value,
        const std::string& memo) const -> BackgroundTask final;
    void StartIntroductionServer(const identifier::Nym& localNymID) const final;
    auto Status(const TaskID taskID) const -> ThreadStatus final;
    auto WithdrawCash(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& account,
        const Amount value) const -> BackgroundTask final;

    OTX(const Flag& running,
        const api::session::Client& client,
        const ContextLockCallback& lockCallback);

    ~OTX() final;

private:
    using TaskStatusMap =
        std::pmr::map<TaskID, std::pair<ThreadStatus, std::promise<Result>>>;
    using ContextID = std::pair<OTNymID, OTServerID>;

    ContextLockCallback lock_callback_;
    const Flag& running_;
    const api::session::Client& api_;
    mutable std::mutex introduction_server_lock_{};
    mutable std::mutex nym_fetch_lock_{};
    mutable std::mutex task_status_lock_{};
    mutable std::atomic<std::uint64_t> refresh_counter_{0};
    mutable std::pmr::map<ContextID, otx::client::implementation::StateMachine>
        operations_;
    mutable std::pmr::map<OTIdentifier, UniqueQueue<OTNymID>> server_nym_fetch_;
    UniqueQueue<otx::client::CheckNymTask> missing_nyms_;
    UniqueQueue<otx::client::CheckNymTask> outdated_nyms_;
    UniqueQueue<OTServerID> missing_servers_;
    UniqueQueue<OTUnitID> missing_unit_definitions_;
    mutable std::unique_ptr<OTServerID> introduction_server_id_;
    mutable TaskStatusMap task_status_;
    mutable std::pmr::map<TaskID, MessageID> task_message_id_;
    OTZMQListenCallback account_subscriber_callback_;
    OTZMQSubscribeSocket account_subscriber_;
    OTZMQListenCallback notification_listener_callback_;
    OTZMQPullSocket notification_listener_;
    OTZMQListenCallback find_nym_callback_;
    OTZMQPullSocket find_nym_listener_;
    OTZMQListenCallback find_server_callback_;
    OTZMQPullSocket find_server_listener_;
    OTZMQListenCallback find_unit_callback_;
    OTZMQPullSocket find_unit_listener_;
    OTZMQPublishSocket task_finished_;
    OTZMQPublishSocket messagability_;
    mutable OTFlag auto_process_inbox_;
    mutable std::atomic<TaskID> next_task_id_;
    mutable std::atomic<bool> shutdown_;
    mutable std::mutex shutdown_lock_;
    mutable OTPasswordPrompt reason_;

    static auto error_task() -> BackgroundTask;
    static auto error_result() -> Result
    {
        return Result{otx::LastReplyStatus::NotSent, nullptr};
    }

    auto add_task(const TaskID taskID, const ThreadStatus status) const
        -> BackgroundTask;
    auto associate_message_id(const Identifier& messageID, const TaskID taskID)
        const -> void final;
    auto can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const Identifier& accountIDHint,
        identifier::Server& depositServer,
        identifier::UnitDefinition& unitID,
        Identifier& depositAccount) const -> Depositability final;
    auto can_message(
        const identifier::Nym& senderNymID,
        const Identifier& recipientContactID,
        identifier::Nym& recipientNymID,
        identifier::Server& serverID) const -> Messagability;
    auto extract_payment_data(
        const OTPayment& payment,
        identifier::Nym& nymID,
        identifier::Server& serverID,
        identifier::UnitDefinition& unitID) const -> bool;
    auto find_nym(const opentxs::network::zeromq::Message& message) const
        -> void;
    auto find_server(const opentxs::network::zeromq::Message& message) const
        -> void;
    auto find_unit(const opentxs::network::zeromq::Message& message) const
        -> void;
    auto finish_task(const TaskID taskID, const bool success, Result&& result)
        const -> bool final;
    auto get_introduction_server(const Lock& lock) const -> OTServerID;
    auto get_nym_fetch(const identifier::Server& serverID) const
        -> UniqueQueue<OTNymID>& final;
    auto get_operations(const ContextID& id) const noexcept(false)
        -> otx::client::implementation::StateMachine&;
    auto get_task(const ContextID& id) const
        -> otx::client::implementation::StateMachine&;
    auto load_introduction_server(const Lock& lock) const -> void;
    auto next_task_id() const -> TaskID { return ++next_task_id_; }
    auto process_account(const opentxs::network::zeromq::Message& message) const
        -> void;
    auto process_notification(
        const opentxs::network::zeromq::Message& message) const -> void;
    auto publish_messagability(
        const identifier::Nym& sender,
        const Identifier& contact,
        Messagability value) const noexcept -> Messagability;
    auto publish_server_registration(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const bool forcePrimary) const -> bool;
    auto queue_cheque_deposit(
        const identifier::Nym& nymID,
        const Cheque& cheque) const -> bool;
    auto refresh_accounts() const -> bool;
    auto refresh_contacts() const -> bool;
    auto schedule_download_nymbox(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const -> BackgroundTask;
    auto schedule_register_account(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& unitID,
        const std::string& label) const -> BackgroundTask;
    auto set_contact(
        const identifier::Nym& nymID,
        const identifier::Server& serverID) const -> void;
    auto set_introduction_server(
        const Lock& lock,
        const contract::Server& contract) const -> OTServerID;
    auto start_task(const TaskID taskID, bool success) const
        -> BackgroundTask final;
    auto status(const Lock& lock, const TaskID taskID) const -> ThreadStatus;
    auto update_task(
        const TaskID taskID,
        const ThreadStatus status,
        Result&& result) const noexcept -> void;
    auto start_introduction_server(const identifier::Nym& nymID) const -> void;
    auto trigger_all() const -> void;
    auto valid_account(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& unitID,
        const Identifier& accountIDHint,
        Identifier& depositAccount) const -> Depositability;
    auto valid_context(
        const identifier::Nym& nymID,
        const identifier::Server& serverID) const -> bool;
    auto valid_recipient(
        const OTPayment& payment,
        const identifier::Nym& specifiedNymID,
        const identifier::Nym& recipient) const -> Depositability;

    OTX() = delete;
    OTX(const OTX&) = delete;
    OTX(OTX&&) = delete;
    auto operator=(const OTX&) -> OTX& = delete;
    auto operator=(OTX&&) -> OTX& = delete;
};
}  // namespace opentxs::api::session::imp
