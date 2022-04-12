// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "api/session/Workflow.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/api/session/Factory.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Types.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/PaymentWorkflow.hpp"
#include "internal/serialization/protobuf/verify/RPCPush.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/AccountEvent.pb.h"
#include "serialization/protobuf/InstrumentRevision.pb.h"
#include "serialization/protobuf/PaymentEvent.pb.h"
#include "serialization/protobuf/PaymentWorkflow.pb.h"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "serialization/protobuf/Purse.pb.h"
#include "serialization/protobuf/RPCEnums.pb.h"
#include "serialization/protobuf/RPCPush.pb.h"

namespace opentxs
{
constexpr auto RPC_ACCOUNT_EVENT_VERSION = 1;
constexpr auto RPC_PUSH_VERSION = 1;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto Workflow(
    const api::Session& api,
    const api::session::Activity& activity,
    const api::session::Contacts& contact) noexcept
    -> std::unique_ptr<api::session::Workflow>
{
    using ReturnType = api::session::imp::Workflow;

    return std::make_unique<ReturnType>(api, activity, contact);
}
}  // namespace opentxs::factory

namespace opentxs::api::session
{
using PaymentWorkflowState = otx::client::PaymentWorkflowState;
using PaymentWorkflowType = otx::client::PaymentWorkflowType;

auto Workflow::ContainsCash(const proto::PaymentWorkflow& workflow) -> bool
{
    switch (translate(workflow.type())) {
        case PaymentWorkflowType::OutgoingCash:
        case PaymentWorkflowType::IncomingCash: {
            return true;
        }
        case PaymentWorkflowType::Error:
        case PaymentWorkflowType::OutgoingCheque:
        case PaymentWorkflowType::IncomingCheque:
        case PaymentWorkflowType::OutgoingInvoice:
        case PaymentWorkflowType::IncomingInvoice:
        case PaymentWorkflowType::OutgoingTransfer:
        case PaymentWorkflowType::IncomingTransfer:
        case PaymentWorkflowType::InternalTransfer:
        default: {
        }
    }

    return false;
}

auto Workflow::ContainsCheque(const proto::PaymentWorkflow& workflow) -> bool
{
    switch (translate(workflow.type())) {
        case PaymentWorkflowType::OutgoingCheque:
        case PaymentWorkflowType::IncomingCheque:
        case PaymentWorkflowType::OutgoingInvoice:
        case PaymentWorkflowType::IncomingInvoice: {
            return true;
        }
        case PaymentWorkflowType::Error:
        case PaymentWorkflowType::OutgoingTransfer:
        case PaymentWorkflowType::IncomingTransfer:
        case PaymentWorkflowType::InternalTransfer:
        case PaymentWorkflowType::OutgoingCash:
        case PaymentWorkflowType::IncomingCash:
        default: {
        }
    }

    return false;
}

auto Workflow::ContainsTransfer(const proto::PaymentWorkflow& workflow) -> bool
{
    switch (translate(workflow.type())) {
        case PaymentWorkflowType::OutgoingTransfer:
        case PaymentWorkflowType::IncomingTransfer:
        case PaymentWorkflowType::InternalTransfer: {
            return true;
        }
        case PaymentWorkflowType::Error:
        case PaymentWorkflowType::OutgoingCheque:
        case PaymentWorkflowType::IncomingCheque:
        case PaymentWorkflowType::OutgoingInvoice:
        case PaymentWorkflowType::IncomingInvoice:
        case PaymentWorkflowType::OutgoingCash:
        case PaymentWorkflowType::IncomingCash:
        default: {
        }
    }

    return false;
}

auto Workflow::ExtractCheque(const proto::PaymentWorkflow& workflow)
    -> UnallocatedCString
{
    if (false == ContainsCheque(workflow)) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Wrong workflow type").Flush();

        return {};
    }

    if (1 != workflow.source().size()) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Invalid workflow").Flush();

        return {};
    }

    return workflow.source(0).item();
}

auto Workflow::ExtractPurse(
    const proto::PaymentWorkflow& workflow,
    proto::Purse& out) -> bool
{
    if (false == ContainsCash(workflow)) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Wrong workflow type").Flush();

        return false;
    }

    if (1 != workflow.source().size()) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Invalid workflow").Flush();

        return false;
    }

    const auto& serialized = workflow.source(0).item();
    out = proto::Factory<proto::Purse>(serialized);

    return true;
}

auto Workflow::ExtractTransfer(const proto::PaymentWorkflow& workflow)
    -> UnallocatedCString
{
    if (false == ContainsTransfer(workflow)) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Wrong workflow type").Flush();

        return {};
    }

    if (1 != workflow.source().size()) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Invalid workflow").Flush();

        return {};
    }

    return workflow.source(0).item();
}

auto Workflow::InstantiateCheque(
    const api::Session& api,
    const proto::PaymentWorkflow& workflow) -> Workflow::Cheque
{
    Cheque output{PaymentWorkflowState::Error, nullptr};
    auto& [state, cheque] = output;

    switch (translate(workflow.type())) {
        case PaymentWorkflowType::OutgoingCheque:
        case PaymentWorkflowType::IncomingCheque:
        case PaymentWorkflowType::OutgoingInvoice:
        case PaymentWorkflowType::IncomingInvoice: {
            cheque.reset(api.Factory().InternalSession().Cheque().release());

            OT_ASSERT(cheque)

            const auto serialized = ExtractCheque(workflow);

            if (serialized.empty()) { return output; }

            const auto loaded = cheque->LoadContractFromString(
                String::Factory(serialized.c_str()));

            if (false == loaded) {
                LogError()(OT_PRETTY_STATIC(Workflow))(
                    "Failed to instantiate cheque")
                    .Flush();
                cheque.reset();

                return output;
            }

            state = translate(workflow.state());
        } break;
        case PaymentWorkflowType::Error:
        case PaymentWorkflowType::OutgoingTransfer:
        case PaymentWorkflowType::IncomingTransfer:
        case PaymentWorkflowType::InternalTransfer:
        case PaymentWorkflowType::OutgoingCash:
        case PaymentWorkflowType::IncomingCash:
        default: {
            LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow type")
                .Flush();
        }
    }

    return output;
}

auto Workflow::InstantiatePurse(
    const api::Session& api,
    const proto::PaymentWorkflow& workflow) -> Workflow::Purse
{
    auto output = Purse{};
    auto& [state, purse] = output;
    state = PaymentWorkflowState::Error;

    switch (translate(workflow.type())) {
        case PaymentWorkflowType::OutgoingCash:
        case PaymentWorkflowType::IncomingCash: {
            try {
                const auto serialized = [&] {
                    auto out = proto::Purse{};

                    if (false == ExtractPurse(workflow, out)) {
                        throw std::runtime_error{"Missing purse"};
                    }

                    return out;
                }();

                purse = api.Factory().InternalSession().Purse(serialized);

                if (false == bool(purse)) {
                    throw std::runtime_error{"Failed to instantiate purse"};
                }

                state = translate(workflow.state());
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_STATIC(Workflow))(e.what()).Flush();

                return output;
            }
        } break;
        case PaymentWorkflowType::Error:
        case PaymentWorkflowType::OutgoingCheque:
        case PaymentWorkflowType::IncomingCheque:
        case PaymentWorkflowType::OutgoingInvoice:
        case PaymentWorkflowType::IncomingInvoice:
        case PaymentWorkflowType::OutgoingTransfer:
        case PaymentWorkflowType::IncomingTransfer:
        case PaymentWorkflowType::InternalTransfer:
        default: {
            LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow type")
                .Flush();
        }
    }

    return output;
}

