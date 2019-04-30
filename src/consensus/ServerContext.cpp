// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#if OT_CASH
#include "opentxs/blind/Mint.hpp"
#include "opentxs/blind/Purse.hpp"
#include "opentxs/blind/Token.hpp"
#endif  // OT_CASH
#include "opentxs/client/Helpers.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTClient.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/consensus/ManagedNumber.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/consensus/TransactionStatement.hpp"
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NymFile.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/otx/Reply.hpp"

#include "core/StateMachine.hpp"
#include "Context.hpp"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <iterator>

#include "ServerContext.hpp"

#define START()                                                                \
    Lock lock(decision_lock_);                                                 \
                                                                               \
    if (running().load()) {                                                    \
        LogDebug(OT_METHOD)(__FUNCTION__)(                                     \
            ": State machine is already running.")                             \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }

#define CURRENT_VERSION 3
#define PENDING_COMMAND_VERSION 1
#define DEFAULT_NODE_NAME "Stash Node Pro"
#define NYMBOX_BOX_TYPE 0
#define FAILURE_COUNT_LIMIT 3

#define OT_METHOD "opentxs::implementation::ServerContext::"

namespace opentxs
{
internal::ServerContext* Factory::ServerContext(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& requestSent,
    const network::zeromq::PublishSocket& replyReceived,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Server& server,
    network::ServerConnection& connection)
{
    return new implementation::ServerContext(
        api, requestSent, replyReceived, local, remote, server, connection);
}

internal::ServerContext* Factory::ServerContext(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& requestSent,
    const network::zeromq::PublishSocket& replyReceived,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    network::ServerConnection& connection)
{
    return new implementation::ServerContext(
        api, requestSent, replyReceived, serialized, local, remote, connection);
}
}  // namespace opentxs

namespace opentxs::implementation
{
const std::string ServerContext::default_node_name_{DEFAULT_NODE_NAME};
const std::set<MessageType> ServerContext::do_not_need_request_number_{
    MessageType::pingNotary,
    MessageType::registerNym,
    MessageType::getRequestNumber,
};

ServerContext::ServerContext(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& requestSent,
    const network::zeromq::PublishSocket& replyReceived,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Server& server,
    network::ServerConnection& connection)
    : Signable(local, CURRENT_VERSION)
    , implementation::Context(api, CURRENT_VERSION, local, remote, server)
    , StateMachine(std::bind(&ServerContext::state_machine, this))
    , request_sent_(requestSent)
    , reply_received_(replyReceived)
    , client_(nullptr)
    , connection_(connection)
    , admin_password_("")
    , admin_attempted_(Flag::Factory(false))
    , admin_success_(Flag::Factory(false))
    , revision_(0)
    , highest_transaction_number_(0)
    , tentative_transaction_numbers_()
    , state_(proto::DELIVERTYSTATE_IDLE)
    , last_status_(proto::LASTREPLYSTATUS_NONE)
    , pending_message_()
    , pending_args_("", false)
    , pending_result_()
    , pending_result_set_(false)
    , process_nymbox_(false)
    , failure_counter_(0)
    , inbox_()
    , outbox_()
    , numbers_(nullptr)
    , find_nym_(api.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
    , find_server_(api.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
    , find_unit_definition_(
          api.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
{
    init_sockets();
}

ServerContext::ServerContext(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& requestSent,
    const network::zeromq::PublishSocket& replyReceived,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    network::ServerConnection& connection)
    : Signable(local, CURRENT_VERSION)
    , implementation::Context(
          api,
          CURRENT_VERSION,
          serialized,
          local,
          remote,
          api.Factory().ServerID(serialized.servercontext().serverid()))
    , StateMachine(std::bind(&ServerContext::state_machine, this))
    , request_sent_(requestSent)
    , reply_received_(replyReceived)
    , client_(nullptr)
    , connection_(connection)
    , admin_password_(serialized.servercontext().adminpassword())
    , admin_attempted_(
          Flag::Factory(serialized.servercontext().adminattempted()))
    , admin_success_(Flag::Factory(serialized.servercontext().adminsuccess()))
    , revision_(serialized.servercontext().revision())
    , highest_transaction_number_(
          serialized.servercontext().highesttransactionnumber())
    , tentative_transaction_numbers_()
    , state_(serialized.servercontext().state())
    , last_status_(serialized.servercontext().laststatus())
    , pending_message_(instantiate_message(
          api,
          serialized.servercontext().pending().serialized()))
    , pending_args_(
          serialized.servercontext().pending().accountlabel(),
          serialized.servercontext().pending().resync())
    , pending_result_()
    , pending_result_set_(false)
    , enable_otx_push_(true)
    , failure_counter_(0)
    , inbox_()
    , outbox_()
    , numbers_(nullptr)
    , find_nym_(api.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
    , find_server_(api.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
    , find_unit_definition_(
          api.ZeroMQ().PushSocket(zmq::Socket::Direction::Connect))
{
    for (const auto& it : serialized.servercontext().tentativerequestnumber()) {
        tentative_transaction_numbers_.insert(it);
    }

    if (3 > serialized.version()) {
        state_.store(proto::DELIVERTYSTATE_IDLE);
        last_status_.store(proto::LASTREPLYSTATUS_NONE);
    }

    init_sockets();
}

bool ServerContext::accept_entire_nymbox(
    const Lock& lock,
    const api::client::Manager& client,
    Ledger& nymbox,
    Message& output,
    ReplyNoticeOutcomes& notices,
    std::size_t& alreadySeenNotices)
{
    alreadySeenNotices = 0;
    const auto& nym = *nym_;
    const auto& nymID = nym.ID();

    if (nymbox.GetTransactionCount() < 1) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Nymbox is empty.").Flush();

        return false;
    }

    if (false == nymbox.VerifyAccount(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid nymbox").Flush();

        return false;
    }

    if (nymbox.GetNymID() != nymID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong nymbox").Flush();

        return false;
    }

    TransactionNumber lStoredTransactionNumber{0};
    auto processLedger = api_.Factory().Ledger(nymID, nymID, server_id_);

    OT_ASSERT(processLedger);

    processLedger->GenerateLedger(nymID, server_id_, ledgerType::message);
    std::shared_ptr<OTTransaction> acceptTransaction{api_.Factory().Transaction(
        nymID,
        nymID,
        server_id_,
        transactionType::processNymbox,
        originType::not_applicable,
        lStoredTransactionNumber)};

    OT_ASSERT(acceptTransaction);

    processLedger->AddTransaction(acceptTransaction);
    TransactionNumbers verifiedNumbers{};
    TransactionNumbers setNoticeNumbers;

    for (const auto& it : nymbox.GetTransactionMap()) {
        OT_ASSERT(it.second);

        auto& transaction = *it.second;

        if (transaction.IsAbbreviated() &&
            (transaction.GetType() != transactionType::replyNotice)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Unexpected abbreviated receipt in Nymbox, even after "
                "supposedly loading all box receipts. (And it's not a "
                "replyNotice, either)!")
                .Flush();

            continue;
        }

        auto strRespTo = String::Factory();
        transaction.GetReferenceString(strRespTo);

        switch (transaction.GetType()) {
            case transactionType::message: {
                make_accept_item(
                    itemType::acceptMessage, transaction, *acceptTransaction);
            } break;
            case transactionType::instrumentNotice: {
                make_accept_item(
                    itemType::acceptNotice, transaction, *acceptTransaction);
            } break;
            case transactionType::notice: {
                make_accept_item(
                    itemType::acceptNotice, transaction, *acceptTransaction);
            } break;
            case transactionType::successNotice: {
                verify_success(lock, transaction, setNoticeNumbers);
                make_accept_item(
                    itemType::acceptNotice, transaction, *acceptTransaction);
            } break;
            case transactionType::replyNotice: {
                const bool seen = verify_acknowledged_number(
                    lock, transaction.GetRequestNum());

                if (seen) {
                    ++alreadySeenNotices;
                } else {
                    auto item = transaction.GetItem(itemType::replyNotice);

                    if (item) {
                        process_unseen_reply(lock, client, *item, notices);
                    } else {
                        LogNormal(OT_METHOD)(__FUNCTION__)(
                            ": Missing reply notice item")
                            .Flush();
                    }

                    make_accept_item(
                        itemType::acceptNotice,
                        transaction,
                        *acceptTransaction);
                }
            } break;
            case transactionType::blank: {
                verify_blank(lock, transaction, verifiedNumbers);
                make_accept_item(
                    itemType::acceptTransaction,
                    transaction,
                    *acceptTransaction,
                    verifiedNumbers);
            } break;
            case transactionType::finalReceipt: {
                const auto number = transaction.GetReferenceToNum();
                const bool removed = consume_issued(lock, number);

                if (removed) {
                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": **** Due to finding a finalReceipt, consuming "
                        "Nym's issued opening number: ")(number)
                        .Flush();
                } else {
                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": **** Noticed a finalReceipt, but Opening Number ")(
                        number)(" had ALREADY been consumed from nym.")
                        .Flush();
                }

                OTCronItem::EraseActiveCronReceipt(
                    api_.DataFolder(), number, nymID, server_id_);
                make_accept_item(
                    itemType::acceptFinalReceipt,
                    transaction,
                    *acceptTransaction);
            } break;
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Not accepting item ")(
                    transaction.GetTransactionNum())(", type ")(
                    transaction.GetTypeString())
                    .Flush();
            }
        }
    }

    const auto acceptedItems = acceptTransaction->GetItemCount();

    if ((1 > acceptedItems) && (1 > alreadySeenNotices)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Nothing to accept.").Flush();

        return false;
    }

    for (auto& number : setNoticeNumbers) {
        accept_issued_number(lock, number);
    }

    bool ready{true};
    nymbox.ReleaseTransactions();

    for (const auto& number : verifiedNumbers) {
        add_tentative_number(lock, number);
    }

    std::shared_ptr<Item> balanceItem{
        statement(lock, *acceptTransaction, verifiedNumbers)};

    OT_ASSERT(balanceItem)

    acceptTransaction->AddItem(balanceItem);

    OT_ASSERT((acceptedItems + 1) == acceptTransaction->GetItemCount())

    ready &= acceptTransaction->SignContract(nym);

    OT_ASSERT(ready)

    ready &= acceptTransaction->SaveContract();

    OT_ASSERT(ready)

    ready &= processLedger->SignContract(nym);

    OT_ASSERT(ready)

    ready &= processLedger->SaveContract();

    OT_ASSERT(ready)

    const auto serialized = String::Factory(*processLedger);
    initialize_server_command(
        lock, MessageType::processNymbox, -1, true, true, output);
    ready &= output.m_ascPayload->SetString(serialized);
    finalize_server_command(output);

    OT_ASSERT(ready)

    return true;
}

void ServerContext::accept_numbers(
    const Lock& lock,
    OTTransaction& transaction,
    OTTransaction& replyTransaction)
{
    auto item = transaction.GetItem(itemType::transactionStatement);

    if (false == bool(item)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Strange... found transaction in ledger , but didn't find a "
            "transactionStatement item within.")
            .Flush();

        return;
    }

    if (false == replyTransaction.GetSuccess()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Found the receipt you're talking about, but the Server's Reply "
            "transaction says ")("FAILED.")
            .Flush();

        return;
    }

    auto serialized = String::Factory();
    item->GetAttachment(serialized);

    if (false == serialized->Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Strange... found transaction item in ledger, but didn't find "
            "statement within.")
            .Flush();

        return;
    }

    TransactionStatement statement(serialized);
    accept_issued_number(lock, statement);
}

bool ServerContext::accept_issued_number(
    const Lock& lock,
    const TransactionNumber& number)
{
    OT_ASSERT(verify_write_lock(lock));

    bool accepted = false;
    const bool tentative = remove_tentative_number(lock, number);

    if (tentative) { accepted = issue_number(lock, number); }

    return accepted;
}

bool ServerContext::AcceptIssuedNumber(const TransactionNumber& number)
{
    Lock lock(lock_);

    return accept_issued_number(lock, number);
}

bool ServerContext::accept_issued_number(
    const Lock& lock,
    const TransactionStatement& statement)
{
    OT_ASSERT(verify_write_lock(lock));

    std::size_t added = 0;
    const auto offered = statement.Issued().size();

    if (0 == offered) { return false; }

    TransactionNumbers adding, accepted, rejected;

    for (const auto& number : statement.Issued()) {
        // If number wasn't already on issued list, then add to BOTH
        // lists. Otherwise do nothing (it's already on the issued list,
        // and no longer valid on the available list--thus shouldn't be
        // re-added thereanyway.)
        const bool tentative =
            (1 == tentative_transaction_numbers_.count(number));
        const bool issued = (1 == issued_transaction_numbers_.count(number));

        if (tentative && !issued) { adding.insert(number); }
    }

    // Looks like we found some numbers to accept (tentative numbers we had
    // already been waiting for, yet hadn't processed onto our issued list yet)
    if (!adding.empty()) {
        update_highest(lock, adding, accepted, rejected);

        // We only remove-tentative-num/add-transaction-num for the numbers
        // that were above our 'last highest number'. The contents of rejected
        // are thus ignored for these purposes.
        for (const auto& number : accepted) {
            tentative_transaction_numbers_.erase(number);

            if (issue_number(lock, number)) { added++; }
        }
    }

    return (added == offered);
}

bool ServerContext::AcceptIssuedNumbers(const TransactionStatement& statement)
{
    Lock lock(lock_);

    return accept_issued_number(lock, statement);
}

std::vector<OTIdentifier> ServerContext::Accounts() const
{
    std::vector<OTIdentifier> output{};
    const auto serverSet = api_.Storage().AccountsByServer(server_id_);
    const auto nymSet = api_.Storage().AccountsByOwner(nym_->ID());
    std::set_intersection(
        serverSet.begin(),
        serverSet.end(),
        nymSet.begin(),
        nymSet.end(),
        std::back_inserter(output));

    return output;
}

bool ServerContext::add_item_to_payment_inbox(
    const TransactionNumber number,
    const std::string& payment) const
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    auto paymentInbox = load_or_create_payment_inbox();

    if (false == bool(paymentInbox)) { return false; }

    // I create the client-side-created instrumentNotice using the same
    // transaction number that was already on the box receipt where it came
    // from. Meaning the server already placed an "transactionType::message"
    // in my Nymbox with Txn # X, so I will create the corresponding
    // instrumentNotice for my Payments Inbox using Txn # X as well.
    // After all, if the notary had created it (as normally happens) then
    // that's the Txn# that would have been on it anyway.
    std::shared_ptr<OTTransaction> transaction{api_.Factory().Transaction(
        *paymentInbox,
        transactionType::instrumentNotice,
        originType::not_applicable,
        number)};

    OT_ASSERT(transaction);

    transaction->SetReferenceToNum(number);
    transaction->SetReferenceString(String::Factory(payment));
    transaction->SignContract(nym);
    transaction->SaveContract();
    add_transaction_to_ledger(number, transaction, *paymentInbox);

    return true;
}

bool ServerContext::add_item_to_workflow(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& transportItem,
    const std::string& item) const
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    auto message = api_.Factory().Message();

    OT_ASSERT(message);

    const auto loaded =
        message->LoadContractFromString(String::Factory(item.c_str()));

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate message.")
            .Flush();

        return false;
    }

    OTEnvelope envelope(message->m_ascPayload);
    auto plaintext = String::Factory();
    const auto decrypted = envelope.Open(nym, plaintext);

    if (false == decrypted) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt message.")
            .Flush();

        return false;
    }

    auto payment = api_.Factory().Payment(plaintext);

    if (false == bool(payment)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid payment").Flush();

        return false;
    }

    if (false == payment->IsCheque()) { return false; }

    if (payment->IsCancelledCheque()) { return false; }

    auto pCheque = api_.Factory().Cheque();

    OT_ASSERT(pCheque);

    auto& cheque = *pCheque;
    cheque.LoadContractFromString(payment->Payment());
    // The sender nym and notary of the cheque may not match the sender nym and
    // notary of the message which conveyed the cheque.
    find_nym_->Push(cheque.GetSenderNymID().str());
    find_server_->Push(cheque.GetNotaryID().str());
    find_unit_definition_->Push(cheque.GetInstrumentDefinitionID().str());

    // We already made sure a contact exists for the sender of the message, but
    // it's possible the sender of the cheque is a different nym
    client.Contacts().NymToContact(cheque.GetSenderNymID());
    const auto workflow =
        client.Workflow().ReceiveCheque(nym.ID(), cheque, transportItem);

    if (workflow->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create workflow.")
            .Flush();

        return false;
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Started workflow ")(workflow)
            .Flush();
    }

    return true;
}

bool ServerContext::add_tentative_number(
    const Lock& lock,
    const TransactionNumber& number)
{
    OT_ASSERT(verify_write_lock(lock));

    if (number < highest_transaction_number_.load()) { return false; }

    auto output = tentative_transaction_numbers_.insert(number);

    return output.second;
}

bool ServerContext::add_transaction_to_ledger(
    const TransactionNumber number,
    std::shared_ptr<OTTransaction> transaction,
    Ledger& ledger) const
{
    OT_ASSERT(nym_);

    if (nullptr != ledger.GetTransaction(number)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Transaction already exists")
            .Flush();

        return true;
    }

    if (false == ledger.AddTransaction(transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to add transaction to ledger")
            .Flush();

        return false;
    }

    ledger.ReleaseSignatures();
    ledger.SignContract(*nym_);
    ledger.SaveContract();
    bool saved{false};

    switch (ledger.GetType()) {
        case ledgerType::paymentInbox: {
            saved = ledger.SavePaymentInbox();
        } break;
        case ledgerType::recordBox: {
            saved = ledger.SaveRecordBox();
        } break;
        case ledgerType::expiredBox: {
            saved = ledger.SaveExpiredBox();
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected ledger type: ")(
                ledger.GetTypeString())
                .Flush();

            return false;
        }
    }

    if (false == saved) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save ")(
            ledger.GetTypeString())
            .Flush();

        return false;
    }

    if (false == transaction->SaveBoxReceipt(ledger)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save box receipt")
            .Flush();

        return false;
    }

    return true;
}

bool ServerContext::AddTentativeNumber(const TransactionNumber& number)
{
    Lock lock(lock_);

    return add_tentative_number(lock, number);
}

bool ServerContext::AdminAttempted() const { return admin_attempted_.get(); }

const std::string& ServerContext::AdminPassword() const
{
    Lock lock(lock_);

    return admin_password_;
}

NetworkReplyMessage ServerContext::attempt_delivery(
    const Lock& contextLock,
    const Lock& messageLock,
    const api::client::Manager& client,
    Message& message)
{
    request_sent_.Publish(message.m_strCommand->Get());
    auto output = connection_.Send(
        *this,
        message,
        static_cast<opentxs::network::ServerConnection::Push>(
            enable_otx_push_.load()));
    auto& [status, reply] = output;
    const auto needRequestNumber =
        need_request_number(Message::Type(message.m_strCommand->Get()));

    switch (status) {
        case SendResult::VALID_REPLY: {
            OT_ASSERT(reply);

            reply_received_.Publish(message.m_strCommand->Get());
            static std::set<OTManagedNumber> empty{};
            std::set<OTManagedNumber>* numbers = numbers_;

            if (nullptr == numbers) { numbers = &empty; }

            OT_ASSERT(nullptr != numbers);

            process_reply(contextLock, client, *numbers, *reply);

            if (reply->m_bSuccess) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Success delivering ")(
                    message.m_strCommand)
                    .Flush();

                return output;
            }

            if (false == needRequestNumber) { break; }

            bool sent{false};
            auto number = update_request_number(contextLock, messageLock, sent);

            if ((0 == number) || (false == sent)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unable to resync request number")
                    .Flush();
                status = SendResult::TIMEOUT;
                reply.reset();

                return output;
            } else {
                LogVerbose(OT_METHOD)(__FUNCTION__)(
                    ": Success resyncing request number ")(message.m_strCommand)
                    .Flush();
            }

            const auto updated = update_request_number(contextLock, message);

            if (false == updated) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to update ")(
                    message.m_strCommand)(" with new request number")
                    .Flush();
                status = SendResult::TIMEOUT;
                reply.reset();
                ++failure_counter_;

                return output;
            } else {
                LogVerbose(OT_METHOD)(__FUNCTION__)(
                    ": Success updating request number on ")(
                    message.m_strCommand)
                    .Flush();
            }

            output = connection_.Send(
                *this,
                message,
                static_cast<opentxs::network::ServerConnection::Push>(
                    enable_otx_push_.load()));

            if (SendResult::VALID_REPLY == status) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Success delivering ")(
                    message.m_strCommand)(" (second attempt)")
                    .Flush();
                process_reply(contextLock, client, {}, *reply);

                return output;
            }
        } break;
        case SendResult::TIMEOUT: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Timeout delivering ")(
                message.m_strCommand)
                .Flush();
            ++failure_counter_;
        } break;
        case SendResult::INVALID_REPLY: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid reply to ")(
                message.m_strCommand)
                .Flush();
            ++failure_counter_;
        } break;
        case SendResult::ERROR: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Malformed ")(
                message.m_strCommand)
                .Flush();
            ++failure_counter_;
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown error").Flush();
            ++failure_counter_;
        }
    }

    ++failure_counter_;

    return output;
}

const identifier::Nym& ServerContext::client_nym_id(const Lock& lock) const
{
    OT_ASSERT(nym_);

    return nym_->ID();
}

network::ServerConnection& ServerContext::Connection() { return connection_; }

bool ServerContext::create_instrument_notice_from_peer_object(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& message,
    const PeerObject& peerObject,
    const TransactionNumber number) const
{
    OT_ASSERT(proto::PEEROBJECT_PAYMENT == peerObject.Type());

    if (false == peerObject.Validate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid peer object.").Flush();

        return false;
    }

    const auto& payment = *peerObject.Payment();

    if (payment.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Payment as received was apparently empty. Maybe the sender "
            "sent it that way?")
            .Flush();

        return false;
    }

    // Extract the OTPayment so that we know whether to use the new Workflow
    // code or the old payment inbox code
    if (add_item_to_workflow(lock, client, message, payment)) {

        return true;
    } else {

        return add_item_to_payment_inbox(number, payment);
    }
}

