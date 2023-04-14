// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/session/Workflow.hpp"  // IWYU pragma: associated

#include <AccountEvent.pb.h>
#include <InstrumentRevision.pb.h>
#include <PaymentEvent.pb.h>
#include <PaymentWorkflow.pb.h>
#include <PaymentWorkflowEnums.pb.h>
#include <Purse.pb.h>
#include <RPCEnums.pb.h>
#include <RPCPush.pb.h>
#include <algorithm>
#include <chrono>
#include <compare>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Types.hpp"
#include "internal/core/String.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/PaymentWorkflow.hpp"
#include "internal/serialization/protobuf/verify/RPCPush.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/PaymentWorkflowType.hpp"   // IWYU pragma: keep
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"

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

            OT_ASSERT(cheque);

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
    const proto::PaymentWorkflow& workflow) -> identifier::Generic
{
    auto output = identifier::Generic{};
    auto notaryID = identifier::Generic{};
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
        case PaymentWorkflowType::Error:
        default: {
            LogError()(OT_PRETTY_STATIC(Workflow))("Unknown workflow type")
                .Flush();
        }
    }

    return UUID(api, notaryID, number);
}

auto Workflow::UUID(
    const api::Session& api,
    const identifier::Generic& notary,
    const TransactionNumber& number) -> identifier::Generic
{
    LogTrace()(OT_PRETTY_STATIC(Workflow))("UUID for notary ")(
        notary)(" and transaction number ")(number)(" is ");
    auto preimage = api.Factory().Data();
    preimage.Assign(notary);
    preimage.Concatenate(&number, sizeof(number));

    return api.Factory().IdentifierFromPreimage(preimage.Bytes());
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
    , account_publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
    , rpc_publisher_(api_.Network().ZeroMQ().Internal().PushSocket(
          zmq::socket::Direction::Connect))
    , workflow_locks_()
{
    // WARNING: do not access api_.Wallet() during construction
    const auto endpoint = api_.Endpoints().WorkflowAccountUpdate();
    LogDetail()(OT_PRETTY_CLASS())("Binding to ")(endpoint.data()).Flush();
    auto bound = account_publisher_->Start(endpoint.data());

    OT_ASSERT(bound);

    bound =
        rpc_publisher_->Start(opentxs::network::zeromq::MakeDeterministicInproc(
            "rpc/push/internal", -1, 1));

    OT_ASSERT(bound);
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
    const auto workflow = get_workflow(global, type, nymID, transfer);

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
        nymID,
        {},
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

    const auto& senderNymID = transfer->GetNymID();
    const auto& recipientNymID = pending.GetNymID();
    const auto& accountID = pending.GetPurportedAccountID();

    if (pending.GetNymID() != nymID) {
        LogError()(OT_PRETTY_CLASS())("Invalid recipient").Flush();

        return false;
    }

    const bool isInternal = (senderNymID == recipientNymID);

    // Ignore this event for internal transfers.
    if (isInternal) { return true; }

    const UnallocatedSet<PaymentWorkflowType> type{
        PaymentWorkflowType::IncomingTransfer};
    Lock global(lock_);
    const auto workflow = get_workflow(global, type, nymID, *transfer);

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
        nymID,
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
    const auto workflow = get_workflow(global, type, nymID, transfer);

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
        nymID,
        {},
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
    const otx::blind::Purse& purse) const -> identifier::Generic
{
    Lock global(lock_);
    auto workflowID = api_.Factory().IdentifierFromRandom();
    proto::PaymentWorkflow workflow{};
    workflow.set_version(
        versions_.at(otx::client::PaymentWorkflowType::OutgoingCash).workflow_);
    workflow.set_id(workflowID.asBase58(api_.Crypto()));
    workflow.set_type(translate(PaymentWorkflowType::OutgoingCash));
    workflow.set_state(translate(PaymentWorkflowState::Unsent));
    auto& source = *(workflow.add_source());
    source.set_version(versions_.at(PaymentWorkflowType::OutgoingCash).source_);
    source.set_id(workflowID.asBase58(api_.Crypto()));
    source.set_revision(1);
    source.set_item([&] {
        auto proto = proto::Purse{};
        purse.Internal().Serialize(proto);

        return proto::ToString(proto);
    }());
    workflow.set_notary(purse.Notary().asBase58(api_.Crypto()));
    auto& event = *workflow.add_event();
    event.set_version(versions_.at(PaymentWorkflowType::OutgoingCash).event_);
    event.set_time(Clock::to_time_t(Clock::now()));
    event.set_type(proto::PAYMENTEVENTTYPE_CREATE);
    event.set_method(proto::TRANSPORTMETHOD_NONE);
    event.set_success(true);
    workflow.add_unit(purse.Unit().asBase58(api_.Crypto()));
    const auto saved = save_workflow(id, workflow);

    if (false == saved) {
        LogError()(OT_PRETTY_CLASS())("Failed to save workflow").Flush();

        return {};
    }

    return workflowID;
}

auto Workflow::add_cheque_event(
    const eLock& lock,
    const identifier::Nym& nymID,
    const identifier::Nym&,
    proto::PaymentWorkflow& workflow,
    const PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const VersionNumber version,
    const Message& request,
    const Message* reply,
    const identifier::Generic& account) const -> bool
{
    const bool haveReply = (nullptr != reply);
    const bool success = cheque_deposit_success(reply);

    if (success) {
        workflow.set_state(translate(newState));

        if ((false == account.empty()) && (0 == workflow.account_size())) {
            workflow.add_account(account.asBase58(api_.Crypto()));
        }
    }

    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(String::Factory(request)->Get());
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(request.notary_id_->Get());

    switch (newEventType) {
        case proto::PAYMENTEVENTTYPE_CANCEL:
        case proto::PAYMENTEVENTTYPE_COMPLETE: {
        } break;
        case proto::PAYMENTEVENTTYPE_CONVEY:
        case proto::PAYMENTEVENTTYPE_ACCEPT: {
            event.set_nym(request.nym_id2_->Get());
        } break;
        case proto::PAYMENTEVENTTYPE_ERROR:
        case proto::PAYMENTEVENTTYPE_CREATE:
        case proto::PAYMENTEVENTTYPE_ABORT:
        case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE:
        case proto::PAYMENTEVENTTYPE_EXPIRE:
        case proto::PAYMENTEVENTTYPE_REJECT:
        default: {
            OT_FAIL;
        }
    }

    event.set_success(success);

    if (haveReply) {
        event.add_item(String::Factory(*reply)->Get());
        event.set_time(reply->time_);
    } else {
        event.set_time(request.time_);
    }

    if (false == account.empty()) {
        workflow.set_notary(
            api_.Storage().AccountServer(account).asBase58(api_.Crypto()));
    }

    return save_workflow(nymID, account, workflow);
}

// Only used for ClearCheque
auto Workflow::add_cheque_event(
    const eLock& lock,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID,
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
    event.set_transport(receipt.GetRealNotaryID().asBase58(api_.Crypto()));
    event.set_nym(recipientNymID.asBase58(api_.Crypto()));
    event.set_success(true);

    if (0 == workflow.party_size()) {
        workflow.add_party(recipientNymID.asBase58(api_.Crypto()));
    }

    return save_workflow(nymID, accountID, workflow);
}

auto Workflow::add_transfer_event(
    const eLock& lock,
    const identifier::Nym& nymID,
    const identifier::Nym& eventNym,
    proto::PaymentWorkflow& workflow,
    const PaymentWorkflowState newState,
    const proto::PaymentEventType newEventType,
    const VersionNumber version,
    const Message& message,
    const identifier::Generic& account,
    const bool success) const -> bool
{
    if (success) { workflow.set_state(translate(newState)); }

    auto& event = *(workflow.add_event());
    event.set_version(version);
    event.set_type(newEventType);
    event.add_item(String::Factory(message)->Get());
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(message.notary_id_->Get());

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
        case proto::PAYMENTEVENTTYPE_EXPIRE:
        case proto::PAYMENTEVENTTYPE_REJECT:
        default: {
            OT_FAIL;
        }
    }

    event.set_success(success);
    event.set_time(message.time_);

    if (0 == workflow.party_size() && (false == eventNym.empty())) {
        workflow.add_party(eventNym.asBase58(api_.Crypto()));
    }

    return save_workflow(nymID, account, workflow);
}

