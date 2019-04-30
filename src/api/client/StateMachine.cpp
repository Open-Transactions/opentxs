// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include "internal/api/client/Client.hpp"

#include "StateMachine.tpp"

#define CONTRACT_DOWNLOAD_MILLISECONDS 10000
#define NYM_REGISTRATION_MILLISECONDS 10000
#define STATE_MACHINE_READY_MILLISECONDS 100

#define DO_OPERATION(a, ...)                                                   \
    auto started = op_.a(__VA_ARGS__);                                         \
                                                                               \
    while (false == started) {                                                 \
        LogDebug(OT_METHOD)(__FUNCTION__)(": State machine is not ready")      \
            .Flush();                                                          \
        Log::Sleep(                                                            \
            std::chrono::milliseconds(STATE_MACHINE_READY_MILLISECONDS));      \
                                                                               \
        if (shutdown().load()) {                                               \
            op_.Shutdown();                                                    \
                                                                               \
            return false;                                                      \
        } else {                                                               \
            started = op_.a(__VA_ARGS__);                                      \
        }                                                                      \
    }                                                                          \
                                                                               \
    if (shutdown().load()) {                                                   \
        op_.Shutdown();                                                        \
                                                                               \
        return false;                                                          \
    }                                                                          \
                                                                               \
    Result result = op_.GetFuture().get();                                     \
    const auto success =                                                       \
        proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result);

#define DO_OPERATION_TASK_DONE(a, ...)                                         \
    auto started = op_.a(__VA_ARGS__);                                         \
                                                                               \
    while (false == started) {                                                 \
        LogDebug(OT_METHOD)(__FUNCTION__)(": State machine is not ready")      \
            .Flush();                                                          \
        Log::Sleep(                                                            \
            std::chrono::milliseconds(STATE_MACHINE_READY_MILLISECONDS));      \
                                                                               \
        if (shutdown().load()) {                                               \
            op_.Shutdown();                                                    \
                                                                               \
            return task_done(false);                                           \
        } else {                                                               \
            started = op_.a(__VA_ARGS__);                                      \
        }                                                                      \
    }                                                                          \
                                                                               \
    if (shutdown().load()) {                                                   \
        op_.Shutdown();                                                        \
        return task_done(false);                                               \
    }                                                                          \
                                                                               \
    Result result = op_.GetFuture().get();                                     \
    const auto success =                                                       \
        proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result);

#define SHUTDOWN()                                                             \
    {                                                                          \
        YIELD(50);                                                             \
    }

#define YIELD(a)                                                               \
    {                                                                          \
        if (shutdown().load()) { return false; }                               \
                                                                               \
        Log::Sleep(std::chrono::milliseconds(a));                              \
    }

#define OT_METHOD "opentxs::api::client::implementation::StateMachine::"

namespace opentxs::api::client::implementation
{
StateMachine::StateMachine(
    const api::client::Manager& client,
    const api::client::internal::OTX& parent,
    const Flag& running,
    const api::client::Manager& api,
    const ContextID& id,
    std::atomic<TaskID>& nextTaskID,
    const UniqueQueue<CheckNymTask>& missingnyms,
    const UniqueQueue<CheckNymTask>& outdatednyms,
    const UniqueQueue<OTServerID>& missingservers,
    const UniqueQueue<OTUnitID>& missingUnitDefinitions)
    : opentxs::internal::StateMachine(
          std::bind(&implementation::StateMachine::state_machine, this))
    , client_(client)
    , parent_(parent)
    , next_task_id_(nextTaskID)
    , missing_nyms_(missingnyms)
    , outdated_nyms_(outdatednyms)
    , missing_servers_(missingservers)
    , missing_unit_definitions_(missingUnitDefinitions)
    , pOp_(opentxs::Factory::Operation(api, id.first, id.second))
    , op_(*pOp_)

