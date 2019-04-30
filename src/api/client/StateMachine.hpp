// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/UniqueQueue.hpp"

#include "core/StateMachine.hpp"
#include "internal/api/client/Client.hpp"

#include <functional>
#include <future>
#include <tuple>

namespace std
{
using MESSAGETASK = tuple<opentxs::OTNymID, string, opentxs::SetID>;
using PAYMENTTASK =
    pair<opentxs::OTIdentifier, shared_ptr<const opentxs::OTPayment>>;

template <>
struct less<PAYMENTTASK> {
    bool operator()(const PAYMENTTASK& lhs, const PAYMENTTASK& rhs) const
    {
        /* TODO: use structured bindings */
        const auto& lID = std::get<0>(lhs);
        const auto& lPayment = std::get<1>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rPayment = std::get<1>(rhs);

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        auto lPaymentID = opentxs::Identifier::Factory();
        auto rPaymentID = opentxs::Identifier::Factory();
        
        lPayment->GetIdentifier(lPaymentID);
        rPayment->GetIdentifier(rPaymentID);

        if (lPaymentID->str() < rPaymentID->str()) { return true; }
        
        return false;
    }
};

template <>
struct less<MESSAGETASK> {
    bool operator()(const MESSAGETASK& lhs, const MESSAGETASK& rhs) const
    {
        /* TODO: use structured bindings */
        const auto& lID = std::get<0>(lhs);
        const auto& lMessage = std::get<1>(lhs);
        const auto& lFunction = std::get<2>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rMessage = std::get<1>(rhs);
        const auto& rFunction = std::get<2>(rhs);

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        if (lMessage < rMessage) { return true; }

        if (rMessage < lMessage) { return false; }

        if (&lFunction < &rFunction) { return true; }

        return false;
    }
};
}  // namespace std

namespace opentxs::api::client::implementation
{
class StateMachine : public opentxs::internal::StateMachine
{
public:
    using BackgroundTask = api::client::OTX::BackgroundTask;
    using ContextID = std::pair<OTNymID, OTServerID>;
    using Future = api::client::OTX::Future;
    using RefreshTask = std::pair<int, std::promise<void>>;
    using Result = api::client::OTX::Result;
    using TaskID = api::client::OTX::TaskID;
    using Thread = std::function<void()>;

    union Params {
        CheckNymTask check_nym_;
        DepositPaymentTask deposit_payment_;
        DownloadContractTask download_contract_;
#if OT_CASH
        DownloadMintTask download_mint_;
#endif
        DownloadNymboxTask download_nymbox_;
        DownloadUnitDefinitionTask download_unit_definition_;
        GetTransactionNumbersTask get_transaction_numbers_;
        IssueUnitDefinitionTask issue_unit_definition_;
        MessageTask send_message_;
#if OT_CASH
        PayCashTask send_cash_;
#endif
        PaymentTask send_payment_;
        PeerReplyTask peer_reply_;
        PeerRequestTask peer_request_;
        ProcessInboxTask process_inbox_;
        PublishServerContractTask publish_server_contract_;
        RegisterAccountTask register_account_;
        RegisterNymTask register_nym_;
        SendChequeTask send_cheque_;
        SendTransferTask send_transfer_;
#if OT_CASH
        WithdrawCashTask withdraw_cash_;
#endif

        Params() { memset(this, 0, sizeof(Params)); }
        ~Params() {}
    };

    static Result error_result();

    template <typename T>
    BackgroundTask StartTask(const T& params) const;

    void Shutdown() { op_.Shutdown(); }

    StateMachine(
        const api::client::Manager& client,
        const api::client::internal::OTX& parent,
        const Flag& running,
        const api::client::Manager& api,
        const ContextID& id,
        std::atomic<TaskID>& nextTaskID,
        const UniqueQueue<CheckNymTask>& missingNyms,
        const UniqueQueue<CheckNymTask>& outdatedNyms,
        const UniqueQueue<OTServerID>& missingServers,
        const UniqueQueue<OTUnitID>& missingUnitDefinitions);

private:
    enum class TaskDone : int { no, yes, retry };
    enum class State : int { needServerContract, needRegistration, ready };

