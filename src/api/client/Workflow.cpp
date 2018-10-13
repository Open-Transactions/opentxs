// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"
#include "opentxs/Proto.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>

#include "Workflow.hpp"

#define OUTGOING_CHEQUE_EVENT_VERSION 1
#define OUTGOING_CHEQUE_SOURCE_VERSION 1
#define OUTGOING_CHEQUE_WORKFLOW_VERSION 1
#define INCOMING_CHEQUE_EVENT_VERSION 1
#define INCOMING_CHEQUE_SOURCE_VERSION 1
#define INCOMING_CHEQUE_WORKFLOW_VERSION 1
#define OUTGOING_TRANSFER_EVENT_VERSION 2
#define OUTGOING_TRANSFER_SOURCE_VERSION 1
#define OUTGOING_TRANSFER_WORKFLOW_VERSION 2
#define INCOMING_TRANSFER_EVENT_VERSION 2
#define INCOMING_TRANSFER_SOURCE_VERSION 1
#define INCOMING_TRANSFER_WORKFLOW_VERSION 2
#define INTERNAL_TRANSFER_EVENT_VERSION 2
#define INTERNAL_TRANSFER_SOURCE_VERSION 1
#define INTERNAL_TRANSFER_WORKFLOW_VERSION 2
#define RPC_ACCOUNT_EVENT_VERSION 1
#define RPC_PUSH_VERSION 1

#define OT_METHOD "opentxs::api::client::implementation::Workflow::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
api::client::Workflow* Factory::Workflow(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contact)
{
    return new api::client::implementation::Workflow(api, activity, contact);
}
}  // namespace opentxs

namespace opentxs::api::client
{
bool Workflow::ContainsCheque(const proto::PaymentWorkflow& workflow)
{
    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {

            return true;
        } break;
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        default: {
        }
    }

    return false;
}

bool Workflow::ContainsTransfer(const proto::PaymentWorkflow& workflow)
{
    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {

            return true;
        } break;
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        default: {
        }
    }

    return false;
}

std::string Workflow::ExtractCheque(const proto::PaymentWorkflow& workflow)
{
    if (false == ContainsCheque(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong workflow type").Flush();

        return {};
    }

    if (1 != workflow.source().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid workflow").Flush();

        return {};
    }

    return workflow.source(0).item();
}

std::string Workflow::ExtractTransfer(const proto::PaymentWorkflow& workflow)
{
    if (false == ContainsTransfer(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong workflow type").Flush();

        return {};
    }

    if (1 != workflow.source().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid workflow").Flush();

        return {};
    }

    return workflow.source(0).item();
}

Workflow::Cheque Workflow::InstantiateCheque(
    const api::Core& core,
    const proto::PaymentWorkflow& workflow)
{
    Cheque output{proto::PAYMENTWORKFLOWSTATE_ERROR, nullptr};
    auto& [state, cheque] = output;

    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            cheque.reset(core.Factory().Cheque().release());

            OT_ASSERT(cheque)

            const auto serialized = ExtractCheque(workflow);

            if (serialized.empty()) { return output; }

            const auto loaded = cheque->LoadContractFromString(
                String::Factory(serialized.c_str()));

            if (false == loaded) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to instantiate cheque")
                    .Flush();
                cheque.reset();

                return output;
            }

            state = workflow.state();
        } break;
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow type")
                .Flush();
        }
    }

    return output;
}

Workflow::Transfer Workflow::InstantiateTransfer(
    const api::Core& core,
    const proto::PaymentWorkflow& workflow)
{
    Transfer output{proto::PAYMENTWORKFLOWSTATE_ERROR, nullptr};
    auto& [state, transfer] = output;

    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
            const auto serialized = ExtractTransfer(workflow);

            if (serialized.empty()) { return output; }

            transfer.reset(core.Factory().Item(serialized).release());

            if (false == bool(transfer)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to instantiate transfer")
                    .Flush();
                transfer.reset();

                return output;
            }

            state = workflow.state();
        } break;

        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow type")
                .Flush();
        }
    }

    return output;
}

namespace implementation
{
Workflow::Workflow(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contact)
    : api_(api)
    , activity_(activity)
    , contact_(contact)
    , account_publisher_(api_.ZeroMQ().PublishSocket())
    , rpc_publisher_(api_.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
{
    // WARNING: do not access api_.Wallet() during construction
    const auto endpoint = api_.Endpoints().WorkflowAccountUpdate();
    LogDetail(OT_METHOD)(__FUNCTION__)(": Binding to ")(endpoint).Flush();
    auto bound = account_publisher_->Start(endpoint);

    OT_ASSERT(bound)

    bound = rpc_publisher_->Start(
        api_.ZeroMQ().BuildEndpoint("rpc/push/internal", -1, 1));

    OT_ASSERT(bound)
}

bool Workflow::AbortTransfer(
    const Identifier& nymID,
    const Item& transfer,
    const Message& reply) const
{
    if (false == isTransfer(transfer)) { return false; }

    const bool isInternal = isInternalTransfer(
        transfer.GetRealAccountID(), transfer.GetDestinationAcctID());
    const std::set<proto::PaymentWorkflowType> type{
        isInternal ? proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER
                   : proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), transfer);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_abort_transfer(*workflow)) { return false; }

    return add_transfer_event(
        lock,
        nymID.str(),
        "",
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_ABORTED,
        proto::PAYMENTEVENTTYPE_ABORT,
        (isInternal ? INTERNAL_TRANSFER_EVENT_VERSION
                    : OUTGOING_TRANSFER_EVENT_VERSION),
        reply,
        transfer.GetRealAccountID().str(),
        true);

    return false;
}

// Works for Incoming and Internal transfer workflows.
bool Workflow::AcceptTransfer(
    const Identifier& nymID,
    const Identifier& notaryID,
    const OTTransaction& pending,
    const Message& reply) const
{
    const auto transfer = extract_transfer_from_pending(pending);

    if (false == bool(transfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction").Flush();

        return false;
    }

    const auto senderNymID = transfer->GetNymID().str();
    const auto recipientNymID = pending.GetNymID().str();
    const auto& accountID = pending.GetPurportedAccountID();

    if (pending.GetNymID() != nymID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid recipient").Flush();

        return false;
    }

    const bool isInternal = (0 == senderNymID.compare(recipientNymID));

    // Ignore this event for internal transfers.
    if (isInternal) { return true; }

    const std::set<proto::PaymentWorkflowType> type{
        proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), *transfer);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_accept_transfer(*workflow)) { return false; }

    return add_transfer_event(
        lock,
        nymID.str(),
        senderNymID,
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_COMPLETED,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        OUTGOING_TRANSFER_EVENT_VERSION,
        reply,
        accountID.str(),
        true);
}