std::shared_ptr<OTTransaction> ServerContext::extract_box_receipt(
    const String& serialized,
    const identity::Nym& signer,
    const identifier::Nym& owner,
    const TransactionNumber target)
{
    if (false == serialized.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid input").Flush();

        return {};
    }

    std::shared_ptr<OTTransactionType> transaction{
        api_.Factory().Transaction(serialized)};

    if (false == bool(transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate transaction")
            .Flush();

        return {};
    }

    auto receipt = std::dynamic_pointer_cast<OTTransaction>(transaction);

    if (false == bool(receipt)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction type")
            .Flush();

        return {};
    }

    if (false == receipt->VerifyAccount(signer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid receipt").Flush();

        return {};
    }

    if ((0 != target) && (receipt->GetTransactionNum() != target)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect transaction number")
            .Flush();

        return {};
    }

    if (receipt->GetNymID() != owner) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid nym").Flush();

        return {};
    }

    return receipt;
}

std::unique_ptr<Ledger> ServerContext::extract_ledger(
    const Armored& armored,
    const Identifier& accountID,
    const identity::Nym& signer) const
{
    OT_ASSERT(nym_);

    if (false == armored.Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Error: empty input").Flush();

        return {};
    }

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto output = api_.Factory().Ledger(nymID, accountID, server_id_);

    OT_ASSERT(output);

    auto serialized = String::Factory();
    armored.GetString(serialized);

    if (false == output->LoadLedgerFromString(serialized)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Error: Failed to instantiate ledger")
            .Flush();

        return {};
    }

    if (false == output->VerifySignature(signer)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Error: Invalid signature")
            .Flush();

        return {};
    }

    return output;
}

std::unique_ptr<Message> ServerContext::extract_message(
    const Armored& armored,
    const identity::Nym& signer) const
{
    auto output = api_.Factory().Message();

    OT_ASSERT(output);

    if (false == armored.Exists()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Error: empty input").Flush();

        return {};
    }

    auto serialized = String::Factory();
    armored.GetString(serialized);

    if (false == output->LoadContractFromString(serialized)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Error: Failed to instantiate message")
            .Flush();

        return {};
    }

    if (false == output->VerifySignature(signer)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Error: Invalid signature")
            .Flush();

        return {};
    }

    return output;
}

ServerContext::TransactionNumbers ServerContext::extract_numbers(
    OTTransaction& input)
{
    TransactionNumbers output{};
    NumList list{};
    input.GetNumList(list);
    list.Output(output);

    return output;
}

std::unique_ptr<Item> ServerContext::extract_original_item(
    const Item& response) const
{
    auto serialized = String::Factory();
    response.GetReferenceString(serialized);

    if (serialized->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Input item does not contain reference string")
            .Flush();

        return {};
    }

    auto transaction = api_.Factory().Transaction(serialized);

    if (false == bool(transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate serialized item")
            .Flush();

        return {};
    }

    std::unique_ptr<Item> output{dynamic_cast<Item*>(transaction.get())};

    if (output) {
        transaction.release();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Reference string is not a serialized item")
            .Flush();
    }

    return output;
}

std::unique_ptr<Item> ServerContext::extract_transfer(
    const OTTransaction& receipt) const
{
    if (transactionType::transferReceipt == receipt.GetType()) {

        return extract_transfer_receipt(receipt);
    } else if (transactionType::pending == receipt.GetType()) {

        return extract_transfer_pending(receipt);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect receipt type: ")(
            receipt.GetTypeString())
            .Flush();

        return nullptr;
    }
}

std::unique_ptr<Item> ServerContext::extract_transfer_pending(
    const OTTransaction& receipt) const
{
    if (transactionType::pending != receipt.GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect receipt type: ")(
            receipt.GetTypeString())
            .Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    receipt.GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing serialized transfer item")
            .Flush();

        return nullptr;
    }

    auto transfer = api_.Factory().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate transfer item")
            .Flush();

        return nullptr;
    }

    if (itemType::transfer != transfer->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer item type.")
            .Flush();

        return nullptr;
    }

    return transfer;
}

std::unique_ptr<Item> ServerContext::extract_transfer_receipt(
    const OTTransaction& receipt) const
{
    auto serializedAcceptPending = String::Factory();
    receipt.GetReferenceString(serializedAcceptPending);

    if (serializedAcceptPending->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Missing serialized accept pending item")
            .Flush();

        return nullptr;
    }

    const auto acceptPending = api_.Factory().Item(serializedAcceptPending);

    if (false == bool(acceptPending)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate accept pending item")
            .Flush();

        return nullptr;
    }

    if (itemType::acceptPending != acceptPending->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid accept pending item type.")
            .Flush();

        return nullptr;
    }

    auto serializedPending = String::Factory();
    acceptPending->GetAttachment(serializedPending);

    if (serializedPending->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Missing serialized pending transaction")
            .Flush();

        return nullptr;
    }

    auto pending = api_.Factory().Transaction(
        receipt.GetNymID(),
        receipt.GetRealAccountID(),
        receipt.GetRealNotaryID());

    if (false == bool(pending)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate pending transaction")
            .Flush();

        return nullptr;
    }

    const bool loaded = pending->LoadContractFromString(serializedPending);

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to deserialize pending transaction")
            .Flush();

        return nullptr;
    }

    if (transactionType::pending != pending->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid pending transaction type.")
            .Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    pending->GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing serialized transfer item")
            .Flush();

        return nullptr;
    }

    auto transfer = api_.Factory().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to instantiate transfer item")
            .Flush();

        return nullptr;
    }

    if (itemType::transfer != transfer->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer item type.")
            .Flush();

        return nullptr;
    }

    return transfer;
}

bool ServerContext::finalize_server_command(Message& command) const
{
    OT_ASSERT(nym_);

    if (false == command.SignContract(*nym_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign server message.")
            .Flush();

        return false;
    }

    if (false == command.SaveContract()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to serialize server message.")
            .Flush();

        return false;
    }

    return true;
}

bool ServerContext::FinalizeServerCommand(Message& command) const
{
    return finalize_server_command(command);
}

std::unique_ptr<TransactionStatement> ServerContext::generate_statement(
    const Lock& lock,
    const TransactionNumbers& adding,
    const TransactionNumbers& without) const
{
    OT_ASSERT(verify_write_lock(lock));

    TransactionNumbers issued;
    TransactionNumbers available;

    for (const auto& number : issued_transaction_numbers_) {
        const bool include = (0 == without.count(number));

        if (include) {
            issued.insert(number);
            available.insert(number);
        }
    }

    for (const auto& number : adding) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Accepting number ")(number)
            .Flush();
        issued.insert(number);
        available.insert(number);
    }

    std::unique_ptr<TransactionStatement> output(new TransactionStatement(
        String::Factory(server_id_)->Get(), issued, available));

    return output;
}

ServerContext::Exit ServerContext::get_item_type(
    OTTransaction& input,
    itemType& output)
{
    switch (input.GetType()) {
        case transactionType::atDeposit: {
            output = itemType::atDeposit;
        } break;
        case transactionType::atWithdrawal: {
            auto cash = input.GetItem(itemType::atWithdrawal);
            auto voucher = input.GetItem(itemType::atWithdrawVoucher);

            if (cash) {
                output = itemType::atWithdrawal;
            } else if (voucher) {
                output = itemType::atWithdrawVoucher;
            }
        } break;
        case transactionType::atPayDividend: {
            output = itemType::atPayDividend;
        } break;
        case transactionType::atTransfer: {
            output = itemType::atTransfer;
        } break;
        case transactionType::atMarketOffer: {
            output = itemType::atMarketOffer;
        } break;
        case transactionType::atPaymentPlan: {
            output = itemType::atPaymentPlan;
        } break;
        case transactionType::atSmartContract: {
            output = itemType::atSmartContract;
        } break;
        case transactionType::atCancelCronItem: {
            output = itemType::atCancelCronItem;
        } break;
        case transactionType::atExchangeBasket: {
            output = itemType::atExchangeBasket;
        } break;
        default:
        case transactionType::atProcessInbox: {
            return Exit::Yes;
        }
    }

    return Exit::Continue;
}

ServerContext::BoxType ServerContext::get_type(const std::int64_t depth)
{
    switch (depth) {
        case 0: {
            return BoxType::Nymbox;
        }
        case 1: {
            return BoxType::Inbox;
        }
        case 2: {
            return BoxType::Outbox;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown box type: ")(depth)
                .Flush();
        }
    }

    return BoxType::Invalid;
}

bool ServerContext::harvest_unused(
    const Lock& lock,
    const api::client::Manager& client)
{
    OT_ASSERT(verify_write_lock(lock));
    OT_ASSERT(nym_);

    bool output{true};
    const auto& nymID = nym_->ID();
    auto available = issued_transaction_numbers_;
    const auto workflows = client.Storage().PaymentWorkflowList(nymID.str());
    std::set<proto::PaymentWorkflowState> keepStates{};

    // Loop through workflows to determine which issued numbers should not be
    // harvested
    for (const auto& [id, alias] : workflows) {
        const auto workflowID = api_.Factory().Identifier(id);
        const auto workflow = client.Workflow().LoadWorkflow(nymID, workflowID);

        if (false == bool(workflow)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load workflow ")(
                workflowID)(".")
                .Flush();

            continue;
        }

        switch (workflow->type()) {
            case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
            case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE: {
                keepStates.insert(proto::PAYMENTWORKFLOWSTATE_UNSENT);
                keepStates.insert(proto::PAYMENTWORKFLOWSTATE_CONVEYED);
            } break;
            case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER: {
                keepStates.insert(proto::PAYMENTWORKFLOWSTATE_INITIATED);
                keepStates.insert(proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED);
            } break;
            case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
                keepStates.insert(proto::PAYMENTWORKFLOWSTATE_INITIATED);
                keepStates.insert(proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED);
                keepStates.insert(proto::PAYMENTWORKFLOWSTATE_CONVEYED);
            } break;
            case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
            case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
            case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
                continue;
            }
            case proto::PAYMENTWORKFLOWTYPE_ERROR:
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Warning: Unhandled workflow type.")
                    .Flush();
                output &= false;
                continue;
            }
        }

        if (0 == keepStates.count(workflow->state())) { continue; }

        // At this point, this workflow contains a transaction whose
        // number(s) must not be added to the available list (recovered).

        switch (workflow->type()) {
            case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
            case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE: {
                [[maybe_unused]] auto [state, cheque] =
                    api::client::Workflow::InstantiateCheque(api_, *workflow);

                if (false == bool(cheque)) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failed to load cheque.")
                        .Flush();

                    continue;
                }

                const auto number = cheque->GetTransactionNum();
                available.erase(number);
            } break;
            case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
            case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
                [[maybe_unused]] auto [state, pTransfer] =
                    api::client::Workflow::InstantiateTransfer(api_, *workflow);

                if (false == bool(pTransfer)) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failed to load transfer.")
                        .Flush();

                    continue;
                }

                const auto& transfer = *pTransfer;
                const auto number = transfer.GetTransactionNum();
                available.erase(number);
            } break;
            case proto::PAYMENTWORKFLOWTYPE_ERROR:
            case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
            case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
            case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Warning: unhandled workflow type.")
                    .Flush();
                output &= false;
                continue;
            }
        }
    }

    // Any numbers which remain in available are not allocated and should
    // be returned to the available list.
    for (const auto& number : available) {
        if (false == verify_available_number(lock, number)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Restoring number ")(number)(
                ".")
                .Flush();
            recover_available_number(lock, number);
        }
    }

    return output;
}

bool ServerContext::HaveAdminPassword() const
{
    return false == admin_password_.empty();
}

bool ServerContext::HaveSufficientNumbers(const MessageType reason) const
{
	if (MessageType::processInbox == reason) {
		return 0 < available_transaction_numbers_.size();
	}
	
	return 1 < available_transaction_numbers_.size();
}

TransactionNumber ServerContext::Highest() const
{
    return highest_transaction_number_.load();
}

bool ServerContext::init_new_account(const Identifier& accountID)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    auto account = api_.Wallet().mutable_Account(accountID);

    OT_ASSERT(account);

    if (false == account.get().InitBoxes(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error initializing boxes for account ")(accountID)(".")
            .Flush();

        return false;
    }

    auto inboxHash = api_.Factory().Identifier();
    auto outboxHash = api_.Factory().Identifier();
    auto haveHash = account.get().GetInboxHash(inboxHash);

    if (false == haveHash) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get inbox hash.")
            .Flush();

        return false;
    }

    haveHash = account.get().GetOutboxHash(outboxHash);

    if (false == haveHash) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get outbox hash.")
            .Flush();

        return false;
    }

    account.Release();
    auto nymfile = mutable_Nymfile("");
    auto hashSet = nymfile.It().SetInboxHash(accountID.str(), inboxHash);

    if (false == hashSet) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to set inbox hash on nymfile.")
            .Flush();

        return false;
    }

    hashSet = nymfile.It().SetOutboxHash(accountID.str(), outboxHash);

    if (false == hashSet) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to set outbox hash on nymfile.")
            .Flush();

        return false;
    }

    return true;
}

void ServerContext::init_sockets()
{
    const auto endpoint =
        std::string("inproc://") + Identifier::Random()->str();
    auto started = find_nym_->Start(api_.Endpoints().FindNym());

    if (false == started) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to start find nym socket ")
            .Flush();

        OT_FAIL;
    }

    started = find_server_->Start(api_.Endpoints().FindServer());

    if (false == started) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to start find server socket ")
            .Flush();

        OT_FAIL;
    }

    started =
        find_unit_definition_->Start(api_.Endpoints().FindUnitDefinition());

    if (false == started) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to start find unit socket ")
            .Flush();

        OT_FAIL;
    }
}

std::unique_ptr<Message> ServerContext::initialize_server_command(
    const MessageType type) const
{
    auto output = api_.Factory().Message();

    OT_ASSERT(output);

    initialize_server_command(type, *output);

    return output;
}

void ServerContext::initialize_server_command(
    const MessageType type,
    Message& output) const
{
    OT_ASSERT(nym_);

    output.ReleaseSignatures();
    output.m_strCommand->Set(Message::Command(type).data());
    output.m_strNymID = String::Factory(nym_->ID());
    output.m_strNotaryID = String::Factory(server_id_);
}

RequestNumber ServerContext::initialize_server_command(
    const Lock& lock,
    const MessageType type,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash,
    Message& output)
{
    OT_ASSERT(verify_write_lock(lock));

    RequestNumber number{0};
    initialize_server_command(type, output);

    if (-1 == provided) {
        number = request_number_++;
    } else {
        number = provided;
    }

    output.m_strRequestNum = String::Factory(std::to_string(number).c_str());

    if (withAcknowledgments) {
        output.SetAcknowledgments(acknowledged_request_numbers_);
    }

    if (withNymboxHash) {
        local_nymbox_hash_->GetString(output.m_strNymboxHash);
    }

    return number;
}

std::pair<RequestNumber, std::unique_ptr<Message>> ServerContext::
    initialize_server_command(
        const Lock& lock,
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash)
{
    OT_ASSERT(verify_write_lock(lock));

    std::pair<RequestNumber, std::unique_ptr<Message>> output{
        0, api_.Factory().Message()};
    auto& [requestNumber, message] = output;

    OT_ASSERT(message);

    requestNumber = initialize_server_command(
        lock, type, provided, withAcknowledgments, withNymboxHash, *message);

    return output;
}

std::pair<RequestNumber, std::unique_ptr<Message>> ServerContext::
    InitializeServerCommand(
        const MessageType type,
        const Armored& payload,
        const Identifier& accountID,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash)
{
    Lock lock(lock_);
    auto output = initialize_server_command(
        lock, type, provided, withAcknowledgments, withNymboxHash);
    auto& [requestNumber, message] = output;
    const auto& notUsed [[maybe_unused]] = requestNumber;

    message->m_ascPayload = payload;
    message->m_strAcctID = String::Factory(accountID);

    return output;
}

std::pair<RequestNumber, std::unique_ptr<Message>> ServerContext::
    InitializeServerCommand(
        const MessageType type,
        const identifier::Nym& recipientNymID,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash)
{
    Lock lock(lock_);
    auto output = initialize_server_command(
        lock, type, provided, withAcknowledgments, withNymboxHash);
    [[maybe_unused]] auto& [requestNumber, message] = output;
    message->m_strNymID2 = String::Factory(recipientNymID);

    return output;
}

std::pair<RequestNumber, std::unique_ptr<Message>> ServerContext::
    InitializeServerCommand(
        const MessageType type,
        const RequestNumber provided,
        const bool withAcknowledgments,
        const bool withNymboxHash)
{
    Lock lock(lock_);

    return initialize_server_command(
        lock, type, provided, withAcknowledgments, withNymboxHash);
}

std::unique_ptr<opentxs::Message> ServerContext::instantiate_message(
    const api::Core& api,
    const std::string& serialized)
{
    if (serialized.empty()) { return {}; }

    auto output = api.Factory().Message();

    OT_ASSERT(output);

    const auto loaded =
        output->LoadContractFromString(String::Factory(serialized));

    if (false == loaded) { return {}; }

    return output;
}

bool ServerContext::isAdmin() const { return admin_success_.get(); }

bool ServerContext::is_internal_transfer(const Item& item) const
{
    if (itemType::transfer != item.GetType()) {
        throw std::runtime_error("Not a transfer item");
    }

    const auto& source = item.GetPurportedAccountID();
    const auto& destination = item.GetDestinationAcctID();

    if (source.empty()) {
        throw std::runtime_error("Missing source account id");
    }

    if (destination.empty()) {
        throw std::runtime_error("Missing destination account id");
    }

    auto sourceOwner = api_.Storage().AccountOwner(source);
    auto destinationOwner = api_.Storage().AccountOwner(destination);

    return sourceOwner == destinationOwner;
}

void ServerContext::Join() { Wait().get(); }

const Item& ServerContext::make_accept_item(
    const itemType type,
    const OTTransaction& input,
    OTTransaction& acceptTransaction,
    const TransactionNumbers& accept)
{
    std::shared_ptr<Item> acceptItem{api_.Factory().Item(
        acceptTransaction, type, api_.Factory().Identifier())};

    OT_ASSERT(acceptItem);

    acceptTransaction.AddItem(acceptItem);
    acceptItem->SetReferenceToNum(input.GetTransactionNum());

    if (false == accept.empty()) {
        acceptItem->AddBlankNumbersToItem(NumList{accept});
    }

    acceptItem->SignContract(*nym_);
    acceptItem->SaveContract();

    return *acceptItem;
}

std::unique_ptr<Ledger> ServerContext::load_account_inbox(
    const Identifier& accountID) const
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto inbox = api_.Factory().Ledger(nymID, accountID, server_id_);

    OT_ASSERT(inbox);

    bool output = OTDB::Exists(
        api_.DataFolder(),
        OTFolders::Inbox().Get(),
        server_id_->str().c_str(),
        accountID.str().c_str(),
        "");

    if (output && inbox->LoadInbox()) {
        output = inbox->VerifyAccount(nym);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load inbox").Flush();
    }

    if (output) { return inbox; }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to verify inbox").Flush();

    return {};
}

std::unique_ptr<Ledger> ServerContext::load_or_create_account_recordbox(
    const Identifier& accountID) const
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto recordBox = api_.Factory().Ledger(nymID, accountID, server_id_);

    OT_ASSERT(recordBox);

    bool output = OTDB::Exists(
        api_.DataFolder(),
        OTFolders::RecordBox().Get(),
        server_id_->str().c_str(),
        accountID.str().c_str(),
        "");

    if (false == output) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Creating recordbox").Flush();
        output = recordBox->GenerateLedger(
            accountID, server_id_, ledgerType::recordBox, true);
        recordBox->ReleaseSignatures();
        output &= recordBox->SignContract(nym);
        output &= recordBox->SaveContract();
        output &= recordBox->SaveRecordBox();
    }

    if (output && recordBox->LoadRecordBox()) {
        output &= recordBox->VerifyContractID();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load recordbox")
            .Flush();

        return {};
    }

    if (output && recordBox->VerifySignature(nym)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Recordbox verified").Flush();

        return recordBox;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to verify recordbox").Flush();

    return {};
}

std::unique_ptr<Ledger> ServerContext::load_or_create_payment_inbox() const
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto paymentInbox = api_.Factory().Ledger(nymID, nymID, server_id_);

    OT_ASSERT(paymentInbox);

    bool output = OTDB::Exists(
        api_.DataFolder(),
        OTFolders::PaymentInbox().Get(),
        server_id_->str().c_str(),
        nymID.str().c_str(),
        "");

    if (false == output) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Creating payment inbox").Flush();
        output = paymentInbox->GenerateLedger(
            nymID, server_id_, ledgerType::paymentInbox, true);
        paymentInbox->ReleaseSignatures();
        output &= paymentInbox->SignContract(nym);
        output &= paymentInbox->SaveContract();
        output &= paymentInbox->SavePaymentInbox();
    }

    if (output && paymentInbox->LoadPaymentInbox()) {
        output &= paymentInbox->VerifyContractID();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load payment inbox")
            .Flush();

        return {};
    }

    if (output && paymentInbox->VerifySignature(nym)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Payment inbox verified").Flush();

        return paymentInbox;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to verify payment inbox")
        .Flush();

    return {};
}

#if OT_CASH
Editor<blind::Purse> ServerContext::mutable_Purse(
    const identifier::UnitDefinition& id)
{
    return api_.Wallet().mutable_Purse(nym_->ID(), server_id_, id);
}
#endif

void ServerContext::need_box_items(const api::client::Manager& client)
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);
    auto nymbox{api_.Factory().Ledger(
        nym_->ID(), nym_->ID(), server_id_, ledgerType::nymbox)};

    OT_ASSERT(nymbox);

    const auto loaded = nymbox->LoadNymbox();

    if (false == loaded) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load nymbox").Flush();

        return;
    }

    const auto verified = nymbox->VerifyAccount(*nym_);

    if (false == verified) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to verify nymbox").Flush();

        return;
    }

    std::size_t have{0};

    for (const auto& [number, transaction] : nymbox->GetTransactionMap()) {
        if (1 > number) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Invalid index").Flush();

            continue;
        }

        if (false == bool(transaction)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Invalid transaction").Flush();

            continue;
        }

        const auto exists = VerifyBoxReceiptExists(
            api_.DataFolder(),
            server_id_,
            nym_->ID(),
            nym_->ID(),
            NYMBOX_BOX_TYPE,
            number);

        if (exists) {
            ++have;

            continue;
        }

        [[maybe_unused]] auto [requestNumber, message] =
            initialize_server_command(
                contextLock, MessageType::getBoxReceipt, -1, false, false);

        OT_ASSERT(message);

        message->m_strAcctID = String::Factory(nym_->ID());
        message->m_lDepth = NYMBOX_BOX_TYPE;
        message->m_lTransactionNum = number;
        const auto finalized = FinalizeServerCommand(*message);

        OT_ASSERT(finalized);

        auto result =
            attempt_delivery(contextLock, messageLock, client, *message);

        switch (result.first) {
            case SendResult::SHUTDOWN: {
                return;
            }
            case SendResult::VALID_REPLY: {
                OT_ASSERT(result.second);

                if (result.second->m_bSuccess) {
                    ++have;

                    continue;
                } else {
                    // Downloading a box receipt shouldn't fail. If it does, the
                    // only reasonable option is to download the nymbox again.
                    update_state(contextLock, proto::DELIVERTYSTATE_NEEDNYMBOX);
                }

                [[fallthrough]];
            }
            case SendResult::TIMEOUT:
            case SendResult::INVALID_REPLY:
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error downloading box item")
                    .Flush();

                return;
            }
        }
    }

    if (nymbox->GetTransactionMap().size() == have) {
        update_state(contextLock, proto::DELIVERTYSTATE_NEEDPROCESSNYMBOX);
    }
}