auto Workflow::add_transfer_event(
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
        case proto::PAYMENTEVENTTYPE_EXPIRE:
        case proto::PAYMENTEVENTTYPE_REJECT:
        default: {
            OT_FAIL;
        }
    }

    event.set_success(success);
    event.set_time(Clock::to_time_t(Clock::now()));

    if (0 == workflow.party_size() && (false == eventNym.empty())) {
        workflow.add_party(eventNym.asBase58(api_.Crypto()));
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
            OT_FAIL;
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

    const auto& nymID = cheque.GetSenderNymID();
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

    static const auto accountID = identifier::Generic{};

    return add_cheque_event(
        lock,
        nymID,
        {},
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

    return message->success_;
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

    const auto& nymID = cheque->GetSenderNymID();
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

    OT_ASSERT(1 == workflow->account_size());

    const bool needNym = (0 == workflow->party_size());
    const auto time = Clock::now();
    const auto output = add_cheque_event(
        lock,
        nymID,
        api_.Factory().IdentifierFromBase58(workflow->account(0)),
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
            api_.Factory().Internal().Identifier(*cheque),
            api_.Factory().IdentifierFromBase58(workflow->id()),
            otx::client::StorageBox::OUTGOINGCHEQUE,
            extract_conveyed_time(*workflow));
    }

    update_rpc(
        nymID,
        cheque->GetRecipientNymID(),
        cheque->SourceAccountID().asBase58(api_.Crypto()),
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
    auto depositorNymID = identifier::Nym{};
    const auto transfer =
        extract_transfer_from_receipt(receipt, depositorNymID);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Invalid transfer").Flush();

        return false;
    }

    if (depositorNymID.empty()) {
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
    const auto workflow = get_workflow(global, type, nymID, *transfer);

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
        nymID,
        notaryID.asBase58(api_.Crypto()),
        (isInternal ? identifier::Nym{} : depositorNymID),
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
            api_.Factory().Internal().Identifier(*transfer),
            api_.Factory().IdentifierFromBase58(workflow->id()),
            otx::client::StorageBox::OUTGOINGTRANSFER,
            time);
        update_rpc(
            nymID,
            depositorNymID,
            accountID.asBase58(api_.Crypto()),
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
    auto depositorNymID = identifier::Nym{};
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
    const auto workflow = get_workflow(global, type, nymID, *transfer);

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
        nymID,
        notaryID.asBase58(api_.Crypto()),
        (isInternal ? identifier::Nym{} : depositorNymID),
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
    const identifier::Nym& senderNymID,
    const identifier::Nym& recipientNymID,
    const Item& transfer) const -> identifier::Generic
{
    Lock global(lock_);
    const auto existing = get_workflow(
        global, {PaymentWorkflowType::IncomingTransfer}, nymID, transfer);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer already exist.")
            .Flush();

        return api_.Factory().IdentifierFromBase58(existing->id());
    }

    const auto& accountID = pending.GetPurportedAccountID();
    const auto [workflowID, workflow] = create_transfer(
        global,
        nymID,
        transfer,
        PaymentWorkflowType::IncomingTransfer,
        PaymentWorkflowState::Conveyed,
        versions_.at(PaymentWorkflowType::IncomingTransfer).workflow_,
        versions_.at(PaymentWorkflowType::IncomingTransfer).source_,
        versions_.at(PaymentWorkflowType::IncomingTransfer).event_,
        senderNymID,
        accountID,
        notaryID.asBase58(api_.Crypto()),
        "");

    if (false == workflowID.empty()) {
        const auto time = extract_conveyed_time(workflow);
        auto note = String::Factory();
        transfer.GetNote(note);
        update_activity(
            nymID,
            transfer.GetNymID(),
            api_.Factory().Internal().Identifier(transfer),
            workflowID,
            otx::client::StorageBox::INCOMINGTRANSFER,
            time);
        update_rpc(
            recipientNymID,
            senderNymID,
            accountID.asBase58(api_.Crypto()),
            proto::ACCOUNTEVENT_INCOMINGTRANSFER,
            workflowID.asBase58(api_.Crypto()),
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
    const identifier::Nym& senderNymID,
    const Item& transfer) const -> identifier::Generic
{
    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {PaymentWorkflowType::InternalTransfer}, nymID, transfer);

    if (false == bool(workflow)) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer does not exist.")
            .Flush();

        return {};
    }

    auto lock = get_workflow_lock(global, workflow->id());

    if (false == can_convey_transfer(*workflow)) { return {}; }

    const auto output = add_transfer_event(
        lock,
        nymID,
        notaryID.asBase58(api_.Crypto()),
        {},
        *workflow,
        PaymentWorkflowState::Conveyed,
        proto::PAYMENTEVENTTYPE_CONVEY,
        versions_.at(PaymentWorkflowType::InternalTransfer).event_,
        pending,
        transfer.GetDestinationAcctID(),
        true);

    if (output) {
        return api_.Factory().IdentifierFromBase58(workflow->id());
    } else {
        return {};
    }
}