bool Workflow::AcknowledgeTransfer(
    const Identifier& nymID,
    const Item& transfer,
    const Message& reply) const
{
    if (false == isTransfer(transfer)) { return false; }

    const bool isInternal = isInternalTransfer(
        transfer.GetRealAccountID(), transfer.GetDestinationAcctID());
    const std::set<proto::PaymentWorkflowType> type{
        isInternal ? proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER
                   : proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), transfer);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_acknowledge_transfer(*workflow)) { return false; }

    // For internal transfers it's possible that a push notification already
    // advanced the state to conveyed before the sender received the
    // acknowledgement. The timing of those two events is indeterminate,
    // therefore if the state has already advanced, add the acknowledge event
    // but do not change the state.
    const proto::PaymentWorkflowState state =
        (proto::PAYMENTWORKFLOWSTATE_CONVEYED == workflow->state())
            ? proto::PAYMENTWORKFLOWSTATE_CONVEYED
            : proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED;

    return add_transfer_event(
        lock,
        nymID.str(),
        "",
        *workflow,
        state,
        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
        (isInternal ? INTERNAL_TRANSFER_EVENT_VERSION
                    : OUTGOING_TRANSFER_EVENT_VERSION),
        reply,
        transfer.GetRealAccountID().str(),
        true);
}

bool Workflow::add_cheque_event(
    const eLock& lock,
    const std::string& nymID,
    const std::string&,
    proto::PaymentWorkflow& workflow,
    const proto::PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const std::uint32_t version,
    const Message& request,
    const Message* reply,
    const std::string& account) const
{
    const bool haveReply = (nullptr != reply);
    const bool success = cheque_deposit_success(reply);

    if (success) {
        workflow.set_state(newState);

        if ((false == account.empty()) && (0 == workflow.account_size())) {
            workflow.add_account(account);
        }
    }

    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(String::Factory(request)->Get());
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(request.m_strNotaryID->Get());

    switch (newEventType) {
        case proto::PAYMENTEVENTTYPE_CANCEL:
        case proto::PAYMENTEVENTTYPE_COMPLETE: {
        } break;
        case proto::PAYMENTEVENTTYPE_CONVEY:
        case proto::PAYMENTEVENTTYPE_ACCEPT: {
            event.set_nym(request.m_strNymID2->Get());
        } break;
        case proto::PAYMENTEVENTTYPE_ERROR:
        case proto::PAYMENTEVENTTYPE_CREATE:
        default: {
            OT_FAIL
        }
    }

    event.set_success(success);

    if (haveReply) {
        event.add_item(String::Factory(*reply)->Get());
        event.set_time(reply->m_lTime);
    } else {
        event.set_time(request.m_lTime);
    }

    if (false == account.empty()) {
        workflow.set_notary(
            api_.Storage().AccountServer(Identifier::Factory(account))->str());
    }

    return save_workflow(nymID, account, workflow);
}

// Only used for ClearCheque
bool Workflow::add_cheque_event(
    const eLock& lock,
    const std::string& nymID,
    const std::string& accountID,
    proto::PaymentWorkflow& workflow,
    const proto::PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const std::uint32_t version,
    const Identifier& recipientNymID,
    const OTTransaction& receipt,
    const std::chrono::time_point<std::chrono::system_clock> time) const
{
    auto message = String::Factory();
    receipt.SaveContractRaw(message);
    workflow.set_state(newState);
    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(message->Get());
    event.set_time(std::chrono::system_clock::to_time_t(time));
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(receipt.GetRealNotaryID().str());
    event.set_nym(recipientNymID.str());
    event.set_success(true);

    if (0 == workflow.party_size()) {
        workflow.add_party(recipientNymID.str());
    }

    return save_workflow(nymID, accountID, workflow);
}

bool Workflow::add_transfer_event(
    const eLock& lock,
    const std::string& nymID,
    const std::string& eventNym,
    proto::PaymentWorkflow& workflow,
    const proto::PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const std::uint32_t version,
    const Message& message,
    const std::string& account,
    const bool success) const
{
    if (success) { workflow.set_state(newState); }

    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(String::Factory(message)->Get());
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(message.m_strNotaryID->Get());

    switch (newEventType) {
        case proto::PAYMENTEVENTTYPE_CONVEY:
        case proto::PAYMENTEVENTTYPE_ACCEPT:
        case proto::PAYMENTEVENTTYPE_COMPLETE:
        case proto::PAYMENTEVENTTYPE_ABORT:
        case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE: {
            // TODO
        } break;
        case proto::PAYMENTEVENTTYPE_ERROR:
        case proto::PAYMENTEVENTTYPE_CREATE:
        case proto::PAYMENTEVENTTYPE_CANCEL:
        default: {
            OT_FAIL
        }
    }

    event.set_success(success);
    event.set_time(message.m_lTime);

    if (0 == workflow.party_size() && (false == eventNym.empty())) {
        workflow.add_party(eventNym);
    }

    return save_workflow(nymID, account, workflow);
}

bool Workflow::add_transfer_event(
    const eLock& lock,
    const std::string& nymID,
    const std::string& notaryID,
    const std::string& eventNym,
    proto::PaymentWorkflow& workflow,
    const proto::PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const std::uint32_t version,
    const OTTransaction& receipt,
    const std::string& account,
    const bool success) const
{
    if (success) { workflow.set_state(newState); }

    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(String::Factory(receipt)->Get());
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(notaryID);

    switch (newEventType) {
        case proto::PAYMENTEVENTTYPE_CONVEY:
        case proto::PAYMENTEVENTTYPE_ACCEPT:
        case proto::PAYMENTEVENTTYPE_COMPLETE:
        case proto::PAYMENTEVENTTYPE_ABORT:
        case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE: {
            // TODO
        } break;
        case proto::PAYMENTEVENTTYPE_ERROR:
        case proto::PAYMENTEVENTTYPE_CREATE:
        case proto::PAYMENTEVENTTYPE_CANCEL:
        default: {
            OT_FAIL
        }
    }

    event.set_success(success);
    event.set_time(now());

    if (0 == workflow.party_size() && (false == eventNym.empty())) {
        workflow.add_party(eventNym);
    }

    return save_workflow(nymID, account, workflow);
}

