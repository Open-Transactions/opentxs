// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ConsensusEnums.pb.h>
#include <Context.pb.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <utility>

#include "core/StateMachine.hpp"
#include "internal/network/ServerConnection.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/ManagedNumber.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/consensus/Base.hpp"
#include "otx/consensus/ServerPrivate.hpp"

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
class Account;
class Notary;
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
namespace context
{
class Server;
class TransactionStatement;
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
class OPENTXS_NO_EXPORT Server final : public internal::Server,
                                       public Base<Server, ServerPrivate>,
                                       public opentxs::internal::StateMachine
{
public:
    auto Accounts() const -> UnallocatedVector<identifier::Generic> final;
    auto AdminPassword() const -> const UnallocatedCString& final;
    auto AdminAttempted() const -> bool final;
    auto FinalizeServerCommand(Message& command, const PasswordPrompt& reason)
        const -> bool final;
    auto HaveAdminPassword() const -> bool final;
    auto HaveSufficientNumbers(const otx::MessageType reason) const
        -> bool final;
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
    auto Verify(const otx::context::TransactionStatement& statement) const
        -> bool final;
    auto VerifyTentativeNumber(const TransactionNumber& number) const
        -> bool final;

    auto AcceptIssuedNumber(const TransactionNumber& number) -> bool final;
    auto AcceptIssuedNumbers(
        const otx::context::TransactionStatement& statement) -> bool final;
    auto AddTentativeNumber(const TransactionNumber& number) -> bool final;
    auto CloseCronItem(const TransactionNumber) -> bool final { return false; }
    auto Connection() -> network::ServerConnection& final;
    auto InitializeServerCommand(
        const otx::MessageType type,
        const Armored& payload,
        const identifier::Account& accountID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = true)
        -> std::pair<RequestNumber, std::unique_ptr<Message>> final;
    auto InitializeServerCommand(
        const otx::MessageType type,
        const identifier::Nym& recipientNymID,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false)
        -> std::pair<RequestNumber, std::unique_ptr<Message>> final;
    auto InitializeServerCommand(
        const otx::MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments = true,
        const bool withNymboxHash = false)
        -> std::pair<RequestNumber, std::unique_ptr<Message>> final;
    auto Join() const -> void final;
    auto mutable_Purse(
        const identifier::UnitDefinition& id,
        const PasswordPrompt& reason)
        -> Editor<blind::Purse, std::shared_mutex> final;
    auto NextTransactionNumber(const otx::MessageType reason)
        -> otx::context::ManagedNumber final;
    auto OpenCronItem(const TransactionNumber) -> bool final { return false; }
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
    using Base<Server, ServerPrivate>::Request;
    auto Request(const ServerPrivate& data) const -> RequestNumber final;
    auto ResetThread() -> void final;
    auto Resync(const proto::Context& serialized) -> bool final;
    auto SendMessage(
        const api::session::Client& client,
        const UnallocatedSet<otx::context::ManagedNumber>& pending,
        otx::context::Server&,
        const Message& message,
        const PasswordPrompt& reason,
        const UnallocatedCString& label,
        const bool resync) -> client::NetworkReplyMessage final;
    auto SetAdminAttempted() -> void final;
    auto SetAdminPassword(const UnallocatedCString& password) -> void final;
    auto SetAdminSuccess() -> void final;
    auto SetHighest(const TransactionNumber& highest) -> bool final;
    auto SetPush(const bool on) -> void final;
    auto SetRevision(const std::uint64_t revision) -> void final;
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
    friend Base<Server, ServerPrivate>;
    friend ServerPrivate;

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

    static const UnallocatedSet<otx::MessageType> do_not_need_request_number_;

    GuardedData data_;

    static auto client(const api::Session& api) -> const api::session::Client&;
    static auto extract_numbers(OTTransaction& input) -> TransactionNumbers;
    static auto get_item_type(OTTransaction& input, otx::itemType& output)
        -> Exit;
    static auto get_type(const std::int64_t depth) -> BoxType;
    static auto instantiate_message(
        const api::Session& api,
        const UnallocatedCString& serialized)
        -> std::unique_ptr<opentxs::Message>;
    static auto need_request_number(const otx::MessageType type) -> bool;
    static auto scan_number_set(
        const TransactionNumbers& input,
        TransactionNumber& highest,
        TransactionNumber& lowest) -> void;
    static auto validate_number_set(
        const TransactionNumbers& input,
        const TransactionNumber limit,
        TransactionNumbers& good,
        TransactionNumbers& bad) -> void;

    auto add_item_to_payment_inbox(
        const TransactionNumber number,
        const UnallocatedCString& payment,
        const PasswordPrompt& reason) const -> bool;
    auto add_item_to_workflow(
        const Data& data,
        const api::session::Client& client,
        const Message& transportItem,
        const UnallocatedCString& item,
        const PasswordPrompt& reason) const -> bool;
    auto add_transaction_to_ledger(
        const TransactionNumber number,
        std::shared_ptr<OTTransaction> transaction,
        Ledger& ledger,
        const PasswordPrompt& reason) const -> bool;
    auto client_nym_id() const -> const identifier::Nym& final;
    auto create_instrument_notice_from_peer_object(
        const Data& data,
        const api::session::Client& client,
        const Message& message,
        const PeerObject& peerObject,
        const TransactionNumber number,
        const PasswordPrompt& reason) const -> bool;
    auto extract_box_receipt(
        const String& serialized,
        const identity::Nym& signer,
        const identifier::Nym& owner,
        const TransactionNumber target) const -> std::shared_ptr<OTTransaction>;
    auto extract_ledger(
        const Armored& armored,
        const identifier::Account& accountID,
        const identity::Nym& signer) const -> std::unique_ptr<Ledger>;
    auto extract_message(const Armored& armored, const identity::Nym& signer)
        const -> std::unique_ptr<Message>;
    auto extract_original_item(const Item& response) const
        -> std::unique_ptr<Item>;
    auto extract_original_item(
        const Data& data,
        const otx::itemType type,
        OTTransaction& response) const -> std::unique_ptr<Item>;
    auto extract_payment_instrument_from_notice(
        const api::Session& api,
        const identity::Nym& theNym,
        std::shared_ptr<OTTransaction> pTransaction,
        const PasswordPrompt& reason) const -> std::shared_ptr<OTPayment>;
    auto extract_transfer(const OTTransaction& receipt) const
        -> std::unique_ptr<Item>;
    auto extract_transfer_pending(const OTTransaction& receipt) const
        -> std::unique_ptr<Item>;
    auto extract_transfer_receipt(const OTTransaction& receipt) const
        -> std::unique_ptr<Item>;
    auto finalize_server_command(Message& command, const PasswordPrompt& reason)
        const -> bool;
    auto generate_statement(
        const Data& data,
        const TransactionNumbers& adding,
        const TransactionNumbers& without) const
        -> std::unique_ptr<otx::context::TransactionStatement>;
    auto get_instrument(
        const api::Session& api,
        const identity::Nym& theNym,
        Ledger& ledger,
        std::shared_ptr<OTTransaction> pTransaction,
        const PasswordPrompt& reason) const -> std::shared_ptr<OTPayment>;
    auto get_instrument_by_receipt_id(
        const api::Session& api,
        const identity::Nym& theNym,
        const TransactionNumber lReceiptId,
        Ledger& ledger,
        const PasswordPrompt& reason) const -> std::shared_ptr<OTPayment>;
    auto initialize_server_command(const otx::MessageType type) const
        -> std::unique_ptr<Message>;
    auto initialize_server_command(const otx::MessageType type, Message& output)
        const -> void;
    auto is_internal_transfer(const Item& item) const -> bool;
    auto load_account_inbox(const identifier::Account& accountID) const
        -> std::unique_ptr<Ledger>;
    auto load_or_create_account_recordbox(
        const identifier::Account& accountID,
        const PasswordPrompt& reason) const -> std::unique_ptr<Ledger>;
    auto load_or_create_payment_inbox(const PasswordPrompt& reason) const
        -> std::unique_ptr<Ledger>;
    auto make_accept_item(
        const PasswordPrompt& reason,
        const otx::itemType type,
        const OTTransaction& input,
        OTTransaction& acceptTransaction,
        const TransactionNumbers& accept = {}) const -> const Item&;
    auto nym_to_account(const identifier::Nym& id) const noexcept
        -> identifier::Account;
    auto process_accept_cron_receipt_reply(
        const identifier::Account& accountID,
        OTTransaction& inboxTransaction) const -> void;
    auto process_accept_pending_reply(
        const api::session::Client& client,
        const identifier::Account& accountID,
        const Item& acceptItemReceipt,
        const Message& reply) const -> void;
    auto process_get_market_list_response(const Message& reply) const -> bool;
    auto process_get_market_offers_response(const Message& reply) const -> bool;
    auto process_get_market_recent_trades_response(const Message& reply) const
        -> bool;
    auto process_get_mint_response(const Message& reply) const -> bool;
    auto process_get_nym_market_offers_response(const Message& reply) const
        -> bool;
    auto process_incoming_cash(
        const api::session::Client& client,
        const TransactionNumber number,
        const PeerObject& incoming,
        const Message& message) const -> bool;
    auto process_incoming_cash_withdrawal(
        const Item& item,
        const PasswordPrompt& reason) const -> void;
    auto process_incoming_instrument(
        const std::shared_ptr<OTTransaction> receipt,
        const PasswordPrompt& reason) const -> void;
    auto process_incoming_message(
        const Data& data,
        const api::session::Client& client,
        const OTTransaction& receipt,
        const PasswordPrompt& reason) const -> void;
    auto process_unregister_account_response(
        const Message& reply,
        const PasswordPrompt& reason) const -> bool;
    using Base<Server, ServerPrivate>::serialize;
    auto serialize(const Data& data) const -> proto::Context final;
    auto server_nym_id() const -> const identifier::Nym& final;
    auto statement(
        const Data& data,
        const OTTransaction& owner,
        const TransactionNumbers& adding,
        const PasswordPrompt& reason) const -> std::unique_ptr<Item>;
    auto type() const -> UnallocatedCString final { return "server"; }
    auto verify_blank(
        const Data& data,
        OTTransaction& blank,
        TransactionNumbers& output) const -> void;
    auto verify_success(
        const Data& data,
        OTTransaction& blank,
        TransactionNumbers& output) const -> void;
    auto verify_tentative_number(
        const Data& data,
        const TransactionNumber& number) const -> bool;

    auto accept_entire_nymbox(
        Data& data,
        const api::session::Client& client,
        Ledger& theNymbox,
        Message& output,
        ReplyNoticeOutcomes& notices,
        std::size_t& alreadySeenNotices,
        const PasswordPrompt& reason) -> bool;
    auto accept_issued_number(Data& data, const TransactionNumber& number)
        -> bool;
    auto accept_issued_number(
        Data& data,
        const otx::context::TransactionStatement& statement) -> bool;
    auto accept_numbers(
        Data& data,
        OTTransaction& transaction,
        OTTransaction& replyTransaction) -> void;
    auto add_tentative_number(Data& data, const TransactionNumber& number)
        -> bool;
    auto attempt_delivery(
        Data& data,
        const Lock& messageLock,
        const api::session::Client& client,
        Message& message,
        const PasswordPrompt& reason) -> client::NetworkReplyMessage;
    auto harvest_unused(Data& data, const api::session::Client& client) -> bool;
    auto init_new_account(
        const identifier::Account& accountID,
        const PasswordPrompt& reason) -> bool;
    auto init_sockets(Data& data) -> void;
    auto initialize_server_command(
        Data& data,
        const otx::MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash)
        -> std::pair<RequestNumber, std::unique_ptr<Message>>;
    auto initialize_server_command(
        Data& data,
        const otx::MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash,
        Message& output) -> RequestNumber;
    auto need_box_items(
        Data& data,
        const api::session::Client& client,
        const PasswordPrompt& reason) -> void;
    auto need_nymbox(
        Data& data,
        const api::session::Client& client,
        const PasswordPrompt& reason) -> void;
    auto need_process_nymbox(
        Data& data,
        const api::session::Client& client,
        const PasswordPrompt& reason) -> void;
    auto next_transaction_number(Data& data, const otx::MessageType reason)
        -> otx::context::ManagedNumber;
    auto pending_send(
        Data& data,
        const api::session::Client& client,
        const PasswordPrompt& reason) -> void;
    auto process_accept_basket_receipt_reply(
        Data& data,
        const OTTransaction& inboxTransaction) -> void;
    auto process_accept_final_receipt_reply(
        Data& data,
        const OTTransaction& inboxTransaction) -> void;
    auto process_accept_item_receipt_reply(
        Data& data,
        const api::session::Client& client,
        const identifier::Account& accountID,
        const Message& reply,
        const OTTransaction& inboxTransaction) -> void;
    auto process_account_data(
        Data& data,
        const identifier::Account& accountID,
        const String& account,
        const identifier::Generic& inboxHash,
        const String& inbox,
        const identifier::Generic& outboxHash,
        const String& outbox,
        const PasswordPrompt& reason) -> bool;
    auto process_account_push(
        Data& data,
        const api::session::Client& client,
        const proto::OTXPush& push,
        const PasswordPrompt& reason) -> bool;
    auto process_box_item(
        Data& data,
        const api::session::Client& client,
        const identifier::Account& accountID,
        const proto::OTXPush& push,
        const PasswordPrompt& reason) -> bool;
    auto process_check_nym_response(
        Data& data,
        const api::session::Client& client,
        const Message& reply) -> bool;
    auto process_get_account_data(
        Data& data,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_get_box_receipt_response(
        Data& data,
        const api::session::Client& client,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_get_box_receipt_response(
        Data& data,
        const api::session::Client& client,
        const identifier::Account& accountID,
        const std::shared_ptr<OTTransaction> receipt,
        const String& serialized,
        const BoxType type,
        const PasswordPrompt& reason) -> bool;
    auto process_get_nymbox_response(
        Data& data,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_get_unit_definition_response(Data& data, const Message& reply)
        -> bool;
    auto process_issue_unit_definition_response(
        Data& data,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_notarize_transaction_response(
        Data& data,
        const api::session::Client& client,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_process_box_response(
        Data& data,
        const api::session::Client& client,
        const Message& reply,
        const BoxType inbox,
        const identifier::Account& accountID,
        const PasswordPrompt& reason) -> bool;
    auto process_process_inbox_response(
        Data& data,
        const api::session::Client& client,
        const Message& reply,
        Ledger& ledger,
        Ledger& responseLedger,
        std::shared_ptr<OTTransaction>& transaction,
        std::shared_ptr<OTTransaction>& replyTransaction,
        const PasswordPrompt& reason) -> bool;
    auto process_process_nymbox_response(
        Data& data,
        const Message& reply,
        Ledger& ledger,
        Ledger& responseLedger,
        std::shared_ptr<OTTransaction>& transaction,
        std::shared_ptr<OTTransaction>& replyTransaction,
        const PasswordPrompt& reason) -> bool;
    auto process_register_account_response(
        Data& data,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_register_nym_response(
        Data& data,
        const api::session::Client& client,
        const Message& reply) -> bool;
    auto process_reply(
        Data& data,
        const api::session::Client& client,
        const UnallocatedSet<otx::context::ManagedNumber>& managed,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_request_admin_response(Data& data, const Message& reply)
        -> bool;
    auto process_response_transaction(
        Data& data,
        const api::session::Client& client,
        const Message& reply,
        OTTransaction& responseTransaction,
        const PasswordPrompt& reason) -> void;
    auto process_response_transaction_cancel(
        Data& data,
        const Message& reply,
        const otx::itemType type,
        OTTransaction& response) -> void;
    auto process_response_transaction_cash_deposit(
        Item& replyItem,
        const PasswordPrompt& reason) -> void;
    auto process_response_transaction_cheque_deposit(
        Data& data,
        const api::session::Client& client,
        const identifier::Account& accountID,
        const Message* reply,
        const Item& replyItem,
        const PasswordPrompt& reason) -> void;
    auto process_response_transaction_cron(
        Data& data,
        const Message& reply,
        const otx::itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason) -> void;
    auto process_response_transaction_deposit(
        Data& data,
        const api::session::Client& client,
        const Message& reply,
        const otx::itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason) -> void;
    auto process_response_transaction_exchange_basket(
        Data& data,
        const Message& reply,
        const otx::itemType type,
        OTTransaction& response) -> void;
    auto process_response_transaction_pay_dividend(
        Data& data,
        const Message& reply,
        const otx::itemType type,
        OTTransaction& response) -> void;
    auto process_response_transaction_transfer(
        Data& data,
        const api::session::Client& client,
        const Message& reply,
        const otx::itemType type,
        OTTransaction& response) -> void;
    auto process_response_transaction_withdrawal(
        Data& data,
        const api::session::Client& client,
        const Message& reply,
        const otx::itemType type,
        OTTransaction& response,
        const PasswordPrompt& reason) -> void;
    auto process_unregister_nym_response(
        Data& data,
        const Message& reply,
        const PasswordPrompt& reason) -> bool;
    auto process_unseen_reply(
        Data& data,
        const api::session::Client& client,
        const Item& input,
        ReplyNoticeOutcomes& notices,
        const PasswordPrompt& reason) -> void;
    using Base<Server, ServerPrivate>::remove_acknowledged_number;
    auto remove_acknowledged_number(Data& data, const Message& reply) -> bool;
    auto remove_nymbox_item(
        Data& data,
        const Item& replyItem,
        Ledger& nymbox,
        OTTransaction& transaction,
        const PasswordPrompt& reason) -> bool;
    auto remove_tentative_number(Data& data, const TransactionNumber& number)
        -> bool;
    auto resolve_queue(
        Data& data,
        DeliveryResult&& result,
        const PasswordPrompt& reason,
        const proto::DeliveryState state = proto::DELIVERTYSTATE_ERROR) -> void;
    auto resync(Data& data, const proto::Context& serialized) -> bool;
    auto start(
        Data& data,
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
    auto state_machine(Data& data) noexcept -> bool;
    auto update_highest(
        Data& data,
        const TransactionNumbers& numbers,
        TransactionNumbers& good,
        TransactionNumbers& bad) -> TransactionNumber;
    auto update_nymbox_hash(
        Data& data,
        const Message& reply,
        const UpdateHash which = UpdateHash::Remote) -> bool;
    auto update_remote_hash(Data& data, const Message& reply)
        -> identifier::Generic;
    auto update_request_number(
        Data& data,
        const PasswordPrompt& reason,
        Message& command) -> bool;
    auto update_request_number(
        Data& data,
        const PasswordPrompt& reason,
        const Lock& messageLock,
        bool& sendStatus) -> RequestNumber;
    auto update_state(
        Data& data,
        const proto::DeliveryState state,
        const PasswordPrompt& reason,
        const otx::LastReplyStatus status = otx::LastReplyStatus::Invalid)
        -> void;
};
}  // namespace opentxs::otx::context::implementation