void ServerContext::need_nymbox(const api::client::Manager& client)
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);
    [[maybe_unused]] auto [number, message] = initialize_server_command(
        contextLock, MessageType::getNymbox, -1, true, true);

    OT_ASSERT(message);

    const auto finalized = FinalizeServerCommand(*message);

    OT_ASSERT(finalized);

    auto result = attempt_delivery(contextLock, messageLock, client, *message);

    switch (result.first) {
        case SendResult::SHUTDOWN: {
            return;
        }
        case SendResult::VALID_REPLY: {
            OT_ASSERT(result.second);
            auto& reply = *result.second;

            if (reply.m_bSuccess) {
                if (process_nymbox_.load() && false == bool(pending_message_)) {
                    pending_message_ = result.second;
                }

                update_state(contextLock, proto::DELIVERTYSTATE_NEEDBOXITEMS);

                return;
            }

            [[fallthrough]];
        }
        case SendResult::TIMEOUT:
        case SendResult::INVALID_REPLY:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error downloading nymbox")
                .Flush();

            return;
        }
    }
}

void ServerContext::need_process_nymbox(const api::client::Manager& client)
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);
    auto nymbox{api_.Factory().Ledger(
        nym_->ID(), nym_->ID(), server_id_, ledgerType::nymbox)};

    OT_ASSERT(nymbox);

    const auto loaded = nymbox->LoadNymbox();

    if (false == loaded) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to load nymbox").Flush();

        return;
    }

    const auto verified = nymbox->VerifyAccount(*nym_);

    if (false == verified) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Unable to verify nymbox").Flush();

        return;
    }

    const auto count = nymbox->GetTransactionCount();

    if (1 > count) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Nymbox is empty (so, skipping processNymbox).")
            .Flush();

        if (process_nymbox_) {
            DeliveryResult result{proto::LASTREPLYSTATUS_MESSAGESUCCESS,
                                  pending_message_};
            resolve_queue(
                contextLock, std::move(result), proto::DELIVERTYSTATE_IDLE);
        } else {
            // The server never received the original message.
            update_state(contextLock, proto::DELIVERTYSTATE_PENDINGSEND);
        }

        return;
    }

    auto message{api_.Factory().Message()};

    OT_ASSERT(message);

    ReplyNoticeOutcomes outcomes{};
    std::size_t alreadySeen{0};
    const auto accepted = accept_entire_nymbox(
        contextLock, client, *nymbox, *message, outcomes, alreadySeen);

    if (pending_message_) {
        const RequestNumber targetNumber =
            String::StringToUlong(pending_message_->m_strRequestNum->Get());

        for (auto& [number, status] : outcomes) {
            if (number == targetNumber) {
                resolve_queue(contextLock, std::move(status));
            }
        }
    }

    if (false == accepted) {
        if (alreadySeen == static_cast<std::size_t>(count)) {
            if (process_nymbox_) {
                DeliveryResult result{proto::LASTREPLYSTATUS_MESSAGESUCCESS,
                                      pending_message_};
                resolve_queue(
                    contextLock, std::move(result), proto::DELIVERTYSTATE_IDLE);
            } else {
                // The server never received the original message.
                update_state(contextLock, proto::DELIVERTYSTATE_PENDINGSEND);
            }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to accept the "
                "entire Nymbox. (And no, it's not empty).")
                .Flush();
            update_state(contextLock, proto::DELIVERTYSTATE_NEEDNYMBOX);
        }

        return;
    }

    local_nymbox_hash_->GetString(message->m_strNymboxHash);

    if (false == finalize_server_command(*message)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to finalize server message.")
            .Flush();

        return;
    }

    auto result = attempt_delivery(contextLock, messageLock, client, *message);

    switch (result.first) {
        case SendResult::SHUTDOWN: {
            return;
        }
        case SendResult::VALID_REPLY: {
            auto& message = result.second;

            OT_ASSERT(message);

            if (message->m_bSuccess) {
                [[maybe_unused]] auto [number, message] =
                    initialize_server_command(
                        contextLock, MessageType::getNymbox, -1, true, true);

                OT_ASSERT(message);

                const auto finalized = FinalizeServerCommand(*message);

                OT_ASSERT(finalized);

                auto again = attempt_delivery(
                    contextLock, messageLock, client, *message);

                if (process_nymbox_) {
                    DeliveryResult result{proto::LASTREPLYSTATUS_MESSAGESUCCESS,
                                          pending_message_};
                    resolve_queue(
                        contextLock,
                        std::move(result),
                        proto::DELIVERTYSTATE_IDLE);
                } else {
                    // The server never received the original message.
                    update_state(
                        contextLock, proto::DELIVERTYSTATE_PENDINGSEND);
                }

                return;
            }

            [[fallthrough]];
        }
        case SendResult::TIMEOUT:
        case SendResult::INVALID_REPLY:
        default: {
            // If processing a nymbox fails, then it must have changed since
            // the last time we downloaded it. Also if the reply was dropped
            // we might have actually processed it without realizig it.
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error processing nymbox")
                .Flush();
            update_state(contextLock, proto::DELIVERTYSTATE_NEEDNYMBOX);

            return;
        }
    }
}

bool ServerContext::need_request_number(const MessageType type)
{
    return 0 == do_not_need_request_number_.count(type);
}

OTManagedNumber ServerContext::next_transaction_number(
    const Lock& lock,
    const MessageType reason)
{
    OT_ASSERT(verify_write_lock(lock));

    const std::size_t reserve = (MessageType::processInbox == reason) ? 0 : 1;

    if (0 == reserve) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Allocating a transaction number for process inbox.")
            .Flush();
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Allocating a transaction number for normal transaction.")
            .Flush();
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(
        available_transaction_numbers_.size())(" numbers available.")
        .Flush();
    LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(
        issued_transaction_numbers_.size())(" numbers issued.")
        .Flush();

    if (reserve >= available_transaction_numbers_.size()) {

        return OTManagedNumber(Factory::ManagedNumber(0, *this));
    }

    auto first = available_transaction_numbers_.begin();
    const auto output = *first;
    available_transaction_numbers_.erase(first);

    return OTManagedNumber(Factory::ManagedNumber(output, *this));
}

OTManagedNumber ServerContext::NextTransactionNumber(const MessageType reason)
{
    Lock lock(lock_);

    return next_transaction_number(lock, reason);
}

void ServerContext::pending_send(const api::client::Manager& client)
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);

    OT_ASSERT(pending_message_);

    auto result =
        attempt_delivery(contextLock, messageLock, client, *pending_message_);
    const auto needRequestNumber = need_request_number(
        Message::Type(pending_message_->m_strCommand->Get()));

    switch (result.first) {
        case SendResult::SHUTDOWN: {
            return;
        }
        case SendResult::VALID_REPLY: {
            OT_ASSERT(result.second);

            DeliveryResult output{};
            auto& [status, message] = output;
            message = result.second;
            const auto& reply = *message;

            if (reply.m_bSuccess) {
                status = proto::LASTREPLYSTATUS_MESSAGESUCCESS;
            } else {
                status = proto::LASTREPLYSTATUS_MESSAGEFAILED;
            }

            resolve_queue(
                contextLock, std::move(output), proto::DELIVERTYSTATE_IDLE);

        } break;
        case SendResult::TIMEOUT:
        case SendResult::INVALID_REPLY: {
            if (needRequestNumber) {
                update_state(
                    contextLock,
                    proto::DELIVERTYSTATE_NEEDNYMBOX,
                    proto::LASTREPLYSTATUS_UNKNOWN);
            }
        } break;
        default: {
        }
    }
}

NetworkReplyMessage ServerContext::PingNotary()
{
    Lock lock(message_lock_);

    OT_ASSERT(nym_);

    auto request = initialize_server_command(MessageType::pingNotary);

    if (false == bool(request)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to initialize server message.")
            .Flush();

        return {};
    }

    auto serializedAuthKey = String::Factory();
    auto serializedEncryptKey = String::Factory();
    const auto& authKey = nym_->GetPublicAuthKey();
    const auto& encrKey = nym_->GetPublicEncrKey();
    authKey.GetPublicKey(serializedAuthKey);
    encrKey.GetPublicKey(serializedEncryptKey);
    request->m_strRequestNum =
        String::Factory(std::to_string(FIRST_REQUEST_NUMBER).c_str());
    request->m_strNymPublicKey = serializedAuthKey;
    request->m_strNymID2 = serializedEncryptKey;
    request->keytypeAuthent_ = authKey.keyType();
    request->keytypeEncrypt_ = encrKey.keyType();

    if (false == finalize_server_command(*request)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to finalize server message.")
            .Flush();

        return {};
    }

    return connection_.Send(
        *this,
        *request,
        static_cast<opentxs::network::ServerConnection::Push>(
            enable_otx_push_.load()));
}

bool ServerContext::ProcessNotification(
    const api::client::Manager& client,
    const otx::Reply& notification)
{
    OT_ASSERT(nym_);

    Lock lock(lock_);
    const auto pPush = notification.Push();

    if (false == bool(pPush)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing push payload").Flush();

        return false;
    }

    auto& push = *pPush;

    switch (push.type()) {
        case proto::OTXPUSH_NYMBOX: {
            // Nymbox items don't have an intrinsic account ID. Use nym ID
            // instead.
            return process_box_item(lock, client, nym_->ID(), push);
        } break;
        case proto::OTXPUSH_INBOX: {
            return process_account_push(lock, client, push);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported push type")
                .Flush();

            return false;
        }
    }
}

void ServerContext::process_accept_basket_receipt_reply(
    const Lock& lock,
    const OTTransaction& inboxTransaction)
{
    const auto number = inboxTransaction.GetClosingNum();
    LogVerbose(OT_METHOD)(__FUNCTION__)(
        "Successfully removed basketReceipt with closing number: ")(number)
        .Flush();
    consume_issued(lock, number);
}

void ServerContext::process_accept_cron_receipt_reply(
    const Lock& lock,
    const Identifier& accountID,
    OTTransaction& inboxTransaction)
{
    auto pServerItem = inboxTransaction.GetItem(itemType::marketReceipt);

    if (false == bool(pServerItem)) {
        // Only marketReceipts have required action here.

        return;
    }

    auto strOffer = String::Factory(), strTrade = String::Factory();
    // contains updated offer.
    pServerItem->GetAttachment(strOffer);
    // contains updated trade.
    pServerItem->GetNote(strTrade);
    auto theOffer = api_.Factory().Offer();

    OT_ASSERT((theOffer));

    auto theTrade = api_.Factory().Trade();

    OT_ASSERT((theTrade));

    api_.Factory().Trade();
    bool bLoadOfferFromString = theOffer->LoadContractFromString(strOffer);
    bool bLoadTradeFromString = theTrade->LoadContractFromString(strTrade);

    if (bLoadOfferFromString && bLoadTradeFromString) {
        std::unique_ptr<OTDB::TradeDataNym> pData(
            dynamic_cast<OTDB::TradeDataNym*>(
                OTDB::CreateObject(OTDB::STORED_OBJ_TRADE_DATA_NYM)));
        OT_ASSERT((pData));

        std::int64_t lScale = theOffer->GetScale();

        // TransID for original offer.
        // (Offer may trade many times.)
        pData->transaction_id =
            to_string<std::int64_t>(theTrade->GetTransactionNum());
        // TransID for BOTH receipts for current trade.
        // (Asset/Currency.)
        pData->updated_id =
            to_string<std::int64_t>(pServerItem->GetTransactionNum());
        pData->completed_count =
            to_string<std::int32_t>(theTrade->GetCompletedCount());
        auto account = api_.Wallet().Account(accountID);

        OT_ASSERT(account)

        bool bIsAsset =
            (theTrade->GetInstrumentDefinitionID() ==
             account.get().GetInstrumentDefinitionID());
        bool bIsCurrency =
            (theTrade->GetCurrencyID() ==
             account.get().GetInstrumentDefinitionID());
        const auto strAcctID = String::Factory(accountID);
        const auto strServerTransaction = String::Factory(inboxTransaction);

        if (bIsAsset) {
            const auto strInstrumentDefinitionID =
                String::Factory(theTrade->GetInstrumentDefinitionID());
            std::int64_t lAssetsThisTrade = pServerItem->GetAmount();
            pData->instrument_definition_id = strInstrumentDefinitionID->Get();
            // The amount of ASSETS moved, this trade.
            pData->amount_sold = to_string<std::int64_t>(lAssetsThisTrade);
            pData->asset_acct_id = strAcctID->Get();
            pData->asset_receipt = strServerTransaction->Get();
        } else if (bIsCurrency) {
            const auto strCurrencyID =
                String::Factory(theTrade->GetCurrencyID());
            std::int64_t lCurrencyThisTrade = pServerItem->GetAmount();
            pData->currency_id = strCurrencyID->Get();
            pData->currency_paid = to_string<std::int64_t>(lCurrencyThisTrade);
            pData->currency_acct_id = strAcctID->Get();
            pData->currency_receipt = strServerTransaction->Get();
        }

        const time64_t& tProcessDate = inboxTransaction.GetDateSigned();
        pData->date = to_string<time64_t>(tProcessDate);

        // The original offer price. (Might
        // be 0, if it's a market order.)
        //
        const std::int64_t& lPriceLimit = theOffer->GetPriceLimit();
        pData->offer_price = to_string<std::int64_t>(lPriceLimit);
        const std::int64_t& lFinishedSoFar = theOffer->GetFinishedSoFar();
        pData->finished_so_far = to_string<std::int64_t>(lFinishedSoFar);
        pData->scale = to_string<std::int64_t>(lScale);
        pData->is_bid = theOffer->IsBid();

        // save to local storage...
        auto strNymID = String::Factory(nym_->ID());
        std::unique_ptr<OTDB::TradeListNym> pList;

        if (OTDB::Exists(
                api_.DataFolder(),
                OTFolders::Nym().Get(),
                "trades",  // todo stop
                           // hardcoding.
                server_id_->str().c_str(),
                strNymID->Get()))
            pList.reset(dynamic_cast<OTDB::TradeListNym*>(OTDB::QueryObject(
                OTDB::STORED_OBJ_TRADE_LIST_NYM,
                api_.DataFolder(),
                OTFolders::Nym().Get(),
                "trades",  // todo stop
                // hardcoding.
                server_id_->str().c_str(),
                strNymID->Get())));
        if (false == bool(pList)) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                "Creating storage list of trade ")("receipts for Nym: ")(
                strNymID)
                .Flush();
            pList.reset(dynamic_cast<OTDB::TradeListNym*>(
                OTDB::CreateObject(OTDB::STORED_OBJ_TRADE_LIST_NYM)));
        }
        OT_ASSERT((pList));

        // Loop through and see if we can find one that's
        // ALREADY there. We can match the asset receipt and
        // currency receipt. This way we ensure there is
        // only one in the end, which combines info from
        // both. This also enables us to calculate the sale
        // price!
        //
        bool bWeFoundIt = false;

        size_t nTradeDataNymCount = pList->GetTradeDataNymCount();

        for (size_t nym_count = 0; nym_count < nTradeDataNymCount;
             ++nym_count) {
            OTDB::TradeDataNym* pTradeData = pList->GetTradeDataNym(nym_count);

            if (nullptr == pTradeData) continue;  // Should never happen.

            if (0 == pTradeData->updated_id.compare(pData->updated_id)) {
                // It's a repeat of the same one. (Discard.)
                if ((!pTradeData->instrument_definition_id.empty() &&
                     !pData->instrument_definition_id.empty()) ||
                    (!pTradeData->currency_id.empty() &&
                     !pData->currency_id.empty()))
                    break;
                // Okay looks like one is the asset receipt,
                // and the other is the currency receipt.
                // Therefore let's combine them into
                // pTradeData!
                //
                if (pTradeData->instrument_definition_id.empty()) {
                    pTradeData->instrument_definition_id =
                        pData->instrument_definition_id;
                    pTradeData->amount_sold = pData->amount_sold;
                    pTradeData->asset_acct_id = pData->asset_acct_id;
                    pTradeData->asset_receipt = pData->asset_receipt;
                }
                if (pTradeData->currency_id.empty()) {
                    pTradeData->currency_id = pData->currency_id;
                    pTradeData->currency_paid = pData->currency_paid;
                    pTradeData->currency_acct_id = pData->currency_acct_id;
                    pTradeData->currency_receipt = pData->currency_receipt;
                }
                if (!pTradeData->amount_sold.empty() &&
                    !pTradeData->currency_paid.empty()) {

                    const std::int64_t lAmountSold =
                        String::StringToLong(pTradeData->amount_sold);
                    const std::int64_t lCurrencyPaid =
                        String::StringToLong(pTradeData->currency_paid);

                    // just in case (divide by 0.)
                    if ((lAmountSold != 0) && (lScale != 0)) {
                        const std::int64_t lSalePrice =
                            (lCurrencyPaid / (lAmountSold / lScale));

                        auto strSalePrice = String::Factory();
                        strSalePrice->Format("%" PRId64 "", lSalePrice);

                        pTradeData->price = strSalePrice->Get();
                    }
                }

                bWeFoundIt = true;

                break;
            }
        }

        if (!bWeFoundIt) { pList->AddTradeDataNym(*pData); }

        if (false == OTDB::StoreObject(
                         *pList,
                         api_.DataFolder(),
                         OTFolders::Nym().Get(),
                         "trades",  // todo stop hardcoding.
                         server_id_->str().c_str(),
                         strNymID->Get()))
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed storing list of trades for "
                "Nym. Notary ID: ")(server_id_)(". Nym ID: ")(strNymID)(".")
                .Flush();
    }
}

void ServerContext::process_accept_final_receipt_reply(
    const Lock& lock,
    const OTTransaction& inboxTransaction)
{
    const auto number = inboxTransaction.GetClosingNum();
    const auto referenceNumber = inboxTransaction.GetReferenceToNum();
    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Successfully removed finalReceipt with closing number: ")(number)
        .Flush();
    consume_issued(lock, number);

    if (consume_issued(lock, referenceNumber)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": **** Due to finding a finalReceipt, consuming issued opening "
            "number from nym: ")(referenceNumber)
            .Flush();
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": **** Noticed a finalReceipt, but Opening Number ")(
            referenceNumber)(" had ALREADY been removed from nym.")
            .Flush();
    }

    OTCronItem::EraseActiveCronReceipt(
        api_.DataFolder(),
        inboxTransaction.GetReferenceToNum(),
        nym_->ID(),
        inboxTransaction.GetPurportedNotaryID());
}

void ServerContext::process_accept_item_receipt_reply(
    const Lock& lock,
    const api::client::Manager& client,
    const Identifier& accountID,
    const Message& reply,
    const OTTransaction& inboxTransaction)
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    const auto& nymID = nym_->ID();
    auto serializedOriginal = String::Factory();
    inboxTransaction.GetReferenceString(serializedOriginal);
    auto pOriginalItem = api_.Factory().Item(
        serializedOriginal, server_id_, inboxTransaction.GetReferenceToNum());

    if (false == bool(pOriginalItem)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to load original item from string while "
            "accepting item receipt: ")(serializedOriginal)(".")
            .Flush();

        return;
    }

    auto& originalItem = *pOriginalItem;
    auto originalType = String::Factory();
    originalItem.GetTypeString(originalType);

    switch (originalItem.GetType()) {
        case itemType::depositCheque: {
            auto serialized = String::Factory();
            originalItem.GetAttachment(serialized);
            auto cheque = api_.Factory().Cheque();

            OT_ASSERT(cheque);

            if (false == cheque->LoadContractFromString(serialized)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to deserialize cheque")
                    .Flush();

                break;
            }

            const auto number = cheque->GetTransactionNum();
            consume_issued(lock, number);
        } break;
        case itemType::acceptPending: {
            consume_issued(lock, originalItem.GetNumberOfOrigin());

            auto serialized = String::Factory();
            originalItem.GetAttachment(serialized);

            if (serialized->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Missing attachment")
                    .Flush();

                break;
            }

            const auto transferReceipt = api_.Factory().Transaction(
                remote_nym_->ID(), accountID, server_id_);

            OT_ASSERT(transferReceipt);

            const auto loaded =
                transferReceipt->LoadContractFromString(serialized);

            if (false == loaded) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unable to instantiate transfer receipt")
                    .Flush();

                break;
            }

            const auto pTransfer = extract_transfer(*transferReceipt);

            if (pTransfer) {
                const auto& transfer = *pTransfer;

                if (is_internal_transfer(transfer)) {
                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": Completing internal transfer")
                        .Flush();
                } else {
                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": Completing outgoing transfer")
                        .Flush();
                }

                client.Workflow().CompleteTransfer(
                    nymID, server_id_, *transferReceipt, reply);
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer")
                    .Flush();
            }
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unexpected original item type: ")(originalType)
                .Flush();
        }
    }
}

void ServerContext::process_accept_pending_reply(
    const Lock& lock,
    const api::client::Manager& client,
    const Identifier& accountID,
    const Item& acceptItemReceipt,
    const Message& reply) const
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    if (itemType::acceptPending != acceptItemReceipt.GetType()) {
        auto type = String::Factory();
        acceptItemReceipt.GetTypeString(type);
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid type: ")(type).Flush();

        return;
    }

    auto attachment = String::Factory();
    acceptItemReceipt.GetAttachment(attachment);

    if (attachment->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing attachment").Flush();

        return;
    }

    OT_ASSERT(false == attachment->empty());

    const auto pending =
        api_.Factory().Transaction(remote_nym_->ID(), accountID, server_id_);

    OT_ASSERT(pending);

    const auto loaded = pending->LoadContractFromString(attachment);

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate").Flush();

        return;
    }

    if (transactionType::pending != pending->GetType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong item type").Flush();

        return;
    }

    const auto pTransfer = extract_transfer(*pending);

    if (pTransfer) {
        const auto& transfer = *pTransfer;

        if (false == is_internal_transfer(transfer)) {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Accepting incoming transfer")
                .Flush();
            client.Workflow().AcceptTransfer(
                nym_->ID(), server_id_, *pending, reply);
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer").Flush();
    }
}