bool Workflow::can_abort_transfer(const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.state()) {
        case proto::PAYMENTWORKFLOWSTATE_INITIATED: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_accept_cheque(const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.state()) {
        case proto::PAYMENTWORKFLOWSTATE_EXPIRED:
        case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_accept_transfer(const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.state()) {
        case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_acknowledge_transfer(const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.state()) {
        case proto::PAYMENTWORKFLOWSTATE_INITIATED:
        case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state (")(
            workflow.state())(")")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_cancel_cheque(const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.state()) {
        case proto::PAYMENTWORKFLOWSTATE_UNSENT:
        case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_clear_transfer(const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER: {
            correctState =
                (proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED == workflow.state());
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
            correctState =
                (proto::PAYMENTWORKFLOWSTATE_CONVEYED == workflow.state());
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_complete_transfer(const proto::PaymentWorkflow& workflow)
{
    if (proto::PAYMENTWORKFLOWSTATE_ACCEPTED != workflow.state()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state (")(
            workflow.state())(")")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_convey_cheque(const proto::PaymentWorkflow& workflow)
{
    if (proto::PAYMENTWORKFLOWSTATE_UNSENT != workflow.state()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_convey_transfer(const proto::PaymentWorkflow& workflow)
{
    switch (workflow.state()) {
        case proto::PAYMENTWORKFLOWSTATE_INITIATED:
        case proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED: {
            return true;
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
                .Flush();

            return false;
        }
    }
}

bool Workflow::can_deposit_cheque(const proto::PaymentWorkflow& workflow)
{
    if (proto::PAYMENTWORKFLOWSTATE_CONVEYED != workflow.state()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_expire_cheque(
    const opentxs::Cheque& cheque,
    const proto::PaymentWorkflow& workflow)
{
    bool correctState{false};

    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_UNSENT:
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
                    correctState = true;
                } break;
                default: {
                }
            }
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED: {
                    correctState = true;
                } break;
                default: {
                }
            }
        } break;
        default: {
            OT_FAIL
        }
    }

    if (false == correctState) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
            .Flush();

        return false;
    }

    if (now() < cheque.GetValidTo()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Can not expire valid cheque.")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::can_finish_cheque(const proto::PaymentWorkflow& workflow)
{
    if (proto::PAYMENTWORKFLOWSTATE_ACCEPTED != workflow.state()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::CancelCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE}, nymID, cheque);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_cancel_cheque(*workflow)) { return false; }

    return add_cheque_event(
        lock,
        nymID,
        "",
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_CANCELLED,
        proto::PAYMENTEVENTTYPE_CANCEL,
        OUTGOING_CHEQUE_EVENT_VERSION,
        request,
        reply);
}

bool Workflow::cheque_deposit_success(const Message* message)
{
    if (nullptr == message) { return false; }

    // TODO this might not be sufficient

    return message->m_bSuccess;
}

bool Workflow::ClearCheque(
    const Identifier& recipientNymID,
    const OTTransaction& receipt) const
{
    if (recipientNymID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid cheque recipient")
            .Flush();

        return false;
    }

    auto cheque{api_.Factory().Cheque(receipt)};

    if (false == bool(cheque)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load cheque from receipt.")
            .Flush();

        return false;
    }

    if (false == isCheque(*cheque)) { return false; }

    const auto nymID = cheque->GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE}, nymID, *cheque);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_accept_cheque(*workflow)) { return false; }

    OT_ASSERT(1 == workflow->account_size())

    const bool needNym = (0 == workflow->party_size());
    const auto time = std::chrono::system_clock::from_time_t(now());
    const auto output = add_cheque_event(
        lock,
        nymID,
        workflow->account(0),
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_ACCEPTED,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        OUTGOING_CHEQUE_EVENT_VERSION,
        recipientNymID,
        receipt,
        time);

    if (needNym) {
        update_activity(
            cheque->GetSenderNymID(),
            recipientNymID,
            Identifier::Factory(*cheque),
            Identifier::Factory(workflow->id()),
            StorageBox::OUTGOINGCHEQUE,
            extract_conveyed_time(*workflow));
    }

    update_rpc(
        nymID,
        cheque->GetRecipientNymID().str(),
        cheque->SourceAccountID().str(),
        proto::ACCOUNTEVENT_OUTGOINGCHEQUE,
        workflow->id(),
        -1 * cheque->GetAmount(),
        0,
        time,
        cheque->GetMemo().Get());

    return output;
}

bool Workflow::ClearTransfer(
    const Identifier& nymID,
    const Identifier& notaryID,
    const OTTransaction& receipt) const
{
    auto depositorNymID = Identifier::Factory();
    const auto transfer =
        extract_transfer_from_receipt(receipt, depositorNymID);

    if (false == bool(transfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer").Flush();

        return false;
    }

    if (depositorNymID->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing recipient").Flush();

        return false;
    }

    const auto& accountID = transfer->GetPurportedAccountID();

    if (accountID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Transfer does not contain source account ID")
            .Flush();

        return false;
    }

    const auto& destinationAccountID = transfer->GetDestinationAcctID();

    if (destinationAccountID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Transfer does not contain destination account ID")
            .Flush();

        return false;
    }

    const bool isInternal = isInternalTransfer(accountID, destinationAccountID);
    const std::set<proto::PaymentWorkflowType> type{
        isInternal ? proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER
                   : proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), *transfer);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_clear_transfer(*workflow)) { return false; }

    const auto output = add_transfer_event(
        lock,
        nymID.str(),
        notaryID.str(),
        (isInternal ? std::string{""} : depositorNymID->str()),
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_ACCEPTED,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        (isInternal ? INTERNAL_TRANSFER_EVENT_VERSION
                    : OUTGOING_TRANSFER_EVENT_VERSION),
        receipt,
        accountID.str(),
        true);

    if (output) {
        const auto time = extract_conveyed_time(*workflow);
        auto note = String::Factory();
        transfer->GetNote(note);
        update_activity(
            nymID,
            depositorNymID,
            Identifier::Factory(*transfer),
            Identifier::Factory(workflow->id()),
            StorageBox::OUTGOINGTRANSFER,
            time);
        update_rpc(
            nymID.str(),
            depositorNymID->str(),
            accountID.str(),
            proto::ACCOUNTEVENT_OUTGOINGTRANSFER,
            workflow->id(),
            transfer->GetAmount(),
            0,
            time,
            note->Get());
    }

    return output;
}

// Works for outgoing and internal transfer workflows.
bool Workflow::CompleteTransfer(
    const Identifier& nymID,
    const Identifier& notaryID,
    const OTTransaction& receipt,
    const Message& reply) const
{
    auto depositorNymID = Identifier::Factory();
    const auto transfer =
        extract_transfer_from_receipt(receipt, depositorNymID);

    if (false == bool(transfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer").Flush();

        return false;
    }

    const auto& accountID = transfer->GetPurportedAccountID();

    if (accountID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Transfer does not contain source account ID")
            .Flush();

        return false;
    }

    const auto& destinationAccountID = transfer->GetDestinationAcctID();

    if (destinationAccountID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Transfer does not contain destination account ID")
            .Flush();

        return false;
    }

    const bool isInternal = isInternalTransfer(accountID, destinationAccountID);
    const std::set<proto::PaymentWorkflowType> type{
        isInternal ? proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER
                   : proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), *transfer);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_complete_transfer(*workflow)) { return false; }

    return add_transfer_event(
        lock,
        nymID.str(),
        notaryID.str(),
        (isInternal ? std::string{""} : depositorNymID->str()),
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_COMPLETED,
        proto::PAYMENTEVENTTYPE_COMPLETE,
        (isInternal ? INTERNAL_TRANSFER_EVENT_VERSION
                    : OUTGOING_TRANSFER_EVENT_VERSION),
        receipt,
        transfer->GetRealAccountID().str(),
        true);

    return false;
}

// NOTE: Since this is an INCOMING transfer, then we need to CREATE its
// corresponding transfer workflow, since it does not already exist.
//
// (Whereas if this had been an INTERNAL transfer, then it would ALREADY
// have been created, and thus we'd need to GET the existing workflow, and
// then add the new event to it).
OTIdentifier Workflow::convey_incoming_transfer(
    const Identifier& nymID,
    const Identifier& notaryID,
    const OTTransaction& pending,
    const std::string& senderNymID,
    const std::string& recipientNymID,
    const Item& transfer) const
{
    Lock global(lock_);
    const auto existing = get_workflow(
        global,
        {proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER},
        senderNymID,
        transfer);

    if (existing) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    const auto& accountID = pending.GetPurportedAccountID();
    const auto [workflowID, workflow] = create_transfer(
        global,
        nymID.str(),
        transfer,
        proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        INCOMING_TRANSFER_WORKFLOW_VERSION,
        INCOMING_TRANSFER_SOURCE_VERSION,
        INCOMING_TRANSFER_EVENT_VERSION,
        senderNymID,
        accountID.str(),
        notaryID.str(),
        "");

    if (false == workflowID->empty()) {
        const auto time = extract_conveyed_time(workflow);
        auto note = String::Factory();
        transfer.GetNote(note);
        update_activity(
            nymID,
            transfer.GetNymID(),
            Identifier::Factory(transfer),
            workflowID,
            StorageBox::INCOMINGTRANSFER,
            time);
        update_rpc(
            recipientNymID,
            senderNymID,
            accountID.str(),
            proto::ACCOUNTEVENT_INCOMINGTRANSFER,
            workflowID->str(),
            transfer.GetAmount(),
            0,
            time,
            note->Get());
    }

    return workflowID;
}

// NOTE: Since this is an INTERNAL transfer, then it was already CREATED,
// and thus we need to GET the existing workflow, and then add the new
// event to it.
// Whereas if this is an INCOMING transfer, then we need to CREATE its
// corresponding transfer workflow since it does not already exist.
OTIdentifier Workflow::convey_internal_transfer(
    const Identifier& nymID,
    const Identifier& notaryID,
    const OTTransaction& pending,
    const std::string& senderNymID,
    const Item& transfer) const
{
    Lock global(lock_);
    const auto workflow = get_workflow(
        global,
        {proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER},
        nymID.str(),
        transfer);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer does not exist.")
            .Flush();

        return Identifier::Factory();
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_convey_transfer(*workflow)) {
        return Identifier::Factory();
    }

    const auto output = add_transfer_event(
        lock,
        nymID.str(),
        notaryID.str(),
        "",
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        proto::PAYMENTEVENTTYPE_CONVEY,
        INTERNAL_TRANSFER_EVENT_VERSION,
        pending,
        transfer.GetDestinationAcctID().str(),
        true);

    if (output) {

        return Identifier::Factory(workflow->id());
    } else {

        return Identifier::Factory();
    }
}

OTIdentifier Workflow::ConveyTransfer(
    const Identifier& nymID,
    const Identifier& notaryID,
    const OTTransaction& pending) const
{
    const auto transfer = extract_transfer_from_pending(pending);

    if (false == bool(transfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction").Flush();

        return Identifier::Factory();
    }

    const auto senderNymID = transfer->GetNymID().str();
    const auto recipientNymID = pending.GetNymID().str();

    if (pending.GetNymID() != nymID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid recipient").Flush();

        return Identifier::Factory();
    }

    const bool isInternal = (0 == senderNymID.compare(recipientNymID));

    if (isInternal) {

        return convey_internal_transfer(
            nymID, notaryID, pending, senderNymID, *transfer);
    } else {

        return convey_incoming_transfer(
            nymID, notaryID, pending, senderNymID, recipientNymID, *transfer);
    }
}

std::pair<OTIdentifier, proto::PaymentWorkflow> Workflow::create_cheque(
    const Lock& lock,
    const std::string& nymID,
    const opentxs::Cheque& cheque,
    const proto::PaymentWorkflowType workflowType,
    const proto::PaymentWorkflowState workflowState,
    const std::uint32_t workflowVersion,
    const std::uint32_t sourceVersion,
    const std::uint32_t eventVersion,
    const std::string& party,
    const std::string& account,
    const Message* message) const
{
    OT_ASSERT(verify_lock(lock))

    std::pair<OTIdentifier, proto::PaymentWorkflow> output{
        Identifier::Factory(), {}};
    auto& [workflowID, workflow] = output;
    const auto chequeID = Identifier::Factory(cheque);
    const std::string serialized = String::Factory(cheque)->Get();
    workflowID = Identifier::Random();
    workflow.set_version(workflowVersion);
    workflow.set_id(workflowID->str());
    workflow.set_type(workflowType);
    workflow.set_state(workflowState);
    auto& source = *(workflow.add_source());
    source.set_version(sourceVersion);
    source.set_id(chequeID->str());
    source.set_revision(1);
    source.set_item(serialized);

    // add party if it was passed in and is not already present
    if ((false == party.empty()) && (0 == workflow.party_size())) {
        workflow.add_party(party);
    }

    auto& event = *workflow.add_event();
    event.set_version(eventVersion);

    if (nullptr != message) {
        event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
        event.add_item(String::Factory(*message)->Get());
        event.set_time(message->m_lTime);
        event.set_method(proto::TRANSPORTMETHOD_OT);
        event.set_transport(message->m_strNotaryID->Get());
    } else {
        event.set_time(now());

        if (proto::PAYMENTWORKFLOWSTATE_UNSENT == workflowState) {
            event.set_type(proto::PAYMENTEVENTTYPE_CREATE);
            event.set_method(proto::TRANSPORTMETHOD_NONE);
        } else if (proto::PAYMENTWORKFLOWSTATE_CONVEYED == workflowState) {
            event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
            event.set_method(proto::TRANSPORTMETHOD_OOB);
        } else {
            OT_FAIL
        }
    }

    if (false == party.empty()) { event.set_nym(party); }

    event.set_success(true);
    workflow.add_unit(cheque.GetInstrumentDefinitionID().str());

    // add account if it was passed in and is not already present
    if ((false == account.empty()) && (0 == workflow.account_size())) {
        workflow.add_account(account);
    }

    if ((false == account.empty()) && (workflow.notary().empty())) {
        workflow.set_notary(
            api_.Storage().AccountServer(Identifier::Factory(account))->str());
    }

    if (workflow.notary().empty() && (nullptr != message)) {
        workflow.set_notary(message->m_strNotaryID->Get());
    }

    return save_workflow(std::move(output), nymID, account, workflow);
}

std::pair<OTIdentifier, proto::PaymentWorkflow> Workflow::create_transfer(
    const Lock& global,
    const std::string& nymID,
    const Item& transfer,
    const proto::PaymentWorkflowType workflowType,
    const proto::PaymentWorkflowState workflowState,
    const std::uint32_t workflowVersion,
    const std::uint32_t sourceVersion,
    const std::uint32_t eventVersion,
    const std::string& party,
    const std::string& account,
    const std::string& notaryID,
    const std::string& destinationAccountID) const
{
    OT_ASSERT(verify_lock(global))
    OT_ASSERT(false == nymID.empty());
    OT_ASSERT(false == account.empty());
    OT_ASSERT(false == notaryID.empty());

    std::pair<OTIdentifier, proto::PaymentWorkflow> output{
        Identifier::Factory(), {}};
    auto& [workflowID, workflow] = output;
    const auto transferID = Identifier::Factory(transfer);
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Transfer ID: ")(transferID).Flush();
    const std::string serialized = String::Factory(transfer)->Get();
    const auto existing = get_workflow(global, {workflowType}, nymID, transfer);

    if (existing) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer already exists.")
            .Flush();
        workflowID = Identifier::Factory(existing->id());

        return output;
    }

    workflowID = Identifier::Random();
    workflow.set_version(workflowVersion);
    workflow.set_id(workflowID->str());
    workflow.set_type(workflowType);
    workflow.set_state(workflowState);
    auto& source = *(workflow.add_source());
    source.set_version(sourceVersion);
    source.set_id(transferID->str());
    source.set_revision(1);
    source.set_item(serialized);
    workflow.set_notary(notaryID);

    // add party if it was passed in and is not already present
    if ((false == party.empty()) && (0 == workflow.party_size())) {
        workflow.add_party(party);
    }

    auto& event = *workflow.add_event();
    event.set_version(eventVersion);
    event.set_time(now());

    if (proto::PAYMENTWORKFLOWSTATE_INITIATED == workflowState) {
        event.set_type(proto::PAYMENTEVENTTYPE_CREATE);
        event.set_method(proto::TRANSPORTMETHOD_OT);
    } else if (proto::PAYMENTWORKFLOWSTATE_CONVEYED == workflowState) {
        event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
        event.set_method(proto::TRANSPORTMETHOD_OT);
    } else {
        OT_FAIL
    }

    event.set_transport(notaryID);

    if (false == party.empty()) { event.set_nym(party); }

    event.set_success(true);
    workflow.add_unit(
        api_.Storage().AccountContract(Identifier::Factory(account))->str());

    // add account if it is not already present
    if (0 == workflow.account_size()) {
        workflow.add_account(account);

        if (false == destinationAccountID.empty()) {
            workflow.add_account(destinationAccountID);
        }
    }

    return save_workflow(std::move(output), nymID, account, workflow);
}

// Creates outgoing and internal transfer workflows.
OTIdentifier Workflow::CreateTransfer(
    const Item& transfer,
    const Message& request) const
{
    if (false == isTransfer(transfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid item type on object")
            .Flush();

        return Identifier::Factory();
    }

    const String& senderNymID = request.m_strNymID;
    const auto& accountID = transfer.GetRealAccountID();
    const bool isInternal =
        isInternalTransfer(accountID, transfer.GetDestinationAcctID());
    Lock global(lock_);
    const auto existing = get_workflow(
        global,
        {isInternal ? proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER
                    : proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER},
        senderNymID.Get(),
        transfer);

    if (existing) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    const auto [workflowID, workflow] = create_transfer(
        global,
        senderNymID.Get(),
        transfer,
        (isInternal ? proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER
                    : proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER),
        proto::PAYMENTWORKFLOWSTATE_INITIATED,
        (isInternal ? INTERNAL_TRANSFER_WORKFLOW_VERSION
                    : OUTGOING_TRANSFER_WORKFLOW_VERSION),
        (isInternal ? INTERNAL_TRANSFER_SOURCE_VERSION
                    : OUTGOING_TRANSFER_SOURCE_VERSION),
        (isInternal ? INTERNAL_TRANSFER_EVENT_VERSION
                    : OUTGOING_TRANSFER_EVENT_VERSION),
        "",
        accountID.str(),
        request.m_strNotaryID->Get(),
        (isInternal ? transfer.GetDestinationAcctID().str() : ""));

    if (false == workflowID->empty()) {
        const auto time = extract_conveyed_time(workflow);
        auto note = String::Factory();
        transfer.GetNote(note);
        update_rpc(
            senderNymID.Get(),
            "",
            accountID.str(),
            proto::ACCOUNTEVENT_OUTGOINGTRANSFER,
            workflowID->str(),
            transfer.GetAmount(),
            0,
            time,
            note->Get());
    }

    return workflowID;
}

bool Workflow::DepositCheque(
    const Identifier& receiver,
    const Identifier& accountID,
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = receiver.str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE}, nymID, cheque);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_deposit_cheque(*workflow)) { return false; }

    const auto output = add_cheque_event(
        lock,
        nymID,
        cheque.GetSenderNymID().str(),
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_COMPLETED,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        INCOMING_CHEQUE_EVENT_VERSION,
        request,
        reply,
        accountID.str());

    if (output && cheque_deposit_success(reply)) {
        update_rpc(
            receiver.str(),
            cheque.GetSenderNymID().str(),
            accountID.str(),
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflow->id(),
            cheque.GetAmount(),
            0,
            std::chrono::system_clock::from_time_t(reply->m_lTime),
            cheque.GetMemo().Get());
    }

    return output;
}

