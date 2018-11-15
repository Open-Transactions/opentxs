// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/ServerAction.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/PullSocket.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/Proto.hpp"

#include "internal/rpc/Internal.hpp"

#include <algorithm>

#include "RPC.hpp"

#define ACCOUNTEVENT_VERSION 2
#define ACCOUNTDATA_VERSION 2
#define RPCTASK_VERSION 1
#define SEED_VERSION 1
#define SESSION_DATA_VERSION 1

#define OT_METHOD "opentxs::rpc::implementation::RPC::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
rpc::internal::RPC* Factory::RPC(const api::Native& native)
{
    return new rpc::implementation::RPC(native);
}
}  // namespace opentxs

namespace opentxs::rpc::implementation
{
RPC::RPC(const api::Native& native)
    : Lockable()
    , ot_(native)
    , push_callback_(zmq::ListenCallback::Factory([&](const zmq::Message& in) {
        rpc_publisher_->Publish(OTZMQMessage{in});
    }))
    , push_receiver_(
          ot_.ZMQ().PullSocket(push_callback_, zmq::Socket::Direction::Bind))
    , rpc_publisher_(ot_.ZMQ().PublishSocket())
{
    auto bound = push_receiver_->Start(
        ot_.ZMQ().BuildEndpoint("rpc/push/internal", -1, 1));

    OT_ASSERT(bound)

    bound = rpc_publisher_->Start(ot_.ZMQ().BuildEndpoint("rpc/push", -1, 1));

    OT_ASSERT(bound)
}

proto::RPCResponse RPC::accept_pending_payments(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (0 == command.acceptpendingpayment_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    for (auto acceptpendingpayment : command.acceptpendingpayment()) {
        const auto destinationaccountID =
            Identifier::Factory(acceptpendingpayment.destinationaccount());
        const auto workflowID =
            Identifier::Factory(acceptpendingpayment.workflow());
        const auto nymID = client.Storage().AccountOwner(destinationaccountID);
        const auto paymentWorkflow =
            client.Workflow().LoadWorkflow(nymID, workflowID);

        if (false == bool(paymentWorkflow)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid workflow ")(
                workflowID)
                .Flush();
            add_output_task(output, "");
            add_output_status(output, proto::RPCRESPONSE_ERROR);

            continue;
        }

        std::shared_ptr<const OTPayment> payment{};

        switch (paymentWorkflow->type()) {
            case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
            case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
                auto chequeState =
                    opentxs::api::client::Workflow::InstantiateCheque(
                        client, *paymentWorkflow);
                const auto& [state, cheque] = chequeState;

                if (false == bool(cheque)) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Unable to load cheque from workflow")
                        .Flush();
                    add_output_task(output, "");
                    add_output_status(output, proto::RPCRESPONSE_ERROR);

                    continue;
                }

                payment.reset(client.Factory().Payment(*cheque).release());
            } break;
            case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
            case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
            case proto::PAYMENTWORKFLOWTYPE_ERROR:
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unsupported workflow type")
                    .Flush();
                add_output_task(output, "");
                add_output_status(output, proto::RPCRESPONSE_ERROR);

                continue;
            }
        }

        if (false == bool(payment)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to instantiate payment")
                .Flush();
            add_output_task(output, "");
            add_output_status(output, proto::RPCRESPONSE_ERROR);

            continue;
        }

        const auto taskid =
            client.Sync().DepositPayment(nymID, destinationaccountID, payment);

        if (false == taskid->empty()) {
            add_output_task(output, taskid->str());
            add_output_status(output, proto::RPCRESPONSE_QUEUED);
        } else {
            add_output_task(output, "");
            add_output_status(output, proto::RPCRESPONSE_ERROR);
        }
    }

    if (0 == output.task_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    }

    return output;
}

proto::RPCResponse RPC::add_claim(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    if (false == session.Wallet().IsLocalNym(command.owner())) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    auto nymdata =
        session.Wallet().mutable_Nym(Identifier::Factory(command.owner()));
    for (const auto& addclaim : command.claim()) {
        const auto& contactitem = addclaim.item();
        std::set<std::uint32_t> attributes(
            contactitem.attribute().begin(), contactitem.attribute().end());
        auto claim = Claim(
            contactitem.id(),
            addclaim.sectionversion(),
            contactitem.type(),
            contactitem.value(),
            contactitem.start(),
            contactitem.end(),
            attributes);
        auto added = nymdata.AddClaim(claim);
        if (added) {
            add_output_status(output, proto::RPCRESPONSE_SUCCESS);
        } else {
            add_output_status(output, proto::RPCRESPONSE_ERROR);
        }
    }

    if (0 == output.status_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    }

    return output;
}

