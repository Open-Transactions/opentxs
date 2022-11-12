// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "otx/consensus/Server.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <ConsensusEnums.pb.h>
#include <Context.pb.h>
#include <Nym.pb.h>
#include <OTXEnums.pb.h>
#include <OTXPush.pb.h>
#include <PaymentWorkflow.pb.h>
#include <PendingCommand.pb.h>
#include <Purse.pb.h>
#include <ServerContext.pb.h>
#include <ServerContract.pb.h>
#include <UnitDefinition.pb.h>
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <iterator>
#include <stdexcept>
#include <string_view>

#include "core/StateMachine.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/api/session/Activity.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/api/session/Types.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/PeerObject.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/network/ServerConnection.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/otx/OTX.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/blind/Mint.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/NymFile.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "internal/otx/common/basket/Basket.hpp"
#include "internal/otx/common/basket/BasketItem.hpp"
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/trade/OTOffer.hpp"
#include "internal/otx/common/trade/OTTrade.hpp"
#include "internal/otx/common/transaction/Helpers.hpp"
#include "internal/otx/consensus/ManagedNumber.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/otx/consensus/TransactionStatement.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/Context.hpp"
#include "internal/serialization/protobuf/verify/Purse.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/contract/peer/PeerObjectType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/otx/ConsensusType.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "otx/common/OTStorage.hpp"
#include "otx/consensus/Base.hpp"

#define START_SERVER_CONTEXT()                                                 \
    Lock lock(decision_lock_);                                                 \
                                                                               \
    if (running().load()) {                                                    \
        LogDebug()(OT_PRETTY_CLASS())("State machine is already running.")     \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

namespace opentxs
{
constexpr auto FIRST_REQUEST_NUMBER = 1;
}  // namespace opentxs

namespace opentxs::factory
{
auto ServerContext(
    const api::session::Client& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server,
    network::ServerConnection& connection) -> otx::context::internal::Server*
{
    using ReturnType = opentxs::otx::context::implementation::Server;

    return new ReturnType(
        api, requestSent, replyReceived, local, remote, server, connection);
}

auto ServerContext(
    const api::session::Client& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    network::ServerConnection& connection) -> otx::context::internal::Server*
{
    using ReturnType = opentxs::otx::context::implementation::Server;

    return new ReturnType(
        api, requestSent, replyReceived, serialized, local, remote, connection);
}
}  // namespace opentxs::factory

namespace opentxs::otx::context::implementation
{
const UnallocatedSet<MessageType> Server::do_not_need_request_number_{
    MessageType::pingNotary,
    MessageType::registerNym,
    MessageType::getRequestNumber,
};

Server::Server(
    const api::session::Client& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server,
    network::ServerConnection& connection)
    : Base(api, current_version_, local, remote, server)
    , StateMachine([this] { return state_machine(); })
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
    , last_status_(otx::LastReplyStatus::None)
    , pending_message_()
    , pending_args_("", false)
    , pending_result_()
    , pending_result_set_(false)
    , process_nymbox_(false)
    , enable_otx_push_(true)
    , failure_counter_(0)
    , inbox_()
    , outbox_()
    , numbers_(nullptr)
    , find_nym_(api.Network().ZeroMQ().Internal().PushSocket(
          zmq::socket::Direction::Connect))
    , find_server_(api.Network().ZeroMQ().Internal().PushSocket(
          zmq::socket::Direction::Connect))
    , find_unit_definition_(api.Network().ZeroMQ().Internal().PushSocket(
          zmq::socket::Direction::Connect))

{
    {
        Lock lock(lock_);
        first_time_init(lock);
    }

    init_sockets();
}

Server::Server(
    const api::session::Client& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const proto::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    network::ServerConnection& connection)
    : Base(
          api,
          current_version_,
          serialized,
          local,
          remote,
          api.Factory().NotaryIDFromBase58(
              serialized.servercontext().serverid()))
    , StateMachine([this] { return state_machine(); })
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
    , last_status_(translate(serialized.servercontext().laststatus()))
    , pending_message_(instantiate_message(
          api,
          serialized.servercontext().pending().serialized()))
    , pending_args_(
          serialized.servercontext().pending().accountlabel(),
          serialized.servercontext().pending().resync())
    , pending_result_()
    , pending_result_set_(false)
    , process_nymbox_(false)
    , enable_otx_push_(true)
    , failure_counter_(0)
    , inbox_()
    , outbox_()
    , numbers_(nullptr)
    , find_nym_(api.Network().ZeroMQ().Internal().PushSocket(
          zmq::socket::Direction::Connect))
    , find_server_(api.Network().ZeroMQ().Internal().PushSocket(
          zmq::socket::Direction::Connect))
    , find_unit_definition_(api.Network().ZeroMQ().Internal().PushSocket(
          zmq::socket::Direction::Connect))
{
    for (const auto& it : serialized.servercontext().tentativerequestnumber()) {
        tentative_transaction_numbers_.insert(it);
    }

    if (3 > serialized.version()) {
        state_.store(proto::DELIVERTYSTATE_IDLE);
        last_status_.store(otx::LastReplyStatus::None);
    }

    {
        Lock lock(lock_);
        init_serialized(lock);
    }

    init_sockets();
}

auto Server::accept_entire_nymbox(
    const Lock& lock,
    const api::session::Client& client,
    Ledger& nymbox,
    Message& output,
    ReplyNoticeOutcomes& notices,
    std::size_t& alreadySeenNotices,
    const PasswordPrompt& reason) -> bool
{
    alreadySeenNotices = 0;
    const auto& nym = *nym_;
    const auto& nymID = nym.ID();

    if (nymbox.GetTransactionCount() < 1) {
        LogTrace()(OT_PRETTY_CLASS())("Nymbox is empty.").Flush();

        return false;
    }

    if (false == nymbox.VerifyAccount(nym)) {
        LogError()(OT_PRETTY_CLASS())("Invalid nymbox").Flush();

        return false;
    }

    if (nymbox.GetNymID() != nymID) {
        LogError()(OT_PRETTY_CLASS())("Wrong nymbox").Flush();

        return false;
    }

    TransactionNumber lStoredTransactionNumber{0};
    auto processLedger =
        api_.Factory().InternalSession().Ledger(nymID, nymID, server_id_);

    OT_ASSERT(processLedger);

    processLedger->GenerateLedger(nymID, server_id_, ledgerType::message);
    std::shared_ptr<OTTransaction> acceptTransaction{
        api_.Factory().InternalSession().Transaction(
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
            LogError()(OT_PRETTY_CLASS())(
                "Error: Unexpected abbreviated receipt in Nymbox, even after "
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
                    reason,
                    itemType::acceptMessage,
                    transaction,
                    *acceptTransaction);
            } break;
            case transactionType::instrumentNotice: {
                make_accept_item(
                    reason,
                    itemType::acceptNotice,
                    transaction,
                    *acceptTransaction);
            } break;
            case transactionType::notice: {
                make_accept_item(
                    reason,
                    itemType::acceptNotice,
                    transaction,
                    *acceptTransaction);
            } break;
            case transactionType::successNotice: {
                verify_success(lock, transaction, setNoticeNumbers);
                make_accept_item(
                    reason,
                    itemType::acceptNotice,
                    transaction,
                    *acceptTransaction);
            } break;
            case transactionType::replyNotice: {
                const bool seen = verify_acknowledged_number(
                    lock, transaction.GetRequestNum());

                if (seen) {
                    ++alreadySeenNotices;
                } else {
                    auto item = transaction.GetItem(itemType::replyNotice);

                    if (item) {
                        process_unseen_reply(
                            lock, client, *item, notices, reason);
                    } else {
                        LogConsole()(OT_PRETTY_CLASS())(
                            "Missing reply notice item")
                            .Flush();
                    }

                    make_accept_item(
                        reason,
                        itemType::acceptNotice,
                        transaction,
                        *acceptTransaction);
                }
            } break;
            case transactionType::blank: {
                verify_blank(lock, transaction, verifiedNumbers);
                make_accept_item(
                    reason,
                    itemType::acceptTransaction,
                    transaction,
                    *acceptTransaction,
                    verifiedNumbers);
            } break;
            case transactionType::finalReceipt: {
                const auto number = transaction.GetReferenceToNum();
                const bool removed = consume_issued(lock, number);

                if (removed) {
                    LogDetail()(OT_PRETTY_CLASS())(
                        "**** Due to finding a finalReceipt, consuming "
                        "Nym's issued opening number: ")(number)
                        .Flush();
                } else {
                    LogDetail()(OT_PRETTY_CLASS())(
                        "**** Noticed a finalReceipt, but Opening "
                        "Number ")(number)(" had ALREADY been consumed from "
                                           "nym.")
                        .Flush();
                }

                OTCronItem::EraseActiveCronReceipt(
                    api_,
                    api_.DataFolder().string(),
                    number,
                    nymID,
                    server_id_);
                make_accept_item(
                    reason,
                    itemType::acceptFinalReceipt,
                    transaction,
                    *acceptTransaction);
            } break;
            default: {
                LogError()(OT_PRETTY_CLASS())("Not accepting item ")(
                    transaction.GetTransactionNum())(
                    ", "
                    "type"
                    " ")(transaction.GetTypeString())
                    .Flush();
            }
        }
    }

    const auto acceptedItems = acceptTransaction->GetItemCount();

    if ((1 > acceptedItems) && (1 > alreadySeenNotices)) {
        LogDetail()(OT_PRETTY_CLASS())("Nothing to accept.").Flush();

        return false;
    }

    for (const auto& number : setNoticeNumbers) {
        accept_issued_number(lock, number);
    }

    bool ready{true};
    nymbox.ReleaseTransactions();

    for (const auto& number : verifiedNumbers) {
        add_tentative_number(lock, number);
    }

    std::shared_ptr<Item> balanceItem{
        statement(lock, *acceptTransaction, verifiedNumbers, reason)};

    OT_ASSERT(balanceItem);

    acceptTransaction->AddItem(balanceItem);

    OT_ASSERT((acceptedItems + 1) == acceptTransaction->GetItemCount());

    ready &= acceptTransaction->SignContract(nym, reason);

    OT_ASSERT(ready);

    ready &= acceptTransaction->SaveContract();

    OT_ASSERT(ready);

    ready &= processLedger->SignContract(nym, reason);

    OT_ASSERT(ready);

    ready &= processLedger->SaveContract();

    OT_ASSERT(ready);

    const auto serialized = String::Factory(*processLedger);
    initialize_server_command(
        lock, MessageType::processNymbox, -1, true, true, output);
    ready &= output.payload_->SetString(serialized);
    finalize_server_command(output, reason);

    OT_ASSERT(ready);

    return true;
}

void Server::accept_numbers(
    const Lock& lock,
    OTTransaction& transaction,
    OTTransaction& replyTransaction)
{
    auto item = transaction.GetItem(itemType::transactionStatement);

    if (false == bool(item)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Strange... found transaction in ledger , but didn't find a "
            "transactionStatement item within.")
            .Flush();

        return;
    }

    if (false == replyTransaction.GetSuccess()) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Found the receipt you're talking about, but the Server's Reply "
            "transaction says ")("FAILED.")
            .Flush();

        return;
    }

    auto serialized = String::Factory();
    item->GetAttachment(serialized);

    if (false == serialized->Exists()) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Strange... found transaction item in ledger, but didn't find "
            "statement within.")
            .Flush();

        return;
    }

    otx::context::TransactionStatement statement(serialized);
    accept_issued_number(lock, statement);
}

auto Server::accept_issued_number(
    const Lock& lock,
    const TransactionNumber& number) -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    bool accepted = false;
    const bool tentative = remove_tentative_number(lock, number);

    if (tentative) { accepted = issue_number(lock, number); }

    return accepted;
}

auto Server::AcceptIssuedNumber(const TransactionNumber& number) -> bool
{
    Lock lock(lock_);

    return accept_issued_number(lock, number);
}

auto Server::accept_issued_number(
    const Lock& lock,
    const otx::context::TransactionStatement& statement) -> bool
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

auto Server::AcceptIssuedNumbers(
    const otx::context::TransactionStatement& statement) -> bool
{
    Lock lock(lock_);

    return accept_issued_number(lock, statement);
}

auto Server::Accounts() const -> UnallocatedVector<identifier::Generic>
{
    UnallocatedVector<identifier::Generic> output{};
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

auto Server::add_item_to_payment_inbox(
    const TransactionNumber number,
    const UnallocatedCString& payment,
    const PasswordPrompt& reason) const -> bool
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    auto paymentInbox = load_or_create_payment_inbox(reason);

    if (false == bool(paymentInbox)) { return false; }

    // I create the client-side-created instrumentNotice using the same
    // transaction number that was already on the box receipt where it came
    // from. Meaning the server already placed an "transactionType::message"
    // in my Nymbox with Txn # X, so I will create the corresponding
    // instrumentNotice for my Payments Inbox using Txn # X as well.
    // After all, if the notary had created it (as normally happens) then
    // that's the Txn# that would have been on it anyway.
    std::shared_ptr<OTTransaction> transaction{
        api_.Factory().InternalSession().Transaction(
            *paymentInbox,
            transactionType::instrumentNotice,
            originType::not_applicable,
            number)};

    OT_ASSERT(transaction);

    transaction->SetReferenceToNum(number);
    transaction->SetReferenceString(String::Factory(payment));
    transaction->SignContract(nym, reason);
    transaction->SaveContract();
    add_transaction_to_ledger(number, transaction, *paymentInbox, reason);

    return true;
}

auto Server::add_item_to_workflow(
    const Lock& lock,
    const api::session::Client& client,
    const Message& transportItem,
    const UnallocatedCString& item,
    const PasswordPrompt& reason) const -> bool
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    auto message = api_.Factory().InternalSession().Message();

    OT_ASSERT(message);

    const auto loaded =
        message->LoadContractFromString(String::Factory(item.c_str()));

    if (false == loaded) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate message.").Flush();

        return false;
    }

    auto plaintext = String::Factory();

    try {
        auto envelope =
            api_.Factory().InternalSession().Envelope(message->payload_);
        const auto decrypted =
            envelope->Open(nym, plaintext->WriteInto(), reason);

        if (false == decrypted) {
            LogError()(OT_PRETTY_CLASS())("Failed to decrypt message.").Flush();

            return false;
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to decode message.").Flush();

        return false;
    }

    auto payment = api_.Factory().InternalSession().Payment(plaintext);

    if (false == bool(payment)) {
        LogError()(OT_PRETTY_CLASS())("Invalid payment").Flush();

        return false;
    }

    if (false == payment->IsCheque()) { return false; }

    if (payment->IsCancelledCheque(reason)) { return false; }

    auto pCheque = api_.Factory().InternalSession().Cheque();

    OT_ASSERT(pCheque);

    auto& cheque = *pCheque;
    cheque.LoadContractFromString(payment->Payment());

    // The sender nym and notary of the cheque may not match the sender nym and
    // notary of the message which conveyed the cheque.
    {
        find_nym_->Send([&] {
            auto work = network::zeromq::tagged_message(WorkType::OTXSearchNym);
            work.AddFrame(cheque.GetSenderNymID());

            return work;
        }());
    }
    {
        find_server_->Send([&] {
            auto work =
                network::zeromq::tagged_message(WorkType::OTXSearchServer);
            work.AddFrame(cheque.GetNotaryID());

            return work;
        }());
    }
    {
        find_unit_definition_->Send([&] {
            auto work =
                network::zeromq::tagged_message(WorkType::OTXSearchUnit);
            work.AddFrame(cheque.GetInstrumentDefinitionID());

            return work;
        }());
    }

    // We already made sure a contact exists for the sender of the message, but
    // it's possible the sender of the cheque is a different nym
    client.Contacts().NymToContact(cheque.GetSenderNymID());
    const auto workflow =
        client.Workflow().ReceiveCheque(nym.ID(), cheque, transportItem);

    if (workflow.empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to create workflow.").Flush();

        return false;
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("Started workflow ")(workflow).Flush();
    }

    return true;
}

auto Server::add_tentative_number(
    const Lock& lock,
    const TransactionNumber& number) -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    if (number < highest_transaction_number_.load()) { return false; }

    auto output = tentative_transaction_numbers_.insert(number);

    return output.second;
}

auto Server::add_transaction_to_ledger(
    const TransactionNumber number,
    std::shared_ptr<OTTransaction> transaction,
    Ledger& ledger,
    const PasswordPrompt& reason) const -> bool
{
    OT_ASSERT(nym_);

    if (nullptr != ledger.GetTransaction(number)) {
        LogTrace()(OT_PRETTY_CLASS())("Transaction already exists").Flush();

        return true;
    }

    if (false == ledger.AddTransaction(transaction)) {
        LogError()(OT_PRETTY_CLASS())("Failed to add transaction to ledger")
            .Flush();

        return false;
    }

    ledger.ReleaseSignatures();
    ledger.SignContract(*nym_, reason);
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
            LogError()(OT_PRETTY_CLASS())("Unexpected ledger type: ")(
                ledger.GetTypeString())
                .Flush();

            return false;
        }
    }

    if (false == saved) {
        LogError()(OT_PRETTY_CLASS())("Failed to save ")(ledger.GetTypeString())
            .Flush();

        return false;
    }

    if (false == transaction->SaveBoxReceipt(ledger)) {
        LogError()(OT_PRETTY_CLASS())("Failed to save box receipt").Flush();

        return false;
    }

    return true;
}

auto Server::AddTentativeNumber(const TransactionNumber& number) -> bool
{
    Lock lock(lock_);

    return add_tentative_number(lock, number);
}

auto Server::AdminAttempted() const -> bool { return admin_attempted_.get(); }

auto Server::AdminPassword() const -> const UnallocatedCString&
{
    Lock lock(lock_);

    return admin_password_;
}

auto Server::attempt_delivery(
    const Lock& contextLock,
    const Lock& messageLock,
    const api::session::Client& client,
    Message& message,
    const PasswordPrompt& reason) -> client::NetworkReplyMessage
{
    request_sent_.Send([&] {
        auto out = network::zeromq::Message{};
        out.AddFrame(message.command_->Get());

        return out;
    }());
    auto output = connection_.Send(
        *this,
        message,
        reason,
        static_cast<opentxs::network::ServerConnection::Push>(
            enable_otx_push_.load()));
    auto& [status, reply] = output;
    const auto needRequestNumber =
        need_request_number(Message::Type(message.command_->Get()));

    switch (status) {
        case client::SendResult::VALID_REPLY: {
            OT_ASSERT(reply);

            reply_received_.Send([&] {
                auto out = network::zeromq::Message{};
                out.AddFrame(message.command_->Get());

                return out;
            }());
            static UnallocatedSet<otx::context::ManagedNumber> empty{};
            UnallocatedSet<otx::context::ManagedNumber>* numbers = numbers_;

            if (nullptr == numbers) { numbers = &empty; }

            OT_ASSERT(nullptr != numbers);

            process_reply(contextLock, client, *numbers, *reply, reason);

            if (reply->success_) {
                LogVerbose()(OT_PRETTY_CLASS())("Success delivering ")(
                    message.command_.get())
                    .Flush();

                return output;
            }

            if (false == needRequestNumber) { break; }

            bool sent{false};
            auto number =
                update_request_number(reason, contextLock, messageLock, sent);

            if ((0 == number) || (false == sent)) {
                LogError()(OT_PRETTY_CLASS())("Unable to resync request number")
                    .Flush();
                status = client::SendResult::TIMEOUT;
                reply.reset();

                return output;
            } else {
                LogVerbose()(OT_PRETTY_CLASS())(
                    "Success resyncing request number ")(message.command_.get())
                    .Flush();
            }

            const auto updated =
                update_request_number(reason, contextLock, message);

            if (false == updated) {
                LogError()(OT_PRETTY_CLASS())("Unable to update ")(
                    message.command_.get())(" with new request number")
                    .Flush();
                status = client::SendResult::TIMEOUT;
                reply.reset();
                ++failure_counter_;

                return output;
            } else {
                LogVerbose()(OT_PRETTY_CLASS())(
                    "Success updating request number on ")(
                    message.command_.get())
                    .Flush();
            }

            output = connection_.Send(
                *this,
                message,
                reason,
                static_cast<opentxs::network::ServerConnection::Push>(
                    enable_otx_push_.load()));

            if (client::SendResult::VALID_REPLY == status) {
                LogVerbose()(OT_PRETTY_CLASS())("Success delivering ")(
                    message.command_.get())(" (second attempt)")
                    .Flush();
                process_reply(contextLock, client, {}, *reply, reason);

                return output;
            }
        } break;
        case client::SendResult::TIMEOUT: {
            LogError()(OT_PRETTY_CLASS())("Timeout delivering ")(
                message.command_.get())
                .Flush();
            ++failure_counter_;
        } break;
        case client::SendResult::INVALID_REPLY: {
            LogError()(OT_PRETTY_CLASS())("Invalid reply to ")(
                message.command_.get())
                .Flush();
            ++failure_counter_;
        } break;
        case client::SendResult::Error: {
            LogError()(OT_PRETTY_CLASS())("Malformed ")(message.command_.get())
                .Flush();
            ++failure_counter_;
        } break;
        case client::SendResult::SHUTDOWN:
        case client::SendResult::TRANSACTION_NUMBERS:
        case client::SendResult::UNNECESSARY:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unknown error").Flush();
            ++failure_counter_;
        }
    }

    ++failure_counter_;

    return output;
}

auto Server::client_nym_id(const Lock& lock) const -> const identifier::Nym&
{
    OT_ASSERT(nym_);

    return nym_->ID();
}

auto Server::Connection() -> network::ServerConnection& { return connection_; }

auto Server::create_instrument_notice_from_peer_object(
    const Lock& lock,
    const api::session::Client& client,
    const Message& message,
    const PeerObject& peerObject,
    const TransactionNumber number,
    const PasswordPrompt& reason) const -> bool
{
    OT_ASSERT(contract::peer::PeerObjectType::Payment == peerObject.Type());

    if (false == peerObject.Validate()) {
        LogError()(OT_PRETTY_CLASS())("Invalid peer object.").Flush();

        return false;
    }

    const auto& payment = *peerObject.Payment();

    if (payment.empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Payment as received was apparently empty. Maybe the sender "
            "sent it that way?")
            .Flush();

        return false;
    }

    // Extract the OTPayment so that we know whether to use the new Workflow
    // code or the old payment inbox code
    if (add_item_to_workflow(lock, client, message, payment, reason)) {

        return true;
    } else {

        return add_item_to_payment_inbox(number, payment, reason);
    }
}

auto Server::extract_box_receipt(
    const String& serialized,
    const identity::Nym& signer,
    const identifier::Nym& owner,
    const TransactionNumber target) -> std::shared_ptr<OTTransaction>
{
    if (false == serialized.Exists()) {
        LogError()(OT_PRETTY_CLASS())("Invalid input").Flush();

        return {};
    }

    std::shared_ptr<OTTransactionType> transaction{
        api_.Factory().InternalSession().Transaction(serialized)};

    if (false == bool(transaction)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate transaction")
            .Flush();

        return {};
    }

    auto receipt = std::dynamic_pointer_cast<OTTransaction>(transaction);

    if (false == bool(receipt)) {
        LogError()(OT_PRETTY_CLASS())("Invalid transaction type").Flush();

        return {};
    }

    if (false == receipt->VerifyAccount(signer)) {
        LogError()(OT_PRETTY_CLASS())("Invalid receipt").Flush();

        return {};
    }

    if ((0 != target) && (receipt->GetTransactionNum() != target)) {
        LogError()(OT_PRETTY_CLASS())("Incorrect transaction number").Flush();

        return {};
    }

    if (receipt->GetNymID() != owner) {
        LogError()(OT_PRETTY_CLASS())("Invalid nym").Flush();

        return {};
    }

    return receipt;
}