bool Workflow::ExpireCheque(
    const Identifier& nym,
    const opentxs::Cheque& cheque) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = nym.str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global,
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        nymID,
        cheque);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_expire_cheque(cheque, *workflow)) { return false; }

    workflow->set_state(proto::PAYMENTWORKFLOWSTATE_EXPIRED);

    return save_workflow(nymID, cheque.GetSenderAcctID().str(), *workflow);
}

bool Workflow::ExportCheque(const opentxs::Cheque& cheque) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(global, {}, nymID, cheque);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_convey_cheque(*workflow)) { return false; }

    workflow->set_state(proto::PAYMENTWORKFLOWSTATE_CONVEYED);
    auto& event = *(workflow->add_event());
    event.set_version(OUTGOING_CHEQUE_EVENT_VERSION);
    event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
    event.set_time(now());
    event.set_method(proto::TRANSPORTMETHOD_OOB);
    event.set_success(true);

    return save_workflow(nymID, cheque.GetSenderAcctID().str(), *workflow);
}

std::chrono::time_point<std::chrono::system_clock> Workflow::
    extract_conveyed_time(const proto::PaymentWorkflow& workflow)
{
    for (const auto& event : workflow.event()) {
        if (proto::PAYMENTEVENTTYPE_CONVEY == event.type()) {
            if (event.success()) {

                return std::chrono::system_clock::from_time_t(event.time());
            }
        }
    }

    return {};
}