proto::RPCResponse RPC::add_contact(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (0 == command.addcontact_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    for (const auto& addContact : command.addcontact()) {
        const auto contact = client.Contacts().NewContact(
            addContact.label(),
            Identifier::Factory(addContact.nymid()),
            client.Factory().PaymentCode(addContact.paymentcode()));

        if (false == bool(contact)) {
            add_output_status(output, proto::RPCRESPONSE_ERROR);
        } else {
            output.add_identifier(contact->ID().str());
            add_output_status(output, proto::RPCRESPONSE_SUCCESS);
        }
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    }

    return output;
}

void RPC::add_output_status(
    proto::RPCResponse& output,
    proto::RPCResponseCode code)
{
    auto& status = *output.add_status();
    status.set_version(output.version());
    status.set_index(output.status_size() - 1);
    status.set_code(code);
}

void RPC::add_output_task(proto::RPCResponse& output, const std::string& taskid)
{
    auto& task = *output.add_task();
    task.set_version(RPCTASK_VERSION);
    task.set_index(output.task_size() - 1);
    task.set_id(taskid);
}

proto::RPCResponse RPC::create_account(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    if (false == client.Wallet().IsLocalNym(command.owner())) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto ownerid = Identifier::Factory(command.owner()),
               notaryid = Identifier::Factory(command.notary()),
               unitdefinitionid = Identifier::Factory(command.unit());
    const auto registered =
        client.OTAPI().IsNym_RegisteredAtServer(ownerid, notaryid);
    const auto unitdefinition =
        client.Wallet().UnitDefinition(unitdefinitionid);
    std::string label{};

    if (0 < command.identifier_size()) { label = command.identifier(0); }

    if (registered && bool(unitdefinition)) {
        auto action = client.ServerAction().RegisterAccount(
            ownerid, notaryid, unitdefinitionid, label);
        action->Run();

        if (SendResult::VALID_REPLY == action->LastSendResult()) {
            const auto reply = action->Reply();

            if (false == reply->m_bSuccess) {
                add_output_status(output, proto::RPCRESPONSE_ERROR);
            } else {
                output.add_identifier(reply->m_strAcctID->Get());
                add_output_status(output, proto::RPCRESPONSE_SUCCESS);
            }
        } else {
            add_output_status(output, proto::RPCRESPONSE_ERROR);
        }
    } else {
        const auto taskid = client.Sync().ScheduleRegisterAccount(
            ownerid, notaryid, unitdefinitionid, label);
        add_output_task(output, taskid->str());
        add_output_status(output, proto::RPCRESPONSE_QUEUED);
    }

    return output;
}

proto::RPCResponse RPC::create_compatible_account(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    if (false == client.Wallet().IsLocalNym(command.owner())) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto ownerID = Identifier::Factory(command.owner());
    const auto workflowID = Identifier::Factory(command.identifier(0));
    const auto paymentWorkflow =
        client.Workflow().LoadWorkflow(ownerID, workflowID);

    if (false == bool(paymentWorkflow)) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    auto chequeState = opentxs::api::client::Workflow::InstantiateCheque(
        client, *paymentWorkflow);

    const auto& [state, cheque] = chequeState;

    if (false == bool(cheque)) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto& unitdefinitionid = cheque->GetInstrumentDefinitionID();
    const auto& notaryid = cheque->GetNotaryID();

    const auto registered =
        client.OTAPI().IsNym_RegisteredAtServer(ownerID, notaryid);

    auto contract = client.Wallet().UnitDefinition(unitdefinitionid);

    if (false == bool(contract)) {
        auto action = client.ServerAction().DownloadContract(
            ownerID, notaryid, unitdefinitionid);
        action->Run();

        if (SendResult::VALID_REPLY == action->LastSendResult()) {
            contract = client.Wallet().UnitDefinition(unitdefinitionid);

            if (false == bool(contract)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Notary ")(notaryid)(
                    " does not have unit definition ")(unitdefinitionid)
                    .Flush();
            }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Communication error while downloading unit definition ")(
                unitdefinitionid)(" on server ")(notaryid)
                .Flush();
        }
    }

    if (registered && bool(contract)) {
        auto action = client.ServerAction().RegisterAccount(
            ownerID, notaryid, unitdefinitionid);
        action->Run();

        if (SendResult::VALID_REPLY == action->LastSendResult()) {
            const auto reply = action->Reply();

            if (false == reply->m_bSuccess) {
                add_output_status(output, proto::RPCRESPONSE_ERROR);
            } else {
                output.add_identifier(reply->m_strAcctID->Get());
                add_output_status(output, proto::RPCRESPONSE_SUCCESS);
            }
        } else {
            add_output_status(output, proto::RPCRESPONSE_ERROR);
        }
    } else {
        const auto taskid = client.Sync().ScheduleRegisterAccount(
            ownerID, notaryid, unitdefinitionid);
        add_output_task(output, taskid->str());
        add_output_status(output, proto::RPCRESPONSE_QUEUED);
    }

    return output;
}

proto::RPCResponse RPC::create_issuer_account(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    if (false == client.Wallet().IsLocalNym(command.owner())) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto ownerid = Identifier::Factory(command.owner()),
               notaryid = Identifier::Factory(command.notary()),
               unitdefinitionid = Identifier::Factory(command.unit());

    const auto unitdefinition =
        client.Wallet().UnitDefinition(unitdefinitionid);

    if (false == bool(unitdefinition) ||
        ownerid != unitdefinition->Nym()->ID()) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto account = client.Wallet().IssuerAccount(unitdefinitionid);

    if (false != bool(account)) {
        add_output_status(output, proto::RPCRESPONSE_UNNECESSARY);
        return output;
    }

    auto registered =
        client.OTAPI().IsNym_RegisteredAtServer(ownerid, notaryid);
    std::string label{};

    if (0 < command.identifier_size()) { label = command.identifier(0); }

    if (registered) {
        auto action = client.ServerAction().IssueUnitDefinition(
            ownerid, notaryid, unitdefinition->PublicContract(), label);
        action->Run();

        if (SendResult::VALID_REPLY == action->LastSendResult()) {
            auto reply = action->Reply();

            if (false == bool(reply) || false == reply->m_bSuccess) {
                add_output_status(output, proto::RPCRESPONSE_ERROR);
            } else {
                output.add_identifier(action->Reply()->m_strAcctID->Get());
                add_output_status(output, proto::RPCRESPONSE_SUCCESS);
            }
        } else {
            add_output_status(output, proto::RPCRESPONSE_ERROR);
        }
    } else {
        const auto taskid = client.Sync().ScheduleIssueUnitDefinition(
            ownerid, notaryid, unitdefinitionid, label);
        add_output_task(output, taskid->str());
        add_output_status(output, proto::RPCRESPONSE_QUEUED);
    }

    return output;
}