auto Server::extract_ledger(
    const Armored& armored,
    const identifier::Generic& accountID,
    const identity::Nym& signer) const -> std::unique_ptr<Ledger>
{
    OT_ASSERT(nym_);

    if (false == armored.Exists()) {
        LogConsole()(OT_PRETTY_CLASS())("Error: empty input").Flush();

        return {};
    }

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto output =
        api_.Factory().InternalSession().Ledger(nymID, accountID, server_id_);

    OT_ASSERT(output);

    auto serialized = String::Factory();
    armored.GetString(serialized);

    if (false == output->LoadLedgerFromString(serialized)) {
        LogConsole()(OT_PRETTY_CLASS())("Error: Failed to instantiate ledger")
            .Flush();

        return {};
    }

    if (false == output->VerifySignature(signer)) {
        LogConsole()(OT_PRETTY_CLASS())("Error: Invalid signature").Flush();

        return {};
    }

    return output;
}

auto Server::extract_message(
    const Armored& armored,
    const identity::Nym& signer) const -> std::unique_ptr<Message>
{
    auto output = api_.Factory().InternalSession().Message();

    OT_ASSERT(output);

    if (false == armored.Exists()) {
        LogConsole()(OT_PRETTY_CLASS())("Error: empty input").Flush();

        return {};
    }

    auto serialized = String::Factory();
    armored.GetString(serialized);

    if (false == output->LoadContractFromString(serialized)) {
        LogConsole()(OT_PRETTY_CLASS())("Error: Failed to instantiate message")
            .Flush();

        return {};
    }

    if (false == output->VerifySignature(signer)) {
        LogConsole()(OT_PRETTY_CLASS())("Error: Invalid signature").Flush();

        return {};
    }

    return output;
}

auto Server::extract_numbers(OTTransaction& input) -> Server::TransactionNumbers
{
    TransactionNumbers output{};
    NumList list{};
    input.GetNumList(list);
    list.Output(output);

    return output;
}

auto Server::extract_original_item(const Item& response) const
    -> std::unique_ptr<Item>
{
    auto serialized = String::Factory();
    response.GetReferenceString(serialized);

    if (serialized->empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Input item does not contain reference string")
            .Flush();

        return {};
    }

    auto transaction = api_.Factory().InternalSession().Transaction(serialized);

    if (false == bool(transaction)) {
        LogError()(OT_PRETTY_CLASS())("Unable to instantiate serialized item")
            .Flush();

        return {};
    }

    std::unique_ptr<Item> output{dynamic_cast<Item*>(transaction.get())};

    if (output) {
        transaction.release();
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Reference string is not a serialized item")
            .Flush();
    }

    return output;
}

auto Server::extract_payment_instrument_from_notice(
    const api::Session& api,
    const identity::Nym& theNym,
    std::shared_ptr<OTTransaction> pTransaction,
    const PasswordPrompt& reason) -> std::shared_ptr<OTPayment>
{
    const bool bValidNotice =
        (transactionType::instrumentNotice == pTransaction->GetType()) ||
        (transactionType::payDividend == pTransaction->GetType()) ||
        (transactionType::notice == pTransaction->GetType());
    OT_NEW_ASSERT_MSG(
        bValidNotice, "Invalid receipt type passed to this function.");
    // ----------------------------------------------------------------
    if ((transactionType::instrumentNotice ==
         pTransaction->GetType()) ||  // It's encrypted.
        (transactionType::payDividend == pTransaction->GetType())) {
        auto strMsg = String::Factory();
        pTransaction->GetReferenceString(strMsg);

        if (!strMsg->Exists()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failure: Expected OTTransaction::instrumentNotice to "
                "contain an 'in reference to' string, but it was empty. "
                "(Returning).")
                .Flush();
            return nullptr;
        }
        // --------------------
        auto pMsg{api.Factory().InternalSession().Message()};
        if (false == bool(pMsg)) {
            LogError()(OT_PRETTY_CLASS())(
                "Null: Assert while allocating memory "
                "for an OTMessage!")
                .Flush();
            OT_FAIL;
        }
        if (!pMsg->LoadContractFromString(strMsg)) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failed trying to load OTMessage from string: ")(strMsg.get())(
                ".")
                .Flush();
            return nullptr;
        }
        // --------------------
        // By this point, the original OTMessage has been loaded from string
        // successfully.
        // Now we need to decrypt the payment on that message (which contains
        // the instrument
        // itself that we need to return.)

        // SENDER:     pMsg->nym_id_
        // RECIPIENT:  pMsg->nym_id2_
        // INSTRUMENT: pMsg->payload_ (in an OTEnvelope)
        //
        try {
            auto theEnvelope =
                api_.Factory().InternalSession().Envelope(pMsg->payload_);
            auto strEnvelopeContents = String::Factory();

            // Decrypt the Envelope.
            if (!theEnvelope->Open(
                    *nym_, strEnvelopeContents->WriteInto(), reason)) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Failed trying to decrypt the financial instrument "
                    "that was supposedly attached as a payload to this "
                    "payment message: ")(strMsg.get())(".")
                    .Flush();
            } else if (!strEnvelopeContents->Exists()) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Failed: after decryption, cleartext is empty. "
                    "From: ")(strMsg.get())(".")
                    .Flush();
            } else {
                // strEnvelopeContents contains a PURSE or CHEQUE
                // (etc) and not specifically a generic "PAYMENT".
                //
                auto pPayment{api.Factory().InternalSession().Payment(
                    strEnvelopeContents)};
                if (false == bool(pPayment) || !pPayment->IsValid()) {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "Failed: after decryption, payment is invalid. "
                        "Contents: ")(strEnvelopeContents.get())(".")
                        .Flush();
                } else  // success.
                {
                    std::shared_ptr<OTPayment> payment{pPayment.release()};
                    return payment;
                }
            }
        } catch (...) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failed trying to set ASCII-armored data for "
                "envelope: ")(strMsg.get())(".")
                .Flush();
        }
    } else if (transactionType::notice == pTransaction->GetType()) {
        auto strNotice = String::Factory(*pTransaction);
        auto pPayment{api.Factory().InternalSession().Payment(strNotice)};

        if (false == bool(pPayment) || !pPayment->IsValid()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failed: the notice is invalid. Contents: ")(strNotice.get())(
                ".")
                .Flush();
        } else  // success.
        {
            std::shared_ptr<OTPayment> payment{pPayment.release()};
            return payment;
        }
    }

    return nullptr;
}

auto Server::extract_transfer(const OTTransaction& receipt) const
    -> std::unique_ptr<Item>
{
    if (transactionType::transferReceipt == receipt.GetType()) {

        return extract_transfer_receipt(receipt);
    } else if (transactionType::pending == receipt.GetType()) {

        return extract_transfer_pending(receipt);
    } else {
        LogError()(OT_PRETTY_CLASS())("Incorrect receipt type: ")(
            receipt.GetTypeString())
            .Flush();

        return nullptr;
    }
}

auto Server::extract_transfer_pending(const OTTransaction& receipt) const
    -> std::unique_ptr<Item>
{
    if (transactionType::pending != receipt.GetType()) {
        LogError()(OT_PRETTY_CLASS())("Incorrect receipt type: ")(
            receipt.GetTypeString())
            .Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    receipt.GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing serialized transfer item")
            .Flush();

        return nullptr;
    }

    auto transfer = api_.Factory().InternalSession().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Unable to instantiate transfer item")
            .Flush();

        return nullptr;
    }

    if (itemType::transfer != transfer->GetType()) {
        LogError()(OT_PRETTY_CLASS())("Invalid transfer item type.").Flush();

        return nullptr;
    }

    return transfer;
}

auto Server::extract_transfer_receipt(const OTTransaction& receipt) const
    -> std::unique_ptr<Item>
{
    auto serializedAcceptPending = String::Factory();
    receipt.GetReferenceString(serializedAcceptPending);

    if (serializedAcceptPending->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing serialized accept pending item")
            .Flush();

        return nullptr;
    }

    const auto acceptPending =
        api_.Factory().InternalSession().Item(serializedAcceptPending);

    if (false == bool(acceptPending)) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to instantiate accept pending item")
            .Flush();

        return nullptr;
    }

    if (itemType::acceptPending != acceptPending->GetType()) {
        LogError()(OT_PRETTY_CLASS())("Invalid accept pending item type.")
            .Flush();

        return nullptr;
    }

    auto serializedPending = String::Factory();
    acceptPending->GetAttachment(serializedPending);

    if (serializedPending->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing serialized pending transaction")
            .Flush();

        return nullptr;
    }

    auto pending = api_.Factory().InternalSession().Transaction(
        receipt.GetNymID(),
        receipt.GetRealAccountID(),
        receipt.GetRealNotaryID());

    if (false == bool(pending)) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to instantiate pending transaction")
            .Flush();

        return nullptr;
    }

    const bool loaded = pending->LoadContractFromString(serializedPending);

    if (false == loaded) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to deserialize pending transaction")
            .Flush();

        return nullptr;
    }

    if (transactionType::pending != pending->GetType()) {
        LogError()(OT_PRETTY_CLASS())("Invalid pending transaction type.")
            .Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    pending->GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing serialized transfer item")
            .Flush();

        return nullptr;
    }

    auto transfer = api_.Factory().InternalSession().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogError()(OT_PRETTY_CLASS())("Unable to instantiate transfer item")
            .Flush();

        return nullptr;
    }

    if (itemType::transfer != transfer->GetType()) {
        LogError()(OT_PRETTY_CLASS())("Invalid transfer item type.").Flush();

        return nullptr;
    }

    return transfer;
}

auto Server::finalize_server_command(
    Message& command,
    const PasswordPrompt& reason) const -> bool
{
    OT_ASSERT(nym_);

    if (false == command.SignContract(*nym_, reason)) {
        LogError()(OT_PRETTY_CLASS())("Failed to sign server message.").Flush();

        return false;
    }

    if (false == command.SaveContract()) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize server message.")
            .Flush();

        return false;
    }

    return true;
}

auto Server::FinalizeServerCommand(
    Message& command,
    const PasswordPrompt& reason) const -> bool
{
    return finalize_server_command(command, reason);
}

auto Server::generate_statement(
    const Lock& lock,
    const TransactionNumbers& adding,
    const TransactionNumbers& without) const
    -> std::unique_ptr<otx::context::TransactionStatement>
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
        LogTrace()(OT_PRETTY_CLASS())("Accepting number ")(number).Flush();
        issued.insert(number);
        available.insert(number);
    }

    std::unique_ptr<otx::context::TransactionStatement> output(
        new otx::context::TransactionStatement(
            String::Factory(server_id_)->Get(), issued, available));

    return output;
}

auto Server::get_instrument(
    const api::Session& api,
    const identity::Nym& theNym,
    Ledger& ledger,
    std::shared_ptr<OTTransaction> pTransaction,
    const PasswordPrompt& reason) -> std::shared_ptr<OTPayment>
{
    OT_ASSERT(false != bool(pTransaction));

    const std::int64_t lTransactionNum = pTransaction->GetTransactionNum();

    // Update: for transactions in ABBREVIATED form, the string is empty,
    // since it has never actually been signed (in fact the whole postd::int32_t
    // with abbreviated transactions in a ledger is that they take up very
    // little room, and have no signature of their own, but exist merely as
    // XML tags on their parent ledger.)
    //
    // THEREFORE I must check to see if this transaction is abbreviated and
    // if so, sign it in order to force the UpdateContents() call, so the
    // programmatic user of this API will be able to load it up.
    //
    if (pTransaction->IsAbbreviated()) {
        ledger.LoadBoxReceipt(static_cast<std::int64_t>(
            lTransactionNum));  // I don't check return val here because I still
                                // want it to send the abbreviated form, if this
                                // fails.
        pTransaction =
            ledger.GetTransaction(static_cast<std::int64_t>(lTransactionNum));

        if (false == bool(pTransaction)) {
            LogError()(OT_PRETTY_CLASS())(
                "Good index but uncovered nullptr "
                "after trying to load full version of abbreviated receipt "
                "with transaction number: ")(lTransactionNum)(".")
                .Flush();
            return nullptr;  // Weird. Clearly I need the full box receipt, if
                             // I'm to get the instrument out of it.
        }
    }
    // ------------------------------------------------------------
    /*
    TO EXTRACT INSTRUMENT FROM PAYMENTS INBOX:
    -- Iterate through the transactions in the payments inbox.
    -- (They should all be "instrumentNotice" transactions.)
    -- Each transaction contains an
       OTMessage in the "in ref to" field, which in turn contains
           an encrypted OTPayment in the payload field, which contains
           the actual financial instrument.
    -- Therefore, this function, based purely on ledger index (as we iterate):
     1. extracts the OTMessage from the Transaction at each index,
        from its "in ref to" field.
     2. then decrypts the payload on that message, producing an OTPayment
    object, 3. ...which contains the actual instrument.
    */

    if ((transactionType::instrumentNotice != pTransaction->GetType()) &&
        (transactionType::payDividend != pTransaction->GetType()) &&
        (transactionType::notice != pTransaction->GetType())) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failure: Expected OTTransaction::instrumentNotice, "
            "::payDividend or ::notice, "
            "but found: OTTransaction::")(pTransaction->GetTypeString())(".")
            .Flush();

        return nullptr;
    }
    // ------------------------------------------------------------
    // By this point, we know the transaction is loaded up, it's
    // not abbreviated, and is one of the accepted receipt types
    // that would contain the sort of instrument we're looking for.
    //
    return extract_payment_instrument_from_notice(
        api, theNym, pTransaction, reason);
}

auto Server::get_instrument_by_receipt_id(
    const api::Session& api,
    const identity::Nym& theNym,
    const TransactionNumber lReceiptId,
    Ledger& ledger,
    const PasswordPrompt& reason) -> std::shared_ptr<OTPayment>
{
    OT_VERIFY_MIN_BOUND(lReceiptId, 1);

    auto pTransaction = ledger.GetTransaction(lReceiptId);
    if (false == bool(pTransaction)) {
        LogError()(OT_PRETTY_CLASS())(
            "Supposedly good receipt ID, but uncovered nullptr "
            "transaction: ")(lReceiptId)(".")
            .Flush();
        return nullptr;  // Weird.
    }
    return get_instrument(api, theNym, ledger, pTransaction, reason);
}

auto Server::get_item_type(OTTransaction& input, itemType& output)
    -> Server::Exit
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

auto Server::get_type(const std::int64_t depth) -> Server::BoxType
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
            LogError()(OT_PRETTY_STATIC(Server))("Unknown box type: ")(depth)
                .Flush();
        }
    }

    return BoxType::Invalid;
}

auto Server::harvest_unused(
    const Lock& lock,
    const api::session::Client& client) -> bool
{
    OT_ASSERT(verify_write_lock(lock));
    OT_ASSERT(nym_);

    bool output{true};
    const auto& nymID = nym_->ID();
    auto available = issued_transaction_numbers_;
    const auto workflows = client.Storage().PaymentWorkflowList(nymID);
    UnallocatedSet<client::PaymentWorkflowState> keepStates{};

    // Loop through workflows to determine which issued numbers should not be
    // harvested
    for (const auto& [id, alias] : workflows) {
        const auto workflowID = api_.Factory().IdentifierFromBase58(id);
        auto proto = proto::PaymentWorkflow{};

        if (false == client.Workflow().LoadWorkflow(nymID, workflowID, proto)) {
            LogError()(OT_PRETTY_CLASS())("Failed to load workflow ")(
                workflowID)(".")
                .Flush();

            continue;
        }

        switch (translate(proto.type())) {
            case client::PaymentWorkflowType::OutgoingCheque:
            case client::PaymentWorkflowType::OutgoingInvoice: {
                keepStates.insert(client::PaymentWorkflowState::Unsent);
                keepStates.insert(client::PaymentWorkflowState::Conveyed);
            } break;
            case client::PaymentWorkflowType::OutgoingTransfer: {
                keepStates.insert(client::PaymentWorkflowState::Initiated);
                keepStates.insert(client::PaymentWorkflowState::Acknowledged);
            } break;
            case client::PaymentWorkflowType::InternalTransfer: {
                keepStates.insert(client::PaymentWorkflowState::Initiated);
                keepStates.insert(client::PaymentWorkflowState::Acknowledged);
                keepStates.insert(client::PaymentWorkflowState::Conveyed);
            } break;
            case client::PaymentWorkflowType::IncomingTransfer:
            case client::PaymentWorkflowType::IncomingCheque:
            case client::PaymentWorkflowType::IncomingInvoice: {
                continue;
            }
            case client::PaymentWorkflowType::Error:
            case client::PaymentWorkflowType::OutgoingCash:
            case client::PaymentWorkflowType::IncomingCash:
            default: {
                LogError()(OT_PRETTY_CLASS())(
                    "Warning: Unhandled workflow type.")
                    .Flush();
                output &= false;
                continue;
            }
        }

        if (0 == keepStates.count(translate(proto.state()))) { continue; }

        // At this point, this workflow contains a transaction whose
        // number(s) must not be added to the available list (recovered).

        switch (translate(proto.type())) {
            case client::PaymentWorkflowType::OutgoingCheque:
            case client::PaymentWorkflowType::OutgoingInvoice: {
                [[maybe_unused]] auto [state, cheque] =
                    api::session::Workflow::InstantiateCheque(api_, proto);

                if (false == bool(cheque)) {
                    LogError()(OT_PRETTY_CLASS())("Failed to load cheque.")
                        .Flush();

                    continue;
                }

                const auto number = cheque->GetTransactionNum();
                available.erase(number);
            } break;
            case client::PaymentWorkflowType::OutgoingTransfer:
            case client::PaymentWorkflowType::InternalTransfer: {
                [[maybe_unused]] auto [state, pTransfer] =
                    api::session::Workflow::InstantiateTransfer(api_, proto);

                if (false == bool(pTransfer)) {
                    LogError()(OT_PRETTY_CLASS())("Failed to load transfer.")
                        .Flush();

                    continue;
                }

                const auto& transfer = *pTransfer;
                const auto number = transfer.GetTransactionNum();
                available.erase(number);
            } break;
            case client::PaymentWorkflowType::Error:
            case client::PaymentWorkflowType::IncomingTransfer:
            case client::PaymentWorkflowType::IncomingCheque:
            case client::PaymentWorkflowType::IncomingInvoice:
            case client::PaymentWorkflowType::OutgoingCash:
            case client::PaymentWorkflowType::IncomingCash:
            default: {
                LogError()(OT_PRETTY_CLASS())(
                    "Warning: unhandled workflow type.")
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
            LogError()(OT_PRETTY_CLASS())("Restoring number ")(number)(".")
                .Flush();
            recover_available_number(lock, number);
        }
    }

    return output;
}

auto Server::HaveAdminPassword() const -> bool
{
    return false == admin_password_.empty();
}

auto Server::HaveSufficientNumbers(const MessageType reason) const -> bool
{
    if (MessageType::processInbox == reason) {
        return 0 < available_transaction_numbers_.size();
    }

    return 1 < available_transaction_numbers_.size();
}

auto Server::Highest() const -> TransactionNumber
{
    return highest_transaction_number_.load();
}

auto Server::init_new_account(
    const identifier::Generic& accountID,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    auto account = api_.Wallet().Internal().mutable_Account(accountID, reason);

    OT_ASSERT(account);

    if (false == account.get().InitBoxes(nym, reason)) {
        LogError()(OT_PRETTY_CLASS())("Error initializing boxes for account ")(
            accountID)(".")
            .Flush();

        return false;
    }

    auto inboxHash = identifier::Generic{};
    auto outboxHash = identifier::Generic{};
    auto haveHash = account.get().GetInboxHash(inboxHash);

    if (false == haveHash) {
        LogError()(OT_PRETTY_CLASS())("Failed to get inbox hash.").Flush();

        return false;
    }

    haveHash = account.get().GetOutboxHash(outboxHash);

    if (false == haveHash) {
        LogError()(OT_PRETTY_CLASS())("Failed to get outbox hash.").Flush();

        return false;
    }

    account.Release();
    auto nymfile = mutable_Nymfile(reason);
    auto hashSet = nymfile.get().SetInboxHash(
        accountID.asBase58(api_.Crypto()), inboxHash);

    if (false == hashSet) {
        LogError()(OT_PRETTY_CLASS())("Failed to set inbox hash on nymfile.")
            .Flush();

        return false;
    }

    hashSet = nymfile.get().SetOutboxHash(
        accountID.asBase58(api_.Crypto()), outboxHash);

    if (false == hashSet) {
        LogError()(OT_PRETTY_CLASS())("Failed to set outbox hash on nymfile.")
            .Flush();

        return false;
    }

    return true;
}

void Server::init_sockets()
{
    auto started = find_nym_->Start(api_.Endpoints().FindNym().data());

    if (false == started) {
        LogError()(OT_PRETTY_CLASS())("Failed to start find nym socket ")
            .Flush();

        OT_FAIL;
    }

    started = find_server_->Start(api_.Endpoints().FindServer().data());

    if (false == started) {
        LogError()(OT_PRETTY_CLASS())("Failed to start find server socket ")
            .Flush();

        OT_FAIL;
    }

    started = find_unit_definition_->Start(
        api_.Endpoints().FindUnitDefinition().data());

    if (false == started) {
        LogError()(OT_PRETTY_CLASS())("Failed to start find unit socket ")
            .Flush();

        OT_FAIL;
    }
}

auto Server::initialize_server_command(const MessageType type) const
    -> std::unique_ptr<Message>
{
    auto output = api_.Factory().InternalSession().Message();

    OT_ASSERT(output);

    initialize_server_command(type, *output);

    return output;
}

void Server::initialize_server_command(const MessageType type, Message& output)
    const
{
    OT_ASSERT(nym_);

    output.ReleaseSignatures();
    output.command_->Set(Message::Command(type).data());
    output.nym_id_ = String::Factory(nym_->ID());
    output.notary_id_ = String::Factory(server_id_);
}

auto Server::initialize_server_command(
    const Lock& lock,
    const MessageType type,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash,
    Message& output) -> RequestNumber
{
    OT_ASSERT(verify_write_lock(lock));

    RequestNumber number{0};
    initialize_server_command(type, output);

    if (-1 == provided) {
        number = request_number_++;
    } else {
        number = provided;
    }

    output.request_num_ = String::Factory(std::to_string(number).c_str());

    if (withAcknowledgments) {
        output.SetAcknowledgments(acknowledged_request_numbers_);
    }

    if (withNymboxHash) {
        local_nymbox_hash_.GetString(api_.Crypto(), output.nymbox_hash_);
    }

    return number;
}

auto Server::initialize_server_command(
    const Lock& lock,
    const MessageType type,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash)
    -> std::pair<RequestNumber, std::unique_ptr<Message>>
{
    OT_ASSERT(verify_write_lock(lock));

    std::pair<RequestNumber, std::unique_ptr<Message>> output{
        0, api_.Factory().InternalSession().Message()};
    auto& [requestNumber, message] = output;

    OT_ASSERT(message);

    requestNumber = initialize_server_command(
        lock, type, provided, withAcknowledgments, withNymboxHash, *message);

    return output;
}

auto Server::InitializeServerCommand(
    const MessageType type,
    const Armored& payload,
    const identifier::Generic& accountID,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash)
    -> std::pair<RequestNumber, std::unique_ptr<Message>>
{
    Lock lock(lock_);
    auto output = initialize_server_command(
        lock, type, provided, withAcknowledgments, withNymboxHash);
    auto& [requestNumber, message] = output;
    const auto& notUsed [[maybe_unused]] = requestNumber;

    message->payload_ = payload;
    message->acct_id_ = String::Factory(accountID);

    return output;
}

auto Server::InitializeServerCommand(
    const MessageType type,
    const identifier::Nym& recipientNymID,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash)
    -> std::pair<RequestNumber, std::unique_ptr<Message>>
{
    Lock lock(lock_);
    auto output = initialize_server_command(
        lock, type, provided, withAcknowledgments, withNymboxHash);
    [[maybe_unused]] auto& [requestNumber, message] = output;
    message->nym_id2_ = String::Factory(recipientNymID);

    return output;
}

auto Server::InitializeServerCommand(
    const MessageType type,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash)
    -> std::pair<RequestNumber, std::unique_ptr<Message>>
{
    Lock lock(lock_);

    return initialize_server_command(
        lock, type, provided, withAcknowledgments, withNymboxHash);
}

auto Server::instantiate_message(
    const api::Session& api,
    const UnallocatedCString& serialized) -> std::unique_ptr<opentxs::Message>
{
    if (serialized.empty()) { return {}; }

    auto output = api.Factory().InternalSession().Message();

    OT_ASSERT(output);

    const auto loaded =
        output->LoadContractFromString(String::Factory(serialized));

    if (false == loaded) { return {}; }

    return output;
}

auto Server::isAdmin() const -> bool { return admin_success_.get(); }

auto Server::is_internal_transfer(const Item& item) const -> bool
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

void Server::Join() const { Wait().get(); }

auto Server::make_accept_item(
    const PasswordPrompt& reason,
    const itemType type,
    const OTTransaction& input,
    OTTransaction& acceptTransaction,
    const TransactionNumbers& accept) -> const Item&
{
    std::shared_ptr<Item> acceptItem{api_.Factory().InternalSession().Item(
        acceptTransaction, type, identifier::Generic{})};

    OT_ASSERT(acceptItem);

    acceptTransaction.AddItem(acceptItem);
    acceptItem->SetReferenceToNum(input.GetTransactionNum());

    if (false == accept.empty()) {
        acceptItem->AddBlankNumbersToItem(NumList{accept});
    }

    acceptItem->SignContract(*nym_, reason);
    acceptItem->SaveContract();

    return *acceptItem;
}

auto Server::load_account_inbox(const identifier::Generic& accountID) const
    -> std::unique_ptr<Ledger>
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto inbox =
        api_.Factory().InternalSession().Ledger(nymID, accountID, server_id_);

    OT_ASSERT(inbox);

    bool output = OTDB::Exists(
        api_,
        api_.DataFolder().string(),
        api_.Internal().Legacy().Inbox(),
        server_id_.asBase58(api_.Crypto()).c_str(),
        accountID.asBase58(api_.Crypto()).c_str(),
        "");

    if (output && inbox->LoadInbox()) {
        output = inbox->VerifyAccount(nym);
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to load inbox").Flush();
    }

    if (output) { return inbox; }

    LogError()(OT_PRETTY_CLASS())("Failed to verify inbox").Flush();

    return {};
}