std::unique_ptr<Item> Workflow::extract_transfer_from_pending(
    const OTTransaction& receipt) const
{
    if (transactionType::pending != receipt.GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect receipt type: ")(
            receipt.GetTypeString())
            .Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    receipt.GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing serialized transfer item")
            .Flush();

        return nullptr;
    }

    auto transfer = api_.Factory().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate transfer item")
            .Flush();

        return nullptr;
    }

    if (itemType::transfer != transfer->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer item type.")
            .Flush();

        return nullptr;
    }

    return transfer;
}

std::unique_ptr<Item> Workflow::extract_transfer_from_receipt(
    const OTTransaction& receipt,
    Identifier& depositorNymID) const
{
    if (transactionType::transferReceipt != receipt.GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect receipt type: ")(
            receipt.GetTypeString())
            .Flush();

        return nullptr;
    }

    auto serializedAcceptPending = String::Factory();
    receipt.GetReferenceString(serializedAcceptPending);

    if (serializedAcceptPending->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Missing serialized accept pending item")
            .Flush();

        return nullptr;
    }

    const auto acceptPending = api_.Factory().Item(serializedAcceptPending);

    if (false == bool(acceptPending)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate accept pending item")
            .Flush();

        return nullptr;
    }

    if (itemType::acceptPending != acceptPending->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid accept pending item type.")
            .Flush();

        return nullptr;
    }

    depositorNymID.Assign(acceptPending->GetNymID());
    auto serializedPending = String::Factory();
    acceptPending->GetAttachment(serializedPending);

    if (serializedPending->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Missing serialized pending transaction")
            .Flush();

        return nullptr;
    }

    auto pending = api_.Factory().Transaction(
        receipt.GetNymID(),
        receipt.GetRealAccountID(),
        receipt.GetRealNotaryID());

    if (false == bool(pending)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate pending transaction")
            .Flush();

        return nullptr;
    }

    const bool loaded = pending->LoadContractFromString(serializedPending);

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to deserialize pending transaction")
            .Flush();

        return nullptr;
    }

    if (transactionType::pending != pending->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid pending transaction type.")
            .Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    pending->GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing serialized transfer item")
            .Flush();

        return nullptr;
    }

    auto transfer = api_.Factory().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate transfer item")
            .Flush();

        return nullptr;
    }

    if (itemType::transfer != transfer->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer item type.")
            .Flush();

        return nullptr;
    }

    return transfer;
}