proto::RPCResponse RPC::create_nym(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);

        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);

        return output;
    }

    auto* session = get_client(command.session());

    if (nullptr == session) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);

        return output;
    }

    const auto& client = *session;
    const auto& createnym = command.createnym();
    auto identifier = client.Exec().CreateNymHD(
        createnym.type(),
        createnym.name(),
        createnym.seedid(),
        createnym.index());

    if (identifier.empty()) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
    } else {
        if (0 < createnym.claims_size()) {
            auto nymdata =
                client.Wallet().mutable_Nym(Identifier::Factory(identifier));

            for (const auto& addclaim : createnym.claims()) {
                const auto& contactitem = addclaim.item();
                std::set<std::uint32_t> attributes(
                    contactitem.attribute().begin(),
                    contactitem.attribute().end());
                auto claim = Claim(
                    contactitem.id(),
                    addclaim.sectionversion(),
                    contactitem.type(),
                    contactitem.value(),
                    contactitem.start(),
                    contactitem.end(),
                    attributes);
                nymdata.AddClaim(claim);
            }
        }

        output.add_identifier(identifier);
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::create_unit_definition(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    if (false == session.Wallet().IsLocalNym(command.owner())) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto& createunit = command.createunit();
    auto unitdefinition = session.Wallet().UnitDefinition(
        command.owner(),
        createunit.primaryunitname(),
        createunit.name(),
        createunit.symbol(),
        createunit.terms(),
        createunit.tla(),
        createunit.power(),
        createunit.fractionalunitname());

    if (false != bool(unitdefinition)) {
        output.add_identifier(unitdefinition->ID()->str());
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    } else {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
    }

    return output;
}

proto::RPCResponse RPC::delete_claim(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    if (false == session.Wallet().IsLocalNym(command.owner())) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    auto nymdata =
        session.Wallet().mutable_Nym(Identifier::Factory(command.owner()));
    for (const auto& id : command.identifier()) {
        auto deleted = nymdata.DeleteClaim(Identifier::Factory(id));

        if (deleted) {
            add_output_status(output, proto::RPCRESPONSE_SUCCESS);
        } else {
            add_output_status(output, proto::RPCRESPONSE_ERROR);
        }
    }

    if (0 == output.status_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    }

    return output;
}

bool RPC::establish_payment_prerequisites(
    const api::client::Manager& client,
    const Identifier& nymID,
    const Identifier& serverID,
    const std::size_t transactionNumbers) const
{
    auto downloaded = client.ServerAction().DownloadNymbox(nymID, serverID);

    if (false == downloaded) { return false; }

    auto gotnumbers = client.ServerAction().GetTransactionNumbers(
        nymID, serverID, transactionNumbers);

    if (false == gotnumbers) { return false; }

    downloaded = client.ServerAction().DownloadNymbox(nymID, serverID);

    if (false == downloaded) { return false; }

    return true;
}

proto::RPCResponse RPC::get_account_activity(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    for (const auto& id : command.identifier()) {
        const auto accountid = Identifier::Factory(id);
        const auto accountownerid = client.Storage().AccountOwner(accountid);
        auto& accountactivity =
            client.UI().AccountActivity(accountownerid, accountid);
        auto balanceitem = accountactivity.First();

        if (!balanceitem->Valid()) {
            add_output_status(output, proto::RPCRESPONSE_NONE);
            continue;
        }

        auto last = false;

        while (false == last) {
            auto& accountevent = *output.add_accountevent();
            accountevent.set_version(ACCOUNTEVENT_VERSION);
            accountevent.set_id(id);

            auto accounteventtype =
                storagebox_to_accounteventtype(balanceitem->Type());
            if (proto::ACCOUNTEVENT_ERROR == accounteventtype &&
                StorageBox::INTERNALTRANSFER == balanceitem->Type()) {
                if (0 > balanceitem->Amount()) {
                    accounteventtype = proto::ACCOUNTEVENT_OUTGOINGTRANSFER;
                } else {
                    accounteventtype = proto::ACCOUNTEVENT_INCOMINGTRANSFER;
                }
            }
            accountevent.set_type(accounteventtype);

            if (0 < balanceitem->Contacts().size()) {
                accountevent.set_contact(balanceitem->Contacts().at(0));
            } else if (
                proto::ACCOUNTEVENT_INCOMINGTRANSFER == accounteventtype) {
                const auto contactid =
                    client.Contacts().ContactID(accountownerid);
                accountevent.set_contact(contactid->str());
            }

            accountevent.set_workflow(balanceitem->Workflow());
            accountevent.set_amount(balanceitem->Amount());
            accountevent.set_pendingamount(balanceitem->Amount());
            accountevent.set_timestamp(
                std::chrono::system_clock::to_time_t(balanceitem->Timestamp()));
            accountevent.set_memo(balanceitem->Memo());
            accountevent.set_uuid(balanceitem->UUID());
            auto workflow = client.Workflow().LoadWorkflow(
                accountownerid, Identifier::Factory(balanceitem->Workflow()));

            if (workflow) { accountevent.set_state(workflow->state()); }

            if (balanceitem->Last()) {
                last = true;
            } else {
                balanceitem = accountactivity.Next();
            }
        }

        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::get_account_balance(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    auto& session = get_session(command.session());

    for (const auto& id : command.identifier()) {
        const auto accountid = Identifier::Factory(id);
        const auto account = session.Wallet().Account(accountid);

        if (false != bool(account)) {
            auto& accountdata = *output.add_balance();
            accountdata.set_version(ACCOUNTDATA_VERSION);
            accountdata.set_id(id);
            accountdata.set_label(session.Storage().AccountAlias(accountid));
            accountdata.set_unit(
                account.get().GetInstrumentDefinitionID().str());
            accountdata.set_owner(
                session.Storage().AccountOwner(accountid)->str());
            accountdata.set_issuer(
                session.Storage().AccountIssuer(accountid)->str());
            accountdata.set_balance(account.get().GetBalance());
            accountdata.set_pendingbalance(account.get().GetBalance());

            if (account.get().IsIssuer()) {
                accountdata.set_type(proto::ACCOUNTTYPE_ISSUER);
            } else {
                accountdata.set_type(proto::ACCOUNTTYPE_NORMAL);
            }

            add_output_status(output, proto::RPCRESPONSE_SUCCESS);
        } else {
            add_output_status(output, proto::RPCRESPONSE_ERROR);
        }
    }

    return output;
}

ArgList RPC::get_args(const Args& serialized)
{
    ArgList output{};

    for (const auto& arg : serialized) {
        auto& row = output[arg.key()];

        for (const auto& value : arg.value()) { row.emplace(value); }
    }

    return output;
}

const api::client::Manager* RPC::get_client(const std::int32_t instance) const
{
    if (is_server_session(instance)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: provided instance ")(
            instance)(" is a server session.")
            .Flush();

        return nullptr;
    } else {
        try {
            return &ot_.Client(get_index(instance));
        } catch (...) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error: provided instance ")(
                instance)(" is not a valid client session.")
                .Flush();

            return nullptr;
        }
    }
}