auto Server::load_or_create_account_recordbox(
    const identifier::Generic& accountID,
    const PasswordPrompt& reason) const -> std::unique_ptr<Ledger>
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto recordBox =
        api_.Factory().InternalSession().Ledger(nymID, accountID, server_id_);

    OT_ASSERT(recordBox);

    bool output = OTDB::Exists(
        api_,
        api_.DataFolder().string(),
        api_.Internal().Legacy().RecordBox(),
        server_id_.asBase58(api_.Crypto()).c_str(),
        accountID.asBase58(api_.Crypto()).c_str(),
        "");

    if (false == output) {
        LogVerbose()(OT_PRETTY_CLASS())("Creating recordbox").Flush();
        output = recordBox->GenerateLedger(
            accountID, server_id_, ledgerType::recordBox, true);
        recordBox->ReleaseSignatures();
        output &= recordBox->SignContract(nym, reason);
        output &= recordBox->SaveContract();
        output &= recordBox->SaveRecordBox();
    }

    if (output && recordBox->LoadRecordBox()) {
        output &= recordBox->VerifyContractID();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to load recordbox").Flush();

        return {};
    }

    if (output && recordBox->VerifySignature(nym)) {
        LogTrace()(OT_PRETTY_CLASS())("Recordbox verified").Flush();

        return recordBox;
    }

    LogError()(OT_PRETTY_CLASS())("Failed to verify recordbox").Flush();

    return {};
}

auto Server::load_or_create_payment_inbox(const PasswordPrompt& reason) const
    -> std::unique_ptr<Ledger>
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto paymentInbox =
        api_.Factory().InternalSession().Ledger(nymID, nymID, server_id_);

    OT_ASSERT(paymentInbox);

    bool output = OTDB::Exists(
        api_,
        api_.DataFolder().string(),
        api_.Internal().Legacy().PaymentInbox(),
        server_id_.asBase58(api_.Crypto()).c_str(),
        nymID.asBase58(api_.Crypto()).c_str(),
        "");

    if (false == output) {
        LogVerbose()(OT_PRETTY_CLASS())("Creating payment inbox").Flush();
        output = paymentInbox->GenerateLedger(
            nymID, server_id_, ledgerType::paymentInbox, true);
        paymentInbox->ReleaseSignatures();
        output &= paymentInbox->SignContract(nym, reason);
        output &= paymentInbox->SaveContract();
        output &= paymentInbox->SavePaymentInbox();
    }

    if (output && paymentInbox->LoadPaymentInbox()) {
        output &= paymentInbox->VerifyContractID();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to load payment inbox").Flush();

        return {};
    }

    if (output && paymentInbox->VerifySignature(nym)) {
        LogTrace()(OT_PRETTY_CLASS())("Payment inbox verified").Flush();

        return paymentInbox;
    }

    LogError()(OT_PRETTY_CLASS())("Failed to verify payment inbox").Flush();

    return {};
}

auto Server::mutable_Purse(
    const identifier::UnitDefinition& id,
    const PasswordPrompt& reason)
    -> Editor<opentxs::otx::blind::Purse, std::shared_mutex>
{
    return api_.Wallet().Internal().mutable_Purse(
        nym_->ID(), server_id_, id, reason);
}

void Server::need_box_items(
    const api::session::Client& client,
    const PasswordPrompt& reason)
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);
    auto nymbox{api_.Factory().InternalSession().Ledger(
        nym_->ID(), nym_->ID(), server_id_, ledgerType::nymbox)};

    OT_ASSERT(nymbox);

    const auto loaded = nymbox->LoadNymbox();

    if (false == loaded) {
        LogConsole()(OT_PRETTY_CLASS())("Unable to load nymbox").Flush();

        return;
    }

    const auto verified = nymbox->VerifyAccount(*nym_);

    if (false == verified) {
        LogConsole()(OT_PRETTY_CLASS())("Unable to verify nymbox").Flush();

        return;
    }

    std::size_t have{0};

    for (const auto& [number, transaction] : nymbox->GetTransactionMap()) {
        if (1 > number) {
            LogConsole()(OT_PRETTY_CLASS())("Invalid index").Flush();

            continue;
        }

        if (false == bool(transaction)) {
            LogConsole()(OT_PRETTY_CLASS())("Invalid transaction").Flush();

            continue;
        }

        const auto exists = VerifyBoxReceiptExists(
            api_,
            api_.DataFolder().string(),
            server_id_,
            nym_->ID(),
            nym_->ID(),
            nymbox_box_type_,
            number);

        if (exists) {
            ++have;

            continue;
        }

        [[maybe_unused]] auto [requestNumber, message] =
            initialize_server_command(
                contextLock, MessageType::getBoxReceipt, -1, false, false);

        OT_ASSERT(message);

        message->acct_id_ = String::Factory(nym_->ID());
        message->depth_ = nymbox_box_type_;
        message->transaction_num_ = number;
        const auto finalized = FinalizeServerCommand(*message, reason);

        OT_ASSERT(finalized);

        auto result = attempt_delivery(
            contextLock, messageLock, client, *message, reason);

        switch (result.first) {
            case client::SendResult::SHUTDOWN: {
                return;
            }
            case client::SendResult::VALID_REPLY: {
                OT_ASSERT(result.second);

                if (result.second->success_) {
                    ++have;

                    continue;
                } else {
                    // Downloading a box receipt shouldn't fail. If it does, the
                    // only reasonable option is to download the nymbox again.
                    update_state(
                        contextLock, proto::DELIVERTYSTATE_NEEDNYMBOX, reason);
                }

                [[fallthrough]];
            }
            case client::SendResult::TIMEOUT:
            case client::SendResult::INVALID_REPLY:
            case client::SendResult::Error:
            case client::SendResult::TRANSACTION_NUMBERS:
            case client::SendResult::UNNECESSARY:
            default: {
                LogError()(OT_PRETTY_CLASS())("Error downloading box item")
                    .Flush();

                return;
            }
        }
    }

    if (nymbox->GetTransactionMap().size() == have) {
        update_state(
            contextLock, proto::DELIVERTYSTATE_NEEDPROCESSNYMBOX, reason);
    }
}

void Server::need_nymbox(
    const api::session::Client& client,
    const PasswordPrompt& reason)
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);
    [[maybe_unused]] auto [number, message] = initialize_server_command(
        contextLock, MessageType::getNymbox, -1, true, true);

    OT_ASSERT(message);

    const auto finalized = FinalizeServerCommand(*message, reason);

    OT_ASSERT(finalized);

    auto result =
        attempt_delivery(contextLock, messageLock, client, *message, reason);

    switch (result.first) {
        case client::SendResult::SHUTDOWN: {
            return;
        }
        case client::SendResult::VALID_REPLY: {
            OT_ASSERT(result.second);
            auto& reply = *result.second;

            if (reply.success_) {
                if (process_nymbox_.load() && false == bool(pending_message_)) {
                    pending_message_ = result.second;
                }

                update_state(
                    contextLock, proto::DELIVERTYSTATE_NEEDBOXITEMS, reason);

                return;
            }

            [[fallthrough]];
        }
        case client::SendResult::TIMEOUT:
        case client::SendResult::INVALID_REPLY:
        case client::SendResult::Error:
        case client::SendResult::TRANSACTION_NUMBERS:
        case client::SendResult::UNNECESSARY:
        default: {
            LogError()(OT_PRETTY_CLASS())("Error downloading nymbox").Flush();

            return;
        }
    }
}

void Server::need_process_nymbox(
    const api::session::Client& client,
    const PasswordPrompt& reason)
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);
    auto nymbox{api_.Factory().InternalSession().Ledger(
        nym_->ID(), nym_->ID(), server_id_, ledgerType::nymbox)};

    OT_ASSERT(nymbox);

    const auto loaded = nymbox->LoadNymbox();

    if (false == loaded) {
        LogConsole()(OT_PRETTY_CLASS())("Unable to load nymbox").Flush();

        return;
    }

    const auto verified = nymbox->VerifyAccount(*nym_);

    if (false == verified) {
        LogConsole()(OT_PRETTY_CLASS())("Unable to verify nymbox").Flush();

        return;
    }

    const auto count = nymbox->GetTransactionCount();

    if (1 > count) {
        LogDetail()(OT_PRETTY_CLASS())(
            "Nymbox is empty (so, skipping processNymbox).")
            .Flush();

        if (process_nymbox_) {
            DeliveryResult result{
                otx::LastReplyStatus::MessageSuccess, pending_message_};
            resolve_queue(
                contextLock,
                std::move(result),
                reason,
                proto::DELIVERTYSTATE_IDLE);
        } else {
            // The server never received the original message.
            update_state(
                contextLock, proto::DELIVERTYSTATE_PENDINGSEND, reason);
        }

        return;
    }

    auto message{api_.Factory().InternalSession().Message()};

    OT_ASSERT(message);

    ReplyNoticeOutcomes outcomes{};
    std::size_t alreadySeen{0};
    const auto accepted = accept_entire_nymbox(
        contextLock, client, *nymbox, *message, outcomes, alreadySeen, reason);

    if (pending_message_) {
        const RequestNumber targetNumber =
            String::StringToUlong(pending_message_->request_num_->Get());

        for (auto& [number, status] : outcomes) {
            if (number == targetNumber) {
                resolve_queue(contextLock, std::move(status), reason);
            }
        }
    }

    if (false == accepted) {
        if (alreadySeen == static_cast<std::size_t>(count)) {
            if (process_nymbox_) {
                DeliveryResult result{
                    otx::LastReplyStatus::MessageSuccess, pending_message_};
                resolve_queue(
                    contextLock,
                    std::move(result),
                    reason,
                    proto::DELIVERTYSTATE_IDLE);
            } else {
                // The server never received the original message.
                update_state(
                    contextLock, proto::DELIVERTYSTATE_PENDINGSEND, reason);
            }
        } else {
            LogError()(OT_PRETTY_CLASS())(
                "Failed trying to accept the "
                "entire Nymbox. (And no, it's not empty).")
                .Flush();
            update_state(contextLock, proto::DELIVERTYSTATE_NEEDNYMBOX, reason);
        }

        return;
    }

    local_nymbox_hash_.GetString(api_.Crypto(), message->nymbox_hash_);

    if (false == finalize_server_command(*message, reason)) {
        LogError()(OT_PRETTY_CLASS())("Failed to finalize server message.")
            .Flush();

        return;
    }

    auto result =
        attempt_delivery(contextLock, messageLock, client, *message, reason);

    switch (result.first) {
        case client::SendResult::SHUTDOWN: {
            return;
        }
        case client::SendResult::VALID_REPLY: {
            auto& pReply = result.second;

            OT_ASSERT(pReply);

            if (pReply->success_) {
                [[maybe_unused]] auto [number, message] =
                    initialize_server_command(
                        contextLock, MessageType::getNymbox, -1, true, true);

                OT_ASSERT(message);

                const auto finalized = FinalizeServerCommand(*message, reason);

                OT_ASSERT(finalized);

                auto again = attempt_delivery(
                    contextLock, messageLock, client, *message, reason);

                if (process_nymbox_) {
                    DeliveryResult resultProcessNymbox{
                        otx::LastReplyStatus::MessageSuccess, pending_message_};
                    resolve_queue(
                        contextLock,
                        std::move(resultProcessNymbox),
                        reason,
                        proto::DELIVERTYSTATE_IDLE);
                } else {
                    // The server never received the original message.
                    update_state(
                        contextLock, proto::DELIVERTYSTATE_PENDINGSEND, reason);
                }

                return;
            }

            [[fallthrough]];
        }
        case client::SendResult::TIMEOUT:
        case client::SendResult::INVALID_REPLY:
        case client::SendResult::Error:
        case client::SendResult::TRANSACTION_NUMBERS:
        case client::SendResult::UNNECESSARY:
        default: {
            // If processing a nymbox fails, then it must have changed since
            // the last time we downloaded it. Also if the reply was dropped
            // we might have actually processed it without realizig it.
            LogError()(OT_PRETTY_CLASS())("Error processing nymbox").Flush();
            update_state(contextLock, proto::DELIVERTYSTATE_NEEDNYMBOX, reason);

            return;
        }
    }
}

auto Server::need_request_number(const MessageType type) -> bool
{
    return 0 == do_not_need_request_number_.count(type);
}

auto Server::next_transaction_number(const Lock& lock, const MessageType reason)
    -> otx::context::ManagedNumber
{
    OT_ASSERT(verify_write_lock(lock));

    const std::size_t reserve = (MessageType::processInbox == reason) ? 0 : 1;

    if (0 == reserve) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Allocating a transaction number for process inbox.")
            .Flush();
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Allocating a transaction number for normal transaction.")
            .Flush();
    }

    LogVerbose()(OT_PRETTY_CLASS())(available_transaction_numbers_.size())(
        " numbers available.")
        .Flush();
    LogVerbose()(OT_PRETTY_CLASS())(issued_transaction_numbers_.size())(
        " numbers issued.")
        .Flush();

    if (reserve >= available_transaction_numbers_.size()) {

        return factory::ManagedNumber(0, *this);
    }

    auto first = available_transaction_numbers_.begin();
    const auto output = *first;
    available_transaction_numbers_.erase(first);

    return factory::ManagedNumber(output, *this);
}

auto Server::NextTransactionNumber(const MessageType reason)
    -> otx::context::ManagedNumber
{
    Lock lock(lock_);

    return next_transaction_number(lock, reason);
}

void Server::pending_send(
    const api::session::Client& client,
    const PasswordPrompt& reason)
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);

    OT_ASSERT(pending_message_);

    auto result = attempt_delivery(
        contextLock, messageLock, client, *pending_message_, reason);
    const auto needRequestNumber =
        need_request_number(Message::Type(pending_message_->command_->Get()));

    switch (result.first) {
        case client::SendResult::SHUTDOWN: {
            return;
        }
        case client::SendResult::VALID_REPLY: {
            OT_ASSERT(result.second);

            DeliveryResult output{};
            auto& [status, message] = output;
            message = result.second;
            const auto& reply = *message;

            if (reply.success_) {
                status = otx::LastReplyStatus::MessageSuccess;
            } else {
                status = otx::LastReplyStatus::MessageFailed;
            }

            resolve_queue(
                contextLock,
                std::move(output),
                reason,
                proto::DELIVERTYSTATE_IDLE);

        } break;
        case client::SendResult::TIMEOUT:
        case client::SendResult::INVALID_REPLY: {
            if (needRequestNumber) {
                update_state(
                    contextLock,
                    proto::DELIVERTYSTATE_NEEDNYMBOX,
                    reason,
                    otx::LastReplyStatus::Unknown);
            }
        } break;
        case client::SendResult::Error:
        case client::SendResult::TRANSACTION_NUMBERS:
        case client::SendResult::UNNECESSARY:
        default: {
        }
    }
}

auto Server::PingNotary(const PasswordPrompt& reason)
    -> client::NetworkReplyMessage
{
    Lock lock(message_lock_);

    OT_ASSERT(nym_);

    auto request = initialize_server_command(MessageType::pingNotary);

    if (false == bool(request)) {
        LogError()(OT_PRETTY_CLASS())("Failed to initialize server message.")
            .Flush();

        return {};
    }

    auto serializedAuthKey = proto::AsymmetricKey{};
    if (false ==
        nym_->GetPublicAuthKey().Internal().Serialize(serializedAuthKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize auth key").Flush();

        return {};
    }

    auto serializedEncryptKey = proto::AsymmetricKey{};
    if (false ==
        nym_->GetPublicEncrKey().Internal().Serialize(serializedEncryptKey)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize encrypt key")
            .Flush();

        return {};
    }

    request->request_num_ =
        String::Factory(std::to_string(FIRST_REQUEST_NUMBER).c_str());
    request->nym_public_key_ = api_.Factory().InternalSession().Armored(
        serializedAuthKey, "ASYMMETRIC KEY");
    request->nym_id2_ = api_.Factory().InternalSession().Armored(
        serializedEncryptKey, "ASYMMETRIC KEY");

    if (false == finalize_server_command(*request, reason)) {
        LogError()(OT_PRETTY_CLASS())("Failed to finalize server message.")
            .Flush();

        return {};
    }

    return connection_.Send(
        *this,
        *request,
        reason,
        static_cast<opentxs::network::ServerConnection::Push>(
            enable_otx_push_.load()));
}

auto Server::ProcessNotification(
    const api::session::Client& client,
    const otx::Reply& notification,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);

    Lock lock(lock_);
    const auto pPush = notification.Push();

    if (false == bool(pPush)) {
        LogError()(OT_PRETTY_CLASS())("Missing push payload").Flush();

        return false;
    }

    const auto& push = *pPush;

    switch (push.type()) {
        case proto::OTXPUSH_NYMBOX: {
            // Nymbox items don't have an intrinsic account ID. Use nym ID
            // instead.
            return process_box_item(lock, client, nym_->ID(), push, reason);
        }
        case proto::OTXPUSH_INBOX: {
            return process_account_push(lock, client, push, reason);
        }
        case proto::OTXPUSH_ERROR:
        case proto::OTXPUSH_OUTBOX:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported push type").Flush();

            return false;
        }
    }
}

void Server::process_accept_basket_receipt_reply(
    const Lock& lock,
    const OTTransaction& inboxTransaction)
{
    const auto number = inboxTransaction.GetClosingNum();
    LogVerbose()(OT_PRETTY_CLASS())(
        "Successfully removed basketReceipt with closing number: ")(number)
        .Flush();
    consume_issued(lock, number);
}