    , check_nym_()
    , deposit_payment_()
    , download_contract_()
#if OT_CASH
    , download_mint_()
#endif  // OT_CASH
    , download_nymbox_()
    , download_unit_definition_()
    , get_transaction_numbers_()
    , issue_unit_definition_()
    , send_message_()
#if OT_CASH
    , send_cash_()
#endif  // OT_CASH
    , send_payment_()
    , peer_reply_()
    , peer_request_()
    , process_inbox_()
    , publish_server_contract_()
    , register_account_()
    , register_nym_()
    , send_cheque_()
    , send_transfer_()
#if OT_CASH
    , withdraw_cash_()
#endif  // OT_CASH
    , param_()
    , task_id_()
    , counter_(0)
    , task_count_(0)
    , lock_()
    , tasks_()
    , state_(State::needServerContract)
    , unknown_nyms_()
    , unknown_servers_()
    , unknown_units_()
{
    OT_ASSERT(pOp_);
}

std::future<void> StateMachine::add_task(const Lock& lock)
{
    OT_ASSERT(lock.owns_lock());

    tasks_.emplace_back(counter_.load() + 2, std::promise<void>{});
    auto& task = *tasks_.rbegin();
    auto& promise = task.second;

    return promise.get_future();
}

bool StateMachine::bump_task(const bool bump) const
{
    if (bump) {
        LogInsane(OT_METHOD)(__FUNCTION__)(": ")(++task_count_).Flush();
    }

    return bump;
}

// If this server was added by a pairing operation that included a server
// password then request admin permissions on the server
bool StateMachine::check_admin(const ServerContext& context) const
{
    bool needAdmin{false};
    const auto haveAdmin = context.isAdmin();
    needAdmin = context.HaveAdminPassword() && (false == haveAdmin);

    if (needAdmin) {
        OTPassword serverPassword;
        serverPassword.setPassword(context.AdminPassword());
        get_admin(next_task_id(), serverPassword);
    }

    SHUTDOWN()

    if (haveAdmin) { check_server_name(context); }

    SHUTDOWN()

    return true;
}

template <typename T, typename C, typename M, typename U>
bool StateMachine::check_missing_contract(M& missing, U& unknown, bool skip)
    const
{
    auto items = missing.Copy();

    for (const auto& [targetID, taskID] : items) {
        SHUTDOWN()

        find_contract<T, C>(taskID, targetID, missing, unknown, skip);
    }

    return true;
}

// Queue registerNym if the local nym has updated since the last registernym
// operation
void StateMachine::check_nym_revision(const ServerContext& context) const
{
    if (context.StaleNym()) {
        const auto& nymID = context.Nym()->ID();
        LogDetail(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " has is newer than version last registered version on server ")(
            context.Server())(".")
            .Flush();
        bump_task(get_task<RegisterNymTask>().Push(next_task_id(), true));
    }
}

bool StateMachine::check_registration(
    const identifier::Nym& nymID,
    const identifier::Server& serverID) const
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    auto context = client_.Wallet().ServerContext(nymID, serverID);
    RequestNumber request{0};

    if (context) {
        request = context->Request();
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " has never registered on ")(serverID)
            .Flush();
    }

    if (0 != request) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " has registered on server ")(serverID)(" at least once.")
            .Flush();
        state_ = State::ready;

        OT_ASSERT(context)

        return false;
    }

    const auto output = register_nym(next_task_id(), false);

    if (output) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
            " is now registered on server ")(serverID)
            .Flush();
        state_ = State::ready;
        context = client_.Wallet().ServerContext(nymID, serverID);

        OT_ASSERT(context)

        return false;
    } else {
        YIELD(NYM_REGISTRATION_MILLISECONDS);

        return true;
    }
}

bool StateMachine::check_server_contract(
    const identifier::Server& serverID) const
{
    OT_ASSERT(false == serverID.empty())

    const auto serverContract = client_.Wallet().Server(serverID);

    if (serverContract) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Server contract ")(serverID)(
            " exists.")
            .Flush();
        state_ = State::needRegistration;

        return false;
    }

    LogDetail(OT_METHOD)(__FUNCTION__)(": Server contract for ")(serverID)(
        " is not in the wallet.")
        .Flush();
    missing_servers_.Push(next_task_id(), serverID);

    YIELD(CONTRACT_DOWNLOAD_MILLISECONDS);

    return true;
}

bool StateMachine::check_server_name(const ServerContext& context) const
{
    const auto server = client_.Wallet().Server(op_.ServerID());

    OT_ASSERT(server)

    const auto myName = server->Alias();
    const auto hisName = server->EffectiveName();

    if (myName == hisName) { return true; }

    DO_OPERATION(
        AddClaim,
        proto::CONTACTSECTION_SCOPE,
        proto::CITEMTYPE_SERVER,
        String::Factory(myName),
        true);

    if (success) {
        bump_task(get_task<CheckNymTask>().Push(
            next_task_id(), context.RemoteNym().ID()));
    }

    return success;
}