bool ServerContext::process_account_data(
    const Lock& lock,
    const Identifier& accountID,
    const String& account,
    const Identifier& inboxHash,
    const String& inbox,
    const Identifier& outboxHash,
    const String& outbox)
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    const auto& nymID = nym_->ID();
    const auto updated = api_.Wallet().UpdateAccount(accountID, *this, account);

    if (updated) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Saved updated account file.")
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save account.").Flush();

        return false;
    }

    if (false == bool(inbox_)) {
        inbox_.reset(
            api_.Factory()
                .Ledger(nymID, accountID, server_id_, ledgerType::inbox)
                .release());
    }

    OT_ASSERT(inbox_);
    OT_ASSERT(ledgerType::inbox == inbox_->GetType());

    if (false == inbox_->LoadInboxFromString(inbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to deserialize inbox")
            .Flush();

        return false;
    }

    if (false == inbox_->VerifySignature(*remote_nym_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid inbox signature").Flush();

        return false;
    }

    auto nymfile = mutable_Nymfile("");

    if (false == inboxHash.empty()) {
        const bool hashSet =
            nymfile.It().SetInboxHash(accountID.str(), inboxHash);

        if (false == hashSet) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed setting inbox hash for account: ")(accountID.str())(
                " to (")(inboxHash)(").")
                .Flush();
        }
    }

    for (auto& [number, pTransaction] : inbox_->GetTransactionMap()) {

        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction ")(
                number)(" in inbox")
                .Flush();

            return false;
        }

        auto& transaction = *pTransaction;

        if (transactionType::finalReceipt == transaction.GetType()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                "*** Removing opening issued number (")(
                transaction.GetReferenceToNum())(
                "), since finalReceipt found when ")(
                "retrieving asset account inbox. ***")
                .Flush();

            if (consume_issued(lock, transaction.GetReferenceToNum())) {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": **** Due to finding a finalReceipt, "
                    "consuming issued opening number from nym:  ")(
                    transaction.GetReferenceToNum())
                    .Flush();
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": **** Noticed a finalReceipt, but issued opening "
                    "number ")(transaction.GetReferenceToNum())(
                    " had ALREADY been consumed from nym.")
                    .Flush();
            }

            OTCronItem::EraseActiveCronReceipt(
                api_.DataFolder(),
                transaction.GetReferenceToNum(),
                nym_->ID(),
                transaction.GetPurportedNotaryID());
        }
    }

    inbox_->ReleaseSignatures();
    inbox_->SignContract(*nym_);
    inbox_->SaveContract();
    inbox_->SaveInbox(api_.Factory().Identifier());

    if (false == bool(outbox_)) {
        outbox_.reset(
            api_.Factory()
                .Ledger(nymID, accountID, server_id_, ledgerType::outbox)
                .release());
    }

    OT_ASSERT(outbox_);
    OT_ASSERT(ledgerType::outbox == outbox_->GetType());

    if (false == outbox_->LoadOutboxFromString(outbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to deserialize outbox")
            .Flush();

        return false;
    }

    if (false == outbox_->VerifySignature(*remote_nym_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid outbox signature")
            .Flush();

        return false;
    }

    if (false == outboxHash.empty()) {
        const bool hashSet =
            nymfile.It().SetOutboxHash(accountID.str(), outboxHash);

        if (false == hashSet) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed setting outbox hash for account: ")(accountID.str())(
                " to (")(outboxHash)(").")
                .Flush();
        }
    }

    outbox_->ReleaseSignatures();
    outbox_->SignContract(*nym_);
    outbox_->SaveContract();
    outbox_->SaveOutbox(api_.Factory().Identifier());

    return true;
}

bool ServerContext::process_account_push(
    const Lock& lock,
    const api::client::Manager& client,
    const proto::OTXPush& push)
{
    const auto accountID = api_.Factory().Identifier(push.accountid());
    const auto inboxHash = api_.Factory().Identifier(push.inboxhash());
    const auto outboxHash = api_.Factory().Identifier(push.outboxhash());
    const auto account = String::Factory(push.account());
    const auto inbox = String::Factory(push.inbox());
    const auto outbox = String::Factory(push.outbox());

    const auto processed = process_account_data(
        lock, accountID, account, inboxHash, inbox, outboxHash, outbox);

    if (processed) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Success saving new account data.")
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure saving new account data.")
            .Flush();

        return false;
    }

    return process_box_item(lock, client, accountID, push);
}

bool ServerContext::process_box_item(
    const Lock& lock,
    const api::client::Manager& client,
    const Identifier& accountID,
    const proto::OTXPush& push)
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    auto& nym = *nym_;
    auto& remoteNym = *remote_nym_;
    auto& nymID = nym.ID();
    BoxType box{BoxType::Invalid};

    switch (push.type()) {
        case proto::OTXPUSH_NYMBOX: {
            box = BoxType::Nymbox;
        } break;
        case proto::OTXPUSH_INBOX: {
            box = BoxType::Inbox;
        } break;
        case proto::OTXPUSH_OUTBOX: {
            box = BoxType::Outbox;
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid box type").Flush();

            return false;
        }
    }

    const auto& payload = push.item();

    if (payload.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing payload").Flush();

        return false;
    }

    std::shared_ptr<OTTransactionType> base{
        api_.Factory().Transaction(String::Factory(payload))};

    if (false == bool(base)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid payload").Flush();

        return false;
    }

    auto receipt = std::dynamic_pointer_cast<OTTransaction>(base);

    if (false == bool(base)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong transaction type").Flush();

        return false;
    }

    if (receipt->GetNymID() != nymID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong nym id on box receipt")
            .Flush();

        return false;
    }

    if (false == receipt->VerifyAccount(remoteNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to verify box receipt")
            .Flush();

        return false;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(
        ": Validated a push notification of type: ")(receipt->GetTypeString())
        .Flush();

    return process_get_box_receipt_response(
        lock,
        client,
        accountID,
        receipt,
        String::Factory(payload.c_str()),
        box);
}

bool ServerContext::process_get_nymbox_response(
    const Lock& lock,
    const Message& reply)
{
    OT_ASSERT(nym_);

    const auto& nymID = nym_->ID();
    auto serialized = String::Factory(reply.m_ascPayload);
    auto nymbox = api_.Factory().Ledger(nymID, nymID, server_id_);

    OT_ASSERT(nymbox);

    if (false == nymbox->LoadNymboxFromString(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading or verifying nymbox")
            .Flush();

        return false;
    }

    auto nymboxHash = api_.Factory().Identifier();
    nymbox->ReleaseSignatures();
    nymbox->SignContract(*nym_);
    nymbox->SaveContract();
    nymbox->SaveNymbox(nymboxHash);
    update_nymbox_hash(lock, reply, UpdateHash::Both);

    return true;
}

bool ServerContext::process_check_nym_response(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply)
{
    update_nymbox_hash(lock, reply);

    if ((false == reply.m_bBool) || (reply.m_ascPayload->empty())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Server ")(server_id_)(
            " does not have nym ")(reply.m_strNymID2)
            .Flush();

        return true;
    }

    auto serialized = proto::DataToProto<proto::CredentialIndex>(
        Data::Factory(reply.m_ascPayload));

    auto nym = client.Wallet().Nym(serialized);

    if (nym) {
        client.Contacts().Update(serialized);

        return true;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": checkNymResponse: Retrieved nym (")(serialized.nymid())(
            ") is invalid.")
            .Flush();
    }

    return false;
}

bool ServerContext::process_get_account_data(
    const Lock& lock,
    const Message& reply)
{
    const auto accountID = api_.Factory().Identifier(reply.m_strAcctID);
    auto serializedAccount = String::Factory();
    auto serializedInbox = String::Factory();
    auto serializedOutbox = String::Factory();

    if (accountID->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid account ID").Flush();

        return false;
    }

    if (false == reply.m_ascPayload->GetString(serializedAccount)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid serialized account")
            .Flush();

        return false;
    }

    if (false == reply.m_ascPayload2->GetString(serializedInbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid serialized inbox")
            .Flush();

        return false;
    }

    if (false == reply.m_ascPayload3->GetString(serializedOutbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid serialized outbox")
            .Flush();

        return false;
    }

    return process_account_data(
        lock,
        accountID,
        serializedAccount,
        api_.Factory().Identifier(reply.m_strInboxHash),
        serializedInbox,
        api_.Factory().Identifier(reply.m_strOutboxHash),
        serializedOutbox);
}

bool ServerContext::process_get_box_receipt_response(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply)
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    update_nymbox_hash(lock, reply);
    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    const auto& serverNym = *remote_nym_;
    const auto type = get_type(reply.m_lDepth);

    if (BoxType::Invalid == type) { return false; }

    auto serialized = String::Factory(reply.m_ascPayload);
    auto boxReceipt = extract_box_receipt(
        serialized, serverNym, nymID, reply.m_lTransactionNum);

    if (false == bool(boxReceipt)) { return false; }

    return process_get_box_receipt_response(
        lock,
        client,
        api_.Factory().Identifier(reply.m_strAcctID),
        boxReceipt,
        serialized,
        type);
}

bool ServerContext::process_get_box_receipt_response(
    const Lock& lock,
    const api::client::Manager& client,
    const Identifier& accountID,
    const std::shared_ptr<OTTransaction> receipt,
    const String& serialized,
    const BoxType type)
{
    OT_ASSERT(nym_);
    OT_ASSERT(receipt);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    bool processInbox{false};

    switch (receipt->GetType()) {
        case transactionType::message: {
            process_incoming_message(lock, client, *receipt);
        } break;
        case transactionType::instrumentNotice:
        case transactionType::instrumentRejection: {
            processInbox = true;
            process_incoming_instrument(receipt);
        } break;
        case transactionType::transferReceipt: {
            processInbox = true;
            const auto pTransfer = extract_transfer(*receipt);

            if (pTransfer) {
                const auto& transfer = *pTransfer;

                if (is_internal_transfer(transfer)) {
                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": Clearing internal transfer")
                        .Flush();
                } else {
                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": Clearing outgoing transfer")
                        .Flush();
                }

                client.Workflow().ClearTransfer(nymID, server_id_, *receipt);
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer")
                    .Flush();
            }
        } break;
        case transactionType::pending: {
            processInbox = true;
            const auto pTransfer = extract_transfer(*receipt);

            if (pTransfer) {
                const auto& transfer = *pTransfer;

                if (is_internal_transfer(transfer)) {
                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": Conveying internal transfer")
                        .Flush();
                    client.Workflow().ConveyTransfer(
                        nymID, server_id_, *receipt);
                } else if (transfer.GetNymID() != nymID) {

                    LogDetail(OT_METHOD)(__FUNCTION__)(
                        ": Conveying incoming transfer")
                        .Flush();
                    client.Workflow().ConveyTransfer(
                        nymID, server_id_, *receipt);
                }
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transfer")
                    .Flush();
            }
        } break;
        case transactionType::chequeReceipt: {
        } break;
        case transactionType::voucherReceipt: {
        } break;
        default: {
        }
    }

    if (false == receipt->SaveBoxReceipt(static_cast<std::int64_t>(type))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save box receipt:")
            .Flush();
        LogOutput(serialized).Flush();

        return false;
    }

    if (processInbox) {
        const auto& otx = client.OTX();

        switch (type) {
            case BoxType::Nymbox: {
                if (otx.AutoProcessInboxEnabled()) {
                    otx.DownloadNymbox(nymID, server_id_);
                }
            } break;
            case BoxType::Inbox:
            case BoxType::Outbox: {
                if (otx.AutoProcessInboxEnabled()) {
                    otx.ProcessInbox(nymID, server_id_, accountID);
                }
            } break;
            default: {
            }
        }
    }

    return true;
}

bool ServerContext::process_get_market_list_response(
    const Lock& lock,
    const Message& reply)
{
    auto marketDatafile = String::Factory();
    marketDatafile->Format("%s", "market_data.bin");

    auto pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase the
    // data file. (So when the file is loaded from storage, it will correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    if (reply.m_lDepth == 0) {
        bool success = storage.EraseValueByKey(
            api_.DataFolder(),
            OTFolders::Market().Get(),   // "markets"
            reply.m_strNotaryID->Get(),  // "markets/<notaryID>"
            marketDatafile->Get(),
            "");  // "markets/<notaryID>/market_data.bin"
        if (!success)
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error erasing market list from market folder: ")(
                marketDatafile)(".")
                .Flush();

        return true;
    }

    auto serialized = Data::Factory();

    if ((reply.m_ascPayload->GetLength() <= 2) ||
        (false == reply.m_ascPayload->GetData(serialized))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to decode ascii-armored "
                                           "payload.")
            .Flush();
        return true;
    }

    // Unpack the market list...

    auto pPacker = storage.GetPacker();

    OT_ASSERT(nullptr != pPacker);

    auto& packer = *pPacker;

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(packer.CreateBuffer());

    OT_ASSERT((pBuffer));

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized->data()),
        serialized->size());

    std::unique_ptr<OTDB::MarketList> pMarketList(
        dynamic_cast<OTDB::MarketList*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_LIST)));

    auto unpacked = pPacker->Unpack(buffer, *pMarketList);

    if (!unpacked) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed unpacking data.").Flush();
        return true;
    }

    bool success = storage.StoreObject(
        *pMarketList,
        api_.DataFolder(),
        OTFolders::Market().Get(),   // "markets"
        reply.m_strNotaryID->Get(),  // "markets/<notaryID>"
        marketDatafile->Get(),
        "");  // "markets/<notaryID>/market_data.bin"
    if (!success) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error storing market list to market folder: ")(marketDatafile)(
            ".")
            .Flush();
    }

    return true;
}

bool ServerContext::process_get_market_offers_response(
    const Lock& lock,
    const Message& reply)
{
    const String& marketID = reply.m_strNymID2;  // market ID stored here.

    auto offerDatafile = String::Factory();
    offerDatafile->Format("%s.bin", marketID.Get());

    auto pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase the
    // data file. (So when the file is loaded from storage, it will correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    if (reply.m_lDepth == 0) {
        auto success = storage.EraseValueByKey(
            api_.DataFolder(),
            OTFolders::Market().Get(),   // "markets"
            reply.m_strNotaryID->Get(),  // "markets/<notaryID>",
            "offers",                    // "markets/<notaryID>/offers"
                                         // todo stop hardcoding.
            offerDatafile
                ->Get());  // "markets/<notaryID>/offers/<marketID>.bin"
        if (!success) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error erasing offers list from market folder: ")(
                offerDatafile)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = Data::Factory();

    if ((reply.m_ascPayload->GetLength() <= 2) ||
        (false == reply.m_ascPayload->GetData(serialized))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to decode asci-armored "
                                           "payload.")
            .Flush();
        return true;
    }

    // Unpack the market list...

    OTDB::OTPacker* pPacker = storage.GetPacker();

    OT_ASSERT(nullptr != pPacker);

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());
    OT_ASSERT((pBuffer));

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized->data()),
        serialized->size());

    std::unique_ptr<OTDB::OfferListMarket> pOfferList(
        dynamic_cast<OTDB::OfferListMarket*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_MARKET)));

    auto unpacked = pPacker->Unpack(buffer, *pOfferList);

    if (!unpacked) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed unpacking data.").Flush();
        return true;
    }

    bool success = storage.StoreObject(
        *pOfferList,
        api_.DataFolder(),
        OTFolders::Market().Get(),   // "markets"
        reply.m_strNotaryID->Get(),  // "markets/<notaryID>",
        "offers",                    // "markets/<notaryID>/offers"
                                     // todo stop hardcoding.
        offerDatafile->Get());  // "markets/<notaryID>/offers/<marketID>.bin"
    if (!success) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error storing ")(offerDatafile)(
            " to market folder.")
            .Flush();
    }

    return true;
}

bool ServerContext::process_get_market_recent_trades_response(
    const Lock& lock,
    const Message& reply)
{
    const String& marketID = reply.m_strNymID2;  // market ID stored here.

    auto tradeDatafile = String::Factory();
    tradeDatafile->Format("%s.bin", marketID.Get());

    auto pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    //
    if (reply.m_lDepth == 0) {
        bool success = storage.EraseValueByKey(
            api_.DataFolder(),
            OTFolders::Market().Get(),   // "markets"
            reply.m_strNotaryID->Get(),  // "markets/<notaryID>recent",
                                         // //
                                         // "markets/<notaryID>/recent"
                                         // // todo stop
                                         // hardcoding.
            tradeDatafile->Get(),
            "");  // "markets/<notaryID>/recent/<marketID>.bin"
        if (!success) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error erasing recent trades list from market folder: ")(
                tradeDatafile)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = Data::Factory();

    if ((reply.m_ascPayload->GetLength() <= 2) ||
        (false == reply.m_ascPayload->GetData(serialized))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to decode asci-armored "
                                           "payload.")
            .Flush();
        return true;
    }

    // Unpack the market list...

    auto pPacker = storage.GetPacker();

    OT_ASSERT(nullptr != pPacker);

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());

    OT_ASSERT(nullptr != pBuffer);

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized->data()),
        serialized->size());

    std::unique_ptr<OTDB::TradeListMarket> pTradeList(
        dynamic_cast<OTDB::TradeListMarket*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_TRADE_LIST_MARKET)));

    auto unpacked = pPacker->Unpack(buffer, *pTradeList);

    if (!unpacked) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed unpacking data.").Flush();
        return true;
    }

    bool success = storage.StoreObject(
        *pTradeList,
        api_.DataFolder(),
        OTFolders::Market().Get(),   // "markets"
        reply.m_strNotaryID->Get(),  // "markets/<notaryID>"
        "recent",                    // "markets/<notaryID>/recent"
                                     // todo stop hardcoding.
        tradeDatafile->Get());  // "markets/<notaryID>/recent/<marketID>.bin"
    if (!success) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error storing ")(tradeDatafile)(
            " to market folder.")
            .Flush();
    }

    return true;
}

#if OT_CASH
bool ServerContext::process_get_mint_response(
    const Lock& lock,
    const Message& reply)
{
    auto serialized = String::Factory(reply.m_ascPayload);

    auto mint = api_.Factory().Mint(
        reply.m_strNotaryID, reply.m_strInstrumentDefinitionID);

    OT_ASSERT(mint);

    // TODO check the server signature on the mint here...
    if (false == mint->LoadContractFromString(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading mint from message payload.")
            .Flush();

        return false;
    }

    return mint->SaveMint();
}
#endif

bool ServerContext::process_get_nym_market_offers_response(
    const Lock& lock,
    const Message& reply)
{
    auto offerDatafile = String::Factory();
    offerDatafile->Format("%s.bin", reply.m_strNymID->Get());

    auto pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly display an empty list on the screen, instead of a list of
    // outdated items.)
    //
    if (reply.m_lDepth == 0) {
        bool success = storage.EraseValueByKey(
            api_.DataFolder(),
            OTFolders::Nym().Get(),      // "nyms"
            reply.m_strNotaryID->Get(),  // "nyms/<notaryID>",
            "offers",                    // "nyms/<notaryID>/offers"
                                         // todo stop hardcoding.
            offerDatafile->Get());       // "nyms/<notaryID>/offers/<NymID>.bin"
        if (!success) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error erasing offers list from nyms folder: ")(
                offerDatafile)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = Data::Factory();

    if ((reply.m_ascPayload->GetLength() <= 2) ||
        (false == reply.m_ascPayload->GetData(serialized))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to decode ascii-armored "
                                           "payload.")
            .Flush();
        return true;
    }

    // Unpack the nym's offer list...

    auto pPacker = storage.GetPacker();

    OT_ASSERT(nullptr != pPacker);

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());
    OT_ASSERT(nullptr != pBuffer);

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized->data()),
        serialized->size());

    std::unique_ptr<OTDB::OfferListNym> pOfferList(
        dynamic_cast<OTDB::OfferListNym*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_NYM)));

    auto unpacked = pPacker->Unpack(buffer, *pOfferList);

    if (!unpacked) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed unpacking data.").Flush();
        return true;
    }

    bool success = storage.StoreObject(
        *pOfferList,
        api_.DataFolder(),
        OTFolders::Nym().Get(),      // "nyms"
        reply.m_strNotaryID->Get(),  // "nyms/<notaryID>",
        "offers",                    // "nyms/<notaryID>/offers",
        offerDatafile->Get());       // "nyms/<notaryID>/offers/<NymID>.bin"
    if (!success) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error storing ")(offerDatafile)(
            " to nyms folder.")
            .Flush();
    }

    return true;
}

void ServerContext::process_incoming_instrument(
    const std::shared_ptr<OTTransaction> receipt) const
{
    OT_ASSERT(receipt);

    auto paymentInbox = load_or_create_payment_inbox();

    if (false == bool(paymentInbox)) { return; }

    add_transaction_to_ledger(
        receipt->GetTransactionNum(), receipt, *paymentInbox);
}

void ServerContext::process_incoming_message(
    const Lock& lock,
    const api::client::Manager& client,
    const OTTransaction& receipt) const
{
    OT_ASSERT(nym_);

    const auto& nymID = nym_->ID();
    auto serialized = String::Factory();
    receipt.GetReferenceString(serialized);
    auto message = api_.Factory().Message();

    OT_ASSERT(message);

    if (false == message->LoadContractFromString(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to decode peer object: failed to deserialize message.")
            .Flush();

        return;
    }

    const auto recipientNymId = identifier::Nym::Factory(message->m_strNymID2);
    const auto senderNymID = identifier::Nym::Factory(message->m_strNymID);

    if (senderNymID->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing sender nym ID").Flush();
    } else {
        client.Contacts().NymToContact(senderNymID);
    }

    if (recipientNymId == nymID) {
        const auto pPeerObject =
            api_.Factory().PeerObject(nym_, message->m_ascPayload);

        if (false == bool(pPeerObject)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate object")
                .Flush();

            return;
        }

        auto& peerObject = *pPeerObject;

        switch (peerObject.Type()) {
            case (proto::PEEROBJECT_MESSAGE): {
                client.Activity().Mail(
                    recipientNymId, *message, StorageBox::MAILINBOX);
            } break;
#if OT_CASH
            case (proto::PEEROBJECT_CASH): {
                process_incoming_cash(
                    lock,
                    client,
                    receipt.GetTransactionNum(),
                    peerObject,
                    *message);
            } break;
#endif
            case (proto::PEEROBJECT_PAYMENT): {
                const bool created = create_instrument_notice_from_peer_object(
                    lock,
                    client,
                    *message,
                    peerObject,
                    receipt.GetTransactionNum());

                if (!created) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failed to create object")
                        .Flush();
                }
            } break;
            case (proto::PEEROBJECT_REQUEST): {
                api_.Wallet().PeerRequestReceive(recipientNymId, peerObject);
            } break;
            case (proto::PEEROBJECT_RESPONSE): {
                api_.Wallet().PeerReplyReceive(recipientNymId, peerObject);
            } break;
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unable to decode peer object: unknown peer object type.")
                    .Flush();
            }
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing recipient nym.").Flush();
    }
}

bool ServerContext::process_get_unit_definition_response(
    const Lock& lock,
    const Message& reply)
{
    update_nymbox_hash(lock, reply);
    const auto unitID =
        api_.Factory().UnitID(reply.m_strInstrumentDefinitionID);

    if (reply.m_ascPayload->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Server reply does not contain unit definition ")(unitID)
            .Flush();

        return false;
    }

    const auto raw = Data::Factory(reply.m_ascPayload);

    switch (static_cast<ContractType>(reply.enum_)) {
        case ContractType::NYM: {
            auto serialized =
                proto::DataToProto<proto::CredentialIndex>(raw.get());
            auto contract = api_.Wallet().Nym(serialized);

            if (contract) { return (unitID->str() == serialized.nymid()); }
        } break;
        case ContractType::SERVER: {
            auto serialized =
                proto::DataToProto<proto::ServerContract>(raw.get());
            auto contract = api_.Wallet().Server(serialized);

            if (contract) { return (unitID->str() == serialized.id()); }
        } break;
        case ContractType::UNIT: {
            auto serialized =
                proto::DataToProto<proto::UnitDefinition>(raw.get());
            auto contract = api_.Wallet().UnitDefinition(serialized);

            if (contract) { return (unitID->str() == serialized.id()); }
        } break;
        case ContractType::ERROR:
        default: {
            auto serialized =
                proto::DataToProto<proto::UnitDefinition>(raw.get());
            auto contract = api_.Wallet().UnitDefinition(serialized);

            if (contract) {

                return (unitID->str() == serialized.id());
            } else {
                // Maybe it's actually a server contract?
                auto serialized =
                    proto::DataToProto<proto::ServerContract>(raw.get());

                auto contract = api_.Wallet().Server(serialized);

                if (contract) { return (unitID->str() == serialized.id()); }
            }
        }
    }

    return false;
}

