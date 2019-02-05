// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#if OT_CASH
#include "opentxs/blind/Mint.hpp"
#include "opentxs/blind/Purse.hpp"
#endif  // OT_CASH
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/consensus/ManagedNumber.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NymFile.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/PullSocket.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"

#include "InternalClient.hpp"

#include <atomic>
#include <future>
#include <map>
#include <memory>
#include <mutex>

#include "Operation.hpp"

#define START()                                                                \
    {                                                                          \
        const auto running = running_.exchange(true);                          \
                                                                               \
        if (running) {                                                         \
            LogDebug(": state machine is already running.").Flush();           \
                                                                               \
            return false;                                                      \
        }                                                                      \
                                                                               \
        reset();                                                               \
    }

#define OPERATION_POLL_MILLISECONDS 100
#define OPERATION_JOIN_MILLISECONDS OPERATION_POLL_MILLISECONDS
#define MAX_ERROR_COUNT 3

#define OT_METHOD "opentxs::api::client::implementation::Operation::"

#define PREPARE_CONTEXT()                                                      \
    auto contextEditor = context();                                            \
    auto& context = contextEditor.It();                                        \
    [[maybe_unused]] auto& nym = *context.Nym();                               \
    [[maybe_unused]] auto& nymID = nym.ID();                                   \
    [[maybe_unused]] auto& serverID = context.Server();                        \
    [[maybe_unused]] auto& serverNym = context.RemoteNym();                    \
    context.SetPush(enable_otx_push_.load());