// Periodically download server nym in case it has been renamed
void StateMachine::check_server_nym(const ServerContext& context) const
{
    if (0 == counter() % 100) {
        // download server nym in case it has been renamed
        bump_task(get_task<CheckNymTask>().Push(
            next_task_id(), context.RemoteNym().ID()));
    }
}

// Queue getTransactionNumbers if necessary
void StateMachine::check_transaction_numbers(const ServerContext& context) const
{
    if (0 == context.Accounts().size()) { return; }

    if (0 < context.AvailableNumbers()) { return; }

    bump_task(get_task<GetTransactionNumbersTask>().Push(next_task_id(), {}));
}

bool StateMachine::deposit_cheque(
    const TaskID taskID,
    const DepositPaymentTask& task) const
{
    const auto& [accountID, payment] = task;

    OT_ASSERT(false == accountID->empty());
    OT_ASSERT(payment);

    if ((false == payment->IsCheque()) && (false == payment->IsVoucher())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled payment type.").Flush();

        return finish_task(taskID, false, error_result());
    }

    std::shared_ptr<Cheque> cheque{client_.Factory().Cheque()};

    OT_ASSERT(cheque);

    const auto loaded = cheque->LoadContractFromString(payment->Payment());

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid cheque.").Flush();

        return finish_task(taskID, false, error_result());
    }

    DO_OPERATION(DepositCheque, accountID, cheque);

    if (success) { return finish_task(taskID, success, std::move(result)); }

    return false;
}

bool StateMachine::deposit_cheque_wrapper(
    const TaskID task,
    const DepositPaymentTask& param,
    UniqueQueue<DepositPaymentTask>& retry) const
{
    bool output{false};
    const auto& [accountIDHint, payment] = param;

    OT_ASSERT(payment);

    auto depositServer = identifier::Server::Factory();
    auto depositAccount = Identifier::Factory();

    const auto status = can_deposit(
        *payment, op_.NymID(), accountIDHint, depositServer, depositAccount);

    switch (status) {
        case Depositability::READY: {
            auto revised{param};
            revised.first = depositAccount;
            output = deposit_cheque(task, revised);

            if (false == output) {
                retry.Push(task, revised);
                bump_task(
                    get_task<RegisterNymTask>().Push(next_task_id(), false));
            }
        } break;
        case Depositability::NOT_REGISTERED:
        case Depositability::NO_ACCOUNT: {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Temporary failure trying to deposit payment")
                .Flush();
            retry.Push(task, param);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Permanent failure trying to deposit payment.")
                .Flush();
        }
    }

    return output;
}

#if OT_CASH
bool StateMachine::download_mint(
    const TaskID taskID,
    const DownloadMintTask& task) const
{
    DO_OPERATION(
        Start, internal::Operation::Type::DownloadMint, task.first, {});

    return finish_task(taskID, success, std::move(result));
}
#endif

bool StateMachine::download_nym(const TaskID taskID, const CheckNymTask& id)
    const
{
    OT_ASSERT(false == id->empty())

    ServerContext::ExtraArgs args{};

    DO_OPERATION(Start, internal::Operation::Type::CheckNym, id, args);

    resolve_unknown(id, success, unknown_nyms_);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::download_nymbox(const TaskID taskID) const
{
    op_.join();
    auto contextE =
        client_.Wallet().mutable_ServerContext(op_.NymID(), op_.ServerID());
    auto& context = contextE.It();
    context.Join();
    context.ResetThread();
    auto future = context.RefreshNymbox(client_);

    if (false == bool(future)) {

        return finish_task(taskID, false, error_result());
    }

    Result result{future->get()};
    const auto success =
        proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result);

    return finish_task(taskID, success, std::move(result));
}

StateMachine::Result StateMachine::error_result()
{
    Result output{proto::LASTREPLYSTATUS_NOTSENT, nullptr};

    return output;
}