bool ServerContext::process_issue_unit_definition_response(
    const Lock& lock,
    const Message& reply)
{
    update_nymbox_hash(lock, reply);
    const auto accountID = api_.Factory().Identifier(reply.m_strAcctID);

    if (reply.m_ascPayload->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Server reply does not contain issuer account ")(accountID)
            .Flush();

        return false;
    }

    auto serialized = String::Factory(reply.m_ascPayload);
    const auto updated = api_.Wallet().UpdateAccount(
        accountID, *this, serialized, std::get<0>(pending_args_));

    if (updated) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Saved new issuer account.")
            .Flush();

        return init_new_account(accountID);
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save account.").Flush();

    return false;
}

bool ServerContext::process_notarize_transaction_response(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply)
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    update_nymbox_hash(lock, reply);
    const auto accountID = api_.Factory().Identifier(reply.m_strAcctID);
    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    const auto& serverNym = *remote_nym_;
    auto responseLedger = api_.Factory().Ledger(nymID, accountID, server_id_);

    OT_ASSERT(responseLedger);

    bool loaded = responseLedger->LoadLedgerFromString(
        String::Factory(reply.m_ascPayload));

    if (loaded) { loaded &= responseLedger->VerifyAccount(serverNym); }

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading ledger from message payload.")
            .Flush();

        return false;
    }

    for (auto& [number, pTransaction] : responseLedger->GetTransactionMap()) {
        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction ")(
                number)(" in response ledger")
                .Flush();

            return false;
        }

        auto& transaction = *pTransaction;
        const auto& transactionNumber = transaction.GetTransactionNum();

        if (false == verify_issued_number(lock, transactionNumber)) {
            LogVerbose(OT_METHOD)(__FUNCTION__)("Skipping ")(
                "processing of server reply to transaction number ")(
                transactionNumber)(
                " since the number isn't even issued to me. Usually this ")(
                "means that I ALREADY processed it, and we are now ")(
                "processing the redundant nymbox notice for the same ")(
                "transaction. (Which was only sent to make sure we saw ")(
                "it.) ")
                .Flush();

            continue;
        }

        if (false == transaction.VerifyAccount(serverNym)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Unable to verify transaction ")(transactionNumber)
                .Flush();

            return false;
        }

        process_response_transaction(lock, client, reply, transaction);
    }

    return true;
}

bool ServerContext::process_process_box_response(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply,
    const BoxType type,
    const Identifier& accountID)
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    update_nymbox_hash(lock, reply);
    auto originalMessage = extract_message(reply.m_ascInReferenceTo, *nym_);

    if (false == bool(originalMessage)) { return false; }

    OT_ASSERT(originalMessage);

    auto ledger =
        extract_ledger(originalMessage->m_ascPayload, accountID, *nym_);

    if (false == bool(ledger)) { return false; }

    auto responseLedger =
        extract_ledger(reply.m_ascPayload, accountID, *remote_nym_);

    if (false == bool(responseLedger)) { return false; }

    std::shared_ptr<OTTransaction> transaction;
    std::shared_ptr<OTTransaction> replyTransaction;

    if (BoxType::Inbox == type) {
        process_process_inbox_response(
            lock,
            client,
            reply,
            *ledger,
            *responseLedger,
            transaction,
            replyTransaction);
    } else {
        process_process_nymbox_response(
            lock,
            reply,
            *ledger,
            *responseLedger,
            transaction,
            replyTransaction);
    }

    if (false == (transaction && replyTransaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid transaction").Flush();

        return false;
    }

    const auto notaryID = String::Factory(server_id_);
    auto receiptID = String::Factory("NOT_SET_YET");
    auto replyItem = replyTransaction->GetItem(itemType::atBalanceStatement);

    if (replyItem) {
        receiptID = reply.m_strAcctID;
    } else {
        replyItem = replyTransaction->GetItem(itemType::atTransactionStatement);

        if (replyItem) { nym_->GetIdentifier(receiptID); }
    }

    auto serialized = String::Factory();
    replyTransaction->SaveContractRaw(serialized);
    auto filename = String::Factory();

    if (replyTransaction->GetSuccess()) {
        filename->Format("%s.success", receiptID->Get());
    } else {
        filename->Format("%s.fail", receiptID->Get());
    }

    auto encoded = String::Factory();
    auto armored = Armored::Factory(serialized);

    if (false == armored->WriteArmoredString(encoded, "TRANSACTION")) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encode receipt")
            .Flush();

        return false;
    }

    if (replyItem) {
        OTDB::StorePlainString(
            encoded->Get(),
            api_.DataFolder(),
            OTFolders::Receipt().Get(),
            notaryID->Get(),
            filename->Get(),
            "");
    } else {
        // This should never happen...
        filename->Format("%s.error", receiptID->Get());
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error saving transaction receipt: ")(notaryID)(
            Log::PathSeparator())(filename)(".")
            .Flush();
        OTDB::StorePlainString(
            encoded->Get(),
            api_.DataFolder(),
            OTFolders::Receipt().Get(),
            notaryID->Get(),
            filename->Get(),
            "");
    }

    return true;
}

bool ServerContext::process_process_inbox_response(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply,
    Ledger& ledger,
    Ledger& responseLedger,
    std::shared_ptr<OTTransaction>& transaction,
    std::shared_ptr<OTTransaction>& replyTransaction)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto accountID = api_.Factory().Identifier(reply.m_strAcctID);
    transaction = ledger.GetTransaction(transactionType::processInbox);
    replyTransaction =
        responseLedger.GetTransaction(transactionType::atProcessInbox);

    if (false == (transaction && replyTransaction)) { return false; }

    const auto number = transaction->GetTransactionNum();
    const bool isIssued = verify_issued_number(lock, number);

    if (false == isIssued) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": This reply has already been processed.")
            .Flush();

        return true;
    }

    consume_issued(lock, transaction->GetTransactionNum());
    auto inbox = load_account_inbox(accountID);

    OT_ASSERT(inbox);

    auto recordBox = load_or_create_account_recordbox(accountID);

    OT_ASSERT(recordBox);

    for (auto& pReplyItem : replyTransaction->GetItemList()) {
        if (false == bool(pReplyItem)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid reply item").Flush();

            return false;
        }

        auto& replyItem = *pReplyItem;
        auto replyItemType = String::Factory();
        replyItem.GetTypeString(replyItemType);
        itemType requestType{itemType::error_state};

        switch (replyItem.GetType()) {
            case itemType::atAcceptPending: {
                requestType = itemType::acceptPending;
            } break;
            case itemType::atAcceptCronReceipt: {
                requestType = itemType::acceptCronReceipt;
            } break;
            case itemType::atAcceptItemReceipt: {
                requestType = itemType::acceptItemReceipt;
            } break;
            case itemType::atAcceptFinalReceipt: {
                requestType = itemType::acceptFinalReceipt;
            } break;
            case itemType::atAcceptBasketReceipt: {
                requestType = itemType::acceptBasketReceipt;
            } break;
            case itemType::atRejectPending:
            case itemType::atDisputeCronReceipt:
            case itemType::atDisputeItemReceipt:
            case itemType::atDisputeFinalReceipt:
            case itemType::atDisputeBasketReceipt:
            case itemType::atBalanceStatement:
            case itemType::atTransactionStatement: {

                continue;
            }
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unexpected reply item type (")(replyItemType)(
                    ") in processInboxResponse")
                    .Flush();

                continue;
            }
        }

        if (Item::acknowledgement != replyItem.GetStatus()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": processInboxResponse reply item ")(replyItemType)(
                ": status == FAILED.")
                .Flush();

            continue;
        }

        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": processInboxResponse reply item ")(replyItemType)(
            ": status == SUCCESS")
            .Flush();

        auto serializedOriginalItem = String::Factory();
        replyItem.GetReferenceString(serializedOriginalItem);
        auto pReferenceItem = api_.Factory().Item(
            serializedOriginalItem, server_id_, replyItem.GetReferenceToNum());
        auto pItem = (pReferenceItem) ? transaction->GetItemInRefTo(
                                            pReferenceItem->GetReferenceToNum())
                                      : nullptr;

        if (false == bool(pItem)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unable to find original item in original processInbox "
                "transaction request, based on reply item.")
                .Flush();

            continue;
        }

        const auto& referenceItem = *pReferenceItem;
        const auto& item = *pItem;

        if (item.GetType() != requestType) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Wrong original item TYPE, on reply item's copy of "
                "original item, than what was expected based on reply "
                "item's type.")
                .Flush();

            continue;
        }

        // TODO here: any other verification of item against referenceItem,
        // which are supposedly copies of the same item.

        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Checking client-side inbox for expected "
            "pending or receipt transaction: ")(item.GetReferenceToNum())("...")
            .Flush();
        auto pInboxTransaction =
            inbox->GetTransaction(item.GetReferenceToNum());

        if (false == bool(pInboxTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unable to find the server's receipt, in my inbox, "
                "that my original processInbox's item was referring to.")
                .Flush();

            continue;
        }

        auto& inboxTransaction = *pInboxTransaction;
        const auto inboxNumber = inboxTransaction.GetTransactionNum();

        switch (replyItem.GetType()) {
            case itemType::atAcceptPending: {
                process_accept_pending_reply(
                    lock, client, accountID, referenceItem, reply);
            } break;
            case itemType::atAcceptItemReceipt: {
                process_accept_item_receipt_reply(
                    lock, client, accountID, reply, inboxTransaction);
            } break;
            case itemType::atAcceptCronReceipt: {
                process_accept_cron_receipt_reply(
                    lock, accountID, inboxTransaction);
            } break;
            case itemType::atAcceptFinalReceipt: {
                process_accept_final_receipt_reply(lock, inboxTransaction);
            } break;
            case itemType::atAcceptBasketReceipt: {
                process_accept_basket_receipt_reply(lock, inboxTransaction);
            } break;
            default: {
            }
        }

        if (recordBox->AddTransaction(pInboxTransaction)) {
            recordBox->ReleaseSignatures();
            recordBox->SignContract(nym);
            recordBox->SaveContract();
            recordBox->SaveRecordBox();

            if (false == inboxTransaction.SaveBoxReceipt(*recordBox)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save receipt ")(
                    inboxNumber)(" to record box")
                    .Flush();
            }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to add transaction ")(
                inboxNumber)(
                " to record box (still removing it from asset account "
                "inbox, however).")
                .Flush();
        }

        inboxTransaction.DeleteBoxReceipt(*inbox);
        inbox->RemoveTransaction(inboxNumber);
    }

    inbox->ReleaseSignatures();
    inbox->SignContract(nym);
    inbox->SaveContract();
    inbox->SaveInbox(api_.Factory().Identifier());

    return true;
}

bool ServerContext::process_process_nymbox_response(
    const Lock& lock,
    const Message& reply,
    Ledger& ledger,
    Ledger& responseLedger,
    std::shared_ptr<OTTransaction>& transaction,
    std::shared_ptr<OTTransaction>& replyTransaction)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    transaction = ledger.GetTransaction(transactionType::processNymbox);
    replyTransaction =
        responseLedger.GetTransaction(transactionType::atProcessNymbox);

    if (false == (transaction && replyTransaction)) { return false; }

    accept_numbers(lock, *transaction, *replyTransaction);
    auto nymbox = api_.Factory().Ledger(nymID, nymID, server_id_);

    OT_ASSERT((nymbox));

    bool loadedNymbox{true};
    loadedNymbox &= nymbox->LoadNymbox();
    loadedNymbox &= nymbox->VerifyAccount(nym);

    OT_ASSERT(loadedNymbox);

    for (const auto& item : replyTransaction->GetItemList()) {
        OT_ASSERT(item);

        remove_nymbox_item(lock, *item, *nymbox, *transaction);
    }

    nymbox->ReleaseSignatures();
    nymbox->SignContract(nym);
    nymbox->SaveContract();
    auto nymboxHash = api_.Factory().Identifier();
    nymbox->SaveNymbox(nymboxHash);
    set_local_nymbox_hash(lock, nymboxHash);

    return true;
}

bool ServerContext::process_register_account_response(
    const Lock& lock,
    const Message& reply)
{
    update_nymbox_hash(lock, reply);
    const auto accountID = api_.Factory().Identifier(reply.m_strAcctID);

    if (reply.m_ascPayload->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Server reply does not contain an account")
            .Flush();

        return false;
    }

    auto serialized = String::Factory(reply.m_ascPayload);
    const auto updated = api_.Wallet().UpdateAccount(
        accountID, *this, serialized, std::get<0>(pending_args_));

    if (updated) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Saved new issuer account.")
            .Flush();

        return init_new_account(accountID);
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save account.").Flush();

    return false;
}

bool ServerContext::process_request_admin_response(
    const Lock& lock,
    const Message& reply)
{
    update_nymbox_hash(lock, reply);
    admin_attempted_->On();
    const auto& serverID = reply.m_strNotaryID;

    if (reply.m_bBool) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Became admin on server ")(
            serverID)
            .Flush();
        admin_success_->On();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Server ")(serverID)(
            " rejected admin request")
            .Flush();
    }

    return true;
}

bool ServerContext::process_register_nym_response(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply)
{
    auto serialized =
        proto::DataToProto<proto::Context>(Data::Factory(reply.m_ascPayload));
    auto verified = proto::Validate(serialized, VERBOSE);

    if (false == verified) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid context.").Flush();

        return false;
    }

    auto output = resync(lock, serialized);
    output &= harvest_unused(lock, client);

    return output;
}

bool ServerContext::process_reply(
    const Lock& lock,
    const api::client::Manager& client,
    const std::set<OTManagedNumber>& managed,
    const Message& reply)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    const auto accountID = api_.Factory().Identifier(reply.m_strAcctID);
    const auto& serverNym = *remote_nym_;

    LogVerbose(OT_METHOD)(__FUNCTION__)("Received ")(reply.m_strCommand)("(")(
        reply.m_bSuccess ? "success" : "failure")(")")
        .Flush();

    if (false == reply.VerifySignature(serverNym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Server reply signature failed to verify.")
            .Flush();

        return false;
    }

    const RequestNumber requestNumber = reply.m_strRequestNum->ToLong();
    add_acknowledged_number(lock, requestNumber);
    RequestNumbers acknowledgedReplies{};

    if (reply.m_AcknowledgedReplies.Output(acknowledgedReplies)) {
        remove_acknowledged_number(lock, acknowledgedReplies);
    }

    if (reply.m_bSuccess) {
        for (const auto& number : managed) { number->SetSuccess(true); }
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Message status: failed for ")(
            reply.m_strCommand)
            .Flush();

        return false;
    }

    switch (Message::Type(reply.m_strCommand->Get())) {
        case MessageType::checkNymResponse: {
            return process_check_nym_response(lock, client, reply);
        }
        case MessageType::getAccountDataResponse: {
            return process_get_account_data(lock, reply);
        }
        case MessageType::getBoxReceiptResponse: {
            return process_get_box_receipt_response(lock, client, reply);
        }
        case MessageType::getInstrumentDefinitionResponse: {
            return process_get_unit_definition_response(lock, reply);
        }
        case MessageType::getMarketListResponse: {
            return process_get_market_list_response(lock, reply);
        }
        case MessageType::getMarketOffersResponse: {
            return process_get_market_offers_response(lock, reply);
        }
        case MessageType::getMarketRecentTradesResponse: {
            return process_get_market_recent_trades_response(lock, reply);
        }
#if OT_CASH
        case MessageType::getMintResponse: {
            return process_get_mint_response(lock, reply);
        }
#endif
        case MessageType::getNymboxResponse: {
            return process_get_nymbox_response(lock, reply);
        }
        case MessageType::getNymMarketOffersResponse: {
            return process_get_nym_market_offers_response(lock, reply);
        }
        case MessageType::notarizeTransactionResponse: {
            return process_notarize_transaction_response(lock, client, reply);
        }
        case MessageType::processInboxResponse: {
            return process_process_box_response(
                lock,
                client,
                reply,
                BoxType::Inbox,
                api_.Factory().Identifier(reply.m_strAcctID));
        }
        case MessageType::processNymboxResponse: {
            return process_process_box_response(
                lock, client, reply, BoxType::Nymbox, nymID);
        }
        case MessageType::registerAccountResponse: {
            return process_register_account_response(lock, reply);
        }
        case MessageType::requestAdminResponse: {
            return process_request_admin_response(lock, reply);
        }
        case MessageType::registerInstrumentDefinitionResponse: {
            return process_issue_unit_definition_response(lock, reply);
        }
        case MessageType::registerNymResponse: {
            update_nymbox_hash(lock, reply);
            revision_.store(nym.Revision());
            const auto& resync = std::get<1>(pending_args_);

            if (resync) {
                return process_register_nym_response(lock, client, reply);
            }

            return true;
        }
        case MessageType::triggerClauseResponse: {
            update_nymbox_hash(lock, reply);
            return true;
        }
        case MessageType::unregisterAccountResponse: {
            return process_unregister_account_response(lock, reply);
        }
        case MessageType::unregisterNymResponse: {
            return process_unregister_nym_response(lock, reply);
        }
        default: {
            update_nymbox_hash(lock, reply);

            return true;
        }
    }
}

void ServerContext::process_response_transaction(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply,
    OTTransaction& response)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    itemType type{itemType::error_state};

    if (Exit::Yes == get_item_type(response, type)) { return; }

    switch (response.GetType()) {
        case transactionType::atDeposit: {
            process_response_transaction_deposit(
                lock, client, reply, type, response);
        } break;
        case transactionType::atPayDividend: {
            process_response_transaction_pay_dividend(
                lock, reply, type, response);
        } break;
        case transactionType::atExchangeBasket: {
            process_response_transaction_exchange_basket(
                lock, reply, type, response);
        } break;
        case transactionType::atCancelCronItem: {
            process_response_transaction_cancel(lock, reply, type, response);
        } break;
        case transactionType::atWithdrawal: {
#if OT_CASH
            process_response_transaction_withdrawal(
                lock, client, reply, type, response);
#endif  // OT_CASH
        } break;
        case transactionType::atTransfer: {
            process_response_transaction_transfer(
                lock, client, reply, type, response);
        } break;
        case transactionType::atMarketOffer:
        case transactionType::atPaymentPlan:
        case transactionType::atSmartContract: {
            process_response_transaction_cron(lock, reply, type, response);
        } break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": wrong transaction type: ")(
                response.GetTypeString())(".")
                .Flush();
            break;
    }

    auto receiptID = String::Factory("ID_NOT_SET_YET");
    auto pItem = response.GetItem(itemType::atBalanceStatement);

    if (pItem) {
        receiptID->Set(reply.m_strAcctID);
    } else {
        pItem = response.GetItem(itemType::atTransactionStatement);

        if (pItem) {
            nym.GetIdentifier(receiptID);
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Response transaction contains neither balance statement nor "
                "transaction statement")
                .Flush();
        }
    }

    auto serialized = String::Factory();
    response.SaveContractRaw(serialized);
    auto armored = Armored::Factory(serialized);
    auto encoded = String::Factory();

    if (false == armored->WriteArmoredString(encoded, "TRANSACTION")) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error saving transaction receipt "
                                           "(failed writing armored string): ")(
            OTFolders::Receipt())(Log::PathSeparator())(server_id_->str())(
            Log::PathSeparator())(receiptID)
            .Flush();

        return;
    }

    auto filename = String::Factory();

    if (pItem) {
        if (response.GetSuccess()) {
            filename->Format("%s.success", receiptID->Get());
        } else {
            filename->Format("%s.fail", receiptID->Get());
        }

        OTDB::StorePlainString(
            encoded->Get(),
            api_.DataFolder(),
            OTFolders::Receipt().Get(),
            server_id_->str(),
            filename->Get(),
            "");
    } else {
        filename->Format("%s.error", receiptID->Get());
        OTDB::StorePlainString(
            encoded->Get(),
            api_.DataFolder(),
            OTFolders::Receipt().Get(),
            server_id_->str(),
            filename->Get(),
            "");
    }
}

void ServerContext::process_response_transaction_cancel(
    const Lock& lock,
    const Message& reply,
    const itemType type,
    OTTransaction& response)
{
    consume_issued(lock, response.GetTransactionNum());
    auto item = response.GetItem(type);

    if (item && Item::acknowledgement == item->GetStatus()) {
        auto originalItem = extract_original_item(*item);

        if (originalItem) {
            const auto originalNumber = originalItem->GetReferenceToNum();

            if (false == consume_issued(lock, originalNumber)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": (atCancelCronItem) Error removing issued number from "
                    "user nym.")
                    .Flush();
            }
        }
    }
}

