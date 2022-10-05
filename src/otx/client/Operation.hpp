// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/ContractType.hpp"

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <iosfwd>
#include <memory>
#include <optional>

#include "core/StateMachine.hpp"
#include "internal/otx/client/Client.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/otx/OperationType.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/otx/consensus/ManagedNumber.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

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

namespace otx
{
namespace blind
{
class Purse;
}  // namespace blind

namespace context
{
class Base;
class ManagedNumber;
}  // namespace context
}  // namespace otx

namespace proto
{
class UnitDefinition;
}  // namespace proto

class Armored;
class Cheque;
class Context;
class Factory;
class OTPayment;
class OTTransaction;
class PeerObject;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client::implementation
{
class Operation final : virtual public otx::client::internal::Operation,
                        public opentxs::internal::StateMachine
{
public:
    auto NymID() const -> const identifier::Nym& override { return nym_id_; }
    auto ServerID() const -> const identifier::Notary& override
    {
        return server_id_;
    }

    auto AddClaim(
        const identity::wot::claim::SectionType section,
        const identity::wot::claim::ClaimType type,
        const String& value,
        const bool primary) -> bool override;
    auto ConveyPayment(
        const identifier::Nym& recipient,
        const std::shared_ptr<const OTPayment> payment) -> bool override;
    auto DepositCash(
        const identifier::Generic& depositAccountID,
        blind::Purse&& purse) -> bool override;
    auto DepositCheque(
        const identifier::Generic& depositAccountID,
        const std::shared_ptr<Cheque> cheque) -> bool override;
    auto DownloadContract(
        const identifier::Generic& ID,
        const contract::Type type) -> bool override;
    auto GetFuture() -> Future override;
    auto IssueUnitDefinition(
        const std::shared_ptr<const proto::UnitDefinition> unitDefinition,
        const otx::context::Server::ExtraArgs& args) -> bool override;
    auto IssueUnitDefinition(
        const ReadView& unitDefinition,
        const otx::context::Server::ExtraArgs& args) -> bool override;
    void join() override;
    auto PublishContract(const identifier::Nym& id) -> bool override;
    auto PublishContract(const identifier::Notary& id) -> bool override;
    auto PublishContract(const identifier::UnitDefinition& id) -> bool override;
    auto RequestAdmin(const String& password) -> bool override;
    auto SendCash(
        const identifier::Nym& recipient,
        const identifier::Generic& workflowID) -> bool override;
    auto SendMessage(
        const identifier::Nym& recipient,
        const String& message,
        const SetID setID) -> bool override;
    auto SendPeerReply(
        const identifier::Nym& targetNymID,
        const OTPeerReply peerreply,
        const OTPeerRequest peerrequest) -> bool override;
    auto SendPeerRequest(
        const identifier::Nym& targetNymID,
        const OTPeerRequest peerrequest) -> bool override;
    auto SendTransfer(
        const identifier::Generic& sourceAccountID,
        const identifier::Generic& destinationAccountID,
        const Amount& amount,
        const String& memo) -> bool override;
    void SetPush(const bool on) override { enable_otx_push_.store(on); }
    void Shutdown() override;
    auto Start(
        const otx::OperationType type,
        const otx::context::Server::ExtraArgs& args) -> bool override;
    auto Start(
        const otx::OperationType type,
        const identifier::UnitDefinition& targetUnitID,
        const otx::context::Server::ExtraArgs& args) -> bool override;
    auto Start(
        const otx::OperationType type,
        const identifier::Nym& targetNymID,
        const otx::context::Server::ExtraArgs& args) -> bool override;
    auto UpdateAccount(const identifier::Generic& accountID) -> bool override;
    auto WithdrawCash(
        const identifier::Generic& accountID,
        const Amount& amount) -> bool override;

    Operation() = delete;
    Operation(const Operation&) = delete;
    Operation(Operation&&) = delete;
    auto operator=(const Operation&) -> Operation& = delete;
    auto operator=(Operation&&) -> Operation& = delete;

    ~Operation() override;

private:
    using Promise = std::promise<Result>;
    friend opentxs::Factory;

    enum class Category : int {
        Invalid = 0,
        Basic = 1,
        NymboxPost = 2,
        NymboxPre = 4,
        CreateAccount = 5,
        UpdateAccount = 6,
        Transaction = 7,
    };
    enum class State : int {
        Invalid,
        Idle,
        NymboxPre,
        TransactionNumbers,
        AccountPre,
        Execute,
        AccountPost,
        NymboxPost,
    };
    enum class BoxType : std::int32_t {
        Nymbox = 0,
        Inbox = 1,
        Outbox = 2,
    };

    static const UnallocatedMap<otx::OperationType, Category> category_;
    static const UnallocatedMap<otx::OperationType, std::size_t>
        transaction_numbers_;

    const api::session::Client& api_;
    const OTPasswordPrompt reason_;
    const identifier::Nym nym_id_;
    const identifier::Notary server_id_;
    std::atomic<otx::OperationType> type_;
    std::atomic<State> state_;
    std::atomic<bool> refresh_account_;
    otx::context::Server::ExtraArgs args_;
    std::shared_ptr<Message> message_;
    std::shared_ptr<Message> outmail_message_;
    std::atomic<bool> result_set_;
    std::atomic<bool> enable_otx_push_;
    Promise result_;
    identifier::Nym target_nym_id_;
    identifier::Notary target_server_id_;
    identifier::UnitDefinition target_unit_id_;
    contract::Type contract_type_;
    std::shared_ptr<const proto::UnitDefinition> unit_definition_;
    identifier::Generic account_id_;
    identifier::Generic generic_id_;
    Amount amount_;
    OTString memo_;
    bool bool_;
    identity::wot::claim::SectionType claim_section_;
    identity::wot::claim::ClaimType claim_type_;
    std::shared_ptr<Cheque> cheque_;
    std::shared_ptr<const OTPayment> payment_;
    std::shared_ptr<Ledger> inbox_;
    std::shared_ptr<Ledger> outbox_;
    std::optional<blind::Purse> purse_;
    UnallocatedSet<identifier::Generic> affected_accounts_;
    UnallocatedSet<identifier::Generic> redownload_accounts_;
    UnallocatedSet<otx::context::ManagedNumber> numbers_;
    std::atomic<std::size_t> error_count_;
    OTPeerReply peer_reply_;
    OTPeerRequest peer_request_;
    SetID set_id_;

    static auto check_future(otx::context::Server::SendFuture& future) -> bool;
    static void set_consensus_hash(
        const api::Session& api,
        OTTransaction& transaction,
        const otx::context::Base& context,
        const Account& account,
        const PasswordPrompt& reason);

    auto context() const -> Editor<otx::context::Server>;
    auto evaluate_transaction_reply(
        const identifier::Generic& accountID,
        const Message& reply) const -> bool;
    auto hasContext() const -> bool;
    void update_workflow(
        const Message& request,
        const otx::context::Server::DeliveryResult& result) const;
    void update_workflow_convey_payment(
        const Message& request,
        const otx::context::Server::DeliveryResult& result) const;
    void update_workflow_send_cash(
        const Message& request,
        const otx::context::Server::DeliveryResult& result) const;

    void account_pre();
    void account_post();
    auto construct() -> std::shared_ptr<Message>;
    auto construct_add_claim() -> std::shared_ptr<Message>;
    auto construct_check_nym() -> std::shared_ptr<Message>;
    auto construct_convey_payment() -> std::shared_ptr<Message>;
    auto construct_deposit_cash() -> std::shared_ptr<Message>;
    auto construct_deposit_cheque() -> std::shared_ptr<Message>;
    auto construct_download_contract() -> std::shared_ptr<Message>;
    auto construct_download_mint() -> std::shared_ptr<Message>;
    auto construct_get_account_data(const identifier::Generic& accountID)
        -> std::shared_ptr<Message>;
    auto construct_get_transaction_numbers() -> std::shared_ptr<Message>;
    auto construct_issue_unit_definition() -> std::shared_ptr<Message>;
    auto construct_process_inbox(
        const identifier::Generic& accountID,
        const Ledger& payload,
        otx::context::Server& context) -> std::shared_ptr<Message>;
    auto construct_publish_nym() -> std::shared_ptr<Message>;
    auto construct_publish_server() -> std::shared_ptr<Message>;
    auto construct_publish_unit() -> std::shared_ptr<Message>;
    auto construct_register_account() -> std::shared_ptr<Message>;
    auto construct_register_nym() -> std::shared_ptr<Message>;
    auto construct_request_admin() -> std::shared_ptr<Message>;
    auto construct_send_nym_object(
        const PeerObject& object,
        const Nym_p recipient,
        otx::context::Server& context,
        const RequestNumber number = -1) -> std::shared_ptr<Message>;
    auto construct_send_nym_object(
        const PeerObject& object,
        const Nym_p recipient,
        otx::context::Server& context,
        Armored& envelope,
        const RequestNumber number = -1) -> std::shared_ptr<Message>;
    auto construct_send_peer_reply() -> std::shared_ptr<Message>;
    auto construct_send_peer_request() -> std::shared_ptr<Message>;
    auto construct_send_cash() -> std::shared_ptr<Message>;
    auto construct_send_message() -> std::shared_ptr<Message>;
    auto construct_send_transfer() -> std::shared_ptr<Message>;
    auto construct_withdraw_cash() -> std::shared_ptr<Message>;
    auto download_account(
        const identifier::Generic& accountID,
        otx::context::Server::DeliveryResult& lastResult) -> std::size_t;
    auto download_accounts(
        const State successState,
        const State failState,
        otx::context::Server::DeliveryResult& lastResult) -> bool;
    auto download_box_receipt(
        const identifier::Generic& accountID,
        const BoxType box,
        const TransactionNumber number) -> bool;
    void evaluate_transaction_reply(
        otx::context::Server::DeliveryResult&& result);
    void execute();
    auto get_account_data(
        const identifier::Generic& accountID,
        std::shared_ptr<Ledger> inbox,
        std::shared_ptr<Ledger> outbox,
        otx::context::Server::DeliveryResult& lastResult) -> bool;
    auto get_receipts(
        const identifier::Generic& accountID,
        std::shared_ptr<Ledger> inbox,
        std::shared_ptr<Ledger> outbox) -> bool;
    auto get_receipts(
        const identifier::Generic& accountID,
        const BoxType type,
        Ledger& box) -> bool;
    void nymbox_post();
    void nymbox_pre();
    auto process_inbox(
        const identifier::Generic& accountID,
        std::shared_ptr<Ledger> inbox,
        std::shared_ptr<Ledger> outbox,
        otx::context::Server::DeliveryResult& lastResult) -> bool;
    void refresh();
    void reset();
    void set_result(otx::context::Server::DeliveryResult&& result);
    auto start(
        const Lock& decisionLock,
        const otx::OperationType type,
        const otx::context::Server::ExtraArgs& args) -> bool;
    auto state_machine() -> bool;
    void transaction_numbers();

    Operation(
        const api::session::Client& api,
        const identifier::Nym& nym,
        const identifier::Notary& server,
        const PasswordPrompt& reason);
};
}  // namespace opentxs::otx::client::implementation
