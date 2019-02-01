// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

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

        if (lPayment.get() < rPayment.get()) { return true; }

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
class OTX : virtual public client::OTX, Lockable
{
public:
    BackgroundTask AcknowledgeBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const Identifier& requestID,
        const std::string& instructions,
        const SetID setID) const override;
    BackgroundTask AcknowledgeNotice(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const bool ack,
        const SetID setID) const override;
    BackgroundTask AcknowledgeOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const std::string& details,
        const SetID setID) const override;
    BackgroundTask AcknowledgeConnection(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& recipientID,
        const Identifier& requestID,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const SetID setID) const override;
    bool AutoProcessInboxEnabled() const override
    {
        return auto_process_inbox_.get();
    };
    Depositability CanDeposit(
        const identifier::Nym& recipientNymID,
        const OTPayment& payment) const override;
    Depositability CanDeposit(
        const identifier::Nym& recipientNymID,
        const Identifier& accountID,
        const OTPayment& payment) const override;
    Messagability CanMessage(
        const identifier::Nym& senderNymID,
        const Identifier& recipientContactID,
        const bool startIntroductionServer) const override;
    bool CheckTransactionNumbers(
        const identifier::Nym& nym,
        const Identifier& serverID,
        const std::size_t quantity) const override;
    Finished ContextIdle(
        const identifier::Nym& nym,
        const identifier::Server& server) const override;
    std::size_t DepositCheques(const identifier::Nym& nymID) const override;
    std::size_t DepositCheques(
        const identifier::Nym& nymID,
        const std::set<OTIdentifier>& chequeIDs) const override;
    BackgroundTask DepositPayment(
        const identifier::Nym& recipientNymID,
        const std::shared_ptr<const OTPayment>& payment) const override;
    BackgroundTask DepositPayment(
        const identifier::Nym& recipientNymID,
        const Identifier& accountID,
        const std::shared_ptr<const OTPayment>& payment) const override;
    void DisableAutoaccept() const override;
    BackgroundTask DownloadContract(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const override;
#if OT_CASH
    BackgroundTask DownloadMint(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit) const override;
#endif
    BackgroundTask DownloadNym(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const identifier::Nym& targetNymID) const override;
    BackgroundTask DownloadNymbox(
        const identifier::Nym& localNymID,
        const Identifier& serverID) const override;
    BackgroundTask FindNym(const identifier::Nym& nymID) const override;
    BackgroundTask FindNym(
        const identifier::Nym& nymID,
        const Identifier& serverIDHint) const override;
    BackgroundTask FindServer(const Identifier& serverID) const override;
    BackgroundTask InitiateBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const SetID setID) const override;
    BackgroundTask InitiateOutbailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Amount amount,
        const std::string& message,
        const SetID setID) const override;
    BackgroundTask InitiateRequestConnection(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const proto::ConnectionInfoType& type,
        const SetID setID) const override;
    BackgroundTask InitiateStoreSecret(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const proto::SecretType& type,
        const std::string& primary,
        const std::string& secondary,
        const SetID setID) const override;
    const Identifier& IntroductionServer() const override;
    BackgroundTask IssueUnitDefinition(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& unitID,
        const std::string& label) const override;
    BackgroundTask MessageContact(
        const identifier::Nym& senderNymID,
        const Identifier& contactID,
        const std::string& message,
        const SetID setID) const override;
    std::pair<ThreadStatus, MessageID> MessageStatus(
        const TaskID taskID) const override;
    BackgroundTask NotifyBailment(
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount amount,
        const SetID setID) const override;
    BackgroundTask PayContact(
        const identifier::Nym& senderNymID,
        const Identifier& contactID,
        std::shared_ptr<const OTPayment> payment) const override;
#if OT_CASH
    BackgroundTask PayContactCash(
        const identifier::Nym& senderNymID,
        const Identifier& contactID,
        const Identifier& workflowID) const override;