#if OT_CASH
void ServerContext::process_response_transaction_cash_deposit(Item& replyItem)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    auto serializedRequest = String::Factory();
    replyItem.GetReferenceString(serializedRequest);

    if (serializedRequest->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to get original serialized request")
            .Flush();

        return;
    }

    auto pItem = api_.Factory().Item(serializedRequest);

    if (false == bool(pItem)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate request")
            .Flush();

        return;
    }

    const auto& item = *pItem;
    auto rawPurse = Data::Factory();
    item.GetAttachment(rawPurse);
    const auto serializedPurse = proto::DataToProto<proto::Purse>(rawPurse);

    if (false == proto::Validate(serializedPurse, VERBOSE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid purse").Flush();

        return;
    }

    auto pPurse{api_.Factory().Purse(serializedPurse)};

    if (false == bool(pPurse)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate request purse")
            .Flush();

        return;
    }

    auto& purse = *pPurse;

    if (false == purse.Unlock(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to unlock request purse")
            .Flush();

        return;
    }

    std::set<std::string> spentTokens{};
    std::vector<std::shared_ptr<blind::Token>> keepTokens{};

    for (const auto& token : purse) { spentTokens.insert(token.ID()); }

    auto purseEditor = mutable_Purse(purse.Unit());
    auto& walletPurse = purseEditor.It();

    if (false == walletPurse.Unlock(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to unlock wallet purse")
            .Flush();

        return;
    }

    auto token = walletPurse.Pop();

    while (token) {
        const auto id = token->ID();

        if (1 == spentTokens.count(id)) {
            LogTrace(OT_METHOD)(__FUNCTION__)(": Removing spent token ")(id)(
                " from purse")
                .Flush();
        } else {
            LogTrace(OT_METHOD)(__FUNCTION__)(":Retaining unspent token ")(id)(
                " in purse")
                .Flush();
            keepTokens.emplace_back(std::move(token));
        }

        token = walletPurse.Pop();
    }

    for (auto& token : keepTokens) { walletPurse.Push(token); }
}
#endif

void ServerContext::process_response_transaction_cheque_deposit(
    const api::client::Manager& client,
    const Identifier& accountID,
    const Message* reply,
    const Item& replyItem)
{
    auto empty = client.Factory().Message();

    OT_ASSERT(nym_);
    OT_ASSERT(empty);

    const auto& request = (pending_message_) ? *pending_message_ : *empty;
    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto paymentInbox = load_or_create_payment_inbox();

    if (false == bool(paymentInbox)) { return; }

    auto serializedOriginal = String::Factory();
    Item* pOriginal{nullptr};
    replyItem.GetReferenceString(serializedOriginal);
    auto instantiatedOriginal = api_.Factory().Transaction(serializedOriginal);

    if (false == bool(instantiatedOriginal)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to deserialized original item")
            .Flush();

        return;
    }

    pOriginal = dynamic_cast<Item*>(instantiatedOriginal.get());

    if (nullptr == pOriginal) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Incorrect transaction type on original item")
            .Flush();

        return;
    }

    auto& originalItem = *pOriginal;
    auto pCheque = api_.Factory().Cheque();

    if (false == bool(pCheque)) { return; }

    auto& cheque = *pCheque;
    auto serializedCheque = String::Factory();
    originalItem.GetAttachment(serializedCheque);

    if (false == cheque.LoadContractFromString(serializedCheque)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": ERROR loading cheque from string: ")(serializedCheque)(".")
            .Flush();

        return;
    }

    if (0 < cheque.GetAmount()) {
        // Cheque or voucher
        const auto workflowUpdated = client.Workflow().DepositCheque(
            nymID, accountID, cheque, request, reply);

        if (workflowUpdated) {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Successfully updated workflow.")
                .Flush();
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to update workflow.")
                .Flush();
        }

        return;
    }

    // Invoice
    const auto chequeNumber = cheque.GetTransactionNum();
    auto receipt_ids = paymentInbox->GetTransactionNums();

    for (auto& receipt_id : receipt_ids) {
        TransactionNumber number{0};
        auto pPayment =
            GetInstrumentByReceiptID(nym, receipt_id, *paymentInbox);

        if (false == bool(pPayment) || !pPayment->SetTempValues() ||
            !pPayment->GetTransactionNum(number) || (number != chequeNumber)) {
            continue;
        }

        auto pTransaction = paymentInbox->GetTransaction(receipt_id);
        auto strPmntInboxTransaction = String::Factory();
        TransactionNumber lRemoveTransaction{0};

        if (pTransaction) {
            pTransaction->SaveContractRaw(strPmntInboxTransaction);
            lRemoveTransaction = pTransaction->GetTransactionNum();

            if (false == paymentInbox->DeleteBoxReceipt(lRemoveTransaction)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed trying to delete the box receipt for a "
                    "cheque being removed from a payments inbox: ")(
                    lRemoveTransaction)(".")
                    .Flush();
            }

            if (paymentInbox->RemoveTransaction(lRemoveTransaction)) {
                paymentInbox->ReleaseSignatures();
                paymentInbox->SignContract(nym);
                paymentInbox->SaveContract();

                if (!paymentInbox->SavePaymentInbox()) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failure while trying to save payment "
                        "inbox.")
                        .Flush();
                } else {
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": Removed cheque from payments inbox. "
                        "(Deposited "
                        "successfully). Saved payments inbox.")
                        .Flush();
                }
            }
        }

        if (!strPmntInboxTransaction->Exists()) { continue; }

        // TODO how many record boxes should exist? One per nym, or one per
        // account?
        auto recordBox = load_or_create_account_recordbox(nymID);

        OT_ASSERT(recordBox);

        add_transaction_to_ledger(lRemoveTransaction, pTransaction, *recordBox);
    }
}

void ServerContext::process_response_transaction_cron(
    const Lock& lock,
    const Message& reply,
    const itemType type,
    OTTransaction& response)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto pReplyItem = response.GetItem(type);

    if (false == bool(pReplyItem)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Reply item is missing").Flush();

        return;
    }

    auto& replyItem = *pReplyItem;
    auto pOriginalItem = extract_original_item(replyItem);

    if (false == bool(pOriginalItem)) { return; }

    auto& originalItem = *pOriginalItem;
    auto serialized = String::Factory();
    originalItem.GetAttachment(serialized);

    if (serialized->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Original item does not contain serialized cron item")
            .Flush();

        return;
    }

    auto pCronItem = api_.Factory().CronItem(serialized);

    if (false == bool(pCronItem)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate serialized cron item")
            .Flush();

        return;
    }

    auto& cronItem = *pCronItem;
    const auto openingNumber = response.GetTransactionNum();

    if (Item::rejection == replyItem.GetStatus()) {
        consume_issued(lock, openingNumber);

        if (nym.CompareID(cronItem.GetSenderNymID())) {
            const auto count = cronItem.GetCountClosingNumbers();

            for (int i = 0; i < count; ++i) {
                const auto number = cronItem.GetClosingTransactionNoAt(i);
                recover_available_number(lock, number);
            }
        }
    }

    switch (response.GetType()) {
        case transactionType::atPaymentPlan:
        case transactionType::atSmartContract: {
            break;
        }
        default: {
            // No further processing necessary

            return;
        }
    }

    auto strInstrument = String::Factory();

    if (Item::acknowledgement == replyItem.GetStatus()) {
        cronItem.SaveActiveCronReceipt(nymID);
    }

    NumList numlistOutpayment(openingNumber);
    auto nymfile = mutable_Nymfile("");
    auto pMsg = nymfile.It().GetOutpaymentsByTransNum(openingNumber);

    if ((pMsg)) {
        const bool bRemovedOutpayment =
            nymfile.It().RemoveOutpaymentsByTransNum(openingNumber);
        if (!bRemovedOutpayment) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to remove outpayment with "
                "transaction num: ")(openingNumber)(".")
                .Flush();
        }
        if (!pMsg->m_ascPayload->GetString(strInstrument)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unable to find payment instrument in outpayment "
                "message with transaction num: ")(openingNumber)(".")
                .Flush();
        } else {
            // At this point, we've removed the outpayment already, and
            // it will be deleted when it goes out of scope already. And
            // we've got a copy of the original financial instrument
            // that was SENT in that outpayment.
            //
            // But what for? Why did I want that instrument here in a
            // string, in strInstrument? Do I still need to do something
            // with it? Yes: I need to drop a copy of it into the record
            // box!
            //
            // NOTE: strInstrument is added to the RecordBox below. So
            // there's no need to do that here, ATM.
        }
    }
    // When party receives notice that smart contract has failed activation
    // attempt, then remove the instrument from payments inbox AND
    // outpayments box. (If there -- could be for either.) (Outbox is done
    // just above, so now let's do inbox...)
    //
    // Why only rejected items? Why not remove it from the payments inbox on
    // success as well? Normally wouldn't we expect that a successful
    // activation of an inbox item, should remove that inbox item?
    // Especially if there's already a copy in the outbox as well...
    //
    //  if (Item::rejection == replyItem.GetStatus())
    {
        const bool bExists1 = OTDB::Exists(
            api_.DataFolder(),
            OTFolders::PaymentInbox().Get(),
            server_id_->str(),
            nymID.str(),
            "");
        const bool bExists2 = OTDB::Exists(
            api_.DataFolder(),
            OTFolders::RecordBox().Get(),
            server_id_->str(),
            nymID.str(),
            "");

        auto thePmntInbox = api_.Factory().Ledger(
            nymID,
            nymID,
            server_id_);  // payment inbox

        OT_ASSERT((thePmntInbox));

        auto theRecordBox = api_.Factory().Ledger(
            nymID,
            nymID,
            server_id_);  // record box

        OT_ASSERT((theRecordBox));

        bool bSuccessLoading1 = (bExists1 && thePmntInbox->LoadPaymentInbox());
        bool bSuccessLoading2 = (bExists2 && theRecordBox->LoadRecordBox());

        if (bExists1 && bSuccessLoading1)
            bSuccessLoading1 =
                (thePmntInbox->VerifyContractID() &&
                 thePmntInbox->VerifySignature(nym));
        // (thePmntInbox->VerifyAccount(*pNym));
        // (No need to load all the Box
        // Receipts using VerifyAccount)
        else if (!bExists1)
            bSuccessLoading1 = thePmntInbox->GenerateLedger(
                nymID,
                server_id_,
                ledgerType::paymentInbox,
                true);  // bGenerateFile=true
        if (bExists2 && bSuccessLoading2)
            bSuccessLoading2 =
                (theRecordBox->VerifyContractID() &&
                 theRecordBox->VerifySignature(nym));
        // (theRecordBox->VerifyAccount(*pNym));
        // (No need to load all the Box
        // Receipts using VerifyAccount)
        else if (!bExists2)
            bSuccessLoading2 = theRecordBox->GenerateLedger(
                nymID,
                server_id_,
                ledgerType::recordBox,
                true);  // bGenerateFile=true
        // By this point, the boxes DEFINITELY exist -- or not. (generation
        // might have failed, or verification.)
        //
        if (!bSuccessLoading1 || !bSuccessLoading2) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": While processing server reply containing rejection "
                "of cron item: WARNING: Unable to load, verify, or "
                "generate paymentInbox or recordBox, with IDs: ")(nymID)(" / ")(
                nymID)(".")
                .Flush();
        } else {
            // --- ELSE --- Success loading the payment inbox and recordBox
            // and verifying their contractID and signature, (OR success
            // generating the ledger.)
            //
            // See if there's a receipt in the payments inbox. If so, remove
            // it.
            //
            // What's going on here?
            //
            // Well let's say Alice sends Bob a payment plan. (This applies
            // to smart contracts, too.) This means Bob has a payment plan
            // in his PAYMENTS INBOX, with the recipient's (Alice)
            // transaction number set to X, and the sender's transaction
            // number set to 0. It's 0 because the instrument is still in
            // Bob's inbox -- he hasn't signed it yet -- so his transaction
            // number isn't on it yet. It's blank (0).
            //
            // Next, let's say Bob signs/confirms the contract, which puts a
            // copy of it into his PAYMENTS OUTBOX. On the outbox version,
            // Alice's transaction number is X, and Bob's transaction number
            // is Y.
            //
            // Later on, Bob needs to lookup the payment plan in his
            // PAYMENTS INBOX (for example, to remove it, AS YOU SEE IN THE
            // BELOW LOOP.) Remember, Bob's transaction number is Y. But he
            // can't use that number (Y) to lookup the payment plan in his
            // inbox, since it's set to ZERO in his inbox! The inbox version
            // simply doesn't HAVE Y set onto it yet -- only the outbox
            // version does.
            //
            // So how in the fuck does Bob lookup the inbox version, if the
            // transaction number isn't SET on it yet??
            //
            // The solution: 1. Bob grabs an OTNumList containing all the
            // transaction numbers from the OUTBOX VERSION, which ends up
            // containing "X,Y" (that happens in this block.) 2. Bob loops
            // through the payments INBOX, and for each, he grabs an
            // OTNumList containing all the transaction numbers. One of
            // those (the matching one) will contain "X,0". (Except it will
            // actually only contain "X", since 0 is ignored in the call to
            // GetAllTransactionNumbers.) 3. Bob then checks like this: if
            // (numlistOutpayment.VerifyAny( numlistIncomingPayment)) This
            // is equivalent to saying: if ("X,Y".VerifyAny("X")) which
            // RETURNS TRUE -- and we have found the instrument!

            auto theOutpayment = api_.Factory().Payment();

            OT_ASSERT((theOutpayment));

            if (strInstrument->Exists() &&
                theOutpayment->SetPayment(strInstrument) &&
                theOutpayment->SetTempValues()) {
                theOutpayment->GetAllTransactionNumbers(numlistOutpayment);
            }

            const std::set<std::int64_t> set_receipt_ids{
                thePmntInbox->GetTransactionNums()};
            for (const auto& receipt_id : set_receipt_ids) {
                auto pPayment =
                    GetInstrumentByReceiptID(nym, receipt_id, *thePmntInbox);

                if (false == bool(pPayment)) {
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": While looping payments inbox to remove a "
                        "payment, unable to retrieve payment on receipt ")(
                        receipt_id)(" (skipping).")
                        .Flush();
                    continue;
                } else if (false == pPayment->SetTempValues()) {
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": While looping payments inbox to remove a "
                        "payment, unable to set temp values for "
                        "payment on "
                        "receipt ")(receipt_id)(" (skipping).")
                        .Flush();

                    continue;
                }

                NumList numlistIncomingPayment;

                pPayment->GetAllTransactionNumbers(numlistIncomingPayment);

                if (numlistOutpayment.VerifyAny(numlistIncomingPayment)) {
                    // ** It's the same instrument.**
                    // Remove it from the payments inbox, and save.
                    //
                    auto pTransPaymentInbox =
                        thePmntInbox->GetTransaction(receipt_id);
                    // It DEFINITELY should be there.
                    // (Assert otherwise.)
                    OT_ASSERT((pTransPaymentInbox));

                    // DON'T I NEED to call DeleteBoxReceipt at this point?
                    // Since that needs to be called now whenever removing
                    // something from any box?
                    //
                    // NOTE: might need to just MOVE this box receipt to the
                    // record box, instead of deleting it.
                    //
                    // Probably I need to do that ONLY if the version in the
                    // payments outbox doesn't exist. For example, if
                    // strInstrument doesn't exist, then there was nothing
                    // in the payments outbox, and therefore the version in
                    // the payment INBOX is the ONLY version I have, and
                    // therefore I should stick it in the Record Box.
                    //
                    // HOWEVER, if strInstrument DOES exist, then I should
                    // create its own transaction to add to the record box,
                    // and delete the one that was in the payment inbox. Why
                    // delete it? Because otherwise I would be adding the
                    // same thing TWICE to the record box, which I don't
                    // really need to do. And if I'm going to choose one of
                    // the two, the one in the outpayments box will be the
                    // more recent / more relevant one of the two. So I
                    // favor that one, unless it doesn't exist, in which
                    // case I should add the other one instead. (Todo.)
                    //
                    // NOTE: Until the above is completed, the current
                    // behavior is that the outpayments box item will be
                    // moved to the record box if it exists, and otherwise
                    // nothing will be, since any payments inbox item will
                    // be deleted.

                    if (false == thePmntInbox->DeleteBoxReceipt(receipt_id)) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Failed trying to delete the box receipt "
                            "for a transaction being removed from the "
                            "payment inbox.")
                            .Flush();
                    }
                    if (thePmntInbox->RemoveTransaction(receipt_id)) {
                        thePmntInbox->ReleaseSignatures();
                        thePmntInbox->SignContract(nym);
                        thePmntInbox->SaveContract();

                        if (!thePmntInbox->SavePaymentInbox()) {
                            LogOutput(OT_METHOD)(__FUNCTION__)(
                                ": Failure while trying to save payment "
                                "inbox.")
                                .Flush();
                        } else {
                            LogNormal(OT_METHOD)(__FUNCTION__)(
                                ": Removed instrument from payment inbox."
                                " Saved payment inbox.")
                                .Flush();
                        }
                    } else {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Failed trying to remove transaction "
                            "from payment inbox. (Should never happen).")
                            .Flush();
                    }
                    // Note: I could break right here, if this is the only
                    // transaction in the payment inbox which contains the
                    // instrument in question. Which I believe it is. Todo:
                    // if that's true, which I think it is, then call break
                    // here. After all, you wouldn't send me the SAME
                    // instrument TWICE, would you? But it still seems
                    // theoretically possible (albeit stupid.)
                }
            }
            // -----------------------------------------
            // Also, if there was a message in the outpayments box (which we
            // already removed a bit above), go ahead and add a receipt for
            // it into the record box.
            //
            if (strInstrument->Exists())  // Found the instrument in the
                                          // outpayments box.
            {
                // So what's going on here, in the bigger sense? Well, we
                // "confirmed" a payment plan, which put a copy in the
                // outpayments, and then we activated it at the server, and
                // we received the server reply, so now we're removing the
                // payment plan from the outpayments, and creating a
                // corresponding transaction record to go into the record
                // box.
                //
                // Meaning, only the Nym who actually ACTIVATES the payment
                // plan does this step. So if Alice (acting as merchant)
                // sends the payment plan request to Bob (acting as
                // customer), and Bob activates it on the server, then it is
                // Bob who does the below step. Bob thus takes the copy of
                // strInstrument from his outpayments box and makes a new
                // record in his record box. And since strInstrument from
                // his OUTPAYMENTS box includes his own transaction numbers
                // and his account number, therefore the notice we're
                // placing in his recordbox WILL include Bob's transaction
                // numbers and account number. (Which is how it should be.)
                //
                originType theOriginType = originType::not_applicable;

                if (theOutpayment->IsValid()) {
                    if (theOutpayment->IsPaymentPlan())
                        theOriginType = originType::origin_payment_plan;
                    else if (theOutpayment->IsSmartContract())
                        theOriginType = originType::origin_smart_contract;
                }

                auto pNewTransaction = api_.Factory().Transaction(
                    *theRecordBox,  // recordbox.
                    transactionType::notice,
                    theOriginType,
                    openingNumber);

                // The above has an OT_ASSERT within, but I just like to
                // check my pointers.
                if ((pNewTransaction)) {
                    // Whether the reply item we received was acknowledged
                    // or rejected, we create a corresponding
                    // itemType::notice for our new record, to save that
                    // state for the client. Our record box will contain the
                    // server's most recent version of the payment plan,
                    // (The one I just activated -- since I was the final
                    // signer...)
                    //
                    auto pNewItem = api_.Factory().Item(
                        *pNewTransaction,
                        itemType::notice,
                        api_.Factory().Identifier());
                    OT_ASSERT((pNewItem));
                    // This may be unnecessary, I'll have to check
                    // CreateItemFromTransaction.
                    // I'll leave it for now.
                    pNewItem->SetStatus(replyItem.GetStatus());
                    // Since I am the last signer, the note contains the
                    // final version of the agreement.
                    pNewItem->SetNote(serialized);
                    pNewItem->SignContract(nym);
                    pNewItem->SaveContract();

                    std::shared_ptr<Item> newItem{pNewItem.release()};
                    pNewTransaction->AddItem(newItem);
                    // -----------------------------------------------------
                    // Referencing myself here. We'll see how it works out.
                    pNewTransaction->SetReferenceToNum(openingNumber);
                    // The cheque, invoice, etc that used to be in the
                    // outpayments box.
                    pNewTransaction->SetReferenceString(strInstrument);

                    if (response.IsCancelled())
                        pNewTransaction->SetAsCancelled();

                    pNewTransaction->SignContract(nym);
                    pNewTransaction->SaveContract();

                    std::shared_ptr<OTTransaction> newTransaction{
                        pNewTransaction.release()};
                    const bool bAdded =
                        theRecordBox->AddTransaction(newTransaction);

                    if (!bAdded) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Unable to add transaction ")(
                            newTransaction->GetTransactionNum())(
                            " to record box (after tentatively removing "
                            "from payment outbox, an action that is now "
                            "cancelled).")
                            .Flush();
                    } else {
                        theRecordBox->ReleaseSignatures();
                        theRecordBox->SignContract(nym);
                        theRecordBox->SaveContract();
                        // todo log failure.
                        theRecordBox->SaveRecordBox();

                        // Any inbox/nymbox/outbox ledger will only itself
                        // contain abbreviated versions of the receipts,
                        // including their hashes.
                        //
                        // The rest is stored separately, in the box
                        // receipt, which is created whenever a receipt is
                        // added to a box, and deleted after a receipt is
                        // removed from a box.
                        //
                        if (!newTransaction->SaveBoxReceipt(*theRecordBox)) {
                            auto strNewTransaction =
                                String::Factory(*newTransaction);
                            LogOutput(OT_METHOD)(__FUNCTION__)(
                                ": For Record Box... "
                                "Failed trying to SaveBoxReceipt. "
                                "Contents: ")(strNewTransaction)(".")
                                .Flush();
                        }
                    }
                }     // if (nullptr != pNewTransaction)
                else  // should never happen
                {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failed while trying to generate "
                        "transaction in "
                        "order to add a new transaction to record box "
                        "(for a payment instrument we just removed "
                        "from the "
                        "outpayments box): ")(nymID)(".")
                        .Flush();
                }
            }  // if (strInstrument.Exists())
               // (then add a copy to record box.)
        }      // else (Success loading the payment inbox and recordBox)
    }          // (Item::rejection == replyItem.GetStatus())
               // (loading payment inbox and record box.)
}

void ServerContext::process_response_transaction_deposit(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply,
    const itemType type,
    OTTransaction& response)
{
    for (auto& it : response.GetItemList()) {
        auto pReplyItem = it;

        OT_ASSERT(pReplyItem);

        auto& item = *pReplyItem;
        // if pointer not null, and it's a deposit, and it's an
        // acknowledgement (not a rejection or error)

        if ((itemType::atDeposit == item.GetType()) ||
            (itemType::atDepositCheque == item.GetType())) {
            if (Item::acknowledgement == item.GetStatus()) {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": TRANSACTION SUCCESS -- Server acknowledges "
                    "deposit.")
                    .Flush();
                if (itemType::atDepositCheque == item.GetType()) {
                    process_response_transaction_cheque_deposit(
                        client, response.GetPurportedAccountID(), &reply, item);
                } else if (itemType::atDeposit == item.GetType()) {
#if OT_CASH
                    process_response_transaction_cash_deposit(item);
#endif
                }
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": TRANSACTION FAILURE -- Server rejects deposit.")
                    .Flush();
            }
        }
    }

    consume_issued(lock, response.GetTransactionNum());
}

void ServerContext::process_response_transaction_exchange_basket(
    const Lock& lock,
    const Message& reply,
    const itemType type,
    OTTransaction& response)
{
    consume_issued(lock, response.GetTransactionNum());
    auto item = response.GetItem(type);

    if (item && Item::rejection == item->GetStatus()) {
        auto originalItem = extract_original_item(*item);

        if (originalItem) {
            auto serialized = String::Factory();
            originalItem->GetAttachment(serialized);

            if (serialized->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Original item does not contain serialized basket")
                    .Flush();

                return;
            }

            auto pBasket = api_.Factory().Basket();

            OT_ASSERT(pBasket);

            auto& basket = *pBasket;

            if (false == basket.LoadContractFromString(serialized)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to instantiate basket")
                    .Flush();

                return;
            }

            for (auto i = 0; i < basket.Count(); ++i) {
                auto* requestItem = basket.At(i);

                if (nullptr == requestItem) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid basket")
                        .Flush();

                    return;
                }

                const auto closingNumber = requestItem->lClosingTransactionNo;
                recover_available_number(lock, closingNumber);
            }

            const auto closingNumber = basket.GetClosingNum();
            recover_available_number(lock, closingNumber);
        }
    }
}

void ServerContext::process_response_transaction_pay_dividend(
    const Lock& lock,
    const Message& reply,
    const itemType type,
    OTTransaction& response)
{
    // loop through the ALL items that make up this transaction and check to
    // see if a response to pay dividend.

    for (auto& it : response.GetItemList()) {
        auto pItem = it;
        OT_ASSERT((pItem));

        // if pointer not null, and it's a dividend payout, and it's an
        // acknowledgement (not a rejection or error)

        if (itemType::atPayDividend == pItem->GetType()) {
            if (Item::acknowledgement == pItem->GetStatus()) {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": TRANSACTION SUCCESS -- Server acknowledges "
                    "dividend "
                    "payout.")
                    .Flush();
            } else {
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": TRANSACTION FAILURE -- Server rejects dividend "
                    "payout.")
                    .Flush();
            }
        }
    }

    consume_issued(lock, response.GetTransactionNum());
}