#define CREATE_MESSAGE(a, ...)                                                 \
    [[maybe_unused]] auto [nextNumber, pMessage] =                             \
        context.InitializeServerCommand(MessageType::a, __VA_ARGS__);          \
                                                                               \
    if (false == bool(pMessage)) {                                             \
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct ")(#a)       \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    auto& message = *pMessage;

#define FINISH_MESSAGE(a, b)                                                   \
    const auto finalized = context.FinalizeServerCommand(message);             \
                                                                               \
    if (false == bool(finalized)) {                                            \
        LogOutput(OT_METHOD)(#a)(": Failed to sign ")(#b);                     \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    return std::move(pMessage);

#define PREPARE_TRANSACTION_WITHOUT_BALANCE_ITEM(                              \
    TRANSACTION_TYPE, ORIGIN_TYPE, ITEM_TYPE, DESTINATION_ACCOUNT)             \
                                                                               \
    PREPARE_CONTEXT();                                                         \
                                                                               \
    auto account = api_.Wallet().Account(account_id_);                         \
                                                                               \
    if (false == bool(account)) {                                              \
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account.")        \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    numbers_.insert(                                                           \
        context.NextTransactionNumber(MessageType::notarizeTransaction));      \
    auto& managedNumber = *numbers_.rbegin();                                  \
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocating transaction number ")(   \
        managedNumber->Value())                                                \
        .Flush();                                                              \
                                                                               \
    if (false == managedNumber->Valid()) {                                     \
        LogOutput(OT_METHOD)(__FUNCTION__)(                                    \
            ": No transaction numbers were available. Suggest requesting the " \
            "server for one.")                                                 \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(    \
        managedNumber->Value())                                                \
        .Flush();                                                              \
    const auto transactionNum = managedNumber->Value();                        \
    auto pLedger{api_.Factory().Ledger(nymID, account_id_, serverID)};         \
                                                                               \
    if (false == bool(pLedger)) {                                              \
        LogOutput(OT_METHOD)(__FUNCTION__)(                                    \
            ": Failed to construct transaction ledger.")                       \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    auto& ledger = *pLedger;                                                   \
    const bool generated =                                                     \
        ledger.GenerateLedger(account_id_, serverID, ledgerType::message);     \
                                                                               \
    if (false == generated) {                                                  \
        LogOutput(OT_METHOD)(__FUNCTION__)(                                    \
            ": Failed to generate transaction ledger")                         \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    std::shared_ptr<OTTransaction> pTransaction{api_.Factory()                 \
                                                    .Transaction(              \
                                                        nymID,                 \
                                                        account_id_,           \
                                                        serverID,              \
                                                        TRANSACTION_TYPE,      \
                                                        ORIGIN_TYPE,           \
                                                        transactionNum)        \
                                                    .release()};               \
                                                                               \
    if (false == bool(pTransaction)) {                                         \
        LogOutput(OT_METHOD)(__FUNCTION__)(                                    \
            ": Failed to construct transaction.")                              \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    ledger.AddTransaction(pTransaction);                                       \
    auto& transaction = *pTransaction;                                         \
    std::shared_ptr<Item> pItem{                                               \
        api_.Factory().Item(transaction, ITEM_TYPE, DESTINATION_ACCOUNT)};     \
                                                                               \
    if (false == bool(pItem)) {                                                \
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct item.")      \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    transaction.AddItem(pItem);                                                \
    auto& item = *pItem;                                                       \
    inbox_.reset(account.get().LoadInbox(nym).release());                      \
    outbox_.reset(account.get().LoadOutbox(nym).release());                    \
                                                                               \
    if (false == bool(inbox_)) {                                               \
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed loading inbox for "       \
                                           "account: ")(account_id_)           \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    if (false == bool(outbox_)) {                                              \
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed loading outbox for "      \
                                           "account: ")(account_id_)           \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    auto& inbox = *inbox_;                                                     \
    auto& outbox = *outbox_;                                                   \
    set_consensus_hash(transaction, context, account.get());

#define ADD_BALANCE_ITEM(AMOUNT)                                               \
    std::shared_ptr<Item> pBalanceItem{inbox.GenerateBalanceStatement(         \
        AMOUNT, transaction, context, account.get(), outbox)};                 \
                                                                               \
    if (false == bool(pBalanceItem)) {                                         \
        LogOutput(OT_METHOD)(__FUNCTION__)(                                    \
            ": Failed to construct balance item.")                             \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    transaction.AddItem(pBalanceItem);

#define PREPARE_TRANSACTION(                                                   \
    TRANSACTION_TYPE, ORIGIN_TYPE, ITEM_TYPE, DESTINATION_ACCOUNT, AMOUNT)     \
    PREPARE_TRANSACTION_WITHOUT_BALANCE_ITEM(                                  \
        TRANSACTION_TYPE, ORIGIN_TYPE, ITEM_TYPE, DESTINATION_ACCOUNT);        \
    ADD_BALANCE_ITEM(AMOUNT);

#define SIGN_ITEM()                                                            \
    item.SignContract(nym);                                                    \
    item.SaveContract();

#define SIGN_TRANSACTION_AND_LEDGER()                                          \
    transaction.SignContract(nym);                                             \
    transaction.SaveContract();                                                \
    ledger.SignContract(nym);                                                  \
    ledger.SaveContract();

#define FINISH_TRANSACTION()                                                   \
    SIGN_ITEM();                                                               \
    SIGN_TRANSACTION_AND_LEDGER();                                             \
    CREATE_MESSAGE(                                                            \
        notarizeTransaction,                                                   \
        Armored::Factory(String::Factory(ledger)),                             \
        account_id_,                                                           \
        -1);                                                                   \
    FINISH_MESSAGE(__FUNCTION__, notarizeTransaction);

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
api::client::internal::Operation* Factory::Operation(
    const api::client::Manager& api,
    const identifier::Nym& nym,
    const Identifier& server)
{
    return new api::client::implementation::Operation(api, nym, server);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
const std::map<Operation::Type, Operation::Category> Operation::category_{
    {Type::AddClaim, Category::Basic},
    {Type::CheckNym, Category::Basic},
    {Type::ConveyPayment, Category::Basic},
    {Type::DepositCash, Category::Transaction},
    {Type::DepositCheque, Category::Transaction},
    {Type::DownloadContract, Category::Basic},
    {Type::DownloadMint, Category::Basic},
    {Type::GetTransactionNumbers, Category::NymboxPre},
    {Type::IssueUnitDefinition, Category::CreateAccount},
    {Type::PublishNym, Category::Basic},
    {Type::PublishServer, Category::Basic},
    {Type::PublishUnit, Category::Basic},
    {Type::RefreshAccount, Category::UpdateAccount},
    {Type::RegisterAccount, Category::CreateAccount},
    {Type::RegisterNym, Category::NymboxPost},
    {Type::RequestAdmin, Category::Basic},
    {Type::SendCash, Category::Basic},
    {Type::SendMessage, Category::Basic},
    {Type::SendPeerReply, Category::Basic},
    {Type::SendPeerRequest, Category::Basic},
    {Type::SendTransfer, Category::Transaction},
    {Type::WithdrawCash, Category::Transaction},
};

const std::map<Operation::Type, std::size_t> Operation::transaction_numbers_{
    {Type::AddClaim, 0},
    {Type::CheckNym, 0},
    {Type::ConveyPayment, 0},
    {Type::DepositCash, 2},
    {Type::DepositCheque, 2},
    {Type::DownloadContract, 0},
    {Type::DownloadMint, 0},
    {Type::GetTransactionNumbers, 0},
    {Type::IssueUnitDefinition, 0},
    {Type::PublishNym, 0},
    {Type::PublishServer, 0},
    {Type::PublishUnit, 0},
    {Type::RefreshAccount, 1},
    {Type::RegisterAccount, 0},
    {Type::RegisterNym, 0},
    {Type::RequestAdmin, 0},
    {Type::SendCash, 0},
    {Type::SendMessage, 0},
    {Type::SendPeerReply, 0},
    {Type::SendPeerRequest, 0},
    {Type::SendTransfer, 2},
    {Type::WithdrawCash, 2},
};

Operation::Operation(
    const api::client::Manager& api,
    const identifier::Nym& nym,
    const Identifier& server)
    : api_(api)
    , nym_id_(nym)
    , server_id_(server)
    , shutdown_lock_()
    , running_(false)
    , type_(Type::Invalid)
    , state_(State::Idle)
    , refresh_account_(false)
    , args_()
    , message_()
    , outmail_message_()
    , result_set_(false)
    , enable_otx_push_(true)
    , result_()
    , shutdown_(Flag::Factory(false))
    , target_nym_id_(identifier::Nym::Factory())
    , target_server_id_(identifier::Server::Factory())
    , target_unit_id_(identifier::UnitDefinition::Factory())
    , contract_type_(ContractType::ERROR)
    , unit_definition_()
    , account_id_(Identifier::Factory())
    , generic_id_(Identifier::Factory())
    , amount_(0)
    , memo_(String::Factory())
    , bool_(false)
    , claim_section_(proto::CONTACTSECTION_ERROR)
    , claim_type_(proto::CITEMTYPE_ERROR)
    , cheque_()
    , payment_()
    , inbox_()
    , outbox_()
#if OT_CASH
    , purse_()
#endif
    , affected_accounts_()
    , redownload_accounts_()
    , numbers_()
    , error_count_(0)
    , peer_reply_()
    , peer_request_()
    , set_id_()
    , callback_(zmq::ListenCallback::Factory(
          [=](const zmq::Message&) -> void { this->state_machine(); }))
    , pull_(api_.ZeroMQ().PullSocket(callback_, zmq::Socket::Direction::Bind))
    , push_(api_.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
{
    init();
}

void Operation::account_pre()
{
    ServerContext::DeliveryResult lastResult{};

    switch (category_.at(type_)) {
        case Category::Transaction: {
            download_accounts(State::Execute, State::NymboxPre, lastResult);
        } break;
        default: {
            state_.store(State::Execute);
        }
    }
}

void Operation::account_post()
{
    ServerContext::DeliveryResult lastResult{};

    if (download_accounts(State::NymboxPost, State::NymboxPre, lastResult)) {
        if (false == result_set_.load()) { set_result(std::move(lastResult)); }

        affected_accounts_ = redownload_accounts_;
        redownload_accounts_.clear();

        if (0 < affected_accounts_.size()) { state_.store(State::AccountPost); }
    }
}

bool Operation::AddClaim(
    const proto::ContactSectionName section,
    const proto::ContactItemType type,
    const String& value,
    const bool primary)
{
    START()

    memo_ = value;
    bool_ = primary;
    claim_section_ = section;
    claim_type_ = type;

    return start(Type::AddClaim, {});
}

bool Operation::check_future(ServerContext::SendFuture& future)
{
    return std::future_status::ready !=
           future.wait_for(
               std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));
}

std::shared_ptr<Message> Operation::construct()
{
    switch (type_.load()) {
        case Type::AddClaim: {

            return construct_add_claim();
        }
        case Type::CheckNym: {

            return construct_check_nym();
        }
        case Type::ConveyPayment: {

            return construct_convey_payment();
        }
#if OT_CASH
        case Type::DepositCash: {

            return construct_deposit_cash();
        }
#endif
        case Type::DepositCheque: {

            return construct_deposit_cheque();
        }
        case Type::DownloadContract: {

            return construct_download_contract();
        }
#if OT_CASH
        case Type::DownloadMint: {

            return construct_download_mint();
        }
#endif
        case Type::GetTransactionNumbers: {

            return construct_get_transaction_numbers();
        }
        case Type::IssueUnitDefinition: {

            return construct_issue_unit_definition();
        }
        case Type::PublishNym: {

            return construct_publish_nym();
        }
        case Type::PublishServer: {

            return construct_publish_server();
        }
        case Type::PublishUnit: {

            return construct_publish_unit();
        }
        case Type::RegisterAccount: {

            return construct_register_account();
        }
        case Type::RegisterNym: {

            return construct_register_nym();
        }
        case Type::RequestAdmin: {

            return construct_request_admin();
        }
#if OT_CASH
        case Type::SendCash: {

            return construct_send_cash();
        }
#endif
        case Type::SendMessage: {

            return construct_send_message();
        }
        case Type::SendPeerReply: {

            return construct_send_peer_reply();
        }
        case Type::SendPeerRequest: {

            return construct_send_peer_request();
        }
        case Type::SendTransfer: {

            return construct_send_transfer();
        }
#if OT_CASH
        case Type::WithdrawCash: {

            return construct_withdraw_cash();
        }
#endif
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown message type")
                .Flush();
        }
    }

    return {};
}

std::shared_ptr<Message> Operation::construct_add_claim()
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(addClaim, -1, true, true);

    message.m_strNymID2 = String::Factory(
        std::to_string(static_cast<std::uint32_t>(claim_section_)));
    message.m_strInstrumentDefinitionID = String::Factory(
        std::to_string(static_cast<std::uint32_t>(claim_type_)));
    message.m_strAcctID = memo_;
    message.m_bBool = bool_;

    FINISH_MESSAGE(__FUNCTION__, addClaim);
}

std::shared_ptr<Message> Operation::construct_check_nym()
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(checkNym, target_nym_id_, -1, true, true);
    FINISH_MESSAGE(__FUNCTION__, checkNym);
}

std::shared_ptr<Message> Operation::construct_convey_payment()
{
    if (false == bool(payment_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No payment to convey").Flush();

        return {};
    }

    auto& payment = *payment_;
    const auto recipientNym = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(recipientNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(sendNymInstrument, target_nym_id_, -1, true, true);
    const RequestNumber requestNumber{message.m_strRequestNum->ToLong()};
    const auto& recipientPubkey = recipientNym->GetPublicEncrKey();
    auto serialized = String::Factory();
    const bool havePayment = payment.GetPaymentContents(serialized);

    if (false == havePayment) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed attempt to send a blank payment.")
            .Flush();

        return {};
    }

    OTEnvelope envelope{};
    auto sealed = envelope.Seal(recipientPubkey, serialized);

    if (sealed) { sealed &= envelope.GetCiphertext(message.m_ascPayload); }

    if (false == sealed) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed encrypt payment.").Flush();

        return {};
    }

    const auto finalized = context.FinalizeServerCommand(message);

    if (false == bool(finalized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign message");

        return {};
    }

    const auto pObject = api_.Factory().PeerObject(
        context.Nym(), String::Factory(message)->Get(), true);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create peer object");

        return {};
    }

    auto& object = *pObject;

    return construct_send_nym_object(
        object, *recipientNym, context, requestNumber);
}

#if OT_CASH
std::shared_ptr<Message> Operation::construct_deposit_cash()
{
    if (false == bool(purse_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing purse.").Flush();

        return {};
    }

    auto& purse = *purse_;
    const Amount amount{purse.Value()};

    PREPARE_TRANSACTION(
        transactionType::deposit,
        originType::not_applicable,
        itemType::deposit,
        Identifier::Factory(),
        amount);

    const auto& unitID = account.get().GetInstrumentDefinitionID();

    if (unitID != purse.Unit()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect account type").Flush();

        return {};
    }

    item.SetAttachment(proto::ProtoAsData(purse.Serialize()));

    FINISH_TRANSACTION();
}
#endif

std::shared_ptr<Message> Operation::construct_deposit_cheque()
{
    if (false == bool(cheque_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No cheque to deposit").Flush();

        return {};
    }

    auto& cheque = *cheque_;
    const Amount amount{cheque.GetAmount()};

    PREPARE_TRANSACTION(
        transactionType::deposit,
        originType::not_applicable,
        itemType::depositCheque,
        Identifier::Factory(),
        amount);

    if (cheque.GetNotaryID() != serverID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": NotaryID on cheque (")(
            cheque.GetNotaryID())(
            ") doesn't match notaryID where it's being deposited to (")(
            serverID)(").")
            .Flush();

        return {};
    }

    // If cancellingCheque==true, we're actually cancelling the cheque by
    // "depositing" it back into the same account it's drawn on.
    bool cancellingCheque{false};
    auto copy{api_.Factory().Cheque(
        serverID, account.get().GetInstrumentDefinitionID())};

    if (cheque.HasRemitter()) {
        cancellingCheque =
            ((cheque.GetRemitterAcctID() == account_id_) &&
             (cheque.GetRemitterNymID() == nymID));
    } else {
        cancellingCheque =
            ((cheque.GetSenderAcctID() == account_id_) &&
             (cheque.GetSenderNymID() == nymID));
        if (cancellingCheque) cancellingCheque = cheque.VerifySignature(nym);
    }

    if (cancellingCheque) {
        cancellingCheque =
            context.VerifyIssuedNumber(cheque.GetTransactionNum());

        // If we TRIED to cancel the cheque (being in this block...) yet the
        // signature fails to verify, or the transaction number isn't even
        // issued, then our attempt to cancel the cheque is going to fail.
        if (false == cancellingCheque) {
            // This is the "tried and failed" block.
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Cannot cancel this cheque. Either the signature fails to "
                "verify, or the transaction number is already closed out.")
                .Flush();

            return {};
        }

        // Else we succeeded in verifying signature and issued num. Let's just
        // make sure there isn't a chequeReceipt or voucherReceipt already
        // sitting in the inbox, for this same cheque.
        auto pChequeReceipt =
            inbox_->GetChequeReceipt(cheque.GetTransactionNum());

        if (pChequeReceipt) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Cannot cancel this cheque. There is already a ")(
                cheque.HasRemitter() ? "voucherReceipt"
                                     : "chequeReceipt")(" for it in the inbox.")
                .Flush();

            return {};
        }

        if (false == copy->LoadContractFromString(String::Factory(cheque))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unable to load cheque from string.")
                .Flush();

            return {};
        }
    }

    // By this point, we're either NOT cancelling the cheque, or if we are,
    // we've already verified the signature and transaction number on the
    // cheque. (AND we've already verified that there aren't any
    // chequeReceipts for this cheque, in the inbox.)
    const bool cancel = (cancellingCheque && !cheque.HasRemitter());

    if (cancel) {
        copy->CancelCheque();
        copy->ReleaseSignatures();
        copy->SignContract(nym);
        copy->SaveContract();
        cheque_.reset(copy.release());

        OT_ASSERT(cheque_);
    }

    const auto strNote = String::Factory(
        cancellingCheque ? "Cancel this cheque, please!"
                         : "Deposit this cheque, please!");
    item.SetNote(strNote);
    item.SetAttachment(String::Factory(cheque));

    FINISH_TRANSACTION();
}

std::shared_ptr<Message> Operation::construct_download_contract()
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(getInstrumentDefinition, -1, true, true);

    message.m_strInstrumentDefinitionID = String::Factory(generic_id_);
    message.enum_ = static_cast<std::uint8_t>(contract_type_);

    FINISH_MESSAGE(__FUNCTION__, getInstrumentDefinition);
}

#if OT_CASH
std::shared_ptr<Message> Operation::construct_download_mint()
{
    auto contract = api_.Wallet().UnitDefinition(target_unit_id_);

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid unit definition id");

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(getMint, -1, true, true);

    message.m_strInstrumentDefinitionID = String::Factory(target_unit_id_);

    FINISH_MESSAGE(__FUNCTION__, getMint);
}
#endif

std::shared_ptr<Message> Operation::construct_get_account_data(
    const Identifier& accountID)
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(getAccountData, -1, true, true);

    message.m_strAcctID = String::Factory(accountID);

    FINISH_MESSAGE(__FUNCTION__, getAccountData);
}

std::shared_ptr<Message> Operation::construct_get_transaction_numbers()
{
    return api_.OTAPI().getTransactionNumbers(context().It());
}

std::shared_ptr<Message> Operation::construct_issue_unit_definition()
{
    if (false == bool(unit_definition_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing unit definition");

        return {};
    }

    auto contract = api_.Wallet().UnitDefinition(*unit_definition_);

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid unit definition").Flush();

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(registerInstrumentDefinition, -1, true, true);

    auto id = contract->ID();
    id->GetString(message.m_strInstrumentDefinitionID);
    message.m_ascPayload->SetData(
        proto::ProtoAsData(contract->PublicContract()));

    FINISH_MESSAGE(__FUNCTION__, registerInstrumentDefinition);
}

std::shared_ptr<Message> Operation::construct_publish_nym()
{
    const auto contract = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Nym not found: ")(target_nym_id_)
            .Flush();

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(registerContract, -1, true, true);

    message.enum_ = static_cast<std::uint8_t>(ContractType::NYM);
    message.m_ascPayload->SetData(proto::ProtoAsData(contract->asPublicNym()));

    FINISH_MESSAGE(__FUNCTION__, registerContract);
}

std::shared_ptr<Message> Operation::construct_publish_server()
{
    const auto contract = api_.Wallet().Server(target_server_id_);

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Server not found: ")(
            target_server_id_)
            .Flush();

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(registerContract, -1, true, true);

    message.enum_ = static_cast<std::uint8_t>(ContractType::SERVER);
    message.m_ascPayload->SetData(
        proto::ProtoAsData(contract->PublicContract()));

    FINISH_MESSAGE(__FUNCTION__, registerContract);
}

std::shared_ptr<Message> Operation::construct_publish_unit()
{
    const auto contract = api_.Wallet().UnitDefinition(target_unit_id_);

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unit definition not found: ")(
            target_unit_id_)
            .Flush();

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(registerContract, -1, true, true);

    message.enum_ = static_cast<std::uint8_t>(ContractType::UNIT);
    message.m_ascPayload->SetData(proto::ProtoAsData(contract->Contract()));

    FINISH_MESSAGE(__FUNCTION__, registerContract);
}

