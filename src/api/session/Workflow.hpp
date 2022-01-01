// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/api/session/Workflow.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "serialization/protobuf/RPCEnums.pb.h"

namespace opentxs
{
namespace api
{
namespace session
{
class Activity;
class Contacts;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind
}  // namespace otx

namespace proto
{
class PaymentWorkflow;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::session::imp
{
class Workflow final : public internal::Workflow, Lockable
{
public:
    auto AbortTransfer(
        const identifier::Nym& nymID,
        const Item& transfer,
        const Message& reply) const -> bool final;
    auto AcceptTransfer(
        const identifier::Nym& nymID,
        const identifier::Server& notaryID,
        const OTTransaction& pending,
        const Message& reply) const -> bool final;
    auto AcknowledgeTransfer(
        const identifier::Nym& nymID,
        const Item& transfer,
        const Message& reply) const -> bool final;
    auto AllocateCash(const identifier::Nym& id, const otx::blind::Purse& purse)
        const -> OTIdentifier final;
    auto CancelCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool final;
    auto ClearCheque(
        const identifier::Nym& recipientNymID,
        const OTTransaction& receipt) const -> bool final;
    auto ClearTransfer(
        const identifier::Nym& nymID,
        const identifier::Server& notaryID,
        const OTTransaction& receipt) const -> bool final;
    auto CompleteTransfer(
        const identifier::Nym& nymID,
        const identifier::Server& notaryID,
        const OTTransaction& receipt,
        const Message& reply) const -> bool final;
    auto ConveyTransfer(
        const identifier::Nym& nymID,
        const identifier::Server& notaryID,
        const OTTransaction& pending) const -> OTIdentifier final;
    auto CreateTransfer(const Item& transfer, const Message& request) const
        -> OTIdentifier final;
    auto DepositCheque(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool final;
    auto ExpireCheque(
        const identifier::Nym& nymID,
        const opentxs::Cheque& cheque) const -> bool final;
    auto ExportCheque(const opentxs::Cheque& cheque) const -> bool final;
    auto FinishCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool final;
    auto ImportCheque(
        const identifier::Nym& nymID,
        const opentxs::Cheque& cheque) const -> OTIdentifier final;
    auto InstantiateCheque(
        const identifier::Nym& nymID,
        const Identifier& workflowID) const -> Cheque final;
    auto InstantiatePurse(
        const identifier::Nym& nymID,
        const Identifier& workflowID) const -> Purse final;
    auto List(
        const identifier::Nym& nymID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState state) const
        -> std::pmr::set<OTIdentifier> final;
    auto LoadCheque(const identifier::Nym& nymID, const Identifier& chequeID)
        const -> Cheque final;
    auto LoadChequeByWorkflow(
        const identifier::Nym& nymID,
        const Identifier& workflowID) const -> Cheque final;
    auto LoadTransfer(
        const identifier::Nym& nymID,
        const Identifier& transferID) const -> Transfer final;
    auto LoadTransferByWorkflow(
        const identifier::Nym& nymID,
        const Identifier& workflowID) const -> Transfer final;
    auto LoadWorkflow(
        const identifier::Nym& nymID,
        const Identifier& workflowID,
        proto::PaymentWorkflow& out) const -> bool final;
    auto ReceiveCash(
        const identifier::Nym& receiver,
        const otx::blind::Purse& purse,
        const Message& message) const -> OTIdentifier final;
    auto ReceiveCheque(
        const identifier::Nym& nymID,
        const opentxs::Cheque& cheque,
        const Message& message) const -> OTIdentifier final;
    auto SendCash(
        const identifier::Nym& sender,
        const identifier::Nym& recipient,
        const Identifier& workflowID,
        const Message& request,
        const Message* reply) const -> bool final;
    auto SendCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool final;
    auto WorkflowParty(
        const identifier::Nym& nymID,
        const Identifier& workflowID,
        const int index) const -> const std::string final;
    auto WorkflowPartySize(
        const identifier::Nym& nymID,
        const Identifier& workflowID,
        int& partysize) const -> bool final;
    auto WorkflowState(
        const identifier::Nym& nymID,
        const Identifier& workflowID) const
        -> otx::client::PaymentWorkflowState final;
    auto WorkflowType(
        const identifier::Nym& nymID,
        const Identifier& workflowID) const
        -> otx::client::PaymentWorkflowType final;
    auto WorkflowsByAccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const
        -> std::pmr::vector<OTIdentifier> final;
    auto WriteCheque(const opentxs::Cheque& cheque) const -> OTIdentifier final;

    Workflow(
        const api::Session& api,
        const session::Activity& activity,
        const session::Contacts& contact);

    ~Workflow() final = default;

private:
    struct ProtobufVersions {
        VersionNumber event_;
        VersionNumber source_;
        VersionNumber workflow_;
    };

    using VersionMap =
        std::pmr::map<otx::client::PaymentWorkflowType, ProtobufVersions>;

    static const VersionMap versions_;

    const api::Session& api_;
    const session::Activity& activity_;
    const session::Contacts& contact_;
    const OTZMQPublishSocket account_publisher_;
    const OTZMQPushSocket rpc_publisher_;
    mutable std::pmr::map<std::string, std::shared_mutex> workflow_locks_;

    static auto can_abort_transfer(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_accept_cheque(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_accept_transfer(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_acknowledge_transfer(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_cancel_cheque(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_clear_transfer(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_complete_transfer(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_convey_cash(const proto::PaymentWorkflow& workflow) -> bool;
    static auto can_convey_cheque(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_convey_transfer(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_deposit_cheque(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto can_expire_cheque(
        const opentxs::Cheque& cheque,
        const proto::PaymentWorkflow& workflow) -> bool;
    static auto can_finish_cheque(const proto::PaymentWorkflow& workflow)
        -> bool;
    static auto cheque_deposit_success(const Message* message) -> bool;
    static auto extract_conveyed_time(const proto::PaymentWorkflow& workflow)
        -> Time;
    static auto isCheque(const opentxs::Cheque& cheque) -> bool;
    static auto isTransfer(const Item& item) -> bool;
    static auto validate_recipient(
        const identifier::Nym& nymID,
        const opentxs::Cheque& cheque) -> bool;

    auto add_cheque_event(
        const eLock& lock,
        const std::string& nymID,
        const std::string& eventNym,
        proto::PaymentWorkflow& workflow,
        const otx::client::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const VersionNumber version,
        const Message& request,
        const Message* reply,
        const Identifier& account) const -> bool;
    auto add_cheque_event(
        const eLock& lock,
        const std::string& nymID,
        const Identifier& accountID,
        proto::PaymentWorkflow& workflow,
        const otx::client::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const VersionNumber version,
        const identifier::Nym& recipientNymID,
        const OTTransaction& receipt,
        const Time time = Clock::now()) const -> bool;
    auto add_transfer_event(
        const eLock& lock,
        const std::string& nymID,
        const std::string& eventNym,
        proto::PaymentWorkflow& workflow,
        const otx::client::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const VersionNumber version,
        const Message& message,
        const Identifier& account,
        const bool success) const -> bool;
    auto add_transfer_event(
        const eLock& lock,
        const std::string& nymID,
        const std::string& notaryID,
        const std::string& eventNym,
        proto::PaymentWorkflow& workflow,
        const otx::client::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const VersionNumber version,
        const OTTransaction& receipt,
        const Identifier& account,
        const bool success) const -> bool;
    auto convey_incoming_transfer(
        const identifier::Nym& nymID,
        const identifier::Server& notaryID,
        const OTTransaction& pending,
        const std::string& senderNymID,
        const std::string& recipientNymID,
        const Item& transfer) const -> OTIdentifier;
    auto convey_internal_transfer(
        const identifier::Nym& nymID,
        const identifier::Server& notaryID,
        const OTTransaction& pending,
        const std::string& senderNymID,
        const Item& transfer) const -> OTIdentifier;
    auto create_cheque(
        const Lock& global,
        const std::string& nymID,
        const opentxs::Cheque& cheque,
        const otx::client::PaymentWorkflowType workflowType,
        const otx::client::PaymentWorkflowState workflowState,
        const VersionNumber workflowVersion,
        const VersionNumber sourceVersion,
        const VersionNumber eventVersion,
        const std::string& party,
        const Identifier& account,
        const Message* message = nullptr) const
        -> std::pair<OTIdentifier, proto::PaymentWorkflow>;
    auto create_transfer(
        const Lock& global,
        const std::string& nymID,
        const Item& transfer,
        const otx::client::PaymentWorkflowType workflowType,
        const otx::client::PaymentWorkflowState workflowState,
        const VersionNumber workflowVersion,
        const VersionNumber sourceVersion,
        const VersionNumber eventVersion,
        const std::string& party,
        const Identifier& account,
        const std::string& notaryID,
        const std::string& destinationAccountID) const
        -> std::pair<OTIdentifier, proto::PaymentWorkflow>;
    auto extract_transfer_from_pending(const OTTransaction& receipt) const
        -> std::unique_ptr<Item>;
    auto extract_transfer_from_receipt(
        const OTTransaction& receipt,
        Identifier& depositorNymID) const -> std::unique_ptr<Item>;
    template <typename T>
    auto get_workflow(
        const Lock& global,
        const std::pmr::set<otx::client::PaymentWorkflowType>& types,
        const std::string& nymID,
        const T& source) const -> std::shared_ptr<proto::PaymentWorkflow>;
    auto get_workflow_by_id(
        const std::pmr::set<otx::client::PaymentWorkflowType>& types,
        const std::string& nymID,
        const std::string& workflowID) const
        -> std::shared_ptr<proto::PaymentWorkflow>;
    auto get_workflow_by_id(
        const std::string& nymID,
        const std::string& workflowID) const
        -> std::shared_ptr<proto::PaymentWorkflow>;
    auto get_workflow_by_source(
        const std::pmr::set<otx::client::PaymentWorkflowType>& types,
        const std::string& nymID,
        const std::string& sourceID) const
        -> std::shared_ptr<proto::PaymentWorkflow>;
    // Unlocks global after successfully locking the workflow-specific mutex
    auto get_workflow_lock(Lock& global, const std::string& id) const -> eLock;
    auto isInternalTransfer(
        const Identifier& sourceAccount,
        const Identifier& destinationAccount) const -> bool;
    auto save_workflow(
        const std::string& nymID,
        const proto::PaymentWorkflow& workflow) const -> bool;
    auto save_workflow(
        const std::string& nymID,
        const Identifier& accountID,
        const proto::PaymentWorkflow& workflow) const -> bool;
    auto save_workflow(
        OTIdentifier&& workflowID,
        const std::string& nymID,
        const Identifier& accountID,
        const proto::PaymentWorkflow& workflow) const -> OTIdentifier;
    auto save_workflow(
        std::pair<OTIdentifier, proto::PaymentWorkflow>&& workflowID,
        const std::string& nymID,
        const Identifier& accountID,
        const proto::PaymentWorkflow& workflow) const
        -> std::pair<OTIdentifier, proto::PaymentWorkflow>;
    auto update_activity(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID,
        const Identifier& sourceID,
        const Identifier& workflowID,
        const StorageBox type,
        Time time) const -> bool;
    void update_rpc(
        const std::string& localNymID,
        const std::string& remoteNymID,
        const std::string& accountID,
        const proto::AccountEventType type,
        const std::string& workflowID,
        const Amount amount,
        const Amount pending,
        const Time time,
        const std::string& memo) const;

    Workflow() = delete;
    Workflow(const Workflow&) = delete;
    Workflow(Workflow&&) = delete;
    auto operator=(const Workflow&) -> Workflow& = delete;
    auto operator=(Workflow&&) -> Workflow& = delete;
};
}  // namespace opentxs::api::session::imp