void ServerContext::process_response_transaction_transfer(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply,
    const itemType type,
    OTTransaction& response)
{
    OT_ASSERT(nym_);

    const auto& nymID = nym_->ID();
    auto pItem = response.GetItem(type);

    if (false == bool(pItem)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Response transaction does not contain response transfer item")
            .Flush();

        return;
    }

    const auto& responseItem = *pItem;
    auto status{TransactionAttempt::Accepted};

    if (Item::rejection == pItem->GetStatus()) {
        consume_issued(lock, response.GetTransactionNum());
        status = TransactionAttempt::Rejected;
    }

    auto serialized = String::Factory();
    responseItem.GetReferenceString(serialized);

    if (serialized->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid serialized transaction item.")
            .Flush();

        return;
    }

    auto pTransfer = api_.Factory().Item(
        serialized,
        responseItem.GetRealNotaryID(),
        responseItem.GetReferenceToNum());

    if (false == bool(pTransfer)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to instantiate transaction item.")
            .Flush();

        return;
    }

    const auto& transfer = *pTransfer;

    if (is_internal_transfer(transfer)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Acknowledging internal transfer")
            .Flush();
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Acknowledging outgoing transfer")
            .Flush();
    }

    if (TransactionAttempt::Accepted == status) {

        client.Workflow().AcknowledgeTransfer(nymID, transfer, reply);
    } else {

        client.Workflow().AbortTransfer(nymID, transfer, reply);
    }
}

#if OT_CASH
void ServerContext::process_response_transaction_withdrawal(
    const Lock& lock,
    const api::client::Manager& client,
    const Message& reply,
    const itemType type,
    OTTransaction& response)
{
    OT_ASSERT(nym_);

    // loop through the ALL items that make up this transaction and check to
    // see if a response to withdrawal.

    // if pointer not null, and it's a withdrawal, and it's an
    // acknowledgement (not a rejection or error)
    for (auto& it : response.GetItemList()) {
        auto pItem = it;
        OT_ASSERT((pItem));
        // VOUCHER WITHDRAWAL
        //
        // If we got a reply to a voucher withdrawal, we'll just display the
        // voucher on the screen (if the server sent us one...)
        if ((itemType::atWithdrawVoucher == pItem->GetType()) &&
            (Item::acknowledgement == pItem->GetStatus())) {
            auto strVoucher = String::Factory();
            auto theVoucher = api_.Factory().Cheque();

            OT_ASSERT((theVoucher));

            pItem->GetAttachment(strVoucher);

            if (theVoucher->LoadContractFromString(strVoucher)) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(
                    " Received voucher from server:  ")
                    .Flush();
                LogVerbose(OT_METHOD)(__FUNCTION__)(strVoucher).Flush();
            }
        }
        // CASH WITHDRAWAL
        //
        // If the item is a response to a cash withdrawal, we want to save
        // the coins into a purse somewhere on the computer. That's cash!
        // Gotta keep it safe.
        else if (
            (itemType::atWithdrawal == pItem->GetType()) &&
            (Item::acknowledgement == pItem->GetStatus())) {
            process_incoming_cash_withdrawal(*pItem);
        }
    }

    consume_issued(lock, response.GetTransactionNum());
}

bool ServerContext::process_incoming_cash(
    const Lock& lock,
    const api::client::Manager& client,
    const TransactionNumber number,
    const PeerObject& incoming,
    const Message& message) const
{
    OT_ASSERT(nym_);
    OT_ASSERT(proto::PEEROBJECT_CASH == incoming.Type());

    if (false == incoming.Validate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid peer object.").Flush();

        return false;
    }

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    const auto& serverID = server_id_.get();
    const auto strNotaryID = String::Factory(serverID);
    const auto strNymID = String::Factory(nymID);
    const auto pPurse = incoming.Purse();

    if (false == bool(pPurse)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No purse found").Flush();

        return false;
    }

    auto& purse = *pPurse;
    const auto workflowID =
        client.Workflow().ReceiveCash(nymID, purse, message);

    if (workflowID->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create workflow")
            .Flush();

        return false;
    }

    LogDetail(OT_METHOD)(__FUNCTION__)(": Created workflow ")(workflowID)
        .Flush();

    return true;
}

void ServerContext::process_incoming_cash_withdrawal(const Item& item) const
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    const auto& nym = *nym_;
    const auto& serverNym = *remote_nym_;
    const auto& nymID = nym.ID();
    auto rawPurse = Data::Factory();
    item.GetAttachment(rawPurse);
    const auto serializedPurse = proto::DataToProto<proto::Purse>(rawPurse);
    std::shared_ptr<blind::Purse> pPurse{};

    if (proto::Validate(serializedPurse, VERBOSE)) {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Serialized purse is valid")
            .Flush();
        pPurse.reset(api_.Factory().Purse(serializedPurse).release());
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid serialized purse")
            .Flush();

        return;
    }

    if (pPurse) {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Purse instantiated").Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate purse")
            .Flush();

        return;
    }

    auto& requestPurse = *pPurse;

    if (requestPurse.Unlock(nym)) {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Purse unlocked").Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to unlock purse").Flush();

        return;
    }

    auto pMint = api_.Factory().Mint(
        String::Factory(server_id_), String::Factory(requestPurse.Unit()));

    if (pMint) {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Mint loaded").Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load mint").Flush();

        return;
    }

    auto& mint = *pMint;
    const bool validMint = mint.LoadMint() && mint.VerifyMint(serverNym);

    if (validMint) {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Mint is valid").Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid mint").Flush();

        return;
    }

    const auto processed = requestPurse.Process(nym, mint);

    if (processed) {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Token processed").Flush();
        auto purseEditor = api_.Wallet().mutable_Purse(
            nymID, requestPurse.Notary(), requestPurse.Unit());
        auto& walletPurse = purseEditor.It();

        OT_ASSERT(walletPurse.Unlock(nym));

        auto token = requestPurse.Pop();

        while (token) {
            walletPurse.Push(token);
            token = requestPurse.Pop();
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to process token").Flush();
    }
}
#endif

bool ServerContext::process_unregister_account_response(
    const Lock& lock,
    const Message& reply)
{
    auto serialized = String::Factory();

    if (reply.m_ascInReferenceTo->Exists()) {
        reply.m_ascInReferenceTo->GetString(serialized);
    }

    auto originalMessage = api_.Factory().Message();

    OT_ASSERT((originalMessage));

    if (serialized->Exists() &&
        originalMessage->LoadContractFromString(serialized) &&
        originalMessage->VerifySignature(*Nym()) &&
        originalMessage->m_strNymID->Compare(reply.m_strNymID) &&
        originalMessage->m_strAcctID->Compare(reply.m_strAcctID) &&
        originalMessage->m_strCommand->Compare("unregisterAccount")) {

        const auto theAccountID = api_.Factory().Identifier(reply.m_strAcctID);
        auto account = api_.Wallet().mutable_Account(theAccountID);

        if (account) {
            account.Release();
            api_.Wallet().DeleteAccount(theAccountID);
        }

        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Successfully DELETED Asset Acct ")(reply.m_strAcctID)(
            " from Server: ")(server_id_)(".")
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": The server just for some reason tried to trick me into "
            "erasing my account ")(reply.m_strAcctID)(" on Server ")(
            server_id_)(".")
            .Flush();
    }

    return true;
}

bool ServerContext::process_unregister_nym_response(
    const Lock& lock,
    const Message& reply)
{
    auto serialized = String::Factory();
    auto originalMessage = api_.Factory().Message();

    OT_ASSERT((originalMessage));

    if (reply.m_ascInReferenceTo->Exists()) {
        reply.m_ascInReferenceTo->GetString(serialized);
    }

    if (serialized->Exists() &&
        originalMessage->LoadContractFromString(serialized) &&
        originalMessage->VerifySignature(*Nym()) &&
        originalMessage->m_strNymID->Compare(reply.m_strNymID) &&
        originalMessage->m_strCommand->Compare("unregisterNym")) {
        Reset();
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Successfully DELETED Nym from Server: removed request "
            "number, plus all issued and transaction numbers for Nym ")(
            reply.m_strNymID)(" for Server ")(server_id_)(".")
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": The server just for some reason tried to trick me into "
            "erasing my issued and transaction numbers for Nym ")(
            reply.m_strNymID)(", Server ")(server_id_)(".")
            .Flush();
    }

    return true;
}

void ServerContext::process_unseen_reply(
    const Lock& lock,
    const api::client::Manager& client,
    const Item& input,
    ReplyNoticeOutcomes& notices)
{
    if (Item::acknowledgement != input.GetStatus()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Reply item incorrect status")
            .Flush();

        return;
    }

    auto serializedReply = String::Factory();
    input.GetAttachment(serializedReply);

    if (!serializedReply->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading original server reply message from notice.")
            .Flush();

        return;
    }

    std::shared_ptr<Message> message{api_.Factory().Message()};

    OT_ASSERT(message);

    if (false == message->LoadContractFromString(serializedReply)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed loading original server "
            "reply message from replyNotice: ")(serializedReply)
            .Flush();

        return;
    }

    ReplyNoticeOutcome outcome{};
    auto& [number, result] = outcome;
    auto& [status, reply] = result;
    number = String::StringToUlong(message->m_strRequestNum->Get());
    status = (message->m_bSuccess) ? proto::LASTREPLYSTATUS_MESSAGESUCCESS
                                   : proto::LASTREPLYSTATUS_MESSAGEFAILED;
    reply = message;
    notices.emplace_back(outcome);
    process_reply(lock, client, {}, *reply);
}

#if OT_CASH
std::shared_ptr<const blind::Purse> ServerContext::Purse(
    const identifier::UnitDefinition& id) const
{
    return api_.Wallet().Purse(nym_->ID(), server_id_, id);
}
#endif

ServerContext::QueueResult ServerContext::Queue(
    const api::client::Manager& client,
    std::shared_ptr<Message> message,
    const ExtraArgs& args)
{
    START();

    return start(lock, client, message, args);
}

ServerContext::QueueResult ServerContext::Queue(
    const api::client::Manager& client,
    std::shared_ptr<Message> message,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox,
    std::set<OTManagedNumber>* numbers,
    const ExtraArgs& args)
{
    START();

    if (false == bool(inbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Inbox is not instantiated")
            .Flush();

        return {};
    }

    if (false == bool(outbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Outbox is not instantiated")
            .Flush();

        return {};
    }

    return start(
        lock,
        client,
        message,
        args,
        proto::DELIVERTYSTATE_PENDINGSEND,
        ActionType::Normal,
        inbox,
        outbox,
        numbers);
}

ServerContext::QueueResult ServerContext::RefreshNymbox(
    const api::client::Manager& client)
{
    START();

    return start(
        lock,
        client,
        nullptr,
        {},
        proto::DELIVERTYSTATE_NEEDNYMBOX,
        ActionType::ProcessNymbox);
}

bool ServerContext::remove_acknowledged_number(
    const Lock& lock,
    const Message& reply)
{
    OT_ASSERT(verify_write_lock(lock));

    std::set<RequestNumber> list{};

    if (false == reply.m_AcknowledgedReplies.Output(list)) { return false; }

    return remove_acknowledged_number(lock, list);
}

bool ServerContext::remove_nymbox_item(
    const Lock& lock,
    const Item& replyItem,
    Ledger& nymbox,
    OTTransaction& transaction)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    itemType requestType = itemType::error_state;
    auto replyType = String::Factory();
    replyItem.GetTypeString(replyType);

    switch (replyItem.GetType()) {
        case itemType::atAcceptFinalReceipt: {
            requestType = itemType::acceptFinalReceipt;
        } break;
        case itemType::atAcceptMessage: {
            requestType = itemType::acceptMessage;
        } break;
        case itemType::atAcceptNotice: {
            requestType = itemType::acceptNotice;
        } break;
        case itemType::atAcceptTransaction: {
            requestType = itemType::acceptTransaction;
        } break;
        case itemType::atTransactionStatement: {
            requestType = itemType::transactionStatement;
            // (The transaction statement itself is already handled

            return true;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unexpected replyItem type while processing Nymbox: ")(
                replyType)
                .Flush();

            return false;
        }
    }

    if (Item::acknowledgement != replyItem.GetStatus()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": processNymboxResponse reply item ")(replyType)(
            ": status == FAILED")
            .Flush();

        return true;
    }

    LogDetail(OT_METHOD)(__FUNCTION__)(": processNymboxResponse reply item ")(
        replyType)(": status == SUCCESS")
        .Flush();
    auto serialized = String::Factory();
    replyItem.GetReferenceString(serialized);
    auto processNymboxItem = api_.Factory().Item(serialized, server_id_, 0);

    if (false == bool(processNymboxItem)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to find original item in original processNymbox "
            "transaction request, based on reply item.")
            .Flush();

        return false;
    }

    auto item =
        transaction.GetItemInRefTo(processNymboxItem->GetReferenceToNum());

    OT_ASSERT(item);

    if (item->GetType() != requestType) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Wrong original item TYPE, on reply item's copy of original "
            "item, than what was expected based on reply item's type.")
            .Flush();

        return false;
    }

    std::shared_ptr<OTTransaction> serverTransaction;

    switch (replyItem.GetType()) {
        case itemType::atAcceptNotice:
        case itemType::atAcceptMessage:
        case itemType::atAcceptTransaction:
        case itemType::atAcceptFinalReceipt: {
            serverTransaction =
                nymbox.GetTransaction(item->GetReferenceToNum());
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unexpected replyItem::type while processing Nymbox: ")(
                replyType)
                .Flush();

            return false;
        }
    }

    if (false == bool(serverTransaction)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": The original processNymbox item referred to txn number ")(
            item->GetReferenceToNum())(
            ", but that receipt wasn't in my Nymbox. (We probably processed "
            "this server reply ALREADY, and now we're just seeing it again, "
            "since an extra copy was dropped into the Nymbox originally. It "
            "happens. Skipping).")
            .Flush();

        return true;
    }

    switch (replyItem.GetType()) {
        case itemType::atAcceptNotice: {
            if (transactionType::notice != serverTransaction->GetType()) {
                break;
            }

            if ((Item::rejection != replyItem.GetStatus()) &&
                (Item::acknowledgement != replyItem.GetStatus())) {
                break;
            }

            auto strOriginalCronItem = String::Factory();
            serverTransaction->GetReferenceString(strOriginalCronItem);
            const originType theOriginType = serverTransaction->GetOriginType();
            auto strUpdatedCronItem = String::Factory();
            auto pNoticeItem = serverTransaction->GetItem(itemType::notice);

            if ((pNoticeItem)) { pNoticeItem->GetNote(strUpdatedCronItem); }

            auto pOriginalCronItem =
                (strOriginalCronItem->Exists()
                     ? api_.Factory().CronItem(strOriginalCronItem)
                     : nullptr);
            auto pUpdatedCronItem =
                (strUpdatedCronItem->Exists()
                     ? api_.Factory().CronItem(strUpdatedCronItem)
                     : nullptr);
            std::unique_ptr<OTCronItem>& pCronItem =
                ((pUpdatedCronItem) ? pUpdatedCronItem : pOriginalCronItem);

            if (false == bool(pCronItem) || false == bool(pOriginalCronItem)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error loading original CronItem from Nymbox receipt, "
                    "from string: ")(strOriginalCronItem)(".")
                    .Flush();

                return false;
            }

            auto theCancelerNymID = api_.Factory().NymID();
            const TransactionNumber openingNumber =
                pOriginalCronItem->GetOpeningNumber(nymID);
            const bool bCancelling =
                (pCronItem->IsCanceled() &&
                 pCronItem->GetCancelerID(theCancelerNymID));
            const bool bIsCancelerNym =
                (bCancelling && (nymID == theCancelerNymID));
            const bool bIsActivatingNym =
                (pCronItem->GetOpeningNum() == openingNumber);

            if ((bCancelling && !bIsCancelerNym) ||
                (!bCancelling && !bIsActivatingNym)) {
                if (Item::rejection == replyItem.GetStatus()) {
                    if (false == consume_issued(lock, openingNumber)) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Error removing issued number from user nym (for "
                            "a cron item).")
                            .Flush();
                    }

                    for (std::int32_t i = 0;
                         i < pOriginalCronItem->GetCountClosingNumbers();
                         ++i) {
                        recover_available_number(
                            lock,
                            pOriginalCronItem->GetClosingTransactionNoAt(i));
                    }
                } else {
                    pCronItem->SaveActiveCronReceipt(nymID);
                }

                NumList numlistOutpayment(openingNumber);
                auto strSentInstrument = String::Factory();
                auto nymfile = mutable_Nymfile("");
                auto pMsg =
                    nymfile.It().GetOutpaymentsByTransNum(openingNumber);

                if (pMsg) {
                    const bool bRemovedOutpayment =
                        nymfile.It().RemoveOutpaymentsByTransNum(openingNumber);

                    if (!bRemovedOutpayment) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Failed trying to remove outpayment with trans "
                            "num: ")(openingNumber)(".")
                            .Flush();
                    }

                    if (!pMsg->m_ascPayload->GetString(strSentInstrument)) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Unable to find payment "
                            "instrument in outpayment "
                            "message with trans num: ")(openingNumber)(".")
                            .Flush();
                    }
                }

                const auto notaryID = String::Factory(server_id_);
                const bool exists1 = OTDB::Exists(
                    api_.DataFolder(),
                    OTFolders::PaymentInbox().Get(),
                    notaryID->Get(),
                    nymID.str(),
                    "");
                const bool exists2 = OTDB::Exists(
                    api_.DataFolder(),
                    OTFolders::RecordBox().Get(),
                    notaryID->Get(),
                    nymID.str(),
                    "");
                auto paymentInbox =
                    api_.Factory().Ledger(nymID, nymID, server_id_);

                OT_ASSERT((paymentInbox));

                auto recordBox =
                    api_.Factory().Ledger(nymID, nymID, server_id_);

                OT_ASSERT((recordBox));

                bool loaded1 = (exists1 && paymentInbox->LoadPaymentInbox());
                bool loaded2 = (exists2 && recordBox->LoadRecordBox());

                if (exists1 && loaded1) {
                    loaded1 =
                        (paymentInbox->VerifyContractID() &&
                         paymentInbox->VerifySignature(nym));
                } else if (!exists1) {
                    loaded1 = paymentInbox->GenerateLedger(
                        nymID, server_id_, ledgerType::paymentInbox, true);
                }

                if (exists2 && loaded2) {
                    loaded2 =
                        (recordBox->VerifyContractID() &&
                         recordBox->VerifySignature(nym));
                } else if (!exists2) {
                    loaded2 = recordBox->GenerateLedger(
                        nymID, server_id_, ledgerType::recordBox, true);
                }

                if (!loaded1 || !loaded2) {
                    LogNormal(OT_METHOD)(__FUNCTION__)(
                        ": While processing server rejection of cron item: "
                        "WARNING: Unable to load, verify, or generate "
                        "paymentInbox or recordBox, with IDs: ")(nymID)(" / ")(
                        nymID)(".")
                        .Flush();
                } else {
                    auto theOutpayment = api_.Factory().Payment();

                    OT_ASSERT((theOutpayment));

                    if (strSentInstrument->Exists() &&
                        theOutpayment->SetPayment(strSentInstrument) &&
                        theOutpayment->SetTempValues()) {
                        theOutpayment->GetAllTransactionNumbers(
                            numlistOutpayment);
                    }

                    auto tempPayment = api_.Factory().Payment();

                    OT_ASSERT((tempPayment));

                    const String& strCronItem =
                        (strUpdatedCronItem->Exists() ? strUpdatedCronItem
                                                      : strOriginalCronItem);

                    if (strCronItem.Exists() &&
                        tempPayment->SetPayment(strCronItem) &&
                        tempPayment->SetTempValues()) {
                        tempPayment->GetAllTransactionNumbers(
                            numlistOutpayment);
                    }

                    const std::set<std::int64_t> set_receipt_ids{
                        paymentInbox->GetTransactionNums()};

                    for (const auto& receipt_id : set_receipt_ids) {
                        auto pPayment = GetInstrumentByReceiptID(
                            nym, receipt_id, *paymentInbox);

                        if (false == bool(pPayment)) {
                            LogNormal(OT_METHOD)(__FUNCTION__)(
                                ": (Upon receiving notice) While looping "
                                "payments inbox to remove a payment, unable to "
                                "retrieve payment on receipt ")(receipt_id)(
                                " (skipping).")
                                .Flush();
                            continue;
                        } else if (false == pPayment->SetTempValues()) {
                            LogNormal(OT_METHOD)(__FUNCTION__)(
                                ": (Upon receiving notice) While looping "
                                "payments inbox to remove a payment, unable to "
                                "set temp values for payment on receipt ")(
                                receipt_id)(" (skipping).")
                                .Flush();
                            continue;
                        }

                        NumList numlistIncomingPayment;
                        pPayment->GetAllTransactionNumbers(
                            numlistIncomingPayment);

                        if (numlistOutpayment.VerifyAny(
                                numlistIncomingPayment)) {
                            auto pTransPaymentInbox =
                                paymentInbox->GetTransaction(receipt_id);

                            OT_ASSERT(pTransPaymentInbox);

                            if (false ==
                                paymentInbox->DeleteBoxReceipt(receipt_id)) {
                                LogOutput(OT_METHOD)(__FUNCTION__)(
                                    ": Failed trying to delete the box receipt "
                                    "for a transaction being removed from the "
                                    "payment inbox.")
                                    .Flush();
                            }

                            if (paymentInbox->RemoveTransaction(receipt_id)) {
                                paymentInbox->ReleaseSignatures();
                                paymentInbox->SignContract(nym);
                                paymentInbox->SaveContract();

                                if (!paymentInbox->SavePaymentInbox()) {
                                    LogOutput(OT_METHOD)(__FUNCTION__)(
                                        ": Failure while trying to save "
                                        "payment inbox.")
                                        .Flush();
                                } else {
                                    LogNormal(OT_METHOD)(__FUNCTION__)(
                                        ": Removed instrument from payment "
                                        "inbox. Saved payment inbox.")
                                        .Flush();
                                }
                            } else {
                                LogOutput(OT_METHOD)(__FUNCTION__)(
                                    ": Failed trying to remove transaction "
                                    "from payment inbox. (Should never "
                                    "happen).")
                                    .Flush();
                            }
                        }
                    }

                    std::shared_ptr<OTTransaction> newTransaction{
                        api_.Factory().Transaction(
                            *recordBox,
                            transactionType::notice,
                            theOriginType,
                            serverTransaction->GetTransactionNum())};

                    if (newTransaction) {
                        if (false != bool(pNoticeItem)) {
                            std::shared_ptr<Item> newItem{api_.Factory().Item(
                                *newTransaction,
                                itemType::notice,
                                api_.Factory().Identifier())};

                            OT_ASSERT(newItem);

                            newItem->SetStatus(pNoticeItem->GetStatus());
                            newItem->SetNote(strUpdatedCronItem);
                            newItem->SignContract(nym);
                            newItem->SaveContract();
                            newTransaction->AddItem(newItem);
                        }

                        TransactionNumber lTransNumForDisplay{0};

                        if (!theOutpayment->IsValid() ||
                            !theOutpayment->GetTransNumDisplay(
                                lTransNumForDisplay)) {
                            auto tempPayment = api_.Factory().Payment();

                            OT_ASSERT(false != bool(tempPayment));

                            const String& strCronItem =
                                (strUpdatedCronItem->Exists()
                                     ? strUpdatedCronItem
                                     : strOriginalCronItem);

                            if (strCronItem.Exists() &&
                                tempPayment->SetPayment(strCronItem) &&
                                tempPayment->SetTempValues()) {
                                tempPayment->GetTransNumDisplay(
                                    lTransNumForDisplay);
                            }
                        }

                        newTransaction->SetReferenceToNum(lTransNumForDisplay);

                        if (strSentInstrument->Exists()) {
                            newTransaction->SetReferenceString(
                                strSentInstrument);
                        } else if (strOriginalCronItem->Exists()) {
                            newTransaction->SetReferenceString(
                                strOriginalCronItem);
                        }

                        if (bCancelling) { newTransaction->SetAsCancelled(); }

                        newTransaction->SignContract(nym);
                        newTransaction->SaveContract();
                        const bool bAdded =
                            recordBox->AddTransaction(newTransaction);

                        if (!bAdded) {
                            LogOutput(OT_METHOD)(__FUNCTION__)(
                                ": Unable to add txn ")(
                                newTransaction->GetTransactionNum())(
                                " to record box (after tentatively removing "
                                "from payment outbox, an action that is now "
                                "canceled).")
                                .Flush();

                            return false;
                        }

                        recordBox->ReleaseSignatures();
                        recordBox->SignContract(nym);
                        recordBox->SaveContract();
                        recordBox->SaveRecordBox();

                        if (!newTransaction->SaveBoxReceipt(*recordBox)) {
                            auto strNewTransaction =
                                String::Factory(*newTransaction);
                            LogOutput(OT_METHOD)(__FUNCTION__)(
                                ": For Record Box... Failed trying to "
                                "SaveBoxReceipt. Contents: ")(
                                strNewTransaction)(".")
                                .Flush();
                        }
                    } else {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Failed while trying to generate transaction in "
                            "order to add a new transaction to record box (for "
                            "a payment instrument we just removed from the "
                            "outpayments box): ")(nymID)(".")
                            .Flush();
                    }
                }
            }
        } break;
        case itemType::atAcceptMessage:
        case itemType::atAcceptTransaction: {
            // I don't think we need to do anything here...
        } break;
        case itemType::atAcceptFinalReceipt: {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Successfully removed finalReceipt from Nymbox with opening "
                "num: ")(serverTransaction->GetReferenceToNum())
                .Flush();
            const bool removed =
                consume_issued(lock, serverTransaction->GetReferenceToNum());

            if (removed) {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": **** Due to finding a finalReceipt, consuming issued "
                    "opening number from nym:  ")(
                    serverTransaction->GetReferenceToNum())
                    .Flush();
            } else {
                LogDetail(OT_METHOD)(__FUNCTION__)(
                    ": **** Noticed a finalReceipt, but Opening Number ")(
                    serverTransaction->GetReferenceToNum())(
                    " had ALREADY been removed from nym.")
                    .Flush();
            }

            OTCronItem::EraseActiveCronReceipt(
                api_.DataFolder(),
                serverTransaction->GetReferenceToNum(),
                nymID,
                serverTransaction->GetPurportedNotaryID());
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unexpected replyItem:type while processing Nymbox: ")(
                replyType)(".")
                .Flush();

            return false;
        }
    }

    serverTransaction->DeleteBoxReceipt(nymbox);
    nymbox.RemoveTransaction(serverTransaction->GetTransactionNum());

    return true;
}