std::shared_ptr<Message> Operation::construct_process_inbox(
    const Identifier& accountID,
    const Ledger& payload,
    ServerContext& context)
{
    CREATE_MESSAGE(
        processInbox,
        Armored::Factory(String::Factory(payload)),
        accountID,
        -1);
    FINISH_MESSAGE(__FUNCTION__, processInbox);
}

std::shared_ptr<Message> Operation::construct_register_account()
{
    auto contract = api_.Wallet().UnitDefinition(target_unit_id_);

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid unit definition id");

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(registerAccount, -1, true, true);

    message.m_strInstrumentDefinitionID = String::Factory(target_unit_id_);

    FINISH_MESSAGE(__FUNCTION__, registerAccount);
}

std::shared_ptr<Message> Operation::construct_register_nym()
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(registerNym, -1, true, true);

    message.m_ascPayload->SetData(proto::ProtoAsData(nym.asPublicNym()));

    FINISH_MESSAGE(__FUNCTION__, registerNym);
}

std::shared_ptr<Message> Operation::construct_request_admin()
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(requestAdmin, -1, true, true);

    message.m_strAcctID = memo_;

    FINISH_MESSAGE(__FUNCTION__, requestAdmin);
}

std::shared_ptr<Message> Operation::construct_send_nym_object(
    const PeerObject& object,
    const Nym& recipient,
    ServerContext& context,
    const RequestNumber number)
{
    CREATE_MESSAGE(sendNymMessage, recipient.ID(), number, true, true);

    const auto plaintext(proto::ProtoAsArmored(
        object.Serialize(), String::Factory("PEER OBJECT")));
    OTEnvelope envelope{};
    auto sealed = envelope.Seal(recipient, plaintext);

    if (sealed) { sealed &= envelope.GetCiphertext(message.m_ascPayload); }

    if (false == sealed) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed encrypt object.").Flush();

        return {};
    }

    FINISH_MESSAGE(__FUNCTION__, sendNymMessage);
}