proto::RPCResponse RPC::get_compatible_accounts(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;
    const auto ownerID = Identifier::Factory(command.owner());
    const auto workflowID = Identifier::Factory(command.identifier(0));
    const auto paymentWorkflow =
        client.Workflow().LoadWorkflow(ownerID, workflowID);

    if (false == bool(paymentWorkflow)) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    auto chequeState = opentxs::api::client::Workflow::InstantiateCheque(
        client, *paymentWorkflow);

    const auto& [state, cheque] = chequeState;

    if (false == bool(cheque)) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto& unitdefinitionid = cheque->GetInstrumentDefinitionID();
    const auto owneraccounts = client.Storage().AccountsByOwner(ownerID);
    const auto unitaccounts =
        client.Storage().AccountsByContract(unitdefinitionid);
    std::vector<OTIdentifier> compatible{};
    std::set_union(
        owneraccounts.begin(),
        owneraccounts.end(),
        unitaccounts.begin(),
        unitaccounts.end(),
        std::back_inserter(compatible));

    for (const auto& accountid : compatible) {
        output.add_identifier(accountid->str());
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

std::size_t RPC::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
}

proto::RPCResponse RPC::get_nyms(const proto::RPCCommand& command) const
{
    auto output = init(command);

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    for (const auto& id : command.identifier()) {
        const auto nymdata =
            session.Wallet().mutable_Nym(Identifier::Factory(id));
        if (false == nymdata.Valid()) {
            add_output_status(output, proto::RPCRESPONSE_ERROR);
        } else {
            const auto credentialindex = nymdata.asPublicNym();
            auto& pcredentialindex = *output.add_nym();
            pcredentialindex = credentialindex;
            add_output_status(output, proto::RPCRESPONSE_SUCCESS);
        }
    }

    return output;
}

proto::RPCResponse RPC::get_pending_payments(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    auto& client = *get_client(command.session());

    const auto ownerID = Identifier::Factory(command.owner());

    const auto& workflow = client.Workflow();
    auto checkWorkflows = workflow.List(
        ownerID,
        proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED);
    auto invoiceWorkflows = workflow.List(
        ownerID,
        proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE,
        proto::PAYMENTWORKFLOWSTATE_CONVEYED);
    std::set<OTIdentifier> workflows;
    std::set_union(
        checkWorkflows.begin(),
        checkWorkflows.end(),
        invoiceWorkflows.begin(),
        invoiceWorkflows.end(),
        std::inserter(workflows, workflows.end()));

    for (auto workflowID : workflows) {

        const auto paymentWorkflow = workflow.LoadWorkflow(ownerID, workflowID);

        if (false == bool(paymentWorkflow)) { continue; }

        auto chequeState = opentxs::api::client::Workflow::InstantiateCheque(
            client, *paymentWorkflow);

        const auto& [state, cheque] = chequeState;

        if (false == bool(cheque)) { continue; }

        auto& accountEvent = *output.add_accountevent();
        accountEvent.set_version(ACCOUNTEVENT_VERSION);

        auto accountEventType = proto::ACCOUNTEVENT_INCOMINGCHEQUE;
        if (proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE ==
            paymentWorkflow->type()) {
            accountEventType = proto::ACCOUNTEVENT_INCOMINGINVOICE;
        }
        accountEvent.set_type(accountEventType);

        const auto contactID =
            client.Contacts().ContactID(cheque->GetSenderNymID());
        accountEvent.set_contact(contactID->str());
        accountEvent.set_workflow(paymentWorkflow->id());
        accountEvent.set_pendingamount(cheque->GetAmount());

        if (0 < paymentWorkflow->event_size()) {
            const auto paymentEvent = paymentWorkflow->event(0);
            accountEvent.set_timestamp(paymentEvent.time());
        }
        accountEvent.set_memo(cheque->GetMemo().Get());
    }

    if (0 == output.accountevent_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::get_seeds(const proto::RPCCommand& command) const
{
    auto output = init(command);

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());
    const auto& hdseeds = session.Seeds();

    for (const auto& id : command.identifier()) {
        auto words = hdseeds.Words(id);
        auto passphrase = hdseeds.Passphrase(id);
        if (false == words.empty() || false == passphrase.empty()) {
            auto& seed = *output.add_seed();
            seed.set_version(SEED_VERSION);
            seed.set_id(id);
            seed.set_words(words);
            seed.set_passphrase(passphrase);

            add_output_status(output, proto::RPCRESPONSE_SUCCESS);
        } else {
            add_output_status(output, proto::RPCRESPONSE_NONE);
        }
    }

    return output;
}

const api::server::Manager* RPC::get_server(const std::int32_t instance) const
{
    if (is_client_session(instance)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: provided instance ")(
            instance)(" is a client session.")
            .Flush();

        return nullptr;
    } else {
        try {
            return &ot_.Server(get_index(instance));
        } catch (...) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error: provided instance ")(
                instance)(" is not a valid server session.")
                .Flush();

            return nullptr;
        }
    }
}