bool StateMachine::download_server(
    const TaskID taskID,
    const DownloadContractTask& contractID) const
{
    OT_ASSERT(false == contractID->empty())

    DO_OPERATION(DownloadContract, contractID);

    const bool found = success && std::get<1>(result)->m_bBool;

    resolve_unknown(contractID, found, unknown_servers_);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::download_unit_definition(
    const TaskID taskID,
    const DownloadUnitDefinitionTask& id) const
{
    OT_ASSERT(false == id->empty())

    DO_OPERATION(DownloadContract, id);

    const bool found = success && std::get<1>(result)->m_bBool;

    resolve_unknown(id, found, unknown_units_);

    return finish_task(taskID, success, std::move(result));
}

template <typename T, typename C, typename I, typename M, typename U>
bool StateMachine::find_contract(
    const TaskID taskID,
    const I& targetID,
    M& missing,
    U& unknown,
    const bool skipExisting) const
{
    auto contract = load_contract<T, C>(targetID.get());

    if (contract) {
        if (skipExisting) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Contract ")(targetID)(
                " exists in the wallet.")
                .Flush();
            missing.CancelByValue(targetID);

            return finish_task(taskID, true, error_result());
        } else {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Attempting re-download of contract ")(targetID)
                .Flush();
        }
    }

    if (0 == unknown.count(targetID)) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Queueing contract ")(targetID)(
            " for download on server ")(op_.ServerID())
            .Flush();

        return bump_task(get_task<T>().Push(taskID, targetID));
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Previously failed to download contract ")(targetID)(
            " from server ")(op_.ServerID())
            .Flush();

        finish_task(taskID, false, error_result());
    }

    return false;
}

