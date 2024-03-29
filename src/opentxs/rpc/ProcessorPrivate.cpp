// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ContactItemAttribute

#include "opentxs/rpc/ProcessorPrivate.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/APIArgument.pb.h>
#include <opentxs/protobuf/AcceptPendingPayment.pb.h>
#include <opentxs/protobuf/AccountEvent.pb.h>
#include <opentxs/protobuf/AddClaim.pb.h>
#include <opentxs/protobuf/AddContact.pb.h>
#include <opentxs/protobuf/ContactItem.pb.h>
#include <opentxs/protobuf/ContactItemAttribute.pb.h>
#include <opentxs/protobuf/CreateInstrumentDefinition.pb.h>
#include <opentxs/protobuf/CreateNym.pb.h>
#include <opentxs/protobuf/GetWorkflow.pb.h>
#include <opentxs/protobuf/HDSeed.pb.h>
#include <opentxs/protobuf/ModifyAccount.pb.h>
#include <opentxs/protobuf/MoveFunds.pb.h>
#include <opentxs/protobuf/Nym.pb.h>
#include <opentxs/protobuf/PaymentEvent.pb.h>
#include <opentxs/protobuf/PaymentWorkflow.pb.h>
#include <opentxs/protobuf/RPCCommand.pb.h>
#include <opentxs/protobuf/RPCEnums.pb.h>
#include <opentxs/protobuf/RPCPush.pb.h>
#include <opentxs/protobuf/RPCResponse.pb.h>
#include <opentxs/protobuf/RPCStatus.pb.h>
#include <opentxs/protobuf/RPCTask.pb.h>
#include <opentxs/protobuf/ServerContract.pb.h>
#include <opentxs/protobuf/SessionData.pb.h>
#include <opentxs/protobuf/TaskComplete.pb.h>
#include <opentxs/protobuf/UnitDefinition.pb.h>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <future>
#include <iterator>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/api/session/Types.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Pull.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/otx/client/OTPayment.hpp"  // IWYU pragma: keep
#include "internal/otx/client/obsolete/OT_API.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/blockchain/Type.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Language.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/SeedStyle.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/PaymentWorkflowType.hpp"   // IWYU pragma: keep
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/protobuf/syntax/RPCCommand.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/rpc/CommandType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/ProcessorPrivate.tpp"
#include "opentxs/rpc/ResponseCode.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/request/Message.hpp"
#include "opentxs/rpc/request/internal.factory.hpp"
#include "opentxs/rpc/response/Invalid.hpp"
#include "opentxs/rpc/response/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
constexpr auto ACCOUNTEVENT_VERSION = 2;
constexpr auto RPCTASK_VERSION = 1;
constexpr auto SEED_VERSION = 1;
constexpr auto SESSION_DATA_VERSION = 1;
constexpr auto RPCPUSH_VERSION = 3;
constexpr auto TASKCOMPLETE_VERSION = 2;
}  // namespace opentxs