void Server::process_accept_cron_receipt_reply(
    const Lock& lock,
    const identifier::Generic& accountID,
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
    auto theOffer = api_.Factory().InternalSession().Offer();

    OT_ASSERT((theOffer));

    auto theTrade = api_.Factory().InternalSession().Trade();

    OT_ASSERT((theTrade));

    api_.Factory().InternalSession().Trade();
    bool bLoadOfferFromString = theOffer->LoadContractFromString(strOffer);
    bool bLoadTradeFromString = theTrade->LoadContractFromString(strTrade);

    if (bLoadOfferFromString && bLoadTradeFromString) {
        std::unique_ptr<OTDB::TradeDataNym> pData(
            dynamic_cast<OTDB::TradeDataNym*>(
                OTDB::CreateObject(OTDB::STORED_OBJ_TRADE_DATA_NYM)));

        OT_ASSERT((pData));

        const Amount& lScale = theOffer->GetScale();

        // TransID for original offer.
        // (Offer may trade many times.)
        pData->transaction_id_ = std::to_string(theTrade->GetTransactionNum());
        // TransID for BOTH receipts for current trade.
        // (Asset/Currency.)
        pData->updated_id_ = std::to_string(pServerItem->GetTransactionNum());
        pData->completed_count_ = std::to_string(theTrade->GetCompletedCount());
        auto account = api_.Wallet().Internal().Account(accountID);

        OT_ASSERT(account);

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
            pData->instrument_definition_id_ = strInstrumentDefinitionID->Get();
            // The amount of ASSETS moved, this trade.
            pData->amount_sold_ = [&] {
                auto buf = UnallocatedCString{};
                pServerItem->GetAmount().Serialize(writer(buf));
                return buf;
            }();
            pData->asset_acct_id_ = strAcctID->Get();
            pData->asset_receipt_ = strServerTransaction->Get();
        } else if (bIsCurrency) {
            const auto strCurrencyID =
                String::Factory(theTrade->GetCurrencyID());
            pData->currency_id_ = strCurrencyID->Get();
            pData->currency_paid_ = [&] {
                auto buf = UnallocatedCString{};
                pServerItem->GetAmount().Serialize(writer(buf));
                return buf;
            }();
            pData->currency_acct_id_ = strAcctID->Get();
            pData->currency_receipt_ = strServerTransaction->Get();
        }

        const auto tProcessDate = inboxTransaction.GetDateSigned();
        pData->date_ = std::to_string(Clock::to_time_t(tProcessDate));

        // The original offer price. (Might be 0, if it's a market order.)
        pData->offer_price_ = [&] {
            auto buf = UnallocatedCString{};
            theOffer->GetPriceLimit().Serialize(writer(buf));
            return buf;
        }();
        pData->finished_so_far_ = [&] {
            auto buf = UnallocatedCString{};
            theOffer->GetFinishedSoFar().Serialize(writer(buf));
            return buf;
        }();
        pData->scale_ = [&] {
            auto buf = UnallocatedCString{};
            lScale.Serialize(writer(buf));
            return buf;
        }();
        pData->is_bid_ = theOffer->IsBid();

        // save to local storage...
        auto strNymID = String::Factory(nym_->ID());
        std::unique_ptr<OTDB::TradeListNym> pList;

        if (OTDB::Exists(
                api_,
                api_.DataFolder().string(),
                api_.Internal().Legacy().Nym(),
                "trades",  // todo stop
                           // hardcoding.
                server_id_.asBase58(api_.Crypto()).c_str(),
                strNymID->Get())) {
            pList.reset(dynamic_cast<OTDB::TradeListNym*>(OTDB::QueryObject(
                api_,
                OTDB::STORED_OBJ_TRADE_LIST_NYM,
                api_.DataFolder().string(),
                api_.Internal().Legacy().Nym(),
                "trades",  // todo stop
                // hardcoding.
                server_id_.asBase58(api_.Crypto()).c_str(),
                strNymID->Get())));
        }
        if (false == bool(pList)) {
            LogVerbose()(OT_PRETTY_CLASS())("Creating storage list of trade ")(
                "receipts for "
                "Nym: ")(strNymID.get())
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

        std::size_t nTradeDataNymCount = pList->GetTradeDataNymCount();

        for (auto nym_count = 0_uz; nym_count < nTradeDataNymCount;
             ++nym_count) {
            OTDB::TradeDataNym* pTradeData = pList->GetTradeDataNym(nym_count);

            if (nullptr == pTradeData) {
                continue;  // Should never happen.
            }

            if (0 == pTradeData->updated_id_.compare(pData->updated_id_)) {
                // It's a repeat of the same one. (Discard.)
                if ((!pTradeData->instrument_definition_id_.empty() &&
                     !pData->instrument_definition_id_.empty()) ||
                    (!pTradeData->currency_id_.empty() &&
                     !pData->currency_id_.empty())) {
                    break;
                }
                // Okay looks like one is the asset receipt,
                // and the other is the currency receipt.
                // Therefore let's combine them into
                // pTradeData!
                //
                if (pTradeData->instrument_definition_id_.empty()) {
                    pTradeData->instrument_definition_id_ =
                        pData->instrument_definition_id_;
                    pTradeData->amount_sold_ = pData->amount_sold_;
                    pTradeData->asset_acct_id_ = pData->asset_acct_id_;
                    pTradeData->asset_receipt_ = pData->asset_receipt_;
                }
                if (pTradeData->currency_id_.empty()) {
                    pTradeData->currency_id_ = pData->currency_id_;
                    pTradeData->currency_paid_ = pData->currency_paid_;
                    pTradeData->currency_acct_id_ = pData->currency_acct_id_;
                    pTradeData->currency_receipt_ = pData->currency_receipt_;
                }
                if (!pTradeData->amount_sold_.empty() &&
                    !pTradeData->currency_paid_.empty()) {

                    const Amount lAmountSold =
                        factory::Amount(pTradeData->amount_sold_);
                    const Amount lCurrencyPaid =
                        factory::Amount(pTradeData->currency_paid_);

                    // just in case (divide by 0.)
                    if ((lAmountSold != 0) && (lScale != 0)) {
                        const Amount lSalePrice =
                            (lCurrencyPaid / (lAmountSold / lScale));
                        lSalePrice.Serialize(writer(pTradeData->price_));
                    }
                }

                bWeFoundIt = true;

                break;
            }
        }

        if (!bWeFoundIt) { pList->AddTradeDataNym(*pData); }

        if (false == OTDB::StoreObject(
                         api_,
                         *pList,
                         api_.DataFolder().string(),
                         api_.Internal().Legacy().Nym(),
                         "trades",  // todo stop hardcoding.
                         server_id_.asBase58(api_.Crypto()).c_str(),
                         strNymID->Get())) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed storing list of trades for nym. Notary ID: ")(
                server_id_)(". Nym ID: ")(strNymID.get())(".")
                .Flush();
        }
    }
}

void Server::process_accept_final_receipt_reply(
    const Lock& lock,
    const OTTransaction& inboxTransaction)
{
    const auto number = inboxTransaction.GetClosingNum();
    const auto referenceNumber = inboxTransaction.GetReferenceToNum();
    LogDetail()(OT_PRETTY_CLASS())(
        "Successfully removed finalReceipt with closing number: ")(number)
        .Flush();
    consume_issued(lock, number);

    if (consume_issued(lock, referenceNumber)) {
        LogDetail()(OT_PRETTY_CLASS())(
            "**** Due to finding a finalReceipt, consuming issued opening "
            "number from nym: ")(referenceNumber)
            .Flush();
    } else {
        LogDetail()(OT_PRETTY_CLASS())(
            "**** Noticed a finalReceipt, but Opening "
            "Number ")(referenceNumber)(" had ALREADY been removed from nym.")
            .Flush();
    }

    OTCronItem::EraseActiveCronReceipt(
        api_,
        api_.DataFolder().string(),
        inboxTransaction.GetReferenceToNum(),
        nym_->ID(),
        inboxTransaction.GetPurportedNotaryID());
}

void Server::process_accept_item_receipt_reply(
    const Lock& lock,
    const api::session::Client& client,
    const identifier::Generic& accountID,
    const Message& reply,
    const OTTransaction& inboxTransaction)
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    const auto& nymID = nym_->ID();
    auto serializedOriginal = String::Factory();
    inboxTransaction.GetReferenceString(serializedOriginal);
    auto pOriginalItem = api_.Factory().InternalSession().Item(
        serializedOriginal, server_id_, inboxTransaction.GetReferenceToNum());

    if (false == bool(pOriginalItem)) {
        LogError()(OT_PRETTY_CLASS())("Unable to load original item from "
                                      "string while accepting item receipt: ")(
            serializedOriginal.get())(".")
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
            auto cheque = api_.Factory().InternalSession().Cheque();

            OT_ASSERT(cheque);

            if (false == cheque->LoadContractFromString(serialized)) {
                LogError()(OT_PRETTY_CLASS())("Failed to deserialize cheque")
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
                LogError()(OT_PRETTY_CLASS())("Missing attachment").Flush();

                break;
            }

            const auto transferReceipt =
                api_.Factory().InternalSession().Transaction(
                    remote_nym_->ID(), accountID, server_id_);

            OT_ASSERT(transferReceipt);

            const auto loaded =
                transferReceipt->LoadContractFromString(serialized);

            if (false == loaded) {
                LogError()(OT_PRETTY_CLASS())(
                    "Unable to instantiate transfer receipt")
                    .Flush();

                break;
            }

            const auto pTransfer = extract_transfer(*transferReceipt);

            if (pTransfer) {
                const auto& transfer = *pTransfer;

                if (is_internal_transfer(transfer)) {
                    LogDetail()(OT_PRETTY_CLASS())(
                        "Completing internal transfer")
                        .Flush();
                } else {
                    LogDetail()(OT_PRETTY_CLASS())(
                        "Completing outgoing transfer")
                        .Flush();
                }

                client.Workflow().CompleteTransfer(
                    nymID, server_id_, *transferReceipt, reply);
            } else {
                LogError()(OT_PRETTY_CLASS())("Invalid transfer").Flush();
            }
        } break;
        case itemType::transfer:
        case itemType::atTransfer:
        case itemType::acceptTransaction:
        case itemType::atAcceptTransaction:
        case itemType::acceptMessage:
        case itemType::atAcceptMessage:
        case itemType::acceptNotice:
        case itemType::atAcceptNotice:
        case itemType::atAcceptPending:
        case itemType::rejectPending:
        case itemType::atRejectPending:
        case itemType::acceptCronReceipt:
        case itemType::atAcceptCronReceipt:
        case itemType::acceptItemReceipt:
        case itemType::atAcceptItemReceipt:
        case itemType::disputeCronReceipt:
        case itemType::atDisputeCronReceipt:
        case itemType::disputeItemReceipt:
        case itemType::atDisputeItemReceipt:
        case itemType::acceptFinalReceipt:
        case itemType::atAcceptFinalReceipt:
        case itemType::acceptBasketReceipt:
        case itemType::atAcceptBasketReceipt:
        case itemType::disputeFinalReceipt:
        case itemType::atDisputeFinalReceipt:
        case itemType::disputeBasketReceipt:
        case itemType::atDisputeBasketReceipt:
        case itemType::serverfee:
        case itemType::atServerfee:
        case itemType::issuerfee:
        case itemType::atIssuerfee:
        case itemType::balanceStatement:
        case itemType::atBalanceStatement:
        case itemType::transactionStatement:
        case itemType::atTransactionStatement:
        case itemType::withdrawal:
        case itemType::atWithdrawal:
        case itemType::deposit:
        case itemType::atDeposit:
        case itemType::withdrawVoucher:
        case itemType::atWithdrawVoucher:
        case itemType::atDepositCheque:
        case itemType::payDividend:
        case itemType::atPayDividend:
        case itemType::marketOffer:
        case itemType::atMarketOffer:
        case itemType::paymentPlan:
        case itemType::atPaymentPlan:
        case itemType::smartContract:
        case itemType::atSmartContract:
        case itemType::cancelCronItem:
        case itemType::atCancelCronItem:
        case itemType::exchangeBasket:
        case itemType::atExchangeBasket:
        case itemType::chequeReceipt:
        case itemType::voucherReceipt:
        case itemType::marketReceipt:
        case itemType::paymentReceipt:
        case itemType::transferReceipt:
        case itemType::finalReceipt:
        case itemType::basketReceipt:
        case itemType::replyNotice:
        case itemType::successNotice:
        case itemType::notice:
        case itemType::error_state:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unexpected original item type: ")(
                originalType.get())
                .Flush();
        }
    }
}

void Server::process_accept_pending_reply(
    const Lock& lock,
    const api::session::Client& client,
    const identifier::Generic& accountID,
    const Item& acceptItemReceipt,
    const Message& reply) const
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    if (itemType::acceptPending != acceptItemReceipt.GetType()) {
        auto type = String::Factory();
        acceptItemReceipt.GetTypeString(type);
        LogError()(OT_PRETTY_CLASS())("Invalid type: ")(type.get()).Flush();

        return;
    }

    auto attachment = String::Factory();
    acceptItemReceipt.GetAttachment(attachment);

    if (attachment->empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing attachment").Flush();

        return;
    }

    OT_ASSERT(false == attachment->empty());

    const auto pending = api_.Factory().InternalSession().Transaction(
        remote_nym_->ID(), accountID, server_id_);

    OT_ASSERT(pending);

    const auto loaded = pending->LoadContractFromString(attachment);

    if (false == loaded) {
        LogError()(OT_PRETTY_CLASS())("Unable to instantiate").Flush();

        return;
    }

    if (transactionType::pending != pending->GetType()) {
        LogError()(OT_PRETTY_CLASS())("Wrong item type").Flush();

        return;
    }

    const auto pTransfer = extract_transfer(*pending);

    if (pTransfer) {
        const auto& transfer = *pTransfer;

        if (false == is_internal_transfer(transfer)) {
            LogDetail()(OT_PRETTY_CLASS())("Accepting incoming transfer")
                .Flush();
            client.Workflow().AcceptTransfer(
                nym_->ID(), server_id_, *pending, reply);
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Invalid transfer").Flush();
    }
}

auto Server::process_account_data(
    const Lock& lock,
    const identifier::Generic& accountID,
    const String& account,
    const identifier::Generic& inboxHash,
    const String& inbox,
    const identifier::Generic& outboxHash,
    const String& outbox,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    const auto& nymID = nym_->ID();
    const auto updated = api_.Wallet().Internal().UpdateAccount(
        accountID, *this, account, reason);

    if (updated) {
        LogDetail()(OT_PRETTY_CLASS())("Saved updated account file.").Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to save account.").Flush();

        return false;
    }

    if (false == bool(inbox_)) {
        inbox_.reset(
            api_.Factory()
                .InternalSession()
                .Ledger(nymID, accountID, server_id_, ledgerType::inbox)
                .release());
    }

    OT_ASSERT(inbox_);
    OT_ASSERT(ledgerType::inbox == inbox_->GetType());

    if (false == inbox_->LoadInboxFromString(inbox)) {
        LogError()(OT_PRETTY_CLASS())("Failed to deserialize inbox").Flush();

        return false;
    }

    if (false == inbox_->VerifySignature(*remote_nym_)) {
        LogError()(OT_PRETTY_CLASS())("Invalid inbox signature").Flush();

        return false;
    }

    auto nymfile = mutable_Nymfile(reason);

    if (false == inboxHash.empty()) {
        const bool hashSet = nymfile.get().SetInboxHash(
            accountID.asBase58(api_.Crypto()), inboxHash);

        if (false == hashSet) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed setting inbox hash for "
                "account: ")(accountID)(" to (")(inboxHash)(").")
                .Flush();
        }
    }

    for (const auto& [number, pTransaction] : inbox_->GetTransactionMap()) {

        if (false == bool(pTransaction)) {
            LogError()(OT_PRETTY_CLASS())("Invalid transaction ")(
                number)(" in inbox")
                .Flush();

            return false;
        }

        auto& transaction = *pTransaction;

        if (transactionType::finalReceipt == transaction.GetType()) {
            LogVerbose()(OT_PRETTY_CLASS())(
                "*** Removing opening issued number "
                "(")(transaction.GetReferenceToNum())("), since "
                                                      "finalReceipt "
                                                      "found when ")("re"
                                                                     "tr"
                                                                     "ie"
                                                                     "vi"
                                                                     "ng"
                                                                     " a"
                                                                     "ss"
                                                                     "et"
                                                                     " a"
                                                                     "cc"
                                                                     "ou"
                                                                     "nt"
                                                                     " i"
                                                                     "nb"
                                                                     "ox"
                                                                     ". "
                                                                     "**"
                                                                     "*")
                .Flush();

            if (consume_issued(lock, transaction.GetReferenceToNum())) {
                LogDetail()(OT_PRETTY_CLASS())(
                    "**** Due to finding a finalReceipt, "
                    "consuming issued opening number from nym: "
                    " ")(transaction.GetReferenceToNum())
                    .Flush();
            } else {
                LogDetail()(OT_PRETTY_CLASS())(
                    "**** Noticed a finalReceipt, but issued opening "
                    "number ")(transaction.GetReferenceToNum())(
                    " had ALREADY been "
                    "consumed from nym.")
                    .Flush();
            }

            OTCronItem::EraseActiveCronReceipt(
                api_,
                api_.DataFolder().string(),
                transaction.GetReferenceToNum(),
                nym_->ID(),
                transaction.GetPurportedNotaryID());
        }
    }

    inbox_->ReleaseSignatures();
    inbox_->SignContract(*nym_, reason);
    inbox_->SaveContract();
    inbox_->SaveInbox();

    if (false == bool(outbox_)) {
        outbox_.reset(
            api_.Factory()
                .InternalSession()
                .Ledger(nymID, accountID, server_id_, ledgerType::outbox)
                .release());
    }

    OT_ASSERT(outbox_);
    OT_ASSERT(ledgerType::outbox == outbox_->GetType());

    if (false == outbox_->LoadOutboxFromString(outbox)) {
        LogError()(OT_PRETTY_CLASS())("Failed to deserialize outbox").Flush();

        return false;
    }

    if (false == outbox_->VerifySignature(*remote_nym_)) {
        LogError()(OT_PRETTY_CLASS())("Invalid outbox signature").Flush();

        return false;
    }

    if (false == outboxHash.empty()) {
        const bool hashSet = nymfile.get().SetOutboxHash(
            accountID.asBase58(api_.Crypto()), outboxHash);

        if (false == hashSet) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed setting outbox hash for "
                "account: ")(accountID)(" to (")(outboxHash)(") .")
                .Flush();
        }
    }

    outbox_->ReleaseSignatures();
    outbox_->SignContract(*nym_, reason);
    outbox_->SaveContract();
    outbox_->SaveOutbox();

    return true;
}

auto Server::process_account_push(
    const Lock& lock,
    const api::session::Client& client,
    const proto::OTXPush& push,
    const PasswordPrompt& reason) -> bool
{
    const auto accountID =
        api_.Factory().IdentifierFromBase58(push.accountid());
    const auto inboxHash =
        api_.Factory().IdentifierFromBase58(push.inboxhash());
    const auto outboxHash =
        api_.Factory().IdentifierFromBase58(push.outboxhash());
    const auto account = String::Factory(push.account());
    const auto inbox = String::Factory(push.inbox());
    const auto outbox = String::Factory(push.outbox());

    const auto processed = process_account_data(
        lock, accountID, account, inboxHash, inbox, outboxHash, outbox, reason);

    if (processed) {
        LogVerbose()(OT_PRETTY_CLASS())("Success saving new account data.")
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failure saving new account data.")
            .Flush();

        return false;
    }

    return process_box_item(lock, client, accountID, push, reason);
}

auto Server::process_box_item(
    const Lock& lock,
    const api::session::Client& client,
    const identifier::Generic& accountID,
    const proto::OTXPush& push,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    const auto& nym = *nym_;
    const auto& remoteNym = *remote_nym_;
    const auto& nymID = nym.ID();
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
        case proto::OTXPUSH_ERROR:
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid box type").Flush();

            return false;
        }
    }

    const auto& payload = push.item();

    if (payload.empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing payload").Flush();

        return false;
    }

    std::shared_ptr<OTTransactionType> base{
        api_.Factory().InternalSession().Transaction(String::Factory(payload))};

    if (false == bool(base)) {
        LogError()(OT_PRETTY_CLASS())("Invalid payload").Flush();

        return false;
    }

    auto receipt = std::dynamic_pointer_cast<OTTransaction>(base);

    if (false == bool(base)) {
        LogError()(OT_PRETTY_CLASS())("Wrong transaction type").Flush();

        return false;
    }

    OT_ASSERT(receipt);

    if (receipt->GetNymID() != nymID) {
        LogError()(OT_PRETTY_CLASS())("Wrong nym id on box receipt").Flush();

        return false;
    }

    if (false == receipt->VerifyAccount(remoteNym)) {
        LogError()(OT_PRETTY_CLASS())("Unable to verify box receipt").Flush();

        return false;
    }

    LogVerbose()(OT_PRETTY_CLASS())("Validated a push notification of type: ")(
        receipt->GetTypeString())
        .Flush();

    return process_get_box_receipt_response(
        lock,
        client,
        accountID,
        receipt,
        String::Factory(payload.c_str()),
        box,
        reason);
}

auto Server::process_get_nymbox_response(
    const Lock& lock,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);

    const auto& nymID = nym_->ID();
    auto serialized = String::Factory(reply.payload_);
    auto nymbox =
        api_.Factory().InternalSession().Ledger(nymID, nymID, server_id_);

    OT_ASSERT(nymbox);

    if (false == nymbox->LoadNymboxFromString(serialized)) {
        LogError()(OT_PRETTY_CLASS())("Error loading or verifying nymbox")
            .Flush();

        return false;
    }

    auto nymboxHash = identifier::Generic{};
    nymbox->ReleaseSignatures();
    nymbox->SignContract(*nym_, reason);
    nymbox->SaveContract();
    nymbox->SaveNymbox(nymboxHash);
    update_nymbox_hash(lock, reply, UpdateHash::Both);

    return true;
}

auto Server::process_check_nym_response(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply) -> bool
{
    update_nymbox_hash(lock, reply);

    if ((false == reply.bool_) || (reply.payload_->empty())) {
        LogVerbose()(OT_PRETTY_CLASS())("Server ")(
            server_id_)(" does not have nym ")(reply.nym_id2_.get())
            .Flush();

        return true;
    }

    auto serialized = proto::Factory<proto::Nym>(ByteArray{reply.payload_});

    auto nym = client.Wallet().Internal().Nym(serialized);

    if (nym) {

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("checkNymResponse: Retrieved nym "
                                      "(")(serialized.nymid())(") is invalid.")
            .Flush();
    }

    return false;
}

auto Server::process_get_account_data(
    const Lock& lock,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    const auto accountID =
        api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes());
    auto serializedAccount = String::Factory();
    auto serializedInbox = String::Factory();
    auto serializedOutbox = String::Factory();

    if (accountID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid account ID").Flush();

        return false;
    }

    if (false == reply.payload_->GetString(serializedAccount)) {
        LogError()(OT_PRETTY_CLASS())("Invalid serialized account").Flush();

        return false;
    }

    if (false == reply.payload2_->GetString(serializedInbox)) {
        LogError()(OT_PRETTY_CLASS())("Invalid serialized inbox").Flush();

        return false;
    }

    if (false == reply.payload3_->GetString(serializedOutbox)) {
        LogError()(OT_PRETTY_CLASS())("Invalid serialized outbox").Flush();

        return false;
    }

    return process_account_data(
        lock,
        accountID,
        serializedAccount,
        api_.Factory().IdentifierFromBase58(reply.inbox_hash_->Bytes()),
        serializedInbox,
        api_.Factory().IdentifierFromBase58(reply.outbox_hash_->Bytes()),
        serializedOutbox,
        reason);
}

auto Server::process_get_box_receipt_response(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    update_nymbox_hash(lock, reply);
    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    const auto& serverNym = *remote_nym_;
    const auto type = get_type(reply.depth_);

    if (BoxType::Invalid == type) { return false; }

    auto serialized = String::Factory(reply.payload_);
    auto boxReceipt = extract_box_receipt(
        serialized, serverNym, nymID, reply.transaction_num_);

    if (false == bool(boxReceipt)) { return false; }

    return process_get_box_receipt_response(
        lock,
        client,
        api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes()),
        boxReceipt,
        serialized,
        type,
        reason);
}