bool Workflow::FinishCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE}, nymID, cheque);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_finish_cheque(*workflow)) { return false; }

    return add_cheque_event(
        lock,
        nymID,
        "",
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_COMPLETED,
        proto::PAYMENTEVENTTYPE_COMPLETE,
        OUTGOING_CHEQUE_EVENT_VERSION,
        request,
        reply);
}

template <typename T>
std::shared_ptr<proto::PaymentWorkflow> Workflow::get_workflow(
    const Lock& global,
    const std::set<proto::PaymentWorkflowType>& types,
    const std::string& nymID,
    const T& source) const
{
    OT_ASSERT(verify_lock(global));

    const auto itemID = Identifier::Factory(source)->str();
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Item ID: ")(itemID).Flush();

    return get_workflow_by_source(types, nymID, itemID);
}

std::shared_ptr<proto::PaymentWorkflow> Workflow::get_workflow_by_id(
    const std::string& nymID,
    const std::string& workflowID) const
{
    std::shared_ptr<proto::PaymentWorkflow> output{nullptr};
    const auto loaded = api_.Storage().Load(nymID, workflowID, output);

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Workflow ")(workflowID)(
            " for nym ")(nymID)(" can not be loaded")
            .Flush();

        return output;
    }

    return output;
}

std::shared_ptr<proto::PaymentWorkflow> Workflow::get_workflow_by_id(
    const std::set<proto::PaymentWorkflowType>& types,
    const std::string& nymID,
    const std::string& workflowID) const
{
    auto output = get_workflow_by_id(nymID, workflowID);

    if (0 == types.count(output->type())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect type on workflow ")(
            workflowID)(" for nym ")(nymID)
            .Flush();

        return {nullptr};
    }

    return output;
}