#if OT_CASH
std::shared_ptr<Message> Operation::construct_send_cash()
{
    const auto pRecipient = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(pRecipient)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    const auto& recipient = *pRecipient;

    if (false == bool(purse_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid purse").Flush();

        return {};
    }

    PREPARE_CONTEXT();

    const auto pObject = api_.Factory().PeerObject(context.Nym(), purse_);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create peer object");

        return {};
    }

    auto& object = *pObject;

    return construct_send_nym_object(object, recipient, context, -1);
}
#endif

std::shared_ptr<Message> Operation::construct_send_message()
{
    const auto recipientNym = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(recipientNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    if (memo_->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Message is empty").Flush();

        return {};
    }

    auto contextEditor = context();
    auto& context = contextEditor.It();
    const auto& nym = *context.Nym();
    context.SetPush(enable_otx_push_.load());
    const auto pObject =
        api_.Factory().PeerObject(context.Nym(), memo_->Get(), false);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create peer object")
            .Flush();

        return {};
    }

    auto& object = *pObject;

    OT_ASSERT(proto::PEEROBJECT_MESSAGE == object.Type());

    auto pOutput =
        construct_send_nym_object(object, *recipientNym, context, -1);

    if (false == bool(pOutput)) { return {}; }

    auto& output = *pOutput;
    const TransactionNumber number{output.m_strRequestNum->ToLong()};
    [[maybe_unused]] auto [notUsed, pOutmail] = context.InitializeServerCommand(
        MessageType::outmail, target_nym_id_, number, false, false);

    if (false == bool(pOutmail)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct outmail")
            .Flush();

        return {};
    }

    auto& outmail = *pOutmail;
    auto pCopy = api_.Factory().PeerObject(nullptr, memo_->Get());

    if (false == bool(pCopy)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create peer object")
            .Flush();

        return {};
    }

    const auto& copy = *pCopy;
    const auto plaintext(proto::ProtoAsArmored(
        copy.Serialize(), String::Factory("PEER OBJECT")));
    OTEnvelope envelope{};

    if (false == envelope.Seal(nym, plaintext)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed sealing envelope.")
            .Flush();

        return {};
    }

    if (false == envelope.GetCiphertext(outmail.m_ascPayload)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed sealing envelope.")
            .Flush();

        return {};
    }

    outmail.SignContract(nym);
    outmail.SaveContract();
    outmail_message_ = std::move(pOutmail);

    return pOutput;
}

std::shared_ptr<Message> Operation::construct_send_peer_reply()
{
    const auto recipientNym = api_.Wallet().Nym(target_nym_id_);
    if (false == bool(recipientNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    if (false == bool(peer_reply_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid reply.").Flush();

        return {};
    }

    if (false == bool(peer_request_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request.").Flush();

        return {};
    }

    auto contextEditor = context();
    auto& context = contextEditor.It();
    const auto& nym = *context.Nym();
    context.SetPush(enable_otx_push_.load());
    const bool saved = api_.Wallet().PeerReplyCreate(
        nym.ID(), peer_request_->Contract(), peer_reply_->Contract());

    if (false == saved) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save reply in wallet.")
            .Flush();

        return {};
    }

    const auto pObject = api_.Factory().PeerObject(
        peer_request_, peer_reply_, PEER_OBJECT_PEER_REPLY);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create peer object")
            .Flush();

        return {};
    }

    auto& object = *pObject;

    OT_ASSERT(proto::PEEROBJECT_RESPONSE == object.Type());

    auto pOutput =
        construct_send_nym_object(object, *recipientNym, context, -1);

    if (false == bool(pOutput)) { return {}; }

    return pOutput;
}

std::shared_ptr<Message> Operation::construct_send_peer_request()
{
    const auto recipientNym = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(recipientNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    if (false == bool(peer_request_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid request.").Flush();

        return {};
    }

    auto contextEditor = context();
    auto& context = contextEditor.It();
    const auto& nym = *context.Nym();
    context.SetPush(enable_otx_push_.load());
    const bool saved =
        api_.Wallet().PeerRequestCreate(nym.ID(), peer_request_->Contract());

    if (false == saved) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to save request in wallet.")
            .Flush();

        return {};
    }

    const auto itemID = peer_request_->ID();
    const auto pObject =
        api_.Factory().PeerObject(peer_request_, PEER_OBJECT_PEER_REQUEST);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create peer object")
            .Flush();
        api_.Wallet().PeerRequestCreateRollback(nym.ID(), itemID);

        return {};
    }

    auto& object = *pObject;

    OT_ASSERT(proto::PEEROBJECT_REQUEST == object.Type());

    auto pOutput =
        construct_send_nym_object(object, *recipientNym, context, -1);

    if (false == bool(pOutput)) { return {}; }

    return pOutput;
}

std::shared_ptr<Message> Operation::construct_send_transfer()
{
    PREPARE_TRANSACTION_WITHOUT_BALANCE_ITEM(
        transactionType::transfer,
        originType::not_applicable,
        itemType::transfer,
        generic_id_);

    item.SetAmount(amount_);

    if (memo_->Exists()) { item.SetNote(memo_); }

    SIGN_ITEM();

    // Need to setup a dummy outbox transaction (to mimic the one that will be
    // on the server side when this pending transaction is actually put into the
    // real outbox.) When the server adds its own, and then compares the two,
    // they should both show the same pending transaction, in order for this
    // balance agreement to be valid. Otherwise the server would have to refuse
    // it for being inaccurate (server can't sign something inaccurate!) So I
    // throw a dummy on there before generating balance statement.
    std::shared_ptr<OTTransaction> outboxTransaction{
        api_.Factory()
            .Transaction(
                *outbox_,
                transactionType::pending,
                originType::not_applicable,
                1)
            .release()};

    OT_ASSERT(outboxTransaction);

    outboxTransaction->SetReferenceString(String::Factory(item));
    outboxTransaction->SetReferenceToNum(item.GetTransactionNum());
    outbox.AddTransaction(outboxTransaction);

    ADD_BALANCE_ITEM(amount_ * (-1));
    SIGN_TRANSACTION_AND_LEDGER();
    CREATE_MESSAGE(
        notarizeTransaction,
        Armored::Factory(String::Factory(ledger)),
        account_id_,
        -1);

    // Reset the temporary changes made above
    inbox_.reset(account.get().LoadInbox(nym).release());
    outbox_.reset(account.get().LoadOutbox(nym).release());
    account.Release();
    const auto workflowID = api_.Workflow().CreateTransfer(item, message);

    if (workflowID->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to create transfer workflow")
            .Flush();
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Created transfer ")(
            item.GetTransactionNum())(" workflow ")(workflowID)
            .Flush();
    }

    FINISH_MESSAGE(__FUNCTION__, notarizeTransaction);
}

#if OT_CASH
std::shared_ptr<Message> Operation::construct_withdraw_cash()
{
    const Amount totalAmount(amount_);

    PREPARE_TRANSACTION(
        transactionType::withdrawal,
        originType::not_applicable,
        itemType::withdrawal,
        Identifier::Factory(),
        totalAmount * (-1));

    // TODO unit id type
    const auto unitID = identifier::UnitDefinition::Factory(
        account.get().GetInstrumentDefinitionID().str());
    const bool exists = OTDB::Exists(
        api_.DataFolder(),
        OTFolders::Mint().Get(),
        serverID.str(),
        unitID->str(),
        "");

    if (false == exists) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": File does not exist: ")(
            OTFolders::Mint())(Log::PathSeparator())(serverID)(
            Log::PathSeparator())(unitID)
            .Flush();

        return {};
    }

    auto pMint{api_.Factory().Mint(
        String::Factory(serverID), String::Factory(unitID))};

    if (false == bool(pMint)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing mint").Flush();

        return {};
    }

    auto& mint = *pMint;
    const bool validMint = mint.LoadMint() && mint.VerifyMint(serverNym);

    if (false == validMint) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid mint").Flush();

        return {};
    }

    auto pPurse{api_.Factory().Purse(context, unitID, mint, totalAmount)};

    if (false == bool(pPurse)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct purse")
            .Flush();

        return {};
    }

    auto& purse = *pPurse;
    item.SetNote(String::Factory("Gimme cash!"));
    item.SetAttachment(proto::ProtoAsData(purse.Serialize()));

    FINISH_TRANSACTION();
}
#endif