auto Workflow::ConveyTransfer(
    const identifier::Nym& nymID,
    const identifier::Notary& notaryID,
    const OTTransaction& pending) const -> identifier::Generic
{
    const auto transfer = extract_transfer_from_pending(pending);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Invalid transaction").Flush();

        return {};
    }

    const auto& senderNymID = transfer->GetNymID();
    contact_.NymToContact(transfer->GetNymID());
    const auto& recipientNymID = pending.GetNymID();

    if (pending.GetNymID() != nymID) {
        LogError()(OT_PRETTY_CLASS())("Invalid recipient").Flush();

        return {};
    }

    const bool isInternal = (senderNymID == recipientNymID);

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
    const identifier::Nym& nymID,
    const opentxs::Cheque& cheque,
    const PaymentWorkflowType workflowType,
    const PaymentWorkflowState workflowState,
    const VersionNumber workflowVersion,
    const VersionNumber sourceVersion,
    const VersionNumber eventVersion,
    const identifier::Nym& party,
    const identifier::Generic& account,
    const Message* message) const
    -> std::pair<identifier::Generic, proto::PaymentWorkflow>
{
    OT_ASSERT(verify_lock(lock));

    auto output = std::pair<identifier::Generic, proto::PaymentWorkflow>{};
    auto& [workflowID, workflow] = output;
    const auto chequeID = api_.Factory().Internal().Identifier(cheque);
    const UnallocatedCString serialized = String::Factory(cheque)->Get();
    workflowID = api_.Factory().IdentifierFromRandom();
    workflow.set_version(workflowVersion);
    workflow.set_id(workflowID.asBase58(api_.Crypto()));
    workflow.set_type(translate(workflowType));
    workflow.set_state(translate(workflowState));
    auto& source = *(workflow.add_source());
    source.set_version(sourceVersion);
    source.set_id(chequeID.asBase58(api_.Crypto()));
    source.set_revision(1);
    source.set_item(serialized);

    // add party if it was passed in and is not already present
    if ((false == party.empty()) && (0 == workflow.party_size())) {
        workflow.add_party(party.asBase58(api_.Crypto()));
    }

    auto& event = *workflow.add_event();
    event.set_version(eventVersion);

    if (nullptr != message) {
        event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
        event.add_item(String::Factory(*message)->Get());
        event.set_time(message->time_);
        event.set_method(proto::TRANSPORTMETHOD_OT);
        event.set_transport(message->notary_id_->Get());
    } else {
        event.set_time(Clock::to_time_t(Clock::now()));

        if (PaymentWorkflowState::Unsent == workflowState) {
            event.set_type(proto::PAYMENTEVENTTYPE_CREATE);
            event.set_method(proto::TRANSPORTMETHOD_NONE);
        } else if (PaymentWorkflowState::Conveyed == workflowState) {
            event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
            event.set_method(proto::TRANSPORTMETHOD_OOB);
        } else {
            OT_FAIL;
        }
    }

    if (false == party.empty()) {
        event.set_nym(party.asBase58(api_.Crypto()));
    }

    event.set_success(true);
    workflow.add_unit(
        cheque.GetInstrumentDefinitionID().asBase58(api_.Crypto()));

    // add account if it was passed in and is not already present
    if ((false == account.empty()) && (0 == workflow.account_size())) {
        workflow.add_account(account.asBase58(api_.Crypto()));
    }

    if ((false == account.empty()) && (workflow.notary().empty())) {
        workflow.set_notary(
            api_.Storage().AccountServer(account).asBase58(api_.Crypto()));
    }

    if (workflow.notary().empty() && (nullptr != message)) {
        workflow.set_notary(message->notary_id_->Get());
    }

    return save_workflow(std::move(output), nymID, account, workflow);
}