#endif  // OT_CASH
    BackgroundTask ProcessInbox(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& accountID) const override;
    BackgroundTask PublishServerContract(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const override;
    void Refresh() const override;
    std::uint64_t RefreshCount() const override;
    BackgroundTask RegisterAccount(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& unitID,
        const std::string& label) const override;
    BackgroundTask RegisterNym(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const bool resync) const override;
    BackgroundTask RegisterNymPublic(
        const identifier::Nym& nymID,
        const Identifier& server,
        const bool setContactData,
        const bool forcePrimary,
        const bool resync) const override;
    OTIdentifier SetIntroductionServer(
        const ServerContract& contract) const override;
    BackgroundTask SendCheque(
        const identifier::Nym& localNymID,
        const Identifier& sourceAccountID,
        const Identifier& recipientContactID,
        const Amount value,
        const std::string& memo,
        const Time validFrom,
        const Time validTo) const override;
    BackgroundTask SendExternalTransfer(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount value,
        const std::string& memo) const override;
    BackgroundTask SendTransfer(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount value,
        const std::string& memo) const override;
    void StartIntroductionServer(
        const identifier::Nym& localNymID) const override;
    ThreadStatus Status(const TaskID taskID) const override;
#if OT_CASH
    BackgroundTask WithdrawCash(
        const identifier::Nym& nymID,
        const identifier::Server& serverID,
        const Identifier& account,
        const Amount value) const override;
#endif  // OT_CASH

    ~OTX();

private:
    static const std::string DEFAULT_INTRODUCTION_SERVER;

    friend opentxs::Factory;

    using TaskStatusMap =
        std::map<TaskID, std::pair<ThreadStatus, std::promise<Result>>>;

    /** ContextID: localNymID, serverID */
    using ContextID = std::pair<OTNymID, OTIdentifier>;
    /** MessageTask: recipientID, message */
    using MessageTask = std::tuple<OTNymID, std::string, SetID>;
    /** PaymentTask: recipientID, payment */
    using PaymentTask = std::pair<OTNymID, std::shared_ptr<const OTPayment>>;
#if OT_CASH
    /** PayCashTask: recipientID, workflow ID */
    using PayCashTask = std::pair<OTNymID, OTIdentifier>;
    /** WithdrawCashTask: Account ID, amount*/
    using WithdrawCashTask = std::pair<OTIdentifier, Amount>;
#endif  // OT_CASH
    /** DepositPaymentTask: accountID, payment */
    using DepositPaymentTask =
        std::pair<OTIdentifier, std::shared_ptr<const OTPayment>>;
    /** SendTransferTask: localNymID, serverID, sourceAccountID, targetAccountID
     */
    using SendTransferTask =
        std::tuple<OTNymID, OTIdentifier, uint64_t, std::string>;
    /** SendChequeTask: sourceAccountID, targetNymID, value, memo, validFrom,
     * validTo
     */
    using SendChequeTask =
        std::tuple<OTIdentifier, OTNymID, Amount, std::string, Time, Time>;
    /** RegisterAccountTask: unit definition id, account label */
    using RegisterAccountTask = std::pair<OTIdentifier, std::string>;
    /** IssueUnitDefinitionTask: unit definition id, account label */
    using IssueUnitDefinitionTask = std::pair<OTIdentifier, std::string>;
    using DownloadMintTask = OTUnitID;
    /** PeerReplyTask: targetNymID, peer reply, peer request */
    using PeerReplyTask = std::tuple<
        OTNymID,
        std::shared_ptr<const PeerReply>,
        std::shared_ptr<const PeerRequest>>;
    /** PeerRequestTask: targetNymID, peer request */
    using PeerRequestTask =
        std::pair<OTNymID, std::shared_ptr<const PeerRequest>>;
    struct OperationQueue {
        using RefreshTask = std::pair<int, std::promise<void>>;
        using Thread = std::function<void()>;

        std::unique_ptr<api::client::internal::Operation> op_;
        UniqueQueue<OTNymID> check_nym_;
        UniqueQueue<DepositPaymentTask> deposit_payment_;
        UniqueQueue<OTIdentifier> download_contract_;
        UniqueQueue<DownloadMintTask> download_mint_;
        UniqueQueue<bool> download_nymbox_;
        UniqueQueue<bool> get_transaction_numbers_;
        UniqueQueue<IssueUnitDefinitionTask> issue_unit_definition_;
        UniqueQueue<RegisterAccountTask> register_account_;
        UniqueQueue<bool> register_nym_;
        UniqueQueue<MessageTask> send_message_;
        UniqueQueue<PaymentTask> send_payment_;
#if OT_CASH
        UniqueQueue<PayCashTask> send_cash_;
        UniqueQueue<WithdrawCashTask> withdraw_cash_;
#endif  // OT_CASH
        UniqueQueue<SendTransferTask> send_transfer_;
        UniqueQueue<OTServerID> publish_server_contract_;
        UniqueQueue<OTIdentifier> process_inbox_;
        UniqueQueue<SendChequeTask> send_cheque_;
        UniqueQueue<PeerReplyTask> peer_reply_;
        UniqueQueue<PeerRequestTask> peer_request_;

        Lock lock() const { return Lock{lock_, std::defer_lock}; }
        int counter() const { return counter_.load(); }

        std::future<void> add_task(const Lock& lock);
        void check_thread(Thread task, std::unique_ptr<std::thread>& thread);
        std::future<void> check_future(
            Thread task,
            std::unique_ptr<std::thread>& thread);
        void cleanup(Lock& lock);
        void increment_counter(const bool missing, Lock& lock, bool& run);

        OperationQueue(const api::client::Manager& api, const ContextID& id);
        OperationQueue() = delete;

    private:
        std::atomic<int> counter_;
        mutable std::mutex lock_;
        bool thread_;
        bool continue_;
        std::vector<RefreshTask> tasks_;

        void check_thread(
            const Lock& lock,
            Thread task,
            std::unique_ptr<std::thread>& thread);
    };

    enum class TaskDone : int { no, yes, retry };

    TaskDone task_done(bool done) const
    {
        return done ? TaskDone::yes : TaskDone::no;
    }

    ContextLockCallback lock_callback_;
    const Flag& running_;
    const api::client::Manager& client_;
    OTClient& ot_client_;
    mutable std::mutex introduction_server_lock_{};
    mutable std::mutex nym_fetch_lock_{};
    mutable std::mutex task_status_lock_{};
    mutable std::atomic<std::uint64_t> refresh_counter_{0};
    mutable std::map<ContextID, OperationQueue> operations_;
    mutable std::map<OTIdentifier, UniqueQueue<OTNymID>> server_nym_fetch_;
    UniqueQueue<OTNymID> missing_nyms_;
    UniqueQueue<OTIdentifier> missing_servers_;
    mutable std::map<ContextID, std::unique_ptr<std::thread>> state_machines_;
    mutable std::unique_ptr<OTIdentifier> introduction_server_id_;
    mutable TaskStatusMap task_status_;
    mutable std::map<TaskID, MessageID> task_message_id_;
    OTZMQListenCallback account_subscriber_callback_;
    OTZMQSubscribeSocket account_subscriber_;
    OTZMQListenCallback notification_listener_callback_;
    OTZMQPullSocket notification_listener_;
    OTZMQPublishSocket task_finished_;
    mutable OTFlag auto_process_inbox_;
    mutable std::atomic<TaskID> next_task_id_;

    static Result error_result();
    static BackgroundTask error_task();

    BackgroundTask add_task(const TaskID taskID, const ThreadStatus status)
        const;
    void associate_message_id(const Identifier& messageID, const TaskID taskID)
        const;
    Depositability can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const Identifier& accountIDHint,
        OTIdentifier& depositServer,
        OTIdentifier& depositAccount) const;
    Messagability can_message(
        const identifier::Nym& senderID,
        const Identifier& recipientID,
        OTNymID& recipientNymID,
        OTIdentifier& serverID) const;
    void check_nym_revision(const ServerContext& context, OperationQueue& queue)
        const;
    bool check_registration(
        const identifier::Nym& nymID,
        const Identifier& serverID,
        api::client::internal::Operation& op,
        std::shared_ptr<const ServerContext>& context) const;
    bool check_server_contract(const Identifier& serverID) const;
    bool check_server_name(OperationQueue& queue, const ServerContext& context)
        const;
    bool deposit_cheque(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const Identifier& accountID,
        const std::shared_ptr<const OTPayment>& cheque,
        UniqueQueue<DepositPaymentTask>& retry) const;
    bool download_contract(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const Identifier& contractID) const;