Editor<ServerContext> Operation::context() const
{
    return api_.Wallet().mutable_ServerContext(nym_id_, server_id_);
}

bool Operation::ConveyPayment(
    const identifier::Nym& recipient,
    const std::shared_ptr<const OTPayment> payment)
{
    START()

    target_nym_id_ = recipient;
    payment_ = payment;

    return start(Type::ConveyPayment, {});
}

#if OT_CASH
bool Operation::DepositCash(
    const Identifier& depositAccountID,
    const std::shared_ptr<blind::Purse> purse)
{
    if (false == bool(purse)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid purse").Flush();

        return false;
    }

    auto pContext = api_.Wallet().ServerContext(nym_id_, server_id_);

    if (false == bool(pContext)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load context").Flush();

        return false;
    }

    auto& context = *pContext;
    const auto& nym = *context.Nym();
    const auto& serverNym = context.RemoteNym();

    if (false == purse->Unlock(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to unlock pursed").Flush();

        return false;
    }

    if (false == purse->AddNym(serverNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to encrypt purse to notary")
            .Flush();

        return false;
    }

    START()

    account_id_ = depositAccountID;
    affected_accounts_.insert(depositAccountID);
    purse_ = purse;

    return start(Type::DepositCash, {});
}
#endif

bool Operation::DepositCheque(
    const Identifier& depositAccountID,
    const std::shared_ptr<Cheque> cheque)
{
    START()

    account_id_ = depositAccountID;
    affected_accounts_.insert(depositAccountID);
    cheque_ = cheque;

    return start(Type::DepositCheque, {});
}

bool Operation::download_accounts(
    const State successState,
    const State failState,
    ServerContext::DeliveryResult& lastResult)
{
    if (affected_accounts_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: no accounts to update")
            .Flush();
        state_.store(successState);

        return true;
    }

    std::size_t ready{0};

    for (const auto& accountID : affected_accounts_) {
        if (shutdown_.get()) { return false; }

        ready += download_account(accountID, lastResult);
    }

    if (affected_accounts_.size() == ready) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": All accounts synchronized")
            .Flush();
        state_.store(successState);

        return true;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Retrying account synchronization")
        .Flush();
    state_.store(failState);

    return false;
}

std::size_t Operation::download_account(
    const Identifier& accountID,
    ServerContext::DeliveryResult& lastResult)
{
    std::shared_ptr<Ledger> inbox{api_.Factory().Ledger(
        nym_id_, accountID, server_id_, ledgerType::inbox)};
    std::shared_ptr<Ledger> outbox{api_.Factory().Ledger(
        nym_id_, accountID, server_id_, ledgerType::outbox)};

    OT_ASSERT(inbox);
    OT_ASSERT(outbox);
    OT_ASSERT(ledgerType::inbox == inbox->GetType());
    OT_ASSERT(ledgerType::outbox == outbox->GetType());

    if (get_account_data(accountID, inbox, outbox, lastResult)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Success downloading account ")(
            accountID)
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed downloading account ")(
            accountID)
            .Flush();

        return 0;
    }

    if (shutdown_.get()) { return 0; }

    if (get_receipts(accountID, inbox, outbox)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Success synchronizing account ")(
            accountID)
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed synchronizing account ")(
            accountID)
            .Flush();

        return 0;
    }

    if (shutdown_.get()) { return 0; }

    if (process_inbox(accountID, inbox, outbox, lastResult)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Success processing inbox ")(
            accountID)
            .Flush();

        return 1;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed processing inbox ")(
            accountID)
            .Flush();

        return 0;
    }
}

bool Operation::download_box_receipt(
    const Identifier& accountID,
    const BoxType box,
    const TransactionNumber number)
{
    PREPARE_CONTEXT();

    [[maybe_unused]] auto [requestNumber, message] =
        context.InitializeServerCommand(
            MessageType::getBoxReceipt, -1, false, false);

    if (false == bool(message)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct message")
            .Flush();

        return false;
    }

    std::shared_ptr<Message> command{message.release()};

    OT_ASSERT(command);

    command->m_strAcctID = String::Factory(accountID);
    command->m_lDepth = static_cast<std::int32_t>(box);
    command->m_lTransactionNum = number;
    const auto finalized = context.FinalizeServerCommand(*command);

    if (false == finalized) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign message").Flush();

        return false;
    }

    context.SetPush(enable_otx_push_.load());
    auto result = context.Queue(api_, command, {});

    while (false == bool(result)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Context is busy").Flush();
        Log::Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));
        result = context.Queue(api_, command, {});
    }

    OT_ASSERT(result);

    while (check_future(*result)) {
        if (shutdown_.get()) { return false; }
    }

    return proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result->get());
}

bool Operation::DownloadContract(const Identifier& ID, const ContractType type)
{
    START()

    generic_id_ = ID;
    contract_type_ = type;

    return start(Type::DownloadContract, {});
}

void Operation::evaluate_transaction_reply(
    ServerContext::DeliveryResult&& result)
{
    auto& pMessage = result.second;

    OT_ASSERT(pMessage);

    auto& message = *pMessage;
    auto accountID = Identifier::Factory(message.m_strAcctID);
    const auto success = evaluate_transaction_reply(accountID, message);

    if (false == success) {
        // TODO determine if failure was caused by a consensus hash failure.
        // In this case, set the state to NymboxPre
    }

    set_result(std::move(result));
    state_.store(State::AccountPost);
}

bool Operation::evaluate_transaction_reply(
    const Identifier& accountID,
    const Message& reply) const
{
    if (false == reply.m_bSuccess) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Message failure").Flush();

        return false;
    }

    switch (Message::Type(reply.m_strCommand->Get())) {
        case MessageType::notarizeTransactionResponse:
        case MessageType::processInboxResponse:
        case MessageType::processNymboxResponse:
            break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": ")(reply.m_strCommand)(
                " is not a transaction")
                .Flush();

            return false;
        }
    }

    const auto serialized = String::Factory(reply.m_ascPayload);

    if (false == serialized->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": No response ledger found on message.")
            .Flush();

        return false;
    }

    auto response{api_.Factory().Ledger(nym_id_, accountID, server_id_)};

    OT_ASSERT(response);

    if (false == response->LoadContractFromString(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to deserialize response ledger")
            .Flush();

        return false;
    }

    const std::size_t count = (response->GetTransactionCount() >= 0)
                                  ? response->GetTransactionCount()
                                  : 0;
    std::size_t good{0};

    if (0 == count) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Response ledger does not contain a transaction")
            .Flush();

        return false;
    }

    for (auto& [number, pTransaction] : response->GetTransactionMap()) {
        if (1 > number) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction number ")(
                number)
                .Flush();
            continue;
        }

        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction ")(number)
                .Flush();
            continue;
        }

        auto& transaction = *pTransaction;

        if (transaction.GetSuccess()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Successful transaction ")(
                number)
                .Flush();
            ++good;
        } else {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Failed transaction ")(number)
                .Flush();
        }
    }

    return count == good;
}