auto Workflow::create_transfer(
    const Lock& global,
    const identifier::Nym& nymID,
    const Item& transfer,
    const PaymentWorkflowType workflowType,
    const PaymentWorkflowState workflowState,
    const VersionNumber workflowVersion,
    const VersionNumber sourceVersion,
    const VersionNumber eventVersion,
    const identifier::Nym& party,
    const identifier::Generic& account,
    const UnallocatedCString& notaryID,
    const UnallocatedCString& destinationAccountID) const
    -> std::pair<identifier::Generic, proto::PaymentWorkflow>
{
    OT_ASSERT(verify_lock(global));
    OT_ASSERT(false == nymID.empty());
    OT_ASSERT(false == account.empty());
    OT_ASSERT(false == notaryID.empty());

    auto output = std::pair<identifier::Generic, proto::PaymentWorkflow>{};
    auto& [workflowID, workflow] = output;
    const auto transferID = api_.Factory().Internal().Identifier(transfer);
    LogVerbose()(OT_PRETTY_CLASS())("Transfer ID: ")(transferID).Flush();
    const UnallocatedCString serialized = String::Factory(transfer)->Get();
    const auto existing = get_workflow(global, {workflowType}, nymID, transfer);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer already exists.")
            .Flush();
        workflowID = api_.Factory().IdentifierFromBase58(existing->id());

        return output;
    }

    workflowID = api_.Factory().IdentifierFromRandom();
    workflow.set_version(workflowVersion);
    workflow.set_id(workflowID.asBase58(api_.Crypto()));
    workflow.set_type(translate(workflowType));
    workflow.set_state(translate(workflowState));
    auto& source = *(workflow.add_source());
    source.set_version(sourceVersion);
    source.set_id(transferID.asBase58(api_.Crypto()));
    source.set_revision(1);
    source.set_item(serialized);
    workflow.set_notary(notaryID);

    // add party if it was passed in and is not already present
    if ((false == party.empty()) && (0 == workflow.party_size())) {
        workflow.add_party(party.asBase58(api_.Crypto()));
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
        OT_FAIL;
    }

    event.set_transport(notaryID);

    if (false == party.empty()) {
        event.set_nym(party.asBase58(api_.Crypto()));
    }

    event.set_success(true);
    workflow.add_unit(
        api_.Storage().AccountContract(account).asBase58(api_.Crypto()));

    // add account if it is not already present
    if (0 == workflow.account_size()) {
        workflow.add_account(account.asBase58(api_.Crypto()));

        if (false == destinationAccountID.empty()) {
            workflow.add_account(destinationAccountID);
        }
    }

    return save_workflow(std::move(output), nymID, account, workflow);
}

// Creates outgoing and internal transfer workflows.
auto Workflow::CreateTransfer(const Item& transfer, const Message& request)
    const -> identifier::Generic
{
    if (false == isTransfer(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Invalid item type on object").Flush();

        return {};
    }

    const auto senderNymID =
        api_.Factory().NymIDFromBase58(request.nym_id_->Bytes());
    const auto& accountID = transfer.GetRealAccountID();
    const bool isInternal =
        isInternalTransfer(accountID, transfer.GetDestinationAcctID());
    Lock global(lock_);
    const auto existing = get_workflow(
        global,
        {isInternal ? PaymentWorkflowType::InternalTransfer
                    : PaymentWorkflowType::OutgoingTransfer},
        senderNymID,
        transfer);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())(
            "Workflow for this transfer already exist.")
            .Flush();

        return api_.Factory().IdentifierFromBase58(existing->id());
    }

    const auto [workflowID, workflow] = create_transfer(
        global,
        senderNymID,
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
        {},
        accountID,
        request.notary_id_->Get(),
        (isInternal ? transfer.GetDestinationAcctID().asBase58(api_.Crypto())
                    : ""));

    if (false == workflowID.empty()) {
        const auto time = extract_conveyed_time(workflow);
        auto note = String::Factory();
        transfer.GetNote(note);
        update_rpc(
            senderNymID,
            {},
            accountID.asBase58(api_.Crypto()),
            proto::ACCOUNTEVENT_OUTGOINGTRANSFER,
            workflowID.asBase58(api_.Crypto()),
            transfer.GetAmount(),
            0,
            time,
            note->Get());
    }

    return workflowID;
}