    const api::client::Manager& client_;
    const api::client::internal::OTX& parent_;
    std::atomic<TaskID>& next_task_id_;
    const UniqueQueue<CheckNymTask>& missing_nyms_;
    const UniqueQueue<CheckNymTask>& outdated_nyms_;
    const UniqueQueue<OTServerID>& missing_servers_;
    const UniqueQueue<OTUnitID>& missing_unit_definitions_;
    std::unique_ptr<api::client::internal::Operation> pOp_;
    api::client::internal::Operation& op_;
    UniqueQueue<CheckNymTask> check_nym_;
    UniqueQueue<DepositPaymentTask> deposit_payment_;
    UniqueQueue<DownloadContractTask> download_contract_;
#if OT_CASH
    UniqueQueue<DownloadMintTask> download_mint_;
#endif  // OT_CASH
    UniqueQueue<DownloadNymboxTask> download_nymbox_;
    UniqueQueue<DownloadUnitDefinitionTask> download_unit_definition_;
    UniqueQueue<GetTransactionNumbersTask> get_transaction_numbers_;
    UniqueQueue<IssueUnitDefinitionTask> issue_unit_definition_;
    UniqueQueue<MessageTask> send_message_;
#if OT_CASH
    UniqueQueue<PayCashTask> send_cash_;
#endif  // OT_CASH
    UniqueQueue<PaymentTask> send_payment_;
    UniqueQueue<PeerReplyTask> peer_reply_;
    UniqueQueue<PeerRequestTask> peer_request_;
    UniqueQueue<ProcessInboxTask> process_inbox_;
    UniqueQueue<PublishServerContractTask> publish_server_contract_;
    UniqueQueue<RegisterAccountTask> register_account_;
    UniqueQueue<RegisterNymTask> register_nym_;
    UniqueQueue<SendChequeTask> send_cheque_;
    UniqueQueue<SendTransferTask> send_transfer_;
#if OT_CASH
    UniqueQueue<WithdrawCashTask> withdraw_cash_;
#endif  // OT_CASH
    Params param_;
    TaskID task_id_{};
    std::atomic<int> counter_;
    mutable std::atomic<int> task_count_;
    mutable std::mutex lock_;
    std::vector<RefreshTask> tasks_;
    mutable State state_;
    mutable std::map<OTNymID, int> unknown_nyms_;
    mutable std::map<OTServerID, int> unknown_servers_;
    mutable std::map<OTUnitID, int> unknown_units_;

    static TaskDone task_done(bool done)
    {
        return done ? TaskDone::yes : TaskDone::no;
    }

