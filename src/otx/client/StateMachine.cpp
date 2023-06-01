// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/client/StateMachine.tpp"  // IWYU pragma: associated

#include <UnitDefinition.pb.h>
#include <atomic>
#include <chrono>
#include <compare>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

#include "2_Factory.hpp"
#include "core/StateMachine.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/client/Client.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/client/obsolete/OT_API.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "internal/util/UniqueQueue.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/otx/OperationType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Writer.hpp"
#include "otx/client/StateMachine.hpp"
#include "util/Blank.hpp"

#define CONTRACT_DOWNLOAD_MILLISECONDS 10000
#define NYM_REGISTRATION_MILLISECONDS 10000
#define STATE_MACHINE_READY_MILLISECONDS 100

#define DO_OPERATION(a, ...)                                                   \
    if (shutdown().load()) {                                                   \
        op_.Shutdown();                                                        \
                                                                               \
        return false;                                                          \
    }                                                                          \
                                                                               \
    auto started = op_.a(__VA_ARGS__);                                         \
                                                                               \
    while (false == started) {                                                 \
        LogDebug()(OT_PRETTY_CLASS())("State machine is not ready").Flush();   \
                                                                               \
        if (shutdown().load()) {                                               \
            op_.Shutdown();                                                    \
                                                                               \
            return false;                                                      \
        }                                                                      \
                                                                               \
        Sleep(std::chrono::milliseconds(STATE_MACHINE_READY_MILLISECONDS));    \
                                                                               \
        if (shutdown().load()) {                                               \
            op_.Shutdown();                                                    \
                                                                               \
            return false;                                                      \
        }                                                                      \
                                                                               \
        started = op_.a(__VA_ARGS__);                                          \
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
        otx::LastReplyStatus::MessageSuccess == std::get<0>(result)

#define DO_OPERATION_TASK_DONE(a, ...)                                         \
    auto started = op_.a(__VA_ARGS__);                                         \
                                                                               \
    while (false == started) {                                                 \
        LogDebug()(OT_PRETTY_CLASS())("State machine is not ready").Flush();   \
        if (shutdown().load()) {                                               \
            op_.Shutdown();                                                    \
                                                                               \
            return task_done(false);                                           \
        }                                                                      \
        Sleep(std::chrono::milliseconds(STATE_MACHINE_READY_MILLISECONDS));    \
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
                                                                               \
        return task_done(false);                                               \
    }                                                                          \
                                                                               \
    Result result = op_.GetFuture().get();                                     \
    const auto success =                                                       \
        otx::LastReplyStatus::MessageSuccess == std::get<0>(result)

#define SM_YIELD(a)                                                            \
    if (shutdown().load()) return false;                                       \
                                                                               \
    Sleep(std::chrono::milliseconds(a));                                       \
                                                                               \
    if (shutdown().load()) return false

#define SM_SHUTDOWN() SM_YIELD(50)

namespace opentxs::otx::client::implementation
{
StateMachine::StateMachine(
    const api::session::Client& client,
    const api::session::OTX& parent,
    const Flag& running,
    const api::session::Client& api,
    const ContextID& id,
    std::atomic<TaskID>& nextTaskID,
    const UniqueQueue<CheckNymTask>& missingnyms,
    const UniqueQueue<CheckNymTask>& outdatednyms,
    const UniqueQueue<identifier::Notary>& missingservers,
    const UniqueQueue<identifier::UnitDefinition>& missingUnitDefinitions,
    const PasswordPrompt& reason)
    : opentxs::internal::StateMachine([this] { return state_machine(); })
    , payment_tasks_(*this)
    , api_(client)
    , parent_(parent)
    , next_task_id_(nextTaskID)
    , missing_nyms_(missingnyms)
    , outdated_nyms_(outdatednyms)
    , missing_servers_(missingservers)
    , missing_unit_definitions_(missingUnitDefinitions)
    , reason_(api.Factory().PasswordPrompt(reason))
    , p_op_(opentxs::Factory::Operation(api, id.first, id.second, reason_))
    , op_(*p_op_)

    , check_nym_()
    , deposit_payment_()
    , download_contract_()
    , download_mint_()
    , download_nymbox_()
    , download_unit_definition_()
    , get_transaction_numbers_()
    , issue_unit_definition_()
    , send_message_()
    , send_cash_()
    , send_payment_()
    , peer_reply_()
    , peer_request_()
    , process_inbox_()
    , publish_server_contract_()
    , register_account_()
    , register_nym_()
    , send_cheque_()
    , send_transfer_()
    , withdraw_cash_()
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
    OT_ASSERT(p_op_);
}

auto StateMachine::bump_task(const bool bump) const -> bool
{
    if (bump) { LogInsane()(OT_PRETTY_CLASS())(++task_count_).Flush(); }

    return bump;
}

// If this server was added by a pairing operation that included a server
// password then request admin permissions on the server
auto StateMachine::check_admin(const otx::context::Server& context) const
    -> bool
{
    bool needAdmin{false};
    const auto haveAdmin = context.isAdmin();
    needAdmin = context.HaveAdminPassword() && (false == haveAdmin);

    if (needAdmin) {
        auto serverPassword =
            api_.Factory().SecretFromText(context.AdminPassword());
        get_admin(next_task_id(), serverPassword);
    }

    SM_SHUTDOWN();

    if (haveAdmin) { check_server_name(context); }

    SM_SHUTDOWN();

    return true;
}

template <typename T, typename C, typename M, typename U>
auto StateMachine::check_missing_contract(M& missing, U& unknown, bool skip)
    const -> bool
{
    auto items = missing.Copy();

    for (const auto& [targetID, taskID] : items) {
        SM_SHUTDOWN();

        find_contract<T, C>(taskID, targetID, missing, unknown, skip);
    }

    return true;
}

// Queue registerNym if the local nym has updated since the last registernym
// operation
void StateMachine::check_nym_revision(const otx::context::Server& context) const
{
    if (context.StaleNym()) {
        const auto& nymID = context.Nym()->ID();
        LogDetail()(OT_PRETTY_CLASS())("Nym ")(nymID, api_.Crypto())(
            " has is newer than version last registered version on "
            "server ")(context.Notary(), api_.Crypto())(".")
            .Flush();
        bump_task(get_task<RegisterNymTask>().Push(next_task_id(), true));
    }
}

auto StateMachine::check_registration(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID) const -> bool
{
    OT_ASSERT(false == nymID.empty());
    OT_ASSERT(false == serverID.empty());

    auto context = api_.Wallet().Internal().ServerContext(nymID, serverID);
    RequestNumber request{0};

    if (context) {
        request = context->Request();
    } else {
        LogDetail()(OT_PRETTY_CLASS())("Nym ")(nymID, api_.Crypto())(
            " has never registered on ")(serverID, api_.Crypto())
            .Flush();
    }

    if (0 != request) {
        LogVerbose()(OT_PRETTY_CLASS())("Nym ")(nymID, api_.Crypto())(
            " has registered on server ")(serverID, api_.Crypto())(
            " at least once.")
            .Flush();
        state_ = State::ready;

        OT_ASSERT(context);

        return false;
    }

    const auto output = register_nym(next_task_id(), false);

    if (output) {
        LogVerbose()(OT_PRETTY_CLASS())("Nym ")(nymID, api_.Crypto())(
            " is now registered on server ")(serverID, api_.Crypto())
            .Flush();
        state_ = State::ready;
        context = api_.Wallet().Internal().ServerContext(nymID, serverID);

        OT_ASSERT(context);

        return false;
    } else {
        SM_YIELD(NYM_REGISTRATION_MILLISECONDS);

        return true;
    }
}

auto StateMachine::check_server_contract(
    const identifier::Notary& serverID) const -> bool
{
    OT_ASSERT(false == serverID.empty());

    try {
        api_.Wallet().Internal().Server(serverID);
        LogVerbose()(OT_PRETTY_CLASS())("Server contract ")(
            serverID, api_.Crypto())(" exists.")
            .Flush();
        state_ = State::needRegistration;

        return false;
    } catch (...) {
    }

    LogDetail()(OT_PRETTY_CLASS())("Server contract for ")(
        serverID, api_.Crypto())(" is not in the wallet.")
        .Flush();
    missing_servers_.Push(next_task_id(), serverID);

    SM_YIELD(CONTRACT_DOWNLOAD_MILLISECONDS);

    return true;
}

auto StateMachine::check_server_name(const otx::context::Server& context) const
    -> bool
{
    try {
        const auto server = api_.Wallet().Internal().Server(op_.ServerID());
        const auto myName = server->Alias();
        const auto hisName = server->EffectiveName();

        if (myName == hisName) { return true; }

        DO_OPERATION(
            AddClaim,
            identity::wot::claim::SectionType::Scope,
            identity::wot::claim::ClaimType::Server,
            String::Factory(myName),
            true);

        if (success) {
            bump_task(get_task<CheckNymTask>().Push(
                next_task_id(), context.RemoteNym().ID()));
        }

        return success;
    } catch (...) {

        return false;
    }
}

// Periodically download server nym in case it has been renamed
void StateMachine::check_server_nym(const otx::context::Server& context) const
{
    if (0 == counter() % 100) {
        // download server nym in case it has been renamed
        bump_task(get_task<CheckNymTask>().Push(
            next_task_id(), context.RemoteNym().ID()));
    }
}

// Queue getTransactionNumbers if necessary
void StateMachine::check_transaction_numbers(
    const otx::context::Server& context) const
{
    if (0 == context.Accounts().size()) { return; }

    if (0 < context.AvailableNumbers()) { return; }

    bump_task(get_task<GetTransactionNumbersTask>().Push(next_task_id(), {}));
}

auto StateMachine::deposit_cheque(
    const TaskID taskID,
    const DepositPaymentTask& task) const -> bool
{
    const auto& [unitID, accountID, payment] = task;

    OT_ASSERT(false == accountID.empty());
    OT_ASSERT(payment);

    if ((false == payment->IsCheque()) && (false == payment->IsVoucher())) {
        LogError()(OT_PRETTY_CLASS())("Unhandled payment type.").Flush();

        return finish_task(taskID, false, error_result());
    }

    std::shared_ptr<Cheque> cheque{api_.Factory().InternalSession().Cheque()};

    OT_ASSERT(cheque);

    const auto loaded = cheque->LoadContractFromString(payment->Payment());

    if (false == loaded) {
        LogError()(OT_PRETTY_CLASS())("Invalid cheque.").Flush();

        return finish_task(taskID, false, error_result());
    }

    DO_OPERATION(DepositCheque, accountID, cheque);

    if (success) { return finish_task(taskID, success, std::move(result)); }

    return false;
}

auto StateMachine::deposit_cheque_wrapper(
    const TaskID task,
    const DepositPaymentTask& param,
    UniqueQueue<DepositPaymentTask>& retry) const -> bool
{
    bool output{false};
    const auto& [unitID, accountIDHint, payment] = param;

    OT_ASSERT(payment);

    auto depositServer = identifier::Notary{};
    auto depositUnitID = identifier::UnitDefinition{};
    auto depositAccount = identifier::Generic{};
    output = deposit_cheque(task, param);

    if (false == output) {
        retry.Push(task, param);
        bump_task(get_task<RegisterNymTask>().Push(next_task_id(), false));
    }

    return output;
}

auto StateMachine::download_mint(
    const TaskID taskID,
    const DownloadMintTask& task) const -> bool
{
    DO_OPERATION(Start, otx::OperationType::DownloadMint, task.first, {});

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::download_nym(const TaskID taskID, const CheckNymTask& id)
    const -> bool
{
    OT_ASSERT(false == id.empty());

    otx::context::Server::ExtraArgs args{};

    DO_OPERATION(Start, otx::OperationType::CheckNym, id, args);

    resolve_unknown(id, success, unknown_nyms_);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::download_nymbox(const TaskID taskID) const -> bool
{
    op_.join();
    auto contextE = api_.Wallet().Internal().mutable_ServerContext(
        op_.NymID(), op_.ServerID(), reason_);
    auto& context = contextE.get();
    context.Join();
    context.ResetThread();
    auto future = context.RefreshNymbox(api_, reason_);

    if (false == bool(future)) {

        return finish_task(taskID, false, error_result());
    }

    Result result{future->get()};
    const auto success =
        otx::LastReplyStatus::MessageSuccess == std::get<0>(result);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::error_result() const -> StateMachine::Result
{
    Result output{otx::LastReplyStatus::NotSent, nullptr};

    return output;
}

auto StateMachine::download_server(
    const TaskID taskID,
    const DownloadContractTask& contractID) const -> bool
{
    OT_ASSERT(false == contractID.empty());

    DO_OPERATION(DownloadContract, contractID);

    const bool found = success && std::get<1>(result)->bool_;

    resolve_unknown(contractID, found, unknown_servers_);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::download_unit_definition(
    const TaskID taskID,
    const DownloadUnitDefinitionTask& id) const -> bool
{
    OT_ASSERT(false == id.empty());

    DO_OPERATION(DownloadContract, id);

    const bool found = success && std::get<1>(result)->bool_;

    resolve_unknown(id, found, unknown_units_);

    return finish_task(taskID, success, std::move(result));
}

template <typename T, typename C, typename I, typename M, typename U>
auto StateMachine::find_contract(
    const TaskID taskID,
    const I& targetID,
    M& missing,
    U& unknown,
    const bool skipExisting) const -> bool
{
    if (load_contract<T>(targetID)) {
        if (skipExisting) {
            LogVerbose()(OT_PRETTY_CLASS())("Contract ")(
                targetID, api_.Crypto())(" exists in the wallet.")
                .Flush();
            missing.CancelByValue(targetID);

            return finish_task(taskID, true, error_result());
        } else {
            LogVerbose()(OT_PRETTY_CLASS())(
                "Attempting re-download of contract ")(targetID, api_.Crypto())
                .Flush();
        }
    }

    if (0 == unknown.count(targetID)) {
        LogVerbose()(OT_PRETTY_CLASS())("Queueing contract ")(
            targetID, api_.Crypto())(" for download on server ")(
            op_.ServerID(), api_.Crypto())
            .Flush();

        return bump_task(get_task<T>().Push(taskID, targetID));
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Previously failed to download contract ")(targetID, api_.Crypto())(
            " from server ")(op_.ServerID(), api_.Crypto())
            .Flush();

        finish_task(taskID, false, error_result());
    }

    return false;
}

auto StateMachine::get_admin(const TaskID taskID, const Secret& password) const
    -> bool
{
    DO_OPERATION(
        RequestAdmin,
        String::Factory(reinterpret_cast<const char*>(password.data())));

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::get_transaction_numbers(const TaskID taskID) const -> bool
{
    otx::context::Server::ExtraArgs args{};

    DO_OPERATION(Start, otx::OperationType::GetTransactionNumbers, args);

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

auto StateMachine::initiate_peer_reply(
    const TaskID taskID,
    const PeerReplyTask& task) const -> bool
{
    const auto [targetNymID, peerReply, peerRequest] = task;

    DO_OPERATION(SendPeerReply, targetNymID, peerReply, peerRequest);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::initiate_peer_request(
    const TaskID taskID,
    const PeerRequestTask& task) const -> bool
{
    const auto& [targetNymID, peerRequest] = task;

    DO_OPERATION(SendPeerRequest, targetNymID, peerRequest);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::issue_unit_definition(
    const TaskID taskID,
    const IssueUnitDefinitionTask& task) const -> bool
{
    try {
        const auto& [unitID, label, advertise] = task;
        auto unitDefinition = api_.Wallet().Internal().UnitDefinition(unitID);
        auto serialized = std::make_shared<proto::UnitDefinition>();

        OT_ASSERT(serialized);

        if (false == unitDefinition->Serialize(*serialized, true)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to serialize unit definition.")
                .Flush();

            return finish_task(taskID, false, error_result());
        }
        otx::context::Server::ExtraArgs args{label, false};

        DO_OPERATION(IssueUnitDefinition, serialized, args);

        if (success && (UnitType::Error != advertise)) {
            OT_ASSERT(result.second);

            const auto& reply = *result.second;
            const auto accountID =
                api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes());
            {
                auto nym = api_.Wallet().mutable_Nym(op_.NymID(), reason_);
                nym.AddContract(
                    unitID.asBase58(api_.Crypto()),
                    advertise,
                    true,
                    true,
                    reason_);
            }
        }

        return finish_task(taskID, success, std::move(result));
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Unit definition not found.").Flush();

        return finish_task(taskID, false, error_result());
    }
}

auto StateMachine::issue_unit_definition_wrapper(
    const TaskID task,
    const IssueUnitDefinitionTask& param) const -> bool
{
    const auto output = issue_unit_definition(task, param);
    bump_task(get_task<RegisterNymTask>().Push(next_task_id(), false));

    return output;
}

auto StateMachine::main_loop() noexcept -> bool
{
    int next{0};
    const auto tasks = task_count_.load();
    const auto& nymID = op_.NymID();
    const auto& serverID = op_.ServerID();
    UniqueQueue<DepositPaymentTask> retryDepositPayment{};
    UniqueQueue<RegisterNymTask> retryRegisterNym{};
    UniqueQueue<SendChequeTask> retrySendCheque{};
    auto pContext = api_.Wallet().Internal().ServerContext(nymID, serverID);

    OT_ASSERT(pContext);

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
    run_task<DownloadMintTask>(&StateMachine::download_mint);

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
    run_task<WithdrawCashTask>(&StateMachine::withdraw_cash);
    run_task<PayCashTask>(&StateMachine::pay_nym_cash);

    // Account maintenance
    run_task<RegisterAccountTask>(&StateMachine::register_account_wrapper);
    run_task<IssueUnitDefinitionTask>(
        &StateMachine::issue_unit_definition_wrapper);
    run_task<ProcessInboxTask>(&StateMachine::process_inbox);
    check_transaction_numbers(context);

    Lock lock(decision_lock_);
    const bool run = 0 < (task_count_.load() + tasks + next);
    increment_counter(run);

    if (false == run) {
        op_.join();
        context.Join();
    }

    return run;
}

auto StateMachine::message_nym(const TaskID taskID, const MessageTask& task)
    const -> bool
{
    const auto& [recipient, text, setID] = task;
    auto messageID = identifier::Generic{};
    auto updateID = [&](const identifier::Generic& in) -> void {
        messageID = in;
        const auto& pID = std::get<2>(task);

        if (pID) {
            auto& id = *pID;

            if (id) { id(in); }
        }
    };

    OT_ASSERT(false == recipient.empty());

    DO_OPERATION(SendMessage, recipient, String::Factory(text), updateID);

    if (success) {
        if (false == messageID.empty()) {
            LogVerbose()(OT_PRETTY_CLASS())("Sent message: ")(
                messageID, api_.Crypto())
                .Flush();
            associate_message_id(messageID, taskID);
        } else {
            LogError()(OT_PRETTY_CLASS())("Invalid message ID").Flush();
        }
    }

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::pay_nym(const TaskID taskID, const PaymentTask& task) const
    -> bool
{
    const auto& [recipient, payment] = task;

    OT_ASSERT(false == recipient.empty());

    DO_OPERATION(ConveyPayment, recipient, payment);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::pay_nym_cash(const TaskID taskID, const PayCashTask& task)
    const -> bool
{
    const auto& [recipient, workflowID] = task;

    OT_ASSERT(false == recipient.empty());

    DO_OPERATION(SendCash, recipient, workflowID);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::process_inbox(
    const TaskID taskID,
    const ProcessInboxTask& id) const -> bool
{
    OT_ASSERT(false == id.empty());

    DO_OPERATION(UpdateAccount, id);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::publish_server_contract(
    const TaskID taskID,
    const PublishServerContractTask& task) const -> bool
{
    const auto& id = task.first;

    OT_ASSERT(false == id.empty());

    DO_OPERATION(PublishContract, id);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::queue_contracts(
    const otx::context::Server& context,
    int& next) -> bool
{
    check_server_nym(context);
    check_missing_contract<CheckNymTask, identity::Nym>(
        missing_nyms_, unknown_nyms_);
    check_missing_contract<CheckNymTask, identity::Nym>(
        outdated_nyms_, unknown_nyms_, false);
    check_missing_contract<DownloadContractTask, contract::Server>(
        missing_servers_, unknown_servers_);
    check_missing_contract<DownloadUnitDefinitionTask, contract::Unit>(
        missing_unit_definitions_, unknown_units_);
    queue_nyms();

    scan_unknown<CheckNymTask>(unknown_nyms_, next);
    scan_unknown<DownloadContractTask>(unknown_servers_, next);
    scan_unknown<DownloadUnitDefinitionTask>(unknown_units_, next);

    return true;
}

auto StateMachine::queue_nyms() -> bool
{
    const auto blank = identifier::Notary{};
    auto nymID = identifier::Nym{};

    while (get_nym_fetch(op_.ServerID()).Pop(task_id_, nymID)) {
        SM_SHUTDOWN();

        if (0 == unknown_nyms_.count(nymID)) {
            bump_task(get_task<CheckNymTask>().Push(task_id_, nymID));
        }
    }

    while (get_nym_fetch(blank).Pop(task_id_, nymID)) {
        SM_SHUTDOWN();

        if (0 == unknown_nyms_.count(nymID)) {
            bump_task(get_task<CheckNymTask>().Push(task_id_, nymID));
        }
    }

    return true;
}

auto StateMachine::register_account(
    const TaskID taskID,
    const RegisterAccountTask& task) const -> bool
{
    const auto& [label, unitID] = task;

    OT_ASSERT(false == unitID.empty());

    try {
        api_.Wallet().Internal().UnitDefinition(unitID);
    } catch (...) {
        DO_OPERATION(DownloadContract, unitID, contract::Type::unit);

        if (false == success) {
            return finish_task(taskID, success, std::move(result));
        }
    }

    otx::context::Server::ExtraArgs args{label, false};

    DO_OPERATION(
        Start, otx::OperationType::RegisterAccount, unitID, {label, false});

    finish_task(taskID, success, std::move(result));

    return success;
}

auto StateMachine::register_account_wrapper(
    const TaskID task,
    const RegisterAccountTask& param) const -> bool
{
    const auto done = register_account(task, param);

    if (false == done) {
        bump_task(get_task<RegisterNymTask>().Push(next_task_id(), false));
    }

    return done;
}

auto StateMachine::register_nym(
    const TaskID taskID,
    const RegisterNymTask& resync) const -> bool
{
    otx::context::Server::ExtraArgs args{};

    if (resync) { std::get<1>(args) = true; }

    DO_OPERATION(Start, otx::OperationType::RegisterNym, args);

    return finish_task(taskID, success, std::move(result));
}

// Register the nym, if scheduled. Keep trying until success
auto StateMachine::register_nym_wrapper(
    const TaskID task,
    const RegisterNymTask& param,
    UniqueQueue<RegisterNymTask>& retry) const -> bool
{
    const auto output = register_nym(task, param);

    if (false == output) { retry.Push(next_task_id(), param); }

    return output;
}

template <typename M, typename I>
void StateMachine::resolve_unknown(const I& id, const bool found, M& map) const
{
    if (found) {
        LogVerbose()(OT_PRETTY_CLASS())("Contract ")(id, api_.Crypto())(
            " successfully downloaded from server ")(
            op_.ServerID(), api_.Crypto())
            .Flush();
        map.erase(id);
    } else {
        auto it = map.find(id);

        if (map.end() == it) {
            map.emplace(id, 1);
            LogVerbose()(OT_PRETTY_CLASS())("Contract ")(id, api_.Crypto())(
                " not found on server ")(op_.ServerID(), api_.Crypto())
                .Flush();
        } else {
            auto& value = it->second;

            if (value < (std::numeric_limits<int>::max() / 2)) { value *= 2; }

            LogVerbose()(OT_PRETTY_CLASS())(
                "Increasing retry interval for contract ")(id, api_.Crypto())(
                " to ")(value)
                .Flush();
        }
    }
}

template <typename T>
auto StateMachine::run_task(bool (StateMachine::*func)(const TaskID) const)
    -> bool
{
    // NOLINTNEXTLINE(modernize-avoid-bind)
    return run_task<T>(std::bind(func, this, std::placeholders::_1));
}

template <typename T>
auto StateMachine::run_task(bool (StateMachine::*func)(const TaskID, const T&)
                                const) -> bool
{
    // NOLINTBEGIN(modernize-avoid-bind)
    return run_task<T>(
        std::bind(func, this, std::placeholders::_1, std::placeholders::_2));
    // NOLINTEND(modernize-avoid-bind)
}

template <typename T, typename R>
auto StateMachine::run_task(
    bool (StateMachine::*func)(const TaskID, const T&, R&) const,
    R& retry) -> bool
{
    const auto output = run_task<T>(
        [this, func, &retry](const TaskID task, const T& param) -> bool {
            return (this->*func)(task, param, retry);
        });
    auto param = make_blank<T>::value(api_);

    while (retry.Pop(task_id_, param)) {
        bump_task(get_task<T>().Push(task_id_, param));
    }

    return output;
}

template <typename T>
auto StateMachine::run_task(std::function<bool(const TaskID, const T&)> func)
    -> bool
{
    auto& param = get_param<T>();
    new (&param) T(make_blank<T>::value(api_));

    while (get_task<T>().Pop(task_id_, param)) {
        LogInsane()(OT_PRETTY_CLASS())(--task_count_).Flush();

        SM_SHUTDOWN();

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

auto StateMachine::send_transfer(
    const TaskID taskID,
    const SendTransferTask& task) const -> bool
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
auto StateMachine::StartTask(const T& params) const
    -> StateMachine::BackgroundTask
{
    return StartTask<T>(next_task_id(), params);
}

template <typename T>
auto StateMachine::StartTask(const TaskID taskID, const T& params) const
    -> StateMachine::BackgroundTask
{
    Lock lock(decision_lock_);

    if (shutdown().load()) {
        LogVerbose()(OT_PRETTY_CLASS())("Shutting down").Flush();

        return BackgroundTask{0, Future{}};
    }

    auto output =
        start_task(taskID, bump_task(get_task<T>().Push(taskID, params)));
    trigger(lock);

    return output;
}

auto StateMachine::state_machine() noexcept -> bool
{
    const auto& nymID = op_.NymID();
    const auto& serverID = op_.ServerID();

    switch (state_) {
        case State::needServerContract: {
            SM_SHUTDOWN();

            if (check_server_contract(serverID)) { return true; }

            [[fallthrough]];
        }
        case State::needRegistration: {
            SM_SHUTDOWN();

            if (check_registration(nymID, serverID)) { return true; }

            [[fallthrough]];
        }
        case State::ready:
        default: {
            SM_SHUTDOWN();

            return main_loop();
        }
    }
}

auto StateMachine::withdraw_cash(
    const TaskID taskID,
    const WithdrawCashTask& task) const -> bool
{
    const auto& [accountID, amount] = task;

    DO_OPERATION(WithdrawCash, accountID, amount);

    return finish_task(taskID, success, std::move(result));
}

auto StateMachine::write_and_send_cheque(
    const TaskID taskID,
    const SendChequeTask& task) const -> StateMachine::TaskDone
{
    const auto& [accountID, recipient, value, memo, validFrom, validTo] = task;

    OT_ASSERT(false == accountID.empty());
    OT_ASSERT(false == recipient.empty());

    if (0 >= value) {
        LogError()(OT_PRETTY_CLASS())("Invalid amount.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    auto context =
        api_.Wallet().Internal().ServerContext(op_.NymID(), op_.ServerID());

    OT_ASSERT(context);

    if (false == context->InternalServer().HaveSufficientNumbers(
                     MessageType::notarizeTransaction)) {
        return TaskDone::retry;
    }

    std::unique_ptr<Cheque> cheque(api_.InternalClient().OTAPI().WriteCheque(
        op_.ServerID(),
        value,
        validFrom,
        validTo,
        accountID,
        op_.NymID(),
        String::Factory(memo.c_str()),
        recipient));

    if (false == bool(cheque)) {
        LogError()(OT_PRETTY_CLASS())("Failed to write cheque.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    std::shared_ptr<OTPayment> payment{
        api_.Factory().InternalSession().Payment(String::Factory(*cheque))};

    if (false == bool(payment)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate payment.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    if (false == payment->SetTempValues(reason_)) {
        LogError()(OT_PRETTY_CLASS())("Invalid payment.").Flush();

        return task_done(finish_task(taskID, false, error_result()));
    }

    DO_OPERATION_TASK_DONE(ConveyPayment, recipient, payment);

    return task_done(finish_task(taskID, success, std::move(result)));
}

auto StateMachine::write_and_send_cheque_wrapper(
    const TaskID task,
    const SendChequeTask& param,
    UniqueQueue<SendChequeTask>& retry) const -> bool
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

StateMachine::Params::~Params() {}  // NOLINT
}  // namespace opentxs::otx::client::implementation

#undef SM_SHUTDOWN
#undef SM_YIELD
#undef DO_OPERATION_TASK_DONE
#undef DO_OPERATION
#undef STATE_MACHINE_READY_MILLISECONDS
#undef NYM_REGISTRATION_MILLISECONDS
#undef CONTRACT_DOWNLOAD_MILLISECONDS