#define CHECK_INPUT(field, error)                                              \
    if (0 == command.field().size()) {                                         \
        add_output_status(output, error);                                      \
                                                                               \
        return output;                                                         \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_OWNER()                                                          \
    if (false == session.Wallet().IsLocalNym(command.owner())) {               \
        add_output_status(output, protobuf::RPCRESPONSE_NYM_NOT_FOUND);        \
                                                                               \
        return output;                                                         \
    }                                                                          \
                                                                               \
    const auto ownerID = ot_.Factory().NymIDFromBase58(command.owner())

#define INIT() auto output = init(command)

#define INIT_SESSION()                                                         \
    INIT();                                                                    \
                                                                               \
    if (false == is_session_valid(command.session())) {                        \
        add_output_status(output, protobuf::RPCRESPONSE_BAD_SESSION);          \
                                                                               \
        return output;                                                         \
    }                                                                          \
                                                                               \
    [[maybe_unused]] auto& session = get_session(command.session());           \
    [[maybe_unused]] auto reason = session.Factory().PasswordPrompt("RPC")

#define INIT_CLIENT_ONLY()                                                     \
    INIT_SESSION();                                                            \
                                                                               \
    if (false == is_client_session(command.session())) {                       \
        add_output_status(output, protobuf::RPCRESPONSE_INVALID);              \
                                                                               \
        return output;                                                         \
    }                                                                          \
                                                                               \
    const auto pClient = get_client(command.session());                        \
                                                                               \
    assert_false(nullptr == pClient);                                          \
                                                                               \
    [[maybe_unused]] const auto& client = *pClient

#define INIT_SERVER_ONLY()                                                     \
    INIT_SESSION();                                                            \
                                                                               \
    if (false == is_server_session(command.session())) {                       \
        add_output_status(output, protobuf::RPCRESPONSE_INVALID);              \
                                                                               \
        return output;                                                         \
    }                                                                          \
                                                                               \
    const auto pServer = get_server(command.session());                        \
                                                                               \
    assert_false(nullptr == pServer);                                          \
                                                                               \
    [[maybe_unused]] const auto& server = *pServer

#define INIT_OTX(a, ...)                                                       \
    auto result =                                                              \
        api::session::OTX::Result{otx::LastReplyStatus::NotSent, nullptr};     \
    [[maybe_unused]] const auto& [status, pReply] = result;                    \
    [[maybe_unused]] auto [taskID, future] = client.OTX().a(__VA_ARGS__);      \
    [[maybe_unused]] const auto ready = (0 != taskID)

namespace opentxs::rpc
{
static const UnallocatedMap<VersionNumber, VersionNumber> StatusVersionMap{
    {1, 1},
    {2, 2},
    {3, 2},
};

ProcessorPrivate::ProcessorPrivate(const api::Context& native)
    : Lockable()
    , ot_(native)
    , task_lock_()
    , queued_tasks_()
    , task_callback_(zmq::ListenCallback::Factory([this](auto&& PH1) {
        task_handler(std::forward<decltype(PH1)>(PH1));
    }))
    , push_callback_(zmq::ListenCallback::Factory([&](const zmq::Message& in) {
        rpc_publisher_->Send(network::zeromq::Message{in});
    }))
    , push_receiver_(ot_.ZMQ().Internal().PullSocket(
          push_callback_,
          zmq::socket::Direction::Bind,
          "RPC push receiver"))
    , rpc_publisher_(ot_.ZMQ().Internal().PublishSocket())
    , task_subscriber_(
          ot_.ZMQ().Internal().SubscribeSocket(task_callback_, "RPC task"))
{
    auto bound = push_receiver_->Start(
        network::zeromq::MakeDeterministicInproc("rpc/push/internal", -1, 1));

    assert_true(bound);

    bound = rpc_publisher_->Start(
        network::zeromq::MakeDeterministicInproc("rpc/push", -1, 1));

    assert_true(bound);
}

auto ProcessorPrivate::accept_pending_payments(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_INPUT(acceptpendingpayment, protobuf::RPCRESPONSE_INVALID);

    for (auto acceptpendingpayment : command.acceptpendingpayment()) {
        const auto destinationaccountID = client.Factory().AccountIDFromBase58(
            acceptpendingpayment.destinationaccount());
        const auto workflowID = client.Factory().IdentifierFromBase58(
            acceptpendingpayment.workflow());
        const auto nymID =
            client.Storage().Internal().AccountOwner(destinationaccountID);

        try {
            const auto paymentWorkflow = [&] {
                auto out = protobuf::PaymentWorkflow{};

                if (!client.Workflow().LoadWorkflow(nymID, workflowID, out)) {
                    add_output_task(output, "");
                    add_output_status(
                        output, protobuf::RPCRESPONSE_WORKFLOW_NOT_FOUND);

                    throw std::runtime_error{
                        UnallocatedCString{"Invalid workflow"} +
                        workflowID.asBase58(ot_.Crypto())};
                }

                return out;
            }();

            auto payment = std::shared_ptr<const OTPayment>{};

            switch (opentxs::translate(paymentWorkflow.type())) {
                case otx::client::PaymentWorkflowType::IncomingCheque:
                case otx::client::PaymentWorkflowType::IncomingInvoice: {
                    auto chequeState =
                        opentxs::api::session::Workflow::InstantiateCheque(
                            client, paymentWorkflow);
                    const auto& [state, cheque] = chequeState;

                    if (false == bool(cheque)) {
                        LogError()()("Unable to load cheque from workflow")
                            .Flush();
                        add_output_task(output, "");
                        add_output_status(
                            output, protobuf::RPCRESPONSE_CHEQUE_NOT_FOUND);

                        continue;
                    }

                    payment.reset(client.Factory()
                                      .Internal()
                                      .Session()
                                      .Payment(*cheque, reason)
                                      .release());
                } break;
                case otx::client::PaymentWorkflowType::OutgoingCheque:
                case otx::client::PaymentWorkflowType::OutgoingInvoice:
                case otx::client::PaymentWorkflowType::Error:
                default: {
                    LogError()()("Unsupported workflow type").Flush();
                    add_output_task(output, "");
                    add_output_status(output, protobuf::RPCRESPONSE_ERROR);

                    continue;
                }
            }

            if (false == bool(payment)) {
                LogError()()("Failed to instantiate payment").Flush();
                add_output_task(output, "");
                add_output_status(
                    output, protobuf::RPCRESPONSE_PAYMENT_NOT_FOUND);

                continue;
            }

            // NOLINTBEGIN(misc-const-correctness)
            INIT_OTX(DepositPayment, nymID, destinationaccountID, payment);
            // NOLINTEND(misc-const-correctness)

            if (false == ready) {
                add_output_task(output, "");
                add_output_status(
                    output, protobuf::RPCRESPONSE_START_TASK_FAILED);
            } else {
                queue_task(
                    nymID,
                    std::to_string(taskID),
                    [&](const auto& in, auto& out) -> void {
                        evaluate_deposit_payment(client, in, out);
                    },
                    std::move(future),
                    output);
            }
        } catch (const std::exception& e) {
            LogError()()(e.what()).Flush();

            continue;
        }
    }

    return output;
}

auto ProcessorPrivate::add_claim(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_SESSION();
    CHECK_OWNER();
    CHECK_INPUT(claim, protobuf::RPCRESPONSE_INVALID);

    const auto nymID = ot_.Factory().NymIDFromBase58(command.owner());
    auto nymdata = session.Wallet().mutable_Nym(nymID, reason);

    for (const auto& addclaim : command.claim()) {
        const auto& contactitem = addclaim.item();
        auto claim = session.Factory().Internal().Session().Claim(
            nymdata.Nym().ID(), translate(addclaim.sectiontype()), contactitem);

        for (const auto& attr : contactitem.attribute()) {
            claim.Add(
                translate(static_cast<protobuf::ContactItemAttribute>(attr)));
        }

        if (nymdata.AddClaim(claim, reason)) {
            add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
        } else {
            add_output_status(output, protobuf::RPCRESPONSE_ADD_CLAIM_FAILED);
        }
    }

    return output;
}

auto ProcessorPrivate::add_contact(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_INPUT(addcontact, protobuf::RPCRESPONSE_INVALID);

    for (const auto& addContact : command.addcontact()) {
        const auto contact = client.Contacts().NewContact(
            addContact.label(),
            ot_.Factory().NymIDFromBase58(addContact.nymid()),
            client.Factory().PaymentCodeFromBase58(addContact.paymentcode()));

        if (false == bool(contact)) {
            add_output_status(output, protobuf::RPCRESPONSE_ADD_CONTACT_FAILED);
        } else {
            output.add_identifier(contact->ID().asBase58(ot_.Crypto()));
            add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
        }
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    }

    return output;
}

void ProcessorPrivate::add_output_identifier(
    const UnallocatedCString& id,
    protobuf::TaskComplete& output)
{
    output.set_identifier(id);
}

void ProcessorPrivate::add_output_identifier(
    const UnallocatedCString& id,
    protobuf::RPCResponse& output)
{
    output.add_identifier(id);
}

void ProcessorPrivate::add_output_status(
    protobuf::RPCResponse& output,
    protobuf::RPCResponseCode code)
{
    auto& status = *output.add_status();
    status.set_version(StatusVersionMap.at(output.version()));
    status.set_index(output.status_size() - 1);
    status.set_code(code);
}

void ProcessorPrivate::add_output_status(
    protobuf::TaskComplete& output,
    protobuf::RPCResponseCode code)
{
    output.set_code(code);
}

void ProcessorPrivate::add_output_task(
    protobuf::RPCResponse& output,
    const UnallocatedCString& taskid)
{
    auto& task = *output.add_task();
    task.set_version(RPCTASK_VERSION);
    task.set_index(output.task_size() - 1);
    task.set_id(taskid);
}

auto ProcessorPrivate::client_session(const request::Message& command) const
    noexcept(false) -> const api::session::Client&
{
    const auto session = command.Session();

    if (false == is_session_valid(session)) {

        throw std::runtime_error{"invalid session"};
    }

    if (is_client_session(session)) {

        return dynamic_cast<const api::session::Client&>(
            ot_.ClientSession(static_cast<int>(get_index(session))));
    } else {

        throw std::runtime_error{"expected client session"};
    }
}

auto ProcessorPrivate::create_account(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_OWNER();

    const auto notaryID = ot_.Factory().NotaryIDFromBase58(command.notary());
    const auto unitID = ot_.Factory().UnitIDFromBase58(command.unit());
    UnallocatedCString label{};

    if (0 < command.identifier_size()) { label = command.identifier(0); }

    // NOLINTBEGIN(misc-const-correctness)
    INIT_OTX(RegisterAccount, ownerID, notaryID, unitID, label);
    // NOLINTEND(misc-const-correctness)

    if (false == ready) {
        add_output_status(output, protobuf::RPCRESPONSE_ERROR);

        return output;
    }

    if (immediate_create_account(client, ownerID, notaryID, unitID)) {
        evaluate_register_account(future.get(), output);
    } else {
        queue_task(
            ownerID,
            std::to_string(taskID),
            [&](const auto& in, auto& out) -> void {
                evaluate_register_account(in, out);
            },
            std::move(future),
            output);
    }

    return output;
}

auto ProcessorPrivate::create_compatible_account(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_OWNER();
    CHECK_INPUT(identifier, protobuf::RPCRESPONSE_INVALID);

    const auto workflowID =
        ot_.Factory().IdentifierFromBase58(command.identifier(0));
    auto notaryID = identifier::Notary{};
    auto unitID = identifier::UnitDefinition{};

    try {
        const auto workflow = [&] {
            auto out = protobuf::PaymentWorkflow{};

            if (!client.Workflow().LoadWorkflow(ownerID, workflowID, out)) {
                throw std::runtime_error{""};
            }

            return out;
        }();

        switch (opentxs::translate(workflow.type())) {
            case otx::client::PaymentWorkflowType::IncomingCheque: {
                auto chequeState =
                    opentxs::api::session::Workflow::InstantiateCheque(
                        client, workflow);
                const auto& [state, cheque] = chequeState;

                if (false == bool(cheque)) {
                    add_output_status(
                        output, protobuf::RPCRESPONSE_CHEQUE_NOT_FOUND);

                    return output;
                }

                notaryID = cheque->GetNotaryID();
                unitID = cheque->GetInstrumentDefinitionID();
            } break;
            default: {
                add_output_status(output, protobuf::RPCRESPONSE_INVALID);

                return output;
            }
        }
    } catch (...) {
        add_output_status(output, protobuf::RPCRESPONSE_WORKFLOW_NOT_FOUND);

        return output;
    }

    // NOLINTBEGIN(misc-const-correctness)
    INIT_OTX(RegisterAccount, ownerID, notaryID, unitID, "");
    // NOLINTEND(misc-const-correctness)

    if (false == ready) {
        add_output_status(output, protobuf::RPCRESPONSE_ERROR);

        return output;
    }

    if (immediate_create_account(client, ownerID, notaryID, unitID)) {
        evaluate_register_account(future.get(), output);
    } else {
        queue_task(
            ownerID,
            std::to_string(taskID),
            [&](const auto& in, auto& out) -> void {
                evaluate_register_account(in, out);
            },
            std::move(future),
            output);
    }

    return output;
}

auto ProcessorPrivate::create_issuer_account(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_OWNER();

    UnallocatedCString label{};
    auto notaryID = ot_.Factory().NotaryIDFromBase58(command.notary());
    auto unitID = ot_.Factory().UnitIDFromBase58(command.unit());

    if (0 < command.identifier_size()) { label = command.identifier(0); }

    try {
        const auto unitdefinition =
            client.Wallet().Internal().UnitDefinition(unitID);

        if (ownerID != unitdefinition->Signer()->ID()) {
            add_output_status(
                output, protobuf::RPCRESPONSE_UNITDEFINITION_NOT_FOUND);

            return output;
        }
    } catch (...) {
        add_output_status(
            output, protobuf::RPCRESPONSE_UNITDEFINITION_NOT_FOUND);

        return output;
    }

    const auto account = client.Wallet().Internal().IssuerAccount(unitID);

    if (account) {
        add_output_status(output, protobuf::RPCRESPONSE_UNNECESSARY);

        return output;
    }

    // NOLINTBEGIN(misc-const-correctness)
    INIT_OTX(
        IssueUnitDefinition, ownerID, notaryID, unitID, UnitType::Error, label);
    // NOLINTEND(misc-const-correctness)

    if (false == ready) {
        add_output_status(output, protobuf::RPCRESPONSE_ERROR);

        return output;
    }

    if (immediate_register_issuer_account(client, ownerID, notaryID)) {
        evaluate_register_account(future.get(), output);
    } else {
        queue_task(
            ownerID,
            std::to_string(taskID),
            [&](const auto& in, auto& out) -> void {
                evaluate_register_account(in, out);
            },
            std::move(future),
            output);
    }

    return output;
}

auto ProcessorPrivate::create_nym(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();

    const auto& createnym = command.createnym();
    const auto pNym = client.Wallet().Nym(
        {client.Factory(),
         client.Factory().SeedIDFromBase58(createnym.seedid()),
         createnym.index()},
        ClaimToNym(translate(createnym.type())),
        reason,
        createnym.name());

    if (false == bool(pNym)) {
        add_output_status(output, protobuf::RPCRESPONSE_CREATE_NYM_FAILED);

        return output;
    }

    const auto& nym = *pNym;

    if (0 < createnym.claims_size()) {
        auto nymdata = client.Wallet().mutable_Nym(nym.ID(), reason);

        for (const auto& addclaim : createnym.claims()) {
            const auto& contactitem = addclaim.item();
            auto claim = client.Factory().Internal().Session().Claim(
                nymdata.Nym().ID(),
                translate(addclaim.sectiontype()),
                contactitem);

            for (const auto& attr : contactitem.attribute()) {
                claim.Add(translate(
                    static_cast<protobuf::ContactItemAttribute>(attr)));
            }

            nymdata.AddClaim(claim, reason);
        }
    }

    output.add_identifier(nym.ID().asBase58(ot_.Crypto()));
    add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);

    return output;
}

auto ProcessorPrivate::create_unit_definition(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_SESSION();
    CHECK_OWNER();

    const auto& createunit = command.createunit();

    try {
        const auto unitdefinition =
            session.Wallet().Internal().CurrencyContract(
                command.owner(),
                createunit.name(),
                createunit.terms(),
                ClaimToUnit(translate(createunit.unitofaccount())),
                factory::Amount(createunit.redemptionincrement()),
                reason);

        output.add_identifier(unitdefinition->ID().asBase58(ot_.Crypto()));
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    } catch (...) {
        add_output_status(
            output, protobuf::RPCRESPONSE_CREATE_UNITDEFINITION_FAILED);
    }

    return output;
}

auto ProcessorPrivate::delete_claim(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_SESSION();
    CHECK_OWNER();
    CHECK_INPUT(identifier, protobuf::RPCRESPONSE_INVALID);

    auto nymdata = session.Wallet().mutable_Nym(

        ot_.Factory().NymIDFromBase58(command.owner()), reason);

    for (const auto& id : command.identifier()) {
        auto deleted =
            nymdata.DeleteClaim(ot_.Factory().IdentifierFromBase58(id), reason);

        if (deleted) {
            add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
        } else {
            add_output_status(
                output, protobuf::RPCRESPONSE_DELETE_CLAIM_FAILED);
        }
    }

    return output;
}

void ProcessorPrivate::evaluate_deposit_payment(
    const api::session::Client& client,
    const api::session::OTX::Result& result,
    protobuf::TaskComplete& output) const
{
    // TODO use structured binding
    // const auto& [status, pReply] = result;
    const auto& status = std::get<0>(result);
    const auto& pReply = std::get<1>(result);

    if (otx::LastReplyStatus::NotSent == status) {
        add_output_status(output, protobuf::RPCRESPONSE_ERROR);
    } else if (otx::LastReplyStatus::Unknown == status) {
        add_output_status(output, protobuf::RPCRESPONSE_BAD_SERVER_RESPONSE);
    } else if (otx::LastReplyStatus::MessageFailed == status) {
        add_output_status(output, protobuf::RPCRESPONSE_TRANSACTION_FAILED);
    } else if (otx::LastReplyStatus::MessageSuccess == status) {
        assert_false(nullptr == pReply);

        const auto& reply = *pReply;
        evaluate_transaction_reply(client, reply, output);
    }
}

void ProcessorPrivate::evaluate_move_funds(
    const api::session::Client& client,
    const api::session::OTX::Result& result,
    protobuf::RPCResponse& output) const
{
    // TODO use structured binding
    // const auto& [status, pReply] = result;
    const auto& status = std::get<0>(result);
    const auto& pReply = std::get<1>(result);

    if (otx::LastReplyStatus::NotSent == status) {
        add_output_status(output, protobuf::RPCRESPONSE_ERROR);
    } else if (otx::LastReplyStatus::Unknown == status) {
        add_output_status(output, protobuf::RPCRESPONSE_BAD_SERVER_RESPONSE);
    } else if (otx::LastReplyStatus::MessageFailed == status) {
        add_output_status(output, protobuf::RPCRESPONSE_MOVE_FUNDS_FAILED);
    } else if (otx::LastReplyStatus::MessageSuccess == status) {
        assert_false(nullptr == pReply);

        const auto& reply = *pReply;
        evaluate_transaction_reply(client, reply, output);
    }
}

auto ProcessorPrivate::evaluate_send_payment_cheque(
    const api::session::OTX::Result& result,
    protobuf::TaskComplete& output) const noexcept -> void
{
    const auto& [status, pReply] = result;

    if (otx::LastReplyStatus::NotSent == status) {
        add_output_status(output, protobuf::RPCRESPONSE_ERROR);
    } else if (otx::LastReplyStatus::Unknown == status) {
        add_output_status(output, protobuf::RPCRESPONSE_BAD_SERVER_RESPONSE);
    } else if (otx::LastReplyStatus::MessageFailed == status) {
        add_output_status(output, protobuf::RPCRESPONSE_SEND_PAYMENT_FAILED);
    } else if (otx::LastReplyStatus::MessageSuccess == status) {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }
}

auto ProcessorPrivate::evaluate_send_payment_transfer(
    const api::session::Client& api,
    const api::session::OTX::Result& result,
    protobuf::TaskComplete& output) const noexcept -> void
{
    const auto& [status, pReply] = result;

    if (otx::LastReplyStatus::NotSent == status) {
        add_output_status(output, protobuf::RPCRESPONSE_ERROR);
    } else if (otx::LastReplyStatus::Unknown == status) {
        add_output_status(output, protobuf::RPCRESPONSE_BAD_SERVER_RESPONSE);
    } else if (otx::LastReplyStatus::MessageFailed == status) {
        add_output_status(output, protobuf::RPCRESPONSE_SEND_PAYMENT_FAILED);
    } else if (otx::LastReplyStatus::MessageSuccess == status) {
        assert_false(nullptr == pReply);

        const auto& reply = *pReply;

        evaluate_transaction_reply(api, reply, output);
    }
}

auto ProcessorPrivate::evaluate_transaction_reply(
    const api::session::Client& api,
    const Message& reply) const noexcept -> bool
{
    auto success{true};
    const auto notaryID =
        api.Factory().NotaryIDFromBase58(reply.notary_id_->Bytes());
    const auto nymID = api.Factory().NymIDFromBase58(reply.nym_id_->Bytes());
    const auto accountID =
        api.Factory().AccountIDFromBase58(reply.acct_id_->Bytes());
    const bool transaction =
        reply.command_->Compare("notarizeTransactionResponse") ||
        reply.command_->Compare("processInboxResponse") ||
        reply.command_->Compare("processNymboxResponse");

    if (transaction) {
        if (const auto sLedger = String::Factory(reply.payload_);
            sLedger->Exists()) {
            if (auto ledger{api.Factory().Internal().Session().Ledger(
                    nymID, accountID, notaryID)};
                ledger->LoadContractFromString(sLedger)) {
                if (ledger->GetTransactionCount() > 0) {
                    for (const auto& [key, value] :
                         ledger->GetTransactionMap()) {
                        if (false == bool(value)) {
                            success = false;
                            break;
                        } else {
                            success &= value->GetSuccess();
                        }
                    }
                } else {
                    success = false;
                }
            } else {
                success = false;
            }
        } else {
            success = false;
        }
    } else {
        success = false;
    }

    return success;
}

auto ProcessorPrivate::get_args(const Args& serialized) -> Options
{
    auto output = Options{};

    for (const auto& arg : serialized) {
        for (const auto& value : arg.value()) {
            output.ImportOption(arg.key().c_str(), value.c_str());
        }
    }

    return output;
}

auto ProcessorPrivate::get_client(const std::int32_t instance) const
    -> const api::session::Client*
{
    if (is_server_session(instance)) {
        LogError()()("Error: provided instance ")(
            instance)(" is a server session.")
            .Flush();

        return nullptr;
    } else {
        try {
            return &dynamic_cast<const api::session::Client&>(
                ot_.ClientSession(static_cast<int>(get_index(instance))));
        } catch (...) {
            LogError()()("Error: provided instance ")(
                instance)(" is not a valid "
                          "client session.")
                .Flush();

            return nullptr;
        }
    }
}

auto ProcessorPrivate::get_compatible_accounts(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_OWNER();
    CHECK_INPUT(identifier, protobuf::RPCRESPONSE_INVALID);

    const auto workflowID =
        ot_.Factory().IdentifierFromBase58(command.identifier(0));
    auto unitID = identifier::UnitDefinition{};

    try {
        const auto workflow = [&] {
            auto out = protobuf::PaymentWorkflow{};

            if (!client.Workflow().LoadWorkflow(ownerID, workflowID, out)) {
                throw std::runtime_error{""};
            }

            return out;
        }();

        switch (opentxs::translate(workflow.type())) {
            case otx::client::PaymentWorkflowType::IncomingCheque:
            case otx::client::PaymentWorkflowType::IncomingInvoice: {
                auto chequeState =
                    opentxs::api::session::Workflow::InstantiateCheque(
                        client, workflow);
                const auto& [state, cheque] = chequeState;

                if (false == bool(cheque)) {
                    add_output_status(
                        output, protobuf::RPCRESPONSE_CHEQUE_NOT_FOUND);

                    return output;
                }

                unitID.Assign(cheque->GetInstrumentDefinitionID());
            } break;
            default: {
                add_output_status(
                    output, protobuf::RPCRESPONSE_CHEQUE_NOT_FOUND);

                return output;
            }
        }
    } catch (...) {
        add_output_status(output, protobuf::RPCRESPONSE_WORKFLOW_NOT_FOUND);

        return output;
    }

    const auto owneraccounts =
        client.Storage().Internal().AccountsByOwner(ownerID);
    const auto unitaccounts =
        client.Storage().Internal().AccountsByContract(unitID);
    UnallocatedVector<identifier::Generic> compatible{};
    std::ranges::set_intersection(
        owneraccounts, unitaccounts, std::back_inserter(compatible));

    for (const auto& accountid : compatible) {
        output.add_identifier(accountid.asBase58(ot_.Crypto()));
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::get_index(const std::int32_t instance) -> std::size_t
{
    return (instance - (instance % 2)) / 2;
}

auto ProcessorPrivate::get_nyms(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_SESSION();
    CHECK_INPUT(identifier, protobuf::RPCRESPONSE_INVALID);

    for (const auto& id : command.identifier()) {
        auto pNym = session.Wallet().Nym(ot_.Factory().NymIDFromBase58(id));

        if (pNym) {
            auto publicNym = protobuf::Nym{};
            if (false == pNym->Internal().Serialize(publicNym)) {
                add_output_status(output, protobuf::RPCRESPONSE_NYM_NOT_FOUND);
            } else {
                *output.add_nym() = publicNym;
                add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
            }
        } else {
            add_output_status(output, protobuf::RPCRESPONSE_NYM_NOT_FOUND);
        }
    }

    return output;
}

auto ProcessorPrivate::get_pending_payments(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_OWNER();

    const auto& workflow = client.Workflow();
    auto checkWorkflows = workflow.List(
        ownerID,
        otx::client::PaymentWorkflowType::IncomingCheque,
        otx::client::PaymentWorkflowState::Conveyed);
    auto invoiceWorkflows = workflow.List(
        ownerID,
        otx::client::PaymentWorkflowType::IncomingInvoice,
        otx::client::PaymentWorkflowState::Conveyed);
    UnallocatedSet<identifier::Generic> workflows;
    std::set_union(
        checkWorkflows.begin(),
        checkWorkflows.end(),
        invoiceWorkflows.begin(),
        invoiceWorkflows.end(),
        std::inserter(workflows, workflows.end()));

    for (auto workflowID : workflows) {
        try {
            const auto paymentWorkflow = [&] {
                auto out = protobuf::PaymentWorkflow{};

                if (!workflow.LoadWorkflow(ownerID, workflowID, out)) {
                    throw std::runtime_error{""};
                }

                return out;
            }();

            auto chequeState =
                opentxs::api::session::Workflow::InstantiateCheque(
                    client, paymentWorkflow);

            const auto& [state, cheque] = chequeState;

            if (false == bool(cheque)) { continue; }

            auto& accountEvent = *output.add_accountevent();
            accountEvent.set_version(ACCOUNTEVENT_VERSION);
            auto accountEventType = protobuf::ACCOUNTEVENT_INCOMINGCHEQUE;

            if (otx::client::PaymentWorkflowType::IncomingInvoice ==
                opentxs::translate(paymentWorkflow.type())) {
                accountEventType = protobuf::ACCOUNTEVENT_INCOMINGINVOICE;
            }

            accountEvent.set_type(accountEventType);
            const auto contactID =
                client.Contacts().ContactID(cheque->GetSenderNymID());
            accountEvent.set_contact(contactID.asBase58(ot_.Crypto()));
            accountEvent.set_workflow(paymentWorkflow.id());
            cheque->GetAmount().Serialize(
                writer(accountEvent.mutable_pendingamount()));

            if (0 < paymentWorkflow.event_size()) {
                const auto paymentEvent = paymentWorkflow.event(0);
                accountEvent.set_timestamp(paymentEvent.time());
            }

            accountEvent.set_memo(cheque->GetMemo().Get());
        } catch (...) {

            continue;
        }
    }

    if (0 == output.accountevent_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::get_seeds(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_SESSION();
    CHECK_INPUT(identifier, protobuf::RPCRESPONSE_INVALID);

    if (api::crypto::HaveHDKeys()) {
        const auto& hdseeds = session.Crypto().Seed();

        for (const auto& base58 : command.identifier()) {
            const auto id = session.Factory().SeedIDFromBase58(base58);
            auto words = hdseeds.Words(id, reason);
            auto passphrase = hdseeds.Passphrase(id, reason);

            if (false == words.empty() || false == passphrase.empty()) {
                auto& seed = *output.add_seed();
                seed.set_version(SEED_VERSION);
                id.Internal().Serialize(*seed.mutable_id());
                seed.set_words(words);
                seed.set_passphrase(passphrase);
                add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
            } else {
                add_output_status(output, protobuf::RPCRESPONSE_NONE);
            }
        }
    }

    return output;
}

auto ProcessorPrivate::get_server(const std::int32_t instance) const
    -> const api::session::Notary*
{
    if (is_client_session(instance)) {
        LogError()()("Error: provided instance ")(
            instance)(" is a client session.")
            .Flush();

        return nullptr;
    } else {
        try {
            return &ot_.NotarySession(static_cast<int>(get_index(instance)));
        } catch (...) {
            LogError()()("Error: provided instance ")(
                instance)(" is not a valid server session.")
                .Flush();

            return nullptr;
        }
    }
}

auto ProcessorPrivate::get_server_admin_nym(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_SERVER_ONLY();

    const auto password = server.GetAdminNym();

    if (password.empty()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        output.add_identifier(password);
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::get_server_contracts(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_SESSION();
    CHECK_INPUT(identifier, protobuf::RPCRESPONSE_INVALID);

    for (const auto& id : command.identifier()) {
        try {
            const auto contract = session.Wallet().Internal().Server(
                ot_.Factory().NotaryIDFromBase58(id));
            auto serialized = protobuf::ServerContract{};
            if (false == contract->Serialize(serialized, true)) {
                add_output_status(output, protobuf::RPCRESPONSE_NONE);
            } else {
                *output.add_notary() = serialized;
                add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
            }
        } catch (...) {
            add_output_status(output, protobuf::RPCRESPONSE_NONE);
        }
    }

    return output;
}

auto ProcessorPrivate::get_server_password(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_SERVER_ONLY();

    const auto password = server.GetAdminPassword();

    if (password.empty()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        output.add_identifier(password);
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::get_session(const std::int32_t instance) const
    -> const api::Session&
{
    if (is_server_session(instance)) {
        return ot_.NotarySession(static_cast<int>(get_index(instance)));
    } else {
        return ot_.ClientSession(static_cast<int>(get_index(instance)));
    }
}

auto ProcessorPrivate::get_transaction_data(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_INPUT(identifier, protobuf::RPCRESPONSE_INVALID);

    for ([[maybe_unused]] const auto& id : command.identifier()) {
        add_output_status(output, protobuf::RPCRESPONSE_UNIMPLEMENTED);
    }

    return output;
}

auto ProcessorPrivate::get_unit_definitions(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_SESSION();
    CHECK_INPUT(identifier, protobuf::RPCRESPONSE_INVALID);

    for (const auto& id : command.identifier()) {
        try {
            const auto contract = session.Wallet().Internal().UnitDefinition(
                ot_.Factory().UnitIDFromBase58(id));

            if (contract->Version() > 1 && command.version() < 3) {
                add_output_status(output, protobuf::RPCRESPONSE_INVALID);

            } else {
                auto serialized = protobuf::UnitDefinition{};
                if (false == contract->Serialize(serialized, true)) {
                    add_output_status(output, protobuf::RPCRESPONSE_NONE);
                } else {
                    *output.add_unit() = serialized;
                    add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
                }
            }
        } catch (...) {
            add_output_status(output, protobuf::RPCRESPONSE_NONE);
        }
    }

    return output;
}

auto ProcessorPrivate::get_workflow(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_INPUT(getworkflow, protobuf::RPCRESPONSE_INVALID);

    for (const auto& getworkflow : command.getworkflow()) {
        try {
            const auto workflow = [&] {
                auto out = protobuf::PaymentWorkflow{};

                if (false ==
                    client.Workflow().LoadWorkflow(
                        ot_.Factory().NymIDFromBase58(getworkflow.nymid()),
                        ot_.Factory().IdentifierFromBase58(
                            getworkflow.workflowid()),
                        out)) {
                    throw std::runtime_error{""};
                }

                return out;
            }();

            auto& paymentworkflow = *output.add_workflow();
            paymentworkflow = workflow;
            add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
        } catch (...) {
            add_output_status(output, protobuf::RPCRESPONSE_NONE);
        }
    }

    return output;
}

auto ProcessorPrivate::immediate_create_account(
    const api::session::Client& client,
    const identifier::Nym& owner,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit) const -> bool
{
    const auto registered =
        client.Internal().asClient().OTAPI().IsNym_RegisteredAtServer(
            owner, notary);

    try {
        client.Wallet().Internal().UnitDefinition(unit);
    } catch (...) {

        return false;
    }

    return registered;
}

auto ProcessorPrivate::immediate_register_issuer_account(
    const api::session::Client& client,
    const identifier::Nym& owner,
    const identifier::Notary& notary) const -> bool
{
    return client.Internal().asClient().OTAPI().IsNym_RegisteredAtServer(
        owner, notary);
}

auto ProcessorPrivate::immediate_register_nym(
    const api::session::Client& client,
    const identifier::Notary& notary) const -> bool
{
    try {
        client.Wallet().Internal().Server(notary);

        return true;
    } catch (...) {

        return false;
    }
}

auto ProcessorPrivate::import_seed(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_SESSION();

    if (api::crypto::HaveHDKeys()) {
        const auto& seed = command.hdseed();
        auto words = ot_.Factory().SecretFromText(seed.words());
        auto passphrase = ot_.Factory().SecretFromText(seed.passphrase());
        const auto identifier = session.Crypto().Seed().ImportSeed(
            words,
            passphrase,
            crypto::SeedStyle::BIP39,
            crypto::Language::en,
            reason);

        if (identifier.empty()) {
            add_output_status(output, protobuf::RPCRESPONSE_INVALID);
        } else {
            output.add_identifier(identifier.asBase58(ot_.Crypto()));
            add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
        }
    }

    return output;
}

auto ProcessorPrivate::import_server_contract(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_SESSION();
    CHECK_INPUT(server, protobuf::RPCRESPONSE_INVALID);

    for (const auto& servercontract : command.server()) {
        try {
            session.Wallet().Internal().Server(servercontract);
            add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
        } catch (...) {
            add_output_status(output, protobuf::RPCRESPONSE_NONE);
        }
    }

    return output;
}

auto ProcessorPrivate::init(const protobuf::RPCCommand& command)
    -> protobuf::RPCResponse
{
    protobuf::RPCResponse output{};
    output.set_version(command.version());
    output.set_cookie(command.cookie());
    output.set_type(command.type());

    return output;
}

auto ProcessorPrivate::invalid_command(const protobuf::RPCCommand& command)
    -> protobuf::RPCResponse
{
    INIT();

    add_output_status(output, protobuf::RPCRESPONSE_INVALID);

    return output;
}

auto ProcessorPrivate::is_blockchain_account(
    const request::Message& base,
    const identifier::Account& id) const noexcept -> bool
{
    try {
        const auto& api = client_session(base);
        const auto [chain, owner] = api.Crypto().Blockchain().LookupAccount(id);

        if (blockchain::Type::UnknownBlockchain == chain) { return false; }

        return true;
    } catch (...) {

        return false;
    }
}

auto ProcessorPrivate::is_client_session(std::int32_t instance) const -> bool
{
    return instance % 2 == 0;
}

auto ProcessorPrivate::is_server_session(std::int32_t instance) const -> bool
{
    return instance % 2 != 0;
}

auto ProcessorPrivate::is_session_valid(std::int32_t instance) const -> bool
{
    auto lock = Lock{lock_};
    auto index = get_index(instance);

    if (is_server_session(instance)) {
        return ot_.NotarySessionCount() > index;
    } else {
        return ot_.ClientSessionCount() > index;
    }
}

auto ProcessorPrivate::list_client_sessions(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT();
    auto lock = Lock{lock_};

    for (std::size_t i = 0; i < ot_.ClientSessionCount(); ++i) {
        protobuf::SessionData& data = *output.add_sessions();
        data.set_version(SESSION_DATA_VERSION);
        data.set_instance(ot_.ClientSession(static_cast<int>(i)).Instance());
    }

    if (0 == output.sessions_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::list_contacts(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();

    auto contacts = client.Contacts().ContactList();

    for (const auto& contact : contacts) {
        output.add_identifier(std::get<0>(contact));
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::list_seeds(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_SESSION();

    const auto& seeds = session.Storage().SeedList();

    if (0 < seeds.size()) {
        for (const auto& seed : seeds) { output.add_identifier(seed.first); }
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::list_server_contracts(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_SESSION();

    const auto& servers = session.Wallet().ServerList();

    for (const auto& server : servers) { output.add_identifier(server.first); }

    if (0 == output.identifier_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::list_server_sessions(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT();
    auto lock = Lock{lock_};

    for (std::size_t i = 0; i < ot_.NotarySessionCount(); ++i) {
        auto& data = *output.add_sessions();
        data.set_version(SESSION_DATA_VERSION);
        data.set_instance(ot_.NotarySession(static_cast<int>(i)).Instance());
    }

    if (0 == output.sessions_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::list_unit_definitions(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_SESSION();

    const auto& unitdefinitions = session.Wallet().UnitDefinitionList();

    for (const auto& unitdefinition : unitdefinitions) {
        output.add_identifier(unitdefinition.first);
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::lookup_account_id(
    const protobuf::RPCCommand& command) const -> protobuf::RPCResponse
{
    INIT_SESSION();

    const auto& label = command.param();

    for (const auto& [id, alias] : session.Storage().Internal().AccountList()) {
        if (alias == label) { output.add_identifier(id); }
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, protobuf::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }

    return output;
}

auto ProcessorPrivate::move_funds(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();

    const auto& movefunds = command.movefunds();
    const auto sourceaccount =
        ot_.Factory().AccountIDFromBase58(movefunds.sourceaccount());
    auto sender = client.Storage().Internal().AccountOwner(sourceaccount);

    switch (movefunds.type()) {
        case protobuf::RPCPAYMENTTYPE_TRANSFER: {
            const auto targetaccount = ot_.Factory().AccountIDFromBase58(
                movefunds.destinationaccount());
            const auto notary =
                client.Storage().Internal().AccountServer(sourceaccount);

            // NOLINTBEGIN(misc-const-correctness)
            INIT_OTX(
                SendTransfer,
                sender,
                notary,
                sourceaccount,
                targetaccount,
                factory::Amount(movefunds.amount()),
                movefunds.memo());
            // NOLINTEND(misc-const-correctness)

            if (false == ready) {
                add_output_status(output, protobuf::RPCRESPONSE_ERROR);

                return output;
            }

            evaluate_move_funds(client, future.get(), output);
        } break;
        case protobuf::RPCPAYMENTTYPE_CHEQUE:
        case protobuf::RPCPAYMENTTYPE_VOUCHER:
        case protobuf::RPCPAYMENTTYPE_INVOICE:
        case protobuf::RPCPAYMENTTYPE_BLINDED:
        case protobuf::RPCPAYMENTTYPE_ERROR:
        case protobuf::RPCPAYMENTTYPE_BLOCKCHAIN:
        default: {
            add_output_status(output, protobuf::RPCRESPONSE_INVALID);
        }
    }

    return output;
}

auto ProcessorPrivate::Process(
    const protobuf::RPCCommand& command) const noexcept -> protobuf::RPCResponse
{
    const auto valid = protobuf::syntax::check(LogError(), command);

    if (false == valid) {
        LogError()()("Invalid serialized command").Flush();

        return invalid_command(command);
    }

    switch (command.type()) {
        case protobuf::RPCCOMMAND_LISTNYMS:
        case protobuf::RPCCOMMAND_LISTACCOUNTS:
        case protobuf::RPCCOMMAND_GETACCOUNTBALANCE:
        case protobuf::RPCCOMMAND_GETACCOUNTACTIVITY:
        case protobuf::RPCCOMMAND_SENDPAYMENT: {
            const auto response = Process(*factory::RPCRequest(command));
            auto output = protobuf::RPCResponse{};
            response->Serialize(output);

            return output;
        }
        case protobuf::RPCCOMMAND_ADDCLIENTSESSION: {
            return start_client(command);
        }
        case protobuf::RPCCOMMAND_ADDSERVERSESSION: {
            return start_server(command);
        }
        case protobuf::RPCCOMMAND_LISTCLIENTSESSIONS: {
            return list_client_sessions(command);
        }
        case protobuf::RPCCOMMAND_LISTSERVERSESSIONS: {
            return list_server_sessions(command);
        }
        case protobuf::RPCCOMMAND_IMPORTHDSEED: {
            return import_seed(command);
        }
        case protobuf::RPCCOMMAND_LISTHDSEEDS: {
            return list_seeds(command);
        }
        case protobuf::RPCCOMMAND_GETHDSEED: {
            return get_seeds(command);
        }
        case protobuf::RPCCOMMAND_CREATENYM: {
            return create_nym(command);
        }
        case protobuf::RPCCOMMAND_GETNYM: {
            return get_nyms(command);
        }
        case protobuf::RPCCOMMAND_ADDCLAIM: {
            return add_claim(command);
        }
        case protobuf::RPCCOMMAND_DELETECLAIM: {
            return delete_claim(command);
        }
        case protobuf::RPCCOMMAND_IMPORTSERVERCONTRACT: {
            return import_server_contract(command);
        }
        case protobuf::RPCCOMMAND_LISTSERVERCONTRACTS: {
            return list_server_contracts(command);
        }
        case protobuf::RPCCOMMAND_REGISTERNYM: {
            return register_nym(command);
        }
        case protobuf::RPCCOMMAND_CREATEUNITDEFINITION: {
            return create_unit_definition(command);
        }
        case protobuf::RPCCOMMAND_LISTUNITDEFINITIONS: {
            return list_unit_definitions(command);
        }
        case protobuf::RPCCOMMAND_ISSUEUNITDEFINITION: {
            return create_issuer_account(command);
        }
        case protobuf::RPCCOMMAND_CREATEACCOUNT: {
            return create_account(command);
        }
        case protobuf::RPCCOMMAND_MOVEFUNDS: {
            return move_funds(command);
        }
        case protobuf::RPCCOMMAND_GETSERVERCONTRACT: {
            return get_server_contracts(command);
        }
        case protobuf::RPCCOMMAND_ADDCONTACT: {
            return add_contact(command);
        }
        case protobuf::RPCCOMMAND_LISTCONTACTS: {
            return list_contacts(command);
        }
        case protobuf::RPCCOMMAND_GETCONTACT:
        case protobuf::RPCCOMMAND_ADDCONTACTCLAIM:
        case protobuf::RPCCOMMAND_DELETECONTACTCLAIM:
        case protobuf::RPCCOMMAND_VERIFYCLAIM:
        case protobuf::RPCCOMMAND_ACCEPTVERIFICATION:
        case protobuf::RPCCOMMAND_SENDCONTACTMESSAGE:
        case protobuf::RPCCOMMAND_GETCONTACTACTIVITY: {
            LogError()()("Command not implemented.").Flush();
        } break;
        case protobuf::RPCCOMMAND_ACCEPTPENDINGPAYMENTS: {
            return accept_pending_payments(command);
        }
        case protobuf::RPCCOMMAND_GETPENDINGPAYMENTS: {
            return get_pending_payments(command);
        }
        case protobuf::RPCCOMMAND_GETCOMPATIBLEACCOUNTS: {
            return get_compatible_accounts(command);
        }
        case protobuf::RPCCOMMAND_CREATECOMPATIBLEACCOUNT: {
            return create_compatible_account(command);
        }
        case protobuf::RPCCOMMAND_GETWORKFLOW: {
            return get_workflow(command);
        }
        case protobuf::RPCCOMMAND_GETSERVERPASSWORD: {
            return get_server_password(command);
        }
        case protobuf::RPCCOMMAND_GETADMINNYM: {
            return get_server_admin_nym(command);
        }
        case protobuf::RPCCOMMAND_GETUNITDEFINITION: {
            return get_unit_definitions(command);
        }
        case protobuf::RPCCOMMAND_GETTRANSACTIONDATA: {
            return get_transaction_data(command);
        }
        case protobuf::RPCCOMMAND_LOOKUPACCOUNTID: {
            return lookup_account_id(command);
        }
        case protobuf::RPCCOMMAND_RENAMEACCOUNT: {
            return rename_account(command);
        }
        case protobuf::RPCCOMMAND_ERROR:
        default: {
            LogError()()("Unsupported command.").Flush();
        }
    }

    return invalid_command(command);
}

auto ProcessorPrivate::Process(const request::Message& command) const noexcept
    -> std::unique_ptr<response::Message>
{
    switch (command.Type()) {
        case CommandType::list_nyms: {
            return list_nyms(command);
        }
        case CommandType::list_accounts: {
            return list_accounts(command);
        }
        case CommandType::get_account_balance: {
            return get_account_balance(command);
        }
        case CommandType::get_account_activity: {
            return get_account_activity(command);
        }
        case CommandType::send_payment: {
            return send_payment(command);
        }
        case CommandType::add_client_session:
        case CommandType::add_server_session:
        case CommandType::list_client_sessions:
        case CommandType::list_server_sessions:
        case CommandType::import_hd_seed:
        case CommandType::list_hd_seeds:
        case CommandType::get_hd_seed:
        case CommandType::create_nym:
        case CommandType::get_nym:
        case CommandType::add_claim:
        case CommandType::delete_claim:
        case CommandType::import_server_contract:
        case CommandType::list_server_contracts:
        case CommandType::register_nym:
        case CommandType::create_unit_definition:
        case CommandType::list_unit_definitions:
        case CommandType::issue_unit_definition:
        case CommandType::create_account:
        case CommandType::move_funds:
        case CommandType::add_contact:
        case CommandType::list_contacts:
        case CommandType::get_contact:
        case CommandType::add_contact_claim:
        case CommandType::delete_contact_claim:
        case CommandType::verify_claim:
        case CommandType::accept_verification:
        case CommandType::send_contact_message:
        case CommandType::get_contact_activity:
        case CommandType::get_server_contract:
        case CommandType::get_pending_payments:
        case CommandType::accept_pending_payments:
        case CommandType::get_compatible_accounts:
        case CommandType::create_compatible_account:
        case CommandType::get_workflow:
        case CommandType::get_server_password:
        case CommandType::get_admin_nym:
        case CommandType::get_unit_definition:
        case CommandType::get_transaction_data:
        case CommandType::lookup_accountid:
        case CommandType::rename_account:
        case CommandType::error:
        default: {
            LogError()()("Unsupported command.").Flush();

            return std::make_unique<response::Invalid>(command);
        }
    }
}

auto ProcessorPrivate::queue_task(
    const identifier::Nym& nymID,
    const UnallocatedCString taskID,
    Finish&& finish,
    Future&& future,
    protobuf::RPCResponse& output) const -> void
{
    auto lock = Lock{task_lock_};
    queued_tasks_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(taskID),
        std::forward_as_tuple(std::move(future), std::move(finish), nymID));
    const auto taskIDCompat = ot_.Factory().IdentifierFromPreimage(taskID);
    add_output_task(output, taskIDCompat.asBase58(ot_.Crypto()));
    add_output_status(output, protobuf::RPCRESPONSE_QUEUED);
}

auto ProcessorPrivate::queue_task(
    const api::Session& api,
    const identifier::Nym& nymID,
    const UnallocatedCString taskID,
    Finish&& finish,
    Future&& future) const noexcept -> UnallocatedCString
{
    {
        auto lock = Lock{task_lock_};
        queued_tasks_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(taskID),
            std::forward_as_tuple(std::move(future), std::move(finish), nymID));
    }

    const auto hashedID = ot_.Factory().IdentifierFromPreimage(taskID);

    return hashedID.asBase58(ot_.Crypto());
}

auto ProcessorPrivate::register_nym(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_OWNER();

    const auto notaryID = ot_.Factory().NotaryIDFromBase58(command.notary());
    auto registered =
        client.Internal().asClient().OTAPI().IsNym_RegisteredAtServer(
            ownerID, notaryID);

    if (registered) {
        add_output_status(output, protobuf::RPCRESPONSE_UNNECESSARY);

        return output;
    }

    // NOLINTBEGIN(misc-const-correctness)
    INIT_OTX(RegisterNymPublic, ownerID, notaryID, true);
    // NOLINTEND(misc-const-correctness)

    if (false == ready) {
        add_output_status(output, protobuf::RPCRESPONSE_REGISTER_NYM_FAILED);

        return output;
    }

    if (immediate_register_nym(client, notaryID)) {
        evaluate_register_nym(future.get(), output);
    } else {
        queue_task(
            ownerID,
            std::to_string(taskID),
            [&](const auto& in, auto& out) -> void {
                evaluate_register_nym(in, out);
            },
            std::move(future),
            output);
    }

    return output;
}

auto ProcessorPrivate::rename_account(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT_CLIENT_ONLY();
    CHECK_INPUT(modifyaccount, protobuf::RPCRESPONSE_INVALID);

    for (const auto& rename : command.modifyaccount()) {
        const auto accountID =
            ot_.Factory().AccountIDFromBase58(rename.accountid());
        auto account =
            client.Wallet().Internal().mutable_Account(accountID, reason);

        if (account) {
            account.get().SetAlias(rename.label());
            add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
        } else {
            add_output_status(output, protobuf::RPCRESPONSE_ACCOUNT_NOT_FOUND);
        }
    }

    return output;
}

auto ProcessorPrivate::session(const request::Message& command) const
    noexcept(false) -> const api::Session&
{
    const auto session = command.Session();

    if (false == is_session_valid(session)) {

        throw std::runtime_error{"invalid session"};
    }

    if (is_client_session(session)) {

        return ot_.ClientSession(static_cast<int>(get_index(session)));
    } else {

        return ot_.NotarySession(static_cast<int>(get_index(session)));
    }
}

auto ProcessorPrivate::start_client(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT();
    auto lock = Lock{lock_};
    const auto session{static_cast<std::uint32_t>(ot_.ClientSessionCount())};

    std::uint32_t instance{0};

    try {
        const auto& manager =
            ot_.StartClientSession(get_args(command.arg()), session);
        instance = manager.Instance();

        auto bound = task_subscriber_->Start(
            UnallocatedCString{manager.Endpoints().TaskComplete()});

        assert_true(bound);

    } catch (...) {
        add_output_status(output, protobuf::RPCRESPONSE_INVALID);

        return output;
    }

    output.set_session(instance);
    add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);

    return output;
}

auto ProcessorPrivate::start_server(const protobuf::RPCCommand& command) const
    -> protobuf::RPCResponse
{
    INIT();
    auto lock = Lock{lock_};
    const auto session{static_cast<std::uint32_t>(ot_.NotarySessionCount())};

    std::uint32_t instance{0};

    try {
        const auto& manager =
            ot_.StartNotarySession(get_args(command.arg()), session);
        instance = manager.Instance();
    } catch (const std::invalid_argument&) {
        add_output_status(output, protobuf::RPCRESPONSE_BAD_SERVER_ARGUMENT);
        return output;
    } catch (...) {
        add_output_status(output, protobuf::RPCRESPONSE_INVALID);
        return output;
    }

    output.set_session(instance);
    add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);

    return output;
}

auto ProcessorPrivate::status(
    const response::Message::Identifiers& ids) const noexcept -> ResponseCode
{
    return (0u == ids.size()) ? ResponseCode::none : ResponseCode::success;
}

void ProcessorPrivate::task_handler(const zmq::Message& in)
{
    const auto body = in.Payload();

    assert_true(2 < body.size());

    using ID = api::session::OTX::TaskID;

    const auto taskID = std::to_string(body[1].as<ID>());
    const auto success = body[2].as<bool>();
    LogTrace()()("Received notice for task ")(taskID).Flush();
    auto lock = Lock{task_lock_};
    auto it = queued_tasks_.find(taskID);
    lock.unlock();
    protobuf::RPCPush message{};
    auto& task = *message.mutable_taskcomplete();

    if (queued_tasks_.end() != it) {
        auto& [future, finish, nymID] = it->second;
        message.set_id(nymID.asBase58(ot_.Crypto()));

        if (finish) { finish(future.get(), task); }
    } else {
        LogTrace()()("We don't care about task ")(taskID).Flush();

        return;
    }

    lock.lock();
    queued_tasks_.erase(it);
    lock.unlock();
    message.set_version(RPCPUSH_VERSION);
    message.set_type(protobuf::RPCPUSH_TASK);
    task.set_version(TASKCOMPLETE_VERSION);
    const auto taskIDStr = String::Factory(taskID);
    const auto taskIDCompat =
        ot_.Factory().IdentifierFromPreimage(taskIDStr->Bytes());
    task.set_id(taskIDCompat.asBase58(ot_.Crypto()));
    task.set_result(success);
    auto output = zmq::Message{};
    output.Internal().AddFrame(message);
    rpc_publisher_->Send(std::move(output));
}

ProcessorPrivate::~ProcessorPrivate()
{
    task_subscriber_->Close();
    rpc_publisher_->Close();
    push_receiver_->Close();
}
}  // namespace opentxs::rpc

#undef INIT_OTX
#undef INIT_SERVER_ONLY
#undef INIT_CLIENT_ONLY
#undef INIT_SESSION
#undef INIT
#undef CHECK_OWNER
#undef CHECK_INPUT