void Operation::execute()
{
    if (refresh_account_.load()) {
        state_.store(State::AccountPost);

        return;
    }

    if (result_set_.load()) {
        state_.store(State::AccountPost);

        return;
    }

    if (message_) {
        refresh();
    } else {
        message_ = construct();
    }

    if (false == bool(message_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct command")
            .Flush();
        ++error_count_;

        return;
    }

    const auto& category = category_.at(type_.load());

    PREPARE_CONTEXT();

    ServerContext::QueueResult result{};

    if (Category::Transaction == category) {
        result =
            context.Queue(api_, message_, inbox_, outbox_, &numbers_, args_);
    } else {
        result = context.Queue(api_, message_, args_);
    }

    if (false == bool(result)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Context is busy").Flush();
        Log::Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));

        return;
    }

    while (check_future(*result)) {
        if (shutdown_.get()) { return; }
    }

    auto finished = result->get();
    update_workflow(*message_, finished);

    switch (std::get<0>(finished)) {
        case proto::LASTREPLYSTATUS_MESSAGESUCCESS: {
            if (Category::Transaction == category) {
                evaluate_transaction_reply(std::move(finished));
            } else if (Category::CreateAccount == category) {
                OT_ASSERT(finished.second);

                auto& reply = finished.second;
                const auto accountID = Identifier::Factory(reply->m_strAcctID);
                affected_accounts_.emplace(std::move(accountID));
                set_result(std::move(finished));
                state_.store(State::AccountPost);
            } else {
                if (Type::SendMessage == type_.load()) {
                    OT_ASSERT(outmail_message_);

                    const auto messageID = api_.Activity().Mail(
                        nym_id_, *outmail_message_, StorageBox::MAILOUTBOX);

                    if (set_id_) { set_id_(Identifier::Factory(messageID)); }

                    set_result(std::move(finished));
                } else {
                    set_result(std::move(finished));
                }

                state_.store(State::NymboxPost);
            }
        } break;
        case proto::LASTREPLYSTATUS_MESSAGEFAILED: {
            ++error_count_;
            state_.store(State::NymboxPre);
        } break;
        default: {
            return;
        }
    }
}

bool Operation::get_account_data(
    const Identifier& accountID,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox,
    ServerContext::DeliveryResult& lastResult)
{
    auto message = construct_get_account_data(accountID);

    if (false == bool(message)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct command")
            .Flush();

        return false;
    }

    PREPARE_CONTEXT();

    auto result = context.Queue(api_, message, inbox, outbox, {});

    if (false == bool(result)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Context is busy").Flush();
        Log::Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));

        return false;
    }

    while (check_future(*result)) {
        if (shutdown_.get()) { return false; }
    }

    lastResult = result->get();

    return proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(lastResult);
}

bool Operation::get_receipts(
    const Identifier& accountID,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox)
{
    OT_ASSERT(inbox);
    OT_ASSERT(outbox);

    bool output{true};

    output &= get_receipts(accountID, BoxType::Inbox, *inbox);
    output &= get_receipts(accountID, BoxType::Outbox, *outbox);

    return output;
}

bool Operation::get_receipts(
    const Identifier& accountID,
    const BoxType type,
    Ledger& box)
{
    const auto count = box.GetTransactionMap().size();
    std::size_t good{0};

    if (0 == count) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Box is empty").Flush();
    }

    for (auto [number, pItem] : box.GetTransactionMap()) {
        if (1 > number) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction number ")(
                number)
                .Flush();

            continue;
        }

        if (false == bool(pItem)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: Invalid item ")(
                number)
                .Flush();
        }

        const auto exists = VerifyBoxReceiptExists(
            api_.DataFolder(),
            server_id_,
            nym_id_,
            accountID,
            static_cast<std::int32_t>(type),
            number);

        if (exists) {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Receipt ")(number)(
                " already exists.")
                .Flush();
            ++good;

            continue;
        }

        if (download_box_receipt(accountID, type, number)) {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Downloaded receipt ")(number)
                .Flush();
            ++good;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to download receipt ")(
                number)
                .Flush();
        }
    }

    return count == good;
}

Operation::Future Operation::GetFuture() { return result_.get_future(); }

void Operation::init()
{
    const auto endpoint =
        std::string("inproc://") + Identifier::Random()->str();
    pull_->Start(endpoint);
    push_->Start(endpoint);
}