proto::RPCResponse RPC::get_server_admin_nym(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (false == is_server_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);

        return output;
    }

    auto pSession = get_server(command.session());

    if (nullptr == pSession) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);

        return output;
    }

    const auto& session = *pSession;
    const auto password = session.GetAdminNym();

    if (password.empty()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        output.add_identifier(password);
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::get_server_contracts(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    for (const auto& id : command.identifier()) {
        const auto contract = session.Wallet().Server(Identifier::Factory(id));

        if (contract) {
            *output.add_notary() = contract->PublicContract();
            add_output_status(output, proto::RPCRESPONSE_SUCCESS);
        } else {
            add_output_status(output, proto::RPCRESPONSE_NONE);
        }
    }

    return output;
}

proto::RPCResponse RPC::get_server_password(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (false == is_server_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);

        return output;
    }

    auto pSession = get_server(command.session());

    if (nullptr == pSession) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);

        return output;
    }

    const auto& session = *pSession;
    const auto password = session.GetAdminPassword();

    if (password.empty()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        output.add_identifier(password);
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

const api::Core& RPC::get_session(const std::int32_t instance) const
{
    if (is_server_session(instance)) {
        return ot_.Server(get_index(instance));
    } else {
        return ot_.Client(get_index(instance));
    }
}

proto::RPCResponse RPC::get_transaction_data(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);

        return output;
    }

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);

        return output;
    }

    for ([[maybe_unused]] const auto& id : command.identifier()) {
        add_output_status(output, proto::RPCRESPONSE_UNIMPLEMENTED);
    }

    return output;
}

proto::RPCResponse RPC::get_unit_definitions(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    // This won't be necessary when CHECK_EXISTS is uncommented in
    // CHECK_IDENTIFIERS in Check.hpp.
    if (0 == command.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);

        return output;
    }

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);

        return output;
    }

    auto& session = get_session(command.session());

    for (const auto& id : command.identifier()) {
        const auto contract =
            session.Wallet().UnitDefinition(Identifier::Factory(id));

        if (contract) {
            *output.add_unit() = contract->PublicContract();
            add_output_status(output, proto::RPCRESPONSE_SUCCESS);
        } else {
            add_output_status(output, proto::RPCRESPONSE_NONE);
        }
    }

    return output;
}