#if OT_CASH
    bool download_mint(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const DownloadMintTask& task) const;
#endif
    bool download_nym(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const identifier::Nym& targetNymID) const;
    bool download_nymbox(
        const TaskID taskID,
        api::client::internal::Operation& op) const;
    bool extract_payment_data(
        const OTPayment& payment,
        OTNymID& nymID,
        OTIdentifier& serverID,
        OTIdentifier& unitID) const;
    bool find_nym(
        api::client::internal::Operation& op,
        const identifier::Nym& targetNymID) const;
    bool find_server(
        api::client::internal::Operation& op,
        const identifier::Nym& targetNymID) const;
    bool finish_task(const TaskID taskID, const bool success, Result&& result)
        const;
    bool get_admin(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const OTPassword& password) const;
    std::future<void> get_future(const ContextID& id) const;
    OTIdentifier get_introduction_server(const Lock& lock) const;
    UniqueQueue<OTNymID>& get_nym_fetch(const Identifier& serverID) const;
    OperationQueue& get_operations(const ContextID& id) const;
    OperationQueue& get_queue(const ContextID& id) const;
    bool get_transaction_numbers(
        const TaskID taskID,
        api::client::internal::Operation& op) const;
    OTIdentifier import_default_introduction_server(const Lock& lock) const;
    bool initiate_peer_reply(
        const TaskID taskID,
        api::client::internal::Operation& op,
        PeerReplyTask& task) const;
    bool initiate_peer_request(
        const TaskID taskID,
        api::client::internal::Operation& op,
        PeerRequestTask& task) const;
    bool issue_unit_definition(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const Identifier& unitID,
        const std::string& label) const;
    void load_introduction_server(const Lock& lock) const;
    bool message_nym(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const MessageTask& task) const;
    bool pay_nym(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const identifier::Nym& recipient,
        std::shared_ptr<const OTPayment>& payment) const;