auto Server::process_get_box_receipt_response(
    const Lock& lock,
    const api::session::Client& client,
    const identifier::Generic& accountID,
    const std::shared_ptr<OTTransaction> receipt,
    const String& serialized,
    const BoxType type,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);
    OT_ASSERT(receipt);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    bool processInbox{false};

    switch (receipt->GetType()) {
        case transactionType::message: {
            process_incoming_message(lock, client, *receipt, reason);
        } break;
        case transactionType::instrumentNotice:
        case transactionType::instrumentRejection: {
            processInbox = true;
            process_incoming_instrument(receipt, reason);
        } break;
        case transactionType::transferReceipt: {
            processInbox = true;
            const auto pTransfer = extract_transfer(*receipt);

            if (pTransfer) {
                const auto& transfer = *pTransfer;

                if (is_internal_transfer(transfer)) {
                    LogDetail()(OT_PRETTY_CLASS())("Clearing internal transfer")
                        .Flush();
                } else {
                    LogDetail()(OT_PRETTY_CLASS())("Clearing outgoing transfer")
                        .Flush();
                }

                client.Workflow().ClearTransfer(nymID, server_id_, *receipt);
            } else {
                LogError()(OT_PRETTY_CLASS())("Invalid transfer").Flush();
            }
        } break;
        case transactionType::pending: {
            processInbox = true;
            const auto pTransfer = extract_transfer(*receipt);

            if (pTransfer) {
                const auto& transfer = *pTransfer;

                if (is_internal_transfer(transfer)) {
                    LogDetail()(OT_PRETTY_CLASS())(
                        "Conveying internal transfer")
                        .Flush();
                    client.Workflow().ConveyTransfer(
                        nymID, server_id_, *receipt);
                } else if (transfer.GetNymID() != nymID) {

                    LogDetail()(OT_PRETTY_CLASS())(
                        "Conveying incoming transfer")
                        .Flush();
                    client.Workflow().ConveyTransfer(
                        nymID, server_id_, *receipt);
                }
            } else {
                LogError()(OT_PRETTY_CLASS())("Invalid transfer").Flush();
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
        LogError()(OT_PRETTY_CLASS())("Failed to save box receipt:").Flush();
        LogError()(serialized).Flush();

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
            case BoxType::Invalid:
            default: {
            }
        }
    }

    return true;
}

auto Server::process_get_market_list_response(
    const Lock& lock,
    const Message& reply) -> bool
{
    auto data_file = api::Legacy::GetFilenameBin("market_data");

    auto* pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase the
    // data file. (So when the file is loaded from storage, it will correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    if (reply.depth_ == 0) {
        bool success = storage.EraseValueByKey(
            api_,
            api_.DataFolder().string(),
            api_.Internal().Legacy().Market(),  // "markets"
            reply.notary_id_->Get(),            // "markets/<notaryID>"
            data_file,
            "");  // "markets/<notaryID>/market_data.bin"
        if (!success) {
            LogError()(OT_PRETTY_CLASS())(
                "Error erasing market list from market "
                "folder: ")(data_file)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = ByteArray{};

    if ((reply.payload_->GetLength() <= 2) ||
        (false == reply.payload_->GetData(serialized))) {
        LogError()(OT_PRETTY_CLASS())("Unable to decode ascii-armored "
                                      "payload.")
            .Flush();
        return true;
    }

    // Unpack the market list...

    auto* pPacker = storage.GetPacker();

    OT_ASSERT(nullptr != pPacker);

    auto& packer = *pPacker;

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(packer.CreateBuffer());

    OT_ASSERT((pBuffer));

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized.data()), serialized.size());

    std::unique_ptr<OTDB::MarketList> pMarketList(
        dynamic_cast<OTDB::MarketList*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_LIST)));

    auto unpacked = pPacker->Unpack(buffer, *pMarketList);

    if (!unpacked) {
        LogError()(OT_PRETTY_CLASS())("Failed unpacking data.").Flush();
        return true;
    }

    bool success = storage.StoreObject(
        api_,
        *pMarketList,
        api_.DataFolder().string(),
        api_.Internal().Legacy().Market(),  // "markets"
        reply.notary_id_->Get(),            // "markets/<notaryID>"
        data_file,
        "");  // "markets/<notaryID>/market_data.bin"
    if (!success) {
        LogError()(OT_PRETTY_CLASS())("Error storing market list to market "
                                      "folder: ")(data_file)(".")
            .Flush();
    }

    return true;
}

auto Server::process_get_market_offers_response(
    const Lock& lock,
    const Message& reply) -> bool
{
    const String& marketID = reply.nym_id2_;  // market ID stored here.

    auto data_file = api::Legacy::GetFilenameBin(marketID.Get());

    auto* pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase the
    // data file. (So when the file is loaded from storage, it will correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    if (reply.depth_ == 0) {
        auto success = storage.EraseValueByKey(
            api_,
            api_.DataFolder().string(),
            api_.Internal().Legacy().Market(),  // "markets"
            reply.notary_id_->Get(),            // "markets/<notaryID>",
            "offers",                           // "markets/<notaryID>/offers"
                                                // todo stop hardcoding.
            data_file);  // "markets/<notaryID>/offers/<marketID>.bin"
        if (!success) {
            LogError()(OT_PRETTY_CLASS())(
                "Error erasing offers list from market "
                "folder: ")(data_file)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = ByteArray{};

    if ((reply.payload_->GetLength() <= 2) ||
        (false == reply.payload_->GetData(serialized))) {
        LogError()(OT_PRETTY_CLASS())("Unable to decode asci-armored "
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
        static_cast<const std::uint8_t*>(serialized.data()), serialized.size());

    std::unique_ptr<OTDB::OfferListMarket> pOfferList(
        dynamic_cast<OTDB::OfferListMarket*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_MARKET)));

    auto unpacked = pPacker->Unpack(buffer, *pOfferList);

    if (!unpacked) {
        LogError()(OT_PRETTY_CLASS())("Failed unpacking data.").Flush();
        return true;
    }

    bool success = storage.StoreObject(
        api_,
        *pOfferList,
        api_.DataFolder().string(),
        api_.Internal().Legacy().Market(),  // "markets"
        reply.notary_id_->Get(),            // "markets/<notaryID>",
        "offers",                           // "markets/<notaryID>/offers"
                                            // todo stop hardcoding.
        data_file);  // "markets/<notaryID>/offers/<marketID>.bin"
    if (!success) {
        LogError()(OT_PRETTY_CLASS())("Error storing ")(
            data_file)(" to market folder.")
            .Flush();
    }

    return true;
}

auto Server::process_get_market_recent_trades_response(
    const Lock& lock,
    const Message& reply) -> bool
{
    const String& marketID = reply.nym_id2_;  // market ID stored here.
    auto data_file = api::Legacy::GetFilenameBin(marketID.Get());

    auto* pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    //
    if (reply.depth_ == 0) {
        bool success = storage.EraseValueByKey(
            api_,
            api_.DataFolder().string(),
            api_.Internal().Legacy().Market(),  // "markets"
            reply.notary_id_->Get(),            // "markets/<notaryID>recent",
                                                // //
                                                // "markets/<notaryID>/recent"
                                                // // todo stop
                                                // hardcoding.
            data_file,
            "");  // "markets/<notaryID>/recent/<marketID>.bin"
        if (!success) {
            LogError()(OT_PRETTY_CLASS())(
                "Error erasing recent trades list from market "
                "folder: ")(data_file)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = ByteArray{};

    if ((reply.payload_->GetLength() <= 2) ||
        (false == reply.payload_->GetData(serialized))) {
        LogError()(OT_PRETTY_CLASS())("Unable to decode asci-armored "
                                      "payload.")
            .Flush();
        return true;
    }

    // Unpack the market list...

    auto* pPacker = storage.GetPacker();

    OT_ASSERT(nullptr != pPacker);

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());

    OT_ASSERT(nullptr != pBuffer);

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized.data()), serialized.size());

    std::unique_ptr<OTDB::TradeListMarket> pTradeList(
        dynamic_cast<OTDB::TradeListMarket*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_TRADE_LIST_MARKET)));

    auto unpacked = pPacker->Unpack(buffer, *pTradeList);

    if (!unpacked) {
        LogError()(OT_PRETTY_CLASS())("Failed unpacking data.").Flush();
        return true;
    }

    bool success = storage.StoreObject(
        api_,
        *pTradeList,
        api_.DataFolder().string(),
        api_.Internal().Legacy().Market(),  // "markets"
        reply.notary_id_->Get(),            // "markets/<notaryID>"
        "recent",                           // "markets/<notaryID>/recent"
                                            // todo stop hardcoding.
        data_file);  // "markets/<notaryID>/recent/<marketID>.bin"
    if (!success) {
        LogError()(OT_PRETTY_CLASS())("Error storing ")(
            data_file)(" to market folder.")
            .Flush();
    }

    return true;
}

auto Server::process_get_mint_response(const Lock& lock, const Message& reply)
    -> bool
{
    auto serialized = String::Factory(reply.payload_);
    const auto server =
        api_.Factory().NotaryIDFromBase58(reply.notary_id_->Bytes());
    const auto unit = api_.Factory().UnitIDFromBase58(
        reply.instrument_definition_id_->Bytes());

    auto pMmint = api_.Factory().Mint(server, unit);

    OT_ASSERT(pMmint);

    auto& mint = pMmint.Internal();

    // TODO check the server signature on the mint here...
    if (false == mint.LoadContractFromString(serialized)) {
        LogError()(OT_PRETTY_CLASS())(
            "Error loading mint from message payload.")
            .Flush();

        return false;
    }

    return mint.SaveMint({});
}

auto Server::process_get_nym_market_offers_response(
    const Lock& lock,
    const Message& reply) -> bool
{
    auto data_file = api::Legacy::GetFilenameBin(reply.nym_id_->Get());

    auto* pStorage = OTDB::GetDefaultStorage();
    OT_ASSERT(nullptr != pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly display an empty list on the screen, instead of a list of
    // outdated items.)
    //
    if (reply.depth_ == 0) {
        bool success = storage.EraseValueByKey(
            api_,
            api_.DataFolder().string(),
            api_.Internal().Legacy().Nym(),  // "nyms"
            reply.notary_id_->Get(),         // "nyms/<notaryID>",
            "offers",                        // "nyms/<notaryID>/offers"
                                             // todo stop hardcoding.
            data_file);  // "nyms/<notaryID>/offers/<NymID>.bin"
        if (!success) {
            LogError()(OT_PRETTY_CLASS())("Error erasing offers list from nyms "
                                          "folder: ")(data_file)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = ByteArray{};

    if ((reply.payload_->GetLength() <= 2) ||
        (false == reply.payload_->GetData(serialized))) {
        LogError()(OT_PRETTY_CLASS())("Unable to decode ascii-armored "
                                      "payload.")
            .Flush();
        return true;
    }

    // Unpack the nym's offer list...

    auto* pPacker = storage.GetPacker();

    OT_ASSERT(nullptr != pPacker);

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());
    OT_ASSERT(nullptr != pBuffer);

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized.data()), serialized.size());

    std::unique_ptr<OTDB::OfferListNym> pOfferList(
        dynamic_cast<OTDB::OfferListNym*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_NYM)));

    auto unpacked = pPacker->Unpack(buffer, *pOfferList);

    if (!unpacked) {
        LogError()(OT_PRETTY_CLASS())("Failed unpacking data.").Flush();
        return true;
    }

    bool success = storage.StoreObject(
        api_,
        *pOfferList,
        api_.DataFolder().string(),
        api_.Internal().Legacy().Nym(),  // "nyms"
        reply.notary_id_->Get(),         // "nyms/<notaryID>",
        "offers",                        // "nyms/<notaryID>/offers",
        data_file);                      // "nyms/<notaryID>/offers/<NymID>.bin"
    if (!success) {
        LogError()(OT_PRETTY_CLASS())("Error storing ")(
            data_file)(" to nyms folder.")
            .Flush();
    }

    return true;
}

void Server::process_incoming_instrument(
    const std::shared_ptr<OTTransaction> receipt,
    const PasswordPrompt& reason) const
{
    OT_ASSERT(receipt);

    auto paymentInbox = load_or_create_payment_inbox(reason);

    if (false == bool(paymentInbox)) { return; }

    add_transaction_to_ledger(
        receipt->GetTransactionNum(), receipt, *paymentInbox, reason);
}

void Server::process_incoming_message(
    const Lock& lock,
    const api::session::Client& client,
    const OTTransaction& receipt,
    const PasswordPrompt& reason) const
{
    OT_ASSERT(nym_);

    const auto& nymID = nym_->ID();
    auto serialized = String::Factory();
    receipt.GetReferenceString(serialized);
    auto message = api_.Factory().InternalSession().Message();

    OT_ASSERT(message);

    if (false == message->LoadContractFromString(serialized)) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to decode peer object: failed to deserialize message.")
            .Flush();

        return;
    }

    const auto recipientNymId =
        api_.Factory().NymIDFromBase58(message->nym_id2_->Bytes());
    const auto senderNymID =
        api_.Factory().NymIDFromBase58(message->nym_id_->Bytes());

    if (senderNymID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Missing sender nym ID").Flush();
    } else {
        client.Contacts().NymToContact(senderNymID);
    }

    if (recipientNymId == nymID) {
        const auto pPeerObject = api_.Factory().InternalSession().PeerObject(
            nym_, message->payload_, reason);

        if (false == bool(pPeerObject)) {
            LogError()(OT_PRETTY_CLASS())("Failed to instantiate object")
                .Flush();

            return;
        }

        auto& peerObject = *pPeerObject;

        switch (peerObject.Type()) {
            case (contract::peer::PeerObjectType::Message): {
                client.Activity().Internal().Mail(
                    recipientNymId,
                    *message,
                    otx::client::StorageBox::MAILINBOX,
                    peerObject);
            } break;
            case (contract::peer::PeerObjectType::Cash): {
                process_incoming_cash(
                    lock,
                    client,
                    receipt.GetTransactionNum(),
                    peerObject,
                    *message);
            } break;
            case (contract::peer::PeerObjectType::Payment): {
                const bool created = create_instrument_notice_from_peer_object(
                    lock,
                    client,
                    *message,
                    peerObject,
                    receipt.GetTransactionNum(),
                    reason);

                if (!created) {
                    LogError()(OT_PRETTY_CLASS())("Failed to create object")
                        .Flush();
                }
            } break;
            case (contract::peer::PeerObjectType::Request): {
                api_.Wallet().PeerRequestReceive(recipientNymId, peerObject);
            } break;
            case (contract::peer::PeerObjectType::Response): {
                api_.Wallet().PeerReplyReceive(recipientNymId, peerObject);
            } break;
            case contract::peer::PeerObjectType::Error:
            default: {
                LogError()(OT_PRETTY_CLASS())(
                    "Unable to decode peer object: unknown peer object type.")
                    .Flush();
            }
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Missing recipient nym.").Flush();
    }
}

auto Server::process_get_unit_definition_response(
    const Lock& lock,
    const Message& reply) -> bool
{
    update_nymbox_hash(lock, reply);
    const auto id = api_.Factory().IdentifierFromBase58(
        reply.instrument_definition_id_->Bytes());

    if (reply.payload_->empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Server reply does not contain contract ")(id)
            .Flush();

        return false;
    }

    const auto raw = ByteArray{reply.payload_};

    switch (static_cast<contract::Type>(reply.enum_)) {
        case contract::Type::nym: {
            const auto serialized = proto::Factory<proto::Nym>(raw);
            const auto contract = api_.Wallet().Internal().Nym(serialized);

            if (contract) {
                return (id == contract->ID());
            } else {
                LogError()(OT_PRETTY_CLASS())("Invalid nym").Flush();
            }
        } break;
        case contract::Type::notary: {
            const auto serialized = proto::Factory<proto::ServerContract>(raw);

            try {
                const auto contract =
                    api_.Wallet().Internal().Server(serialized);

                return (id == contract->ID());
            } catch (...) {
                LogError()(OT_PRETTY_CLASS())("Invalid server contract")
                    .Flush();
            }
        } break;
        case contract::Type::unit: {
            auto serialized = proto::Factory<proto::UnitDefinition>(raw);

            try {
                const auto contract =
                    api_.Wallet().Internal().UnitDefinition(serialized);

                return (id == contract->ID());
            } catch (...) {
                LogError()(OT_PRETTY_CLASS())("Invalid unit definition")
                    .Flush();
            }
        } break;
        case contract::Type::invalid:
        default: {
            LogError()(OT_PRETTY_CLASS())("invalid contract type").Flush();
        }
    }

    return false;
}

auto Server::process_issue_unit_definition_response(
    const Lock& lock,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    update_nymbox_hash(lock, reply);
    const auto accountID =
        api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes());

    if (reply.payload_->empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Server reply does not contain issuer account ")(accountID)
            .Flush();

        return false;
    }

    auto serialized = String::Factory(reply.payload_);
    const auto updated = api_.Wallet().Internal().UpdateAccount(
        accountID, *this, serialized, std::get<0>(pending_args_), reason);

    if (updated) {
        LogDetail()(OT_PRETTY_CLASS())("Saved new issuer account.").Flush();

        return init_new_account(accountID, reason);
    }

    LogError()(OT_PRETTY_CLASS())("Failed to save account.").Flush();

    return false;
}

auto Server::process_notarize_transaction_response(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    update_nymbox_hash(lock, reply);
    const auto accountID =
        api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes());
    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    const auto& serverNym = *remote_nym_;
    auto responseLedger =
        api_.Factory().InternalSession().Ledger(nymID, accountID, server_id_);

    OT_ASSERT(responseLedger);

    bool loaded =
        responseLedger->LoadLedgerFromString(String::Factory(reply.payload_));

    if (loaded) { loaded &= responseLedger->VerifyAccount(serverNym); }

    if (false == loaded) {
        LogError()(OT_PRETTY_CLASS())(
            "Error loading ledger from message payload.")
            .Flush();

        return false;
    }

    for (const auto& [number, pTransaction] :
         responseLedger->GetTransactionMap()) {
        if (false == bool(pTransaction)) {
            LogError()(OT_PRETTY_CLASS())("Invalid transaction ")(
                number)(" in response ledger")
                .Flush();

            return false;
        }

        auto& transaction = *pTransaction;
        const auto& transactionNumber = transaction.GetTransactionNum();

        if (false == verify_issued_number(lock, transactionNumber)) {
            LogVerbose()(OT_PRETTY_CLASS())("Skipping ")(
                "processing of server reply to transaction "
                "number ")(transactionNumber)(" since the number "
                                              "isn't even issued "
                                              "to me. Usually "
                                              "this ")("means "
                                                       "that I "
                                                       "ALREADY "
                                                       "processed "
                                                       "it, and "
                                                       "we are "
                                                       "now ")(
                "pr"
                "oc"
                "es"
                "si"
                "ng"
                " t"
                "he"
                " r"
                "ed"
                "un"
                "da"
                "nt"
                " n"
                "ym"
                "bo"
                "x "
                "no"
                "ti"
                "ce"
                " f"
                "or"
                " t"
                "he"
                " s"
                "am"
                "e"
                " ")("transaction. (Which was only sent to make sure we saw ")(
                "it.) ")
                .Flush();

            continue;
        }

        if (false == transaction.VerifyAccount(serverNym)) {
            LogConsole()(OT_PRETTY_CLASS())("Unable to verify transaction ")(
                transactionNumber)
                .Flush();

            return false;
        }

        process_response_transaction(lock, client, reply, transaction, reason);
    }

    return true;
}

auto Server::process_process_box_response(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply,
    const BoxType type,
    const identifier::Generic& accountID,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    update_nymbox_hash(lock, reply);
    auto originalMessage = extract_message(reply.in_reference_to_, *nym_);

    if (false == bool(originalMessage)) { return false; }

    OT_ASSERT(originalMessage);

    auto ledger = extract_ledger(originalMessage->payload_, accountID, *nym_);

    if (false == bool(ledger)) { return false; }

    auto responseLedger =
        extract_ledger(reply.payload_, accountID, *remote_nym_);

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
            replyTransaction,
            reason);
    } else {
        process_process_nymbox_response(
            lock,
            reply,
            *ledger,
            *responseLedger,
            transaction,
            replyTransaction,
            reason);
    }

    if (false == (transaction && replyTransaction)) {
        LogError()(OT_PRETTY_CLASS())("Invalid transaction").Flush();

        return false;
    }

    const auto notaryID = String::Factory(server_id_);
    auto receiptID = String::Factory("NOT_SET_YET");
    auto replyItem = replyTransaction->GetItem(itemType::atBalanceStatement);

    if (replyItem) {
        receiptID = reply.acct_id_;
    } else {
        replyItem = replyTransaction->GetItem(itemType::atTransactionStatement);

        if (replyItem) { nym_->GetIdentifier(receiptID); }
    }

    auto serialized = String::Factory();
    replyTransaction->SaveContractRaw(serialized);
    auto filename = replyTransaction->GetSuccess()
                        ? api::Legacy::GetFilenameSuccess(receiptID->Get())
                        : api::Legacy::GetFilenameFail(receiptID->Get());

    auto encoded = String::Factory();
    auto armored = Armored::Factory(serialized);

    if (false == armored->WriteArmoredString(encoded, "TRANSACTION")) {
        LogError()(OT_PRETTY_CLASS())("Failed to encode receipt").Flush();

        return false;
    }

    if (replyItem) {
        OTDB::StorePlainString(
            api_,
            encoded->Get(),
            api_.DataFolder().string(),
            api_.Internal().Legacy().Receipt(),
            notaryID->Get(),
            filename,
            "");
    } else {
        // This should never happen...
        filename = api::Legacy::GetFilenameError(receiptID->Get());
        LogError()(OT_PRETTY_CLASS())(
            "Error saving transaction "
            "receipt: ")(notaryID.get())('/')(filename)(".")
            .Flush();
        OTDB::StorePlainString(
            api_,
            encoded->Get(),
            api_.DataFolder().string(),
            api_.Internal().Legacy().Receipt(),
            notaryID->Get(),
            filename,
            "");
    }

    return true;
}