bool Operation::IssueUnitDefinition(
    const std::shared_ptr<const proto::UnitDefinition> unitDefinition,
    const ServerContext::ExtraArgs& args)
{
    if (false == bool(unitDefinition)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing unit definition").Flush();

        return false;
    }

    if (false == proto::Validate(*unitDefinition, VERBOSE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid unit definition").Flush();

        return false;
    }

    START()

    unit_definition_ = unitDefinition;

    return start(Type::IssueUnitDefinition, args);
}

void Operation::join()
{
    while (State::Idle != state_.load()) {
        Log::Sleep(std::chrono::milliseconds(OPERATION_JOIN_MILLISECONDS));
    }
}

void Operation::nymbox_post()
{
    auto contextEditor = context();
    auto& context = contextEditor.It();
    context.SetPush(enable_otx_push_.load());
    auto mismatch = !context.NymboxHashMatch();
    bool post{false};

    switch (category_.at(type_)) {
        case Category::Transaction:
        case Category::UpdateAccount:
        case Category::CreateAccount:
        case Category::NymboxPre:
        case Category::NymboxPost: {
            post = true;
        } break;
        case Category::Basic:
        default: {
            state_.store(State::Idle);
        }
    }

    if (post || mismatch) {
        auto result = context.RefreshNymbox(api_);

        if (false == bool(result)) {
            LogTrace(OT_METHOD)(__FUNCTION__)(": Context is busy").Flush();

            return;
        }

        while (check_future(*result)) {
            if (shutdown_.get()) { return; }
        }

        switch (std::get<0>(result->get())) {
            case proto::LASTREPLYSTATUS_MESSAGESUCCESS: {
                if (context.NymboxHashMatch()) { state_.store(State::Idle); }
            } break;
            default: {
            }
        }
    }
}

void Operation::nymbox_pre()
{
    bool needInbox{false};

    switch (category_.at(type_)) {
        case Category::UpdateAccount:
        case Category::Transaction: {
            needInbox = true;
            [[fallthrough]];
        }
        case Category::CreateAccount:
        case Category::NymboxPre: {
            auto contextEditor = context();
            auto& context = contextEditor.It();
            context.SetPush(enable_otx_push_.load());

            if (context.NymboxHashMatch()) {
                if (needInbox) {
                    state_.store(State::TransactionNumbers);
                } else {
                    state_.store(State::Execute);
                }

                return;
            }

            auto result = context.RefreshNymbox(api_);

            if (false == bool(result)) {
                LogTrace(OT_METHOD)(__FUNCTION__)(": Context is busy").Flush();

                break;
            }

            while (check_future(*result)) {
                if (shutdown_.get()) { return; }
            }

            switch (std::get<0>(result->get())) {
                case proto::LASTREPLYSTATUS_MESSAGESUCCESS: {
                    if (needInbox) {
                        state_.store(State::TransactionNumbers);
                    } else {
                        state_.store(State::Execute);
                    }
                } break;
                default: {
                }
            }
        } break;
        case Category::NymboxPost:
        case Category::Basic:
        default: {
            state_.store(State::Execute);
        }
    }
}

bool Operation::process_inbox(
    const Identifier& accountID,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox,
    ServerContext::DeliveryResult& lastResult)
{
    class Cleanup
    {
    public:
        void SetSuccess() { recover_ = false; }

        Cleanup(const TransactionNumber number, ServerContext& context)
            : context_(context)
            , number_(number)
        {
        }
        Cleanup() = delete;
        ~Cleanup()
        {
            if (recover_ && (0 != number_)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Recovering unused number ")(number_)(".")
                    .Flush();
                const bool recovered = context_.RecoverAvailableNumber(number_);

                if (false == recovered) {
                    LogOutput(OT_METHOD)(__FUNCTION__)("Failed.").Flush();
                }
            }
        }

    private:
        ServerContext& context_;
        const TransactionNumber number_{0};
        bool recover_{true};
    };

    OT_ASSERT(inbox);
    OT_ASSERT(outbox);
    OT_ASSERT(ledgerType::inbox == inbox->GetType());
    OT_ASSERT(ledgerType::outbox == outbox->GetType());

    const std::size_t count =
        (inbox->GetTransactionCount() > 0) ? inbox->GetTransactionCount() : 0;

    if (1 > count) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": No items to accept in account ")(
            accountID)
            .Flush();

        return true;
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": ")(count)(
            " items to accept in account ")(accountID)
            .Flush();
        redownload_accounts_.insert(accountID);
    }

    PREPARE_CONTEXT();

    auto [response, recoverNumber] =
        api_.OTAPI().CreateProcessInbox(accountID, context, *inbox);

    if (false == bool(response)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error instantiating processInbox for account: ")(accountID)
            .Flush();

        return false;
    }

    Cleanup cleanup(recoverNumber, context);

    for (std::size_t i = 0; i < count; ++i) {
        auto transaction = inbox->GetTransactionByIndex(i);

        if (false == bool(transaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction").Flush();

            return false;
        }

        const auto number = transaction->GetTransactionNum();

        if (transaction->IsAbbreviated()) {
            inbox->LoadBoxReceipt(number);
            transaction = inbox->GetTransaction(number);

            if (false == bool(transaction)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to load item: ")(
                    number)(".")
                    .Flush();

                continue;
            }
        }

        // TODO This should happen when the box receipt is downloaded
        if (transactionType::chequeReceipt == transaction->GetType()) {
            const auto workflowUpdated =
                api_.Workflow().ClearCheque(context.Nym()->ID(), *transaction);

            if (workflowUpdated) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Updated workflow.")
                    .Flush();
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to update workflow.")
                    .Flush();
            }
        }

        const bool accepted = api_.OTAPI().IncludeResponse(
            accountID, true, context, *transaction, *response);

        if (false == accepted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to accept item: ")(
                number)
                .Flush();

            return false;
        }
    }

    const bool finalized = api_.OTAPI().FinalizeProcessInbox(
        accountID, context, *response, *inbox, *outbox);

    if (false == finalized) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to finalize response.")
            .Flush();

        return false;
    }

    auto message = construct_process_inbox(accountID, *response, context);

    if (false == bool(message)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct command")
            .Flush();

        return false;
    }

    auto result = context.Queue(api_, message, {});

    while (false == bool(result)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Context is busy").Flush();
        Log::Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));
        result = context.Queue(api_, message, {});
    }

    while (check_future(*result)) {
        if (shutdown_.get()) { return false; }
    }

    lastResult = result->get();
    const auto [status, reply] = lastResult;

    if (proto::LASTREPLYSTATUS_MESSAGESUCCESS != status) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to deliver processInbox ")(
            status)
            .Flush();

        return false;
    }

    cleanup.SetSuccess();

    OT_ASSERT(reply);

    const auto success = evaluate_transaction_reply(accountID, *reply);

    if (success) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Success processing inbox ")(
            accountID)
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure processing inbox ")(
            accountID)
            .Flush();
    }

    return success;
}

bool Operation::PublishContract(const identifier::Nym& id)
{
    START()

    target_nym_id_ = id;

    return start(Type::PublishNym, {});
}

bool Operation::PublishContract(const identifier::Server& id)
{
    START()

    target_server_id_ = id;

    return start(Type::PublishServer, {});
}

bool Operation::PublishContract(const identifier::UnitDefinition& id)
{
    START()

    target_unit_id_ = id;

    return start(Type::PublishUnit, {});
}

void Operation::refresh()
{
    OT_ASSERT(message_);

    context().It().UpdateRequestNumber(*message_);
}

bool Operation::RequestAdmin(const String& password)
{
    START()

    memo_ = password;

    return start(Type::RequestAdmin, {});
}

void Operation::reset()
{
    state_.store(State::NymboxPre);
    refresh_account_.store(false);
    message_.reset();
    outmail_message_.reset();
    result_set_.store(false);
    result_ = Promise{};
    target_nym_id_ = identifier::Nym::Factory();
    target_server_id_ = identifier::Server::Factory();
    target_unit_id_ = identifier::UnitDefinition::Factory();
    contract_type_ = ContractType::ERROR;
    unit_definition_.reset();
    account_id_ = Identifier::Factory();
    generic_id_ = Identifier::Factory();
    amount_ = 0;
    memo_ = String::Factory();
    bool_ = false;
    claim_section_ = proto::CONTACTSECTION_ERROR;
    claim_type_ = proto::CITEMTYPE_ERROR;
    cheque_.reset();
    payment_.reset();
    inbox_.reset();
    outbox_.reset();
#if OT_CASH
    purse_.reset();
#endif
    affected_accounts_.clear();
    redownload_accounts_.clear();
    numbers_.clear();
    error_count_ = 0;
    peer_reply_.reset();
    peer_request_.reset();
    set_id_ = {};
}

#if OT_CASH
bool Operation::SendCash(
    const identifier::Nym& recipientID,
    const Identifier& workflowID)
{
    const auto pSender = api_.Wallet().Nym(nym_id_);
    const auto pRecipient = api_.Wallet().Nym(recipientID);

    if (false == bool(pRecipient)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load recipient nym")
            .Flush();

        return false;
    }

    if (false == bool(pSender)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load sender nym")
            .Flush();

        return false;
    }

    const auto& recipient = *pRecipient;
    const auto& sender = *pSender;
    auto pWorkflow = api_.Workflow().LoadWorkflow(nym_id_, workflowID);

    if (false == bool(pWorkflow)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load workflow").Flush();

        return false;
    }

    const auto& workflow = *pWorkflow;
    auto [state, pPurse] =
        api::client::Workflow::InstantiatePurse(api_, workflow);

    if (proto::PAYMENTWORKFLOWSTATE_UNSENT != state) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect workflow state")
            .Flush();

        return false;
    }

    if (false == bool(pPurse)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Purse not found").Flush();

        return false;
    }

    auto& purse = *pPurse;

    if (false == purse.Unlock(sender)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to unlock pursed").Flush();

        return false;
    }

    if (false == purse.AddNym(recipient)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to encrypt purse to recipient")
            .Flush();

        return false;
    }

    START()

    target_nym_id_ = recipientID;
    generic_id_ = workflowID;
#if OT_CASH
    purse_ = std::move(pPurse);
#endif
    return start(Type::SendCash, {});
}
#endif
bool Operation::SendMessage(
    const identifier::Nym& recipient,
    const String& message,
    const SetID setID)
{
    START()

    target_nym_id_ = recipient;
    memo_ = message;
    set_id_ = setID;

    return start(Type::SendMessage, {});
}

bool Operation::SendPeerReply(
    const identifier::Nym& targetNymID,
    const std::shared_ptr<const PeerReply> peerreply,
    const std::shared_ptr<const PeerRequest> peerrequest)
{
    START()

    target_nym_id_ = targetNymID;
    peer_reply_ = peerreply;
    peer_request_ = peerrequest;

    return start(Type::SendPeerReply, {});
}