auto Workflow::DepositCheque(
    const identifier::Nym& receiver,
    const identifier::Generic& accountID,
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    Lock global(lock_);
    const auto workflow = get_workflow(
        global, {PaymentWorkflowType::IncomingCheque}, receiver, cheque);

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
        receiver,
        cheque.GetSenderNymID(),
        *workflow,
        PaymentWorkflowState::Completed,
        proto::PAYMENTEVENTTYPE_ACCEPT,
        versions_.at(PaymentWorkflowType::IncomingCheque).event_,
        request,
        reply,
        accountID);

    if (output && cheque_deposit_success(reply)) {
        update_rpc(
            receiver,
            cheque.GetSenderNymID(),
            accountID.asBase58(api_.Crypto()),
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflow->id(),
            cheque.GetAmount(),
            0,
            convert_stime(reply->time_),
            cheque.GetMemo().Get());
    }

    return output;
}

auto Workflow::ExpireCheque(
    const identifier::Nym& nym,
    const opentxs::Cheque& cheque) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    Lock global(lock_);
    const auto workflow = get_workflow(
        global,
        {PaymentWorkflowType::OutgoingCheque,
         PaymentWorkflowType::IncomingCheque},
        nym,
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

    return save_workflow(nym, cheque.GetSenderAcctID(), *workflow);
}

auto Workflow::ExportCheque(const opentxs::Cheque& cheque) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    const auto& nymID = cheque.GetSenderNymID();
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
            if (event.success()) { return convert_stime(event.time()); }
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
    identifier::Nym& depositorNymID) const -> std::unique_ptr<Item>
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

    depositorNymID = acceptPending->GetNymID();
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

    const auto& nymID = cheque.GetSenderNymID();
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

    static const auto accountID = identifier::Generic{};

    return add_cheque_event(
        lock,
        nymID,
        {},
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
    const identifier::Nym& nymID,
    const T& source) const -> std::shared_ptr<proto::PaymentWorkflow>
{
    OT_ASSERT(verify_lock(global));

    const auto itemID =
        api_.Factory().Internal().Identifier(source).asBase58(api_.Crypto());
    LogVerbose()(OT_PRETTY_CLASS())("Item ID: ")(itemID).Flush();

    return get_workflow_by_source(types, nymID, itemID);
}

auto Workflow::get_workflow_by_id(
    const identifier::Nym& nymID,
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
    const identifier::Nym& nymID,
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
    const identifier::Nym& nymID,
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
    const opentxs::Cheque& cheque) const -> identifier::Generic
{
    if (false == isCheque(cheque)) { return {}; }

    if (false == validate_recipient(nymID, cheque)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(
            nymID)(" can not deposit this cheque.")
            .Flush();

        return {};
    }

    Lock global(lock_);
    const auto existing = get_workflow(
        global, {PaymentWorkflowType::IncomingCheque}, nymID, cheque);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())("Workflow for this cheque already exist.")
            .Flush();

        return api_.Factory().IdentifierFromBase58(existing->id());
    }

    const auto& party = cheque.GetSenderNymID();
    static const auto accountID = identifier::Generic{};
    const auto [workflowID, workflow] = create_cheque(
        global,
        nymID,
        cheque,
        PaymentWorkflowType::IncomingCheque,
        PaymentWorkflowState::Conveyed,
        versions_.at(PaymentWorkflowType::IncomingCheque).workflow_,
        versions_.at(PaymentWorkflowType::IncomingCheque).source_,
        versions_.at(PaymentWorkflowType::IncomingCheque).event_,
        party,
        accountID);

    if (false == workflowID.empty()) {
        const auto time = extract_conveyed_time(workflow);
        update_activity(
            nymID,
            cheque.GetSenderNymID(),
            api_.Factory().Internal().Identifier(cheque),
            workflowID,
            otx::client::StorageBox::INCOMINGCHEQUE,
            time);
        update_rpc(
            nymID,
            cheque.GetSenderNymID(),
            {},
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflowID.asBase58(api_.Crypto()),
            0,
            cheque.GetAmount(),
            time,
            cheque.GetMemo().Get());
    }

    return workflowID;
}