proto::RPCResponse RPC::get_workflow(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    auto& client = *get_client(command.session());

    for (const auto& getworkflow : command.getworkflow()) {
        const auto workflow = client.Workflow().LoadWorkflow(
            Identifier::Factory(getworkflow.nymid()),
            Identifier::Factory(getworkflow.workflowid()));

        if (workflow) {
            auto& paymentworkflow = *output.add_workflow();
            paymentworkflow = *workflow;
        }
    }

    if (0 == output.workflow_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::import_seed(const proto::RPCCommand& command) const
{
    auto output = init(command);

    auto& seed = command.hdseed();
    OTPassword words;
    words.setPassword(seed.words());
    OTPassword passphrase;
    passphrase.setPassword(seed.passphrase());

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());
    const auto identifier = session.Seeds().ImportSeed(words, passphrase);
    if (identifier.empty()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
    } else {
        output.add_identifier(identifier);
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::import_server_contract(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (0 == command.server_size()) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    auto& session = get_session(command.session());
    for (const auto& servercontract : command.server()) {
        auto server = session.Wallet().Server(servercontract);
        if (false == bool(server)) {
            add_output_status(output, proto::RPCRESPONSE_NONE);
        } else {
            add_output_status(output, proto::RPCRESPONSE_SUCCESS);
        }
    }

    return output;
}

proto::RPCResponse RPC::init(const proto::RPCCommand& command)
{
    proto::RPCResponse output{};
    output.set_version(command.version());
    output.set_cookie(command.cookie());
    output.set_type(command.type());

    return output;
}

proto::RPCResponse RPC::invalid_command(const proto::RPCCommand& command)
{
    auto output = init(command);
    add_output_status(output, proto::RPCRESPONSE_INVALID);

    return output;
}

bool RPC::is_client_session(std::int32_t instance) const
{
    return instance % 2 == 0;
}

bool RPC::is_server_session(std::int32_t instance) const
{
    return instance % 2 != 0;
}

bool RPC::is_session_valid(std::int32_t instance) const
{
    auto index = get_index(instance);

    if (is_server_session(instance)) {
        return ot_.Servers() > index;
    } else {
        return ot_.Clients() > index;
    }
}

proto::RPCResponse RPC::list_accounts(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (is_server_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& list = session.Storage().AccountList();
    for (const auto& account : list) { output.add_identifier(account.first); }

    if (0 == output.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::list_client_sessions(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    for (std::size_t i = 0; i < ot_.Clients(); ++i) {
        proto::SessionData& data = *output.add_sessions();
        data.set_version(SESSION_DATA_VERSION);
        data.set_instance(ot_.Client(i).Instance());
    }

    if (0 == output.sessions_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::list_contacts(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (is_server_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    auto contacts = client.Contacts().ContactList();

    for (const auto& contact : contacts) {
        output.add_identifier(std::get<0>(contact));
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::list_nyms(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& localnyms = session.Wallet().LocalNyms();
    if (0 < localnyms.size()) {
        for (const auto& id : localnyms) { output.add_identifier(id->str()); }
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::list_seeds(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& seeds = session.Storage().SeedList();
    if (0 < seeds.size()) {
        for (const auto& seed : seeds) { output.add_identifier(seed.first); }
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::list_server_contracts(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& servers = session.Wallet().ServerList();
    for (const auto& server : servers) { output.add_identifier(server.first); }

    if (0 == output.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::list_server_sessions(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    for (std::size_t i = 0; i < ot_.Servers(); ++i) {
        auto& data = *output.add_sessions();
        data.set_version(SESSION_DATA_VERSION);
        data.set_instance(ot_.Server(i).Instance());
    }

    if (0 == output.sessions_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::list_unit_definitions(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    auto& session = get_session(command.session());

    const auto& unitdefinitions = session.Wallet().UnitDefinitionList();
    for (const auto& unitdefinition : unitdefinitions) {
        output.add_identifier(unitdefinition.first);
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::lookup_account_id(
    const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (false == is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);

        return output;
    }

    const auto& session = get_session(command.session());
    const auto& label = command.param();

    for (const auto& [id, alias] : session.Storage().AccountList()) {
        if (alias == label) { output.add_identifier(id); }
    }

    if (0 == output.identifier_size()) {
        add_output_status(output, proto::RPCRESPONSE_NONE);
    } else {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }

    return output;
}

proto::RPCResponse RPC::move_funds(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    const auto& movefunds = command.movefunds();

    const auto sourceaccount = Identifier::Factory(movefunds.sourceaccount());

    auto sender = client.Storage().AccountOwner(sourceaccount);

    switch (movefunds.type()) {
        case proto::RPCPAYMENTTYPE_TRANSFER: {
            const auto targetaccount =
                Identifier::Factory(movefunds.destinationaccount());

            const auto notary = client.Storage().AccountServer(sourceaccount);

            auto action = client.ServerAction().SendTransfer(
                sender,
                notary,
                sourceaccount,
                targetaccount,
                movefunds.amount(),
                movefunds.memo());
            action->Run();
            if (SendResult::VALID_REPLY == action->LastSendResult()) {
                auto reply = action->Reply();

                if (false == bool(reply) || false == reply->m_bSuccess) {
                    add_output_status(output, proto::RPCRESPONSE_ERROR);
                    return output;
                }
            } else {
                add_output_status(output, proto::RPCRESPONSE_ERROR);
                return output;
            }
            break;
        }
        case proto::RPCPAYMENTTYPE_CHEQUE:
            [[fallthrough]];
        case proto::RPCPAYMENTTYPE_VOUCHER:
            [[fallthrough]];
        case proto::RPCPAYMENTTYPE_INVOICE:
            [[fallthrough]];
        case proto::RPCPAYMENTTYPE_BLINDED:
            [[fallthrough]];
        default: {
            add_output_status(output, proto::RPCRESPONSE_INVALID);
            return output;
        }
    }

    add_output_status(output, proto::RPCRESPONSE_SUCCESS);

    return output;
}

proto::RPCResponse RPC::Process(const proto::RPCCommand& command) const
{
    const auto valid = proto::Validate(command, VERBOSE);

    if (false == valid) { return invalid_command(command); }

    switch (command.type()) {
        case proto::RPCCOMMAND_ADDCLIENTSESSION: {
            return start_client(command);
        } break;
        case proto::RPCCOMMAND_ADDSERVERSESSION: {
            return start_server(command);
        } break;
        case proto::RPCCOMMAND_LISTCLIENTSESSIONS: {
            return list_client_sessions(command);
        } break;
        case proto::RPCCOMMAND_LISTSERVERSESSIONS: {
            return list_server_sessions(command);
        } break;
        case proto::RPCCOMMAND_IMPORTHDSEED: {
            return import_seed(command);
        } break;
        case proto::RPCCOMMAND_LISTHDSEEDS: {
            return list_seeds(command);
        } break;
        case proto::RPCCOMMAND_GETHDSEED: {
            return get_seeds(command);
        } break;
        case proto::RPCCOMMAND_CREATENYM: {
            return create_nym(command);
        } break;
        case proto::RPCCOMMAND_LISTNYMS: {
            return list_nyms(command);
        } break;
        case proto::RPCCOMMAND_GETNYM: {
            return get_nyms(command);
        } break;
        case proto::RPCCOMMAND_ADDCLAIM: {
            return add_claim(command);
        } break;
        case proto::RPCCOMMAND_DELETECLAIM: {
            return delete_claim(command);
        } break;
        case proto::RPCCOMMAND_IMPORTSERVERCONTRACT: {
            return import_server_contract(command);
        } break;
        case proto::RPCCOMMAND_LISTSERVERCONTRACTS: {
            return list_server_contracts(command);
        } break;
        case proto::RPCCOMMAND_REGISTERNYM: {
            return register_nym(command);
        } break;
        case proto::RPCCOMMAND_CREATEUNITDEFINITION: {
            return create_unit_definition(command);
        } break;
        case proto::RPCCOMMAND_LISTUNITDEFINITIONS: {
            return list_unit_definitions(command);
        } break;
        case proto::RPCCOMMAND_ISSUEUNITDEFINITION: {
            return create_issuer_account(command);
        } break;
        case proto::RPCCOMMAND_CREATEACCOUNT: {
            return create_account(command);
        } break;
        case proto::RPCCOMMAND_LISTACCOUNTS: {
            return list_accounts(command);
        } break;
        case proto::RPCCOMMAND_GETACCOUNTBALANCE: {
            return get_account_balance(command);
        } break;
        case proto::RPCCOMMAND_GETACCOUNTACTIVITY: {
            return get_account_activity(command);
        } break;
        case proto::RPCCOMMAND_SENDPAYMENT: {
            return send_payment(command);
        } break;
        case proto::RPCCOMMAND_MOVEFUNDS: {
            return move_funds(command);
        } break;
        case proto::RPCCOMMAND_GETSERVERCONTRACT: {
            return get_server_contracts(command);
        } break;
        case proto::RPCCOMMAND_ADDCONTACT: {
            return add_contact(command);
        } break;
        case proto::RPCCOMMAND_LISTCONTACTS: {
            return list_contacts(command);
        } break;
        case proto::RPCCOMMAND_GETCONTACT:
        case proto::RPCCOMMAND_ADDCONTACTCLAIM:
        case proto::RPCCOMMAND_DELETECONTACTCLAIM:
        case proto::RPCCOMMAND_VERIFYCLAIM:
        case proto::RPCCOMMAND_ACCEPTVERIFICATION:
        case proto::RPCCOMMAND_SENDCONTACTMESSAGE:
        case proto::RPCCOMMAND_GETCONTACTACTIVITY: {
            otErr << OT_METHOD << __FUNCTION__ << ": Command not implemented"
                  << std::endl;
        } break;
        case proto::RPCCOMMAND_ACCEPTPENDINGPAYMENTS: {
            return accept_pending_payments(command);
        } break;
        case proto::RPCCOMMAND_GETPENDINGPAYMENTS: {
            return get_pending_payments(command);
        } break;
        case proto::RPCCOMMAND_GETCOMPATIBLEACCOUNTS: {
            return get_compatible_accounts(command);
        } break;
        case proto::RPCCOMMAND_CREATECOMPATIBLEACCOUNT: {
            return create_compatible_account(command);
        } break;
        case proto::RPCCOMMAND_GETWORKFLOW: {
            return get_workflow(command);
        } break;
        case proto::RPCCOMMAND_GETSERVERPASSWORD: {
            return get_server_password(command);
        } break;
        case proto::RPCCOMMAND_GETADMINNYM: {
            return get_server_admin_nym(command);
        } break;
        case proto::RPCCOMMAND_GETUNITDEFINITION: {
            return get_unit_definitions(command);
        } break;
        case proto::RPCCOMMAND_GETTRANSACTIONDATA: {
            return get_transaction_data(command);
        } break;
        case proto::RPCCOMMAND_LOOKUPACCOUNTID: {
            return lookup_account_id(command);
        } break;
        case proto::RPCCOMMAND_ERROR:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported command.")
                .Flush();
        }
    }

    return invalid_command(command);
}

proto::RPCResponse RPC::register_nym(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    if (false == client.Wallet().IsLocalNym(command.owner())) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto ownerid = Identifier::Factory(command.owner()),
               notaryid = Identifier::Factory(command.notary());

    auto registered =
        client.OTAPI().IsNym_RegisteredAtServer(ownerid, notaryid);

    if (false == registered) {
        auto servercontract = client.Wallet().Server(notaryid);

        if (false != bool(servercontract)) {
            auto nym = client.Wallet().mutable_Nym(ownerid);
            const auto server = nym.PreferredOTServer();

            if (server.empty()) {
                nym.AddPreferredOTServer(command.notary(), true);
            }

            auto action = client.ServerAction().RegisterNym(ownerid, notaryid);
            action->Run();

            if (SendResult::VALID_REPLY == action->LastSendResult()) {
                auto reply = action->Reply();

                if (false == bool(reply) || false == reply->m_bSuccess) {
                    add_output_status(output, proto::RPCRESPONSE_ERROR);
                } else {
                    add_output_status(output, proto::RPCRESPONSE_SUCCESS);
                }
            } else {
                add_output_status(output, proto::RPCRESPONSE_ERROR);
            }
        } else {
            const auto taskid =
                client.Sync().ScheduleRegisterNym(ownerid, notaryid);
            if (false == taskid->empty()) {
                add_output_task(output, taskid->str());
                add_output_status(output, proto::RPCRESPONSE_QUEUED);
            } else {
                add_output_task(output, "");
                add_output_status(output, proto::RPCRESPONSE_ERROR);
            }
        }
    } else {
        add_output_status(output, proto::RPCRESPONSE_UNNECESSARY);
    }

    return output;
}

proto::RPCResponse RPC::send_payment(const proto::RPCCommand& command) const
{
    auto output = init(command);

    if (!is_session_valid(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SESSION);
        return output;
    }

    if (!is_client_session(command.session())) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    const auto pClient = get_client(command.session());

    OT_ASSERT(nullptr != pClient)

    const auto& client = *pClient;

    const auto& sendpayment = command.sendpayment();
    const auto contactid = Identifier::Factory(sendpayment.contact()),
               sourceaccountid =
                   Identifier::Factory(sendpayment.sourceaccount());
    auto& contacts = client.Contacts();
    const auto contact = contacts.Contact(contactid);

    if (false == bool(contact)) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    }

    const auto sender = client.Storage().AccountOwner(sourceaccountid);

    if (sender->empty()) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);

        return output;
    }

    const auto ready = client.Sync().CanMessage(sender, contactid);

    switch (ready) {
        case Messagability::MISSING_CONTACT:
        case Messagability::CONTACT_LACKS_NYM:
        case Messagability::NO_SERVER_CLAIM:
        case Messagability::INVALID_SENDER:
        case Messagability::MISSING_SENDER: {
            add_output_status(output, proto::RPCRESPONSE_NO_PATH_TO_RECIPIENT);
            return output;
        } break;
        case Messagability::MISSING_RECIPIENT:
        case Messagability::UNREGISTERED: {
            add_output_status(output, proto::RPCRESPONSE_RETRY);
            return output;
        }
        case Messagability::READY:
        default: {
        }
    }

    switch (sendpayment.type()) {
        case proto::RPCPAYMENTTYPE_CHEQUE: {
            const auto taskid = client.Sync().ScheduleSendCheque(
                sender,
                sourceaccountid,
                contactid,
                sendpayment.amount(),
                sendpayment.memo(),
                std::chrono::system_clock::now(),
                std::chrono::system_clock::now() +
                    std::chrono::hours(OT_CHEQUE_HOURS));
            add_output_task(output, taskid->str());
            add_output_status(output, proto::RPCRESPONSE_QUEUED);
        } break;
        case proto::RPCPAYMENTTYPE_TRANSFER: {
            const auto targetaccount =
                Identifier::Factory(sendpayment.destinationaccount());

            auto notary = client.Storage().AccountServer(sourceaccountid);

            auto action = client.ServerAction().SendTransfer(
                sender,
                notary,
                sourceaccountid,
                targetaccount,
                sendpayment.amount(),
                sendpayment.memo());
            action->Run();
            if (SendResult::VALID_REPLY == action->LastSendResult()) {
                auto reply = action->Reply();

                if (false == bool(reply) || false == reply->m_bSuccess) {
                    add_output_status(output, proto::RPCRESPONSE_ERROR);
                } else {
                    std::string strReply = String::Factory(*reply)->Get();
                    auto transuccess =
                        client.Exec().Message_GetTransactionSuccess(
                            notary->str(),
                            sender->str(),
                            sourceaccountid->str(),
                            strReply);
                    if (1 == transuccess) {
                        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
                    } else {
                        add_output_status(output, proto::RPCRESPONSE_ERROR);
                    }
                }
            } else {
                add_output_status(output, proto::RPCRESPONSE_ERROR);
            }
        } break;
        case proto::RPCPAYMENTTYPE_VOUCHER:
            // TODO
        case proto::RPCPAYMENTTYPE_INVOICE:
            // TODO
        case proto::RPCPAYMENTTYPE_BLINDED:
            // TODO
        default: {
            add_output_status(output, proto::RPCRESPONSE_INVALID);
            return output;
        }
    }

    return output;
}

proto::RPCResponse RPC::start_client(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    const auto session{static_cast<std::uint32_t>(ot_.Clients())};

    std::uint32_t instance{0};

    try {
        auto& manager = ot_.StartClient(get_args(command.arg()), session);
        instance = manager.Instance();
    } catch (...) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    output.set_session(instance);
    add_output_status(output, proto::RPCRESPONSE_SUCCESS);

    return output;
}

proto::RPCResponse RPC::start_server(const proto::RPCCommand& command) const
{
    auto output = init(command);
    Lock lock(lock_);
    const auto session{static_cast<std::uint32_t>(ot_.Servers())};

    std::uint32_t instance{0};

    try {
        auto& manager = ot_.StartServer(get_args(command.arg()), session);
        instance = manager.Instance();
    } catch (const std::invalid_argument& e) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
        return output;
    } catch (...) {
        add_output_status(output, proto::RPCRESPONSE_INVALID);
        return output;
    }

    output.set_session(instance);
    add_output_status(output, proto::RPCRESPONSE_SUCCESS);

    return output;
}

proto::AccountEventType RPC::storagebox_to_accounteventtype(
    StorageBox storagebox)
{
    proto::AccountEventType accounteventtype = proto::ACCOUNTEVENT_ERROR;

    switch (storagebox) {
        case StorageBox::INCOMINGCHEQUE: {
            accounteventtype = proto::ACCOUNTEVENT_INCOMINGCHEQUE;
        } break;
        case StorageBox::OUTGOINGCHEQUE: {
            accounteventtype = proto::ACCOUNTEVENT_OUTGOINGCHEQUE;
        } break;
        case StorageBox::INCOMINGTRANSFER: {
            accounteventtype = proto::ACCOUNTEVENT_INCOMINGTRANSFER;
        } break;
        case StorageBox::OUTGOINGTRANSFER: {
            accounteventtype = proto::ACCOUNTEVENT_OUTGOINGTRANSFER;
        } break;
        default: {
        }
    }

    return accounteventtype;
}

}  // namespace opentxs::rpc::implementation
