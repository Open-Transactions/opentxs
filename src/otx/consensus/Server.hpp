// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ConsensusEnums.pb.h>
#include <Context.pb.h>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>

#include "core/StateMachine.hpp"
#include "internal/network/ServerConnection.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/consensus/Base.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/ManagedNumber.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/otx/consensus/TransactionStatement.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/ConsensusType.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/consensus/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind

namespace context
{
class Server;
}  // namespace context

class Reply;
}  // namespace otx

namespace proto
{
class OTXPush;
}  // namespace proto

class Armored;
class Ledger;
class OTPayment;
class PasswordPrompt;
class PeerObject;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::otx::context::implementation
{
class Server final : virtual public internal::Server,
                     public Base,
                     public opentxs::internal::StateMachine
{
public:
    auto Accounts() const -> UnallocatedVector<identifier::Generic> final;
    auto AdminPassword() const -> const UnallocatedCString& final;
    auto AdminAttempted() const -> bool final;
    auto FinalizeServerCommand(Message& command, const PasswordPrompt& reason)
        const -> bool final;
    auto GetContract(const Lock& lock) const -> proto::Context final
    {
        return contract(lock);
    }
    auto HaveAdminPassword() const -> bool final;
    auto HaveSufficientNumbers(const MessageType reason) const -> bool final;
    auto Highest() const -> TransactionNumber final;
    auto isAdmin() const -> bool final;
    auto Purse(const identifier::UnitDefinition& id) const
        -> const otx::blind::Purse& final;
    auto Revision() const -> std::uint64_t final;
    auto ShouldRename(const UnallocatedCString& defaultName = "") const
        -> bool final;
    auto StaleNym() const -> bool final;
    auto Statement(const OTTransaction& owner, const PasswordPrompt& reason)
        const -> std::unique_ptr<Item> final;
    auto Statement(
        const OTTransaction& owner,
        const TransactionNumbers& adding,
        const PasswordPrompt& reason) const -> std::unique_ptr<Item> final;
    auto Statement(
        const TransactionNumbers& adding,
        const TransactionNumbers& without,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<otx::context::TransactionStatement> final;
    auto Type() const -> otx::ConsensusType final;
    auto ValidateContext(const Lock& lock) const -> bool final
    {
        return validate(lock);
    }
    auto Verify(const otx::context::TransactionStatement& statement) const
        -> bool final;
    auto VerifyTentativeNumber(const TransactionNumber& number) const
        -> bool final;

    auto AcceptIssuedNumber(const TransactionNumber& number) -> bool final;
    auto AcceptIssuedNumbers(
        const otx::context::TransactionStatement& statement) -> bool final;
    auto AddTentativeNumber(const TransactionNumber& number) -> bool final;
    auto Connection() -> network::ServerConnection& final;
    auto GetLock() -> std::mutex& final { return lock_; }
    auto InitializeServerCommand(
        const MessageType type,
        const Armored& payload,
        const identifier::Generic& accountID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = true)
        -> std::pair<RequestNumber, std::unique_ptr<Message>> final;
    auto InitializeServerCommand(
        const MessageType type,
        const identifier::Nym& recipientNymID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false)
        -> std::pair<RequestNumber, std::unique_ptr<Message>> final;
    auto InitializeServerCommand(
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false)
        -> std::pair<RequestNumber, std::unique_ptr<Message>> final;
    void Join() const final;
    auto mutable_Purse(
        const identifier::UnitDefinition& id,
        const PasswordPrompt& reason)
        -> Editor<blind::Purse, std::shared_mutex> final;
    auto NextTransactionNumber(const MessageType reason)
        -> otx::context::ManagedNumber final;
    auto PingNotary(const PasswordPrompt& reason)
        -> client::NetworkReplyMessage final;
    auto ProcessNotification(
        const api::session::Client& client,
        const otx::Reply& notification,
        const PasswordPrompt& reason) -> bool final;
    auto Queue(
        const api::session::Client& client,
        std::shared_ptr<Message> message,
        const PasswordPrompt& reason,
        const ExtraArgs& args) -> QueueResult final;
    auto Queue(
        const api::session::Client& client,
        std::shared_ptr<Message> message,
        std::shared_ptr<Ledger> inbox,
        std::shared_ptr<Ledger> outbox,
        UnallocatedSet<otx::context::ManagedNumber>* numbers,
        const PasswordPrompt& reason,
        const ExtraArgs& args) -> QueueResult final;
    auto RefreshNymbox(
        const api::session::Client& client,
        const PasswordPrompt& reason) -> QueueResult final;
    auto RemoveTentativeNumber(const TransactionNumber& number) -> bool final;
    void ResetThread() final;
    auto Resync(const proto::Context& serialized) -> bool final;
    auto SendMessage(
        const api::session::Client& client,
        const UnallocatedSet<otx::context::ManagedNumber>& pending,
        otx::context::Server&,
        const Message& message,
        const PasswordPrompt& reason,
        const UnallocatedCString& label,
        const bool resync) -> client::NetworkReplyMessage final;
    void SetAdminAttempted() final;
    void SetAdminPassword(const UnallocatedCString& password) final;
    void SetAdminSuccess() final;
    auto SetHighest(const TransactionNumber& highest) -> bool final;
    void SetPush(const bool on) final { enable_otx_push_.store(on); }
    void SetRevision(const std::uint64_t revision) final;
    auto UpdateHighest(
        const TransactionNumbers& numbers,
        TransactionNumbers& good,
        TransactionNumbers& bad) -> TransactionNumber final;
    auto UpdateRequestNumber(const PasswordPrompt& reason)
        -> RequestNumber final;
    auto UpdateRequestNumber(bool& sendStatus, const PasswordPrompt& reason)
        -> RequestNumber final;
    auto UpdateRequestNumber(Message& command, const PasswordPrompt& reason)
        -> bool final;
    auto UpdateSignature(const Lock& lock, const PasswordPrompt& reason)
        -> bool final
    {
        return update_signature(lock, reason);
    }

    Server(
        const api::session::Client& api,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        const Nym_p& local,
        const Nym_p& remote,
        const identifier::Notary& server,
        network::ServerConnection& connection);
    Server(
        const api::session::Client& api,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        const proto::Context& serialized,
        const Nym_p& local,
        const Nym_p& remote,
        network::ServerConnection& connection);
    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;

    ~Server() final;

private:
    using ReplyNoticeOutcome = std::pair<RequestNumber, Server::DeliveryResult>;
    using ReplyNoticeOutcomes = UnallocatedVector<ReplyNoticeOutcome>;

    enum class Exit : bool { Yes = true, Continue = false };
    enum class UpdateHash : bool { Remote = false, Both = true };
    enum class BoxType : std::int64_t {
        Invalid = -1,
        Nymbox = 0,
        Inbox = 1,
        Outbox = 2
    };
    enum class ActionType : bool { ProcessNymbox = true, Normal = false };
    enum class TransactionAttempt : bool { Accepted = true, Rejected = false };

    static constexpr auto current_version_ = VersionNumber{3};
    static constexpr auto pending_command_version_ = VersionNumber{1};
    static constexpr auto default_node_name_{"Remote Notary"};
    static constexpr auto nymbox_box_type_{0};
    static constexpr auto failure_count_limit_{3};

    static const UnallocatedSet<MessageType> do_not_need_request_number_;

    const network::zeromq::socket::Publish& request_sent_;
    const network::zeromq::socket::Publish& reply_received_;
    // WARNING the lifetime of the object pointed to by this member variable
    // has a shorter lifetime than this ServerContext object. Call Join()
    // on all ServerContext objects before allowing the client api to shut down.
    std::atomic<const api::session::Client*> client_;
    network::ServerConnection& connection_;
    std::mutex message_lock_{};
    UnallocatedCString admin_password_{""};
    OTFlag admin_attempted_;
    OTFlag admin_success_;
    std::atomic<std::uint64_t> revision_{0};
    std::atomic<TransactionNumber> highest_transaction_number_{0};
    TransactionNumbers tentative_transaction_numbers_{};
    std::atomic<proto::DeliveryState> state_;
    std::atomic<otx::LastReplyStatus> last_status_;
    std::shared_ptr<opentxs::Message> pending_message_;
    ExtraArgs pending_args_;
    std::promise<DeliveryResult> pending_result_;
    std::atomic<bool> pending_result_set_;
    std::atomic<bool> process_nymbox_;
    std::atomic<bool> enable_otx_push_;
    std::atomic<int> failure_counter_;
    std::shared_ptr<Ledger> inbox_;
    std::shared_ptr<Ledger> outbox_;
    UnallocatedSet<otx::context::ManagedNumber>* numbers_;
    OTZMQPushSocket find_nym_;
    OTZMQPushSocket find_server_;
    OTZMQPushSocket find_unit_definition_;

    static auto client(const api::Session& api) -> const api::session::Client&;
    static auto extract_numbers(OTTransaction& input) -> TransactionNumbers;
    static auto get_item_type(OTTransaction& input, itemType& output) -> Exit;
    static auto get_type(const std::int64_t depth) -> BoxType;
    static auto instantiate_message(
        const api::Session& api,
        const UnallocatedCString& serialized)
        -> std::unique_ptr<opentxs::Message>;
    static auto need_request_number(const MessageType type) -> bool;
    static void scan_number_set(
        const TransactionNumbers& input,
        TransactionNumber& highest,
        TransactionNumber& lowest);
    static void validate_number_set(
        const TransactionNumbers& input,
        const TransactionNumber limit,
        TransactionNumbers& good,
        TransactionNumbers& bad);

    auto add_item_to_payment_inbox(
        const TransactionNumber number,
        const UnallocatedCString& payment,
        const PasswordPrompt& reason) const -> bool;
    auto add_item_to_workflow(
        const Lock& lock,
        const api::session::Client& client,
        const Message& transportItem,
        const UnallocatedCString& item,
        const PasswordPrompt& reason) const -> bool;
    auto add_transaction_to_ledger(
        const TransactionNumber number,
        std::shared_ptr<OTTransaction> transaction,
        Ledger& ledger,
        const PasswordPrompt& reason) const -> bool;
    auto client_nym_id(const Lock& lock) const -> const identifier::Nym& final;
    auto create_instrument_notice_from_peer_object(
        const Lock& lock,
        const api::session::Client& client,
        const Message& message,
        const PeerObject& peerObject,
        const TransactionNumber number,
        const PasswordPrompt& reason) const -> bool;
    auto extract_box_receipt(
        const String& serialized,
        const identity::Nym& signer,
        const identifier::Nym& owner,
        const TransactionNumber target) -> std::shared_ptr<OTTransaction>;
    auto extract_ledger(
        const Armored& armored,
        const identifier::Generic& accountID,
        const identity::Nym& signer) const -> std::unique_ptr<Ledger>;
    auto extract_message(const Armored& armored, const identity::Nym& signer)
        const -> std::unique_ptr<Message>;
    auto extract_original_item(const itemType type, OTTransaction& response)
        const -> std::unique_ptr<Item>;
    auto extract_original_item(const Item& response) const
        -> std::unique_ptr<Item>;
    auto extract_payment_instrument_from_notice(
        const api::Session& api,
        const identity::Nym& theNym,
        std::shared_ptr<OTTransaction> pTransaction,
        const PasswordPrompt& reason) -> std::shared_ptr<OTPayment>;
    auto extract_transfer(const OTTransaction& receipt) const
        -> std::unique_ptr<Item>;
    auto extract_transfer_pending(const OTTransaction& receipt) const
        -> std::unique_ptr<Item>;
    auto extract_transfer_receipt(const OTTransaction& receipt) const
        -> std::unique_ptr<Item>;
    auto finalize_server_command(Message& command, const PasswordPrompt& reason)
        const -> bool;
    auto generate_statement(
        const Lock& lock,
        const TransactionNumbers& adding,
        const TransactionNumbers& without) const
        -> std::unique_ptr<otx::context::TransactionStatement>;
    auto get_instrument(
        const api::Session& api,
        const identity::Nym& theNym,
        Ledger& ledger,
        std::shared_ptr<OTTransaction> pTransaction,
        const PasswordPrompt& reason) -> std::shared_ptr<OTPayment>;
    auto get_instrument_by_receipt_id(
        const api::Session& api,
        const identity::Nym& theNym,
        const TransactionNumber lReceiptId,
        Ledger& ledger,
        const PasswordPrompt& reason) -> std::shared_ptr<OTPayment>;
    auto init_new_account(
        const identifier::Generic& accountID,
        const PasswordPrompt& reason) -> bool;
    auto initialize_server_command(const MessageType type) const
        -> std::unique_ptr<Message>;
    void initialize_server_command(const MessageType type, Message& output)
        const;
    auto is_internal_transfer(const Item& item) const -> bool;
    auto load_account_inbox(const identifier::Generic& accountID) const
        -> std::unique_ptr<Ledger>;
    auto load_or_create_account_recordbox(
        const identifier::Generic& accountID,
        const PasswordPrompt& reason) const -> std::unique_ptr<Ledger>;
    auto load_or_create_payment_inbox(const PasswordPrompt& reason) const
        -> std::unique_ptr<Ledger>;
    void process_accept_pending_reply(
        const Lock& lock,
        const api::session::Client& client,
        const identifier::Generic& accountID,
        const Item& acceptItemReceipt,
        const Message& reply) const;
    auto process_incoming_cash(
        const Lock& lock,
        const api::session::Client& client,
        const TransactionNumber number,
        const PeerObject& incoming,
        const Message& message) const -> bool;
    void process_incoming_cash_withdrawal(
        const Item& item,
        const PasswordPrompt& reason) const;
    void process_incoming_instrument(
        const std::shared_ptr<OTTransaction> receipt,
        const PasswordPrompt& reason) const;
    void process_incoming_message(
        const Lock& lock,
        const api::session::Client& client,
        const OTTransaction& receipt,
        const PasswordPrompt& reason) const;
    auto type() const -> UnallocatedCString final { return "server"; }
    void verify_blank(
        const Lock& lock,
        OTTransaction& blank,
        TransactionNumbers& output) const;
    void verify_success(
        const Lock& lock,
        OTTransaction& blank,
        TransactionNumbers& output) const;

    auto accept_entire_nymbox(
        const Lock& lock,
        const api::session::Client& client,
        Ledger& theNymbox,
        Message& output,
        ReplyNoticeOutcomes& notices,
        std::size_t& alreadySeenNotices,
        const PasswordPrompt& reason) -> bool;
    auto accept_issued_number(const Lock& lock, const TransactionNumber& number)
        -> bool;
    auto accept_issued_number(
        const Lock& lock,
        const otx::context::TransactionStatement& statement) -> bool;
    void accept_numbers(
        const Lock& lock,
        OTTransaction& transaction,
        OTTransaction& replyTransaction);
    auto add_tentative_number(const Lock& lock, const TransactionNumber& number)
        -> bool;
    auto attempt_delivery(
        const Lock& contextLock,
        const Lock& messageLock,
        const api::session::Client& client,
        Message& message,
        const PasswordPrompt& reason) -> client::NetworkReplyMessage;
    auto harvest_unused(const Lock& lock, const api::session::Client& client)
        -> bool;
    void init_sockets();
    auto initialize_server_command(
        const Lock& lock,
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash,
        Message& output) -> RequestNumber;
    auto initialize_server_command(
        const Lock& lock,
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash)
        -> std::pair<RequestNumber, std::unique_ptr<Message>>;
    auto make_accept_item(
        const PasswordPrompt& reason,
        const itemType type,
        const OTTransaction& input,
        OTTransaction& acceptTransaction,
        const TransactionNumbers& accept = {}) -> const Item&;
    void need_box_items(
        const api::session::Client& client,
        const PasswordPrompt& reason);
    void need_nymbox(
        const api::session::Client& client,
        const PasswordPrompt& reason);
    void need_process_nymbox(
        const api::session::Client& client,
        const PasswordPrompt& reason);
    auto next_transaction_number(const Lock& lock, const MessageType reason)
        -> otx::context::ManagedNumber;
    void pending_send(
        const api::session::Client& client,
        const PasswordPrompt& reason);
    void process_accept_basket_receipt_reply(
        const Lock& lock,
        const OTTransaction& inboxTransaction);
    void process_accept_cron_receipt_reply(
        const Lock& lock,
        const identifier::Generic& accountID,
        OTTransaction& inboxTransaction);
    void process_accept_final_receipt_reply(
        const Lock& lock,
        const OTTransaction& inboxTransaction);
    void process_accept_item_receipt_reply(
        const Lock& lock,
        const api::session::Client& client,
        const identifier::Generic& accountID,
        const Message& reply,
        const OTTransaction& inboxTransaction);
    auto process_account_data(
        const Lock& lock,
        const identifier::Generic& accountID,
        const String& account,
        const identifier::Generic& inboxHash,
        const String& inbox,
        const identifier::Generic& outboxHash,
        const String& outbox,
        const PasswordPrompt& reason) -> bool;
    auto process_account_push(
        const Lock& lock,
        const api::session::Client& client,
        const proto::OTXPush& push,
        const PasswordPrompt& reason) -> bool;
    auto process_box_item(
        const Lock& lock,
        const api::session::Client& client,
        const identifier::Generic& accountID,
        const proto::OTXPush& push,
        const PasswordPrompt& reason) -> bool;
    auto process_check_nym_response(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply) -> bool;
    auto process_get_account_data(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_get_box_receipt_response(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_get_box_receipt_response(
        const Lock& lock,
        const api::session::Client& client,
        const identifier::Generic& accountID,
        const std::shared_ptr<OTTransaction> receipt,
        const String& serialized,
        const BoxType type,
        const PasswordPrompt& reason) -> bool;
    auto process_get_market_list_response(
        const Lock& lock,
        const Message& reply) -> bool;
    auto process_get_market_offers_response(
        const Lock& lock,
        const Message& reply) -> bool;
    auto process_get_market_recent_trades_response(
        const Lock& lock,
        const Message& reply) -> bool;
    auto process_get_mint_response(const Lock& lock, const Message& reply)
        -> bool;
    auto process_get_nym_market_offers_response(
        const Lock& lock,
        const Message& reply) -> bool;
    auto process_get_unit_definition_response(
        const Lock& lock,
        const Message& reply) -> bool;
    auto process_issue_unit_definition_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_get_nymbox_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_notarize_transaction_response(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_process_box_response(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply,
        const BoxType inbox,
        const identifier::Generic& accountID,
        const PasswordPrompt& reason) -> bool;
    auto process_process_inbox_response(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply,
        Ledger& ledger,
        Ledger& responseLedger,
        std::shared_ptr<OTTransaction>& transaction,
        std::shared_ptr<OTTransaction>& replyTransaction,
        const PasswordPrompt& reason) -> bool;
    auto process_process_nymbox_response(
        const Lock& lock,
        const Message& reply,
        Ledger& ledger,
        Ledger& responseLedger,
        std::shared_ptr<OTTransaction>& transaction,
        std::shared_ptr<OTTransaction>& replyTransaction,
        const PasswordPrompt& reason) -> bool;
    auto process_register_account_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_request_admin_response(const Lock& lock, const Message& reply)
        -> bool;
    auto process_register_nym_response(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply) -> bool;
    auto process_reply(
        const Lock& lock,
        const api::session::Client& client,
        const UnallocatedSet<otx::context::ManagedNumber>& managed,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    void process_response_transaction(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply,
        OTTransaction& responseTransaction,
        const PasswordPrompt& reason);
    void process_response_transaction_cancel(
        const Lock& lock,
        const Message& reply,
        const itemType type,
        OTTransaction& response);
    void process_response_transaction_cash_deposit(
        Item& replyItem,
        const PasswordPrompt& reason);
    void process_response_transaction_cheque_deposit(
        const api::session::Client& client,
        const identifier::Generic& accountID,
        const Message* reply,
        const Item& replyItem,
        const PasswordPrompt& reason);
    void process_response_transaction_cron(
        const Lock& lock,
        const Message& reply,
        const itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason);
    void process_response_transaction_deposit(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply,
        const itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason);
    void process_response_transaction_exchange_basket(
        const Lock& lock,
        const Message& reply,
        const itemType type,
        OTTransaction& response);
    void process_response_transaction_pay_dividend(
        const Lock& lock,
        const Message& reply,
        const itemType type,
        OTTransaction& response);
    void process_response_transaction_transfer(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply,
        const itemType type,
        OTTransaction& response);
    void process_response_transaction_withdrawal(
        const Lock& lock,
        const api::session::Client& client,
        const Message& reply,
        const itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason);
    auto process_unregister_nym_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_unregister_account_response(
        const Lock& lock,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    void process_unseen_reply(
        const Lock& lock,
        const api::session::Client& client,
        const Item& input,
        ReplyNoticeOutcomes& notices,
        const PasswordPrompt& reason);
    using Base::remove_acknowledged_number;
    auto remove_acknowledged_number(const Lock& lock, const Message& reply)
        -> bool;
    auto remove_nymbox_item(
        const Lock& lock,
        const Item& replyItem,
        Ledger& nymbox,
        OTTransaction& transaction,
        const PasswordPrompt& reason) -> bool;
    auto remove_tentative_number(
        const Lock& lock,
        const TransactionNumber& number) -> bool;
    void resolve_queue(
        const Lock& contextLock,
        DeliveryResult&& result,
        const PasswordPrompt& reason,
        const proto::DeliveryState state = proto::DELIVERTYSTATE_ERROR);
    auto resync(const Lock& lock, const proto::Context& serialized) -> bool;
    using Base::serialize;
    auto serialize(const Lock& lock) const -> proto::Context final;
    auto server_nym_id(const Lock& lock) const -> const identifier::Nym& final;
    auto start(
        const Lock& decisionLock,
        const PasswordPrompt& reason,
        const api::session::Client& client,
        std::shared_ptr<Message> message,
        const ExtraArgs& args,
        const proto::DeliveryState state = proto::DELIVERTYSTATE_PENDINGSEND,
        const ActionType type = ActionType::Normal,
        std::shared_ptr<Ledger> inbox = {},
        std::shared_ptr<Ledger> outbox = {},
        UnallocatedSet<otx::context::ManagedNumber>* numbers = nullptr)
        -> QueueResult;
    auto state_machine() noexcept -> bool;
    auto statement(
        const Lock& lock,
        const OTTransaction& owner,
        const TransactionNumbers& adding,
        const PasswordPrompt& reason) const -> std::unique_ptr<Item>;
    auto update_highest(
        const Lock& lock,
        const TransactionNumbers& numbers,
        TransactionNumbers& good,
        TransactionNumbers& bad) -> TransactionNumber;
    auto update_nymbox_hash(
        const Lock& lock,
        const Message& reply,
        const UpdateHash which = UpdateHash::Remote) -> bool;
    auto update_remote_hash(const Lock& lock, const Message& reply)
        -> identifier::Generic;
    auto update_request_number(
        const PasswordPrompt& reason,
        const Lock& contextLock,
        const Lock& messageLock,
        bool& sendStatus) -> RequestNumber;
    auto update_request_number(
        const PasswordPrompt& reason,
        const Lock& lock,
        Message& command) -> bool;
    void update_state(
        const Lock& contextLock,
        const proto::DeliveryState state,
        const PasswordPrompt& reason,
        const otx::LastReplyStatus status = otx::LastReplyStatus::Invalid);
    auto verify_tentative_number(
        const Lock& lock,
        const TransactionNumber& number) const -> bool;
};
}  // namespace opentxs::otx::context::implementation