std::shared_ptr<proto::PaymentWorkflow> Workflow::get_workflow_by_source(
    const std::set<proto::PaymentWorkflowType>& types,
    const std::string& nymID,
    const std::string& sourceID) const
{
    const auto workflowID =
        api_.Storage().PaymentWorkflowLookup(nymID, sourceID);

    if (workflowID.empty()) { return {}; }

    return get_workflow_by_id(types, nymID, workflowID);
}

eLock Workflow::get_workflow_lock(Lock& global, const std::string& id) const
{
    OT_ASSERT(verify_lock(global));

    auto output = eLock(workflow_locks_[id]);
    global.unlock();

    return output;
}

OTIdentifier Workflow::ImportCheque(
    const Identifier& nymID,
    const opentxs::Cheque& cheque) const
{
    if (false == isCheque(cheque)) { return Identifier::Factory(); }

    if (false == validate_recipient(nymID, cheque)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " can not deposit this cheque.")
            .Flush();

        return Identifier::Factory();
    }

    Lock global(lock_);
    const auto existing = get_workflow(
        global,
        {proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        nymID.str(),
        cheque);

    if (existing) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    const std::string party = cheque.GetSenderNymID().str();
    const auto [workflowID, workflow] = create_cheque(
        global,
        nymID.str(),
        cheque,
        proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        INCOMING_CHEQUE_WORKFLOW_VERSION,
        INCOMING_CHEQUE_SOURCE_VERSION,
        INCOMING_CHEQUE_EVENT_VERSION,
        party,
        "");

    if (false == workflowID->empty()) {
        const auto time = extract_conveyed_time(workflow);
        update_activity(
            nymID,
            cheque.GetSenderNymID(),
            Identifier::Factory(cheque),
            workflowID,
            StorageBox::INCOMINGCHEQUE,
            time);
        update_rpc(
            nymID.str(),
            cheque.GetSenderNymID().str(),
            "",
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflowID->str(),
            0,
            cheque.GetAmount(),
            time,
            cheque.GetMemo().Get());
    }

    return workflowID;
}

bool Workflow::isCheque(const opentxs::Cheque& cheque)
{
    if (cheque.HasRemitter()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Provided instrument is a voucher")
            .Flush();

        return false;
    }

    if (0 > cheque.GetAmount()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Provided instrument is an invoice")
            .Flush();

        return false;
    }

    if (0 == cheque.GetAmount()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Provided instrument is a cancellation")
            .Flush();

        return false;
    }

    return true;
}

bool Workflow::isInternalTransfer(
    const Identifier& sourceAccount,
    const Identifier& destinationAccount) const
{
    const auto ownerNymID = api_.Storage().AccountOwner(sourceAccount);

    OT_ASSERT(false == ownerNymID->empty());

    const auto recipientNymID = api_.Storage().AccountOwner(destinationAccount);

    if (recipientNymID->empty()) { return false; }

    return ownerNymID == recipientNymID;
}

bool Workflow::isTransfer(const Item& item)
{
    return itemType::transfer == item.GetType();
}

std::set<OTIdentifier> Workflow::List(
    const Identifier& nymID,
    const proto::PaymentWorkflowType type,
    const proto::PaymentWorkflowState state) const
{
    const auto input =
        api_.Storage().PaymentWorkflowsByState(nymID.str(), type, state);
    std::set<OTIdentifier> output{};
    std::transform(
        input.begin(),
        input.end(),
        std::inserter(output, output.end()),
        [](const std::string& id) -> OTIdentifier {
            return Identifier::Factory(id);
        });

    return output;
}

Workflow::Cheque Workflow::LoadCheque(
    const Identifier& nymID,
    const Identifier& chequeID) const
{
    auto workflow = get_workflow_by_source(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        nymID.str(),
        chequeID.str());

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque does not exist.")
            .Flush();

        return {};
    }

    return InstantiateCheque(api_, *workflow);
}

Workflow::Cheque Workflow::LoadChequeByWorkflow(
    const Identifier& nymID,
    const Identifier& workflowID) const
{
    auto workflow = get_workflow_by_id(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        nymID.str(),
        workflowID.str());

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque does not exist.")
            .Flush();

        return {};
    }

    return InstantiateCheque(api_, *workflow);
}

Workflow::Transfer Workflow::LoadTransfer(
    const Identifier& nymID,
    const Identifier& transferID) const
{
    auto workflow = get_workflow_by_source(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER,
         proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER},
        nymID.str(),
        transferID.str());

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer does not exist.")
            .Flush();

        return {};
    }

    return InstantiateTransfer(api_, *workflow);
}

Workflow::Transfer Workflow::LoadTransferByWorkflow(
    const Identifier& nymID,
    const Identifier& workflowID) const
{
    auto workflow = get_workflow_by_id(
        {proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER,
         proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER},
        nymID.str(),
        workflowID.str());

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this transfer does not exist.")
            .Flush();

        return {};
    }

    return InstantiateTransfer(api_, *workflow);
}

std::shared_ptr<proto::PaymentWorkflow> Workflow::LoadWorkflow(
    const Identifier& nymID,
    const Identifier& workflowID) const
{
    return get_workflow_by_id(nymID.str(), workflowID.str());
}

OTIdentifier Workflow::ReceiveCheque(
    const Identifier& nymID,
    const opentxs::Cheque& cheque,
    const Message& message) const
{
    if (false == isCheque(cheque)) { return Identifier::Factory(); }

    if (false == validate_recipient(nymID, cheque)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " can not deposit this cheque.")
            .Flush();

        return Identifier::Factory();
    }

    Lock global(lock_);
    const auto existing = get_workflow(
        global,
        {proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        nymID.str(),
        cheque);

    if (existing) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    const std::string party = cheque.GetSenderNymID().str();
    const auto [workflowID, workflow] = create_cheque(
        global,
        nymID.str(),
        cheque,
        proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        INCOMING_CHEQUE_WORKFLOW_VERSION,
        INCOMING_CHEQUE_SOURCE_VERSION,
        INCOMING_CHEQUE_EVENT_VERSION,
        party,
        "",
        &message);

    if (false == workflowID->empty()) {
        const auto time = extract_conveyed_time(workflow);
        update_activity(
            nymID,
            cheque.GetSenderNymID(),
            Identifier::Factory(cheque),
            workflowID,
            StorageBox::INCOMINGCHEQUE,
            time);
        update_rpc(
            nymID.str(),
            cheque.GetSenderNymID().str(),
            "",
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflowID->str(),
            0,
            cheque.GetAmount(),
            time,
            cheque.GetMemo().Get());
    }

    return workflowID;
}