auto Server::process_process_inbox_response(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply,
    Ledger& ledger,
    Ledger& responseLedger,
    std::shared_ptr<OTTransaction>& transaction,
    std::shared_ptr<OTTransaction>& replyTransaction,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto accountID =
        api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes());
    transaction = ledger.GetTransaction(transactionType::processInbox);
    replyTransaction =
        responseLedger.GetTransaction(transactionType::atProcessInbox);

    if (false == (transaction && replyTransaction)) { return false; }

    const auto number = transaction->GetTransactionNum();
    const bool isIssued = verify_issued_number(lock, number);

    if (false == isIssued) {
        LogError()(OT_PRETTY_CLASS())("This reply has already been processed.")
            .Flush();

        return true;
    }

    consume_issued(lock, transaction->GetTransactionNum());
    auto inbox = load_account_inbox(accountID);

    OT_ASSERT(inbox);

    auto recordBox = load_or_create_account_recordbox(accountID, reason);

    OT_ASSERT(recordBox);

    for (auto& pReplyItem : replyTransaction->GetItemList()) {
        if (false == bool(pReplyItem)) {
            LogError()(OT_PRETTY_CLASS())("Invalid reply item").Flush();

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
                LogError()(OT_PRETTY_CLASS())(
                    "Unexpected reply item type "
                    "(")(replyItemType.get())(") in "
                                              "processInboxResponse")
                    .Flush();

                continue;
            }
        }

        if (Item::acknowledgement != replyItem.GetStatus()) {
            LogError()(OT_PRETTY_CLASS())(
                "processInboxResponse reply "
                "item ")(replyItemType.get())(": status == FAILED.")
                .Flush();

            continue;
        }

        LogDetail()(OT_PRETTY_CLASS())("processInboxResponse reply item ")(
            replyItemType.get())(": status == "
                                 "SUCCESS")
            .Flush();

        auto serializedOriginalItem = String::Factory();
        replyItem.GetReferenceString(serializedOriginalItem);
        auto pReferenceItem = api_.Factory().InternalSession().Item(
            serializedOriginalItem, server_id_, replyItem.GetReferenceToNum());
        auto pItem = (pReferenceItem) ? transaction->GetItemInRefTo(
                                            pReferenceItem->GetReferenceToNum())
                                      : nullptr;

        if (false == bool(pItem)) {
            LogError()(OT_PRETTY_CLASS())(
                "Unable to find original item in original processInbox "
                "transaction request, based on reply item.")
                .Flush();

            continue;
        }

        const auto& referenceItem = *pReferenceItem;
        const auto& item = *pItem;

        if (item.GetType() != requestType) {
            LogError()(OT_PRETTY_CLASS())(
                "Wrong original item TYPE, on reply item's copy of "
                "original item, than what was expected based on reply "
                "item's type.")
                .Flush();

            continue;
        }

        // TODO here: any other verification of item against referenceItem,
        // which are supposedly copies of the same item.

        LogDetail()(OT_PRETTY_CLASS())(
            "Checking client-side inbox for expected "
            "pending or receipt transaction: ")(item.GetReferenceToNum())("...")
            .Flush();
        auto pInboxTransaction =
            inbox->GetTransaction(item.GetReferenceToNum());

        if (false == bool(pInboxTransaction)) {
            LogError()(OT_PRETTY_CLASS())(
                "Unable to find the server's receipt, in my inbox, "
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
            recordBox->SignContract(nym, reason);
            recordBox->SaveContract();
            recordBox->SaveRecordBox();

            if (false == inboxTransaction.SaveBoxReceipt(*recordBox)) {
                LogError()(OT_PRETTY_CLASS())("Failed to save receipt ")(
                    inboxNumber)(" to record box")
                    .Flush();
            }
        } else {
            LogError()(OT_PRETTY_CLASS())("Unable to add transaction ")(
                inboxNumber)(" to record box "
                             "(still removing "
                             "it from asset "
                             "account "
                             "inbox, however).")
                .Flush();
        }

        inboxTransaction.DeleteBoxReceipt(*inbox);
        inbox->RemoveTransaction(inboxNumber);
    }

    inbox->ReleaseSignatures();
    inbox->SignContract(nym, reason);
    inbox->SaveContract();
    inbox->SaveInbox();

    return true;
}

auto Server::process_process_nymbox_response(
    const Lock& lock,
    const Message& reply,
    Ledger& ledger,
    Ledger& responseLedger,
    std::shared_ptr<OTTransaction>& transaction,
    std::shared_ptr<OTTransaction>& replyTransaction,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    transaction = ledger.GetTransaction(transactionType::processNymbox);
    replyTransaction =
        responseLedger.GetTransaction(transactionType::atProcessNymbox);

    if (false == (transaction && replyTransaction)) { return false; }

    accept_numbers(lock, *transaction, *replyTransaction);
    auto nymbox =
        api_.Factory().InternalSession().Ledger(nymID, nymID, server_id_);

    OT_ASSERT((nymbox));

    bool loadedNymbox{true};
    loadedNymbox &= nymbox->LoadNymbox();
    loadedNymbox &= nymbox->VerifyAccount(nym);

    OT_ASSERT(loadedNymbox);

    for (const auto& item : replyTransaction->GetItemList()) {
        OT_ASSERT(item);

        remove_nymbox_item(lock, *item, *nymbox, *transaction, reason);
    }

    nymbox->ReleaseSignatures();
    nymbox->SignContract(nym, reason);
    nymbox->SaveContract();
    auto nymboxHash = identifier::Generic{};
    nymbox->SaveNymbox(nymboxHash);
    set_local_nymbox_hash(lock, nymboxHash);

    return true;
}

auto Server::process_register_account_response(
    const Lock& lock,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    update_nymbox_hash(lock, reply);
    const auto accountID =
        api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes());

    if (reply.payload_->empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Server reply does not contain an account")
            .Flush();

        return false;
    }

    auto serialized = String::Factory(reply.payload_);
    const auto updated = api_.Wallet().Internal().UpdateAccount(
        accountID, *this, serialized, std::get<0>(pending_args_), reason);

    if (updated) {
        LogDetail()(OT_PRETTY_CLASS())("Saved new issuer account.").Flush();

        return init_new_account(accountID, reason);
    }

    LogError()(OT_PRETTY_CLASS())("Failed to save account.").Flush();

    return false;
}

auto Server::process_request_admin_response(
    const Lock& lock,
    const Message& reply) -> bool
{
    update_nymbox_hash(lock, reply);
    admin_attempted_->On();
    const auto& serverID = reply.notary_id_;

    if (reply.bool_) {
        LogDetail()(OT_PRETTY_CLASS())("Became admin on server ")(
            serverID.get())
            .Flush();
        admin_success_->On();
    } else {
        LogError()(OT_PRETTY_CLASS())("Server ")(serverID.get())(
            " rejected admin request")
            .Flush();
    }

    return true;
}

auto Server::process_register_nym_response(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply) -> bool
{
    auto serialized = proto::Factory<proto::Context>(ByteArray{reply.payload_});
    auto verified = proto::Validate(serialized, VERBOSE);

    if (false == verified) {
        LogError()(OT_PRETTY_CLASS())("Invalid context.").Flush();

        return false;
    }

    auto output = resync(lock, serialized);
    output &= harvest_unused(lock, client);

    return output;
}

auto Server::process_reply(
    const Lock& lock,
    const api::session::Client& client,
    const UnallocatedSet<otx::context::ManagedNumber>& managed,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    const auto accountID =
        api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes());
    const auto& serverNym = *remote_nym_;

    LogVerbose()(OT_PRETTY_CLASS())("Received ")(reply.command_.get())("(")(
        reply.success_ ? "success" : "failure")(")")
        .Flush();

    if (false == reply.VerifySignature(serverNym)) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: Server reply signature failed to verify.")
            .Flush();

        return false;
    }

    const RequestNumber requestNumber = reply.request_num_->ToLong();
    add_acknowledged_number(lock, requestNumber);
    RequestNumbers acknowledgedReplies{};

    if (reply.acknowledged_replies_.Output(acknowledgedReplies)) {
        remove_acknowledged_number(lock, acknowledgedReplies);
    }

    if (reply.success_) {
        for (const auto& number : managed) { number.SetSuccess(true); }
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("Message status: failed for ")(
            reply.command_.get())
            .Flush();

        return false;
    }

    switch (Message::Type(reply.command_->Get())) {
        case MessageType::checkNymResponse: {
            return process_check_nym_response(lock, client, reply);
        }
        case MessageType::getAccountDataResponse: {
            return process_get_account_data(lock, reply, reason);
        }
        case MessageType::getBoxReceiptResponse: {
            return process_get_box_receipt_response(
                lock, client, reply, reason);
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
        case MessageType::getMintResponse: {
            return process_get_mint_response(lock, reply);
        }
        case MessageType::getNymboxResponse: {
            return process_get_nymbox_response(lock, reply, reason);
        }
        case MessageType::getNymMarketOffersResponse: {
            return process_get_nym_market_offers_response(lock, reply);
        }
        case MessageType::notarizeTransactionResponse: {
            return process_notarize_transaction_response(
                lock, client, reply, reason);
        }
        case MessageType::processInboxResponse: {
            return process_process_box_response(
                lock,
                client,
                reply,
                BoxType::Inbox,
                api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes()),
                reason);
        }
        case MessageType::processNymboxResponse: {
            return process_process_box_response(
                lock, client, reply, BoxType::Nymbox, nymID, reason);
        }
        case MessageType::registerAccountResponse: {
            return process_register_account_response(lock, reply, reason);
        }
        case MessageType::requestAdminResponse: {
            return process_request_admin_response(lock, reply);
        }
        case MessageType::registerInstrumentDefinitionResponse: {
            return process_issue_unit_definition_response(lock, reply, reason);
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
            return process_unregister_account_response(lock, reply, reason);
        }
        case MessageType::unregisterNymResponse: {
            return process_unregister_nym_response(lock, reply, reason);
        }
        default: {
            update_nymbox_hash(lock, reply);

            return true;
        }
    }
}

void Server::process_response_transaction(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply,
    OTTransaction& response,
    const PasswordPrompt& reason)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    itemType type{itemType::error_state};

    if (Exit::Yes == get_item_type(response, type)) { return; }

    switch (response.GetType()) {
        case transactionType::atDeposit: {
            process_response_transaction_deposit(
                lock, client, reply, type, response, reason);
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
            process_response_transaction_withdrawal(
                lock, client, reply, type, response, reason);
        } break;
        case transactionType::atTransfer: {
            process_response_transaction_transfer(
                lock, client, reply, type, response);
        } break;
        case transactionType::atMarketOffer:
        case transactionType::atPaymentPlan:
        case transactionType::atSmartContract: {
            process_response_transaction_cron(
                lock, reply, type, response, reason);
        } break;
        default:
            LogError()(OT_PRETTY_CLASS())("wrong transaction type: ")(
                response.GetTypeString())(".")
                .Flush();
            break;
    }

    auto receiptID = String::Factory("ID_NOT_SET_YET");
    auto pItem = response.GetItem(itemType::atBalanceStatement);

    if (pItem) {
        receiptID->Set(reply.acct_id_);
    } else {
        pItem = response.GetItem(itemType::atTransactionStatement);

        if (pItem) {
            nym.GetIdentifier(receiptID);
        } else {
            LogError()(OT_PRETTY_CLASS())(
                "Response transaction contains neither balance statement nor "
                "transaction statement")
                .Flush();
        }
    }

    auto serialized = String::Factory();
    response.SaveContractRaw(serialized);
    auto armored = Armored::Factory(serialized);
    auto encoded = String::Factory();

    if (false == armored->WriteArmoredString(encoded, "TRANSACTION")) {
        LogError()(OT_PRETTY_CLASS())("Error saving transaction receipt "
                                      "(failed writing armored string): ")(
            api_.Internal().Legacy().Receipt())('/')(server_id_)('/')(
            receiptID.get())
            .Flush();

        return;
    }

    if (pItem) {
        auto filename = response.GetSuccess()
                            ? api::Legacy::GetFilenameSuccess(receiptID->Get())
                            : api::Legacy::GetFilenameFail(receiptID->Get());
        OTDB::StorePlainString(
            api_,
            encoded->Get(),
            api_.DataFolder().string(),
            api_.Internal().Legacy().Receipt(),
            server_id_.asBase58(api_.Crypto()),
            filename,
            "");
    } else {
        auto filename = api::Legacy::GetFilenameError(receiptID->Get());
        OTDB::StorePlainString(
            api_,
            encoded->Get(),
            api_.DataFolder().string(),
            api_.Internal().Legacy().Receipt(),
            server_id_.asBase58(api_.Crypto()),
            filename,
            "");
    }
}

void Server::process_response_transaction_cancel(
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
                LogError()(OT_PRETTY_CLASS())(
                    "(atCancelCronItem) Error removing issued number from "
                    "user nym.")
                    .Flush();
            }
        }
    }
}

void Server::process_response_transaction_cash_deposit(
    Item& replyItem,
    const PasswordPrompt& reason)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    auto serializedRequest = String::Factory();
    replyItem.GetReferenceString(serializedRequest);

    if (serializedRequest->empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to get original serialized request")
            .Flush();

        return;
    }

    auto pItem = api_.Factory().InternalSession().Item(serializedRequest);

    if (false == bool(pItem)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate request").Flush();

        return;
    }

    const auto& item = *pItem;
    auto rawPurse = ByteArray{};
    item.GetAttachment(rawPurse);
    const auto serializedPurse = proto::Factory<proto::Purse>(rawPurse);

    if (false == proto::Validate(serializedPurse, VERBOSE)) {
        LogError()(OT_PRETTY_CLASS())("Invalid purse").Flush();

        return;
    }

    auto purse = api_.Factory().InternalSession().Purse(serializedPurse);

    if (false == bool(purse)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate request purse")
            .Flush();

        return;
    }

    if (false == purse.Unlock(nym, reason)) {
        LogError()(OT_PRETTY_CLASS())("Failed to unlock request purse").Flush();

        return;
    }

    UnallocatedSet<UnallocatedCString> spentTokens{};
    UnallocatedVector<blind::Token> keepTokens{};

    for (const auto& token : purse) { spentTokens.insert(token.ID(reason)); }

    auto purseEditor = mutable_Purse(purse.Unit(), reason);
    auto& walletPurse = purseEditor.get();

    if (false == walletPurse.Unlock(nym, reason)) {
        LogError()(OT_PRETTY_CLASS())("Failed to unlock wallet purse").Flush();

        return;
    }

    auto token = walletPurse.Pop();

    while (token) {
        const auto id = token.ID(reason);

        if (1 == spentTokens.count(id)) {
            LogTrace()(OT_PRETTY_CLASS())("Removing spent token ")(
                id)(" from purse")
                .Flush();
        } else {
            LogTrace()(OT_PRETTY_CLASS())("Retaining unspent token ")(
                id)(" in purse")
                .Flush();
            keepTokens.emplace_back(std::move(token));
        }

        token = walletPurse.Pop();
    }

    for (auto& keepToken : keepTokens) {
        walletPurse.Push(std::move(keepToken), reason);
    }
}

void Server::process_response_transaction_cheque_deposit(
    const api::session::Client& client,
    const identifier::Generic& accountID,
    const Message* reply,
    const Item& replyItem,
    const PasswordPrompt& reason)
{
    auto empty = client.Factory().InternalSession().Message();

    OT_ASSERT(nym_);
    OT_ASSERT(empty);

    const auto& request = (pending_message_) ? *pending_message_ : *empty;
    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto paymentInbox = load_or_create_payment_inbox(reason);

    if (false == bool(paymentInbox)) { return; }

    auto serializedOriginal = String::Factory();
    Item* pOriginal{nullptr};
    replyItem.GetReferenceString(serializedOriginal);
    auto instantiatedOriginal =
        api_.Factory().InternalSession().Transaction(serializedOriginal);

    if (false == bool(instantiatedOriginal)) {
        LogError()(OT_PRETTY_CLASS())("Failed to deserialized original item")
            .Flush();

        return;
    }

    pOriginal = dynamic_cast<Item*>(instantiatedOriginal.get());

    if (nullptr == pOriginal) {
        LogError()(OT_PRETTY_CLASS())(
            "Incorrect transaction type on original item")
            .Flush();

        return;
    }

    auto& originalItem = *pOriginal;
    auto pCheque = api_.Factory().InternalSession().Cheque();

    if (false == bool(pCheque)) { return; }

    auto& cheque = *pCheque;
    auto serializedCheque = String::Factory();
    originalItem.GetAttachment(serializedCheque);

    if (false == cheque.LoadContractFromString(serializedCheque)) {
        LogError()(OT_PRETTY_CLASS())("ERROR loading cheque from string: ")(
            serializedCheque.get())(".")
            .Flush();

        return;
    }

    if (0 < cheque.GetAmount()) {
        // Cheque or voucher
        const auto workflowUpdated = client.Workflow().DepositCheque(
            nymID, accountID, cheque, request, reply);

        if (workflowUpdated) {
            LogDetail()(OT_PRETTY_CLASS())("Successfully updated workflow.")
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())("Failed to update workflow.").Flush();
        }

        return;
    }

    // Invoice
    const auto chequeNumber = cheque.GetTransactionNum();
    auto receipt_ids = paymentInbox->GetTransactionNums();

    for (const auto& receipt_id : receipt_ids) {
        TransactionNumber number{0};
        auto pPayment = get_instrument_by_receipt_id(
            api_, nym, receipt_id, *paymentInbox, reason);

        if (false == bool(pPayment) || !pPayment->SetTempValues(reason) ||
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
                LogError()(OT_PRETTY_CLASS())(
                    "Failed trying to delete the box receipt for a "
                    "cheque being removed from a payments "
                    "inbox: ")(lRemoveTransaction)(".")
                    .Flush();
            }

            if (paymentInbox->RemoveTransaction(lRemoveTransaction)) {
                paymentInbox->ReleaseSignatures();
                paymentInbox->SignContract(nym, reason);
                paymentInbox->SaveContract();

                if (!paymentInbox->SavePaymentInbox()) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Failure while trying to save payment "
                        "inbox.")
                        .Flush();
                } else {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "Removed cheque from payments inbox. "
                        "(Deposited "
                        "successfully). Saved payments inbox.")
                        .Flush();
                }
            }
        }

        if (!strPmntInboxTransaction->Exists()) { continue; }

        // TODO how many record boxes should exist? One per nym, or one per
        // account?
        auto recordBox = load_or_create_account_recordbox(nymID, reason);

        OT_ASSERT(recordBox);

        add_transaction_to_ledger(
            lRemoveTransaction, pTransaction, *recordBox, reason);
    }
}

void Server::process_response_transaction_cron(
    const Lock& lock,
    const Message& reply,
    const itemType type,
    OTTransaction& response,
    const PasswordPrompt& reason)
{
    OT_ASSERT(nym_);

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    auto pReplyItem = response.GetItem(type);

    if (false == bool(pReplyItem)) {
        LogError()(OT_PRETTY_CLASS())("Reply item is missing").Flush();

        return;
    }

    auto& replyItem = *pReplyItem;
    auto pOriginalItem = extract_original_item(replyItem);

    if (false == bool(pOriginalItem)) { return; }

    auto& originalItem = *pOriginalItem;
    auto serialized = String::Factory();
    originalItem.GetAttachment(serialized);

    if (serialized->empty()) {
        LogError()(OT_PRETTY_CLASS())(
            "Original item does not contain serialized cron item")
            .Flush();

        return;
    }

    auto pCronItem = api_.Factory().InternalSession().CronItem(serialized);

    if (false == bool(pCronItem)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to instantiate serialized cron item")
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
    auto nymfile = mutable_Nymfile(reason);
    auto pMsg = nymfile.get().GetOutpaymentsByTransNum(openingNumber, reason);

    if ((pMsg)) {
        const bool bRemovedOutpayment =
            nymfile.get().RemoveOutpaymentsByTransNum(openingNumber, reason);
        if (!bRemovedOutpayment) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed trying to remove outpayment with "
                "transaction num: ")(openingNumber)(".")
                .Flush();
        }
        if (!pMsg->payload_->GetString(strInstrument)) {
            LogError()(OT_PRETTY_CLASS())(
                "Unable to find payment instrument in outpayment "
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
            api_,
            api_.DataFolder().string(),
            api_.Internal().Legacy().PaymentInbox(),
            server_id_.asBase58(api_.Crypto()),
            nymID.asBase58(api_.Crypto()),
            "");
        const bool bExists2 = OTDB::Exists(
            api_,
            api_.DataFolder().string(),
            api_.Internal().Legacy().RecordBox(),
            server_id_.asBase58(api_.Crypto()),
            nymID.asBase58(api_.Crypto()),
            "");

        auto thePmntInbox = api_.Factory().InternalSession().Ledger(
            nymID,
            nymID,
            server_id_);  // payment inbox

        OT_ASSERT((thePmntInbox));

        auto theRecordBox = api_.Factory().InternalSession().Ledger(
            nymID,
            nymID,
            server_id_);  // record box

        OT_ASSERT((theRecordBox));

        bool bSuccessLoading1 = (bExists1 && thePmntInbox->LoadPaymentInbox());
        bool bSuccessLoading2 = (bExists2 && theRecordBox->LoadRecordBox());

        if (bExists1 && bSuccessLoading1) {
            bSuccessLoading1 =
                (thePmntInbox->VerifyContractID() &&
                 thePmntInbox->VerifySignature(nym));
            // (thePmntInbox->VerifyAccount(*pNym));
            // (No need to load all the Box
            // Receipts using VerifyAccount)
        } else if (!bExists1) {
            bSuccessLoading1 = thePmntInbox->GenerateLedger(
                nymID,
                server_id_,
                ledgerType::paymentInbox,

                true);  // bGenerateFile=true
        }
        if (bExists2 && bSuccessLoading2) {
            bSuccessLoading2 =
                (theRecordBox->VerifyContractID() &&
                 theRecordBox->VerifySignature(nym));
            // (theRecordBox->VerifyAccount(*pNym));
            // (No need to load all the Box
            // Receipts using VerifyAccount)
        } else if (!bExists2) {
            bSuccessLoading2 = theRecordBox->GenerateLedger(
                nymID,
                server_id_,
                ledgerType::recordBox,

                true);  // bGenerateFile=true
        }
        // By this point, the boxes DEFINITELY exist -- or not. (generation
        // might have failed, or verification.)
        //
        if (!bSuccessLoading1 || !bSuccessLoading2) {
            LogConsole()(OT_PRETTY_CLASS())(
                "While processing server reply containing "
                "rejection "
                "of cron item: WARNING: Unable to load, verify, "
                "or "
                "generate paymentInbox or recordBox, with "
                "IDs: ")(nymID)(" / ")(nymID)(".")
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

            auto theOutpayment = api_.Factory().InternalSession().Payment();

            OT_ASSERT((theOutpayment));

            if (strInstrument->Exists() &&
                theOutpayment->SetPayment(strInstrument) &&
                theOutpayment->SetTempValues(reason)) {
                theOutpayment->GetAllTransactionNumbers(
                    numlistOutpayment, reason);
            }

            const UnallocatedSet<std::int64_t> set_receipt_ids{
                thePmntInbox->GetTransactionNums()};
            for (const auto& receipt_id : set_receipt_ids) {
                auto pPayment = get_instrument_by_receipt_id(
                    api_, nym, receipt_id, *thePmntInbox, reason);

                if (false == bool(pPayment)) {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "While looping payments inbox to remove a "
                        "payment, unable to retrieve payment on "
                        "receipt ")(receipt_id)(" (skipping).")
                        .Flush();
                    continue;
                } else if (false == pPayment->SetTempValues(reason)) {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "While looping payments inbox to remove a "
                        "payment, unable to set temp values for "
                        "payment on "
                        "receipt ")(receipt_id)(" (skipping).")
                        .Flush();

                    continue;
                }

                NumList numlistIncomingPayment;

                pPayment->GetAllTransactionNumbers(
                    numlistIncomingPayment, reason);

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
                        LogError()(OT_PRETTY_CLASS())(
                            "Failed trying to delete the box receipt "
                            "for a transaction being removed from the "
                            "payment inbox.")
                            .Flush();
                    }
                    if (thePmntInbox->RemoveTransaction(receipt_id)) {
                        thePmntInbox->ReleaseSignatures();
                        thePmntInbox->SignContract(nym, reason);
                        thePmntInbox->SaveContract();

                        if (!thePmntInbox->SavePaymentInbox()) {
                            LogError()(OT_PRETTY_CLASS())(
                                "Failure while trying to save payment "
                                "inbox.")
                                .Flush();
                        } else {
                            LogConsole()(OT_PRETTY_CLASS())(
                                "Removed instrument from payment inbox."
                                " Saved payment inbox.")
                                .Flush();
                        }
                    } else {
                        LogError()(OT_PRETTY_CLASS())(
                            "Failed trying to remove transaction "
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
                    if (theOutpayment->IsPaymentPlan()) {
                        theOriginType = originType::origin_payment_plan;
                    } else if (theOutpayment->IsSmartContract()) {
                        theOriginType = originType::origin_smart_contract;
                    }
                }

                auto pNewTransaction =
                    api_.Factory().InternalSession().Transaction(
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
                    auto pNewItem = api_.Factory().InternalSession().Item(
                        *pNewTransaction,
                        itemType::notice,
                        identifier::Generic{});
                    OT_ASSERT((pNewItem));
                    // This may be unnecessary, I'll have to check
                    // CreateItemFromTransaction.
                    // I'll leave it for now.
                    pNewItem->SetStatus(replyItem.GetStatus());
                    // Since I am the last signer, the note contains the
                    // final version of the agreement.
                    pNewItem->SetNote(serialized);
                    pNewItem->SignContract(nym, reason);
                    pNewItem->SaveContract();

                    std::shared_ptr<Item> newItem{pNewItem.release()};
                    pNewTransaction->AddItem(newItem);
                    // -----------------------------------------------------
                    // Referencing myself here. We'll see how it works out.
                    pNewTransaction->SetReferenceToNum(openingNumber);
                    // The cheque, invoice, etc that used to be in the
                    // outpayments box.
                    pNewTransaction->SetReferenceString(strInstrument);

                    if (response.IsCancelled()) {
                        pNewTransaction->SetAsCancelled();
                    }

                    pNewTransaction->SignContract(nym, reason);
                    pNewTransaction->SaveContract();

                    std::shared_ptr<OTTransaction> newTransaction{
                        pNewTransaction.release()};
                    const bool bAdded =
                        theRecordBox->AddTransaction(newTransaction);

                    if (!bAdded) {
                        LogError()(OT_PRETTY_CLASS())("Unable to add "
                                                      "transaction ")(
                            newTransaction->GetTransactionNum())(" to "
                                                                 "record "
                                                                 "box "
                                                                 "(after "
                                                                 "tentati"
                                                                 "vely "
                                                                 "removin"
                                                                 "g "
                                                                 "from "
                                                                 "payment"
                                                                 " outbox"
                                                                 ", an "
                                                                 "action "
                                                                 "that "
                                                                 "is now "
                                                                 "cancell"
                                                                 "ed).")
                            .Flush();
                    } else {
                        theRecordBox->ReleaseSignatures();
                        theRecordBox->SignContract(nym, reason);
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
                            LogError()(OT_PRETTY_CLASS())(
                                "For Record Box... "
                                "Failed trying to SaveBoxReceipt. "
                                "Contents: ")(strNewTransaction.get())(".")
                                .Flush();
                        }
                    }
                }     // if (nullptr != pNewTransaction)
                else  // should never happen
                {
                    LogError()(OT_PRETTY_CLASS())(
                        "Failed while trying to generate "
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

void Server::process_response_transaction_deposit(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply,
    const itemType type,
    OTTransaction& response,
    const PasswordPrompt& reason)
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
                LogDetail()(OT_PRETTY_CLASS())(
                    "TRANSACTION SUCCESS -- Server acknowledges "
                    "deposit.")
                    .Flush();
                if (itemType::atDepositCheque == item.GetType()) {
                    process_response_transaction_cheque_deposit(
                        client,
                        response.GetPurportedAccountID(),
                        &reply,
                        item,
                        reason);
                } else if (itemType::atDeposit == item.GetType()) {
                    process_response_transaction_cash_deposit(item, reason);
                }
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "TRANSACTION FAILURE -- Server rejects deposit.")
                    .Flush();
            }
        }
    }

    consume_issued(lock, response.GetTransactionNum());
}