#if OT_CASH
    bool pay_nym_cash(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const identifier::Nym& recipient,
        const Identifier& workflowID) const;
#endif  // OT_CASH
    void process_account(
        const opentxs::network::zeromq::Message& message) const;
    bool process_inbox(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const Identifier& accountID) const;
    void process_notification(
        const opentxs::network::zeromq::Message& message) const;
    bool publish_server_contract(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const identifier::Server& serverID) const;
    bool publish_server_registration(
        const identifier::Nym& nymID,
        const Identifier& serverID,
        const bool forcePrimary) const;
    bool queue_cheque_deposit(
        const identifier::Nym& nymID,
        const Cheque& cheque) const;
    void refresh_accounts() const;
    void refresh_contacts() const;
    bool register_account(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const Identifier& unitID,
        const std::string& label) const;
    bool register_nym(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const bool resync) const;
    BackgroundTask schedule_download_nymbox(
        const identifier::Nym& localNymID,
        const Identifier& serverID) const;
    BackgroundTask schedule_register_account(
        const identifier::Nym& localNymID,
        const Identifier& serverID,
        const Identifier& unitID,
        const std::string& label) const;
    bool send_transfer(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const Identifier& sourceAccountID,
        const Identifier& targetAccountID,
        const Amount value,
        const std::string& memo) const;
    void set_contact(const identifier::Nym& nymID, const Identifier& serverID)
        const;
    OTIdentifier set_introduction_server(
        const Lock& lock,
        const ServerContract& contract) const;
    BackgroundTask start_task(const TaskID taskID, bool success) const;
    void state_machine(const ContextID id, OperationQueue& queue) const;
    ThreadStatus status(const Lock& lock, const TaskID taskID) const;
    void update_task(
        const TaskID taskID,
        const ThreadStatus status,
        Result&& result) const;
    void start_introduction_server(const identifier::Nym& nymID) const;
    Depositability valid_account(
        const OTPayment& payment,
        const Identifier& recipient,
        const Identifier& serverID,
        const Identifier& unitID,
        const Identifier& accountIDHint,
        OTIdentifier& depositAccount) const;
    bool valid_context(const identifier::Nym& nymID, const Identifier& serverID)
        const;
    Depositability valid_recipient(
        const OTPayment& payment,
        const identifier::Nym& specifiedNymID,
        const Identifier& recipient) const;
#if OT_CASH
    bool withdraw_cash(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const WithdrawCashTask& task) const;
#endif  // OT_CASH
    TaskDone write_and_send_cheque(
        const TaskID taskID,
        api::client::internal::Operation& op,
        const Identifier& accountID,
        const identifier::Nym& recipient,
        const Amount value,
        const std::string& memo,
        const Time validFrom,
        const Time validTo) const;

    OTX(const Flag& running,
        const api::client::Manager& client,
        OTClient& otclient,
        const ContextLockCallback& lockCallback);
    OTX() = delete;
    OTX(const OTX&) = delete;
    OTX(OTX&&) = delete;
    OTX& operator=(const OTX&) = delete;
    OTX& operator=(OTX&&) = delete;
};
}  // namespace opentxs::api::client::implementation