bool Operation::SendPeerRequest(
    const identifier::Nym& targetNymID,
    const std::shared_ptr<const PeerRequest> peerrequest)
{
    START()

    target_nym_id_ = targetNymID;
    peer_request_ = peerrequest;

    return start(Type::SendPeerRequest, {});
}

bool Operation::SendTransfer(
    const Identifier& sourceAccountID,
    const Identifier& destinationAccountID,
    const Amount amount,
    const String& memo)
{
    START()

    account_id_ = sourceAccountID;
    generic_id_ = destinationAccountID;
    amount_ = amount;
    memo_ = memo;
    affected_accounts_.insert(sourceAccountID);

    return start(Type::SendTransfer, {});
}

void Operation::set_consensus_hash(
    OTTransaction& transaction,
    const Context& context,
    const Account& account)
{
    auto accountHash{Identifier::Factory()};
    auto accountid{Identifier::Factory()};
    auto inboxHash{Identifier::Factory()};
    auto outboxHash{Identifier::Factory()};
    account.GetIdentifier(accountid);
    account.ConsensusHash(context, accountHash);
    auto nymfile = context.Nymfile(__FUNCTION__);
    nymfile->GetInboxHash(accountid->str(), inboxHash);
    nymfile->GetOutboxHash(accountid->str(), outboxHash);
    transaction.SetAccountHash(accountHash);
    transaction.SetInboxHash(inboxHash);
    transaction.SetOutboxHash(outboxHash);
}

void Operation::set_result(ServerContext::DeliveryResult&& result)
{
    result_set_.store(true);
    result_.set_value(std::move(result));
}

bool Operation::Start(const Type type, const ServerContext::ExtraArgs& args)
{
    START()

    switch (type) {
        case Type::GetTransactionNumbers:
        case Type::RegisterNym: {
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect arguments").Flush();

            return false;
        }
    }

    return start(type, args);
}

bool Operation::Start(
    const Type type,
    const identifier::UnitDefinition& targetUnitID,
    const ServerContext::ExtraArgs& args)
{
    START()

    switch (type) {
        case Type::DownloadMint:
        case Type::RegisterAccount: {
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect arguments").Flush();

            return false;
        }
    }

    target_unit_id_ = targetUnitID;

    return start(type, args);
}

bool Operation::Start(
    const Type type,
    const identifier::Nym& targetNymID,
    const ServerContext::ExtraArgs& args)
{
    START()

    switch (type) {
        case Type::CheckNym: {
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect arguments").Flush();

            return false;
        }
    }

    target_nym_id_ = targetNymID;

    return start(type, args);
}

bool Operation::start(const Type type, const ServerContext::ExtraArgs& args)
{
    type_.store(type);
    args_ = args;

    if (Type::RefreshAccount == type) { refresh_account_.store(true); }

    return push_->Push("");
}

void Operation::state_machine()
{
    struct Cleanup {
        Operation& parent_;
        bool continue_{false};

        Cleanup(Operation& parent)
            : parent_(parent)
        {
        }

        ~Cleanup()
        {
            if (false == continue_) { parent_.state_.store(State::Idle); }

            if (false == (parent_.result_set_.load() || continue_)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error: state machine termination with dangling promise")
                    .Flush();

                OT_FAIL;
            }
        }
    };

    Cleanup cleanup(*this);

    switch (state_.load()) {
        case State::NymboxPre: {
            nymbox_pre();
        } break;
        case State::AccountPre: {
            account_pre();
        } break;
        case State::TransactionNumbers: {
            transaction_numbers();
        } break;
        case State::Execute: {
            execute();
        } break;
        case State::AccountPost: {
            account_post();
        } break;
        case State::NymboxPost: {
            nymbox_post();
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected state").Flush();
        }
    }

    Lock lock(shutdown_lock_);
    const auto error = error_count_ > MAX_ERROR_COUNT;

    if (error) { set_result({proto::LASTREPLYSTATUS_UNKNOWN, nullptr}); }

    const bool run = (false == shutdown_.get()) &&
                     (State::Idle != state_.load()) && (false == error);

    if (run) {
        cleanup.continue_ = true;
        push_->Push("");
    } else {
        running_.store(false);
    }
}

void Operation::transaction_numbers()
{
    switch (category_.at(type_.load())) {
        case Category::UpdateAccount:
        case Category::Transaction: {
            break;
        }
        case Category::Invalid:
        case Category::Basic:
        case Category::NymboxPost:
        case Category::NymboxPre:
        case Category::CreateAccount:
        default: {
            state_.store(State::Execute);

            return;
        }
    }

    PREPARE_CONTEXT();

    const auto need = transaction_numbers_.at(type_.load());

    if (context.AvailableNumbers() >= need) {
        state_.store(State::AccountPre);

        return;
    }

    std::shared_ptr<Message> message{
        api_.OTAPI().getTransactionNumbers(context)};

    if (false == bool(message)) { return; }

    auto result = context.Queue(api_, message, {});

    if (false == bool(result)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Context is busy").Flush();
        Log::Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));

        return;
    }

    while (check_future(*result)) {
        if (shutdown_.get()) { return; }
    }

    if (proto::LASTREPLYSTATUS_MESSAGESUCCESS == std::get<0>(result->get())) {
        auto nymbox = context.RefreshNymbox(api_);

        if (false == bool(nymbox)) {
            LogTrace(OT_METHOD)(__FUNCTION__)(": Context is busy").Flush();
            Log::Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));
            nymbox = context.RefreshNymbox(api_);
        }

        while (check_future(*nymbox)) {
            if (shutdown_.get()) { return; }
        }

        [[maybe_unused]] auto done = nymbox->get();
        state_.store(State::NymboxPre);
    }
}

void Operation::update_workflow(
    const Message& request,
    const ServerContext::DeliveryResult& result) const
{
    switch (type_.load()) {
        case Type::ConveyPayment: {
            update_workflow_convey_payment(request, result);
        } break;
#if OT_CASH
        case Type::SendCash: {
            update_workflow_send_cash(request, result);
        } break;
#endif
        default: {
        }
    }
}

void Operation::update_workflow_convey_payment(
    const Message& request,
    const ServerContext::DeliveryResult& result) const
{
    const auto& [status, reply] = result;

    OT_ASSERT(payment_);

    if (false == payment_->IsCheque()) {
        // TODO Handle other payment types once the workflow api handles them

        return;
    }

    bool workflowUpdated{false};
    auto pCheque{api_.Factory().Cheque()};

    OT_ASSERT(pCheque);

    auto& cheque = *pCheque;
    const auto loaded = cheque.LoadContractFromString(payment_->Payment());

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load cheque.").Flush();

        return;
    }

    if (const_cast<OTPayment&>(*payment_).IsCancelledCheque()) {
        workflowUpdated =
            api_.Workflow().CancelCheque(cheque, request, reply.get());
    } else {
        workflowUpdated =
            api_.Workflow().SendCheque(cheque, request, reply.get());
    }

    if (workflowUpdated) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Successfully updated workflow.")
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to update workflow.")
            .Flush();
    }
}

#if OT_CASH
void Operation::update_workflow_send_cash(
    const Message& request,
    const ServerContext::DeliveryResult& result) const
{
    api_.Workflow().SendCash(
        nym_id_, target_nym_id_, generic_id_, request, result.second.get());
}
#endif

bool Operation::UpdateAccount(const Identifier& accountID)
{
    START()

    affected_accounts_.insert(accountID);
    redownload_accounts_.clear();

    return start(Type::RefreshAccount, {});
}

#if OT_CASH
bool Operation::WithdrawCash(const Identifier& accountID, const Amount amount)
{
    START()

    account_id_ = accountID;
    amount_ = amount;
    affected_accounts_.insert(accountID);

    return start(Type::WithdrawCash, {});
}
#endif

Operation::~Operation()
{
    Lock lock(shutdown_lock_);
    shutdown_->On();
    lock.unlock();
    push_->Close();
    pull_->Close();
    context().It().Join();
}
}  // namespace opentxs::api::client::implementation