void Server::process_response_transaction_exchange_basket(
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
                LogError()(OT_PRETTY_CLASS())(
                    "Original item does not contain serialized basket")
                    .Flush();

                return;
            }

            auto pBasket = api_.Factory().InternalSession().Basket();

            OT_ASSERT(pBasket);

            auto& basket = *pBasket;

            if (false == basket.LoadContractFromString(serialized)) {
                LogError()(OT_PRETTY_CLASS())("Failed to instantiate basket")
                    .Flush();

                return;
            }

            for (auto i = 0; i < basket.Count(); ++i) {
                auto* requestItem = basket.At(i);

                if (nullptr == requestItem) {
                    LogError()(OT_PRETTY_CLASS())("Invalid basket").Flush();

                    return;
                }

                const auto closingNumber = requestItem->closing_transaction_no_;
                recover_available_number(lock, closingNumber);
            }

            const auto closingNumber = basket.GetClosingNum();
            recover_available_number(lock, closingNumber);
        }
    }
}

void Server::process_response_transaction_pay_dividend(
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
                LogConsole()(OT_PRETTY_CLASS())(
                    "TRANSACTION SUCCESS -- Server acknowledges "
                    "dividend "
                    "payout.")
                    .Flush();
            } else {
                LogConsole()(OT_PRETTY_CLASS())(
                    "TRANSACTION FAILURE -- Server rejects dividend "
                    "payout.")
                    .Flush();
            }
        }
    }

    consume_issued(lock, response.GetTransactionNum());
}

void Server::process_response_transaction_transfer(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply,
    const itemType type,
    OTTransaction& response)
{
    OT_ASSERT(nym_);

    const auto& nymID = nym_->ID();
    auto pItem = response.GetItem(type);

    if (false == bool(pItem)) {
        LogError()(OT_PRETTY_CLASS())(
            "Response transaction does not contain response transfer item")
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
        LogError()(OT_PRETTY_CLASS())("Invalid serialized transaction item.")
            .Flush();

        return;
    }

    auto pTransfer = api_.Factory().InternalSession().Item(
        serialized,
        responseItem.GetRealNotaryID(),
        responseItem.GetReferenceToNum());

    if (false == bool(pTransfer)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate transaction item.")
            .Flush();

        return;
    }

    const auto& transfer = *pTransfer;

    if (is_internal_transfer(transfer)) {
        LogDetail()(OT_PRETTY_CLASS())("Acknowledging internal transfer")
            .Flush();
    } else {
        LogDetail()(OT_PRETTY_CLASS())("Acknowledging outgoing transfer")
            .Flush();
    }

    if (TransactionAttempt::Accepted == status) {

        client.Workflow().AcknowledgeTransfer(nymID, transfer, reply);
    } else {

        client.Workflow().AbortTransfer(nymID, transfer, reply);
    }
}

void Server::process_response_transaction_withdrawal(
    const Lock& lock,
    const api::session::Client& client,
    const Message& reply,
    const itemType type,
    OTTransaction& response,
    const PasswordPrompt& reason)
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
            auto theVoucher = api_.Factory().InternalSession().Cheque();

            OT_ASSERT((theVoucher));

            pItem->GetAttachment(strVoucher);

            if (theVoucher->LoadContractFromString(strVoucher)) {
                LogVerbose()(OT_PRETTY_CLASS())(
                    "Received voucher from server:  ")
                    .Flush();
                LogVerbose()(OT_PRETTY_CLASS())(strVoucher.get()).Flush();
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
            process_incoming_cash_withdrawal(*pItem, reason);
        }
    }

    consume_issued(lock, response.GetTransactionNum());
}

auto Server::process_incoming_cash(
    const Lock& lock,
    const api::session::Client& client,
    const TransactionNumber number,
    const PeerObject& incoming,
    const Message& message) const -> bool
{
    OT_ASSERT(nym_);
    OT_ASSERT(contract::peer::PeerObjectType::Cash == incoming.Type());

    if (false == incoming.Validate()) {
        LogError()(OT_PRETTY_CLASS())("Invalid peer object.").Flush();

        return false;
    }

    const auto& nym = *nym_;
    const auto& nymID = nym.ID();
    const auto& serverID = server_id_;
    const auto strNotaryID = String::Factory(serverID);
    const auto strNymID = String::Factory(nymID);
    const auto& purse = incoming.Purse();

    if (false == bool(purse)) {
        LogError()(OT_PRETTY_CLASS())("No purse found").Flush();

        return false;
    }

    const auto workflowID =
        client.Workflow().ReceiveCash(nymID, purse, message);

    if (workflowID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to create workflow").Flush();

        return false;
    }

    LogDetail()(OT_PRETTY_CLASS())("Created workflow ")(workflowID).Flush();

    return true;
}

void Server::process_incoming_cash_withdrawal(
    const Item& item,
    const PasswordPrompt& reason) const
{
    OT_ASSERT(nym_);
    OT_ASSERT(remote_nym_);

    const auto& nym = *nym_;
    const auto& serverNym = *remote_nym_;
    const auto& nymID = nym.ID();
    auto rawPurse = ByteArray{};
    item.GetAttachment(rawPurse);
    const auto serializedPurse = proto::Factory<proto::Purse>(rawPurse);

    if (false == proto::Validate(serializedPurse, VERBOSE)) {
        LogError()(OT_PRETTY_CLASS())("Invalid serialized purse").Flush();

        return;
    }

    LogInsane()(OT_PRETTY_CLASS())("Serialized purse is valid").Flush();
    auto requestPurse = api_.Factory().InternalSession().Purse(serializedPurse);

    if (requestPurse) {
        LogInsane()(OT_PRETTY_CLASS())("Purse instantiated").Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate purse").Flush();

        return;
    }

    if (requestPurse.Unlock(nym, reason)) {
        LogInsane()(OT_PRETTY_CLASS())("Purse unlocked").Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to unlock purse").Flush();

        return;
    }

    auto pMint = api_.Factory().Mint(server_id_, requestPurse.Unit());

    if (pMint) {
        LogInsane()(OT_PRETTY_CLASS())("Mint loaded").Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to load mint").Flush();

        return;
    }

    auto& mint = pMint.Internal();
    const bool validMint = mint.LoadMint({}) && mint.VerifyMint(serverNym);

    if (validMint) {
        LogInsane()(OT_PRETTY_CLASS())("Mint is valid").Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Invalid mint").Flush();

        return;
    }

    const auto processed = requestPurse.Internal().Process(nym, pMint, reason);

    if (processed) {
        LogInsane()(OT_PRETTY_CLASS())("Token processed").Flush();
        auto purseEditor = api_.Wallet().Internal().mutable_Purse(
            nymID, requestPurse.Notary(), requestPurse.Unit(), reason);
        auto& walletPurse = purseEditor.get();

        OT_ASSERT(walletPurse.Unlock(nym, reason));

        auto token = requestPurse.Pop();

        while (token) {
            walletPurse.Push(std::move(token), reason);
            token = requestPurse.Pop();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to process token").Flush();
    }
}

auto Server::process_unregister_account_response(
    const Lock& lock,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    auto serialized = String::Factory();

    if (reply.in_reference_to_->Exists()) {
        reply.in_reference_to_->GetString(serialized);
    }

    auto originalMessage = api_.Factory().InternalSession().Message();

    OT_ASSERT((originalMessage));

    if (serialized->Exists() &&
        originalMessage->LoadContractFromString(serialized) &&
        originalMessage->VerifySignature(*Nym()) &&
        originalMessage->nym_id_->Compare(reply.nym_id_) &&
        originalMessage->acct_id_->Compare(reply.acct_id_) &&
        originalMessage->command_->Compare("unregisterAccount")) {

        const auto theAccountID =
            api_.Factory().IdentifierFromBase58(reply.acct_id_->Bytes());
        auto account =
            api_.Wallet().Internal().mutable_Account(theAccountID, reason);

        if (account) {
            account.Release();
            api_.Wallet().DeleteAccount(theAccountID);
        }

        LogConsole()(OT_PRETTY_CLASS())("Successfully DELETED Asset Acct ")(
            reply.acct_id_.get())(" from Server: ")(server_id_)(".")
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("The server just for some reason tried "
                                      "to trick me into erasing my account ")(
            reply.acct_id_.get())(" on Server ")(server_id_)(".")
            .Flush();
    }

    return true;
}

auto Server::process_unregister_nym_response(
    const Lock& lock,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    auto serialized = String::Factory();
    auto originalMessage = api_.Factory().InternalSession().Message();

    OT_ASSERT((originalMessage));

    if (reply.in_reference_to_->Exists()) {
        reply.in_reference_to_->GetString(serialized);
    }

    if (serialized->Exists() &&
        originalMessage->LoadContractFromString(serialized) &&
        originalMessage->VerifySignature(*Nym()) &&
        originalMessage->nym_id_->Compare(reply.nym_id_) &&
        originalMessage->command_->Compare("unregisterNym")) {
        Reset();
        LogConsole()(OT_PRETTY_CLASS())(
            "Successfully DELETED Nym from Server: removed request number, "
            "plus all issued and transaction numbers for Nym ")(
            reply.nym_id_.get())(" for Server ")(server_id_)(".")
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "The server just for some reason tried to trick me into erasing my "
            "issued and transaction numbers for Nym ")(reply.nym_id_.get())(
            ", Server ")(server_id_)(".")
            .Flush();
    }

    return true;
}

void Server::process_unseen_reply(
    const Lock& lock,
    const api::session::Client& client,
    const Item& input,
    ReplyNoticeOutcomes& notices,
    const PasswordPrompt& reason)
{
    if (Item::acknowledgement != input.GetStatus()) {
        LogConsole()(OT_PRETTY_CLASS())("Reply item incorrect status").Flush();

        return;
    }

    auto serializedReply = String::Factory();
    input.GetAttachment(serializedReply);

    if (!serializedReply->Exists()) {
        LogError()(OT_PRETTY_CLASS())(
            "Error loading original server reply message from notice.")
            .Flush();

        return;
    }

    std::shared_ptr<Message> message{
        api_.Factory().InternalSession().Message()};

    OT_ASSERT(message);

    if (false == message->LoadContractFromString(serializedReply)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed loading original server reply message from replyNotice: ")(
            serializedReply.get())
            .Flush();

        return;
    }

    ReplyNoticeOutcome outcome{};
    auto& [number, result] = outcome;
    auto& [status, reply] = result;
    number = String::StringToUlong(message->request_num_->Get());
    status = (message->success_) ? otx::LastReplyStatus::MessageSuccess
                                 : otx::LastReplyStatus::MessageFailed;
    reply = message;
    notices.emplace_back(outcome);
    process_reply(lock, client, {}, *reply, reason);
}

auto Server::Purse(const identifier::UnitDefinition& id) const
    -> const opentxs::otx::blind::Purse&
{
    return api_.Wallet().Purse(nym_->ID(), server_id_, id);
}

auto Server::Queue(
    const api::session::Client& client,
    std::shared_ptr<Message> message,
    const PasswordPrompt& reason,
    const ExtraArgs& args) -> Server::QueueResult
{
    START_SERVER_CONTEXT();

    return start(lock, reason, client, message, args);
}

auto Server::Queue(
    const api::session::Client& client,
    std::shared_ptr<Message> message,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox,
    UnallocatedSet<otx::context::ManagedNumber>* numbers,
    const PasswordPrompt& reason,
    const ExtraArgs& args) -> Server::QueueResult
{
    START_SERVER_CONTEXT();

    if (false == bool(inbox)) {
        LogError()(OT_PRETTY_CLASS())("Inbox is not instantiated").Flush();

        return {};
    }

    if (false == bool(outbox)) {
        LogError()(OT_PRETTY_CLASS())("Outbox is not instantiated").Flush();

        return {};
    }

    return start(
        lock,
        reason,
        client,
        message,
        args,
        proto::DELIVERTYSTATE_PENDINGSEND,
        ActionType::Normal,
        inbox,
        outbox,
        numbers);
}

auto Server::RefreshNymbox(
    const api::session::Client& client,
    const PasswordPrompt& reason) -> Server::QueueResult
{
    START_SERVER_CONTEXT();

    return start(
        lock,
        reason,
        client,
        nullptr,
        {},
        proto::DELIVERTYSTATE_NEEDNYMBOX,
        ActionType::ProcessNymbox);
}

auto Server::remove_acknowledged_number(const Lock& lock, const Message& reply)
    -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    UnallocatedSet<RequestNumber> list{};

    if (false == reply.acknowledged_replies_.Output(list)) { return false; }

    return remove_acknowledged_number(lock, list);
}

auto Server::remove_nymbox_item(
    const Lock& lock,
    const Item& replyItem,
    Ledger& nymbox,
    OTTransaction& transaction,
    const PasswordPrompt& reason) -> bool
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
            // (The transaction statement itself is already handled

            return true;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())(
                "Unexpected replyItem type while processing Nymbox: ")(
                replyType.get())
                .Flush();

            return false;
        }
    }

    if (Item::acknowledgement != replyItem.GetStatus()) {
        LogDetail()(OT_PRETTY_CLASS())("processNymboxResponse reply item ")(
            replyType.get())(": status == FAILED")
            .Flush();

        return true;
    }

    LogDetail()(OT_PRETTY_CLASS())("processNymboxResponse reply item ")(
        replyType.get())(": status == SUCCESS")
        .Flush();
    auto serialized = String::Factory();
    replyItem.GetReferenceString(serialized);
    auto processNymboxItem =
        api_.Factory().InternalSession().Item(serialized, server_id_, 0);

    if (false == bool(processNymboxItem)) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to find original item in original processNymbox "
            "transaction request, based on reply item.")
            .Flush();

        return false;
    }

    auto item =
        transaction.GetItemInRefTo(processNymboxItem->GetReferenceToNum());

    OT_ASSERT(item);

    if (item->GetType() != requestType) {
        LogError()(OT_PRETTY_CLASS())(
            "Wrong original item TYPE, on reply item's copy of original "
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
            LogError()(OT_PRETTY_CLASS())(
                "Unexpected replyItem::type while processing "
                "Nymbox: ")(replyType.get())
                .Flush();

            return false;
        }
    }

    if (false == bool(serverTransaction)) {
        LogDetail()(OT_PRETTY_CLASS())(
            "The original processNymbox item referred to txn "
            "number ")(item->GetReferenceToNum())(", but that "
                                                  "receipt "
                                                  "wasn't in my "
                                                  "Nymbox. (We "
                                                  "probably "
                                                  "processed "
                                                  "this server "
                                                  "reply "
                                                  "ALREADY, and "
                                                  "now we're "
                                                  "just seeing "
                                                  "it again, "
                                                  "since an "
                                                  "extra copy "
                                                  "was dropped "
                                                  "into the "
                                                  "Nymbox "
                                                  "originally. "
                                                  "It "
                                                  "happens. "
                                                  "Skipping).")
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
                     ? api_.Factory().InternalSession().CronItem(
                           strOriginalCronItem)
                     : nullptr);
            auto pUpdatedCronItem =
                (strUpdatedCronItem->Exists()
                     ? api_.Factory().InternalSession().CronItem(
                           strUpdatedCronItem)
                     : nullptr);
            std::unique_ptr<OTCronItem>& pCronItem =
                ((pUpdatedCronItem) ? pUpdatedCronItem : pOriginalCronItem);

            if (false == bool(pCronItem) || false == bool(pOriginalCronItem)) {
                LogError()(OT_PRETTY_CLASS())(
                    "Error loading original CronItem from Nymbox receipt, "
                    "from string: ")(strOriginalCronItem.get())(".")
                    .Flush();

                return false;
            }

            auto theCancelerNymID = identifier::Nym{};
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
                        LogError()(OT_PRETTY_CLASS())(
                            "Error removing issued number from user nym (for "
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
                auto nymfile = mutable_Nymfile(reason);
                auto pMsg = nymfile.get().GetOutpaymentsByTransNum(
                    openingNumber, reason);

                if (pMsg) {
                    const bool bRemovedOutpayment =
                        nymfile.get().RemoveOutpaymentsByTransNum(
                            openingNumber, reason);

                    if (!bRemovedOutpayment) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Failed trying to remove outpayment with trans "
                            "num: ")(openingNumber)(".")
                            .Flush();
                    }

                    if (!pMsg->payload_->GetString(strSentInstrument)) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Unable to find payment "
                            "instrument in outpayment "
                            "message with trans num: ")(openingNumber)(".")
                            .Flush();
                    }
                }

                const auto notaryID = String::Factory(server_id_);
                const bool exists1 = OTDB::Exists(
                    api_,
                    api_.DataFolder().string(),
                    api_.Internal().Legacy().PaymentInbox(),
                    notaryID->Get(),
                    nymID.asBase58(api_.Crypto()),
                    "");
                const bool exists2 = OTDB::Exists(
                    api_,
                    api_.DataFolder().string(),
                    api_.Internal().Legacy().RecordBox(),
                    notaryID->Get(),
                    nymID.asBase58(api_.Crypto()),
                    "");
                auto paymentInbox = api_.Factory().InternalSession().Ledger(
                    nymID, nymID, server_id_);

                OT_ASSERT((paymentInbox));

                auto recordBox = api_.Factory().InternalSession().Ledger(
                    nymID, nymID, server_id_);

                OT_ASSERT((recordBox));

                bool loaded1 = (exists1 && paymentInbox->LoadPaymentInbox());
                bool loaded2 = (exists2 && recordBox->LoadRecordBox());

                if (exists1 && loaded1) {
                    loaded1 =
                        (paymentInbox->VerifyContractID() &&
                         paymentInbox->VerifySignature(nym));
                } else if (!exists1) {
                    loaded1 = paymentInbox->GenerateLedger(
                        nymID,
                        server_id_,
                        ledgerType::paymentInbox,

                        true);
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
                    LogConsole()(OT_PRETTY_CLASS())(
                        "While processing server rejection of "
                        "cron item: "
                        "WARNING: Unable to load, verify, or "
                        "generate "
                        "paymentInbox or recordBox, with "
                        "IDs: ")(nymID)(" / ")(nymID)(".")
                        .Flush();
                } else {
                    auto theOutpayment =
                        api_.Factory().InternalSession().Payment();

                    OT_ASSERT((theOutpayment));

                    if (strSentInstrument->Exists() &&
                        theOutpayment->SetPayment(strSentInstrument) &&
                        theOutpayment->SetTempValues(reason)) {
                        theOutpayment->GetAllTransactionNumbers(
                            numlistOutpayment, reason);
                    }

                    auto tempPayment =
                        api_.Factory().InternalSession().Payment();

                    OT_ASSERT((tempPayment));

                    const String& strCronItem =
                        (strUpdatedCronItem->Exists() ? strUpdatedCronItem
                                                      : strOriginalCronItem);

                    if (strCronItem.Exists() &&
                        tempPayment->SetPayment(strCronItem) &&
                        tempPayment->SetTempValues(reason)) {
                        tempPayment->GetAllTransactionNumbers(
                            numlistOutpayment, reason);
                    }

                    const UnallocatedSet<std::int64_t> set_receipt_ids{
                        paymentInbox->GetTransactionNums()};

                    for (const auto& receipt_id : set_receipt_ids) {
                        auto pPayment = get_instrument_by_receipt_id(
                            api_, nym, receipt_id, *paymentInbox, reason);

                        if (false == bool(pPayment)) {
                            LogConsole()(OT_PRETTY_CLASS())(
                                "(Upon receiving notice) While "
                                "looping "
                                "payments inbox to remove a "
                                "payment, unable to "
                                "retrieve payment on "
                                "receipt ")(receipt_id)(" ("
                                                        "skipping"
                                                        ").")
                                .Flush();
                            continue;
                        } else if (false == pPayment->SetTempValues(reason)) {
                            LogConsole()(OT_PRETTY_CLASS())(
                                "(Upon receiving notice) While looping "
                                "payments inbox to remove a payment, unable to "
                                "set temp values for payment on "
                                "receipt ")(receipt_id)(" (skipping).")
                                .Flush();
                            continue;
                        }

                        NumList numlistIncomingPayment;
                        pPayment->GetAllTransactionNumbers(
                            numlistIncomingPayment, reason);

                        if (numlistOutpayment.VerifyAny(
                                numlistIncomingPayment)) {
                            auto pTransPaymentInbox =
                                paymentInbox->GetTransaction(receipt_id);

                            OT_ASSERT(pTransPaymentInbox);

                            if (false ==
                                paymentInbox->DeleteBoxReceipt(receipt_id)) {
                                LogError()(OT_PRETTY_CLASS())(
                                    "Failed trying to delete the box receipt "
                                    "for a transaction being removed from the "
                                    "payment inbox.")
                                    .Flush();
                            }

                            if (paymentInbox->RemoveTransaction(receipt_id)) {
                                paymentInbox->ReleaseSignatures();
                                paymentInbox->SignContract(nym, reason);
                                paymentInbox->SaveContract();

                                if (!paymentInbox->SavePaymentInbox()) {
                                    LogError()(OT_PRETTY_CLASS())(
                                        "Failure while trying to save "
                                        "payment inbox.")
                                        .Flush();
                                } else {
                                    LogConsole()(OT_PRETTY_CLASS())(
                                        "Removed instrument from payment "
                                        "inbox. Saved payment inbox.")
                                        .Flush();
                                }
                            } else {
                                LogError()(OT_PRETTY_CLASS())(
                                    "Failed trying to remove transaction "
                                    "from payment inbox. (Should never "
                                    "happen).")
                                    .Flush();
                            }
                        }
                    }

                    std::shared_ptr<OTTransaction> newTransaction{
                        api_.Factory().InternalSession().Transaction(
                            *recordBox,
                            transactionType::notice,
                            theOriginType,
                            serverTransaction->GetTransactionNum())};

                    if (newTransaction) {
                        if (false != bool(pNoticeItem)) {
                            std::shared_ptr<Item> newItem{
                                api_.Factory().InternalSession().Item(
                                    *newTransaction,
                                    itemType::notice,
                                    identifier::Generic{})};

                            OT_ASSERT(newItem);

                            newItem->SetStatus(pNoticeItem->GetStatus());
                            newItem->SetNote(strUpdatedCronItem);
                            newItem->SignContract(nym, reason);
                            newItem->SaveContract();
                            newTransaction->AddItem(newItem);
                        }

                        TransactionNumber lTransNumForDisplay{0};

                        if (!theOutpayment->IsValid() ||
                            !theOutpayment->GetTransNumDisplay(
                                lTransNumForDisplay)) {
                            auto temp =
                                api_.Factory().InternalSession().Payment();

                            OT_ASSERT(false != bool(temp));

                            const String& serializedCronItem =
                                (strUpdatedCronItem->Exists()
                                     ? strUpdatedCronItem
                                     : strOriginalCronItem);

                            if (serializedCronItem.Exists() &&
                                temp->SetPayment(serializedCronItem) &&
                                temp->SetTempValues(reason)) {
                                temp->GetTransNumDisplay(lTransNumForDisplay);
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

                        newTransaction->SignContract(nym, reason);
                        newTransaction->SaveContract();
                        const bool bAdded =
                            recordBox->AddTransaction(newTransaction);

                        if (!bAdded) {
                            LogError()(OT_PRETTY_CLASS())(
                                "Unable to add "
                                "txn ")(newTransaction->GetTransactionNum())(
                                " to record "
                                "box (after "
                                "tentatively"
                                " removing "
                                "from "
                                "payment "
                                "outbox, an "
                                "action "
                                "that is "
                                "now "
                                "canceled).")
                                .Flush();

                            return false;
                        }

                        recordBox->ReleaseSignatures();
                        recordBox->SignContract(nym, reason);
                        recordBox->SaveContract();
                        recordBox->SaveRecordBox();

                        if (!newTransaction->SaveBoxReceipt(*recordBox)) {
                            auto strNewTransaction =
                                String::Factory(*newTransaction);
                            LogError()(OT_PRETTY_CLASS())(
                                "For Record Box... Failed trying to "
                                "SaveBoxReceipt. "
                                "Contents: ")(strNewTransaction.get())(".")
                                .Flush();
                        }
                    } else {
                        LogError()(OT_PRETTY_CLASS())(
                            "Failed while trying to generate transaction in "
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
            LogVerbose()(OT_PRETTY_CLASS())(
                "Successfully removed finalReceipt from Nymbox with opening "
                "num: ")(serverTransaction->GetReferenceToNum())
                .Flush();
            const bool removed =
                consume_issued(lock, serverTransaction->GetReferenceToNum());

            if (removed) {
                LogDetail()(OT_PRETTY_CLASS())(
                    "**** Due to finding a finalReceipt, consuming issued "
                    "opening number from nym:  ")(
                    serverTransaction->GetReferenceToNum())
                    .Flush();
            } else {
                LogDetail()(OT_PRETTY_CLASS())(
                    "**** Noticed a finalReceipt, but Opening "
                    "Number ")(serverTransaction->GetReferenceToNum())(
                    " had ALREADY been "
                    "removed from nym.")
                    .Flush();
            }

            OTCronItem::EraseActiveCronReceipt(
                api_,
                api_.DataFolder().string(),
                serverTransaction->GetReferenceToNum(),
                nymID,
                serverTransaction->GetPurportedNotaryID());
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(
                "Unexpected replyItem:type while processing "
                "Nymbox: ")(replyType.get())(".")
                .Flush();

            return false;
        }
    }

    serverTransaction->DeleteBoxReceipt(nymbox);
    nymbox.RemoveTransaction(serverTransaction->GetTransactionNum());

    return true;
}

auto Server::remove_tentative_number(
    const Lock& lock,
    const TransactionNumber& number) -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = tentative_transaction_numbers_.erase(number);

    return (0 < output);
}

auto Server::RemoveTentativeNumber(const TransactionNumber& number) -> bool
{
    Lock lock(lock_);

    return remove_tentative_number(lock, number);
}

void Server::ResetThread() { Join(); }

void Server::resolve_queue(
    const Lock& contextLock,
    DeliveryResult&& result,
    const PasswordPrompt& reason,
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
    const auto saved = save(contextLock, reason);

    OT_ASSERT(saved);
}

auto Server::Resync(const proto::Context& serialized) -> bool
{
    Lock lock(lock_);

    return resync(lock, serialized);
}

auto Server::resync(const Lock& lock, const proto::Context& serialized) -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    TransactionNumbers serverNumbers{};

    for (const auto& number : serialized.issuedtransactionnumber()) {
        serverNumbers.insert(number);
        auto exists = (1 == issued_transaction_numbers_.count(number));

        if (false == exists) {
            LogError()(OT_PRETTY_CLASS())("Server believes number ")(
                number)(" is still issued. "
                        "Restoring.")
                .Flush();
            issued_transaction_numbers_.insert(number);
            available_transaction_numbers_.insert(number);
        }
    }

    for (const auto& number : issued_transaction_numbers_) {
        auto exists = (1 == serverNumbers.count(number));

        if (false == exists) {
            LogError()(OT_PRETTY_CLASS())("Server believes number ")(
                number)(" is no longer issued. "
                        "Removing.")
                .Flush();
            issued_transaction_numbers_.erase(number);
            available_transaction_numbers_.erase(number);
        }
    }

    TransactionNumbers notUsed{};
    update_highest(lock, issued_transaction_numbers_, notUsed, notUsed);

    return true;
}