auto Workflow::InstantiateTransfer(
    const api::Session& api,
    const proto::PaymentWorkflow& workflow) -> Workflow::Transfer
{
    Transfer output{PaymentWorkflowState::Error, nullptr};
    auto& [state, transfer] = output;

    switch (translate(workflow.type())) {
        case PaymentWorkflowType::OutgoingTransfer:
        case PaymentWorkflowType::IncomingTransfer:
        case PaymentWorkflowType::InternalTransfer: {
            const auto serialized = ExtractTransfer(workflow);

            if (serialized.empty()) { return output; }

            transfer.reset(
                api.Factory().InternalSession().Item(serialized).release());

            if (false == bool(transfer)) {
                LogError()(OT_PRETTY_STATIC(Workflow))(
                    "Failed to instantiate transfer")
                    .Flush();
                transfer.reset();

                return output;
            }

            state = translate(workflow.state());
        } break;

        case PaymentWorkflowType::Error:
        case PaymentWorkflowType::OutgoingCheque:
        case PaymentWorkflowType::IncomingCheque:
        case PaymentWorkflowType::OutgoingInvoice:
        case PaymentWorkflowType::IncomingInvoice:
        case PaymentWorkflowType::OutgoingCash:
        case PaymentWorkflowType::IncomingCash:
        default: {
            LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow type")
                .Flush();
        }
    }

    return output;
}

auto Workflow::UUID(
    const api::Session& api,
    const proto::PaymentWorkflow& workflow) -> OTIdentifier
{
    auto output = Identifier::Factory();
    auto notaryID = Identifier::Factory();
    TransactionNumber number{0};

    switch (translate(workflow.type())) {
        case PaymentWorkflowType::OutgoingCheque:
        case PaymentWorkflowType::IncomingCheque:
        case PaymentWorkflowType::OutgoingInvoice:
        case PaymentWorkflowType::IncomingInvoice: {
            [[maybe_unused]] auto [state, cheque] =
                InstantiateCheque(api, workflow);

            if (false == bool(cheque)) {
                LogError()(OT_PRETTY_STATIC(Workflow))("Invalid cheque")
                    .Flush();

                return output;
            }

            notaryID = cheque->GetNotaryID();
            number = cheque->GetTransactionNum();
        } break;
        case PaymentWorkflowType::OutgoingTransfer:
        case PaymentWorkflowType::IncomingTransfer:
        case PaymentWorkflowType::InternalTransfer: {
            [[maybe_unused]] auto [state, transfer] =
                InstantiateTransfer(api, workflow);

            if (false == bool(transfer)) {
                LogError()(OT_PRETTY_STATIC(Workflow))("Invalid transfer")
                    .Flush();

                return output;
            }

            notaryID = transfer->GetPurportedNotaryID();
            number = transfer->GetTransactionNum();
        } break;
        case PaymentWorkflowType::OutgoingCash:
        case PaymentWorkflowType::IncomingCash: {
            // TODO
        } break;
        default: {
            LogError()(OT_PRETTY_STATIC(Workflow))("Unknown workflow type")
                .Flush();
        }
    }

    return UUID(api, notaryID, number);
}

auto Workflow::UUID(
    const api::Session& api,
    const Identifier& notary,
    const TransactionNumber& number) -> OTIdentifier
{
    LogTrace()(OT_PRETTY_STATIC(Workflow))("UUID for notary ")(
        notary)(" and transaction number ")(number)(" is ");
    auto preimage = api.Factory().Data();
    preimage->Assign(notary);
    preimage->Concatenate(&number, sizeof(number));
    auto output = Identifier::Factory();
    output->CalculateDigest(preimage->Bytes());
    LogTrace()(output).Flush();

    return output;
}
}  // namespace opentxs::api::session

namespace opentxs::api::session::imp
{
using PaymentWorkflowState = otx::client::PaymentWorkflowState;
using PaymentWorkflowType = otx::client::PaymentWorkflowType;

const Workflow::VersionMap Workflow::versions_{
    {PaymentWorkflowType::OutgoingCheque, {1, 1, 1}},
    {PaymentWorkflowType::IncomingCheque, {1, 1, 1}},
    {PaymentWorkflowType::OutgoingTransfer, {2, 1, 2}},
    {PaymentWorkflowType::IncomingTransfer, {2, 1, 2}},
    {PaymentWorkflowType::InternalTransfer, {2, 1, 2}},
    {PaymentWorkflowType::OutgoingCash, {3, 1, 3}},
    {PaymentWorkflowType::IncomingCash, {3, 1, 3}},
};

Workflow::Workflow(
    const api::Session& api,
    const api::session::Activity& activity,
    const api::session::Contacts& contact)
    : api_(api)
    , activity_(activity)
    , contact_(contact)
    , account_publisher_(api_.Network().ZeroMQ().PublishSocket())
    , rpc_publisher_(
          api_.Network().ZeroMQ().PushSocket(zmq::socket::Direction::Connect))
    , workflow_locks_()
{
    // WARNING: do not access api_.Wallet() during construction
    const auto endpoint = api_.Endpoints().WorkflowAccountUpdate();
    LogDetail()(OT_PRETTY_CLASS())("Binding to ")(endpoint.data()).Flush();
    auto bound = account_publisher_->Start(endpoint.data());

    OT_ASSERT(bound)

    bound =
        rpc_publisher_->Start(opentxs::network::zeromq::MakeDeterministicInproc(
            "rpc/push/internal", -1, 1));

    OT_ASSERT(bound)
}

auto Workflow::AbortTransfer(
    const identifier::Nym& nymID,
    const Item& transfer,
    const Message& reply) const -> bool
{
    if (false == isTransfer(transfer)) { return false; }

    const bool isInternal = isInternalTransfer(
        transfer.GetRealAccountID(), transfer.GetDestinationAcctID());
    const UnallocatedSet<PaymentWorkflowType> type{
        isInternal ? PaymentWorkflowType::InternalTransfer
                   : PaymentWorkflowType::OutgoingTransfer};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), transfer);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer does not exist.")
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
        PaymentWorkflowState::Aborted,
        proto::PAYMENTEVENTTYPE_ABORT,
        (isInternal
             ? versions_.at(PaymentWorkflowType::InternalTransfer).event_
             : versions_.at(PaymentWorkflowType::OutgoingTransfer).event_),
        reply,
        transfer.GetRealAccountID(),
        true);
}

// Works for Incoming and Internal transfer workflows.
auto Workflow::AcceptTransfer(
    const identifier::Nym& nymID,
    const identifier::Notary& notaryID,
    const OTTransaction& pending,
    const Message& reply) const -> bool
{
    const auto transfer = extract_transfer_from_pending(pending);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Invalid transaction").Flush();

        return false;
    }

    const auto senderNymID = transfer->GetNymID().str();
    const auto recipientNymID = pending.GetNymID().str();
    const auto& accountID = pending.GetPurportedAccountID();

    if (pending.GetNymID() != nymID) {
        LogError()(OT_PRETTY_CLASS())("Invalid recipient").Flush();

        return false;
    }

    const bool isInternal = (0 == senderNymID.compare(recipientNymID));

    // Ignore this event for internal transfers.
    if (isInternal) { return true; }

    const UnallocatedSet<PaymentWorkflowType> type{
        PaymentWorkflowType::IncomingTransfer};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), *transfer);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer does not exist.")
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
        PaymentWorkflowState::Completed,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        versions_.at(PaymentWorkflowType::OutgoingTransfer).event_,
        reply,
        accountID,
        true);
}

auto Workflow::AcknowledgeTransfer(
    const identifier::Nym& nymID,
    const Item& transfer,
    const Message& reply) const -> bool
{
    if (false == isTransfer(transfer)) { return false; }

    const bool isInternal = isInternalTransfer(
        transfer.GetRealAccountID(), transfer.GetDestinationAcctID());
    const UnallocatedSet<PaymentWorkflowType> type{
        isInternal ? PaymentWorkflowType::InternalTransfer
                   : PaymentWorkflowType::OutgoingTransfer};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), transfer);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer does not exist.")
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
    const PaymentWorkflowState state =
        (PaymentWorkflowState::Conveyed == translate(workflow->state()))
            ? PaymentWorkflowState::Conveyed
            : PaymentWorkflowState::Acknowledged;

    return add_transfer_event(
        lock,
        nymID.str(),
        "",
        *workflow,
        state,
        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
        (isInternal
             ? versions_.at(PaymentWorkflowType::InternalTransfer).event_
             : versions_.at(PaymentWorkflowType::OutgoingTransfer).event_),
        reply,
        transfer.GetRealAccountID(),
        true);
}