bool ServerContext::remove_tentative_number(
    const Lock& lock,
    const TransactionNumber& number)
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = tentative_transaction_numbers_.erase(number);

    return (0 < output);
}

bool ServerContext::RemoveTentativeNumber(const TransactionNumber& number)
{
    Lock lock(lock_);

    return remove_tentative_number(lock, number);
}

void ServerContext::ResetThread() { Join(); }

void ServerContext::resolve_queue(
    const Lock& contextLock,
    DeliveryResult&& result,
    const proto::DeliveryState state)
{
    OT_ASSERT(verify_write_lock(contextLock));

    if (proto::DELIVERTYSTATE_ERROR != state) { state_.store(state); }

    last_status_.store(std::get<0>(result));

    inbox_.reset();
    outbox_.reset();
    pending_result_.set_value(std::move(result));
    pending_result_set_.store(true);
    pending_message_.reset();
    pending_args_ = {"", false};
    process_nymbox_.store(false);
    const auto saved = save(contextLock);

    OT_ASSERT(saved);
}

bool ServerContext::Resync(const proto::Context& serialized)
{
    Lock lock(lock_);

    return resync(lock, serialized);
}

bool ServerContext::resync(const Lock& lock, const proto::Context& serialized)
{
    OT_ASSERT(verify_write_lock(lock));

    TransactionNumbers serverNumbers{};

    for (const auto& number : serialized.issuedtransactionnumber()) {
        serverNumbers.insert(number);
        auto exists = (1 == issued_transaction_numbers_.count(number));

        if (false == exists) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Server believes number ")(
                number)(" is still issued. Restoring.")
                .Flush();
            issued_transaction_numbers_.insert(number);
            available_transaction_numbers_.insert(number);
        }
    }

    for (const auto& number : issued_transaction_numbers_) {
        auto exists = (1 == serverNumbers.count(number));

        if (false == exists) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Server believes number ")(
                number)(" is no longer issued. Removing.")
                .Flush();
            issued_transaction_numbers_.erase(number);
            available_transaction_numbers_.erase(number);
        }
    }

    TransactionNumbers notUsed{};
    update_highest(lock, issued_transaction_numbers_, notUsed, notUsed);

    return true;
}

std::uint64_t ServerContext::Revision() const { return revision_.load(); }

void ServerContext::update_state(
    const Lock& contextLock,
    const proto::DeliveryState state,
    const proto::LastReplyStatus status)
{
    state_.store(state);

    if (proto::LASTREPLYSTATUS_INVALID != status) {
        last_status_.store(status);
    }

    const auto saved = save(contextLock);

    OT_ASSERT(saved);
}

void ServerContext::scan_number_set(
    const TransactionNumbers& input,
    TransactionNumber& highest,
    TransactionNumber& lowest)
{
    highest = 0;
    lowest = 0;

    if (0 < input.size()) {
        lowest = *input.cbegin();
        highest = *input.crbegin();
    }
}

NetworkReplyMessage ServerContext::SendMessage(
    const api::client::Manager& client,
    const std::set<OTManagedNumber>& pending,
    opentxs::ServerContext& context,
    const Message& message,
    const std::string& label,
    const bool resync)
{
    Lock lock(lock_);
    pending_args_ = {label, resync};
    client.OTAPI().GetClient()->QueueOutgoingMessage(message);
    request_sent_.Publish(message.m_strCommand->Get());
    auto result = context.Connection().Send(context, message);

    if (SendResult::VALID_REPLY == result.first) {
        process_reply(lock, client, pending, *result.second);
        reply_received_.Publish(message.m_strCommand->Get());
    }

    return result;
}

proto::Context ServerContext::serialize(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock, Type());
    auto& server = *output.mutable_servercontext();
    server.set_version(output.version());
    server.set_serverid(String::Factory(server_id_)->Get());
    server.set_highesttransactionnumber(highest_transaction_number_.load());

    for (const auto& it : tentative_transaction_numbers_) {
        server.add_tentativerequestnumber(it);
    }

    if (output.version() >= 2) {
        server.set_revision(revision_.load());
        server.set_adminpassword(admin_password_);
        server.set_adminattempted(admin_attempted_.get());
        server.set_adminsuccess(admin_success_.get());
    }

    if (output.version() >= 3) {
        server.set_state(state_.load());
        server.set_laststatus(last_status_.load());

        if (pending_message_) {
            auto& pending = *server.mutable_pending();
            pending.set_version(PENDING_COMMAND_VERSION);
            pending.set_serialized(String::Factory(*pending_message_)->Get());
            pending.set_accountlabel(std::get<0>(pending_args_));
            pending.set_resync(std::get<1>(pending_args_));
        }
    }

    return output;
}

const identifier::Nym& ServerContext::server_nym_id(const Lock& lock) const
{
    OT_ASSERT(remote_nym_);

    return remote_nym_->ID();
}

void ServerContext::SetAdminAttempted()
{
    Lock lock(lock_);
    admin_attempted_->On();
}

void ServerContext::SetAdminPassword(const std::string& password)
{
    Lock lock(lock_);
    admin_password_ = password;
}

void ServerContext::SetAdminSuccess()
{
    Lock lock(lock_);
    admin_attempted_->On();
    admin_success_->On();
}

bool ServerContext::SetHighest(const TransactionNumber& highest)
{
    Lock lock(lock_);

    if (highest >= highest_transaction_number_.load()) {
        highest_transaction_number_.store(highest);

        return true;
    }

    return false;
}

void ServerContext::SetRevision(const std::uint64_t revision)
{
    Lock lock(lock_);
    revision_.store(revision);
}

bool ServerContext::StaleNym() const
{
    Lock lock(lock_);

    OT_ASSERT(nym_);

    return revision_.load() < nym_->Revision();
}

std::unique_ptr<Item> ServerContext::Statement(const OTTransaction& owner) const
{
    const TransactionNumbers empty;

    return Statement(owner, empty);
}

std::unique_ptr<Item> ServerContext::statement(
    const Lock& lock,
    const OTTransaction& transaction,
    const TransactionNumbers& adding) const
{
    OT_ASSERT(verify_write_lock(lock));

    std::unique_ptr<Item> output;

    OT_ASSERT(nym_);

    if ((transaction.GetNymID() != nym_->ID())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Transaction has wrong owner.")
            .Flush();

        return output;
    }

    // transaction is the depositPaymentPlan, activateSmartContract, or
    // marketOffer that triggered the need for this transaction statement.
    // Since it uses up a transaction number, I will be sure to remove that one
    // from my list before signing the list.
    output = api_.Factory().Item(
        transaction,
        itemType::transactionStatement,
        api_.Factory().Identifier());

    if (false == bool(output)) { return output; }

    const TransactionNumbers empty;
    auto statement = generate_statement(lock, adding, empty);

    if (!statement) { return output; }

    switch (transaction.GetType()) {
        case transactionType::cancelCronItem: {
            if (transaction.GetTransactionNum() > 0) {
                statement->Remove(transaction.GetTransactionNum());
            }
        } break;
        // Transaction Statements usually only have a transaction number in the
        // case of market offers and payment plans, in which case the number
        // should NOT be removed, and remains in play until final closure from
        // Cron.
        case transactionType::marketOffer:
        case transactionType::paymentPlan:
        case transactionType::smartContract:
        default: {
        }
    }

    // What about cases where no number is being used? (Such as processNymbox)
    // Perhaps then if this function is even called, it's with a 0-number
    // transaction, in which case the above Removes probably won't hurt
    // anything.  Todo.

    output->SetAttachment(OTString(*statement));
    output->SignContract(*nym_);
    // OTTransactionType needs to weasel in a "date signed" variable.
    output->SaveContract();

    return output;
}

std::unique_ptr<Item> ServerContext::Statement(
    const OTTransaction& transaction,
    const TransactionNumbers& adding) const
{
    Lock lock(lock_);

    return statement(lock, transaction, adding);
}

std::unique_ptr<TransactionStatement> ServerContext::Statement(
    const TransactionNumbers& adding,
    const TransactionNumbers& without) const
{
    Lock lock(lock_);

    return generate_statement(lock, adding, without);
}

bool ServerContext::ShouldRename(const std::string& defaultName) const
{
    const auto& name = defaultName.empty() ? default_node_name_ : defaultName;

    if (false == admin_success_.get()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Do not have admin permission.")
            .Flush();

        return false;
    }

    auto contract = api_.Wallet().Server(server_id_);

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing server contract.")
            .Flush();

        return false;
    }

    return (contract->Alias() == name);
}

ServerContext::QueueResult ServerContext::start(
    const Lock& decisionLock,
    const api::client::Manager& client,
    std::shared_ptr<Message> message,
    const ExtraArgs& args,
    const proto::DeliveryState state,
    const ActionType type,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox,
    std::set<OTManagedNumber>* numbers)
{
    Lock contextLock(lock_);
    client_.store(&client);
    process_nymbox_.store(static_cast<bool>(type));
    pending_message_ = message;
    pending_args_ = args;
    pending_result_set_.store(false);
    pending_result_ = std::promise<DeliveryResult>();
    inbox_ = inbox;
    outbox_ = outbox;
    numbers_ = numbers;
    failure_counter_.store(0);
    update_state(contextLock, state);

    if (trigger(decisionLock)) {
        return std::make_unique<SendFuture>(pending_result_.get_future());
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to queue task").Flush();

    return {};
}

bool ServerContext::state_machine() noexcept
{
    const auto* pClient = client_.load();

    OT_ASSERT(nullptr != pClient);

    const auto& client = *pClient;

    switch (state_.load()) {
        case proto::DELIVERTYSTATE_PENDINGSEND: {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Attempting to send message")
                .Flush();
            pending_send(client);
        } break;
        case proto::DELIVERTYSTATE_NEEDNYMBOX: {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Downloading nymbox").Flush();
            need_nymbox(client);
        } break;
        case proto::DELIVERTYSTATE_NEEDBOXITEMS: {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Downloading box items")
                .Flush();
            need_box_items(client);
        } break;
        case proto::DELIVERTYSTATE_NEEDPROCESSNYMBOX: {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Processing nymbox").Flush();
            need_process_nymbox(client);
        } break;
        case proto::DELIVERTYSTATE_IDLE:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected state").Flush();
        }
    }

    const bool more = proto::DELIVERTYSTATE_IDLE != state_.load();
    const bool retry = FAILURE_COUNT_LIMIT >= failure_counter_.load();

    if (false == more) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Delivery complete").Flush();
    } else if (false == retry) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error limit exceeded").Flush();
        Lock lock(lock_);
        resolve_queue(
            lock,
            DeliveryResult{proto::LASTREPLYSTATUS_NOTSENT, nullptr},
            proto::DELIVERTYSTATE_IDLE);
    } else if (shutdown().load()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Shutting down").Flush();
        Lock lock(lock_);
        resolve_queue(
            lock,
            DeliveryResult{proto::LASTREPLYSTATUS_NOTSENT, nullptr},
            proto::DELIVERTYSTATE_IDLE);
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Continuing").Flush();

        return true;
    }

    return false;
}

proto::ConsensusType ServerContext::Type() const
{
    return proto::CONSENSUSTYPE_SERVER;
}

TransactionNumber ServerContext::update_highest(
    const Lock& lock,
    const TransactionNumbers& numbers,
    TransactionNumbers& good,
    TransactionNumbers& bad)
{
    OT_ASSERT(verify_write_lock(lock));

    TransactionNumber output = 0;  // 0 is success.
    TransactionNumber highest = 0;
    TransactionNumber lowest = 0;

    scan_number_set(numbers, highest, lowest);

    const TransactionNumber oldHighest = highest_transaction_number_.load();

    validate_number_set(numbers, oldHighest, good, bad);

    if ((lowest > 0) && (lowest <= oldHighest)) {
        // Return the first invalid number
        output = lowest;
    }

    if (!good.empty()) {
        if (0 != oldHighest) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Raising Highest Transaction Number "
                "from ")(oldHighest)(" to ")(highest)(".")
                .Flush();
        } else {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Creating Highest Transaction Number "
                "entry for this server as '")(highest)("'.")
                .Flush();
        }

        highest_transaction_number_.store(highest);
    }

    return output;
}

OTIdentifier ServerContext::update_remote_hash(
    const Lock& lock,
    const Message& reply)
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = api_.Factory().Identifier();
    const auto& input = reply.m_strNymboxHash;

    if (input->Exists()) {
        output->SetString(input);
        remote_nymbox_hash_ = output;
    }

    return output;
}

bool ServerContext::update_nymbox_hash(
    const Lock& lock,
    const Message& reply,
    const UpdateHash which)
{
    if (false == reply.m_strNymboxHash->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No hash provided by notary")
            .Flush();

        return false;
    }

    const auto hash = api_.Factory().Identifier(reply.m_strNymboxHash);
    set_remote_nymbox_hash(lock, hash);

    if (UpdateHash::Both == which) { set_local_nymbox_hash(lock, hash); }

    return true;
}

RequestNumber ServerContext::update_request_number(
    const Lock& contextLock,
    [[maybe_unused]] const Lock& messageLock,
    bool& sendStatus)
{
    sendStatus = false;

    auto request = initialize_server_command(MessageType::getRequestNumber);

    if (false == bool(request)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to initialize server message.")
            .Flush();

        return {};
    }

    request->m_strRequestNum =
        String::Factory(std::to_string(FIRST_REQUEST_NUMBER).c_str());

    if (false == finalize_server_command(*request)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to finalize server message.")
            .Flush();

        return {};
    }

    const auto response = connection_.Send(
        *this,
        *request,
        static_cast<opentxs::network::ServerConnection::Push>(
            enable_otx_push_.load()));
    const auto& status = response.first;
    const auto& reply = response.second;

    switch (status) {
        case SendResult::TIMEOUT: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Reply timeout.").Flush();

            return {};
        } break;
        case SendResult::INVALID_REPLY: {
            sendStatus = true;
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid reply.").Flush();

            return {};
        } break;
        case SendResult::VALID_REPLY: {
            sendStatus = true;
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown error.").Flush();

            return {};
        }
    }

    OT_ASSERT(reply);

    const RequestNumber newNumber = reply->m_lNewRequestNum;
    add_acknowledged_number(contextLock, newNumber);
    remove_acknowledged_number(contextLock, *reply);
    request_number_.store(newNumber);
    update_remote_hash(contextLock, *reply);

    return newNumber;
}

bool ServerContext::update_request_number(const Lock& lock, Message& command)
{
    OT_ASSERT(verify_write_lock(lock));

    command.m_strRequestNum =
        String::Factory(std::to_string(request_number_++).c_str());

    return finalize_server_command(command);
}

TransactionNumber ServerContext::UpdateHighest(
    const TransactionNumbers& numbers,
    TransactionNumbers& good,
    TransactionNumbers& bad)
{
    Lock lock(lock_);

    return update_highest(lock, numbers, good, bad);
}

bool ServerContext::UpdateRequestNumber(Message& command)
{
    Lock lock(lock_);

    return update_request_number(lock, command);
}

void ServerContext::validate_number_set(
    const TransactionNumbers& input,
    const TransactionNumber limit,
    TransactionNumbers& good,
    TransactionNumbers& bad)
{
    for (const auto& it : input) {
        if (it <= limit) {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": New transaction number is"
                " less-than-or-equal-to last known 'highest trans number'"
                " record. (Must be seeing the same server reply for a "
                "second time, due to a receipt in my Nymbox.) FYI, last "
                "known 'highest' number received: ")(limit)(
                " (Current 'violator': ")(it)(") Skipping...")
                .Flush();
            ;
            bad.insert(it);
        } else {
            good.insert(it);
        }
    }
}

void ServerContext::verify_blank(
    const Lock& lock,
    OTTransaction& blank,
    TransactionNumbers& output) const
{
    for (auto& number : extract_numbers(blank)) {
        if (verify_issued_number(lock, number)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Attempted to accept a blank transaction number that I "
                "ALREADY HAD...(Skipping).")
                .Flush();
        } else if (verify_tentative_number(lock, number)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Attempted to accept a blank transaction number that I "
                "ALREADY ACCEPTED (it's on my tentative list already; "
                "Skipping).")
                .Flush();
        } else if (number <= highest_transaction_number_.load()) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Attempted to accept a blank transaction number that I've "
                "HAD BEFORE, or at least, is <= to ones I've had before. "
                "(Skipping...).")
                .Flush();
        } else {
            LogTrace(OT_METHOD)(__FUNCTION__)(": Accepting number ")(number)
                .Flush();
            output.insert(number);
        }
    }
}

void ServerContext::verify_success(
    const Lock& lock,
    OTTransaction& blank,
    TransactionNumbers& output) const
{
    for (const auto& number : extract_numbers(blank)) {
        if (false == verify_tentative_number(lock, number)) {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": transactionType::successNotice: This wasn't on my tentative "
                "list (")(number)(
                "), I must have already processed it. (Or there was a dropped "
                "message when I did, or the server is trying to slip me an old "
                "number).")
                .Flush();
        } else {
            output.insert(number);
        }
    }
}

RequestNumber ServerContext::UpdateRequestNumber()
{
    bool notUsed{false};

    return UpdateRequestNumber(notUsed);
}

RequestNumber ServerContext::UpdateRequestNumber(bool& sendStatus)
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);

    return update_request_number(contextLock, messageLock, sendStatus);
}

// This is called by VerifyTransactionReceipt and VerifyBalanceReceipt.
//
// It's okay if some issued transaction #s in statement aren't found on the
// context, since the last balance agreement may have cleaned them out.
//
// But I should never see transaction numbers APPEAR in the context that aren't
// on the statement, since a balance agreement can ONLY remove numbers, not add
// them. So any numbers left over should still be accounted for on the last
// signed receipt.
//
// Conclusion: Loop through *this, which is newer, and make sure ALL numbers
// appear on the statement. No need to check the reverse, and no need to match
// the count.
bool ServerContext::Verify(const TransactionStatement& statement) const
{
    Lock lock(lock_);

    for (const auto& number : issued_transaction_numbers_) {
        const bool missing = (1 != statement.Issued().count(number));

        if (missing) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Issued transaction # ")(
                number)(" on context not found on statement.")
                .Flush();

            return false;
        }
    }

    // Getting here means that, though issued numbers may have been removed from
    // my responsibility in a subsequent balance agreement (since the
    // transaction agreement was signed), I know for a fact that no numbers have
    // been ADDED to my list of responsibility. That's the most we can verify
    // here, since we don't know the account number that was used for the last
    // balance agreement.

    return true;
}

bool ServerContext::verify_tentative_number(
    const Lock& lock,
    const TransactionNumber& number) const
{
    OT_ASSERT(verify_write_lock(lock));

    return (0 < tentative_transaction_numbers_.count(number));
}

bool ServerContext::VerifyTentativeNumber(const TransactionNumber& number) const
{
    return (0 < tentative_transaction_numbers_.count(number));
}

ServerContext::~ServerContext()
{
    Stop().get();
    const bool needPromise = (false == pending_result_set_.load()) &&
                             (proto::DELIVERTYSTATE_IDLE != state_.load());

    if (needPromise) {
        Lock lock(lock_);
        resolve_queue(
            lock,
            DeliveryResult{proto::LASTREPLYSTATUS_UNKNOWN, nullptr},
            proto::DELIVERTYSTATE_IDLE);
    }
}
}  // namespace opentxs::implementation