auto Server::Revision() const -> std::uint64_t { return revision_.load(); }

void Server::update_state(
    const Lock& contextLock,
    const proto::DeliveryState state,
    const PasswordPrompt& reason,
    const otx::LastReplyStatus status)
{
    state_.store(state);

    if (otx::LastReplyStatus::Invalid != status) { last_status_.store(status); }

    const auto saved = save(contextLock, reason);

    OT_ASSERT(saved);
}

void Server::scan_number_set(
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

auto Server::SendMessage(
    const api::session::Client& client,
    const UnallocatedSet<otx::context::ManagedNumber>& pending,
    otx::context::Server& context,
    const Message& message,
    const PasswordPrompt& reason,
    const UnallocatedCString& label,
    const bool resync) -> client::NetworkReplyMessage
{
    Lock lock(lock_);
    pending_args_ = {label, resync};
    request_sent_.Send([&] {
        auto out = network::zeromq::Message{};
        out.AddFrame(message.command_->Get());

        return out;
    }());
    auto result = context.Connection().Send(context, message, reason);

    if (client::SendResult::VALID_REPLY == result.first) {
        process_reply(lock, client, pending, *result.second, reason);
        reply_received_.Send([&] {
            auto out = network::zeromq::Message{};
            out.AddFrame(message.command_->Get());

            return out;
        }());
    }

    return result;
}

auto Server::serialize(const Lock& lock) const -> proto::Context
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
        server.set_laststatus(translate(last_status_.load()));

        if (pending_message_) {
            auto& pending = *server.mutable_pending();
            pending.set_version(pending_command_version_);
            pending.set_serialized(String::Factory(*pending_message_)->Get());
            pending.set_accountlabel(std::get<0>(pending_args_));
            pending.set_resync(std::get<1>(pending_args_));
        }
    }

    return output;
}

auto Server::server_nym_id(const Lock& lock) const -> const identifier::Nym&
{
    OT_ASSERT(remote_nym_);

    return remote_nym_->ID();
}

void Server::SetAdminAttempted()
{
    Lock lock(lock_);
    admin_attempted_->On();
}

void Server::SetAdminPassword(const UnallocatedCString& password)
{
    Lock lock(lock_);
    admin_password_ = password;
}

void Server::SetAdminSuccess()
{
    Lock lock(lock_);
    admin_attempted_->On();
    admin_success_->On();
}

auto Server::SetHighest(const TransactionNumber& highest) -> bool
{
    Lock lock(lock_);

    if (highest >= highest_transaction_number_.load()) {
        highest_transaction_number_.store(highest);

        return true;
    }

    return false;
}

void Server::SetRevision(const std::uint64_t revision)
{
    Lock lock(lock_);
    revision_.store(revision);
}

auto Server::StaleNym() const -> bool
{
    Lock lock(lock_);

    OT_ASSERT(nym_);

    return revision_.load() < nym_->Revision();
}

auto Server::Statement(const OTTransaction& owner, const PasswordPrompt& reason)
    const -> std::unique_ptr<Item>
{
    const TransactionNumbers empty;

    return Statement(owner, empty, reason);
}

auto Server::statement(
    const Lock& lock,
    const OTTransaction& transaction,
    const TransactionNumbers& adding,
    const PasswordPrompt& reason) const -> std::unique_ptr<Item>
{
    OT_ASSERT(verify_write_lock(lock));

    std::unique_ptr<Item> output;

    OT_ASSERT(nym_);

    if ((transaction.GetNymID() != nym_->ID())) {
        LogError()(OT_PRETTY_CLASS())("Transaction has wrong owner.").Flush();

        return output;
    }

    // transaction is the depositPaymentPlan, activateSmartContract, or
    // marketOffer that triggered the need for this transaction statement.
    // Since it uses up a transaction number, I will be sure to remove that one
    // from my list before signing the list.
    output = api_.Factory().InternalSession().Item(
        transaction, itemType::transactionStatement, identifier::Generic{});

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
    output->SignContract(*nym_, reason);
    // OTTransactionType needs to weasel in a "date signed" variable.
    output->SaveContract();

    return output;
}

auto Server::Statement(
    const OTTransaction& transaction,
    const TransactionNumbers& adding,
    const PasswordPrompt& reason) const -> std::unique_ptr<Item>
{
    Lock lock(lock_);

    return statement(lock, transaction, adding, reason);
}

auto Server::Statement(
    const TransactionNumbers& adding,
    const TransactionNumbers& without,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<otx::context::TransactionStatement>
{
    Lock lock(lock_);

    return generate_statement(lock, adding, without);
}

auto Server::ShouldRename(const UnallocatedCString& defaultName) const -> bool
{
    try {
        const auto contract = api_.Wallet().Internal().Server(server_id_);

        if (contract->Alias() != contract->EffectiveName()) { return true; }

        return defaultName == contract->EffectiveName();
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Missing server contract.").Flush();

        return false;
    }
}

auto Server::start(
    const Lock& decisionLock,
    const PasswordPrompt& reason,
    const api::session::Client& client,
    std::shared_ptr<Message> message,
    const ExtraArgs& args,
    const proto::DeliveryState state,
    const ActionType type,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox,
    UnallocatedSet<otx::context::ManagedNumber>* numbers) -> Server::QueueResult
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
    update_state(contextLock, state, reason);

    if (trigger(decisionLock)) {
        return std::make_unique<SendFuture>(pending_result_.get_future());
    }

    LogError()(OT_PRETTY_CLASS())("Failed to queue task").Flush();

    return {};
}

auto Server::state_machine() noexcept -> bool
{
    const auto* pClient = client_.load();

    OT_ASSERT(nullptr != pClient);

    auto reason = api_.Factory().PasswordPrompt("Sending server message");
    const auto& client = *pClient;

    switch (state_.load()) {
        case proto::DELIVERTYSTATE_PENDINGSEND: {
            LogDetail()(OT_PRETTY_CLASS())("Attempting to send message")
                .Flush();
            pending_send(client, reason);
        } break;
        case proto::DELIVERTYSTATE_NEEDNYMBOX: {
            LogDetail()(OT_PRETTY_CLASS())("Downloading nymbox").Flush();
            need_nymbox(client, reason);
        } break;
        case proto::DELIVERTYSTATE_NEEDBOXITEMS: {
            LogDetail()(OT_PRETTY_CLASS())("Downloading box items").Flush();
            need_box_items(client, reason);
        } break;
        case proto::DELIVERTYSTATE_NEEDPROCESSNYMBOX: {
            LogDetail()(OT_PRETTY_CLASS())("Processing nymbox").Flush();
            need_process_nymbox(client, reason);
        } break;
        case proto::DELIVERTYSTATE_IDLE:
        case proto::DELIVERTYSTATE_ERROR:
        default: {
            LogAbort()(OT_PRETTY_CLASS())("Unexpected state").Abort();
        }
    }

    const bool more = proto::DELIVERTYSTATE_IDLE != state_.load();
    const bool retry = failure_count_limit_ >= failure_counter_.load();

    if (false == more) {
        LogDetail()(OT_PRETTY_CLASS())("Delivery complete").Flush();
    } else if (false == retry) {
        LogError()(OT_PRETTY_CLASS())("Error limit exceeded").Flush();
        Lock lock(lock_);
        resolve_queue(
            lock,
            DeliveryResult{otx::LastReplyStatus::NotSent, nullptr},
            reason,
            proto::DELIVERTYSTATE_IDLE);
    } else if (shutdown().load()) {
        LogError()(OT_PRETTY_CLASS())("Shutting down").Flush();
        Lock lock(lock_);
        resolve_queue(
            lock,
            DeliveryResult{otx::LastReplyStatus::NotSent, nullptr},
            reason,
            proto::DELIVERTYSTATE_IDLE);
    } else {
        LogDetail()(OT_PRETTY_CLASS())("Continuing").Flush();

        return true;
    }

    return false;
}

auto Server::Type() const -> otx::ConsensusType
{
    return otx::ConsensusType::Server;
}

auto Server::update_highest(
    const Lock& lock,
    const TransactionNumbers& numbers,
    TransactionNumbers& good,
    TransactionNumbers& bad) -> TransactionNumber
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
            LogVerbose()(OT_PRETTY_CLASS())(
                "Raising Highest Transaction Number "
                "from ")(oldHighest)(" to ")(highest)(".")
                .Flush();
        } else {
            LogVerbose()(OT_PRETTY_CLASS())(
                "Creating Highest Transaction Number "
                "entry for this server as '")(highest)("'.")
                .Flush();
        }

        highest_transaction_number_.store(highest);
    }

    return output;
}

auto Server::update_remote_hash(const Lock& lock, const Message& reply)
    -> identifier::Generic
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = identifier::Generic{};
    const auto& input = reply.nymbox_hash_;

    if (input->Exists()) {
        output = api_.Factory().IdentifierFromBase58(input->Bytes());
        remote_nymbox_hash_ = output;
    }

    return output;
}

auto Server::update_nymbox_hash(
    const Lock& lock,
    const Message& reply,
    const UpdateHash which) -> bool
{
    if (false == reply.nymbox_hash_->Exists()) {
        LogError()(OT_PRETTY_CLASS())("No hash provided by notary").Flush();

        return false;
    }

    const auto hash =
        api_.Factory().IdentifierFromBase58(reply.nymbox_hash_->Bytes());
    set_remote_nymbox_hash(lock, hash);

    if (UpdateHash::Both == which) { set_local_nymbox_hash(lock, hash); }

    return true;
}

auto Server::update_request_number(
    const PasswordPrompt& reason,
    const Lock& contextLock,
    [[maybe_unused]] const Lock& messageLock,
    bool& sendStatus) -> RequestNumber
{
    sendStatus = false;

    auto request = initialize_server_command(MessageType::getRequestNumber);

    if (false == bool(request)) {
        LogError()(OT_PRETTY_CLASS())("Failed to initialize server message.")
            .Flush();

        return {};
    }

    request->request_num_ =
        String::Factory(std::to_string(FIRST_REQUEST_NUMBER).c_str());

    if (false == finalize_server_command(*request, reason)) {
        LogError()(OT_PRETTY_CLASS())("Failed to finalize server message.")
            .Flush();

        return {};
    }

    const auto push = static_cast<opentxs::network::ServerConnection::Push>(
        enable_otx_push_.load());
    const auto response = connection_.Send(*this, *request, reason, push);
    const auto& status = response.first;
    const auto& reply = response.second;

    switch (status) {
        case client::SendResult::TIMEOUT: {
            LogError()(OT_PRETTY_CLASS())("Reply timeout.").Flush();

            return {};
        }
        case client::SendResult::INVALID_REPLY: {
            sendStatus = true;
            LogError()(OT_PRETTY_CLASS())("Invalid reply.").Flush();

            return {};
        }
        case client::SendResult::VALID_REPLY: {
            sendStatus = true;
        } break;
        case client::SendResult::Error:
        case client::SendResult::TRANSACTION_NUMBERS:
        case client::SendResult::UNNECESSARY:
        case client::SendResult::SHUTDOWN:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unknown error.").Flush();

            return {};
        }
    }

    OT_ASSERT(reply);

    const RequestNumber newNumber = reply->new_request_num_;
    add_acknowledged_number(contextLock, newNumber);
    remove_acknowledged_number(contextLock, *reply);
    request_number_.store(newNumber);
    update_remote_hash(contextLock, *reply);

    return newNumber;
}

auto Server::update_request_number(
    const PasswordPrompt& reason,
    const Lock& lock,
    Message& command) -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    command.request_num_ =
        String::Factory(std::to_string(request_number_++).c_str());

    return finalize_server_command(command, reason);
}

auto Server::UpdateHighest(
    const TransactionNumbers& numbers,
    TransactionNumbers& good,
    TransactionNumbers& bad) -> TransactionNumber
{
    Lock lock(lock_);

    return update_highest(lock, numbers, good, bad);
}

auto Server::UpdateRequestNumber(Message& command, const PasswordPrompt& reason)
    -> bool
{
    Lock lock(lock_);

    return update_request_number(reason, lock, command);
}

void Server::validate_number_set(
    const TransactionNumbers& input,
    const TransactionNumber limit,
    TransactionNumbers& good,
    TransactionNumbers& bad)
{
    for (const auto& it : input) {
        if (it <= limit) {
            LogDetail()(OT_PRETTY_STATIC(Server))(
                "New transaction number is less-than-or-equal-to last known "
                "'highest trans number' record. (Must be seeing the same "
                "server reply for a second time, due to a receipt in my "
                "Nymbox.) FYI, last known 'highest' number received: ")(
                limit)(" (Current 'violator': ")(it)(") Skipping...")
                .Flush();
            bad.insert(it);
        } else {
            good.insert(it);
        }
    }
}

void Server::verify_blank(
    const Lock& lock,
    OTTransaction& blank,
    TransactionNumbers& output) const
{
    for (const auto& number : extract_numbers(blank)) {
        if (verify_issued_number(lock, number)) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Attempted to accept a blank transaction number that I "
                "ALREADY HAD...(Skipping).")
                .Flush();
        } else if (verify_tentative_number(lock, number)) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Attempted to accept a blank transaction number that I "
                "ALREADY ACCEPTED (it's on my tentative list already; "
                "Skipping).")
                .Flush();
        } else if (number <= highest_transaction_number_.load()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Attempted to accept a blank transaction number that I've "
                "HAD BEFORE, or at least, is <= to ones I've had before. "
                "(Skipping...).")
                .Flush();
        } else {
            LogTrace()(OT_PRETTY_CLASS())("Accepting number ")(number).Flush();
            output.insert(number);
        }
    }
}

void Server::verify_success(
    const Lock& lock,
    OTTransaction& blank,
    TransactionNumbers& output) const
{
    for (const auto& number : extract_numbers(blank)) {
        if (false == verify_tentative_number(lock, number)) {
            LogDetail()(OT_PRETTY_CLASS())(
                "transactionType::successNotice: This wasn't on my tentative "
                "list (")(number)("), I must have already processed it. (Or "
                                  "there was a dropped "
                                  "message when I did, or the server is trying "
                                  "to slip me an old "
                                  "number).")
                .Flush();
        } else {
            output.insert(number);
        }
    }
}

auto Server::UpdateRequestNumber(const PasswordPrompt& reason) -> RequestNumber
{
    bool notUsed{false};

    return UpdateRequestNumber(notUsed, reason);
}

auto Server::UpdateRequestNumber(bool& sendStatus, const PasswordPrompt& reason)
    -> RequestNumber
{
    Lock messageLock(message_lock_, std::defer_lock);
    Lock contextLock(lock_, std::defer_lock);
    std::lock(messageLock, contextLock);

    return update_request_number(reason, contextLock, messageLock, sendStatus);
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
auto Server::Verify(const otx::context::TransactionStatement& statement) const
    -> bool
{
    Lock lock(lock_);

    for (const auto& number : issued_transaction_numbers_) {
        const bool missing = (1 != statement.Issued().count(number));

        if (missing) {
            LogConsole()(OT_PRETTY_CLASS())("Issued transaction # ")(
                number)(" on context not found on "
                        "statement.")
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

auto Server::verify_tentative_number(
    const Lock& lock,
    const TransactionNumber& number) const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    return (0 < tentative_transaction_numbers_.count(number));
}

auto Server::VerifyTentativeNumber(const TransactionNumber& number) const
    -> bool
{
    return (0 < tentative_transaction_numbers_.count(number));
}

Server::~Server()
{
    auto reason = api_.Factory().PasswordPrompt("Shutting down server context");
    Stop().get();
    const bool needPromise = (false == pending_result_set_.load()) &&
                             (proto::DELIVERTYSTATE_IDLE != state_.load());

    if (needPromise) {
        Lock lock(lock_);
        resolve_queue(
            lock,
            DeliveryResult{otx::LastReplyStatus::Unknown, nullptr},
            reason,
            proto::DELIVERTYSTATE_IDLE);
    }
}
}  // namespace opentxs::otx::context::implementation

#undef START_SERVER_CONTEXT
