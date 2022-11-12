// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::otx::client::PaymentWorkflowState
// IWYU pragma: no_forward_declare opentxs::otx::client::PaymentWorkflowType
// IWYU pragma: no_forward_declare opentxs::otx::client::StorageBox
// IWYU pragma: no_forward_declare opentxs::proto::AccountEventType
// IWYU pragma: no_forward_declare opentxs::proto::PaymentEventType

#pragma once

#include <PaymentWorkflowEnums.pb.h>
#include <RPCEnums.pb.h>
#include <chrono>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <utility>

#include "internal/api/session/Workflow.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Notary;
class Nym;
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
// NOLINTEND(modernize-concat-nested-namespaces)

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
        const identifier::Notary& notaryID,
        const OTTransaction& pending,
        const Message& reply) const -> bool final;
    auto AcknowledgeTransfer(
        const identifier::Nym& nymID,
        const Item& transfer,
        const Message& reply) const -> bool final;
    auto AllocateCash(const identifier::Nym& id, const otx::blind::Purse& purse)
        const -> identifier::Generic final;
    auto CancelCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool final;
    auto ClearCheque(
        const identifier::Nym& recipientNymID,
        const OTTransaction& receipt) const -> bool final;
    auto ClearTransfer(
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const OTTransaction& receipt) const -> bool final;
    auto CompleteTransfer(
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const OTTransaction& receipt,
        const Message& reply) const -> bool final;
    auto ConveyTransfer(
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const OTTransaction& pending) const -> identifier::Generic final;
    auto CreateTransfer(const Item& transfer, const Message& request) const
        -> identifier::Generic final;
    auto DepositCheque(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
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
        const opentxs::Cheque& cheque) const -> identifier::Generic final;
    auto InstantiateCheque(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const -> Cheque final;
    auto InstantiatePurse(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const -> Purse final;
    auto List(
        const identifier::Nym& nymID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState state) const
        -> UnallocatedSet<identifier::Generic> final;
    auto LoadCheque(
        const identifier::Nym& nymID,
        const identifier::Generic& chequeID) const -> Cheque final;
    auto LoadChequeByWorkflow(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const -> Cheque final;
    auto LoadTransfer(
        const identifier::Nym& nymID,
        const identifier::Generic& transferID) const -> Transfer final;
    auto LoadTransferByWorkflow(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const -> Transfer final;
    auto LoadWorkflow(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID,
        proto::PaymentWorkflow& out) const -> bool final;
    auto ReceiveCash(
        const identifier::Nym& receiver,
        const otx::blind::Purse& purse,
        const Message& message) const -> identifier::Generic final;
    auto ReceiveCheque(
        const identifier::Nym& nymID,
        const opentxs::Cheque& cheque,
        const Message& message) const -> identifier::Generic final;
    auto SendCash(
        const identifier::Nym& sender,
        const identifier::Nym& recipient,
        const identifier::Generic& workflowID,
        const Message& request,
        const Message* reply) const -> bool final;
    auto SendCheque(
        const opentxs::Cheque& cheque,
        const Message& request,
        const Message* reply) const -> bool final;
    auto WorkflowParty(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID,
        const int index) const -> const UnallocatedCString final;
    auto WorkflowPartySize(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID,
        int& partysize) const -> bool final;
    auto WorkflowState(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const
        -> otx::client::PaymentWorkflowState final;
    auto WorkflowType(
        const identifier::Nym& nymID,
        const identifier::Generic& workflowID) const
        -> otx::client::PaymentWorkflowType final;
    auto WorkflowsByAccount(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID) const
        -> UnallocatedVector<identifier::Generic> final;
    auto WriteCheque(const opentxs::Cheque& cheque) const
        -> identifier::Generic final;

    Workflow(
        const api::Session& api,
        const session::Activity& activity,
        const session::Contacts& contact);
    Workflow() = delete;
    Workflow(const Workflow&) = delete;
    Workflow(Workflow&&) = delete;
    auto operator=(const Workflow&) -> Workflow& = delete;
    auto operator=(Workflow&&) -> Workflow& = delete;

    ~Workflow() final = default;

private:
    struct ProtobufVersions {
        VersionNumber event_;
        VersionNumber source_;
        VersionNumber workflow_;
    };

    using VersionMap =
        UnallocatedMap<otx::client::PaymentWorkflowType, ProtobufVersions>;

    static const VersionMap versions_;

    const api::Session& api_;
    const session::Activity& activity_;
    const session::Contacts& contact_;
    const OTZMQPublishSocket account_publisher_;
    const OTZMQPushSocket rpc_publisher_;
    mutable UnallocatedMap<UnallocatedCString, std::shared_mutex>
        workflow_locks_;

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
        const identifier::Nym& nymID,
        const identifier::Nym& eventNym,
        proto::PaymentWorkflow& workflow,
        const otx::client::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const VersionNumber version,
        const Message& request,
        const Message* reply,
        const identifier::Generic& account) const -> bool;
    auto add_cheque_event(
        const eLock& lock,
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        proto::PaymentWorkflow& workflow,
        const otx::client::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const VersionNumber version,
        const identifier::Nym& recipientNymID,
        const OTTransaction& receipt,
        const Time time = Clock::now()) const -> bool;
    auto add_transfer_event(
        const eLock& lock,
        const identifier::Nym& nymID,
        const identifier::Nym& eventNym,
        proto::PaymentWorkflow& workflow,
        const otx::client::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const VersionNumber version,
        const Message& message,
        const identifier::Generic& account,
        const bool success) const -> bool;
    auto add_transfer_event(
        const eLock& lock,
        const identifier::Nym& nymID,
        const UnallocatedCString& notaryID,
        const identifier::Nym& eventNym,
        proto::PaymentWorkflow& workflow,
        const otx::client::PaymentWorkflowState newState,
        const proto::PaymentEventType newEventType,
        const VersionNumber version,
        const OTTransaction& receipt,
        const identifier::Generic& account,
        const bool success) const -> bool;
    auto convey_incoming_transfer(
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const OTTransaction& pending,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const Item& transfer) const -> identifier::Generic;
    auto convey_internal_transfer(
        const identifier::Nym& nymID,
        const identifier::Notary& notaryID,
        const OTTransaction& pending,
        const identifier::Nym& senderNymID,
        const Item& transfer) const -> identifier::Generic;
    auto create_cheque(
        const Lock& global,
        const identifier::Nym& nymID,
        const opentxs::Cheque& cheque,
        const otx::client::PaymentWorkflowType workflowType,
        const otx::client::PaymentWorkflowState workflowState,
        const VersionNumber workflowVersion,
        const VersionNumber sourceVersion,
        const VersionNumber eventVersion,
        const identifier::Nym& party,
        const identifier::Generic& account,
        const Message* message = nullptr) const
        -> std::pair<identifier::Generic, proto::PaymentWorkflow>;
    auto create_transfer(
        const Lock& global,
        const identifier::Nym& nymID,
        const Item& transfer,
        const otx::client::PaymentWorkflowType workflowType,
        const otx::client::PaymentWorkflowState workflowState,
        const VersionNumber workflowVersion,
        const VersionNumber sourceVersion,
        const VersionNumber eventVersion,
        const identifier::Nym& party,
        const identifier::Generic& account,
        const UnallocatedCString& notaryID,
        const UnallocatedCString& destinationAccountID) const
        -> std::pair<identifier::Generic, proto::PaymentWorkflow>;
    auto extract_transfer_from_pending(const OTTransaction& receipt) const
        -> std::unique_ptr<Item>;
    auto extract_transfer_from_receipt(
        const OTTransaction& receipt,
        identifier::Nym& depositorNymID) const -> std::unique_ptr<Item>;
    template <typename T>
    auto get_workflow(
        const Lock& global,
        const UnallocatedSet<otx::client::PaymentWorkflowType>& types,
        const identifier::Nym& nymID,
        const T& source) const -> std::shared_ptr<proto::PaymentWorkflow>;
    auto get_workflow_by_id(
        const UnallocatedSet<otx::client::PaymentWorkflowType>& types,
        const identifier::Nym& nymID,
        const UnallocatedCString& workflowID) const
        -> std::shared_ptr<proto::PaymentWorkflow>;
    auto get_workflow_by_id(
        const identifier::Nym& nymID,
        const UnallocatedCString& workflowID) const
        -> std::shared_ptr<proto::PaymentWorkflow>;
    auto get_workflow_by_source(
        const UnallocatedSet<otx::client::PaymentWorkflowType>& types,
        const identifier::Nym& nymID,
        const UnallocatedCString& sourceID) const
        -> std::shared_ptr<proto::PaymentWorkflow>;
    // Unlocks global after successfully locking the workflow-specific mutex
    auto get_workflow_lock(Lock& global, const UnallocatedCString& id) const
        -> eLock;
    auto isInternalTransfer(
        const identifier::Generic& sourceAccount,
        const identifier::Generic& destinationAccount) const -> bool;
    auto save_workflow(
        const identifier::Nym& nymID,
        const proto::PaymentWorkflow& workflow) const -> bool;
    auto save_workflow(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const proto::PaymentWorkflow& workflow) const -> bool;
    auto save_workflow(
        identifier::Generic&& workflowID,
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const proto::PaymentWorkflow& workflow) const -> identifier::Generic;
    auto save_workflow(
        std::pair<identifier::Generic, proto::PaymentWorkflow>&& workflowID,
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const proto::PaymentWorkflow& workflow) const
        -> std::pair<identifier::Generic, proto::PaymentWorkflow>;
    auto update_activity(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID,
        const identifier::Generic& sourceID,
        const identifier::Generic& workflowID,
        const otx::client::StorageBox type,
        Time time) const -> bool;
    void update_rpc(
        const identifier::Nym& localNymID,
        const identifier::Nym& remoteNymID,
        const UnallocatedCString& accountID,
        const proto::AccountEventType type,
        const UnallocatedCString& workflowID,
        const Amount amount,
        const Amount pending,
        const Time time,
        const UnallocatedCString& memo) const;
};
}  // namespace opentxs::api::session::imp