auto Workflow::AllocateCash(
    const identifier::Nym& id,
    const otx::blind::Purse& purse) const -> OTIdentifier
{
    Lock global(lock_);
    auto workflowID = Identifier::Random();
    proto::PaymentWorkflow workflow{};
    workflow.set_version(
        versions_.at(otx::client::PaymentWorkflowType::OutgoingCash).workflow_);
    workflow.set_id(workflowID->str());
    workflow.set_type(translate(PaymentWorkflowType::OutgoingCash));
    workflow.set_state(translate(PaymentWorkflowState::Unsent));
    auto& source = *(workflow.add_source());
    source.set_version(versions_.at(PaymentWorkflowType::OutgoingCash).source_);
    source.set_id(workflowID->str());
    source.set_revision(1);
    source.set_item([&] {
        auto proto = proto::Purse{};
        purse.Internal().Serialize(proto);

        return proto::ToString(proto);
    }());
    workflow.set_notary(purse.Notary().str());
    auto& event = *workflow.add_event();
    event.set_version(versions_.at(PaymentWorkflowType::OutgoingCash).event_);
    event.set_time(Clock::to_time_t(Clock::now()));
    event.set_type(proto::PAYMENTEVENTTYPE_CREATE);
    event.set_method(proto::TRANSPORTMETHOD_NONE);
    event.set_success(true);
    workflow.add_unit(purse.Unit().str());
    const auto saved = save_workflow(id.str(), workflow);

    if (false == saved) {
        LogError()(OT_PRETTY_CLASS())("Failed to save workflow").Flush();

        return Identifier::Factory();
    }

    return workflowID;
}