bool Workflow::save_workflow(
    const std::string& nymID,
    const std::string& accountID,
    const proto::PaymentWorkflow& workflow) const
{
    const bool valid = proto::Validate(workflow, VERBOSE);

    OT_ASSERT(valid)

    const auto saved = api_.Storage().Store(nymID, workflow);

    OT_ASSERT(saved)

    if (false == accountID.empty()) { account_publisher_->Publish(accountID); }

    return valid && saved;
}

OTIdentifier Workflow::save_workflow(
    OTIdentifier&& output,
    const std::string& nymID,
    const std::string& accountID,
    const proto::PaymentWorkflow& workflow) const
{
    if (save_workflow(nymID, accountID, workflow)) { return std::move(output); }

    return Identifier::Factory();
}

std::pair<OTIdentifier, proto::PaymentWorkflow> Workflow::save_workflow(
    std::pair<OTIdentifier, proto::PaymentWorkflow>&& output,
    const std::string& nymID,
    const std::string& accountID,
    const proto::PaymentWorkflow& workflow) const
{
    if (save_workflow(nymID, accountID, workflow)) { return std::move(output); }

    return {Identifier::Factory(), {}};
}

bool Workflow::SendCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE}, nymID, cheque);

    if (false == bool(workflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_convey_cheque(*workflow)) { return false; }

    return add_cheque_event(
        lock,
        nymID,
        request.m_strNymID2->Get(),
        *workflow,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED,
        proto::PAYMENTEVENTTYPE_CONVEY,
        OUTGOING_CHEQUE_EVENT_VERSION,
        request,
        reply);
}

void Workflow::update_activity(
    const Identifier& localNymID,
    const Identifier& remoteNymID,
    const Identifier& sourceID,
    const Identifier& workflowID,
    const StorageBox type,
    std::chrono::time_point<std::chrono::system_clock> time) const
{
    const auto contactID = contact_.ContactID(remoteNymID);

    OT_ASSERT(false == contactID->empty())

    const bool added = activity_.AddPaymentEvent(
        localNymID, contactID, type, sourceID, workflowID, time);

    if (added) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Success adding payment event to thread ")(contactID->str())
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to add payment event to thread ")(contactID->str())
            .Flush();
    }
}

void Workflow::update_rpc(
    const std::string& localNymID,
    const std::string& remoteNymID,
    const std::string& accountID,
    const proto::AccountEventType type,
    const std::string& workflowID,
    const Amount amount,
    const Amount pending,
    const std::chrono::time_point<std::chrono::system_clock> time,
    const std::string& memo) const
{
    proto::RPCPush push{};
    push.set_version(RPC_PUSH_VERSION);
    push.set_type(proto::RPCPUSH_ACCOUNT);
    push.set_id(localNymID);
    auto& event = *push.mutable_accountevent();
    event.set_version(RPC_ACCOUNT_EVENT_VERSION);
    event.set_id(accountID);
    event.set_type(type);

    if (false == remoteNymID.empty()) {
        event.set_contact(
            contact_.NymToContact(Identifier::Factory(remoteNymID))->str());
    }

    event.set_workflow(workflowID);
    event.set_amount(amount);
    event.set_pendingamount(pending);
    event.set_timestamp(std::chrono::system_clock::to_time_t(time));
    event.set_memo(memo);

    OT_ASSERT(proto::Validate(push, VERBOSE));

    auto message = zmq::Message::Factory();
    message->AddFrame();
    message->AddFrame(localNymID);
    message->AddFrame(proto::ProtoAsData(push));
    rpc_publisher_->Push(message);
}

bool Workflow::validate_recipient(
    const Identifier& nymID,
    const opentxs::Cheque& cheque)
{
    if (nymID.empty()) { return true; }

    return (nymID == cheque.GetRecipientNymID());
}

std::vector<OTIdentifier> Workflow::WorkflowsByAccount(
    const Identifier& nymID,
    const Identifier& accountID) const
{
    std::vector<OTIdentifier> output{};
    const auto workflows =
        api_.Storage().PaymentWorkflowsByAccount(nymID.str(), accountID.str());
    std::transform(
        workflows.begin(),
        workflows.end(),
        std::inserter(output, output.end()),
        [](const std::string& id) -> OTIdentifier {
            return Identifier::Factory(id);
        });

    return output;
}

OTIdentifier Workflow::WriteCheque(const opentxs::Cheque& cheque) const
{
    if (false == isCheque(cheque)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid item type on cheque object")
            .Flush();

        return Identifier::Factory();
    }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto existing = get_workflow(
        global, {proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE}, nymID, cheque);

    if (existing) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow for this cheque already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    const std::string party =
        cheque.HasRecipient() ? cheque.GetRecipientNymID().str() : "";
    const auto [workflowID, workflow] = create_cheque(
        global,
        nymID,
        cheque,
        proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_UNSENT,
        OUTGOING_CHEQUE_WORKFLOW_VERSION,
        OUTGOING_CHEQUE_SOURCE_VERSION,
        OUTGOING_CHEQUE_EVENT_VERSION,
        party,
        cheque.GetSenderAcctID().str());
    global.unlock();
    const bool haveWorkflow = (false == workflowID->empty());
    const auto time{
        std::chrono::system_clock::from_time_t(workflow.event(0).time())};

    if (haveWorkflow && cheque.HasRecipient()) {
        update_activity(
            cheque.GetSenderNymID(),
            cheque.GetRecipientNymID(),
            Identifier::Factory(cheque),
            workflowID,
            StorageBox::OUTGOINGCHEQUE,
            time);
    }

    if (false == workflowID->empty()) {
        update_rpc(
            nymID,
            cheque.GetRecipientNymID().str(),
            cheque.SourceAccountID().str(),
            proto::ACCOUNTEVENT_OUTGOINGCHEQUE,
            workflowID->str(),
            0,
            -1 * cheque.GetAmount(),
            time,
            cheque.GetMemo().Get());
    }

    return workflowID;
}
}  // namespace implementation
}  // namespace opentxs::api::client