auto Workflow::InstantiateCheque(
    const identifier::Nym& nym,
    const identifier::Generic& id) const -> Cheque
{
    try {
        const auto workflow = [&] {
            auto out = proto::PaymentWorkflow{};

            if (false == LoadWorkflow(nym, id, out)) {
                throw std::runtime_error{
                    UnallocatedCString{"Workflow "} +
                    id.asBase58(api_.Crypto()) + " not found"};
            }

            return out;
        }();

        if (false == ContainsCheque(workflow)) {

            throw std::runtime_error{
                UnallocatedCString{"Workflow "} + id.asBase58(api_.Crypto()) +
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
    const identifier::Generic& id) const -> Purse
{
    try {
        const auto workflow = [&] {
            auto out = proto::PaymentWorkflow{};

            if (false == LoadWorkflow(nym, id, out)) {
                throw std::runtime_error{
                    UnallocatedCString{"Workflow "} +
                    id.asBase58(api_.Crypto()) + " not found"};
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
    const identifier::Generic& sourceAccount,
    const identifier::Generic& destinationAccount) const -> bool
{
    const auto ownerNymID = api_.Storage().AccountOwner(sourceAccount);

    OT_ASSERT(false == ownerNymID.empty());

    const auto recipientNymID = api_.Storage().AccountOwner(destinationAccount);

    if (recipientNymID.empty()) { return false; }

    return ownerNymID == recipientNymID;
}

auto Workflow::isTransfer(const Item& item) -> bool
{
    return itemType::transfer == item.GetType();
}

auto Workflow::List(
    const identifier::Nym& nymID,
    const PaymentWorkflowType type,
    const PaymentWorkflowState state) const
    -> UnallocatedSet<identifier::Generic>
{
    const auto input =
        api_.Storage().PaymentWorkflowsByState(nymID, type, state);
    UnallocatedSet<identifier::Generic> output{};
    std::transform(
        input.begin(),
        input.end(),
        std::inserter(output, output.end()),
        [this](const auto& id) {
            return api_.Factory().IdentifierFromBase58(id);
        });

    return output;
}

auto Workflow::LoadCheque(
    const identifier::Nym& nymID,
    const identifier::Generic& chequeID) const -> Workflow::Cheque
{
    auto workflow = get_workflow_by_source(
        {PaymentWorkflowType::OutgoingCheque,
         PaymentWorkflowType::IncomingCheque},
        nymID,
        chequeID.asBase58(api_.Crypto()));

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
    const identifier::Generic& workflowID) const -> Workflow::Cheque
{
    auto workflow = get_workflow_by_id(
        {PaymentWorkflowType::OutgoingCheque,
         PaymentWorkflowType::IncomingCheque},
        nymID,
        workflowID.asBase58(api_.Crypto()));

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
    const identifier::Generic& transferID) const -> Workflow::Transfer
{
    auto workflow = get_workflow_by_source(
        {PaymentWorkflowType::OutgoingTransfer,
         PaymentWorkflowType::IncomingTransfer,
         PaymentWorkflowType::InternalTransfer},
        nymID,
        transferID.asBase58(api_.Crypto()));

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
    const identifier::Generic& workflowID) const -> Workflow::Transfer
{
    auto workflow = get_workflow_by_id(
        {PaymentWorkflowType::OutgoingTransfer,
         PaymentWorkflowType::IncomingTransfer,
         PaymentWorkflowType::InternalTransfer},
        nymID,
        workflowID.asBase58(api_.Crypto()));

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
    const identifier::Generic& workflowID,
    proto::PaymentWorkflow& out) const -> bool
{
    auto pWorkflow =
        get_workflow_by_id(nymID, workflowID.asBase58(api_.Crypto()));

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
    const Message& message) const -> identifier::Generic
{
    Lock global(lock_);
    const auto serialized = String::Factory(message);
    const auto* party = message.nym_id_->Get();
    auto workflowID = api_.Factory().IdentifierFromRandom();
    proto::PaymentWorkflow workflow{};
    workflow.set_version(
        versions_.at(PaymentWorkflowType::IncomingCash).workflow_);
    workflow.set_id(workflowID.asBase58(api_.Crypto()));
    workflow.set_type(translate(PaymentWorkflowType::IncomingCash));
    workflow.set_state(translate(PaymentWorkflowState::Conveyed));
    auto& source = *(workflow.add_source());
    source.set_version(versions_.at(PaymentWorkflowType::IncomingCash).source_);
    source.set_id(workflowID.asBase58(api_.Crypto()));
    source.set_revision(1);
    source.set_item([&] {
        auto proto = proto::Purse{};
        purse.Internal().Serialize(proto);

        return proto::ToString(proto);
    }());
    workflow.set_notary(purse.Notary().asBase58(api_.Crypto()));
    auto& event = *workflow.add_event();
    event.set_version(versions_.at(PaymentWorkflowType::IncomingCash).event_);
    event.set_time(message.time_);
    event.set_type(proto::PAYMENTEVENTTYPE_CONVEY);
    event.set_method(proto::TRANSPORTMETHOD_OT);
    event.set_transport(message.notary_id_->Get());
    event.add_item(serialized->Get());
    event.set_nym(party);
    event.set_success(true);
    workflow.add_unit(purse.Unit().asBase58(api_.Crypto()));
    workflow.add_party(party);
    const auto saved = save_workflow(receiver, workflow);

    if (false == saved) {
        LogError()(OT_PRETTY_CLASS())("Failed to save workflow").Flush();

        return {};
    }

    return workflowID;
}

auto Workflow::ReceiveCheque(
    const identifier::Nym& nymID,
    const opentxs::Cheque& cheque,
    const Message& message) const -> identifier::Generic
{
    if (false == isCheque(cheque)) { return {}; }

    if (false == validate_recipient(nymID, cheque)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(
            nymID)(" can not deposit this cheque.")
            .Flush();

        return {};
    }

    Lock global(lock_);
    const auto existing = get_workflow(
        global, {PaymentWorkflowType::IncomingCheque}, nymID, cheque);

    if (existing) {
        LogError()(OT_PRETTY_CLASS())("Workflow for this cheque already exist.")
            .Flush();

        return api_.Factory().IdentifierFromBase58(existing->id());
    }

    const auto& party = cheque.GetSenderNymID();
    static const auto accountID = identifier::Generic{};
    const auto [workflowID, workflow] = create_cheque(
        global,
        nymID,
        cheque,
        PaymentWorkflowType::IncomingCheque,
        PaymentWorkflowState::Conveyed,
        versions_.at(PaymentWorkflowType::IncomingCheque).workflow_,
        versions_.at(PaymentWorkflowType::IncomingCheque).source_,
        versions_.at(PaymentWorkflowType::IncomingCheque).event_,
        party,
        accountID,
        &message);

    if (false == workflowID.empty()) {
        const auto time = extract_conveyed_time(workflow);
        update_activity(
            nymID,
            cheque.GetSenderNymID(),
            api_.Factory().Internal().Identifier(cheque),
            workflowID,
            otx::client::StorageBox::INCOMINGCHEQUE,
            time);
        update_rpc(
            nymID,
            cheque.GetSenderNymID(),
            {},
            proto::ACCOUNTEVENT_INCOMINGCHEQUE,
            workflowID.asBase58(api_.Crypto()),
            0,
            cheque.GetAmount(),
            time,
            cheque.GetMemo().Get());
    }

    return workflowID;
}

auto Workflow::save_workflow(
    const identifier::Nym& nymID,
    const proto::PaymentWorkflow& workflow) const -> bool
{
    static const auto id = identifier::Generic{};

    return save_workflow(nymID, id, workflow);
}

auto Workflow::save_workflow(
    const identifier::Nym& nymID,
    const identifier::Generic& accountID,
    const proto::PaymentWorkflow& workflow) const -> bool
{
    const bool valid = proto::Validate(workflow, VERBOSE);

    OT_ASSERT(valid);

    const auto saved = api_.Storage().Store(nymID, workflow);

    OT_ASSERT(saved);

    if (false == accountID.empty()) {
        account_publisher_->Send([&] {
            auto work = opentxs::network::zeromq::tagged_message(
                WorkType::WorkflowAccountUpdate, true);
            work.AddFrame(accountID);

            return work;
        }());
    }

    return valid && saved;
}

auto Workflow::save_workflow(
    identifier::Generic&& output,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID,
    const proto::PaymentWorkflow& workflow) const -> identifier::Generic
{
    if (save_workflow(nymID, accountID, workflow)) { return std::move(output); }

    return {};
}

auto Workflow::save_workflow(
    std::pair<identifier::Generic, proto::PaymentWorkflow>&& output,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID,
    const proto::PaymentWorkflow& workflow) const
    -> std::pair<identifier::Generic, proto::PaymentWorkflow>
{
    if (save_workflow(nymID, accountID, workflow)) { return std::move(output); }

    return {};
}

auto Workflow::SendCash(
    const identifier::Nym& sender,
    const identifier::Nym& recipient,
    const identifier::Generic& workflowID,
    const Message& request,
    const Message* reply) const -> bool
{
    Lock global(lock_);
    const auto pWorkflow =
        get_workflow_by_id(sender, workflowID.asBase58(api_.Crypto()));

    if (false == bool(pWorkflow)) {
        LogError()(OT_PRETTY_CLASS())("Workflow ")(
            workflowID)(" does not exist.")
            .Flush();

        return false;
    }

    auto& workflow = *pWorkflow;
    auto lock = get_workflow_lock(global, workflowID.asBase58(api_.Crypto()));

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
    event.set_transport(request.notary_id_->Get());
    event.set_nym(request.nym_id2_->Get());

    if (haveReply) {
        event.add_item(String::Factory(*reply)->Get());
        event.set_time(reply->time_);
        event.set_success(reply->success_);
    } else {
        event.set_time(request.time_);
        event.set_success(false);
    }

    if (0 == workflow.party_size()) {
        workflow.add_party(recipient.asBase58(api_.Crypto()));
    }

    return save_workflow(sender, workflow);
}

auto Workflow::SendCheque(
    const opentxs::Cheque& cheque,
    const Message& request,
    const Message* reply) const -> bool
{
    if (false == isCheque(cheque)) { return false; }

    const auto& nymID = cheque.GetSenderNymID();
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

    static const auto accountID = identifier::Generic{};

    return add_cheque_event(
        lock,
        nymID,
        api_.Factory().NymIDFromBase58(request.nym_id2_->Bytes()),
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
    const identifier::Generic& workflowID,
    const int index) const -> const UnallocatedCString
{
    auto workflow =
        get_workflow_by_id(nymID, workflowID.asBase58(api_.Crypto()));

    if (false == bool{workflow}) { return {}; }

    return workflow->party(index);
}

auto Workflow::WorkflowPartySize(
    const identifier::Nym& nymID,
    const identifier::Generic& workflowID,
    int& partysize) const -> bool
{
    auto workflow =
        get_workflow_by_id(nymID, workflowID.asBase58(api_.Crypto()));

    if (false == bool{workflow}) { return false; }

    partysize = workflow->party_size();

    return true;
}

auto Workflow::WorkflowState(
    const identifier::Nym& nymID,
    const identifier::Generic& workflowID) const -> PaymentWorkflowState
{
    auto workflow =
        get_workflow_by_id(nymID, workflowID.asBase58(api_.Crypto()));

    if (false == bool{workflow}) { return PaymentWorkflowState::Error; }

    return translate(workflow->state());
}

auto Workflow::WorkflowType(
    const identifier::Nym& nymID,
    const identifier::Generic& workflowID) const -> PaymentWorkflowType
{
    auto workflow =
        get_workflow_by_id(nymID, workflowID.asBase58(api_.Crypto()));

    if (false == bool{workflow}) { return PaymentWorkflowType::Error; }

    return translate(workflow->type());
}

auto Workflow::update_activity(
    const identifier::Nym& localNymID,
    const identifier::Nym& remoteNymID,
    const identifier::Generic& sourceID,
    const identifier::Generic& workflowID,
    const otx::client::StorageBox type,
    Time time) const -> bool
{
    const auto contactID = contact_.ContactID(remoteNymID);

    if (contactID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Contact for nym ")(
            remoteNymID)(" does not exist")
            .Flush();

        return false;
    }

    const bool added = activity_.AddPaymentEvent(
        localNymID, contactID, type, sourceID, workflowID, time);

    if (added) {
        LogDetail()(OT_PRETTY_CLASS())(
            "Success adding payment event to thread ")(
            contactID.asBase58(api_.Crypto()))
            .Flush();

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to add payment event to thread ")(
            contactID.asBase58(api_.Crypto()))
            .Flush();

        return false;
    }
}

void Workflow::update_rpc(
    const identifier::Nym& localNymID,
    const identifier::Nym& remoteNymID,
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
    push.set_id(localNymID.asBase58(api_.Crypto()));
    auto& event = *push.mutable_accountevent();
    event.set_version(RPC_ACCOUNT_EVENT_VERSION);
    event.set_id(accountID);
    event.set_type(type);

    if (false == remoteNymID.empty()) {
        event.set_contact(
            contact_.NymToContact(remoteNymID).asBase58(api_.Crypto()));
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
    const identifier::Generic& accountID) const
    -> UnallocatedVector<identifier::Generic>
{
    UnallocatedVector<identifier::Generic> output{};
    const auto workflows = api_.Storage().PaymentWorkflowsByAccount(
        nymID, accountID.asBase58(api_.Crypto()));
    std::transform(
        workflows.begin(),
        workflows.end(),
        std::inserter(output, output.end()),
        [this](const UnallocatedCString& id) {
            return api_.Factory().IdentifierFromBase58(id);
        });

    return output;
}

auto Workflow::WriteCheque(const opentxs::Cheque& cheque) const
    -> identifier::Generic
{
    if (false == isCheque(cheque)) {
        LogError()(OT_PRETTY_STATIC(Workflow))(
            "Invalid item type on cheque object")
            .Flush();

        return {};
    }

    const auto& nymID = cheque.GetSenderNymID();
    Lock global(lock_);
    const auto existing = get_workflow(
        global, {PaymentWorkflowType::OutgoingCheque}, nymID, cheque);

    if (existing) {
        LogError()(OT_PRETTY_STATIC(Workflow))(
            "Workflow for this cheque already exist.")
            .Flush();

        return api_.Factory().IdentifierFromBase58(existing->id());
    }

    if (cheque.HasRecipient()) {
        const auto& recipient = cheque.GetRecipientNymID();
        const auto contactID = contact_.ContactID(recipient);

        if (contactID.empty()) {
            LogError()(OT_PRETTY_CLASS())(
                "No contact exists for recipient nym ")(recipient)
                .Flush();

            return {};
        }
    }

    const auto party =
        cheque.HasRecipient() ? cheque.GetRecipientNymID() : identifier::Nym{};
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
    const bool haveWorkflow = (false == workflowID.empty());
    const auto time{convert_stime(workflow.event(0).time())};

    if (haveWorkflow && cheque.HasRecipient()) {
        update_activity(
            cheque.GetSenderNymID(),
            cheque.GetRecipientNymID(),
            api_.Factory().Internal().Identifier(cheque),
            workflowID,
            otx::client::StorageBox::OUTGOINGCHEQUE,
            time);
    }

    if (false == workflowID.empty()) {
        update_rpc(
            nymID,
            cheque.GetRecipientNymID(),
            cheque.SourceAccountID().asBase58(api_.Crypto()),
            proto::ACCOUNTEVENT_OUTGOINGCHEQUE,
            workflowID.asBase58(api_.Crypto()),
            0,
            -1 * cheque.GetAmount(),
            time,
            cheque.GetMemo().Get());
    }

    return workflowID;
}
}  // namespace opentxs::api::session::imp