auto Workflow::add_cheque_event(
    const eLock& lock,
    const UnallocatedCString& nymID,
    const UnallocatedCString&,
    proto::PaymentWorkflow& workflow,
    const PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const VersionNumber version,
    const Message& request,
    const Message* reply,
    const Identifier& account) const -> bool
{
    const bool haveReply = (nullptr != reply);
    const bool success = cheque_deposit_success(reply);

    if (success) {
        workflow.set_state(translate(newState));

        if ((false == account.empty()) && (0 == workflow.account_size())) {
            workflow.add_account(account.str());
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
auto Workflow::add_cheque_event(
    const eLock& lock,
    const UnallocatedCString& nymID,
    const Identifier& accountID,
    proto::PaymentWorkflow& workflow,
    const PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const VersionNumber version,
    const identifier::Nym& recipientNymID,
    const OTTransaction& receipt,
    const Time time) const -> bool
{
    auto message = String::Factory();
    receipt.SaveContractRaw(message);
    workflow.set_state(translate(newState));
    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(message->Get());
    event.set_time(Clock::to_time_t(time));
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(receipt.GetRealNotaryID().str());
    event.set_nym(recipientNymID.str());
    event.set_success(true);

    if (0 == workflow.party_size()) {
        workflow.add_party(recipientNymID.str());
    }

    return save_workflow(nymID, accountID, workflow);
}

auto Workflow::add_transfer_event(
    const eLock& lock,
    const UnallocatedCString& nymID,
    const UnallocatedCString& eventNym,
    proto::PaymentWorkflow& workflow,
    const PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const VersionNumber version,
    const Message& message,
    const Identifier& account,
    const bool success) const -> bool
{
    if (success) { workflow.set_state(translate(newState)); }

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

auto Workflow::add_transfer_event(
    const eLock& lock,
    const UnallocatedCString& nymID,
    const UnallocatedCString& notaryID,
    const UnallocatedCString& eventNym,
    proto::PaymentWorkflow& workflow,
    const otx::client::PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const VersionNumber version,
    const OTTransaction& receipt,
    const Identifier& account,
    const bool success) const -> bool
{
    if (success) { workflow.set_state(translate(newState)); }

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
    event.set_time(Clock::to_time_t(Clock::now()));

    if (0 == workflow.party_size() && (false == eventNym.empty())) {
        workflow.add_party(eventNym);
    }

    return save_workflow(nymID, account, workflow);
}

auto Workflow::can_abort_transfer(const proto::PaymentWorkflow& workflow)
    -> bool
{
    bool correctState{false};

    switch (translate(workflow.state())) {
        case PaymentWorkflowState::Initiated: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_accept_cheque(const proto::PaymentWorkflow& workflow) -> bool
{
    bool correctState{false};

    switch (translate(workflow.state())) {
        case PaymentWorkflowState::Expired:
        case PaymentWorkflowState::Conveyed: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_accept_transfer(const proto::PaymentWorkflow& workflow)
    -> bool
{
    bool correctState{false};

    switch (translate(workflow.state())) {
        case PaymentWorkflowState::Conveyed: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_acknowledge_transfer(const proto::PaymentWorkflow& workflow)
    -> bool
{
    bool correctState{false};

    switch (translate(workflow.state())) {
        case PaymentWorkflowState::Initiated:
        case PaymentWorkflowState::Conveyed: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state (")(
            workflow.state())(")")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_cancel_cheque(const proto::PaymentWorkflow& workflow) -> bool
{
    bool correctState{false};

    switch (translate(workflow.state())) {
        case PaymentWorkflowState::Unsent:
        case PaymentWorkflowState::Conveyed: {
            correctState = true;
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_clear_transfer(const proto::PaymentWorkflow& workflow)
    -> bool
{
    bool correctState{false};

    switch (translate(workflow.type())) {
        case PaymentWorkflowType::OutgoingTransfer: {
            correctState =
                (PaymentWorkflowState::Acknowledged ==
                 translate(workflow.state()));
        } break;
        case PaymentWorkflowType::InternalTransfer: {
            correctState =
                (PaymentWorkflowState::Conveyed == translate(workflow.state()));
        } break;
        default: {
        }
    }

    if (false == correctState) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_complete_transfer(const proto::PaymentWorkflow& workflow)
    -> bool
{
    if (PaymentWorkflowState::Accepted != translate(workflow.state())) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state (")(
            workflow.state())(")")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_convey_cash(const proto::PaymentWorkflow& workflow) -> bool
{
    if (PaymentWorkflowState::Expired == translate(workflow.state())) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_convey_cheque(const proto::PaymentWorkflow& workflow) -> bool
{
    if (PaymentWorkflowState::Unsent != translate(workflow.state())) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_convey_transfer(const proto::PaymentWorkflow& workflow)
    -> bool
{
    switch (translate(workflow.state())) {
        case PaymentWorkflowState::Initiated:
        case PaymentWorkflowState::Acknowledged: {
            return true;
        }
        case PaymentWorkflowState::Conveyed: {
            break;
        }
        default: {
            LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
                .Flush();
        }
    }

    return false;
}

auto Workflow::can_deposit_cheque(const proto::PaymentWorkflow& workflow)
    -> bool
{
    if (PaymentWorkflowState::Conveyed != translate(workflow.state())) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_expire_cheque(
    const opentxs::Cheque& cheque,
    const proto::PaymentWorkflow& workflow) -> bool
{
    bool correctState{false};

    switch (translate(workflow.type())) {
        case PaymentWorkflowType::OutgoingCheque: {
            switch (translate(workflow.state())) {
                case PaymentWorkflowState::Unsent:
                case PaymentWorkflowState::Conveyed: {
                    correctState = true;
                } break;
                default: {
                }
            }
        } break;
        case PaymentWorkflowType::IncomingCheque: {
            switch (translate(workflow.state())) {
                case PaymentWorkflowState::Conveyed: {
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
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    if (Clock::now() < cheque.GetValidTo()) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Can not expire valid cheque.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::can_finish_cheque(const proto::PaymentWorkflow& workflow) -> bool
{
    if (PaymentWorkflowState::Accepted != translate(workflow.state())) {
        LogError()(OT_PRETTY_STATIC(Workflow))("Incorrect workflow state.")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::CancelCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {PaymentWorkflowType::OutgoingCheque}, nymID, cheque);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_cancel_cheque(*workflow)) { return false; }

    static const auto accountID = api_.Factory().Identifier();

    return add_cheque_event(
        lock,
        nymID,
        "",
        *workflow,
        PaymentWorkflowState::Cancelled,
        proto::PAYMENTEVENTTYPE_CANCEL,
        versions_.at(PaymentWorkflowType::OutgoingCheque).event_,
        request,
        reply,
        accountID);
}

auto Workflow::cheque_deposit_success(const Message* message) -> bool
{
    if (nullptr == message) { return false; }

    // TODO this might not be sufficient

    return message->m_bSuccess;
}

auto Workflow::ClearCheque(
    const identifier::Nym& recipientNymID,
    const OTTransaction& receipt) const -> bool
{
    if (recipientNymID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid cheque recipient").Flush();

        return false;
    }

    auto cheque{api_.Factory().InternalSession().Cheque(receipt)};

    if (false == bool(cheque)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load cheque from receipt.")
            .Flush();

        return false;
    }

    if (false == isCheque(*cheque)) { return false; }

    const auto nymID = cheque->GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {PaymentWorkflowType::OutgoingCheque}, nymID, *cheque);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_accept_cheque(*workflow)) { return false; }

    OT_ASSERT(1 == workflow->account_size())

    const bool needNym = (0 == workflow->party_size());
    const auto time = Clock::now();
    const auto output = add_cheque_event(
        lock,
        nymID,
        api_.Factory().Identifier(workflow->account(0)),
        *workflow,
        PaymentWorkflowState::Accepted,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        versions_.at(PaymentWorkflowType::OutgoingCheque).event_,
        recipientNymID,
        receipt,
        time);

    if (needNym) {
        update_activity(
            cheque->GetSenderNymID(),
            recipientNymID,
            Identifier::Factory(*cheque),
            Identifier::Factory(workflow->id()),
            otx::client::StorageBox::OUTGOINGCHEQUE,
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

auto Workflow::ClearTransfer(
    const identifier::Nym& nymID,
    const identifier::Notary& notaryID,
    const OTTransaction& receipt) const -> bool
{
    auto depositorNymID = identifier::Nym::Factory();
    const auto transfer =
        extract_transfer_from_receipt(receipt, depositorNymID);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Invalid transfer").Flush();

        return false;
    }

    if (depositorNymID->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing recipient").Flush();

        return false;
    }

    contact_.NymToContact(depositorNymID);
    const auto& accountID = transfer->GetPurportedAccountID();

    if (accountID.empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Transfer does not contain source account ID")
            .Flush();

        return false;
    }

    const auto& destinationAccountID = transfer->GetDestinationAcctID();

    if (destinationAccountID.empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Transfer does not contain destination account ID")
            .Flush();

        return false;
    }

    const bool isInternal = isInternalTransfer(accountID, destinationAccountID);
    const UnallocatedSet<PaymentWorkflowType> type{
        isInternal ? PaymentWorkflowType::InternalTransfer
                   : PaymentWorkflowType::OutgoingTransfer};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), *transfer);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_clear_transfer(*workflow)) { return false; }

    const auto output = add_transfer_event(
        lock,
        nymID.str(),
        notaryID.str(),
        (isInternal ? UnallocatedCString{""} : depositorNymID->str()),
        *workflow,
        PaymentWorkflowState::Accepted,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        (isInternal
             ? versions_.at(PaymentWorkflowType::InternalTransfer).event_
             : versions_.at(PaymentWorkflowType::OutgoingTransfer).event_),
        receipt,
        accountID,
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
            otx::client::StorageBox::OUTGOINGTRANSFER,
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
auto Workflow::CompleteTransfer(
    const identifier::Nym& nymID,
    const identifier::Notary& notaryID,
    const OTTransaction& receipt,
    const Message& reply) const -> bool
{
    auto depositorNymID = Identifier::Factory();
    const auto transfer =
        extract_transfer_from_receipt(receipt, depositorNymID);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Invalid transfer").Flush();

        return false;
    }

    const auto& accountID = transfer->GetPurportedAccountID();

    if (accountID.empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Transfer does not contain source account ID")
            .Flush();

        return false;
    }

    const auto& destinationAccountID = transfer->GetDestinationAcctID();

    if (destinationAccountID.empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Transfer does not contain destination account ID")
            .Flush();

        return false;
    }

    const bool isInternal = isInternalTransfer(accountID, destinationAccountID);
    const UnallocatedSet<PaymentWorkflowType> type{
        isInternal ? PaymentWorkflowType::InternalTransfer
                   : PaymentWorkflowType::OutgoingTransfer};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID.str(), *transfer);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_complete_transfer(*workflow)) { return false; }

    return add_transfer_event(
        lock,
        nymID.str(),
        notaryID.str(),
        (isInternal ? UnallocatedCString{""} : depositorNymID->str()),
        *workflow,
        PaymentWorkflowState::Completed,
        proto::PAYMENTEVENTTYPE_COMPLETE,
        (isInternal
             ? versions_.at(PaymentWorkflowType::InternalTransfer).event_
             : versions_.at(PaymentWorkflowType::OutgoingTransfer).event_),
        receipt,
        transfer->GetRealAccountID(),
        true);
}

// NOTE: Since this is an INCOMING transfer, then we need to CREATE its
// corresponding transfer workflow, since it does not already exist.
//
// (Whereas if this had been an INTERNAL transfer, then it would ALREADY
// have been created, and thus we'd need to GET the existing workflow, and
// then add the new event to it).
auto Workflow::convey_incoming_transfer(
    const identifier::Nym& nymID,
    const identifier::Notary& notaryID,
    const OTTransaction& pending,
    const UnallocatedCString& senderNymID,
    const UnallocatedCString& recipientNymID,
    const Item& transfer) const -> OTIdentifier
{
    Lock global(lock_);
    const auto existing = get_workflow(
        global, {PaymentWorkflowType::IncomingTransfer}, nymID.str(), transfer);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    const auto& accountID = pending.GetPurportedAccountID();
    const auto [workflowID, workflow] = create_transfer(
        global,
        nymID.str(),
        transfer,
        PaymentWorkflowType::IncomingTransfer,
        PaymentWorkflowState::Conveyed,
        versions_.at(PaymentWorkflowType::IncomingTransfer).workflow_,
        versions_.at(PaymentWorkflowType::IncomingTransfer).source_,
        versions_.at(PaymentWorkflowType::IncomingTransfer).event_,
        senderNymID,
        accountID,
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
            otx::client::StorageBox::INCOMINGTRANSFER,
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
auto Workflow::convey_internal_transfer(
    const identifier::Nym& nymID,
    const identifier::Notary& notaryID,
    const OTTransaction& pending,
    const UnallocatedCString& senderNymID,
    const Item& transfer) const -> OTIdentifier
{
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {PaymentWorkflowType::InternalTransfer}, nymID.str(), transfer);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer does not exist.")
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
        PaymentWorkflowState::Conveyed,
        proto::PAYMENTEVENTTYPE_CONVEY,
        versions_.at(PaymentWorkflowType::InternalTransfer).event_,
        pending,
        transfer.GetDestinationAcctID(),
        true);

    if (output) {
        return Identifier::Factory(workflow->id());
    } else {
        return Identifier::Factory();
    }
}

auto Workflow::ConveyTransfer(
    const identifier::Nym& nymID,
    const identifier::Notary& notaryID,
    const OTTransaction& pending) const -> OTIdentifier
{
    const auto transfer = extract_transfer_from_pending(pending);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Invalid transaction").Flush();

        return Identifier::Factory();
    }

    const auto senderNymID = transfer->GetNymID().str();
    contact_.NymToContact(transfer->GetNymID());
    const auto recipientNymID = pending.GetNymID().str();

    if (pending.GetNymID() != nymID) {
        LogError()(OT_PRETTY_CLASS())("Invalid recipient").Flush();

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

auto Workflow::create_cheque(
    const Lock& lock,
    const UnallocatedCString& nymID,
    const opentxs::Cheque& cheque,
    const PaymentWorkflowType workflowType,
    const PaymentWorkflowState workflowState,
    const VersionNumber workflowVersion,
    const VersionNumber sourceVersion,
    const VersionNumber eventVersion,
    const UnallocatedCString& party,
    const Identifier& account,
    const Message* message) const
    -> std::pair<OTIdentifier, proto::PaymentWorkflow>
{
    OT_ASSERT(verify_lock(lock))

    std::pair<OTIdentifier, proto::PaymentWorkflow> output{
        Identifier::Factory(), {}};
    auto& [workflowID, workflow] = output;
    const auto chequeID = Identifier::Factory(cheque);
    const UnallocatedCString serialized = String::Factory(cheque)->Get();
    workflowID = Identifier::Random();
    workflow.set_version(workflowVersion);
    workflow.set_id(workflowID->str());
    workflow.set_type(translate(workflowType));
    workflow.set_state(translate(workflowState));
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
        event.set_time(Clock::to_time_t(Clock::now()));

        if (PaymentWorkflowState::Unsent == workflowState) {
            event.set_type(proto::PAYMENTEVENTTYPE_CREATE);
            event.set_method(proto::TRANSPORTMETHOD_NONE);
        } else if (PaymentWorkflowState::Conveyed == workflowState) {
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
        workflow.add_account(account.str());
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

auto Workflow::create_transfer(
    const Lock& global,
    const UnallocatedCString& nymID,
    const Item& transfer,
    const PaymentWorkflowType workflowType,
    const PaymentWorkflowState workflowState,
    const VersionNumber workflowVersion,
    const VersionNumber sourceVersion,
    const VersionNumber eventVersion,
    const UnallocatedCString& party,
    const Identifier& account,
    const UnallocatedCString& notaryID,
    const UnallocatedCString& destinationAccountID) const
    -> std::pair<OTIdentifier, proto::PaymentWorkflow>
{
    OT_ASSERT(verify_lock(global))
    OT_ASSERT(false == nymID.empty());
    OT_ASSERT(false == account.empty());
    OT_ASSERT(false == notaryID.empty());

    std::pair<OTIdentifier, proto::PaymentWorkflow> output{
        Identifier::Factory(), {}};
    auto& [workflowID, workflow] = output;
    const auto transferID = Identifier::Factory(transfer);
    LogVerbose()(OT_PRETTY_CLASS())("Transfer ID: ")(transferID).Flush();
    const UnallocatedCString serialized = String::Factory(transfer)->Get();
    const auto existing = get_workflow(global, {workflowType}, nymID, transfer);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer already exists.")
            .Flush();
        workflowID = Identifier::Factory(existing->id());

        return output;
    }

    workflowID = Identifier::Random();
    workflow.set_version(workflowVersion);
    workflow.set_id(workflowID->str());
    workflow.set_type(translate(workflowType));
    workflow.set_state(translate(workflowState));
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
    event.set_time(Clock::to_time_t(Clock::now()));

    if (PaymentWorkflowState::Initiated == workflowState) {
        event.set_type(proto::PAYMENTEVENTTYPE_CREATE);
        event.set_method(proto::TRANSPORTMETHOD_OT);
    } else if (PaymentWorkflowState::Conveyed == workflowState) {
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
        workflow.add_account(account.str());

        if (false == destinationAccountID.empty()) {
            workflow.add_account(destinationAccountID);
        }
    }

    return save_workflow(std::move(output), nymID, account, workflow);
}

// Creates outgoing and internal transfer workflows.
auto Workflow::CreateTransfer(const Item& transfer, const Message& request)
    const -> OTIdentifier
{
    if (false == isTransfer(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Invalid item type on object").Flush();

        return Identifier::Factory();
    }

    const String& senderNymID = request.m_strNymID;
    const auto& accountID = transfer.GetRealAccountID();
    const bool isInternal =
        isInternalTransfer(accountID, transfer.GetDestinationAcctID());
    Lock global(lock_);
    const auto existing = get_workflow(
        global,
        {isInternal ? PaymentWorkflowType::InternalTransfer
                    : PaymentWorkflowType::OutgoingTransfer},
        senderNymID.Get(),
        transfer);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    const auto [workflowID, workflow] = create_transfer(
        global,
        senderNymID.Get(),
        transfer,
        (isInternal ? PaymentWorkflowType::InternalTransfer
                    : PaymentWorkflowType::OutgoingTransfer),
        PaymentWorkflowState::Initiated,
        (isInternal
             ? versions_.at(PaymentWorkflowType::InternalTransfer).workflow_
             : versions_.at(PaymentWorkflowType::OutgoingTransfer).workflow_),
        (isInternal
             ? versions_.at(PaymentWorkflowType::InternalTransfer).source_
             : versions_.at(PaymentWorkflowType::OutgoingTransfer).source_),
        (isInternal
             ? versions_.at(PaymentWorkflowType::InternalTransfer).event_
             : versions_.at(PaymentWorkflowType::OutgoingTransfer).event_),
        "",
        accountID,
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

auto Workflow::DepositCheque(
    const identifier::Nym& receiver,
    const Identifier& accountID,
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = receiver.str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {PaymentWorkflowType::IncomingCheque}, nymID, cheque);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this cheque does not exist.")
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
        PaymentWorkflowState::Completed,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        versions_.at(PaymentWorkflowType::IncomingCheque).event_,
        request,
        reply,
        accountID);

    if (output && cheque_deposit_success(reply)) {
        update_rpc(

            receiver.str(),
            cheque.GetSenderNymID().str(),
            accountID.str(),
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflow->id(),
            cheque.GetAmount(),
            0,
            Clock::from_time_t(reply->m_lTime),
            cheque.GetMemo().Get());
    }

    return output;
}

auto Workflow::ExpireCheque(
    const identifier::Nym& nym,
    const opentxs::Cheque& cheque) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = nym.str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global,
        {PaymentWorkflowType::OutgoingCheque,
         PaymentWorkflowType::IncomingCheque},
        nymID,
        cheque);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_expire_cheque(cheque, *workflow)) { return false; }

    workflow->set_state(translate(PaymentWorkflowState::Expired));

    return save_workflow(nymID, cheque.GetSenderAcctID(), *workflow);
}

auto Workflow::ExportCheque(const opentxs::Cheque& cheque) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(global, {}, nymID, cheque);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_convey_cheque(*workflow)) { return false; }

    workflow->set_state(translate(PaymentWorkflowState::Conveyed));
    auto& event = *(workflow->add_event());
    event.set_version(versions_.at(PaymentWorkflowType::OutgoingCheque).event_);
    event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
    event.set_time(Clock::to_time_t(Clock::now()));
    event.set_method(proto::TRANSPORTMETHOD_OOB);
    event.set_success(true);

    return save_workflow(nymID, cheque.GetSenderAcctID(), *workflow);
}

auto Workflow::extract_conveyed_time(const proto::PaymentWorkflow& workflow)
    -> Time
{
    for (const auto& event : workflow.event()) {
        if (proto::PAYMENTEVENTTYPE_CONVEY == event.type()) {
            if (event.success()) { return Clock::from_time_t(event.time()); }
        }
    }

    return {};
}

auto Workflow::extract_transfer_from_pending(const OTTransaction& receipt) const
    -> std::unique_ptr<Item>
{
    if (transactionType::pending != receipt.GetType()) {
        LogError()(OT_PRETTY_CLASS())("Incorrect receipt type: ")(
            receipt.GetTypeString())
            .Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    receipt.GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing serialized transfer item")
            .Flush();

        return nullptr;
    }

    auto transfer = api_.Factory().InternalSession().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Unable to instantiate transfer item")
            .Flush();

        return nullptr;
    }

    if (itemType::transfer != transfer->GetType()) {
        LogError()(OT_PRETTY_CLASS())("Invalid transfer item type.").Flush();

        return nullptr;
    }

    return transfer;
}

auto Workflow::extract_transfer_from_receipt(
    const OTTransaction& receipt,
    Identifier& depositorNymID) const -> std::unique_ptr<Item>
{
    if (transactionType::transferReceipt != receipt.GetType()) {
        if (transactionType::pending == receipt.GetType()) {
            return extract_transfer_from_pending(receipt);
        } else {
            LogError()(OT_PRETTY_CLASS())("Incorrect receipt type: ")(
                receipt.GetTypeString())
                .Flush();

            return nullptr;
        }
    }

    auto serializedAcceptPending = String::Factory();
    receipt.GetReferenceString(serializedAcceptPending);

    if (serializedAcceptPending->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing serialized accept pending item")
            .Flush();

        return nullptr;
    }

    const auto acceptPending =
        api_.Factory().InternalSession().Item(serializedAcceptPending);

    if (false == bool(acceptPending)) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to instantiate accept pending item")
            .Flush();

        return nullptr;
    }

    if (itemType::acceptPending != acceptPending->GetType()) {
        LogError()(OT_PRETTY_CLASS())("Invalid accept pending item type.")
            .Flush();

        return nullptr;
    }

    depositorNymID.Assign(acceptPending->GetNymID());
    auto serializedPending = String::Factory();
    acceptPending->GetAttachment(serializedPending);

    if (serializedPending->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing serialized pending transaction")
            .Flush();

        return nullptr;
    }

    auto pending = api_.Factory().InternalSession().Transaction(
        receipt.GetNymID(),
        receipt.GetRealAccountID(),
        receipt.GetRealNotaryID());

    if (false == bool(pending)) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to instantiate pending transaction")
            .Flush();

        return nullptr;
    }

    const bool loaded = pending->LoadContractFromString(serializedPending);

    if (false == loaded) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to deserialize pending transaction")
            .Flush();

        return nullptr;
    }

    if (transactionType::pending != pending->GetType()) {
        LogError()(OT_PRETTY_CLASS())("Invalid pending transaction type.")
            .Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    pending->GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing serialized transfer item")
            .Flush();

        return nullptr;
    }

    auto transfer = api_.Factory().InternalSession().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Unable to instantiate transfer item")
            .Flush();

        return nullptr;
    }

    if (itemType::transfer != transfer->GetType()) {
        LogError()(OT_PRETTY_CLASS())("Invalid transfer item type.").Flush();

        return nullptr;
    }

    return transfer;
}

auto Workflow::FinishCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {PaymentWorkflowType::OutgoingCheque}, nymID, cheque);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_finish_cheque(*workflow)) { return false; }

    static const auto accountID = api_.Factory().Identifier();

    return add_cheque_event(
        lock,
        nymID,
        "",
        *workflow,
        PaymentWorkflowState::Completed,
        proto::PAYMENTEVENTTYPE_COMPLETE,
        versions_.at(PaymentWorkflowType::OutgoingCheque).event_,
        request,
        reply,
        accountID);
}

template <typename T>
auto Workflow::get_workflow(
    const Lock& global,
    const UnallocatedSet<PaymentWorkflowType>& types,
    const UnallocatedCString& nymID,
    const T& source) const -> std::shared_ptr<proto::PaymentWorkflow>
{
    OT_ASSERT(verify_lock(global));

    const auto itemID = Identifier::Factory(source)->str();
    LogVerbose()(OT_PRETTY_CLASS())("Item ID: ")(itemID).Flush();

    return get_workflow_by_source(types, nymID, itemID);
}

auto Workflow::get_workflow_by_id(
    const UnallocatedCString& nymID,
    const UnallocatedCString& workflowID) const
    -> std::shared_ptr<proto::PaymentWorkflow>
{
    auto output = std::make_shared<proto::PaymentWorkflow>();

    OT_ASSERT(output);

    if (false == api_.Storage().Load(nymID, workflowID, *output)) {
        LogDetail()(OT_PRETTY_CLASS())("Workflow ")(workflowID)(" for nym ")(
            nymID)(" can not be loaded")
            .Flush();

        return {};
    }

    return output;
}

auto Workflow::get_workflow_by_id(
    const UnallocatedSet<PaymentWorkflowType>& types,
    const UnallocatedCString& nymID,
    const UnallocatedCString& workflowID) const
    -> std::shared_ptr<proto::PaymentWorkflow>
{
    auto output = get_workflow_by_id(nymID, workflowID);

    if (0 == types.count(translate(output->type()))) {
        LogError()(OT_PRETTY_CLASS())("Incorrect type (")(output->type())(
            ") on workflow ")(workflowID)(" for nym ")(nymID)
            .Flush();

        return {nullptr};
    }

    return output;
}

auto Workflow::get_workflow_by_source(
    const UnallocatedSet<PaymentWorkflowType>& types,
    const UnallocatedCString& nymID,
    const UnallocatedCString& sourceID) const
    -> std::shared_ptr<proto::PaymentWorkflow>
{
    const auto workflowID =
        api_.Storage().PaymentWorkflowLookup(nymID, sourceID);

    if (workflowID.empty()) { return {}; }

    return get_workflow_by_id(types, nymID, workflowID);
}

auto Workflow::get_workflow_lock(Lock& global, const UnallocatedCString& id)
    const -> eLock
{
    OT_ASSERT(verify_lock(global));

    auto output = eLock(workflow_locks_[id]);
    global.unlock();

    return output;
}

auto Workflow::ImportCheque(
    const identifier::Nym& nymID,
    const opentxs::Cheque& cheque) const -> OTIdentifier
{
    if (false == isCheque(cheque)) { return Identifier::Factory(); }

    if (false == validate_recipient(nymID, cheque)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(
            nymID)(" can not deposit this cheque.")
            .Flush();

        return Identifier::Factory();
    }

    Lock global(lock_);
    const auto existing = get_workflow(
        global, {PaymentWorkflowType::IncomingCheque}, nymID.str(), cheque);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())("Workflow for this cheque already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    const UnallocatedCString party = cheque.GetSenderNymID().str();
    static const auto accountID = api_.Factory().Identifier();
    const auto [workflowID, workflow] = create_cheque(
        global,
        nymID.str(),
        cheque,
        PaymentWorkflowType::IncomingCheque,
        PaymentWorkflowState::Conveyed,
        versions_.at(PaymentWorkflowType::IncomingCheque).workflow_,
        versions_.at(PaymentWorkflowType::IncomingCheque).source_,
        versions_.at(PaymentWorkflowType::IncomingCheque).event_,
        party,
        accountID);

    if (false == workflowID->empty()) {
        const auto time = extract_conveyed_time(workflow);
        update_activity(
            nymID,
            cheque.GetSenderNymID(),
            Identifier::Factory(cheque),
            workflowID,
            otx::client::StorageBox::INCOMINGCHEQUE,
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

auto Workflow::InstantiateCheque(
    const identifier::Nym& nym,
    const Identifier& id) const -> Cheque
{
    try {
        const auto workflow = [&] {
            auto out = proto::PaymentWorkflow{};

            if (false == LoadWorkflow(nym, id, out)) {
                throw std::runtime_error{
                    UnallocatedCString{"Workflow "} + id.str() + " not found"};
            }

            return out;
        }();

        if (false == ContainsCheque(workflow)) {

            throw std::runtime_error{
                UnallocatedCString{"Workflow "} + id.str() +
                " does not contain a cheque"};
        }

        return session::Workflow::InstantiateCheque(api_, workflow);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto Workflow::InstantiatePurse(
    const identifier::Nym& nym,
    const Identifier& id) const -> Purse
{
    try {
        const auto workflow = [&] {
            auto out = proto::PaymentWorkflow{};

            if (false == LoadWorkflow(nym, id, out)) {
                throw std::runtime_error{
                    UnallocatedCString{"Workflow "} + id.str() + " not found"};
            }

            return out;
        }();

        return session::Workflow::InstantiatePurse(api_, workflow);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto Workflow::isCheque(const opentxs::Cheque& cheque) -> bool
{
    if (cheque.HasRemitter()) {
        LogError()(OT_PRETTY_STATIC(Workflow))(
            "Provided instrument is a voucher")
            .Flush();

        return false;
    }

    if (0 > cheque.GetAmount()) {
        LogError()(OT_PRETTY_STATIC(Workflow))(
            "Provided instrument is an invoice")
            .Flush();

        return false;
    }

    if (0 == cheque.GetAmount()) {
        LogError()(OT_PRETTY_STATIC(Workflow))(
            "Provided instrument is a cancellation")
            .Flush();

        return false;
    }

    return true;
}

auto Workflow::isInternalTransfer(
    const Identifier& sourceAccount,
    const Identifier& destinationAccount) const -> bool
{
    const auto ownerNymID = api_.Storage().AccountOwner(sourceAccount);

    OT_ASSERT(false == ownerNymID->empty());

    const auto recipientNymID = api_.Storage().AccountOwner(destinationAccount);

    if (recipientNymID->empty()) { return false; }

    return ownerNymID == recipientNymID;
}

auto Workflow::isTransfer(const Item& item) -> bool
{
    return itemType::transfer == item.GetType();
}

auto Workflow::List(
    const identifier::Nym& nymID,
    const PaymentWorkflowType type,
    const PaymentWorkflowState state) const -> UnallocatedSet<OTIdentifier>
{
    const auto input =
        api_.Storage().PaymentWorkflowsByState(nymID.str(), type, state);
    UnallocatedSet<OTIdentifier> output{};
    std::transform(
        input.begin(),
        input.end(),
        std::inserter(output, output.end()),
        [](const UnallocatedCString& id) -> OTIdentifier {
            return Identifier::Factory(id);
        });

    return output;
}

auto Workflow::LoadCheque(
    const identifier::Nym& nymID,
    const Identifier& chequeID) const -> Workflow::Cheque
{
    auto workflow = get_workflow_by_source(
        {PaymentWorkflowType::OutgoingCheque,
         PaymentWorkflowType::IncomingCheque},
        nymID.str(),
        chequeID.str());

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this cheque does not exist.")
            .Flush();

        return {};
    }

    return session::Workflow::InstantiateCheque(api_, *workflow);
}

auto Workflow::LoadChequeByWorkflow(
    const identifier::Nym& nymID,
    const Identifier& workflowID) const -> Workflow::Cheque
{
    auto workflow = get_workflow_by_id(
        {PaymentWorkflowType::OutgoingCheque,
         PaymentWorkflowType::IncomingCheque},
        nymID.str(),
        workflowID.str());

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this cheque does not exist.")
            .Flush();

        return {};
    }

    return session::Workflow::InstantiateCheque(api_, *workflow);
}

auto Workflow::LoadTransfer(
    const identifier::Nym& nymID,
    const Identifier& transferID) const -> Workflow::Transfer
{
    auto workflow = get_workflow_by_source(
        {PaymentWorkflowType::OutgoingTransfer,
         PaymentWorkflowType::IncomingTransfer,
         PaymentWorkflowType::InternalTransfer},
        nymID.str(),
        transferID.str());

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer does not exist.")
            .Flush();

        return {};
    }

    return InstantiateTransfer(api_, *workflow);
}

auto Workflow::LoadTransferByWorkflow(
    const identifier::Nym& nymID,
    const Identifier& workflowID) const -> Workflow::Transfer
{
    auto workflow = get_workflow_by_id(
        {PaymentWorkflowType::OutgoingTransfer,
         PaymentWorkflowType::IncomingTransfer,
         PaymentWorkflowType::InternalTransfer},
        nymID.str(),
        workflowID.str());

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer does not exist.")
            .Flush();

        return {};
    }

    return InstantiateTransfer(api_, *workflow);
}

auto Workflow::LoadWorkflow(
    const identifier::Nym& nymID,
    const Identifier& workflowID,
    proto::PaymentWorkflow& out) const -> bool
{
    auto pWorkflow = get_workflow_by_id(nymID.str(), workflowID.str());

    if (pWorkflow) {
        out = *pWorkflow;

        return true;
    } else {

        return false;
    }
}

auto Workflow::ReceiveCash(
    const identifier::Nym& receiver,
    const otx::blind::Purse& purse,
    const Message& message) const -> OTIdentifier
{
    Lock global(lock_);
    const UnallocatedCString serialized = String::Factory(message)->Get();
    const UnallocatedCString party = message.m_strNymID->Get();
    auto workflowID = Identifier::Random();
    proto::PaymentWorkflow workflow{};
    workflow.set_version(
        versions_.at(PaymentWorkflowType::IncomingCash).workflow_);
    workflow.set_id(workflowID->str());
    workflow.set_type(translate(PaymentWorkflowType::IncomingCash));
    workflow.set_state(translate(PaymentWorkflowState::Conveyed));
    auto& source = *(workflow.add_source());
    source.set_version(versions_.at(PaymentWorkflowType::IncomingCash).source_);
    source.set_id(workflowID->str());
    source.set_revision(1);
    source.set_item([&] {
        auto proto = proto::Purse{};
        purse.Internal().Serialize(proto);

        return proto::ToString(proto);
    }());
    workflow.set_notary(purse.Notary().str());
    auto& event = *workflow.add_event();
    event.set_version(versions_.at(PaymentWorkflowType::IncomingCash).event_);
    event.set_time(message.m_lTime);
    event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(message.m_strNotaryID->Get());
    event.add_item(serialized);
    event.set_nym(party);
    event.set_success(true);
    workflow.add_unit(purse.Unit().str());
    workflow.add_party(party);
    const auto saved = save_workflow(receiver.str(), workflow);

    if (false == saved) {
        LogError()(OT_PRETTY_CLASS())("Failed to save workflow").Flush();

        return Identifier::Factory();
    }

    return workflowID;
}

auto Workflow::ReceiveCheque(
    const identifier::Nym& nymID,
    const opentxs::Cheque& cheque,
    const Message& message) const -> OTIdentifier
{
    if (false == isCheque(cheque)) { return Identifier::Factory(); }

    if (false == validate_recipient(nymID, cheque)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(
            nymID)(" can not deposit this cheque.")
            .Flush();

        return Identifier::Factory();
    }

    Lock global(lock_);
    const auto existing = get_workflow(
        global, {PaymentWorkflowType::IncomingCheque}, nymID.str(), cheque);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())("Workflow for this cheque already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    const UnallocatedCString party = cheque.GetSenderNymID().str();
    static const auto accountID = api_.Factory().Identifier();
    const auto [workflowID, workflow] = create_cheque(
        global,
        nymID.str(),
        cheque,
        PaymentWorkflowType::IncomingCheque,
        PaymentWorkflowState::Conveyed,
        versions_.at(PaymentWorkflowType::IncomingCheque).workflow_,
        versions_.at(PaymentWorkflowType::IncomingCheque).source_,
        versions_.at(PaymentWorkflowType::IncomingCheque).event_,
        party,
        accountID,
        &message);

    if (false == workflowID->empty()) {
        const auto time = extract_conveyed_time(workflow);
        update_activity(
            nymID,
            cheque.GetSenderNymID(),
            Identifier::Factory(cheque),
            workflowID,
            otx::client::StorageBox::INCOMINGCHEQUE,
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

auto Workflow::save_workflow(
    const UnallocatedCString& nymID,
    const proto::PaymentWorkflow& workflow) const -> bool
{
    static const auto id = api_.Factory().Identifier();

    return save_workflow(nymID, id, workflow);
}

auto Workflow::save_workflow(
    const UnallocatedCString& nymID,
    const Identifier& accountID,
    const proto::PaymentWorkflow& workflow) const -> bool
{
    const bool valid = proto::Validate(workflow, VERBOSE);

    OT_ASSERT(valid)

    const auto saved = api_.Storage().Store(nymID, workflow);

    OT_ASSERT(saved)

    if (false == accountID.empty()) {
        account_publisher_->Send([&] {
            auto work = opentxs::network::zeromq::tagged_message(
                WorkType::WorkflowAccountUpdate);
            work.AddFrame(accountID);

            return work;
        }());
    }

    return valid && saved;
}

auto Workflow::save_workflow(
    OTIdentifier&& output,
    const UnallocatedCString& nymID,
    const Identifier& accountID,
    const proto::PaymentWorkflow& workflow) const -> OTIdentifier
{
    if (save_workflow(nymID, accountID, workflow)) { return std::move(output); }

    return Identifier::Factory();
}

auto Workflow::save_workflow(
    std::pair<OTIdentifier, proto::PaymentWorkflow>&& output,
    const UnallocatedCString& nymID,
    const Identifier& accountID,
    const proto::PaymentWorkflow& workflow) const
    -> std::pair<OTIdentifier, proto::PaymentWorkflow>
{
    if (save_workflow(nymID, accountID, workflow)) { return std::move(output); }

    return {Identifier::Factory(), {}};
}

auto Workflow::SendCash(
    const identifier::Nym& sender,
    const identifier::Nym& recipient,
    const Identifier& workflowID,
    const Message& request,
    const Message* reply) const -> bool
{
    Lock global(lock_);
    const auto pWorkflow = get_workflow_by_id(sender.str(), workflowID.str());

    if (false == bool(pWorkflow)) {
        LogError()(OT_PRETTY_CLASS())("Workflow ")(
            workflowID)(" does not exist.")
            .Flush();

        return false;
    }

    auto& workflow = *pWorkflow;
    auto lock = get_workflow_lock(global, workflowID.str());

    if (false == can_convey_cash(workflow)) { return false; }

    const bool haveReply = (nullptr != reply);

    if (haveReply) {
        workflow.set_state(translate(PaymentWorkflowState::Conveyed));
    }

    auto& event = *(workflow.add_event());
    event.set_version(versions_.at(PaymentWorkflowType::OutgoingCash).event_);
    event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
    event.add_item(String::Factory(request)->Get());
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(request.m_strNotaryID->Get());
    event.set_nym(request.m_strNymID2->Get());

    if (haveReply) {
        event.add_item(String::Factory(*reply)->Get());
        event.set_time(reply->m_lTime);
        event.set_success(reply->m_bSuccess);
    } else {
        event.set_time(request.m_lTime);
        event.set_success(false);
    }

    if (0 == workflow.party_size()) { workflow.add_party(recipient.str()); }

    return save_workflow(sender.str(), workflow);
}

auto Workflow::SendCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {PaymentWorkflowType::OutgoingCheque}, nymID, cheque);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this cheque does not exist.")
            .Flush();

        return false;
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_convey_cheque(*workflow)) { return false; }

    static const auto accountID = api_.Factory().Identifier();

    return add_cheque_event(
        lock,
        nymID,
        request.m_strNymID2->Get(),
        *workflow,
        PaymentWorkflowState::Conveyed,
        proto::PAYMENTEVENTTYPE_CONVEY,
        versions_.at(PaymentWorkflowType::OutgoingCheque).event_,
        request,
        reply,
        accountID);
}

auto Workflow::WorkflowParty(
    const identifier::Nym& nymID,
    const Identifier& workflowID,
    const int index) const -> const UnallocatedCString
{
    auto workflow = get_workflow_by_id(nymID.str(), workflowID.str());

    if (false == bool{workflow}) return {};

    return workflow->party(index);
}

auto Workflow::WorkflowPartySize(
    const identifier::Nym& nymID,
    const Identifier& workflowID,
    int& partysize) const -> bool
{
    auto workflow = get_workflow_by_id(nymID.str(), workflowID.str());

    if (false == bool{workflow}) { return false; }

    partysize = workflow->party_size();

    return true;
}

auto Workflow::WorkflowState(
    const identifier::Nym& nymID,
    const Identifier& workflowID) const -> PaymentWorkflowState
{
    auto workflow = get_workflow_by_id(nymID.str(), workflowID.str());

    if (false == bool{workflow}) { return PaymentWorkflowState::Error; }

    return translate(workflow->state());
}

auto Workflow::WorkflowType(
    const identifier::Nym& nymID,
    const Identifier& workflowID) const -> PaymentWorkflowType
{
    auto workflow = get_workflow_by_id(nymID.str(), workflowID.str());

    if (false == bool{workflow}) { return PaymentWorkflowType::Error; }

    return translate(workflow->type());
}

auto Workflow::update_activity(
    const identifier::Nym& localNymID,
    const identifier::Nym& remoteNymID,
    const Identifier& sourceID,
    const Identifier& workflowID,
    const otx::client::StorageBox type,
    Time time) const -> bool
{
    const auto contactID = contact_.ContactID(remoteNymID);

    if (contactID->empty()) {
        LogError()(OT_PRETTY_CLASS())("Contact for nym ")(
            remoteNymID)(" does not exist")
            .Flush();

        return false;
    }

    const bool added = activity_.AddPaymentEvent(
        localNymID, contactID, type, sourceID, workflowID, time);

    if (added) {
        LogDetail()(OT_PRETTY_CLASS())(
            "Success adding payment event to thread ")(contactID->str())
            .Flush();

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to add payment event to thread ")(
            contactID->str())
            .Flush();

        return false;
    }
}

void Workflow::update_rpc(
    const UnallocatedCString& localNymID,
    const UnallocatedCString& remoteNymID,
    const UnallocatedCString& accountID,
    const proto::AccountEventType type,
    const UnallocatedCString& workflowID,
    const Amount amount,
    const Amount pending,
    const Time time,
    const UnallocatedCString& memo) const
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
            contact_.NymToContact(identifier::Nym::Factory(remoteNymID))
                ->str());
    }

    event.set_workflow(workflowID);
    amount.Serialize(writer(event.mutable_amount()));
    pending.Serialize(writer(event.mutable_pendingamount()));
    event.set_timestamp(Clock::to_time_t(time));
    event.set_memo(memo);

    OT_ASSERT(proto::Validate(push, VERBOSE));

    auto message = zmq::Message{};
    message.StartBody();
    message.AddFrame(localNymID);
    message.Internal().AddFrame(push);
    message.AddFrame(api_.Instance());
    rpc_publisher_->Send(std::move(message));
}

auto Workflow::validate_recipient(
    const identifier::Nym& nymID,
    const opentxs::Cheque& cheque) -> bool
{
    if (nymID.empty()) { return true; }

    return (nymID == cheque.GetRecipientNymID());
}

auto Workflow::WorkflowsByAccount(
    const identifier::Nym& nymID,
    const Identifier& accountID) const -> UnallocatedVector<OTIdentifier>
{
    UnallocatedVector<OTIdentifier> output{};
    const auto workflows =
        api_.Storage().PaymentWorkflowsByAccount(nymID.str(), accountID.str());
    std::transform(
        workflows.begin(),
        workflows.end(),
        std::inserter(output, output.end()),
        [](const UnallocatedCString& id) -> OTIdentifier {
            return Identifier::Factory(id);
        });

    return output;
}

auto Workflow::WriteCheque(const opentxs::Cheque& cheque) const -> OTIdentifier
{
    if (false == isCheque(cheque)) {
        LogError()(OT_PRETTY_STATIC(Workflow))(
            "Invalid item type on cheque object")
            .Flush();

        return Identifier::Factory();
    }

    const auto nymID = cheque.GetSenderNymID().str();
    Lock global(lock_);
    const auto existing = get_workflow(
        global, {PaymentWorkflowType::OutgoingCheque}, nymID, cheque);

    if (existing) {
        LogError()(OT_PRETTY_STATIC(Workflow))(
            "Workflow for this cheque already exist.")
            .Flush();

        return Identifier::Factory(existing->id());
    }

    if (cheque.HasRecipient()) {
        const auto& recipient = cheque.GetRecipientNymID();
        const auto contactID = contact_.ContactID(recipient);

        if (contactID->empty()) {
            LogError()(OT_PRETTY_CLASS())(
                "No contact exists for recipient nym ")(recipient)
                .Flush();

            return Identifier::Factory();
        }
    }

    const UnallocatedCString party =
        cheque.HasRecipient() ? cheque.GetRecipientNymID().str() : "";
    const auto [workflowID, workflow] = create_cheque(
        global,
        nymID,
        cheque,
        PaymentWorkflowType::OutgoingCheque,
        PaymentWorkflowState::Unsent,
        versions_.at(PaymentWorkflowType::OutgoingCheque).workflow_,
        versions_.at(PaymentWorkflowType::OutgoingCheque).source_,
        versions_.at(PaymentWorkflowType::OutgoingCheque).event_,
        party,
        cheque.GetSenderAcctID());
    global.unlock();
    const bool haveWorkflow = (false == workflowID->empty());
    const auto time{Clock::from_time_t(workflow.event(0).time())};

    if (haveWorkflow && cheque.HasRecipient()) {
        update_activity(
            cheque.GetSenderNymID(),
            cheque.GetRecipientNymID(),
            Identifier::Factory(cheque),
            workflowID,
            otx::client::StorageBox::OUTGOINGCHEQUE,
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
}  // namespace opentxs::api::session::imp