bool StateMachine::get_admin(const TaskID taskID, const OTPassword& password)
    const
{
    DO_OPERATION(RequestAdmin, String::Factory(password.getPassword()));

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::get_transaction_numbers(const TaskID taskID) const
{
    ServerContext::ExtraArgs args{};

    DO_OPERATION(Start, internal::Operation::Type::GetTransactionNumbers, args);

    return finish_task(taskID, success, std::move(result));
}

void StateMachine::increment_counter(const bool run)
{
    ++counter_;
    Lock lock(lock_);

    for (auto i = tasks_.begin(); i < tasks_.end();) {
        // auto& [limit, future] = *i;
        auto& limit = std::get<0>(*i);
        auto& future = std::get<1>(*i);
        const bool erase = (false == run) || (counter_ >= limit);

        if (erase) {
            future.set_value();
            i = tasks_.erase(i);
        } else {
            ++i;
        }
    }
}

bool StateMachine::initiate_peer_reply(
    const TaskID taskID,
    const PeerReplyTask& task) const
{
    const auto [targetNymID, peerReply, peerRequest] = task;

    DO_OPERATION(SendPeerReply, targetNymID, peerReply, peerRequest);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::initiate_peer_request(
    const TaskID taskID,
    const PeerRequestTask& task) const
{
    const auto& [targetNymID, peerRequest] = task;

    DO_OPERATION(SendPeerRequest, targetNymID, peerRequest);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::issue_unit_definition(
    const TaskID taskID,
    const IssueUnitDefinitionTask& task) const
{
    const auto& [unitID, label] = task;
    auto unitDefinition = client_.Wallet().UnitDefinition(unitID);

    if (false == bool(unitDefinition)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unit definition not found.")
            .Flush();

        return false;
    }

    auto serialized = std::make_shared<proto::UnitDefinition>();

    OT_ASSERT(serialized);

    *serialized = unitDefinition->PublicContract();
    ServerContext::ExtraArgs args{label, false};

    DO_OPERATION(IssueUnitDefinition, serialized, args);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::issue_unit_definition_wrapper(
    const TaskID task,
    const IssueUnitDefinitionTask& param) const
{
    const auto output = issue_unit_definition(task, param);

    if (false == output) {
        bump_task(get_task<RegisterNymTask>().Push(next_task_id(), false));
    }

    return output;
}

bool StateMachine::main_loop() noexcept
{
    int next{0};
    const auto tasks = task_count_.load();
    const auto& nymID = op_.NymID();
    const auto& serverID = op_.ServerID();
    UniqueQueue<DepositPaymentTask> retryDepositPayment{};
    UniqueQueue<RegisterNymTask> retryRegisterNym{};
    UniqueQueue<SendChequeTask> retrySendCheque{};
    auto pContext = client_.Wallet().ServerContext(nymID, serverID);

    OT_ASSERT(pContext)

    const auto& context = *pContext;

    // Register nym,
    check_nym_revision(context);
    run_task<RegisterNymTask>(
        &StateMachine::register_nym_wrapper, retryRegisterNym);

    // Pairing
    check_admin(context);
    run_task<PublishServerContractTask>(&StateMachine::publish_server_contract);

    // Download contracts
    queue_contracts(context, next);
    run_task<CheckNymTask>(&StateMachine::download_nym);
    run_task<DownloadContractTask>(&StateMachine::download_server);
    run_task<DownloadUnitDefinitionTask>(
        &StateMachine::download_unit_definition);
#if OT_CASH
    run_task<DownloadMintTask>(&StateMachine::download_mint);
#endif

    // Messaging
    run_task<DownloadNymboxTask>(&StateMachine::download_nymbox);
    run_task<MessageTask>(&StateMachine::message_nym);
    run_task<PeerReplyTask>(&StateMachine::initiate_peer_reply);
    run_task<PeerRequestTask>(&StateMachine::initiate_peer_request);

    // Transactions
    check_transaction_numbers(context);
    run_task<GetTransactionNumbersTask>(&StateMachine::get_transaction_numbers);
    run_task<SendChequeTask>(
        &StateMachine::write_and_send_cheque_wrapper, retrySendCheque);
    run_task<PaymentTask>(&StateMachine::pay_nym);
    run_task<DepositPaymentTask>(
        &StateMachine::deposit_cheque_wrapper, retryDepositPayment);
    run_task<SendTransferTask>(&StateMachine::send_transfer);
#if OT_CASH
    run_task<WithdrawCashTask>(&StateMachine::withdraw_cash);
    run_task<PayCashTask>(&StateMachine::pay_nym_cash);
#endif

    // Account maintenance
    run_task<RegisterAccountTask>(&StateMachine::register_account_wrapper);
    run_task<IssueUnitDefinitionTask>(
        &StateMachine::issue_unit_definition_wrapper);
    run_task<ProcessInboxTask>(&StateMachine::process_inbox);
    check_transaction_numbers(context);

    Lock lock(decision_lock_);
    const bool run = 0 < (task_count_.load() + tasks + next);
    increment_counter(run);

    return run;
}

bool StateMachine::message_nym(const TaskID taskID, const MessageTask& task)
    const
{
    const auto& [recipient, text, setID] = task;
    auto messageID = Identifier::Factory();
    auto updateID = [&](const Identifier& in) -> void {
        messageID = in;
        auto& pID = std::get<2>(task);

        if (pID) {
            auto& id = *pID;

            if (id) { id(in); }
        }
    };

    OT_ASSERT(false == recipient->empty())

    DO_OPERATION(SendMessage, recipient, String::Factory(text), updateID);

    if (success) {
        if (false == messageID->empty()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Sent message: ")(messageID)
                .Flush();
            associate_message_id(messageID, taskID);
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message ID").Flush();
        }
    }

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::pay_nym(const TaskID taskID, const PaymentTask& task) const
{
    const auto& [recipient, payment] = task;

    OT_ASSERT(false == recipient->empty())

    DO_OPERATION(ConveyPayment, recipient, payment);

    return finish_task(taskID, success, std::move(result));
}

#if OT_CASH
bool StateMachine::pay_nym_cash(const TaskID taskID, const PayCashTask& task)
    const
{
    const auto& [recipient, workflowID] = task;

    OT_ASSERT(false == recipient->empty())

    DO_OPERATION(SendCash, recipient, workflowID);

    return finish_task(taskID, success, std::move(result));
}
#endif  // OT_CASH

bool StateMachine::process_inbox(
    const TaskID taskID,
    const ProcessInboxTask& id) const
{
    OT_ASSERT(false == id->empty())

    DO_OPERATION(UpdateAccount, id);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::publish_server_contract(
    const TaskID taskID,
    const PublishServerContractTask& task) const
{
    const auto& id = task.first;

    OT_ASSERT(false == id->empty())

    DO_OPERATION(PublishContract, id);

    return finish_task(taskID, success, std::move(result));
}

bool StateMachine::queue_contracts(const ServerContext& context, int& next)
{
    check_server_nym(context);
    check_missing_contract<CheckNymTask, identity::Nym>(
        missing_nyms_, unknown_nyms_);
    check_missing_contract<CheckNymTask, identity::Nym>(
        outdated_nyms_, unknown_nyms_, false);
    check_missing_contract<DownloadContractTask, opentxs::ServerContract>(
        missing_servers_, unknown_servers_);
    check_missing_contract<DownloadUnitDefinitionTask, opentxs::UnitDefinition>(
        missing_unit_definitions_, unknown_units_);
    queue_nyms();

    scan_unknown<CheckNymTask>(unknown_nyms_, next);
    scan_unknown<DownloadContractTask>(unknown_servers_, next);
    scan_unknown<DownloadUnitDefinitionTask>(unknown_units_, next);

    return true;
}

bool StateMachine::queue_nyms()
{
    const auto blank = client_.Factory().ServerID();
    auto nymID = client_.Factory().NymID();

    while (get_nym_fetch(op_.ServerID()).Pop(task_id_, nymID)) {
        SHUTDOWN();

        if (0 == unknown_nyms_.count(nymID)) {
            bump_task(get_task<CheckNymTask>().Push(task_id_, nymID));
        }
    }

    while (get_nym_fetch(blank).Pop(task_id_, nymID)) {
        SHUTDOWN();

        if (0 == unknown_nyms_.count(nymID)) {
            bump_task(get_task<CheckNymTask>().Push(task_id_, nymID));
        }
    }

    return true;
}

bool StateMachine::register_account(
    const TaskID taskID,
    const RegisterAccountTask& task) const
{
    const auto& [label, unitID] = task;

    OT_ASSERT(false == unitID->empty())

    auto contract = client_.Wallet().UnitDefinition(unitID);

    if (false == bool(contract)) {
        DO_OPERATION(DownloadContract, unitID, ContractType::UNIT);

        if (false == success) {
            return finish_task(taskID, success, std::move(result));
        }
    }

    ServerContext::ExtraArgs args{label, false};

    DO_OPERATION(
        Start,
        internal::Operation::Type::RegisterAccount,
        unitID,
        {label, false});

    finish_task(taskID, success, std::move(result));

    if (success) { client_.Pair().Update(); }

    return success;
}

bool StateMachine::register_account_wrapper(
    const TaskID task,
    const RegisterAccountTask& param) const
{
    const auto done = register_account(task, param);

    if (false == done) {
        bump_task(get_task<RegisterNymTask>().Push(next_task_id(), false));
    }

    return done;
}

bool StateMachine::register_nym(
    const TaskID taskID,
    const RegisterNymTask& resync) const
{
    ServerContext::ExtraArgs args{};

    if (resync) { std::get<1>(args) = true; }

    DO_OPERATION(Start, internal::Operation::Type::RegisterNym, args);

    return finish_task(taskID, success, std::move(result));
}

// Register the nym, if scheduled. Keep trying until success
bool StateMachine::register_nym_wrapper(
    const TaskID task,
    const RegisterNymTask& param,
    UniqueQueue<RegisterNymTask>& retry) const
{
    const auto output = register_nym(task, param);

    if (false == output) { retry.Push(next_task_id(), param); }

    return output;
}

template <typename M, typename I>
void StateMachine::resolve_unknown(const I& id, const bool found, M& map) const
{
    if (found) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Contract ")(id)(
            " successfully downloaded from server ")(op_.ServerID())
            .Flush();
        map.erase(id);
    } else {
        auto it = map.find(id);

        if (map.end() == it) {
            map.emplace(id, 1);
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Contract ")(id)(
                " not found on server ")(op_.ServerID())
                .Flush();
        } else {
            auto& value = it->second;

            if (value < (std::numeric_limits<int>::max() / 2)) { value *= 2; }

            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Increasing retry interval for contract ")(id)(" to ")(value)
                .Flush();
        }
    }
}

template <typename T>
bool StateMachine::run_task(bool (StateMachine::*func)(const TaskID) const)
{
    return run_task<T>(std::bind(func, this, std::placeholders::_1));
}

template <typename T>
bool StateMachine::run_task(bool (StateMachine::*func)(const TaskID, const T&)
                                const)
{
    return run_task<T>(
        std::bind(func, this, std::placeholders::_1, std::placeholders::_2));
}

template <typename T, typename R>
bool StateMachine::run_task(
    bool (StateMachine::*func)(const TaskID, const T&, R&) const,
    R& retry)
{
    const auto output = run_task<T>(
        [this, func, &retry](const TaskID task, const T& param) -> bool {
            return (this->*func)(task, param, retry);
        });
    auto param = make_blank<T>::value();

    while (retry.Pop(task_id_, param)) {
        bump_task(get_task<T>().Push(task_id_, param));
    }

    return output;
}

template <typename T>
bool StateMachine::run_task(std::function<bool(const TaskID, const T&)> func)
{
    auto& param = get_param<T>();
    new (&param) T(make_blank<T>::value());

    while (get_task<T>().Pop(task_id_, param)) {
        LogInsane(OT_METHOD)(__FUNCTION__)(": ")(--task_count_).Flush();

        SHUTDOWN()

        func(task_id_, param);
    }

    param.~T();

    return true;
}

template <typename T, typename M>
void StateMachine::scan_unknown(const M& map, int& next) const
{
    const auto thisLoop = counter_.load();
    const auto nextLoop = thisLoop + 1;

    for (const auto& [id, target] : map) {
        if (0 == thisLoop % target) {
            bump_task(get_task<T>().Push(next_task_id(), id));
        }

        if (0 == nextLoop % target) { ++next; }
    }
}

bool StateMachine::send_transfer(
    const TaskID taskID,
    const SendTransferTask& task) const
{
    const auto& [sourceAccountID, targetAccountID, value, memo] = task;

    DO_OPERATION(
        SendTransfer,
        sourceAccountID,
        targetAccountID,
        value,
        String::Factory(memo));

    return finish_task(taskID, success, std::move(result));
}

template <typename T>
StateMachine::BackgroundTask StateMachine::StartTask(const T& params) const
{
    Lock lock(decision_lock_);

    if (shutdown().load()) { return BackgroundTask{0, Future{}}; }

    const auto taskID{next_task_id()};
    auto output =
        start_task(taskID, bump_task(get_task<T>().Push(taskID, params)));
    trigger(lock);

    return std::move(output);
}

bool StateMachine::state_machine() noexcept
{
    const auto& nymID = op_.NymID();
    const auto& serverID = op_.ServerID();

    switch (state_) {
        case State::needServerContract: {
            SHUTDOWN();

            if (check_server_contract(serverID)) { return true; }

            [[fallthrough]];
        }
        case State::needRegistration: {
            SHUTDOWN();

            if (check_registration(nymID, serverID)) { return true; }

            [[fallthrough]];
        }
        case State::ready:
        default: {
            SHUTDOWN();

            return main_loop();
        }
    }
}

#if OT_CASH
bool StateMachine::withdraw_cash(
    const TaskID taskID,
    const WithdrawCashTask& task) const
{
    const auto& [accountID, amount] = task;

    DO_OPERATION(WithdrawCash, accountID, amount);

    return finish_task(taskID, success, std::move(result));
}
#endif  // OT_CASH

StateMachine::TaskDone StateMachine::write_and_send_cheque(
    const TaskID taskID,
    const SendChequeTask& task) const
{
    const auto& [accountID, recipient, value, memo, validFrom, validTo] = task;

    OT_ASSERT(false == accountID->empty())
    OT_ASSERT(false == recipient->empty())

    if (0 >= value) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid amount.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    auto context = client_.Wallet().ServerContext(op_.NymID(), op_.ServerID());

    OT_ASSERT(context);

    if (false ==
        context->HaveSufficientNumbers(MessageType::notarizeTransaction)) {
        return TaskDone::retry;
    }

    std::unique_ptr<Cheque> cheque(client_.OTAPI().WriteCheque(
        op_.ServerID(),
        value,
        Clock::to_time_t(validFrom),
        Clock::to_time_t(validTo),
        accountID,
        op_.NymID(),
        String::Factory(memo.c_str()),
        recipient));

    if (false == bool(cheque)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to write cheque.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    std::shared_ptr<OTPayment> payment{
        client_.Factory().Payment(String::Factory(*cheque))};

    if (false == bool(payment)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate payment.")
            .Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    if (false == payment->SetTempValues()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid payment.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    DO_OPERATION_TASK_DONE(ConveyPayment, recipient, payment);

    return task_done(finish_task(taskID, success, std::move(result)));
}

bool StateMachine::write_and_send_cheque_wrapper(
    const TaskID task,
    const SendChequeTask& param,
    UniqueQueue<SendChequeTask>& retry) const
{
    const auto done = write_and_send_cheque(task, param);

    if (TaskDone::retry == done) {
        const auto numbersTaskID{next_task_id()};
        start_task(
            numbersTaskID,
            bump_task(
                get_task<GetTransactionNumbersTask>().Push(numbersTaskID, {})));
        retry.Push(task, param);
    }

    return TaskDone::yes == done;
}
}  // namespace opentxs::api::client::implementation