    void associate_message_id(const Identifier& messageID, const TaskID taskID)
        const
    {
        return parent_.associate_message_id(messageID, taskID);
    }
    bool bump_task(const bool bump) const;
    Depositability can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const Identifier& accountIDHint,
        identifier::Server& depositServer,
        Identifier& depositAccount) const
    {
        return parent_.can_deposit(
            payment, recipient, accountIDHint, depositServer, depositAccount);
    }
    bool check_admin(const ServerContext& context) const;
    template <typename T, typename C, typename M, typename U>
    bool check_missing_contract(M& missing, U& unknown, bool skip = true) const;
    void check_nym_revision(const ServerContext& context) const;
    bool check_registration(
        const identifier::Nym& nymID,
        const identifier::Server& serverID) const;
    bool check_server_contract(const identifier::Server& serverID) const;
    bool check_server_name(const ServerContext& context) const;
    void check_server_nym(const ServerContext& context) const;
    void check_transaction_numbers(const ServerContext& context) const;
    int counter() const { return counter_.load(); }
    bool deposit_cheque(const TaskID taskID, const DepositPaymentTask& task)
        const;
    bool deposit_cheque_wrapper(
        const TaskID taskID,
        const DepositPaymentTask& task,
        UniqueQueue<DepositPaymentTask>& retry) const;
#if OT_CASH
    bool download_mint(const TaskID taskID, const DownloadMintTask& task) const;
#endif
    bool download_nym(const TaskID taskID, const CheckNymTask& id) const;
    bool download_nymbox(const TaskID taskID) const;
    bool download_server(
        const TaskID taskID,
        const DownloadContractTask& contractID) const;
    bool download_unit_definition(
        const TaskID taskID,
        const DownloadUnitDefinitionTask& id) const;
    template <typename T, typename C, typename I, typename M, typename U>
    bool find_contract(
        const TaskID taskID,
        const I& targetID,
        M& missing,
        U& unknown,
        const bool skipExisting = true) const;
    bool finish_task(const TaskID taskID, const bool success, Result&& result)
        const
    {
        return parent_.finish_task(taskID, success, std::move(result));
    }
    bool get_admin(const TaskID taskID, const OTPassword& password) const;
    UniqueQueue<OTNymID>& get_nym_fetch(
        const identifier::Server& serverID) const
    {
        return parent_.get_nym_fetch(serverID);
    }
    template <typename T>
    const UniqueQueue<T>& get_task() const;
    bool get_transaction_numbers(const TaskID taskID) const;
    bool initiate_peer_reply(const TaskID taskID, const PeerReplyTask& task)
        const;
    bool initiate_peer_request(const TaskID taskID, const PeerRequestTask& task)
        const;
    bool issue_unit_definition(
        const TaskID taskID,
        const IssueUnitDefinitionTask& task) const;
    bool issue_unit_definition_wrapper(
        const TaskID taskID,
        const IssueUnitDefinitionTask& task) const;
    template <typename T, typename C, typename I>
    std::shared_ptr<const C> load_contract(const I& id) const;
    bool message_nym(const TaskID taskID, const MessageTask& task) const;
    TaskID next_task_id() const { return ++next_task_id_; }
    bool pay_nym(const TaskID taskID, const PaymentTask& task) const;
#if OT_CASH
    bool pay_nym_cash(const TaskID taskID, const PayCashTask& Task) const;
#endif  // OT_CASH
    bool process_inbox(const TaskID taskID, const ProcessInboxTask& accountID)
        const;
    bool publish_server_contract(
        const TaskID taskID,
        const PublishServerContractTask& serverID) const;
    bool register_account(const TaskID taskID, const RegisterAccountTask& task)
        const;
    bool register_account_wrapper(
        const TaskID taskID,
        const RegisterAccountTask& task) const;
    bool register_nym(const TaskID taskID, const RegisterNymTask& resync) const;
    bool register_nym_wrapper(
        const TaskID taskID,
        const RegisterNymTask& resync,
        UniqueQueue<RegisterNymTask>& retry) const;
    template <typename M, typename I>
    void resolve_unknown(const I& id, const bool found, M& map) const;
    template <typename T, typename M>
    void scan_unknown(const M& map, int& next) const;
    bool send_transfer(const TaskID taskID, const SendTransferTask& task) const;
    BackgroundTask start_task(const TaskID taskID, bool success) const
    {
        return parent_.start_task(taskID, success);
    }
#if OT_CASH
    bool withdraw_cash(const TaskID taskID, const WithdrawCashTask& task) const;
#endif  // OT_CASH
    TaskDone write_and_send_cheque(
        const TaskID taskID,
        const SendChequeTask& task) const;
    bool write_and_send_cheque_wrapper(
        const TaskID taskID,
        const SendChequeTask& task,
        UniqueQueue<SendChequeTask>& retry) const;

    std::future<void> add_task(const Lock& lock);
    template <typename T>
    T& get_param();
    void increment_counter(const bool run);
    bool main_loop() noexcept;
    bool queue_contracts(const ServerContext& context, int& next);
    bool queue_nyms();
    template <typename T>
    bool run_task(bool (StateMachine::*func)(const TaskID) const);
    template <typename T>
    bool run_task(bool (StateMachine::*func)(const TaskID, const T&) const);
    template <typename T, typename R>
    bool run_task(
        bool (StateMachine::*func)(const TaskID, const T&, R&) const,
        R& retry);
    template <typename T>
    bool run_task(std::function<bool(const TaskID, const T&)> func);
    bool state_machine() noexcept;

    StateMachine() = delete;
};
}  // namespace opentxs::api::client::implementation
