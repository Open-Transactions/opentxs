// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/consensus/Server.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/AsymmetricKey.pb.h>
#include <opentxs/protobuf/ConsensusEnums.pb.h>
#include <opentxs/protobuf/Context.pb.h>
#include <opentxs/protobuf/Nym.pb.h>
#include <opentxs/protobuf/OTXEnums.pb.h>
#include <opentxs/protobuf/OTXPush.pb.h>
#include <opentxs/protobuf/PaymentWorkflow.pb.h>
#include <opentxs/protobuf/PendingCommand.pb.h>
#include <opentxs/protobuf/Purse.pb.h>
#include <opentxs/protobuf/ServerContext.pb.h>
#include <opentxs/protobuf/ServerContract.pb.h>
#include <opentxs/protobuf/UnitDefinition.pb.h>
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <functional>
#include <future>
#include <iterator>
#include <optional>
#include <stdexcept>

#include "core/StateMachine.hpp"
#include "internal/api/session/Activity.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/api/session/Types.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/Object.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/network/ServerConnection.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
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
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/WorkType.hpp"  // IWYU pragma: keep
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Paths.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/contract/ContractType.hpp"  // IWYU pragma: keep
#include "opentxs/contract/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/ObjectType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/Types.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/otx/ConsensusType.hpp"                // IWYU pragma: keep
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/PaymentWorkflowType.hpp"   // IWYU pragma: keep
#include "opentxs/otx/client/SendResult.hpp"            // IWYU pragma: keep
#include "opentxs/otx/client/StorageBox.hpp"            // IWYU pragma: keep
#include "opentxs/protobuf/Types.internal.tpp"
#include "opentxs/protobuf/syntax/Context.hpp"
#include "opentxs/protobuf/syntax/Purse.hpp"
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Writer.hpp"
#include "otx/common/OTStorage.hpp"
#include "otx/consensus/Base.hpp"

#define START_SERVER_CONTEXT()                                                 \
    auto handle = get_data();                                                  \
    auto& data = *handle;                                                      \
    auto decisionLock = Lock{decision_lock_};                                  \
                                                                               \
    if (running().load()) {                                                    \
        LogDebug()()("State machine is already running.").Flush();             \
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
    const protobuf::Context& serialized,
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
const UnallocatedSet<otx::MessageType> Server::do_not_need_request_number_{
    otx::MessageType::pingNotary,
    otx::MessageType::registerNym,
    otx::MessageType::getRequestNumber,
};

Server::Server(
    const api::session::Client& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const Nym_p& local,
    const Nym_p& remote,
    const identifier::Notary& server,
    network::ServerConnection& connection)
    : Base<Server, ServerPrivate>(api, current_version_, local, remote, server)
    , StateMachine([this] { return state_machine(*get_data()); })
    , data_(api, current_version_, requestSent, replyReceived, connection)
{
    init_sockets(*get_data());
}

Server::Server(
    const api::session::Client& api,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const protobuf::Context& serialized,
    const Nym_p& local,
    const Nym_p& remote,
    network::ServerConnection& connection)
    : Base<Server, ServerPrivate>(
          api,
          current_version_,
          serialized,
          local,
          remote,
          api.Factory().Internal().NotaryID(
              serialized.servercontext().serverid()))
    , StateMachine([this] { return state_machine(*get_data()); })
    , data_(
          api,
          current_version_,
          requestSent,
          replyReceived,
          serialized,
          connection)
{
    init_serialized();
    init_sockets(*get_data(false));
}

auto Server::accept_entire_nymbox(
    Data& data,
    const api::session::Client& client,
    Ledger& nymbox,
    Message& output,
    ReplyNoticeOutcomes& notices,
    std::size_t& alreadySeenNotices,
    const PasswordPrompt& reason) -> bool
{
    alreadySeenNotices = 0;
    const auto& nym = *Signer();
    const auto& nymID = nym.ID();

    if (nymbox.GetTransactionCount() < 1) {
        LogTrace()()("Nymbox is empty.").Flush();

        return false;
    }

    if (false == nymbox.VerifyAccount(nym)) {
        LogError()()("Invalid nymbox").Flush();

        return false;
    }

    if (nymbox.GetNymID() != nymID) {
        LogError()()("Wrong nymbox").Flush();

        return false;
    }

    const TransactionNumber lStoredTransactionNumber{0};
    auto processLedger =
        api_.Factory().Internal().Session().Ledger(nymID, nymID, server_id_);

    assert_false(nullptr == processLedger);

    processLedger->GenerateLedger(nymID, server_id_, otx::ledgerType::message);
    const std::shared_ptr<OTTransaction> acceptTransaction{
        api_.Factory().Internal().Session().Transaction(
            nymID,
            nym_to_account(nymID),
            server_id_,
            otx::transactionType::processNymbox,
            otx::originType::not_applicable,
            lStoredTransactionNumber)};

    assert_false(nullptr == acceptTransaction);

    processLedger->AddTransaction(acceptTransaction);
    TransactionNumbers verifiedNumbers{};
    TransactionNumbers setNoticeNumbers;

    for (const auto& it : nymbox.GetTransactionMap()) {
        assert_false(nullptr == it.second);

        auto& transaction = *it.second;

        if (transaction.IsAbbreviated() &&
            (transaction.GetType() != otx::transactionType::replyNotice)) {
            LogError()()(
                "Error: Unexpected abbreviated receipt in Nymbox, even after "
                "supposedly loading all box receipts. (And it's not a "
                "replyNotice, either)!")
                .Flush();

            continue;
        }

        auto strRespTo = String::Factory();
        transaction.GetReferenceString(strRespTo);

        switch (transaction.GetType()) {
            case otx::transactionType::message: {
                make_accept_item(
                    reason,
                    otx::itemType::acceptMessage,
                    transaction,
                    *acceptTransaction);
            } break;
            case otx::transactionType::instrumentNotice: {
                make_accept_item(
                    reason,
                    otx::itemType::acceptNotice,
                    transaction,
                    *acceptTransaction);
            } break;
            case otx::transactionType::notice: {
                make_accept_item(
                    reason,
                    otx::itemType::acceptNotice,
                    transaction,
                    *acceptTransaction);
            } break;
            case otx::transactionType::successNotice: {
                verify_success(data, transaction, setNoticeNumbers);
                make_accept_item(
                    reason,
                    otx::itemType::acceptNotice,
                    transaction,
                    *acceptTransaction);
            } break;
            case otx::transactionType::replyNotice: {
                const bool seen = verify_acknowledged_number(
                    data, transaction.GetRequestNum());

                if (seen) {
                    ++alreadySeenNotices;
                } else {
                    auto item = transaction.GetItem(otx::itemType::replyNotice);

                    if (item) {
                        process_unseen_reply(
                            data, client, *item, notices, reason);
                    } else {
                        LogConsole()()("Missing reply notice item").Flush();
                    }

                    make_accept_item(
                        reason,
                        otx::itemType::acceptNotice,
                        transaction,
                        *acceptTransaction);
                }
            } break;
            case otx::transactionType::blank: {
                verify_blank(data, transaction, verifiedNumbers);
                make_accept_item(
                    reason,
                    otx::itemType::acceptTransaction,
                    transaction,
                    *acceptTransaction,
                    verifiedNumbers);
            } break;
            case otx::transactionType::finalReceipt: {
                const auto number = transaction.GetReferenceToNum();
                const bool removed = consume_issued(data, number);

                if (removed) {
                    LogDetail()()(
                        "**** Due to finding a finalReceipt, consuming Nym's "
                        "issued opening number: ")(number)
                        .Flush();
                } else {
                    LogDetail()()(
                        "**** Noticed a finalReceipt, but Opening Number")(
                        number)(" had ALREADY been consumed from nym.")
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
                    otx::itemType::acceptFinalReceipt,
                    transaction,
                    *acceptTransaction);
            } break;
            default: {
                LogError()()("Not accepting item ")(
                    transaction.GetTransactionNum())(", type ")(
                    transaction.GetTypeString())
                    .Flush();
            }
        }
    }

    const auto acceptedItems = acceptTransaction->GetItemCount();

    if ((1 > acceptedItems) && (1 > alreadySeenNotices)) {
        LogDetail()()("Nothing to accept.").Flush();

        return false;
    }

    for (const auto& number : setNoticeNumbers) {
        accept_issued_number(data, number);
    }

    auto ready{true};
    nymbox.ReleaseTransactions();

    for (const auto& number : verifiedNumbers) {
        add_tentative_number(data, number);
    }

    const std::shared_ptr<Item> balanceItem{
        statement(data, *acceptTransaction, verifiedNumbers, reason)};

    assert_false(nullptr == balanceItem);

    acceptTransaction->AddItem(balanceItem);

    assert_true((acceptedItems + 1) == acceptTransaction->GetItemCount());

    ready &= acceptTransaction->SignContract(nym, reason);

    assert_true(ready);

    ready &= acceptTransaction->SaveContract();

    assert_true(ready);

    ready &= processLedger->SignContract(nym, reason);

    assert_true(ready);

    ready &= processLedger->SaveContract();

    assert_true(ready);

    const auto serialized = String::Factory(*processLedger);
    initialize_server_command(
        data, otx::MessageType::processNymbox, -1, true, true, output);
    ready &= output.payload_->SetString(serialized);
    finalize_server_command(output, reason);

    assert_true(ready);

    return true;
}

auto Server::accept_numbers(
    Data& data,
    OTTransaction& transaction,
    OTTransaction& replyTransaction) -> void
{
    auto item = transaction.GetItem(otx::itemType::transactionStatement);

    if (false == bool(item)) {
        LogConsole()()(
            "Strange... found transaction in ledger , but didn't find a "
            "transactionStatement item within.")
            .Flush();

        return;
    }

    if (false == replyTransaction.GetSuccess()) {
        LogConsole()()(
            "Found the receipt you're talking about, but the Server's Reply "
            "transaction says ")("FAILED.")
            .Flush();

        return;
    }

    auto serialized = String::Factory();
    item->GetAttachment(serialized);

    if (false == serialized->Exists()) {
        LogConsole()()(
            "Strange... found transaction item in ledger, but didn't find "
            "statement within.")
            .Flush();

        return;
    }

    const otx::context::TransactionStatement statement(api_, serialized);
    accept_issued_number(data, statement);
}

auto Server::AcceptIssuedNumber(const TransactionNumber& number) -> bool
{
    return accept_issued_number(*get_data(), number);
}

auto Server::accept_issued_number(Data& data, const TransactionNumber& number)
    -> bool
{
    bool accepted = false;
    const bool tentative = remove_tentative_number(data, number);

    if (tentative) { accepted = issue_number(data, number); }

    return accepted;
}

auto Server::AcceptIssuedNumbers(
    const otx::context::TransactionStatement& statement) -> bool
{
    return accept_issued_number(*get_data(), statement);
}

auto Server::accept_issued_number(
    Data& data,
    const otx::context::TransactionStatement& statement) -> bool
{
    std::size_t added = 0;
    const auto offered = statement.Issued().size();

    if (0 == offered) { return false; }

    TransactionNumbers adding, accepted, rejected;

    for (const auto& number : statement.Issued()) {
        // If number wasn't already on issued list, then add to BOTH
        // lists. Otherwise do nothing (it's already on the issued list,
        // and no longer valid on the available list--thus shouldn't be
        // re-added thereanyway.)
        const bool tentative = verify_tentative_number(data, number);
        const bool issued = verify_issued_number(data, number);

        if (tentative && !issued) { adding.insert(number); }
    }

    // Looks like we found some numbers to accept (tentative numbers we had
    // already been waiting for, yet hadn't processed onto our issued list yet)
    if (!adding.empty()) {
        update_highest(data, adding, accepted, rejected);

        // We only remove-tentative-num/add-transaction-num for the numbers
        // that were above our 'last highest number'. The contents of rejected
        // are thus ignored for these purposes.
        for (const auto& number : accepted) {
            data.tentative_transaction_numbers_.erase(number);

            if (issue_number(data, number)) { added++; }
        }
    }

    return (added == offered);
}

auto Server::Accounts() const -> UnallocatedVector<identifier::Generic>
{
    UnallocatedVector<identifier::Generic> output{};
    const auto serverSet =
        api_.Storage().Internal().AccountsByServer(server_id_);
    const auto nymSet =
        api_.Storage().Internal().AccountsByOwner(Signer()->ID());
    std::ranges::set_intersection(
        serverSet, nymSet, std::back_inserter(output));

    return output;
}

auto Server::add_item_to_payment_inbox(
    const TransactionNumber number,
    const UnallocatedCString& payment,
    const PasswordPrompt& reason) const -> bool
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    auto paymentInbox = load_or_create_payment_inbox(reason);

    if (false == bool(paymentInbox)) { return false; }

    // I create the client-side-created instrumentNotice using the same
    // transaction number that was already on the box receipt where it came
    // from. Meaning the server already placed an
    // "otx::transactionType::message" in my Nymbox with Txn # X, so I will
    // create the corresponding instrumentNotice for my Payments Inbox using Txn
    // # X as well. After all, if the notary had created it (as normally
    // happens) then that's the Txn# that would have been on it anyway.
    const std::shared_ptr<OTTransaction> transaction{
        api_.Factory().Internal().Session().Transaction(
            *paymentInbox,
            otx::transactionType::instrumentNotice,
            otx::originType::not_applicable,
            number)};

    assert_false(nullptr == transaction);

    transaction->SetReferenceToNum(number);
    transaction->SetReferenceString(String::Factory(payment));
    transaction->SignContract(nym, reason);
    transaction->SaveContract();
    add_transaction_to_ledger(number, transaction, *paymentInbox, reason);

    return true;
}

auto Server::add_item_to_workflow(
    const Data& data,
    const api::session::Client& client,
    const Message& transportItem,
    const UnallocatedCString& item,
    const PasswordPrompt& reason) const -> bool
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    auto message = api_.Factory().Internal().Session().Message();

    assert_false(nullptr == message);

    const auto loaded =
        message->LoadContractFromString(String::Factory(item.c_str()));

    if (false == loaded) {
        LogError()()("Failed to instantiate message.").Flush();

        return false;
    }

    auto plaintext = String::Factory();

    try {
        auto envelope =
            api_.Factory().Internal().Session().Envelope(message->payload_);
        const auto decrypted =
            envelope->Open(nym, plaintext->WriteInto(), reason);

        if (false == decrypted) {
            LogError()()("Failed to decrypt message.").Flush();

            return false;
        }
    } catch (...) {
        LogError()()("Failed to decode message.").Flush();

        return false;
    }

    auto payment = api_.Factory().Internal().Session().Payment(plaintext);

    if (false == bool(payment)) {
        LogError()()("Invalid payment").Flush();

        return false;
    }

    if (false == payment->IsCheque()) { return false; }

    if (payment->IsCancelledCheque(reason)) { return false; }

    auto pCheque = api_.Factory().Internal().Session().Cheque();

    assert_false(nullptr == pCheque);

    auto& cheque = *pCheque;
    cheque.LoadContractFromString(payment->Payment());

    // The sender nym and notary of the cheque may not match the sender nym and
    // notary of the message which conveyed the cheque.
    {
        data.find_nym_->Send([&] {
            auto work =
                network::zeromq::tagged_message(WorkType::OTXSearchNym, true);
            work.AddFrame(cheque.GetSenderNymID());

            return work;
        }());
    }
    {
        data.find_server_->Send([&] {
            auto work = network::zeromq::tagged_message(
                WorkType::OTXSearchServer, true);
            work.AddFrame(cheque.GetNotaryID());

            return work;
        }());
    }
    {
        data.find_unit_definition_->Send([&] {
            auto work =
                network::zeromq::tagged_message(WorkType::OTXSearchUnit, true);
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
        LogError()()("Failed to create workflow.").Flush();

        return false;
    } else {
        LogVerbose()()("Started workflow ")(workflow, api_.Crypto()).Flush();
    }

    return true;
}

auto Server::AddTentativeNumber(const TransactionNumber& number) -> bool
{
    return add_tentative_number(*get_data(), number);
}

auto Server::add_tentative_number(Data& data, const TransactionNumber& number)
    -> bool
{
    if (number < data.highest_transaction_number_.load()) { return false; }

    auto output = data.tentative_transaction_numbers_.insert(number);

    return output.second;
}

auto Server::add_transaction_to_ledger(
    const TransactionNumber number,
    std::shared_ptr<OTTransaction> transaction,
    Ledger& ledger,
    const PasswordPrompt& reason) const -> bool
{
    assert_false(nullptr == Signer());

    if (nullptr != ledger.GetTransaction(number)) {
        LogTrace()()("Transaction already exists").Flush();

        return true;
    }

    if (false == ledger.AddTransaction(transaction)) {
        LogError()()("Failed to add transaction to ledger").Flush();

        return false;
    }

    ledger.ReleaseSignatures();
    ledger.SignContract(*Signer(), reason);
    ledger.SaveContract();
    bool saved{false};

    switch (ledger.GetType()) {
        case otx::ledgerType::paymentInbox: {
            saved = ledger.SavePaymentInbox();
        } break;
        case otx::ledgerType::recordBox: {
            saved = ledger.SaveRecordBox();
        } break;
        case otx::ledgerType::expiredBox: {
            saved = ledger.SaveExpiredBox();
        } break;
        default: {
            LogError()()("Unexpected ledger type: ")(ledger.GetTypeString())
                .Flush();

            return false;
        }
    }

    if (false == saved) {
        LogError()()("Failed to save ")(ledger.GetTypeString()).Flush();

        return false;
    }

    if (false == transaction->SaveBoxReceipt(ledger)) {
        LogError()()("Failed to save box receipt").Flush();

        return false;
    }

    return true;
}

auto Server::AdminAttempted() const -> bool
{
    return get_data()->admin_attempted_.get();
}

auto Server::AdminPassword() const -> const UnallocatedCString&
{
    return get_data()->admin_password_;
}

auto Server::attempt_delivery(
    Data& data,
    const Lock& messageLock,
    const api::session::Client& client,
    Message& message,
    const PasswordPrompt& reason) -> client::NetworkReplyMessage
{
    data.request_sent_.Send([&] {
        auto out = network::zeromq::Message{};
        out.AddFrame(message.command_->Get());

        return out;
    }());
    auto output = data.connection_.Send(
        *this,
        data,
        message,
        reason,
        static_cast<opentxs::network::ServerConnection::Push>(
            data.enable_otx_push_.load()));
    auto& [status, reply] = output;
    const auto needRequestNumber =
        need_request_number(Message::Type(message.command_->Get()));

    switch (status) {
        case client::SendResult::VALID_REPLY: {
            assert_false(nullptr == reply);

            data.reply_received_.Send([&] {
                auto out = network::zeromq::Message{};
                out.AddFrame(message.command_->Get());

                return out;
            }());
            static UnallocatedSet<otx::context::ManagedNumber> empty{};
            auto* numbers = data.numbers_;

            if (nullptr == numbers) { numbers = &empty; }

            assert_false(nullptr == numbers);

            process_reply(data, client, *numbers, *reply, reason);

            if (reply->success_) {
                LogVerbose()()("Success delivering ")(message.command_.get())
                    .Flush();

                return output;
            }

            if (false == needRequestNumber) { break; }

            bool sent{false};
            auto number =
                update_request_number(data, reason, messageLock, sent);

            if ((0 == number) || (false == sent)) {
                LogError()()("Unable to resync request number").Flush();
                status = client::SendResult::TIMEOUT;
                reply.reset();

                return output;
            } else {
                LogVerbose()()("Success resyncing request number ")(
                    message.command_.get())
                    .Flush();
            }

            const auto updated = update_request_number(data, reason, message);

            if (false == updated) {
                LogError()()("Unable to update ")(message.command_.get())(
                    " with new request number")
                    .Flush();
                status = client::SendResult::TIMEOUT;
                reply.reset();
                ++data.failure_counter_;

                return output;
            } else {
                LogVerbose()()("Success updating request number on ")(
                    message.command_.get())
                    .Flush();
            }

            output = data.connection_.Send(
                *this,
                data,
                message,
                reason,
                static_cast<opentxs::network::ServerConnection::Push>(
                    data.enable_otx_push_.load()));

            if (client::SendResult::VALID_REPLY == status) {
                LogVerbose()()("Success delivering ")(message.command_.get())(
                    " (second attempt)")
                    .Flush();
                process_reply(data, client, {}, *reply, reason);

                return output;
            }
        } break;
        case client::SendResult::TIMEOUT: {
            LogError()()("Timeout delivering ")(message.command_.get()).Flush();
            ++data.failure_counter_;
        } break;
        case client::SendResult::INVALID_REPLY: {
            LogError()()("Invalid reply to ")(message.command_.get()).Flush();
            ++data.failure_counter_;
        } break;
        case client::SendResult::Error: {
            LogError()()("Malformed ")(message.command_.get()).Flush();
            ++data.failure_counter_;
        } break;
        case client::SendResult::SHUTDOWN:
        case client::SendResult::TRANSACTION_NUMBERS:
        case client::SendResult::UNNECESSARY:
        default: {
            LogError()()("Unknown error").Flush();
            ++data.failure_counter_;
        }
    }

    ++data.failure_counter_;

    return output;
}

auto Server::client_nym_id() const -> const identifier::Nym&
{
    assert_false(nullptr == Signer());

    return Signer()->ID();
}

auto Server::Connection() -> network::ServerConnection&
{
    return get_data()->connection_;
}

auto Server::create_instrument_notice_from_peer_object(
    const Data& data,
    const api::session::Client& client,
    const Message& message,
    const PeerObject& peerObject,
    const TransactionNumber number,
    const PasswordPrompt& reason) const -> bool
{
    assert_true(contract::peer::ObjectType::Payment == peerObject.Type());

    if (false == peerObject.Validate()) {
        LogError()()("Invalid peer object.").Flush();

        return false;
    }

    const auto& payment = *peerObject.Payment();

    if (payment.empty()) {
        LogError()()(
            "Payment as received was apparently empty. Maybe the sender sent "
            "it that way?")
            .Flush();

        return false;
    }

    // Extract the OTPayment so that we know whether to use the new Workflow
    // code or the old payment inbox code
    if (add_item_to_workflow(data, client, message, payment, reason)) {
        return true;
    } else {
        return add_item_to_payment_inbox(number, payment, reason);
    }
}

auto Server::extract_box_receipt(
    const String& serialized,
    const identity::Nym& signer,
    const identifier::Nym& owner,
    const TransactionNumber target) const -> std::shared_ptr<OTTransaction>
{
    if (false == serialized.Exists()) {
        LogError()()("Invalid input").Flush();

        return {};
    }

    const std::shared_ptr<OTTransactionType> transaction{
        api_.Factory().Internal().Session().Transaction(serialized)};

    if (false == bool(transaction)) {
        LogError()()("Failed to instantiate transaction").Flush();

        return {};
    }

    auto receipt = std::dynamic_pointer_cast<OTTransaction>(transaction);

    if (false == bool(receipt)) {
        LogError()()("Invalid transaction type").Flush();

        return {};
    }

    if (false == receipt->VerifyAccount(signer)) {
        LogError()()("Invalid receipt").Flush();

        return {};
    }

    if ((0 != target) && (receipt->GetTransactionNum() != target)) {
        LogError()()("Incorrect transaction number").Flush();

        return {};
    }

    if (receipt->GetNymID() != owner) {
        LogError()()("Invalid nym").Flush();

        return {};
    }

    return receipt;
}

auto Server::extract_ledger(
    const Armored& armored,
    const identifier::Account& accountID,
    const identity::Nym& signer) const -> std::unique_ptr<Ledger>
{
    assert_false(nullptr == Signer());

    if (false == armored.Exists()) {
        LogConsole()()("Error: empty input").Flush();

        return {};
    }

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    auto output = api_.Factory().Internal().Session().Ledger(
        nymID, accountID, server_id_);

    assert_false(nullptr == output);

    auto serialized = String::Factory();
    armored.GetString(serialized);

    if (false == output->LoadLedgerFromString(serialized)) {
        LogConsole()()("Error: Failed to instantiate ledger").Flush();

        return {};
    }

    if (false == output->VerifySignature(signer)) {
        LogConsole()()("Error: Invalid signature").Flush();

        return {};
    }

    return output;
}

auto Server::extract_message(
    const Armored& armored,
    const identity::Nym& signer) const -> std::unique_ptr<Message>
{
    auto output = api_.Factory().Internal().Session().Message();

    assert_false(nullptr == output);

    if (false == armored.Exists()) {
        LogConsole()()("Error: empty input").Flush();

        return {};
    }

    auto serialized = String::Factory();
    armored.GetString(serialized);

    if (false == output->LoadContractFromString(serialized)) {
        LogConsole()()("Error: Failed to instantiate message").Flush();

        return {};
    }

    if (false == output->VerifySignature(signer)) {
        LogConsole()()("Error: Invalid signature").Flush();

        return {};
    }

    return output;
}

auto Server::extract_numbers(OTTransaction& input) -> Server::TransactionNumbers
{
    auto output = TransactionNumbers{};
    auto list = NumList{};
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
        LogError()()("Input item does not contain reference string").Flush();

        return {};
    }

    auto transaction =
        api_.Factory().Internal().Session().Transaction(serialized);

    if (false == bool(transaction)) {
        LogError()()("Unable to instantiate serialized item").Flush();

        return {};
    }

    std::unique_ptr<Item> output{dynamic_cast<Item*>(transaction.get())};

    if (output) {
        transaction.release();
    } else {
        LogError()()("Reference string is not a serialized item").Flush();
    }

    return output;
}

auto Server::extract_payment_instrument_from_notice(
    const api::Session& api,
    const identity::Nym& theNym,
    std::shared_ptr<OTTransaction> pTransaction,
    const PasswordPrompt& reason) const -> std::shared_ptr<OTPayment>
{
    const bool bValidNotice =
        (otx::transactionType::instrumentNotice == pTransaction->GetType()) ||
        (otx::transactionType::payDividend == pTransaction->GetType()) ||
        (otx::transactionType::notice == pTransaction->GetType());
    OT_NEW_ASSERT_MSG(
        bValidNotice, "Invalid receipt type passed to this function.");
    // ----------------------------------------------------------------
    if ((otx::transactionType::instrumentNotice ==
         pTransaction->GetType()) ||  // It's encrypted.
        (otx::transactionType::payDividend == pTransaction->GetType())) {
        auto strMsg = String::Factory();
        pTransaction->GetReferenceString(strMsg);

        if (!strMsg->Exists()) {
            LogConsole()()(
                "Failure: Expected OTTransaction::instrumentNotice to "
                "contain an 'in reference to' string, but it was empty. "
                "(Returning).")
                .Flush();
            return nullptr;
        }
        // --------------------
        auto pMsg{api.Factory().Internal().Session().Message()};
        if (false == bool(pMsg)) {
            LogError()()(
                "Null: Assert while allocating memory for an OTMessage!")
                .Flush();
            LogAbort()().Abort();
        }
        if (!pMsg->LoadContractFromString(strMsg)) {
            LogConsole()()("Failed trying to load OTMessage from string: ")(
                strMsg.get())(".")
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
                api_.Factory().Internal().Session().Envelope(pMsg->payload_);
            auto strEnvelopeContents = String::Factory();

            // Decrypt the Envelope.
            if (!theEnvelope->Open(
                    *Signer(), strEnvelopeContents->WriteInto(), reason)) {
                LogConsole()()(
                    "Failed trying to decrypt the financial instrument "
                    "that was supposedly attached as a payload to this "
                    "payment message: ")(strMsg.get())(".")
                    .Flush();
            } else if (!strEnvelopeContents->Exists()) {
                LogConsole()()("Failed: after decryption, cleartext is empty. "
                               "From: ")(strMsg.get())(".")
                    .Flush();
            } else {
                // strEnvelopeContents contains a PURSE or CHEQUE
                // (etc) and not specifically a generic "PAYMENT".
                //
                auto pPayment{api.Factory().Internal().Session().Payment(
                    strEnvelopeContents)};
                if (false == bool(pPayment) || !pPayment->IsValid()) {
                    LogConsole()()(
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
            LogConsole()()("Failed trying to set ASCII-armored data for "
                           "envelope: ")(strMsg.get())(".")
                .Flush();
        }
    } else if (otx::transactionType::notice == pTransaction->GetType()) {
        auto strNotice = String::Factory(*pTransaction);
        auto pPayment{api.Factory().Internal().Session().Payment(strNotice)};

        if (false == bool(pPayment) || !pPayment->IsValid()) {
            LogConsole()()("Failed: the notice is invalid. Contents: ")(
                strNotice.get())(".")
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
    if (otx::transactionType::transferReceipt == receipt.GetType()) {
        return extract_transfer_receipt(receipt);
    } else if (otx::transactionType::pending == receipt.GetType()) {
        return extract_transfer_pending(receipt);
    } else {
        LogError()()("Incorrect receipt type: ")(receipt.GetTypeString())
            .Flush();

        return nullptr;
    }
}

auto Server::extract_transfer_pending(const OTTransaction& receipt) const
    -> std::unique_ptr<Item>
{
    if (otx::transactionType::pending != receipt.GetType()) {
        LogError()()("Incorrect receipt type: ")(receipt.GetTypeString())
            .Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    receipt.GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogError()()("Missing serialized transfer item").Flush();

        return nullptr;
    }

    auto transfer =
        api_.Factory().Internal().Session().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogError()()("Unable to instantiate transfer item").Flush();

        return nullptr;
    }

    if (otx::itemType::transfer != transfer->GetType()) {
        LogError()()("Invalid transfer item type.").Flush();

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
        LogError()()("Missing serialized accept pending item").Flush();

        return nullptr;
    }

    const auto acceptPending =
        api_.Factory().Internal().Session().Item(serializedAcceptPending);

    if (false == bool(acceptPending)) {
        LogError()()("Unable to instantiate accept pending item").Flush();

        return nullptr;
    }

    if (otx::itemType::acceptPending != acceptPending->GetType()) {
        LogError()()("Invalid accept pending item type.").Flush();

        return nullptr;
    }

    auto serializedPending = String::Factory();
    acceptPending->GetAttachment(serializedPending);

    if (serializedPending->empty()) {
        LogError()()("Missing serialized pending transaction").Flush();

        return nullptr;
    }

    auto pending = api_.Factory().Internal().Session().Transaction(
        receipt.GetNymID(),
        receipt.GetRealAccountID(),
        receipt.GetRealNotaryID());

    if (false == bool(pending)) {
        LogError()()("Unable to instantiate pending transaction").Flush();

        return nullptr;
    }

    const bool loaded = pending->LoadContractFromString(serializedPending);

    if (false == loaded) {
        LogError()()("Unable to deserialize pending transaction").Flush();

        return nullptr;
    }

    if (otx::transactionType::pending != pending->GetType()) {
        LogError()()("Invalid pending transaction type.").Flush();

        return nullptr;
    }

    auto serializedTransfer = String::Factory();
    pending->GetReferenceString(serializedTransfer);

    if (serializedTransfer->empty()) {
        LogError()()("Missing serialized transfer item").Flush();

        return nullptr;
    }

    auto transfer =
        api_.Factory().Internal().Session().Item(serializedTransfer);

    if (false == bool(transfer)) {
        LogError()()("Unable to instantiate transfer item").Flush();

        return nullptr;
    }

    if (otx::itemType::transfer != transfer->GetType()) {
        LogError()()("Invalid transfer item type.").Flush();

        return nullptr;
    }

    return transfer;
}

auto Server::finalize_server_command(
    Message& command,
    const PasswordPrompt& reason) const -> bool
{
    assert_false(nullptr == Signer());

    if (false == command.SignContract(*Signer(), reason)) {
        LogError()()("Failed to sign server message.").Flush();

        return false;
    }

    if (false == command.SaveContract()) {
        LogError()()("Failed to serialize server message.").Flush();

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
    const Data& data,
    const TransactionNumbers& adding,
    const TransactionNumbers& without) const
    -> std::unique_ptr<otx::context::TransactionStatement>
{
    TransactionNumbers issued;
    TransactionNumbers available;

    for (const auto& number : data.issued_transaction_numbers_) {
        const bool include = (false == without.contains(number));

        if (include) {
            issued.insert(number);
            available.insert(number);
        }
    }

    for (const auto& number : adding) {
        LogTrace()()("Accepting number ")(number).Flush();
        issued.insert(number);
        available.insert(number);
    }

    std::unique_ptr<otx::context::TransactionStatement> output(
        new otx::context::TransactionStatement(
            api_,
            String::Factory(server_id_, api_.Crypto())->Get(),
            issued,
            available));

    return output;
}

auto Server::get_instrument(
    const api::Session& api,
    const identity::Nym& theNym,
    Ledger& ledger,
    std::shared_ptr<OTTransaction> pTransaction,
    const PasswordPrompt& reason) const -> std::shared_ptr<OTPayment>
{
    assert_true(false != bool(pTransaction));

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
            LogError()()(
                "Good index but uncovered nullptr after trying to load full "
                "version of abbreviated receipt with transaction number: ")(
                lTransactionNum)(".")
                .Flush();
            return nullptr;  // Weird. Clearly I need the full box receipt, if
                             // I'm to get the instrument out of it.
        }
    }
    // ------------------------------------------------------------
    // TO EXTRACT INSTRUMENT FROM PAYMENTS INBOX:
    // -- Iterate through the transactions in the payments inbox.
    // -- (They should all be "instrumentNotice" transactions.)
    // -- Each transaction contains an
    //    OTMessage in the "in ref to" field, which in turn contains
    //        an encrypted OTPayment in the payload field, which contains
    //        the actual financial instrument.
    // -- Therefore, this function, based purely on ledger index (as we
    //        iterate):
    //  1. extracts the OTMessage from the Transaction at each index,
    //     from its "in ref to" field.
    //  2. then decrypts the payload on that message, producing an OTPayment
    // object, 3. ...which contains the actual instrument.

    if ((otx::transactionType::instrumentNotice != pTransaction->GetType()) &&
        (otx::transactionType::payDividend != pTransaction->GetType()) &&
        (otx::transactionType::notice != pTransaction->GetType())) {
        LogConsole()()(
            "Failure: Expected OTTransaction::instrumentNotice, ::payDividend "
            "or ::notice, but found: OTTransaction::")(
            pTransaction->GetTypeString())(".")
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
    const PasswordPrompt& reason) const -> std::shared_ptr<OTPayment>
{
    OT_VERIFY_MIN_BOUND(lReceiptId, 1);

    auto pTransaction = ledger.GetTransaction(lReceiptId);
    if (false == bool(pTransaction)) {
        LogError()()("Supposedly good receipt ID, but uncovered nullptr "
                     "transaction: ")(lReceiptId)(".")
            .Flush();
        return nullptr;  // Weird.
    }
    return get_instrument(api, theNym, ledger, pTransaction, reason);
}

auto Server::get_item_type(OTTransaction& input, otx::itemType& output)
    -> Server::Exit
{
    switch (input.GetType()) {
        case otx::transactionType::atDeposit: {
            output = otx::itemType::atDeposit;
        } break;
        case otx::transactionType::atWithdrawal: {
            auto cash = input.GetItem(otx::itemType::atWithdrawal);
            auto voucher = input.GetItem(otx::itemType::atWithdrawVoucher);

            if (cash) {
                output = otx::itemType::atWithdrawal;
            } else if (voucher) {
                output = otx::itemType::atWithdrawVoucher;
            }
        } break;
        case otx::transactionType::atPayDividend: {
            output = otx::itemType::atPayDividend;
        } break;
        case otx::transactionType::atTransfer: {
            output = otx::itemType::atTransfer;
        } break;
        case otx::transactionType::atMarketOffer: {
            output = otx::itemType::atMarketOffer;
        } break;
        case otx::transactionType::atPaymentPlan: {
            output = otx::itemType::atPaymentPlan;
        } break;
        case otx::transactionType::atSmartContract: {
            output = otx::itemType::atSmartContract;
        } break;
        case otx::transactionType::atCancelCronItem: {
            output = otx::itemType::atCancelCronItem;
        } break;
        case otx::transactionType::atExchangeBasket: {
            output = otx::itemType::atExchangeBasket;
        } break;
        default:
        case otx::transactionType::atProcessInbox: {
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
            LogError()()("Unknown box type: ")(depth).Flush();
        }
    }

    return BoxType::Invalid;
}

auto Server::harvest_unused(Data& data, const api::session::Client& client)
    -> bool
{
    assert_false(nullptr == Signer());

    bool output{true};
    const auto& nymID = Signer()->ID();
    auto available = data.issued_transaction_numbers_;
    const auto workflows =
        client.Storage().Internal().PaymentWorkflowList(nymID);
    UnallocatedSet<client::PaymentWorkflowState> keepStates{};

    // Loop through workflows to determine which issued numbers should not be
    // harvested
    for (const auto& [id, alias] : workflows) {
        const auto workflowID = api_.Factory().IdentifierFromBase58(id);
        auto proto = protobuf::PaymentWorkflow{};

        if (false == client.Workflow().LoadWorkflow(nymID, workflowID, proto)) {
            LogError()()("Failed to load workflow ")(workflowID, api_.Crypto())(
                ".")
                .Flush();

            continue;
        }

        switch (opentxs::translate(proto.type())) {
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
                LogError()()("Warning: Unhandled workflow type.").Flush();
                output &= false;
                continue;
            }
        }

        if (false == keepStates.contains(opentxs::translate(proto.state()))) {
            continue;
        }

        // At this point, this workflow contains a transaction whose
        // number(s) must not be added to the available list (recovered).

        switch (opentxs::translate(proto.type())) {
            case client::PaymentWorkflowType::OutgoingCheque:
            case client::PaymentWorkflowType::OutgoingInvoice: {
                [[maybe_unused]] auto [state, cheque] =
                    api::session::Workflow::InstantiateCheque(api_, proto);

                if (false == bool(cheque)) {
                    LogError()()("Failed to load cheque.").Flush();

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
                    LogError()()("Failed to load transfer.").Flush();

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
                LogError()()("Warning: unhandled workflow type.").Flush();
                output &= false;
                continue;
            }
        }
    }

    // Any numbers which remain in available are not allocated and should
    // be returned to the available list.
    for (const auto& number : available) {
        if (false == verify_available_number(data, number)) {
            LogError()()("Restoring number ")(number)(".").Flush();
            recover_available_number(data, number);
        }
    }

    return output;
}

auto Server::HaveAdminPassword() const -> bool
{
    return false == get_data()->admin_password_.empty();
}

auto Server::HaveSufficientNumbers(const otx::MessageType reason) const -> bool
{
    auto handle = get_data();
    const auto& data = *handle;

    if (otx::MessageType::processInbox == reason) {
        return 0 < data.available_transaction_numbers_.size();
    }

    return 1 < data.available_transaction_numbers_.size();
}

auto Server::Highest() const -> TransactionNumber
{
    return get_data()->highest_transaction_number_.load();
}

auto Server::init_new_account(
    const identifier::Account& accountID,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    auto account = api_.Wallet().Internal().mutable_Account(accountID, reason);

    assert_true(account);

    if (false == account.get().InitBoxes(nym, reason)) {
        LogError()()("Error initializing boxes for account ")(
            accountID, api_.Crypto())(".")
            .Flush();

        return false;
    }

    auto inboxHash = identifier::Generic{};
    auto outboxHash = identifier::Generic{};
    auto haveHash = account.get().GetInboxHash(inboxHash);

    if (false == haveHash) {
        LogError()()("Failed to get inbox hash.").Flush();

        return false;
    }

    haveHash = account.get().GetOutboxHash(outboxHash);

    if (false == haveHash) {
        LogError()()("Failed to get outbox hash.").Flush();

        return false;
    }

    account.Release();
    auto nymfile = mutable_Nymfile(reason);
    auto hashSet = nymfile.get().SetInboxHash(
        accountID.asBase58(api_.Crypto()), inboxHash);

    if (false == hashSet) {
        LogError()()("Failed to set inbox hash on nymfile.").Flush();

        return false;
    }

    hashSet = nymfile.get().SetOutboxHash(
        accountID.asBase58(api_.Crypto()), outboxHash);

    if (false == hashSet) {
        LogError()()("Failed to set outbox hash on nymfile.").Flush();

        return false;
    }

    return true;
}

void Server::init_sockets(Data& data)
{
    auto started = data.find_nym_->Start(api_.Endpoints().FindNym().data());

    if (false == started) {
        LogError()()("Failed to start find nym socket ").Flush();

        LogAbort()().Abort();
    }

    started = data.find_server_->Start(api_.Endpoints().FindServer().data());

    if (false == started) {
        LogError()()("Failed to start find server socket ").Flush();

        LogAbort()().Abort();
    }

    started = data.find_unit_definition_->Start(
        api_.Endpoints().FindUnitDefinition().data());

    if (false == started) {
        LogError()()("Failed to start find unit socket ").Flush();

        LogAbort()().Abort();
    }
}

auto Server::initialize_server_command(const otx::MessageType type) const
    -> std::unique_ptr<Message>
{
    auto output = api_.Factory().Internal().Session().Message();

    assert_false(nullptr == output);

    initialize_server_command(type, *output);

    return output;
}

auto Server::initialize_server_command(
    const otx::MessageType type,
    Message& output) const -> void
{
    assert_false(nullptr == Signer());

    output.ReleaseSignatures();
    output.command_->Set(Message::Command(type).data());
    output.nym_id_ = String::Factory(Signer()->ID(), api_.Crypto());
    output.notary_id_ = String::Factory(server_id_, api_.Crypto());
}

auto Server::initialize_server_command(
    Data& data,
    const otx::MessageType type,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash,
    Message& output) -> RequestNumber
{
    RequestNumber number{0};
    initialize_server_command(type, output);

    if (-1 == provided) {
        number = data.request_number_++;
    } else {
        number = provided;
    }

    output.request_num_ = String::Factory(std::to_string(number).c_str());

    if (withAcknowledgments) {
        output.SetAcknowledgments(data.acknowledged_request_numbers_);
    }

    if (withNymboxHash) {
        data.local_nymbox_hash_.GetString(api_.Crypto(), output.nymbox_hash_);
    }

    return number;
}

auto Server::initialize_server_command(
    Data& data,
    const otx::MessageType type,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash)
    -> std::pair<RequestNumber, std::unique_ptr<Message>>
{
    std::pair<RequestNumber, std::unique_ptr<Message>> output{
        0, api_.Factory().Internal().Session().Message()};
    auto& [requestNumber, message] = output;

    assert_false(nullptr == message);

    requestNumber = initialize_server_command(
        data, type, provided, withAcknowledgments, withNymboxHash, *message);

    return output;
}

auto Server::InitializeServerCommand(
    const otx::MessageType type,
    const Armored& payload,
    const identifier::Account& accountID,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash)
    -> std::pair<RequestNumber, std::unique_ptr<Message>>
{
    auto output = initialize_server_command(
        *get_data(), type, provided, withAcknowledgments, withNymboxHash);
    auto& [requestNumber, message] = output;
    const auto& notUsed [[maybe_unused]] = requestNumber;

    message->payload_ = payload;
    message->acct_id_ = String::Factory(accountID, api_.Crypto());

    return output;
}

auto Server::InitializeServerCommand(
    const otx::MessageType type,
    const identifier::Nym& recipientNymID,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash)
    -> std::pair<RequestNumber, std::unique_ptr<Message>>
{
    auto output = initialize_server_command(
        *get_data(), type, provided, withAcknowledgments, withNymboxHash);
    [[maybe_unused]] auto& [requestNumber, message] = output;
    message->nym_id2_ = String::Factory(recipientNymID, api_.Crypto());

    return output;
}

auto Server::InitializeServerCommand(
    const otx::MessageType type,
    const RequestNumber provided,
    const bool withAcknowledgments,
    const bool withNymboxHash)
    -> std::pair<RequestNumber, std::unique_ptr<Message>>
{
    return initialize_server_command(
        *get_data(), type, provided, withAcknowledgments, withNymboxHash);
}

auto Server::instantiate_message(
    const api::Session& api,
    const UnallocatedCString& serialized) -> std::unique_ptr<opentxs::Message>
{
    if (serialized.empty()) { return {}; }

    auto output = api.Factory().Internal().Session().Message();

    assert_false(nullptr == output);

    const auto loaded =
        output->LoadContractFromString(String::Factory(serialized));

    if (false == loaded) { return {}; }

    return output;
}

auto Server::isAdmin() const -> bool
{
    return get_data()->admin_success_.get();
}

auto Server::is_internal_transfer(const Item& item) const -> bool
{
    if (otx::itemType::transfer != item.GetType()) {
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

    auto sourceOwner = api_.Storage().Internal().AccountOwner(source);
    auto destinationOwner = api_.Storage().Internal().AccountOwner(destination);

    return sourceOwner == destinationOwner;
}

auto Server::Join() const -> void { Wait().get(); }

auto Server::make_accept_item(
    const PasswordPrompt& reason,
    const otx::itemType type,
    const OTTransaction& input,
    OTTransaction& acceptTransaction,
    const TransactionNumbers& accept) const -> const Item&
{
    const std::shared_ptr<Item> acceptItem{
        api_.Factory().Internal().Session().Item(
            acceptTransaction, type, identifier::Account{})};

    assert_false(nullptr == acceptItem);

    acceptTransaction.AddItem(acceptItem);
    acceptItem->SetReferenceToNum(input.GetTransactionNum());

    if (false == accept.empty()) {
        acceptItem->AddBlankNumbersToItem(NumList{accept});
    }

    acceptItem->SignContract(*Signer(), reason);
    acceptItem->SaveContract();

    return *acceptItem;
}

auto Server::load_account_inbox(const identifier::Account& accountID) const
    -> std::unique_ptr<Ledger>
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    auto inbox = api_.Factory().Internal().Session().Ledger(
        nymID, accountID, server_id_);

    assert_false(nullptr == inbox);

    bool output = OTDB::Exists(
        api_,
        api_.DataFolder().string(),
        api_.Internal().Paths().Inbox(),
        server_id_.asBase58(api_.Crypto()).c_str(),
        accountID.asBase58(api_.Crypto()).c_str(),
        "");

    if (output && inbox->LoadInbox()) {
        output = inbox->VerifyAccount(nym);
    } else {
        LogError()()("Failed to load inbox").Flush();
    }

    if (output) { return inbox; }

    LogError()()("Failed to verify inbox").Flush();

    return {};
}

auto Server::load_or_create_account_recordbox(
    const identifier::Account& accountID,
    const PasswordPrompt& reason) const -> std::unique_ptr<Ledger>
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    auto recordBox = api_.Factory().Internal().Session().Ledger(
        nymID, accountID, server_id_);

    assert_false(nullptr == recordBox);

    bool output = OTDB::Exists(
        api_,
        api_.DataFolder().string(),
        api_.Internal().Paths().RecordBox(),
        server_id_.asBase58(api_.Crypto()).c_str(),
        accountID.asBase58(api_.Crypto()).c_str(),
        "");

    if (false == output) {
        LogVerbose()()("Creating recordbox").Flush();
        output = recordBox->GenerateLedger(
            accountID, server_id_, otx::ledgerType::recordBox, true);
        recordBox->ReleaseSignatures();
        output &= recordBox->SignContract(nym, reason);
        output &= recordBox->SaveContract();
        output &= recordBox->SaveRecordBox();
    }

    if (output && recordBox->LoadRecordBox()) {
        output &= recordBox->VerifyContractID();
    } else {
        LogError()()("Failed to load recordbox").Flush();

        return {};
    }

    if (output && recordBox->VerifySignature(nym)) {
        LogTrace()()("Recordbox verified").Flush();

        return recordBox;
    }

    LogError()()("Failed to verify recordbox").Flush();

    return {};
}

auto Server::load_or_create_payment_inbox(const PasswordPrompt& reason) const
    -> std::unique_ptr<Ledger>
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    auto paymentInbox =
        api_.Factory().Internal().Session().Ledger(nymID, nymID, server_id_);

    assert_false(nullptr == paymentInbox);

    bool output = OTDB::Exists(
        api_,
        api_.DataFolder().string(),
        api_.Internal().Paths().PaymentInbox(),
        server_id_.asBase58(api_.Crypto()).c_str(),
        nymID.asBase58(api_.Crypto()).c_str(),
        "");

    if (false == output) {
        LogVerbose()()("Creating payment inbox").Flush();
        output = paymentInbox->GenerateLedger(
            nymID, server_id_, otx::ledgerType::paymentInbox, true);
        paymentInbox->ReleaseSignatures();
        output &= paymentInbox->SignContract(nym, reason);
        output &= paymentInbox->SaveContract();
        output &= paymentInbox->SavePaymentInbox();
    }

    if (output && paymentInbox->LoadPaymentInbox()) {
        output &= paymentInbox->VerifyContractID();
    } else {
        LogError()()("Failed to load payment inbox").Flush();

        return {};
    }

    if (output && paymentInbox->VerifySignature(nym)) {
        LogTrace()()("Payment inbox verified").Flush();

        return paymentInbox;
    }

    LogError()()("Failed to verify payment inbox").Flush();

    return {};
}

auto Server::mutable_Purse(
    const identifier::UnitDefinition& id,
    const PasswordPrompt& reason)
    -> Editor<opentxs::otx::blind::Purse, std::shared_mutex>
{
    return api_.Wallet().Internal().mutable_Purse(
        Signer()->ID(), server_id_, id, reason);
}

void Server::need_box_items(
    Data& data,
    const api::session::Client& client,
    const PasswordPrompt& reason)
{
    const auto messageLock = Lock{data.message_lock_};
    auto nymbox{api_.Factory().Internal().Session().Ledger(
        Signer()->ID(), Signer()->ID(), server_id_, otx::ledgerType::nymbox)};

    assert_false(nullptr == nymbox);

    const auto loaded = nymbox->LoadNymbox();

    if (false == loaded) {
        LogConsole()()("Unable to load nymbox").Flush();

        return;
    }

    const auto verified = nymbox->VerifyAccount(*Signer());

    if (false == verified) {
        LogConsole()()("Unable to verify nymbox").Flush();

        return;
    }

    std::size_t have{0};

    for (const auto& [number, transaction] : nymbox->GetTransactionMap()) {
        if (1 > number) {
            LogConsole()()("Invalid index").Flush();

            continue;
        }

        if (false == bool(transaction)) {
            LogConsole()()("Invalid transaction").Flush();

            continue;
        }

        const auto exists = VerifyBoxReceiptExists(
            api_,
            api_.DataFolder().string(),
            server_id_,
            Signer()->ID(),
            nym_to_account(Signer()->ID()),
            nymbox_box_type_,
            number);

        if (exists) {
            ++have;

            continue;
        }

        [[maybe_unused]] auto [requestNumber, message] =
            initialize_server_command(
                data, otx::MessageType::getBoxReceipt, -1, false, false);

        assert_false(nullptr == message);

        message->acct_id_ = String::Factory(Signer()->ID(), api_.Crypto());
        message->depth_ = nymbox_box_type_;
        message->transaction_num_ = number;
        const auto finalized = FinalizeServerCommand(*message, reason);

        assert_true(finalized);

        auto result =
            attempt_delivery(data, messageLock, client, *message, reason);

        switch (result.first) {
            case client::SendResult::SHUTDOWN: {
                return;
            }
            case client::SendResult::VALID_REPLY: {
                assert_false(nullptr == result.second);

                if (result.second->success_) {
                    ++have;

                    continue;
                } else {
                    // Downloading a box receipt shouldn't fail. If it does, the
                    // only reasonable option is to download the nymbox again.
                    update_state(
                        data, protobuf::DELIVERTYSTATE_NEEDNYMBOX, reason);
                }

                [[fallthrough]];
            }
            case client::SendResult::TIMEOUT:
            case client::SendResult::INVALID_REPLY:
            case client::SendResult::Error:
            case client::SendResult::TRANSACTION_NUMBERS:
            case client::SendResult::UNNECESSARY:
            default: {
                LogError()()("Error downloading box item").Flush();

                return;
            }
        }
    }

    if (nymbox->GetTransactionMap().size() == have) {
        update_state(data, protobuf::DELIVERTYSTATE_NEEDPROCESSNYMBOX, reason);
    }
}

auto Server::need_nymbox(
    Data& data,
    const api::session::Client& client,
    const PasswordPrompt& reason) -> void
{
    const auto messageLock = Lock{data.message_lock_};
    [[maybe_unused]] auto [number, message] = initialize_server_command(
        data, otx::MessageType::getNymbox, -1, true, true);

    assert_false(nullptr == message);

    const auto finalized = FinalizeServerCommand(*message, reason);

    assert_true(finalized);

    auto result = attempt_delivery(data, messageLock, client, *message, reason);

    switch (result.first) {
        case client::SendResult::SHUTDOWN: {
            return;
        }
        case client::SendResult::VALID_REPLY: {
            assert_false(nullptr == result.second);
            auto& reply = *result.second;

            if (reply.success_) {
                if (data.process_nymbox_.load() &&
                    false == bool(data.pending_message_)) {
                    data.pending_message_ = result.second;
                }

                update_state(
                    data, protobuf::DELIVERTYSTATE_NEEDBOXITEMS, reason);

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
            LogError()()("Error downloading nymbox").Flush();

            return;
        }
    }
}

auto Server::need_process_nymbox(
    Data& data,
    const api::session::Client& client,
    const PasswordPrompt& reason) -> void
{
    const auto messageLock = Lock{data.message_lock_};
    auto nymbox{api_.Factory().Internal().Session().Ledger(
        Signer()->ID(), Signer()->ID(), server_id_, otx::ledgerType::nymbox)};

    assert_false(nullptr == nymbox);

    const auto loaded = nymbox->LoadNymbox();

    if (false == loaded) {
        LogConsole()()("Unable to load nymbox").Flush();

        return;
    }

    const auto verified = nymbox->VerifyAccount(*Signer());

    if (false == verified) {
        LogConsole()()("Unable to verify nymbox").Flush();

        return;
    }

    const auto count = nymbox->GetTransactionCount();

    if (1 > count) {
        LogDetail()()("Nymbox is empty (so, skipping processNymbox).").Flush();

        if (data.process_nymbox_) {
            DeliveryResult result{
                otx::LastReplyStatus::MessageSuccess, data.pending_message_};
            resolve_queue(
                data, std::move(result), reason, protobuf::DELIVERTYSTATE_IDLE);
        } else {
            // The server never received the original message.
            update_state(data, protobuf::DELIVERTYSTATE_PENDINGSEND, reason);
        }

        return;
    }

    auto message{api_.Factory().Internal().Session().Message()};

    assert_false(nullptr == message);

    ReplyNoticeOutcomes outcomes{};
    std::size_t alreadySeen{0};
    const auto accepted = accept_entire_nymbox(
        data, client, *nymbox, *message, outcomes, alreadySeen, reason);

    if (data.pending_message_) {
        const RequestNumber targetNumber =
            String::StringToUlong(data.pending_message_->request_num_->Get());

        for (auto& [number, status] : outcomes) {
            if (number == targetNumber) {
                resolve_queue(data, std::move(status), reason);
            }
        }
    }

    if (false == accepted) {
        if (alreadySeen == static_cast<std::size_t>(count)) {
            if (data.process_nymbox_) {
                DeliveryResult result{
                    otx::LastReplyStatus::MessageSuccess,
                    data.pending_message_};
                resolve_queue(
                    data,
                    std::move(result),
                    reason,
                    protobuf::DELIVERTYSTATE_IDLE);
            } else {
                // The server never received the original message.
                update_state(
                    data, protobuf::DELIVERTYSTATE_PENDINGSEND, reason);
            }
        } else {
            LogError()()("Failed trying to accept the entire "
                         "Nymbox. (And no, it's not empty).")
                .Flush();
            update_state(data, protobuf::DELIVERTYSTATE_NEEDNYMBOX, reason);
        }

        return;
    }

    data.local_nymbox_hash_.GetString(api_.Crypto(), message->nymbox_hash_);

    if (false == finalize_server_command(*message, reason)) {
        LogError()()("Failed to finalize server message.").Flush();

        return;
    }

    auto result = attempt_delivery(data, messageLock, client, *message, reason);

    switch (result.first) {
        case client::SendResult::SHUTDOWN: {
            return;
        }
        case client::SendResult::VALID_REPLY: {
            auto& pReply = result.second;

            assert_false(nullptr == pReply);

            if (pReply->success_) {
                [[maybe_unused]] auto [number, msg] = initialize_server_command(
                    data, otx::MessageType::getNymbox, -1, true, true);

                assert_false(nullptr == msg);

                const auto finalized = FinalizeServerCommand(*msg, reason);

                assert_true(finalized);

                auto again =
                    attempt_delivery(data, messageLock, client, *msg, reason);

                if (data.process_nymbox_) {
                    DeliveryResult resultProcessNymbox{
                        otx::LastReplyStatus::MessageSuccess,
                        data.pending_message_};
                    resolve_queue(
                        data,
                        std::move(resultProcessNymbox),
                        reason,
                        protobuf::DELIVERTYSTATE_IDLE);
                } else {
                    // The server never received the original message.
                    update_state(
                        data, protobuf::DELIVERTYSTATE_PENDINGSEND, reason);
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
            LogError()()("Error processing nymbox").Flush();
            update_state(data, protobuf::DELIVERTYSTATE_NEEDNYMBOX, reason);

            return;
        }
    }
}

auto Server::need_request_number(const otx::MessageType type) -> bool
{
    return false == do_not_need_request_number_.contains(type);
}

auto Server::NextTransactionNumber(const otx::MessageType reason)
    -> otx::context::ManagedNumber
{
    return next_transaction_number(*get_data(), reason);
}

auto Server::next_transaction_number(Data& data, const otx::MessageType reason)
    -> otx::context::ManagedNumber
{
    const std::size_t reserve =
        (otx::MessageType::processInbox == reason) ? 0 : 1;

    if (0 == reserve) {
        LogVerbose()()("Allocating a transaction number for process inbox.")
            .Flush();
    } else {
        LogVerbose()()(
            "Allocating a transaction number for normal transaction.")
            .Flush();
    }

    LogVerbose()()(data.available_transaction_numbers_.size())(
        " numbers available.")
        .Flush();
    LogVerbose()()(data.issued_transaction_numbers_.size())(" numbers issued.")
        .Flush();

    if (reserve >= data.available_transaction_numbers_.size()) {
        return factory::ManagedNumber(0, *this);
    }

    auto first = data.available_transaction_numbers_.begin();
    const auto output = *first;
    data.available_transaction_numbers_.erase(first);

    return factory::ManagedNumber(output, *this);
}

auto Server::nym_to_account(const identifier::Nym& id) const noexcept
    -> identifier::Account
{
    return api_.Factory().AccountIDFromHash(
        id.Bytes(), identifier::AccountSubtype::custodial_account);
}

auto Server::pending_send(
    Data& data,
    const api::session::Client& client,
    const PasswordPrompt& reason) -> void
{
    const auto messageLock = Lock{data.message_lock_};

    assert_false(nullptr == data.pending_message_);

    auto result = attempt_delivery(
        data, messageLock, client, *data.pending_message_, reason);
    const auto needRequestNumber = need_request_number(
        Message::Type(data.pending_message_->command_->Get()));

    switch (result.first) {
        case client::SendResult::SHUTDOWN: {
            return;
        }
        case client::SendResult::VALID_REPLY: {
            assert_false(nullptr == result.second);

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
                data, std::move(output), reason, protobuf::DELIVERTYSTATE_IDLE);

        } break;
        case client::SendResult::TIMEOUT:
        case client::SendResult::INVALID_REPLY: {
            if (needRequestNumber) {
                update_state(
                    data,
                    protobuf::DELIVERTYSTATE_NEEDNYMBOX,
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
    auto handle = get_data();
    auto& data = *handle;
    const auto lock = Lock{data.message_lock_};

    assert_false(nullptr == Signer());

    auto request = initialize_server_command(otx::MessageType::pingNotary);

    if (false == bool(request)) {
        LogError()()("Failed to initialize server message.").Flush();

        return {};
    }

    auto serializedAuthKey = protobuf::AsymmetricKey{};
    if (false ==
        Signer()->GetPublicAuthKey().Internal().Serialize(serializedAuthKey)) {
        LogError()()("Failed to serialize auth key").Flush();

        return {};
    }

    auto serializedEncryptKey = protobuf::AsymmetricKey{};
    if (false == Signer()->GetPublicEncrKey().Internal().Serialize(
                     serializedEncryptKey)) {
        LogError()()("Failed to serialize encrypt key").Flush();

        return {};
    }

    request->request_num_ =
        String::Factory(std::to_string(FIRST_REQUEST_NUMBER).c_str());
    request->nym_public_key_ =
        api_.Factory().Internal().Armored(serializedAuthKey, "ASYMMETRIC KEY");
    request->nym_id2_ = api_.Factory().Internal().Armored(
        serializedEncryptKey, "ASYMMETRIC KEY");

    if (false == finalize_server_command(*request, reason)) {
        LogError()()("Failed to finalize server message.").Flush();

        return {};
    }

    return data.connection_.Send(
        *this,
        data,
        *request,
        reason,
        static_cast<opentxs::network::ServerConnection::Push>(
            data.enable_otx_push_.load()));
}

auto Server::ProcessNotification(
    const api::session::Client& client,
    const otx::Reply& notification,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());

    const auto pPush = notification.Push();

    if (false == bool(pPush)) {
        LogError()()("Missing push payload").Flush();

        return false;
    }

    auto handle = get_data();
    auto& data = *handle;
    const auto& push = *pPush;

    switch (push.type()) {
        case protobuf::OTXPUSH_NYMBOX: {
            // Nymbox items don't have an intrinsic account ID. Use nym ID
            // instead.
            const auto account = api_.Factory().AccountIDFromHash(
                Signer()->ID().Bytes(),
                identifier::AccountSubtype::custodial_account);

            return process_box_item(data, client, account, push, reason);
        }
        case protobuf::OTXPUSH_INBOX: {
            return process_account_push(data, client, push, reason);
        }
        case protobuf::OTXPUSH_ERROR:
        case protobuf::OTXPUSH_OUTBOX:
        default: {
            LogError()()("Unsupported push type").Flush();

            return false;
        }
    }
}

auto Server::process_accept_basket_receipt_reply(
    Data& data,
    const OTTransaction& inboxTransaction) -> void
{
    const auto number = inboxTransaction.GetClosingNum();
    LogVerbose()()("Successfully removed basketReceipt with closing number: ")(
        number)
        .Flush();
    consume_issued(data, number);
}

auto Server::process_accept_cron_receipt_reply(
    const identifier::Account& accountID,
    OTTransaction& inboxTransaction) const -> void
{
    auto pServerItem = inboxTransaction.GetItem(otx::itemType::marketReceipt);

    if (false == bool(pServerItem)) {
        // Only marketReceipts have required action here.

        return;
    }

    auto strOffer = String::Factory(), strTrade = String::Factory();
    // contains updated offer.
    pServerItem->GetAttachment(strOffer);
    // contains updated trade.
    pServerItem->GetNote(strTrade);
    auto theOffer = api_.Factory().Internal().Session().Offer();

    assert_false(nullptr == theOffer);

    auto theTrade = api_.Factory().Internal().Session().Trade();

    assert_false(nullptr == theTrade);

    api_.Factory().Internal().Session().Trade();
    const bool bLoadOfferFromString =
        theOffer->LoadContractFromString(strOffer);
    const bool bLoadTradeFromString =
        theTrade->LoadContractFromString(strTrade);

    if (bLoadOfferFromString && bLoadTradeFromString) {
        std::unique_ptr<OTDB::TradeDataNym> pData(
            dynamic_cast<OTDB::TradeDataNym*>(
                OTDB::CreateObject(OTDB::STORED_OBJ_TRADE_DATA_NYM)));

        assert_false(nullptr == pData);

        const Amount& lScale = theOffer->GetScale();

        // TransID for original offer.
        // (Offer may trade many times.)
        pData->transaction_id_ = std::to_string(theTrade->GetTransactionNum());
        // TransID for BOTH receipts for current trade.
        // (Asset/Currency.)
        pData->updated_id_ = std::to_string(pServerItem->GetTransactionNum());
        pData->completed_count_ = std::to_string(theTrade->GetCompletedCount());
        auto account = api_.Wallet().Internal().Account(accountID);

        assert_true(account);

        const bool bIsAsset =
            (theTrade->GetInstrumentDefinitionID() ==
             account.get().GetInstrumentDefinitionID());
        const bool bIsCurrency =
            (theTrade->GetCurrencyID() ==
             account.get().GetInstrumentDefinitionID());
        const auto strAcctID = String::Factory(accountID, api_.Crypto());
        const auto strServerTransaction = String::Factory(inboxTransaction);

        if (bIsAsset) {
            const auto strInstrumentDefinitionID = String::Factory(
                theTrade->GetInstrumentDefinitionID(), api_.Crypto());
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
                String::Factory(theTrade->GetCurrencyID(), api_.Crypto());
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
        pData->date_ =
            std::to_string(seconds_since_epoch(tProcessDate).value());

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
        auto strNymID = String::Factory(Signer()->ID(), api_.Crypto());
        std::unique_ptr<OTDB::TradeListNym> pList;

        if (OTDB::Exists(
                api_,
                api_.DataFolder().string(),
                api_.Internal().Paths().Nym(),
                "trades",  // todo stop
                           // hardcoding.
                server_id_.asBase58(api_.Crypto()).c_str(),
                strNymID->Get())) {
            pList.reset(dynamic_cast<OTDB::TradeListNym*>(OTDB::QueryObject(
                api_,
                OTDB::STORED_OBJ_TRADE_LIST_NYM,
                api_.DataFolder().string(),
                api_.Internal().Paths().Nym(),
                "trades",  // todo stop
                // hardcoding.
                server_id_.asBase58(api_.Crypto()).c_str(),
                strNymID->Get())));
        }
        if (false == bool(pList)) {
            LogVerbose()()("Creating storage list of trade ")(
                "receipts for "
                "Nym: ")(strNymID.get())
                .Flush();
            pList.reset(dynamic_cast<OTDB::TradeListNym*>(
                OTDB::CreateObject(OTDB::STORED_OBJ_TRADE_LIST_NYM)));
        }
        assert_false(nullptr == pList);

        // Loop through and see if we can find one that's
        // ALREADY there. We can match the asset receipt and
        // currency receipt. This way we ensure there is
        // only one in the end, which combines info from
        // both. This also enables us to calculate the sale
        // price!
        //
        bool bWeFoundIt = false;

        const std::size_t nTradeDataNymCount = pList->GetTradeDataNymCount();

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
                         api_.Internal().Paths().Nym(),
                         "trades",  // todo stop hardcoding.
                         server_id_.asBase58(api_.Crypto()).c_str(),
                         strNymID->Get())) {
            LogError()()("Failed storing list of trades for nym. Notary ID: ")(
                server_id_, api_.Crypto())(". Nym ID: ")(strNymID.get())(".")
                .Flush();
        }
    }
}

auto Server::process_accept_final_receipt_reply(
    Data& data,
    const OTTransaction& inboxTransaction) -> void
{
    const auto number = inboxTransaction.GetClosingNum();
    const auto referenceNumber = inboxTransaction.GetReferenceToNum();
    LogDetail()()("Successfully removed finalReceipt with closing number: ")(
        number)
        .Flush();
    consume_issued(data, number);

    if (consume_issued(data, referenceNumber)) {
        LogDetail()()(
            "**** Due to finding a finalReceipt, consuming issued opening "
            "number from nym: ")(referenceNumber)
            .Flush();
    } else {
        LogDetail()()(
            "**** Noticed a finalReceipt, but Opening "
            "Number ")(referenceNumber)(" had ALREADY been removed from nym.")
            .Flush();
    }

    OTCronItem::EraseActiveCronReceipt(
        api_,
        api_.DataFolder().string(),
        inboxTransaction.GetReferenceToNum(),
        Signer()->ID(),
        inboxTransaction.GetPurportedNotaryID());
}

void Server::process_accept_item_receipt_reply(
    Data& data,
    const api::session::Client& client,
    const identifier::Account& accountID,
    const Message& reply,
    const OTTransaction& inboxTransaction)
{
    assert_false(nullptr == Signer());
    assert_false(nullptr == remote_nym_);

    const auto& nymID = Signer()->ID();
    auto serializedOriginal = String::Factory();
    inboxTransaction.GetReferenceString(serializedOriginal);
    auto pOriginalItem = api_.Factory().Internal().Session().Item(
        serializedOriginal, server_id_, inboxTransaction.GetReferenceToNum());

    if (false == bool(pOriginalItem)) {
        LogError()()("Unable to load original item from "
                     "string while accepting item receipt: ")(
            serializedOriginal.get())(".")
            .Flush();

        return;
    }

    auto& originalItem = *pOriginalItem;
    auto originalType = String::Factory();
    originalItem.GetTypeString(originalType);

    switch (originalItem.GetType()) {
        case otx::itemType::depositCheque: {
            auto serialized = String::Factory();
            originalItem.GetAttachment(serialized);
            auto cheque = api_.Factory().Internal().Session().Cheque();

            assert_false(nullptr == cheque);

            if (false == cheque->LoadContractFromString(serialized)) {
                LogError()()("Failed to deserialize cheque").Flush();

                break;
            }

            const auto number = cheque->GetTransactionNum();
            consume_issued(data, number);
        } break;
        case otx::itemType::acceptPending: {
            consume_issued(data, originalItem.GetNumberOfOrigin());

            auto serialized = String::Factory();
            originalItem.GetAttachment(serialized);

            if (serialized->empty()) {
                LogError()()("Missing attachment").Flush();

                break;
            }

            const auto transferReceipt =
                api_.Factory().Internal().Session().Transaction(
                    remote_nym_->ID(), accountID, server_id_);

            assert_false(nullptr == transferReceipt);

            const auto loaded =
                transferReceipt->LoadContractFromString(serialized);

            if (false == loaded) {
                LogError()()("Unable to instantiate transfer receipt").Flush();

                break;
            }

            const auto pTransfer = extract_transfer(*transferReceipt);

            if (pTransfer) {
                const auto& transfer = *pTransfer;

                if (is_internal_transfer(transfer)) {
                    LogDetail()()("Completing internal transfer").Flush();
                } else {
                    LogDetail()()("Completing outgoing transfer").Flush();
                }

                client.Workflow().CompleteTransfer(
                    nymID, server_id_, *transferReceipt, reply);
            } else {
                LogError()()("Invalid transfer").Flush();
            }
        } break;
        case otx::itemType::transfer:
        case otx::itemType::atTransfer:
        case otx::itemType::acceptTransaction:
        case otx::itemType::atAcceptTransaction:
        case otx::itemType::acceptMessage:
        case otx::itemType::atAcceptMessage:
        case otx::itemType::acceptNotice:
        case otx::itemType::atAcceptNotice:
        case otx::itemType::atAcceptPending:
        case otx::itemType::rejectPending:
        case otx::itemType::atRejectPending:
        case otx::itemType::acceptCronReceipt:
        case otx::itemType::atAcceptCronReceipt:
        case otx::itemType::acceptItemReceipt:
        case otx::itemType::atAcceptItemReceipt:
        case otx::itemType::disputeCronReceipt:
        case otx::itemType::atDisputeCronReceipt:
        case otx::itemType::disputeItemReceipt:
        case otx::itemType::atDisputeItemReceipt:
        case otx::itemType::acceptFinalReceipt:
        case otx::itemType::atAcceptFinalReceipt:
        case otx::itemType::acceptBasketReceipt:
        case otx::itemType::atAcceptBasketReceipt:
        case otx::itemType::disputeFinalReceipt:
        case otx::itemType::atDisputeFinalReceipt:
        case otx::itemType::disputeBasketReceipt:
        case otx::itemType::atDisputeBasketReceipt:
        case otx::itemType::serverfee:
        case otx::itemType::atServerfee:
        case otx::itemType::issuerfee:
        case otx::itemType::atIssuerfee:
        case otx::itemType::balanceStatement:
        case otx::itemType::atBalanceStatement:
        case otx::itemType::transactionStatement:
        case otx::itemType::atTransactionStatement:
        case otx::itemType::withdrawal:
        case otx::itemType::atWithdrawal:
        case otx::itemType::deposit:
        case otx::itemType::atDeposit:
        case otx::itemType::withdrawVoucher:
        case otx::itemType::atWithdrawVoucher:
        case otx::itemType::atDepositCheque:
        case otx::itemType::payDividend:
        case otx::itemType::atPayDividend:
        case otx::itemType::marketOffer:
        case otx::itemType::atMarketOffer:
        case otx::itemType::paymentPlan:
        case otx::itemType::atPaymentPlan:
        case otx::itemType::smartContract:
        case otx::itemType::atSmartContract:
        case otx::itemType::cancelCronItem:
        case otx::itemType::atCancelCronItem:
        case otx::itemType::exchangeBasket:
        case otx::itemType::atExchangeBasket:
        case otx::itemType::chequeReceipt:
        case otx::itemType::voucherReceipt:
        case otx::itemType::marketReceipt:
        case otx::itemType::paymentReceipt:
        case otx::itemType::transferReceipt:
        case otx::itemType::finalReceipt:
        case otx::itemType::basketReceipt:
        case otx::itemType::replyNotice:
        case otx::itemType::successNotice:
        case otx::itemType::notice:
        case otx::itemType::error_state:
        default: {
            LogError()()("Unexpected original item type: ")(originalType.get())
                .Flush();
        }
    }
}

auto Server::process_accept_pending_reply(
    const api::session::Client& client,
    const identifier::Account& accountID,
    const Item& acceptItemReceipt,
    const Message& reply) const -> void
{
    assert_false(nullptr == Signer());
    assert_false(nullptr == remote_nym_);

    if (otx::itemType::acceptPending != acceptItemReceipt.GetType()) {
        auto type = String::Factory();
        acceptItemReceipt.GetTypeString(type);
        LogError()()("Invalid type: ")(type.get()).Flush();

        return;
    }

    auto attachment = String::Factory();
    acceptItemReceipt.GetAttachment(attachment);

    if (attachment->empty()) {
        LogError()()("Missing attachment").Flush();

        return;
    }

    assert_false(attachment->empty());

    const auto pending = api_.Factory().Internal().Session().Transaction(
        remote_nym_->ID(), accountID, server_id_);

    assert_false(nullptr == pending);

    const auto loaded = pending->LoadContractFromString(attachment);

    if (false == loaded) {
        LogError()()("Unable to instantiate").Flush();

        return;
    }

    if (otx::transactionType::pending != pending->GetType()) {
        LogError()()("Wrong item type").Flush();

        return;
    }

    const auto pTransfer = extract_transfer(*pending);

    if (pTransfer) {
        const auto& transfer = *pTransfer;

        if (false == is_internal_transfer(transfer)) {
            LogDetail()()("Accepting incoming transfer").Flush();
            client.Workflow().AcceptTransfer(
                Signer()->ID(), server_id_, *pending, reply);
        }
    } else {
        LogError()()("Invalid transfer").Flush();
    }
}

auto Server::process_account_data(
    Data& data,
    const identifier::Account& accountID,
    const String& account,
    const identifier::Generic& inboxHash,
    const String& inbox,
    const identifier::Generic& outboxHash,
    const String& outbox,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());
    assert_false(nullptr == remote_nym_);

    const auto& nymID = Signer()->ID();
    const auto updated = api_.Wallet().Internal().UpdateAccount(
        accountID, *this, account, reason);

    if (updated) {
        LogDetail()()("Saved updated account file.").Flush();
    } else {
        LogError()()("Failed to save account.").Flush();

        return false;
    }

    if (false == bool(data.inbox_)) {
        data.inbox_.reset(
            api_.Factory()
                .Internal()
                .Session()
                .Ledger(nymID, accountID, server_id_, otx::ledgerType::inbox)
                .release());
    }

    assert_false(nullptr == data.inbox_);
    assert_true(otx::ledgerType::inbox == data.inbox_->GetType());

    if (false == data.inbox_->LoadInboxFromString(inbox)) {
        LogError()()("Failed to deserialize inbox").Flush();

        return false;
    }

    if (false == data.inbox_->VerifySignature(*remote_nym_)) {
        LogError()()("Invalid inbox signature").Flush();

        return false;
    }

    auto nymfile = mutable_Nymfile(reason);

    if (false == inboxHash.empty()) {
        const bool hashSet = nymfile.get().SetInboxHash(
            accountID.asBase58(api_.Crypto()), inboxHash);

        if (false == hashSet) {
            LogError()()("Failed setting inbox hash for "
                         "account: ")(accountID, api_.Crypto())(" to (")(
                inboxHash, api_.Crypto())(").")
                .Flush();
        }
    }

    for (const auto& [number, pTransaction] :
         data.inbox_->GetTransactionMap()) {
        if (false == bool(pTransaction)) {
            LogError()()("Invalid transaction ")(number)(" in inbox").Flush();

            return false;
        }

        auto& transaction = *pTransaction;

        if (otx::transactionType::finalReceipt == transaction.GetType()) {
            LogVerbose()()("*** Removing opening issued number (")(
                transaction.GetReferenceToNum())(
                "), since finalReceipt found when ")(
                "retrieving asset account inbox. ***")
                .Flush();

            if (consume_issued(data, transaction.GetReferenceToNum())) {
                LogDetail()()(
                    "**** Due to finding a finalReceipt, consuming issued "
                    "opening number from nym: ")(
                    transaction.GetReferenceToNum())
                    .Flush();
            } else {
                LogDetail()()(
                    "**** Noticed a finalReceipt, but issued opening number ")(
                    transaction.GetReferenceToNum())(
                    " had ALREADY been consumed from nym.")
                    .Flush();
            }

            OTCronItem::EraseActiveCronReceipt(
                api_,
                api_.DataFolder().string(),
                transaction.GetReferenceToNum(),
                Signer()->ID(),
                transaction.GetPurportedNotaryID());
        }
    }

    data.inbox_->ReleaseSignatures();
    data.inbox_->SignContract(*Signer(), reason);
    data.inbox_->SaveContract();
    data.inbox_->SaveInbox();

    if (false == bool(data.outbox_)) {
        data.outbox_.reset(
            api_.Factory()
                .Internal()
                .Session()
                .Ledger(nymID, accountID, server_id_, otx::ledgerType::outbox)
                .release());
    }

    assert_false(nullptr == data.outbox_);
    assert_true(otx::ledgerType::outbox == data.outbox_->GetType());

    if (false == data.outbox_->LoadOutboxFromString(outbox)) {
        LogError()()("Failed to deserialize outbox").Flush();

        return false;
    }

    if (false == data.outbox_->VerifySignature(*remote_nym_)) {
        LogError()()("Invalid outbox signature").Flush();

        return false;
    }

    if (false == outboxHash.empty()) {
        const bool hashSet = nymfile.get().SetOutboxHash(
            accountID.asBase58(api_.Crypto()), outboxHash);

        if (false == hashSet) {
            LogError()()("Failed setting outbox hash for "
                         "account: ")(accountID, api_.Crypto())(" to (")(
                outboxHash, api_.Crypto())(") .")
                .Flush();
        }
    }

    data.outbox_->ReleaseSignatures();
    data.outbox_->SignContract(*Signer(), reason);
    data.outbox_->SaveContract();
    data.outbox_->SaveOutbox();

    return true;
}

auto Server::process_account_push(
    Data& data,
    const api::session::Client& client,
    const protobuf::OTXPush& push,
    const PasswordPrompt& reason) -> bool
{
    const auto accountID = api_.Factory().AccountIDFromBase58(push.accountid());
    const auto inboxHash =
        api_.Factory().IdentifierFromBase58(push.inboxhash());
    const auto outboxHash =
        api_.Factory().IdentifierFromBase58(push.outboxhash());
    const auto account = String::Factory(push.account());
    const auto inbox = String::Factory(push.inbox());
    const auto outbox = String::Factory(push.outbox());

    const auto processed = process_account_data(
        data, accountID, account, inboxHash, inbox, outboxHash, outbox, reason);

    if (processed) {
        LogVerbose()()("Success saving new account data.").Flush();
    } else {
        LogError()()("Failure saving new account data.").Flush();

        return false;
    }

    return process_box_item(data, client, accountID, push, reason);
}

auto Server::process_box_item(
    Data& data,
    const api::session::Client& client,
    const identifier::Account& accountID,
    const protobuf::OTXPush& push,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());
    assert_false(nullptr == remote_nym_);

    const auto& nym = *Signer();
    const auto& remoteNym = *remote_nym_;
    const auto& nymID = nym.ID();
    BoxType box{BoxType::Invalid};

    switch (push.type()) {
        case protobuf::OTXPUSH_NYMBOX: {
            box = BoxType::Nymbox;
        } break;
        case protobuf::OTXPUSH_INBOX: {
            box = BoxType::Inbox;
        } break;
        case protobuf::OTXPUSH_OUTBOX: {
            box = BoxType::Outbox;
        } break;
        case protobuf::OTXPUSH_ERROR:
        default: {
            LogError()()("Invalid box type").Flush();

            return false;
        }
    }

    const auto& payload = push.item();

    if (payload.empty()) {
        LogError()()("Missing payload").Flush();

        return false;
    }

    const std::shared_ptr<OTTransactionType> base{
        api_.Factory().Internal().Session().Transaction(
            String::Factory(payload))};

    if (false == bool(base)) {
        LogError()()("Invalid payload").Flush();

        return false;
    }

    auto receipt = std::dynamic_pointer_cast<OTTransaction>(base);

    if (false == bool(base)) {
        LogError()()("Wrong transaction type").Flush();

        return false;
    }

    assert_false(nullptr == receipt);

    if (receipt->GetNymID() != nymID) {
        LogError()()("Wrong nym id on box receipt").Flush();

        return false;
    }

    if (false == receipt->VerifyAccount(remoteNym)) {
        LogError()()("Unable to verify box receipt").Flush();

        return false;
    }

    LogVerbose()()("Validated a push notification of type: ")(
        receipt->GetTypeString())
        .Flush();

    return process_get_box_receipt_response(
        data,
        client,
        accountID,
        receipt,
        String::Factory(payload.c_str()),
        box,
        reason);
}

auto Server::process_get_nymbox_response(
    Data& data,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());

    const auto& nymID = Signer()->ID();
    auto serialized = String::Factory(reply.payload_);
    auto nymbox =
        api_.Factory().Internal().Session().Ledger(nymID, nymID, server_id_);

    assert_false(nullptr == nymbox);

    if (false == nymbox->LoadNymboxFromString(serialized)) {
        LogError()()("Error loading or verifying nymbox").Flush();

        return false;
    }

    auto nymboxHash = identifier::Generic{};
    nymbox->ReleaseSignatures();
    nymbox->SignContract(*Signer(), reason);
    nymbox->SaveContract();
    nymbox->SaveNymbox(nymboxHash);
    update_nymbox_hash(data, reply, UpdateHash::Both);

    return true;
}

auto Server::process_check_nym_response(
    Data& data,
    const api::session::Client& client,
    const Message& reply) -> bool
{
    update_nymbox_hash(data, reply);

    if ((false == reply.bool_) || (reply.payload_->empty())) {
        LogVerbose()()("Server ")(server_id_, api_.Crypto())(
            " does not have nym ")(reply.nym_id2_.get())
            .Flush();

        return true;
    }

    auto serialized =
        protobuf::Factory<protobuf::Nym>(ByteArray{reply.payload_});

    auto nym = client.Wallet().Internal().Nym(serialized);

    if (nym) {
        return true;
    } else {
        LogError()()("checkNymResponse: Retrieved nym is invalid.").Flush();
    }

    return false;
}

auto Server::process_get_account_data(
    Data& data,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    const auto accountID =
        api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes());
    auto serializedAccount = String::Factory();
    auto serializedInbox = String::Factory();
    auto serializedOutbox = String::Factory();

    if (accountID.empty()) {
        LogError()()("Invalid account ID").Flush();

        return false;
    }

    if (false == reply.payload_->GetString(serializedAccount)) {
        LogError()()("Invalid serialized account").Flush();

        return false;
    }

    if (false == reply.payload2_->GetString(serializedInbox)) {
        LogError()()("Invalid serialized inbox").Flush();

        return false;
    }

    if (false == reply.payload3_->GetString(serializedOutbox)) {
        LogError()()("Invalid serialized outbox").Flush();

        return false;
    }

    return process_account_data(
        data,
        accountID,
        serializedAccount,
        api_.Factory().IdentifierFromBase58(reply.inbox_hash_->Bytes()),
        serializedInbox,
        api_.Factory().IdentifierFromBase58(reply.outbox_hash_->Bytes()),
        serializedOutbox,
        reason);
}

auto Server::process_get_box_receipt_response(
    Data& data,
    const api::session::Client& client,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());
    assert_false(nullptr == remote_nym_);

    update_nymbox_hash(data, reply);
    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    const auto& serverNym = *remote_nym_;
    const auto type = get_type(reply.depth_);

    if (BoxType::Invalid == type) { return false; }

    auto serialized = String::Factory(reply.payload_);
    auto boxReceipt = extract_box_receipt(
        serialized, serverNym, nymID, reply.transaction_num_);

    if (false == bool(boxReceipt)) { return false; }

    return process_get_box_receipt_response(
        data,
        client,
        api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes()),
        boxReceipt,
        serialized,
        type,
        reason);
}

auto Server::process_get_box_receipt_response(
    Data& data,
    const api::session::Client& client,
    const identifier::Account& accountID,
    const std::shared_ptr<OTTransaction> receipt,
    const String& serialized,
    const BoxType type,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());
    assert_false(nullptr == receipt);

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    bool processInbox{false};

    switch (receipt->GetType()) {
        case otx::transactionType::message: {
            process_incoming_message(data, client, *receipt, reason);
        } break;
        case otx::transactionType::instrumentNotice:
        case otx::transactionType::instrumentRejection: {
            processInbox = true;
            process_incoming_instrument(receipt, reason);
        } break;
        case otx::transactionType::transferReceipt: {
            processInbox = true;
            const auto pTransfer = extract_transfer(*receipt);

            if (pTransfer) {
                const auto& transfer = *pTransfer;

                if (is_internal_transfer(transfer)) {
                    LogDetail()()("Clearing internal transfer").Flush();
                } else {
                    LogDetail()()("Clearing outgoing transfer").Flush();
                }

                client.Workflow().ClearTransfer(nymID, server_id_, *receipt);
            } else {
                LogError()()("Invalid transfer").Flush();
            }
        } break;
        case otx::transactionType::pending: {
            processInbox = true;
            const auto pTransfer = extract_transfer(*receipt);

            if (pTransfer) {
                const auto& transfer = *pTransfer;

                if (is_internal_transfer(transfer)) {
                    LogDetail()()("Conveying internal transfer").Flush();
                    client.Workflow().ConveyTransfer(
                        nymID, server_id_, *receipt);
                } else if (transfer.GetNymID() != nymID) {
                    LogDetail()()("Conveying incoming transfer").Flush();
                    client.Workflow().ConveyTransfer(
                        nymID, server_id_, *receipt);
                }
            } else {
                LogError()()("Invalid transfer").Flush();
            }
        } break;
        case otx::transactionType::chequeReceipt: {
        } break;
        case otx::transactionType::voucherReceipt: {
        } break;
        default: {
        }
    }

    if (false == receipt->SaveBoxReceipt(static_cast<std::int64_t>(type))) {
        LogError()()("Failed to save box receipt:").Flush();
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

auto Server::process_get_market_list_response(const Message& reply) const
    -> bool
{
    auto data_file = api::internal::Paths::GetFilenameBin("market_data");

    auto* pStorage = OTDB::GetDefaultStorage();
    assert_false(nullptr == pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase the
    // data file. (So when the file is loaded from storage, it will correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    if (reply.depth_ == 0) {
        const bool success = storage.EraseValueByKey(
            api_,
            api_.DataFolder().string(),
            api_.Internal().Paths().Market(),  // "markets"
            reply.notary_id_->Get(),           // "markets/<notaryID>"
            data_file,
            "");  // "markets/<notaryID>/market_data.bin"
        if (!success) {
            LogError()()("Error erasing market list from market folder: ")(
                data_file)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = ByteArray{};

    if ((reply.payload_->GetLength() <= 2) ||
        (false == reply.payload_->GetData(serialized))) {
        LogError()()("Unable to decode ascii-armored payload.").Flush();
        return true;
    }

    // Unpack the market list...

    auto* pPacker = storage.GetPacker();

    assert_false(nullptr == pPacker);

    auto& packer = *pPacker;

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(packer.CreateBuffer());

    assert_false(nullptr == pBuffer);

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized.data()), serialized.size());

    std::unique_ptr<OTDB::MarketList> pMarketList(
        dynamic_cast<OTDB::MarketList*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_MARKET_LIST)));

    auto unpacked = pPacker->Unpack(buffer, *pMarketList);

    if (!unpacked) {
        LogError()()("Failed unpacking data.").Flush();
        return true;
    }

    const bool success = storage.StoreObject(
        api_,
        *pMarketList,
        api_.DataFolder().string(),
        api_.Internal().Paths().Market(),  // "markets"
        reply.notary_id_->Get(),           // "markets/<notaryID>"
        data_file,
        "");  // "markets/<notaryID>/market_data.bin"
    if (!success) {
        LogError()()("Error storing market list to market folder: ")(
            data_file)(".")
            .Flush();
    }

    return true;
}

auto Server::process_get_market_offers_response(const Message& reply) const
    -> bool
{
    const String& marketID = reply.nym_id2_;  // market ID stored here.

    auto data_file = api::internal::Paths::GetFilenameBin(marketID.Get());

    auto* pStorage = OTDB::GetDefaultStorage();
    assert_false(nullptr == pStorage);

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
            api_.Internal().Paths().Market(),  // "markets"
            reply.notary_id_->Get(),           // "markets/<notaryID>",
            "offers",                          // "markets/<notaryID>/offers"
                                               // todo stop hardcoding.
            data_file);  // "markets/<notaryID>/offers/<marketID>.bin"
        if (!success) {
            LogError()()("Error erasing offers list from market folder: ")(
                data_file)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = ByteArray{};

    if ((reply.payload_->GetLength() <= 2) ||
        (false == reply.payload_->GetData(serialized))) {
        LogError()()("Unable to decode asci-armored payload.").Flush();
        return true;
    }

    // Unpack the market list...

    OTDB::OTPacker* pPacker = storage.GetPacker();

    assert_false(nullptr == pPacker);

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());
    assert_false(nullptr == pBuffer);

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized.data()), serialized.size());

    std::unique_ptr<OTDB::OfferListMarket> pOfferList(
        dynamic_cast<OTDB::OfferListMarket*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_MARKET)));

    auto unpacked = pPacker->Unpack(buffer, *pOfferList);

    if (!unpacked) {
        LogError()()("Failed unpacking data.").Flush();
        return true;
    }

    const bool success = storage.StoreObject(
        api_,
        *pOfferList,
        api_.DataFolder().string(),
        api_.Internal().Paths().Market(),  // "markets"
        reply.notary_id_->Get(),           // "markets/<notaryID>",
        "offers",                          // "markets/<notaryID>/offers"
                                           // todo stop hardcoding.
        data_file);  // "markets/<notaryID>/offers/<marketID>.bin"
    if (!success) {
        LogError()()("Error storing ")(data_file)(" to market folder.").Flush();
    }

    return true;
}

auto Server::process_get_market_recent_trades_response(
    const Message& reply) const -> bool
{
    const String& marketID = reply.nym_id2_;  // market ID stored here.
    auto data_file = api::internal::Paths::GetFilenameBin(marketID.Get());

    auto* pStorage = OTDB::GetDefaultStorage();
    assert_false(nullptr == pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly
    // display an empty list on the screen, instead of a list of outdated
    // items.)
    //
    if (reply.depth_ == 0) {
        const bool success = storage.EraseValueByKey(
            api_,
            api_.DataFolder().string(),
            api_.Internal().Paths().Market(),  // "markets"
            reply.notary_id_->Get(),           // "markets/<notaryID>recent",
                                               // //
                                               // "markets/<notaryID>/recent"
                                               // // todo stop
                                               // hardcoding.
            data_file,
            "");  // "markets/<notaryID>/recent/<marketID>.bin"
        if (!success) {
            LogError()()("Error erasing recent trades list from market "
                         "folder: ")(data_file)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = ByteArray{};

    if ((reply.payload_->GetLength() <= 2) ||
        (false == reply.payload_->GetData(serialized))) {
        LogError()()("Unable to decode asci-armored "
                     "payload.")
            .Flush();
        return true;
    }

    // Unpack the market list...

    auto* pPacker = storage.GetPacker();

    assert_false(nullptr == pPacker);

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());

    assert_false(nullptr == pBuffer);

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized.data()), serialized.size());

    std::unique_ptr<OTDB::TradeListMarket> pTradeList(
        dynamic_cast<OTDB::TradeListMarket*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_TRADE_LIST_MARKET)));

    auto unpacked = pPacker->Unpack(buffer, *pTradeList);

    if (!unpacked) {
        LogError()()("Failed unpacking data.").Flush();
        return true;
    }

    const bool success = storage.StoreObject(
        api_,
        *pTradeList,
        api_.DataFolder().string(),
        api_.Internal().Paths().Market(),  // "markets"
        reply.notary_id_->Get(),           // "markets/<notaryID>"
        "recent",                          // "markets/<notaryID>/recent"
                                           // todo stop hardcoding.
        data_file);  // "markets/<notaryID>/recent/<marketID>.bin"
    if (!success) {
        LogError()()("Error storing ")(data_file)(" to market folder.").Flush();
    }

    return true;
}

auto Server::process_get_mint_response(const Message& reply) const -> bool
{
    auto serialized = String::Factory(reply.payload_);
    const auto server =
        api_.Factory().NotaryIDFromBase58(reply.notary_id_->Bytes());
    const auto unit = api_.Factory().UnitIDFromBase58(
        reply.instrument_definition_id_->Bytes());

    auto pMmint = api_.Factory().Mint(server, unit);

    assert_true(pMmint);

    auto& mint = pMmint.Internal();

    // TODO check the server signature on the mint here...
    if (false == mint.LoadContractFromString(serialized)) {
        LogError()()("Error loading mint from message payload.").Flush();

        return false;
    }

    return mint.SaveMint({});
}

auto Server::process_get_nym_market_offers_response(const Message& reply) const
    -> bool
{
    auto data_file = api::internal::Paths::GetFilenameBin(reply.nym_id_->Get());

    auto* pStorage = OTDB::GetDefaultStorage();
    assert_false(nullptr == pStorage);

    auto& storage = *pStorage;

    // The reply is a SUCCESS, and the COUNT is 0 (empty list was returned.)
    // Since it was a success, but the list was empty, then we need to erase
    // the data file. (So when the file is loaded from storage, it will
    // correctly display an empty list on the screen, instead of a list of
    // outdated items.)
    //
    if (reply.depth_ == 0) {
        const bool success = storage.EraseValueByKey(
            api_,
            api_.DataFolder().string(),
            api_.Internal().Paths().Nym(),  // "nyms"
            reply.notary_id_->Get(),        // "nyms/<notaryID>",
            "offers",                       // "nyms/<notaryID>/offers"
                                            // todo stop hardcoding.
            data_file);  // "nyms/<notaryID>/offers/<NymID>.bin"
        if (!success) {
            LogError()()("Error erasing offers list from nyms "
                         "folder: ")(data_file)(".")
                .Flush();
        }

        return true;
    }

    auto serialized = ByteArray{};

    if ((reply.payload_->GetLength() <= 2) ||
        (false == reply.payload_->GetData(serialized))) {
        LogError()()("Unable to decode ascii-armored "
                     "payload.")
            .Flush();
        return true;
    }

    // Unpack the nym's offer list...

    auto* pPacker = storage.GetPacker();

    assert_false(nullptr == pPacker);

    std::unique_ptr<OTDB::PackedBuffer> pBuffer(pPacker->CreateBuffer());
    assert_false(nullptr == pBuffer);

    auto& buffer = *pBuffer;

    buffer.SetData(
        static_cast<const std::uint8_t*>(serialized.data()), serialized.size());

    std::unique_ptr<OTDB::OfferListNym> pOfferList(
        dynamic_cast<OTDB::OfferListNym*>(
            OTDB::CreateObject(OTDB::STORED_OBJ_OFFER_LIST_NYM)));

    auto unpacked = pPacker->Unpack(buffer, *pOfferList);

    if (!unpacked) {
        LogError()()("Failed unpacking data.").Flush();
        return true;
    }

    const bool success = storage.StoreObject(
        api_,
        *pOfferList,
        api_.DataFolder().string(),
        api_.Internal().Paths().Nym(),  // "nyms"
        reply.notary_id_->Get(),        // "nyms/<notaryID>",
        "offers",                       // "nyms/<notaryID>/offers",
        data_file);                     // "nyms/<notaryID>/offers/<NymID>.bin"
    if (!success) {
        LogError()()("Error storing ")(data_file)(" to nyms folder.").Flush();
    }

    return true;
}

auto Server::process_incoming_instrument(
    const std::shared_ptr<OTTransaction> receipt,
    const PasswordPrompt& reason) const -> void
{
    assert_false(nullptr == receipt);

    auto paymentInbox = load_or_create_payment_inbox(reason);

    if (false == bool(paymentInbox)) { return; }

    add_transaction_to_ledger(
        receipt->GetTransactionNum(), receipt, *paymentInbox, reason);
}

auto Server::process_incoming_message(
    const Data& data,
    const api::session::Client& client,
    const OTTransaction& receipt,
    const PasswordPrompt& reason) const -> void
{
    try {
        assert_false(nullptr == Signer());

        const auto& nymID = Signer()->ID();
        auto serialized = String::Factory();
        receipt.GetReferenceString(serialized);
        auto message = api_.Factory().Internal().Session().Message();

        assert_false(nullptr == message);

        if (false == message->LoadContractFromString(serialized)) {
            throw std::runtime_error{
                "failed to deserialize peer object message"};
        }

        const auto recipientNymId =
            api_.Factory().NymIDFromBase58(message->nym_id2_->Bytes());
        const auto senderNymID =
            api_.Factory().NymIDFromBase58(message->nym_id_->Bytes());

        if (senderNymID.empty()) {
            throw std::runtime_error{"missing sender nym ID"};
        } else {
            client.Contacts().NymToContact(senderNymID);
        }

        if (recipientNymId != nymID) {
            throw std::runtime_error{"recipient nym ID missing or invalid"};
        }

        const auto pPeerObject = api_.Factory().Internal().Session().PeerObject(
            Signer(), message->payload_, reason);

        if (false == bool(pPeerObject)) {
            throw std::runtime_error{"failed to instantiate peer object"};
        }

        auto& peerObject = *pPeerObject;
        using enum contract::peer::ObjectType;

        switch (peerObject.Type()) {
            case Message: {
                client.Activity().Internal().Mail(
                    recipientNymId,
                    *message,
                    otx::client::StorageBox::MAILINBOX,
                    peerObject);
            } break;
            case Cash: {
                process_incoming_cash(
                    client, receipt.GetTransactionNum(), peerObject, *message);
            } break;
            case Payment: {
                const bool created = create_instrument_notice_from_peer_object(
                    data,
                    client,
                    *message,
                    peerObject,
                    receipt.GetTransactionNum(),
                    reason);

                if (!created) {
                    throw std::runtime_error{
                        "failed to instantiate otx payment object"};
                }
            } break;
            case Request: {
                api_.Wallet().PeerRequestReceive(recipientNymId, peerObject);
            } break;
            case Response: {
                api_.Wallet().PeerReplyReceive(recipientNymId, peerObject);
            } break;
            case Error:
            default: {
                throw std::runtime_error{"unknown peer object type"};
            }
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
    }
}

auto Server::process_get_unit_definition_response(
    Data& data,
    const Message& reply) -> bool
{
    update_nymbox_hash(data, reply);
    const auto id = api_.Factory().IdentifierFromBase58(
        reply.instrument_definition_id_->Bytes());

    if (reply.payload_->empty()) {
        LogError()()("Server reply does not contain contract ")(
            id, api_.Crypto())
            .Flush();

        return false;
    }

    const auto raw = ByteArray{reply.payload_};

    switch (static_cast<contract::Type>(reply.enum_)) {
        case contract::Type::nym: {
            const auto serialized = protobuf::Factory<protobuf::Nym>(raw);
            const auto contract = api_.Wallet().Internal().Nym(serialized);

            if (contract) {
                return (id == contract->ID());
            } else {
                LogError()()("Invalid nym").Flush();
            }
        } break;
        case contract::Type::notary: {
            const auto serialized =
                protobuf::Factory<protobuf::ServerContract>(raw);

            try {
                const auto contract =
                    api_.Wallet().Internal().Server(serialized);

                return (id == contract->ID());
            } catch (...) {
                LogError()()("Invalid server contract").Flush();
            }
        } break;
        case contract::Type::unit: {
            auto serialized = protobuf::Factory<protobuf::UnitDefinition>(raw);

            try {
                const auto contract =
                    api_.Wallet().Internal().UnitDefinition(serialized);

                return (id == contract->ID());
            } catch (...) {
                LogError()()("Invalid unit definition").Flush();
            }
        } break;
        case contract::Type::invalid:
        default: {
            LogError()()("invalid contract type").Flush();
        }
    }

    return false;
}

auto Server::process_issue_unit_definition_response(
    Data& data,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    update_nymbox_hash(data, reply);
    const auto accountID =
        api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes());

    if (reply.payload_->empty()) {
        LogError()()("Server reply does not contain issuer account ")(
            accountID, api_.Crypto())
            .Flush();

        return false;
    }

    auto serialized = String::Factory(reply.payload_);
    const auto updated = api_.Wallet().Internal().UpdateAccount(
        accountID, *this, serialized, std::get<0>(data.pending_args_), reason);

    if (updated) {
        LogDetail()()("Saved new issuer account.").Flush();

        return init_new_account(accountID, reason);
    }

    LogError()()("Failed to save account.").Flush();

    return false;
}

auto Server::process_notarize_transaction_response(
    Data& data,
    const api::session::Client& client,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());
    assert_false(nullptr == remote_nym_);

    update_nymbox_hash(data, reply);
    const auto accountID =
        api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes());
    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    const auto& serverNym = *remote_nym_;
    auto responseLedger = api_.Factory().Internal().Session().Ledger(
        nymID, accountID, server_id_);

    assert_false(nullptr == responseLedger);

    bool loaded =
        responseLedger->LoadLedgerFromString(String::Factory(reply.payload_));

    if (loaded) { loaded &= responseLedger->VerifyAccount(serverNym); }

    if (false == loaded) {
        LogError()()("Error loading ledger from message payload.").Flush();

        return false;
    }

    for (const auto& [number, pTransaction] :
         responseLedger->GetTransactionMap()) {
        if (false == bool(pTransaction)) {
            LogError()()("Invalid transaction ")(number)(" in response ledger")
                .Flush();

            return false;
        }

        auto& transaction = *pTransaction;
        const auto& transactionNumber = transaction.GetTransactionNum();

        if (false == verify_issued_number(data, transactionNumber)) {
            LogVerbose()()(
                "Skipping processing of server reply to transaction number ")(
                transactionNumber)(" since the number isn't even issued to me. "
                                   "Usually this means that I ALREADY "
                                   "processed it, and we are now processing "
                                   "the redundant nymbox notice for the same "
                                   "transaction. (Which was only sent to make "
                                   "sure we saw it.) ")
                .Flush();

            continue;
        }

        if (false == transaction.VerifyAccount(serverNym)) {
            LogConsole()()("Unable to verify transaction ")(transactionNumber)
                .Flush();

            return false;
        }

        process_response_transaction(data, client, reply, transaction, reason);
    }

    return true;
}

auto Server::process_process_box_response(
    Data& data,
    const api::session::Client& client,
    const Message& reply,
    const BoxType type,
    const identifier::Account& accountID,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());
    assert_false(nullptr == remote_nym_);

    update_nymbox_hash(data, reply);
    auto originalMessage = extract_message(reply.in_reference_to_, *Signer());

    if (false == bool(originalMessage)) { return false; }

    assert_false(nullptr == originalMessage);

    auto ledger =
        extract_ledger(originalMessage->payload_, accountID, *Signer());

    if (false == bool(ledger)) { return false; }

    auto responseLedger =
        extract_ledger(reply.payload_, accountID, *remote_nym_);

    if (false == bool(responseLedger)) { return false; }

    std::shared_ptr<OTTransaction> transaction;
    std::shared_ptr<OTTransaction> replyTransaction;

    if (BoxType::Inbox == type) {
        process_process_inbox_response(
            data,
            client,
            reply,
            *ledger,
            *responseLedger,
            transaction,
            replyTransaction,
            reason);
    } else {
        process_process_nymbox_response(
            data,
            reply,
            *ledger,
            *responseLedger,
            transaction,
            replyTransaction,
            reason);
    }

    if (false == (transaction && replyTransaction)) {
        LogError()()("Invalid transaction").Flush();

        return false;
    }

    const auto notaryID = String::Factory(server_id_, api_.Crypto());
    auto receiptID = String::Factory("NOT_SET_YET");
    auto replyItem =
        replyTransaction->GetItem(otx::itemType::atBalanceStatement);

    if (replyItem) {
        receiptID = reply.acct_id_;
    } else {
        replyItem =
            replyTransaction->GetItem(otx::itemType::atTransactionStatement);

        if (replyItem) { Signer()->GetIdentifier(receiptID); }
    }

    auto serialized = String::Factory();
    replyTransaction->SaveContractRaw(serialized);
    auto filename =
        replyTransaction->GetSuccess()
            ? api::internal::Paths::GetFilenameSuccess(receiptID->Get())
            : api::internal::Paths::GetFilenameFail(receiptID->Get());

    auto encoded = String::Factory();
    auto armored = Armored::Factory(api_.Crypto(), serialized);

    if (false == armored->WriteArmoredString(encoded, "TRANSACTION")) {
        LogError()()("Failed to encode receipt").Flush();

        return false;
    }

    if (replyItem) {
        OTDB::StorePlainString(
            api_,
            encoded->Get(),
            api_.DataFolder().string(),
            api_.Internal().Paths().Receipt(),
            notaryID->Get(),
            filename,
            "");
    } else {
        // This should never happen...
        filename = api::internal::Paths::GetFilenameError(receiptID->Get());
        LogError()()("Error saving transaction receipt: ")(notaryID.get())('/')(
            filename)(".")
            .Flush();
        OTDB::StorePlainString(
            api_,
            encoded->Get(),
            api_.DataFolder().string(),
            api_.Internal().Paths().Receipt(),
            notaryID->Get(),
            filename,
            "");
    }

    return true;
}

auto Server::process_process_inbox_response(
    Data& data,
    const api::session::Client& client,
    const Message& reply,
    Ledger& ledger,
    Ledger& responseLedger,
    std::shared_ptr<OTTransaction>& transaction,
    std::shared_ptr<OTTransaction>& replyTransaction,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    const auto accountID =
        api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes());
    transaction = ledger.GetTransaction(otx::transactionType::processInbox);
    replyTransaction =
        responseLedger.GetTransaction(otx::transactionType::atProcessInbox);

    if (false == (transaction && replyTransaction)) { return false; }

    const auto number = transaction->GetTransactionNum();
    const bool isIssued = verify_issued_number(data, number);

    if (false == isIssued) {
        LogError()()("This reply has already been processed.").Flush();

        return true;
    }

    consume_issued(data, transaction->GetTransactionNum());
    auto inbox = load_account_inbox(accountID);

    assert_false(nullptr == inbox);

    auto recordBox = load_or_create_account_recordbox(accountID, reason);

    assert_false(nullptr == recordBox);

    for (auto& pReplyItem : replyTransaction->GetItemList()) {
        if (false == bool(pReplyItem)) {
            LogError()()("Invalid reply item").Flush();

            return false;
        }

        auto& replyItem = *pReplyItem;
        auto replyItemType = String::Factory();
        replyItem.GetTypeString(replyItemType);
        otx::itemType requestType{otx::itemType::error_state};

        switch (replyItem.GetType()) {
            case otx::itemType::atAcceptPending: {
                requestType = otx::itemType::acceptPending;
            } break;
            case otx::itemType::atAcceptCronReceipt: {
                requestType = otx::itemType::acceptCronReceipt;
            } break;
            case otx::itemType::atAcceptItemReceipt: {
                requestType = otx::itemType::acceptItemReceipt;
            } break;
            case otx::itemType::atAcceptFinalReceipt: {
                requestType = otx::itemType::acceptFinalReceipt;
            } break;
            case otx::itemType::atAcceptBasketReceipt: {
                requestType = otx::itemType::acceptBasketReceipt;
            } break;
            case otx::itemType::atRejectPending:
            case otx::itemType::atDisputeCronReceipt:
            case otx::itemType::atDisputeItemReceipt:
            case otx::itemType::atDisputeFinalReceipt:
            case otx::itemType::atDisputeBasketReceipt:
            case otx::itemType::atBalanceStatement:
            case otx::itemType::atTransactionStatement: {
                continue;
            }
            default: {
                LogError()()("Unexpected reply item type (")(
                    replyItemType.get())(") in processInboxResponse")
                    .Flush();

                continue;
            }
        }

        if (Item::acknowledgement != replyItem.GetStatus()) {
            LogError()()("processInboxResponse reply item ")(
                replyItemType.get())(": status == FAILED.")
                .Flush();

            continue;
        }

        LogDetail()()("processInboxResponse reply item ")(replyItemType.get())(
            ": status == SUCCESS")
            .Flush();

        auto serializedOriginalItem = String::Factory();
        replyItem.GetReferenceString(serializedOriginalItem);
        auto pReferenceItem = api_.Factory().Internal().Session().Item(
            serializedOriginalItem, server_id_, replyItem.GetReferenceToNum());
        auto pItem = (pReferenceItem) ? transaction->GetItemInRefTo(
                                            pReferenceItem->GetReferenceToNum())
                                      : nullptr;

        if (false == bool(pItem)) {
            LogError()()(
                "Unable to find original item in original processInbox "
                "transaction request, based on reply item.")
                .Flush();

            continue;
        }

        const auto& referenceItem = *pReferenceItem;
        const auto& item = *pItem;

        if (item.GetType() != requestType) {
            LogError()()(
                "Wrong original item TYPE, on reply item's copy of original "
                "item, than what was expected based on reply item's type.")
                .Flush();

            continue;
        }

        // TODO here: any other verification of item against referenceItem,
        // which are supposedly copies of the same item.

        LogDetail()()(
            "Checking client-side inbox for expected pending or receipt "
            "transaction: ")(item.GetReferenceToNum())("...")
            .Flush();
        auto pInboxTransaction =
            inbox->GetTransaction(item.GetReferenceToNum());

        if (false == bool(pInboxTransaction)) {
            LogError()()(
                "Unable to find the server's receipt, in my inbox, that my "
                "original processInbox's item was referring to.")
                .Flush();

            continue;
        }

        auto& inboxTransaction = *pInboxTransaction;
        const auto inboxNumber = inboxTransaction.GetTransactionNum();

        switch (replyItem.GetType()) {
            case otx::itemType::atAcceptPending: {
                process_accept_pending_reply(
                    client, accountID, referenceItem, reply);
            } break;
            case otx::itemType::atAcceptItemReceipt: {
                process_accept_item_receipt_reply(
                    data, client, accountID, reply, inboxTransaction);
            } break;
            case otx::itemType::atAcceptCronReceipt: {
                process_accept_cron_receipt_reply(accountID, inboxTransaction);
            } break;
            case otx::itemType::atAcceptFinalReceipt: {
                process_accept_final_receipt_reply(data, inboxTransaction);
            } break;
            case otx::itemType::atAcceptBasketReceipt: {
                process_accept_basket_receipt_reply(data, inboxTransaction);
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
                LogError()()("Failed to save receipt ")(
                    inboxNumber)(" to record box")
                    .Flush();
            }
        } else {
            LogError()()("Unable to add transaction ")(
                inboxNumber)(" to record box (still removing it from asset "
                             "account inbox, however).")
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
    Data& data,
    const Message& reply,
    Ledger& ledger,
    Ledger& responseLedger,
    std::shared_ptr<OTTransaction>& transaction,
    std::shared_ptr<OTTransaction>& replyTransaction,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    transaction = ledger.GetTransaction(otx::transactionType::processNymbox);
    replyTransaction =
        responseLedger.GetTransaction(otx::transactionType::atProcessNymbox);

    if (false == (transaction && replyTransaction)) { return false; }

    accept_numbers(data, *transaction, *replyTransaction);
    auto nymbox =
        api_.Factory().Internal().Session().Ledger(nymID, nymID, server_id_);

    assert_false(nullptr == nymbox);

    bool loadedNymbox{true};
    loadedNymbox &= nymbox->LoadNymbox();
    loadedNymbox &= nymbox->VerifyAccount(nym);

    assert_true(loadedNymbox);

    for (const auto& item : replyTransaction->GetItemList()) {
        assert_false(nullptr == item);

        remove_nymbox_item(data, *item, *nymbox, *transaction, reason);
    }

    nymbox->ReleaseSignatures();
    nymbox->SignContract(nym, reason);
    nymbox->SaveContract();
    auto nymboxHash = identifier::Generic{};
    nymbox->SaveNymbox(nymboxHash);
    set_local_nymbox_hash(data, nymboxHash);

    return true;
}

auto Server::process_register_account_response(
    Data& data,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    update_nymbox_hash(data, reply);
    const auto accountID =
        api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes());

    if (reply.payload_->empty()) {
        LogError()()("Server reply does not contain an account").Flush();

        return false;
    }

    auto serialized = String::Factory(reply.payload_);
    const auto updated = api_.Wallet().Internal().UpdateAccount(
        accountID, *this, serialized, std::get<0>(data.pending_args_), reason);

    if (updated) {
        LogDetail()()("Saved new issuer account.").Flush();

        return init_new_account(accountID, reason);
    }

    LogError()()("Failed to save account.").Flush();

    return false;
}

auto Server::process_request_admin_response(Data& data, const Message& reply)
    -> bool
{
    update_nymbox_hash(data, reply);
    data.admin_attempted_->On();
    const auto& serverID = reply.notary_id_;

    if (reply.bool_) {
        LogDetail()()("Became admin on server ")(serverID.get()).Flush();
        data.admin_success_->On();
    } else {
        LogError()()("Server ")(serverID.get())(" rejected admin request")
            .Flush();
    }

    return true;
}

auto Server::process_register_nym_response(
    Data& data,
    const api::session::Client& client,
    const Message& reply) -> bool
{
    auto serialized =
        protobuf::Factory<protobuf::Context>(ByteArray{reply.payload_});
    auto verified = protobuf::syntax::check(LogError(), serialized);

    if (false == verified) {
        LogError()()("Invalid context.").Flush();

        return false;
    }

    auto output = resync(data, serialized);
    output &= harvest_unused(data, client);

    return output;
}

auto Server::process_reply(
    Data& data,
    const api::session::Client& client,
    const UnallocatedSet<otx::context::ManagedNumber>& managed,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    const auto accountID =
        api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes());
    const auto& serverNym = *remote_nym_;

    LogVerbose()()("Received ")(reply.command_.get())("(")(
        reply.success_ ? "success" : "failure")(")")
        .Flush();

    if (false == reply.VerifySignature(serverNym)) {
        LogError()()("Error: Server reply signature failed to verify.").Flush();

        return false;
    }

    const RequestNumber requestNumber = reply.request_num_->ToLong();
    add_acknowledged_number(data, requestNumber);
    RequestNumbers acknowledgedReplies{};

    if (reply.acknowledged_replies_.Output(acknowledgedReplies)) {
        remove_acknowledged_number(data, acknowledgedReplies);
    }

    if (reply.success_) {
        for (const auto& number : managed) { number.SetSuccess(true); }
    } else {
        LogVerbose()()("Message status: failed for ")(reply.command_.get())
            .Flush();

        return false;
    }

    switch (Message::Type(reply.command_->Get())) {
        case otx::MessageType::checkNymResponse: {
            return process_check_nym_response(data, client, reply);
        }
        case otx::MessageType::getAccountDataResponse: {
            return process_get_account_data(data, reply, reason);
        }
        case otx::MessageType::getBoxReceiptResponse: {
            return process_get_box_receipt_response(
                data, client, reply, reason);
        }
        case otx::MessageType::getInstrumentDefinitionResponse: {
            return process_get_unit_definition_response(data, reply);
        }
        case otx::MessageType::getMarketListResponse: {
            return process_get_market_list_response(reply);
        }
        case otx::MessageType::getMarketOffersResponse: {
            return process_get_market_offers_response(reply);
        }
        case otx::MessageType::getMarketRecentTradesResponse: {
            return process_get_market_recent_trades_response(reply);
        }
        case otx::MessageType::getMintResponse: {
            return process_get_mint_response(reply);
        }
        case otx::MessageType::getNymboxResponse: {
            return process_get_nymbox_response(data, reply, reason);
        }
        case otx::MessageType::getNymMarketOffersResponse: {
            return process_get_nym_market_offers_response(reply);
        }
        case otx::MessageType::notarizeTransactionResponse: {
            return process_notarize_transaction_response(
                data, client, reply, reason);
        }
        case otx::MessageType::processInboxResponse: {
            return process_process_box_response(
                data,
                client,
                reply,
                BoxType::Inbox,
                api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes()),
                reason);
        }
        case otx::MessageType::processNymboxResponse: {
            return process_process_box_response(
                data,
                client,
                reply,
                BoxType::Nymbox,
                nym_to_account(nymID),
                reason);
        }
        case otx::MessageType::registerAccountResponse: {
            return process_register_account_response(data, reply, reason);
        }
        case otx::MessageType::requestAdminResponse: {
            return process_request_admin_response(data, reply);
        }
        case otx::MessageType::registerInstrumentDefinitionResponse: {
            return process_issue_unit_definition_response(data, reply, reason);
        }
        case otx::MessageType::registerNymResponse: {
            update_nymbox_hash(data, reply);
            data.revision_.store(nym.Revision());
            const auto& resync = std::get<1>(data.pending_args_);

            if (resync) {
                return process_register_nym_response(data, client, reply);
            }

            return true;
        }
        case otx::MessageType::triggerClauseResponse: {
            update_nymbox_hash(data, reply);
            return true;
        }
        case otx::MessageType::unregisterAccountResponse: {
            return process_unregister_account_response(reply, reason);
        }
        case otx::MessageType::unregisterNymResponse: {
            return process_unregister_nym_response(data, reply, reason);
        }
        default: {
            update_nymbox_hash(data, reply);

            return true;
        }
    }
}

auto Server::process_response_transaction(
    Data& data,
    const api::session::Client& client,
    const Message& reply,
    OTTransaction& response,
    const PasswordPrompt& reason) -> void
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    otx::itemType type{otx::itemType::error_state};

    if (Exit::Yes == get_item_type(response, type)) { return; }

    switch (response.GetType()) {
        case otx::transactionType::atDeposit: {
            process_response_transaction_deposit(
                data, client, reply, type, response, reason);
        } break;
        case otx::transactionType::atPayDividend: {
            process_response_transaction_pay_dividend(
                data, reply, type, response);
        } break;
        case otx::transactionType::atExchangeBasket: {
            process_response_transaction_exchange_basket(
                data, reply, type, response);
        } break;
        case otx::transactionType::atCancelCronItem: {
            process_response_transaction_cancel(data, reply, type, response);
        } break;
        case otx::transactionType::atWithdrawal: {
            process_response_transaction_withdrawal(
                data, client, reply, type, response, reason);
        } break;
        case otx::transactionType::atTransfer: {
            process_response_transaction_transfer(
                data, client, reply, type, response);
        } break;
        case otx::transactionType::atMarketOffer:
        case otx::transactionType::atPaymentPlan:
        case otx::transactionType::atSmartContract: {
            process_response_transaction_cron(
                data, reply, type, response, reason);
        } break;
        default:
            LogError()()("wrong transaction type: ")(response.GetTypeString())(
                ".")
                .Flush();
            break;
    }

    auto receiptID = String::Factory("ID_NOT_SET_YET");
    auto pItem = response.GetItem(otx::itemType::atBalanceStatement);

    if (pItem) {
        receiptID->Set(reply.acct_id_);
    } else {
        pItem = response.GetItem(otx::itemType::atTransactionStatement);

        if (pItem) {
            nym.GetIdentifier(receiptID);
        } else {
            LogError()()(
                "Response transaction contains neither balance statement nor "
                "transaction statement")
                .Flush();
        }
    }

    auto serialized = String::Factory();
    response.SaveContractRaw(serialized);
    auto armored = Armored::Factory(api_.Crypto(), serialized);
    auto encoded = String::Factory();

    if (false == armored->WriteArmoredString(encoded, "TRANSACTION")) {
        LogError()()("Error saving transaction receipt "
                     "(failed writing armored string): ")(
            api_.Internal().Paths().Receipt())('/')(server_id_, api_.Crypto())(
            '/')(receiptID.get())
            .Flush();

        return;
    }

    if (pItem) {
        auto filename =
            response.GetSuccess()
                ? api::internal::Paths::GetFilenameSuccess(receiptID->Get())
                : api::internal::Paths::GetFilenameFail(receiptID->Get());
        OTDB::StorePlainString(
            api_,
            encoded->Get(),
            api_.DataFolder().string(),
            api_.Internal().Paths().Receipt(),
            server_id_.asBase58(api_.Crypto()),
            filename,
            "");
    } else {
        auto filename =
            api::internal::Paths::GetFilenameError(receiptID->Get());
        OTDB::StorePlainString(
            api_,
            encoded->Get(),
            api_.DataFolder().string(),
            api_.Internal().Paths().Receipt(),
            server_id_.asBase58(api_.Crypto()),
            filename,
            "");
    }
}

auto Server::process_response_transaction_cancel(
    Data& data,
    const Message& reply,
    const otx::itemType type,
    OTTransaction& response) -> void
{
    consume_issued(data, response.GetTransactionNum());
    auto item = response.GetItem(type);

    if (item && Item::acknowledgement == item->GetStatus()) {
        auto originalItem = extract_original_item(*item);

        if (originalItem) {
            const auto originalNumber = originalItem->GetReferenceToNum();

            if (false == consume_issued(data, originalNumber)) {
                LogError()()(
                    "(atCancelCronItem) Error removing issued number from user "
                    "nym.")
                    .Flush();
            }
        }
    }
}

auto Server::process_response_transaction_cash_deposit(
    Item& replyItem,
    const PasswordPrompt& reason) -> void
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    auto serializedRequest = String::Factory();
    replyItem.GetReferenceString(serializedRequest);

    if (serializedRequest->empty()) {
        LogError()()("Failed to get original serialized request").Flush();

        return;
    }

    auto pItem = api_.Factory().Internal().Session().Item(serializedRequest);

    if (false == bool(pItem)) {
        LogError()()("Failed to instantiate request").Flush();

        return;
    }

    const auto& item = *pItem;
    auto rawPurse = ByteArray{};
    item.GetAttachment(rawPurse);
    const auto serializedPurse = protobuf::Factory<protobuf::Purse>(rawPurse);

    if (false == protobuf::syntax::check(LogError(), serializedPurse)) {
        LogError()()("Invalid purse").Flush();

        return;
    }

    auto purse = api_.Factory().Internal().Session().Purse(serializedPurse);

    if (false == bool(purse)) {
        LogError()()("Failed to instantiate request purse").Flush();

        return;
    }

    if (false == purse.Unlock(nym, reason)) {
        LogError()()("Failed to unlock request purse").Flush();

        return;
    }

    UnallocatedSet<UnallocatedCString> spentTokens{};
    UnallocatedVector<blind::Token> keepTokens{};

    for (const auto& token : purse) { spentTokens.insert(token.ID(reason)); }

    auto purseEditor = mutable_Purse(purse.Unit(), reason);
    auto& walletPurse = purseEditor.get();

    if (false == walletPurse.Unlock(nym, reason)) {
        LogError()()("Failed to unlock wallet purse").Flush();

        return;
    }

    auto token = walletPurse.Pop();

    while (token) {
        const auto id = token.ID(reason);

        if (1 == spentTokens.count(id)) {
            LogTrace()()("Removing spent token ")(id)(" from purse").Flush();
        } else {
            LogTrace()()("Retaining unspent token ")(id)(" in purse").Flush();
            keepTokens.emplace_back(std::move(token));
        }

        token = walletPurse.Pop();
    }

    for (auto& keepToken : keepTokens) {
        walletPurse.Push(std::move(keepToken), reason);
    }
}

auto Server::process_response_transaction_cheque_deposit(
    Data& data,
    const api::session::Client& client,
    const identifier::Account& accountID,
    const Message* reply,
    const Item& replyItem,
    const PasswordPrompt& reason) -> void
{
    auto empty = client.Factory().Internal().Session().Message();

    assert_false(nullptr == Signer());
    assert_false(nullptr == empty);

    const auto& request =
        (data.pending_message_) ? *data.pending_message_ : *empty;
    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    auto paymentInbox = load_or_create_payment_inbox(reason);

    if (false == bool(paymentInbox)) { return; }

    auto serializedOriginal = String::Factory();
    Item* pOriginal{nullptr};
    replyItem.GetReferenceString(serializedOriginal);
    auto instantiatedOriginal =
        api_.Factory().Internal().Session().Transaction(serializedOriginal);

    if (false == bool(instantiatedOriginal)) {
        LogError()()("Failed to deserialized original item").Flush();

        return;
    }

    pOriginal = dynamic_cast<Item*>(instantiatedOriginal.get());

    if (nullptr == pOriginal) {
        LogError()()("Incorrect transaction type on original item").Flush();

        return;
    }

    auto& originalItem = *pOriginal;
    auto pCheque = api_.Factory().Internal().Session().Cheque();

    if (false == bool(pCheque)) { return; }

    auto& cheque = *pCheque;
    auto serializedCheque = String::Factory();
    originalItem.GetAttachment(serializedCheque);

    if (false == cheque.LoadContractFromString(serializedCheque)) {
        LogError()()("ERROR loading cheque from string: ")(
            serializedCheque.get())(".")
            .Flush();

        return;
    }

    if (0 < cheque.GetAmount()) {
        // Cheque or voucher
        const auto workflowUpdated = client.Workflow().DepositCheque(
            nymID, accountID, cheque, request, reply);

        if (workflowUpdated) {
            LogDetail()()("Successfully updated workflow.").Flush();
        } else {
            LogError()()("Failed to update workflow.").Flush();
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
                LogError()()(
                    "Failed trying to delete the box receipt for a cheque "
                    "being removed from a payments inbox: ")(
                    lRemoveTransaction)(".")
                    .Flush();
            }

            if (paymentInbox->RemoveTransaction(lRemoveTransaction)) {
                paymentInbox->ReleaseSignatures();
                paymentInbox->SignContract(nym, reason);
                paymentInbox->SaveContract();

                if (!paymentInbox->SavePaymentInbox()) {
                    LogError()()("Failure while trying to save payment inbox.")
                        .Flush();
                } else {
                    LogConsole()()(
                        "Removed cheque from payments inbox. (Deposited "
                        "successfully). Saved payments inbox.")
                        .Flush();
                }
            }
        }

        if (!strPmntInboxTransaction->Exists()) { continue; }

        // TODO how many record boxes should exist? One per nym, or one per
        // account?
        auto recordBox =
            load_or_create_account_recordbox(nym_to_account(nymID), reason);

        assert_false(nullptr == recordBox);

        add_transaction_to_ledger(
            lRemoveTransaction, pTransaction, *recordBox, reason);
    }
}

auto Server::process_response_transaction_cron(
    Data& data,
    const Message& reply,
    const otx::itemType type,
    OTTransaction& response,
    const PasswordPrompt& reason) -> void
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    auto pReplyItem = response.GetItem(type);

    if (false == bool(pReplyItem)) {
        LogError()()("Reply item is missing").Flush();

        return;
    }

    auto& replyItem = *pReplyItem;
    auto pOriginalItem = extract_original_item(replyItem);

    if (false == bool(pOriginalItem)) { return; }

    auto& originalItem = *pOriginalItem;
    auto serialized = String::Factory();
    originalItem.GetAttachment(serialized);

    if (serialized->empty()) {
        LogError()()("Original item does not contain serialized cron item")
            .Flush();

        return;
    }

    auto pCronItem = api_.Factory().Internal().Session().CronItem(serialized);

    if (false == bool(pCronItem)) {
        LogError()()("Failed to instantiate serialized cron item").Flush();

        return;
    }

    auto& cronItem = *pCronItem;
    const auto openingNumber = response.GetTransactionNum();

    if (Item::rejection == replyItem.GetStatus()) {
        consume_issued(data, openingNumber);

        if (nym.CompareID(cronItem.GetSenderNymID())) {
            const auto count = cronItem.GetCountClosingNumbers();

            for (int i = 0; i < count; ++i) {
                const auto number = cronItem.GetClosingTransactionNoAt(i);
                recover_available_number(data, number);
            }
        }
    }

    switch (response.GetType()) {
        case otx::transactionType::atPaymentPlan:
        case otx::transactionType::atSmartContract: {
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
            LogError()()(
                "Failed trying to remove outpayment with transaction num: ")(
                openingNumber)(".")
                .Flush();
        }
        if (!pMsg->payload_->GetString(strInstrument)) {
            LogError()()(
                "Unable to find payment instrument in outpayment message with "
                "transaction num: ")(openingNumber)(".")
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
            api_.Internal().Paths().PaymentInbox(),
            server_id_.asBase58(api_.Crypto()),
            nymID.asBase58(api_.Crypto()),
            "");
        const bool bExists2 = OTDB::Exists(
            api_,
            api_.DataFolder().string(),
            api_.Internal().Paths().RecordBox(),
            server_id_.asBase58(api_.Crypto()),
            nymID.asBase58(api_.Crypto()),
            "");

        auto thePmntInbox = api_.Factory().Internal().Session().Ledger(
            nymID,
            nymID,
            server_id_);  // payment inbox

        assert_false(nullptr == thePmntInbox);

        auto theRecordBox = api_.Factory().Internal().Session().Ledger(
            nymID,
            nymID,
            server_id_);  // record box

        assert_false(nullptr == theRecordBox);

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
                otx::ledgerType::paymentInbox,

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
                otx::ledgerType::recordBox,

                true);  // bGenerateFile=true
        }
        // By this point, the boxes DEFINITELY exist -- or not. (generation
        // might have failed, or verification.)
        //
        if (!bSuccessLoading1 || !bSuccessLoading2) {
            LogConsole()()(
                "While processing server reply containing rejection of cron "
                "item: WARNING: Unable to load, verify, or generate "
                "paymentInbox or recordBox, with IDs: ")(nymID, api_.Crypto())(
                " / ")(nymID, api_.Crypto())(".")
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

            auto theOutpayment = api_.Factory().Internal().Session().Payment();

            assert_false(nullptr == theOutpayment);

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
                    LogConsole()()(
                        "While looping payments inbox to remove a payment, "
                        "unable to retrieve payment on receipt ")(
                        receipt_id)(" (skipping).")
                        .Flush();
                    continue;
                } else if (false == pPayment->SetTempValues(reason)) {
                    LogConsole()()(
                        "While looping payments inbox to remove a payment, "
                        "unable to set temp values for payment on receipt ")(
                        receipt_id)(" (skipping).")
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
                    assert_false(nullptr == pTransPaymentInbox);

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
                        LogError()()(
                            "Failed trying to delete the box receipt for a "
                            "transaction being removed from the payment inbox.")
                            .Flush();
                    }
                    if (thePmntInbox->RemoveTransaction(receipt_id)) {
                        thePmntInbox->ReleaseSignatures();
                        thePmntInbox->SignContract(nym, reason);
                        thePmntInbox->SaveContract();

                        if (!thePmntInbox->SavePaymentInbox()) {
                            LogError()()(
                                "Failure while trying to save payment inbox.")
                                .Flush();
                        } else {
                            LogConsole()()(
                                "Removed instrument from payment inbox. Saved "
                                "payment inbox.")
                                .Flush();
                        }
                    } else {
                        LogError()()(
                            "Failed trying to remove transaction from payment "
                            "inbox. (Should never happen).")
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
                otx::originType theOriginType = otx::originType::not_applicable;

                if (theOutpayment->IsValid()) {
                    if (theOutpayment->IsPaymentPlan()) {
                        theOriginType = otx::originType::origin_payment_plan;
                    } else if (theOutpayment->IsSmartContract()) {
                        theOriginType = otx::originType::origin_smart_contract;
                    }
                }

                auto pNewTransaction =
                    api_.Factory().Internal().Session().Transaction(
                        *theRecordBox,  // recordbox.
                        otx::transactionType::notice,
                        theOriginType,
                        openingNumber);

                if (pNewTransaction) {
                    // Whether the reply item we received was acknowledged
                    // or rejected, we create a corresponding
                    // otx::itemType::notice for our new record, to save that
                    // state for the client. Our record box will contain the
                    // server's most recent version of the payment plan,
                    // (The one I just activated -- since I was the final
                    // signer...)
                    //
                    auto pNewItem = api_.Factory().Internal().Session().Item(
                        *pNewTransaction,
                        otx::itemType::notice,
                        identifier::Account{});
                    assert_false(nullptr == pNewItem);
                    // This may be unnecessary, I'll have to check
                    // CreateItemFromTransaction.
                    // I'll leave it for now.
                    pNewItem->SetStatus(replyItem.GetStatus());
                    // Since I am the last signer, the note contains the
                    // final version of the agreement.
                    pNewItem->SetNote(serialized);
                    pNewItem->SignContract(nym, reason);
                    pNewItem->SaveContract();

                    const std::shared_ptr<Item> newItem{pNewItem.release()};
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

                    const std::shared_ptr<OTTransaction> newTransaction{
                        pNewTransaction.release()};
                    const bool bAdded =
                        theRecordBox->AddTransaction(newTransaction);

                    if (!bAdded) {
                        LogError()()("Unable to add transaction ")(
                            newTransaction->GetTransactionNum())(
                            " to record box (after tentatively removing from "
                            "payment outbox, an action that is now cancelled).")
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
                            LogError()()("For Record Box... Failed trying to "
                                         "SaveBoxReceipt. Contents: ")(
                                strNewTransaction.get())(".")
                                .Flush();
                        }
                    }
                }     // if (nullptr != pNewTransaction)
                else  // should never happen
                {
                    LogError()()(
                        "Failed while trying to generate transaction in order "
                        "to add a new transaction to record box (for a payment "
                        "instrument we just removed from the outpayments "
                        "box): ")(nymID, api_.Crypto())(".")
                        .Flush();
                }
            }  // if (strInstrument.Exists())
               // (then add a copy to record box.)
        }      // else (Success loading the payment inbox and recordBox)
    }          // (Item::rejection == replyItem.GetStatus())
               // (loading payment inbox and record box.)
}

auto Server::process_response_transaction_deposit(
    Data& data,
    const api::session::Client& client,
    const Message& reply,
    const otx::itemType type,
    OTTransaction& response,
    const PasswordPrompt& reason) -> void
{
    for (auto& it : response.GetItemList()) {
        auto pReplyItem = it;

        assert_false(nullptr == pReplyItem);

        auto& item = *pReplyItem;
        // if pointer not null, and it's a deposit, and it's an
        // acknowledgement (not a rejection or error)

        if ((otx::itemType::atDeposit == item.GetType()) ||
            (otx::itemType::atDepositCheque == item.GetType())) {
            if (Item::acknowledgement == item.GetStatus()) {
                LogDetail()()("TRANSACTION SUCCESS -- Server acknowledges "
                              "deposit.")
                    .Flush();
                if (otx::itemType::atDepositCheque == item.GetType()) {
                    process_response_transaction_cheque_deposit(
                        data,
                        client,
                        response.GetPurportedAccountID(),
                        &reply,
                        item,
                        reason);
                } else if (otx::itemType::atDeposit == item.GetType()) {
                    process_response_transaction_cash_deposit(item, reason);
                }
            } else {
                LogError()()("TRANSACTION FAILURE -- Server rejects deposit.")
                    .Flush();
            }
        }
    }

    consume_issued(data, response.GetTransactionNum());
}

auto Server::process_response_transaction_exchange_basket(
    Data& data,
    const Message& reply,
    const otx::itemType type,
    OTTransaction& response) -> void
{
    consume_issued(data, response.GetTransactionNum());
    auto item = response.GetItem(type);

    if (item && Item::rejection == item->GetStatus()) {
        auto originalItem = extract_original_item(*item);

        if (originalItem) {
            auto serialized = String::Factory();
            originalItem->GetAttachment(serialized);

            if (serialized->empty()) {
                LogError()()("Original item does not contain serialized basket")
                    .Flush();

                return;
            }

            auto pBasket = api_.Factory().Internal().Session().Basket();

            assert_false(nullptr == pBasket);

            auto& basket = *pBasket;

            if (false == basket.LoadContractFromString(serialized)) {
                LogError()()("Failed to instantiate basket").Flush();

                return;
            }

            for (auto i = 0; i < basket.Count(); ++i) {
                auto* requestItem = basket.At(i);

                if (nullptr == requestItem) {
                    LogError()()("Invalid basket").Flush();

                    return;
                }

                const auto closingNumber = requestItem->closing_transaction_no_;
                recover_available_number(data, closingNumber);
            }

            const auto closingNumber = basket.GetClosingNum();
            recover_available_number(data, closingNumber);
        }
    }
}

auto Server::process_response_transaction_pay_dividend(
    Data& data,
    const Message& reply,
    const otx::itemType type,
    OTTransaction& response) -> void
{
    // loop through the ALL items that make up this transaction and check to
    // see if a response to pay dividend.

    for (auto& it : response.GetItemList()) {
        auto pItem = it;
        assert_false(nullptr == pItem);

        // if pointer not null, and it's a dividend payout, and it's an
        // acknowledgement (not a rejection or error)

        if (otx::itemType::atPayDividend == pItem->GetType()) {
            if (Item::acknowledgement == pItem->GetStatus()) {
                LogConsole()()("TRANSACTION SUCCESS -- Server acknowledges "
                               "dividend "
                               "payout.")
                    .Flush();
            } else {
                LogConsole()()("TRANSACTION FAILURE -- Server rejects dividend "
                               "payout.")
                    .Flush();
            }
        }
    }

    consume_issued(data, response.GetTransactionNum());
}

auto Server::process_response_transaction_transfer(
    Data& data,
    const api::session::Client& client,
    const Message& reply,
    const otx::itemType type,
    OTTransaction& response) -> void
{
    assert_false(nullptr == Signer());

    const auto& nymID = Signer()->ID();
    auto pItem = response.GetItem(type);

    if (false == bool(pItem)) {
        LogError()()(
            "Response transaction does not contain response transfer item")
            .Flush();

        return;
    }

    const auto& responseItem = *pItem;
    auto status{TransactionAttempt::Accepted};

    if (Item::rejection == pItem->GetStatus()) {
        consume_issued(data, response.GetTransactionNum());
        status = TransactionAttempt::Rejected;
    }

    auto serialized = String::Factory();
    responseItem.GetReferenceString(serialized);

    if (serialized->empty()) {
        LogError()()("Invalid serialized transaction item.").Flush();

        return;
    }

    auto pTransfer = api_.Factory().Internal().Session().Item(
        serialized,
        responseItem.GetRealNotaryID(),
        responseItem.GetReferenceToNum());

    if (false == bool(pTransfer)) {
        LogError()()("Failed to instantiate transaction item.").Flush();

        return;
    }

    const auto& transfer = *pTransfer;

    if (is_internal_transfer(transfer)) {
        LogDetail()()("Acknowledging internal transfer").Flush();
    } else {
        LogDetail()()("Acknowledging outgoing transfer").Flush();
    }

    if (TransactionAttempt::Accepted == status) {
        client.Workflow().AcknowledgeTransfer(nymID, transfer, reply);
    } else {
        client.Workflow().AbortTransfer(nymID, transfer, reply);
    }
}

auto Server::process_response_transaction_withdrawal(
    Data& data,
    const api::session::Client& client,
    const Message& reply,
    const otx::itemType type,
    OTTransaction& response,
    const PasswordPrompt& reason) -> void
{
    assert_false(nullptr == Signer());

    // loop through the ALL items that make up this transaction and check to
    // see if a response to withdrawal.

    // if pointer not null, and it's a withdrawal, and it's an
    // acknowledgement (not a rejection or error)
    for (auto& it : response.GetItemList()) {
        auto pItem = it;
        assert_false(nullptr == pItem);
        // VOUCHER WITHDRAWAL
        //
        // If we got a reply to a voucher withdrawal, we'll just display the
        // voucher on the screen (if the server sent us one...)
        if ((otx::itemType::atWithdrawVoucher == pItem->GetType()) &&
            (Item::acknowledgement == pItem->GetStatus())) {
            auto strVoucher = String::Factory();
            auto theVoucher = api_.Factory().Internal().Session().Cheque();

            assert_false(nullptr == theVoucher);

            pItem->GetAttachment(strVoucher);

            if (theVoucher->LoadContractFromString(strVoucher)) {
                LogVerbose()()("Received voucher from server:  ").Flush();
                LogVerbose()()(strVoucher.get()).Flush();
            }
        }
        // CASH WITHDRAWAL
        //
        // If the item is a response to a cash withdrawal, we want to save
        // the coins into a purse somewhere on the computer. That's cash!
        // Gotta keep it safe.
        else if (
            (otx::itemType::atWithdrawal == pItem->GetType()) &&
            (Item::acknowledgement == pItem->GetStatus())) {
            process_incoming_cash_withdrawal(*pItem, reason);
        }
    }

    consume_issued(data, response.GetTransactionNum());
}

auto Server::process_incoming_cash(
    const api::session::Client& client,
    const TransactionNumber number,
    const PeerObject& incoming,
    const Message& message) const -> bool
{
    assert_false(nullptr == Signer());
    assert_true(contract::peer::ObjectType::Cash == incoming.Type());

    if (false == incoming.Validate()) {
        LogError()()("Invalid peer object.").Flush();

        return false;
    }

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    const auto& serverID = server_id_;
    const auto strNotaryID = String::Factory(serverID, api_.Crypto());
    const auto strNymID = String::Factory(nymID, api_.Crypto());
    const auto& purse = incoming.Purse();

    if (false == bool(purse)) {
        LogError()()("No purse found").Flush();

        return false;
    }

    const auto workflowID =
        client.Workflow().ReceiveCash(nymID, purse, message);

    if (workflowID.empty()) {
        LogError()()("Failed to create workflow").Flush();

        return false;
    }

    LogDetail()()("Created workflow ")(workflowID, api_.Crypto()).Flush();

    return true;
}

auto Server::process_incoming_cash_withdrawal(
    const Item& item,
    const PasswordPrompt& reason) const -> void
{
    assert_false(nullptr == Signer());
    assert_false(nullptr == remote_nym_);

    const auto& nym = *Signer();
    const auto& serverNym = *remote_nym_;
    const auto& nymID = nym.ID();
    auto rawPurse = ByteArray{};
    item.GetAttachment(rawPurse);
    const auto serializedPurse = protobuf::Factory<protobuf::Purse>(rawPurse);

    if (false == protobuf::syntax::check(LogError(), serializedPurse)) {
        LogError()()("Invalid serialized purse").Flush();

        return;
    }

    LogInsane()()("Serialized purse is valid").Flush();
    auto requestPurse =
        api_.Factory().Internal().Session().Purse(serializedPurse);

    if (requestPurse) {
        LogInsane()()("Purse instantiated").Flush();
    } else {
        LogError()()("Failed to instantiate purse").Flush();

        return;
    }

    if (requestPurse.Unlock(nym, reason)) {
        LogInsane()()("Purse unlocked").Flush();
    } else {
        LogError()()("Failed to unlock purse").Flush();

        return;
    }

    auto pMint = api_.Factory().Mint(server_id_, requestPurse.Unit());

    if (pMint) {
        LogInsane()()("Mint loaded").Flush();
    } else {
        LogError()()("Failed to load mint").Flush();

        return;
    }

    auto& mint = pMint.Internal();
    const bool validMint = mint.LoadMint({}) && mint.VerifyMint(serverNym);

    if (validMint) {
        LogInsane()()("Mint is valid").Flush();
    } else {
        LogError()()("Invalid mint").Flush();

        return;
    }

    const auto processed = requestPurse.Internal().Process(nym, pMint, reason);

    if (processed) {
        LogInsane()()("Token processed").Flush();
        auto purseEditor = api_.Wallet().Internal().mutable_Purse(
            nymID, requestPurse.Notary(), requestPurse.Unit(), reason);
        auto& walletPurse = purseEditor.get();

        assert_true(walletPurse.Unlock(nym, reason));

        auto token = requestPurse.Pop();

        while (token) {
            walletPurse.Push(std::move(token), reason);
            token = requestPurse.Pop();
        }
    } else {
        LogError()()("Failed to process token").Flush();
    }
}

auto Server::process_unregister_account_response(
    const Message& reply,
    const PasswordPrompt& reason) const -> bool
{
    auto serialized = String::Factory();

    if (reply.in_reference_to_->Exists()) {
        reply.in_reference_to_->GetString(serialized);
    }

    auto originalMessage = api_.Factory().Internal().Session().Message();

    assert_false(nullptr == originalMessage);

    if (serialized->Exists() &&
        originalMessage->LoadContractFromString(serialized) &&
        originalMessage->VerifySignature(*Signer()) &&
        originalMessage->nym_id_->Compare(reply.nym_id_) &&
        originalMessage->acct_id_->Compare(reply.acct_id_) &&
        originalMessage->command_->Compare("unregisterAccount")) {
        const auto theAccountID =
            api_.Factory().AccountIDFromBase58(reply.acct_id_->Bytes());
        auto account =
            api_.Wallet().Internal().mutable_Account(theAccountID, reason);

        if (account) {
            account.Release();
            api_.Wallet().DeleteAccount(theAccountID);
        }

        LogConsole()()("Successfully DELETED Asset Acct ")(
            reply.acct_id_.get())(" from Server: ")(server_id_, api_.Crypto())(
            ".")
            .Flush();
    } else {
        LogError()()("The server just for some reason tried "
                     "to trick me into erasing my account ")(
            reply.acct_id_.get())(" on Server ")(server_id_, api_.Crypto())(".")
            .Flush();
    }

    return true;
}

auto Server::process_unregister_nym_response(
    Data& data,
    const Message& reply,
    const PasswordPrompt& reason) -> bool
{
    auto serialized = String::Factory();
    auto originalMessage = api_.Factory().Internal().Session().Message();

    assert_false(nullptr == originalMessage);

    if (reply.in_reference_to_->Exists()) {
        reply.in_reference_to_->GetString(serialized);
    }

    if (serialized->Exists() &&
        originalMessage->LoadContractFromString(serialized) &&
        originalMessage->VerifySignature(*Signer()) &&
        originalMessage->nym_id_->Compare(reply.nym_id_) &&
        originalMessage->command_->Compare("unregisterNym")) {
        reset(data);
        LogConsole()()(
            "Successfully DELETED Nym from Server: removed request number, "
            "plus all issued and transaction numbers for Nym ")(
            reply.nym_id_.get())(" for Server ")(server_id_, api_.Crypto())(".")
            .Flush();
    } else {
        LogError()()(
            "The server just for some reason tried to trick me into erasing my "
            "issued and transaction numbers for Nym ")(reply.nym_id_.get())(
            ", Server ")(server_id_, api_.Crypto())(".")
            .Flush();
    }

    return true;
}

auto Server::process_unseen_reply(
    Data& data,
    const api::session::Client& client,
    const Item& input,
    ReplyNoticeOutcomes& notices,
    const PasswordPrompt& reason) -> void
{
    if (Item::acknowledgement != input.GetStatus()) {
        LogConsole()()("Reply item incorrect status").Flush();

        return;
    }

    auto serializedReply = String::Factory();
    input.GetAttachment(serializedReply);

    if (!serializedReply->Exists()) {
        LogError()()("Error loading original server reply message from notice.")
            .Flush();

        return;
    }

    const std::shared_ptr<Message> message{
        api_.Factory().Internal().Session().Message()};

    assert_false(nullptr == message);

    if (false == message->LoadContractFromString(serializedReply)) {
        LogError()()(
            "Failed loading original server reply message from replyNotice: ")(
            serializedReply.get())
            .Flush();

        return;
    }

    auto outcome = ReplyNoticeOutcome{};
    auto& [number, result] = outcome;
    auto& [status, reply] = result;
    number = String::StringToUlong(message->request_num_->Get());
    status = (message->success_) ? otx::LastReplyStatus::MessageSuccess
                                 : otx::LastReplyStatus::MessageFailed;
    reply = message;
    notices.emplace_back(outcome);
    process_reply(data, client, {}, *reply, reason);
}

auto Server::Purse(const identifier::UnitDefinition& id) const
    -> const opentxs::otx::blind::Purse&
{
    return api_.Wallet().Purse(Signer()->ID(), server_id_, id);
}

auto Server::Queue(
    const api::session::Client& client,
    std::shared_ptr<Message> message,
    const PasswordPrompt& reason,
    const ExtraArgs& args) -> Server::QueueResult
{
    START_SERVER_CONTEXT();

    return start(data, decisionLock, reason, client, message, args);
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
        LogError()()("Inbox is not instantiated").Flush();

        return {};
    }

    if (false == bool(outbox)) {
        LogError()()("Outbox is not instantiated").Flush();

        return {};
    }

    return start(
        data,
        decisionLock,
        reason,
        client,
        message,
        args,
        protobuf::DELIVERTYSTATE_PENDINGSEND,
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
        data,
        decisionLock,
        reason,
        client,
        nullptr,
        {},
        protobuf::DELIVERTYSTATE_NEEDNYMBOX,
        ActionType::ProcessNymbox);
}

auto Server::remove_acknowledged_number(Data& data, const Message& reply)
    -> bool
{
    UnallocatedSet<RequestNumber> list{};

    if (false == reply.acknowledged_replies_.Output(list)) { return false; }

    return remove_acknowledged_number(data, list);
}

auto Server::remove_nymbox_item(
    Data& data,
    const Item& replyItem,
    Ledger& nymbox,
    OTTransaction& transaction,
    const PasswordPrompt& reason) -> bool
{
    assert_false(nullptr == Signer());

    const auto& nym = *Signer();
    const auto& nymID = nym.ID();
    otx::itemType requestType = otx::itemType::error_state;
    auto replyType = String::Factory();
    replyItem.GetTypeString(replyType);

    switch (replyItem.GetType()) {
        case otx::itemType::atAcceptFinalReceipt: {
            requestType = otx::itemType::acceptFinalReceipt;
        } break;
        case otx::itemType::atAcceptMessage: {
            requestType = otx::itemType::acceptMessage;
        } break;
        case otx::itemType::atAcceptNotice: {
            requestType = otx::itemType::acceptNotice;
        } break;
        case otx::itemType::atAcceptTransaction: {
            requestType = otx::itemType::acceptTransaction;
        } break;
        case otx::itemType::atTransactionStatement: {
            // (The transaction statement itself is already handled

            return true;
        }
        default: {
            LogError()()("Unexpected replyItem type while processing Nymbox: ")(
                replyType.get())
                .Flush();

            return false;
        }
    }

    if (Item::acknowledgement != replyItem.GetStatus()) {
        LogDetail()()("processNymboxResponse reply item ")(replyType.get())(
            ": status == FAILED")
            .Flush();

        return true;
    }

    LogDetail()()("processNymboxResponse reply item ")(replyType.get())(
        ": status == SUCCESS")
        .Flush();
    auto serialized = String::Factory();
    replyItem.GetReferenceString(serialized);
    auto processNymboxItem =
        api_.Factory().Internal().Session().Item(serialized, server_id_, 0);

    if (false == bool(processNymboxItem)) {
        LogError()()("Unable to find original item in original processNymbox "
                     "transaction request, based on reply item.")
            .Flush();

        return false;
    }

    auto item =
        transaction.GetItemInRefTo(processNymboxItem->GetReferenceToNum());

    assert_false(nullptr == item);

    if (item->GetType() != requestType) {
        LogError()()(
            "Wrong original item TYPE, on reply item's copy of original "
            "item, than what was expected based on reply item's type.")
            .Flush();

        return false;
    }

    std::shared_ptr<OTTransaction> serverTransaction;

    switch (replyItem.GetType()) {
        case otx::itemType::atAcceptNotice:
        case otx::itemType::atAcceptMessage:
        case otx::itemType::atAcceptTransaction:
        case otx::itemType::atAcceptFinalReceipt: {
            serverTransaction =
                nymbox.GetTransaction(item->GetReferenceToNum());
        } break;
        default: {
            LogError()()("Unexpected replyItem::type while processing "
                         "Nymbox: ")(replyType.get())
                .Flush();

            return false;
        }
    }

    if (false == bool(serverTransaction)) {
        LogDetail()()(
            "The original processNymbox item referred to txn number ")(
            item->GetReferenceToNum())(
            ", but that receipt wasn't in my Nymbox. (We probably processed "
            "this server reply ALREADY, and now we're just seeing it again, "
            "since an extra copy was dropped into the Nymbox originally. It "
            "happens. Skipping).")
            .Flush();

        return true;
    }

    switch (replyItem.GetType()) {
        case otx::itemType::atAcceptNotice: {
            if (otx::transactionType::notice != serverTransaction->GetType()) {
                break;
            }

            if ((Item::rejection != replyItem.GetStatus()) &&
                (Item::acknowledgement != replyItem.GetStatus())) {
                break;
            }

            auto strOriginalCronItem = String::Factory();
            serverTransaction->GetReferenceString(strOriginalCronItem);
            const otx::originType theOriginType =
                serverTransaction->GetOriginType();
            auto strUpdatedCronItem = String::Factory();
            auto pNoticeItem =
                serverTransaction->GetItem(otx::itemType::notice);

            if ((pNoticeItem)) { pNoticeItem->GetNote(strUpdatedCronItem); }

            auto pOriginalCronItem =
                (strOriginalCronItem->Exists()
                     ? api_.Factory().Internal().Session().CronItem(
                           strOriginalCronItem)
                     : nullptr);
            auto pUpdatedCronItem =
                (strUpdatedCronItem->Exists()
                     ? api_.Factory().Internal().Session().CronItem(
                           strUpdatedCronItem)
                     : nullptr);
            std::unique_ptr<OTCronItem>& pCronItem =
                ((pUpdatedCronItem) ? pUpdatedCronItem : pOriginalCronItem);

            if (false == bool(pCronItem) || false == bool(pOriginalCronItem)) {
                LogError()()(
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
                    if (false == consume_issued(data, openingNumber)) {
                        LogError()()(
                            "Error removing issued number from user nym (for "
                            "a cron item).")
                            .Flush();
                    }

                    for (std::int32_t i = 0;
                         i < pOriginalCronItem->GetCountClosingNumbers();
                         ++i) {
                        recover_available_number(
                            data,
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
                        LogError()()(
                            "Failed trying to remove outpayment with trans "
                            "num: ")(openingNumber)(".")
                            .Flush();
                    }

                    if (!pMsg->payload_->GetString(strSentInstrument)) {
                        LogError()()(
                            "Unable to find payment "
                            "instrument in outpayment "
                            "message with trans num: ")(openingNumber)(".")
                            .Flush();
                    }
                }

                const auto notaryID =
                    String::Factory(server_id_, api_.Crypto());
                const bool exists1 = OTDB::Exists(
                    api_,
                    api_.DataFolder().string(),
                    api_.Internal().Paths().PaymentInbox(),
                    notaryID->Get(),
                    nymID.asBase58(api_.Crypto()),
                    "");
                const bool exists2 = OTDB::Exists(
                    api_,
                    api_.DataFolder().string(),
                    api_.Internal().Paths().RecordBox(),
                    notaryID->Get(),
                    nymID.asBase58(api_.Crypto()),
                    "");
                auto paymentInbox = api_.Factory().Internal().Session().Ledger(
                    nymID, nymID, server_id_);

                assert_false(nullptr == paymentInbox);

                auto recordBox = api_.Factory().Internal().Session().Ledger(
                    nymID, nymID, server_id_);

                assert_false(nullptr == recordBox);

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
                        otx::ledgerType::paymentInbox,

                        true);
                }

                if (exists2 && loaded2) {
                    loaded2 =
                        (recordBox->VerifyContractID() &&
                         recordBox->VerifySignature(nym));
                } else if (!exists2) {
                    loaded2 = recordBox->GenerateLedger(
                        nymID, server_id_, otx::ledgerType::recordBox, true);
                }

                if (!loaded1 || !loaded2) {
                    LogConsole()()(
                        "While processing server rejection of cron item: "
                        "WARNING: Unable to load, verify, or generate "
                        "paymentInbox or recordBox, with IDs: ")(
                        nymID, api_.Crypto())(" / ")(nymID, api_.Crypto())(".")
                        .Flush();
                } else {
                    auto theOutpayment =
                        api_.Factory().Internal().Session().Payment();

                    assert_false(nullptr == theOutpayment);

                    if (strSentInstrument->Exists() &&
                        theOutpayment->SetPayment(strSentInstrument) &&
                        theOutpayment->SetTempValues(reason)) {
                        theOutpayment->GetAllTransactionNumbers(
                            numlistOutpayment, reason);
                    }

                    auto tempPayment =
                        api_.Factory().Internal().Session().Payment();

                    assert_false(nullptr == tempPayment);

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
                            LogConsole()()("(Upon receiving notice) While "
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
                            LogConsole()()(
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

                            assert_false(nullptr == pTransPaymentInbox);

                            if (false ==
                                paymentInbox->DeleteBoxReceipt(receipt_id)) {
                                LogError()()(
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
                                    LogError()()("Failure while trying to save "
                                                 "payment inbox.")
                                        .Flush();
                                } else {
                                    LogConsole()()(
                                        "Removed instrument from payment "
                                        "inbox. Saved payment inbox.")
                                        .Flush();
                                }
                            } else {
                                LogError()()(
                                    "Failed trying to remove transaction "
                                    "from payment inbox. (Should never "
                                    "happen).")
                                    .Flush();
                            }
                        }
                    }

                    const std::shared_ptr<OTTransaction> newTransaction{
                        api_.Factory().Internal().Session().Transaction(
                            *recordBox,
                            otx::transactionType::notice,
                            theOriginType,
                            serverTransaction->GetTransactionNum())};

                    if (newTransaction) {
                        if (false != bool(pNoticeItem)) {
                            const std::shared_ptr<Item> newItem{
                                api_.Factory().Internal().Session().Item(
                                    *newTransaction,
                                    otx::itemType::notice,
                                    identifier::Account{})};

                            assert_false(nullptr == newItem);

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
                                api_.Factory().Internal().Session().Payment();

                            assert_true(false != bool(temp));

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
                            LogError()()("Unable to add txn ")(
                                newTransaction->GetTransactionNum())(
                                " to record box (after tentatively removing "
                                "from payment outbox, an action that is now "
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
                            LogError()()(
                                "For Record Box... Failed trying to "
                                "SaveBoxReceipt. "
                                "Contents: ")(strNewTransaction.get())(".")
                                .Flush();
                        }
                    } else {
                        LogError()()(
                            "Failed while trying to generate transaction in "
                            "order to add a new transaction to record box (for "
                            "a payment instrument we just removed from the "
                            "outpayments box): ")(nymID, api_.Crypto())(".")
                            .Flush();
                    }
                }
            }
        } break;
        case otx::itemType::atAcceptMessage:
        case otx::itemType::atAcceptTransaction: {
            // I don't think we need to do anything here...
        } break;
        case otx::itemType::atAcceptFinalReceipt: {
            LogVerbose()()(
                "Successfully removed finalReceipt from Nymbox with opening "
                "num: ")(serverTransaction->GetReferenceToNum())
                .Flush();
            const bool removed =
                consume_issued(data, serverTransaction->GetReferenceToNum());

            if (removed) {
                LogDetail()()(
                    "**** Due to finding a finalReceipt, consuming issued "
                    "opening number from nym:  ")(
                    serverTransaction->GetReferenceToNum())
                    .Flush();
            } else {
                LogDetail()()(
                    "**** Noticed a finalReceipt, but Opening Number ")(
                    serverTransaction->GetReferenceToNum())(
                    " had ALREADY been removed from nym.")
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
            LogError()()("Unexpected replyItem:type while processing Nymbox: ")(
                replyType.get())(".")
                .Flush();

            return false;
        }
    }

    serverTransaction->DeleteBoxReceipt(nymbox);
    nymbox.RemoveTransaction(serverTransaction->GetTransactionNum());

    return true;
}

auto Server::RemoveTentativeNumber(const TransactionNumber& number) -> bool
{
    return remove_tentative_number(*get_data(), number);
}

auto Server::remove_tentative_number(
    Data& data,
    const TransactionNumber& number) -> bool
{
    auto output = data.tentative_transaction_numbers_.erase(number);

    return (0 < output);
}

auto Server::Request(const ServerPrivate& data) const -> RequestNumber
{
    return data.request_number_;
}

auto Server::ResetThread() -> void { Join(); }

auto Server::resolve_queue(
    Data& data,
    DeliveryResult&& result,
    const PasswordPrompt& reason,
    const protobuf::DeliveryState state) -> void
{
    if (protobuf::DELIVERTYSTATE_ERROR != state) { data.state_.store(state); }

    data.last_status_.store(std::get<0>(result));
    data.inbox_.reset();
    data.outbox_.reset();
    data.pending_result_.set_value(std::move(result));
    data.pending_result_set_.store(true);
    data.pending_message_.reset();
    data.pending_args_ = {"", false};
    data.process_nymbox_.store(false);
    const auto saved = save(data, reason);

    assert_true(saved);
}

auto Server::Resync(const protobuf::Context& serialized) -> bool
{
    return resync(*get_data(), serialized);
}

auto Server::resync(Data& data, const protobuf::Context& serialized) -> bool
{
    TransactionNumbers serverNumbers{};

    for (const auto& n : serialized.issuedtransactionnumber()) {
        const auto number = static_cast<TransactionNumber>(n);
        serverNumbers.insert(number);
        const auto exists = data.issued_transaction_numbers_.contains(number);

        if (false == exists) {
            LogError()()("Server believes number ")(
                number)(" is still issued. Restoring.")
                .Flush();
            data.issued_transaction_numbers_.insert(number);
            data.available_transaction_numbers_.insert(number);
        }
    }

    for (const auto& number : data.issued_transaction_numbers_) {
        auto exists = serverNumbers.contains(number);

        if (false == exists) {
            LogError()()("Server believes number ")(
                number)(" is no longer issued. Removing.")
                .Flush();
            data.issued_transaction_numbers_.erase(number);
            data.available_transaction_numbers_.erase(number);
        }
    }

    TransactionNumbers notUsed{};
    update_highest(data, data.issued_transaction_numbers_, notUsed, notUsed);

    return true;
}

auto Server::Revision() const -> std::uint64_t
{
    return get_data()->revision_.load();
}

auto Server::update_state(
    Data& data,
    const protobuf::DeliveryState state,
    const PasswordPrompt& reason,
    const otx::LastReplyStatus status) -> void
{
    data.state_.store(state);

    if (otx::LastReplyStatus::Invalid != status) {
        data.last_status_.store(status);
    }

    const auto saved = save(data, reason);

    assert_true(saved);
}

auto Server::scan_number_set(
    const TransactionNumbers& input,
    TransactionNumber& highest,
    TransactionNumber& lowest) -> void
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
    auto handle = get_data();
    auto& data = *handle;
    data.pending_args_ = {label, resync};
    data.request_sent_.Send([&] {
        auto out = network::zeromq::Message{};
        out.AddFrame(message.command_->Get());

        return out;
    }());
    auto result = context.Connection().Send(context, data, message, reason);

    if (client::SendResult::VALID_REPLY == result.first) {
        process_reply(data, client, pending, *result.second, reason);
        data.reply_received_.Send([&] {
            auto out = network::zeromq::Message{};
            out.AddFrame(message.command_->Get());

            return out;
        }());
    }

    return result;
}

auto Server::serialize(const Data& data) const -> protobuf::Context
{
    auto output = serialize(data, Type());
    auto& server = *output.mutable_servercontext();
    server.set_version(output.version());
    server_id_.Internal().Serialize(*server.mutable_serverid());
    server.set_highesttransactionnumber(
        data.highest_transaction_number_.load());

    for (const auto& it : data.tentative_transaction_numbers_) {
        server.add_tentativerequestnumber(it);
    }

    if (output.version() >= 2) {
        server.set_revision(data.revision_.load());
        server.set_adminpassword(data.admin_password_);
        server.set_adminattempted(data.admin_attempted_.get());
        server.set_adminsuccess(data.admin_success_.get());
    }

    if (output.version() >= 3) {
        server.set_state(data.state_.load());
        server.set_laststatus(translate(data.last_status_.load()));

        if (data.pending_message_) {
            auto& pending = *server.mutable_pending();
            pending.set_version(pending_command_version_);
            pending.set_serialized(
                String::Factory(*data.pending_message_)->Get());
            pending.set_accountlabel(std::get<0>(data.pending_args_));
            pending.set_resync(std::get<1>(data.pending_args_));
        }
    }

    return output;
}

auto Server::server_nym_id() const -> const identifier::Nym&
{
    assert_false(nullptr == remote_nym_);

    return remote_nym_->ID();
}

auto Server::SetAdminAttempted() -> void { get_data()->admin_attempted_->On(); }

auto Server::SetAdminPassword(const UnallocatedCString& password) -> void
{
    get_data()->admin_password_ = password;
}

auto Server::SetAdminSuccess() -> void
{
    auto handle = get_data();
    auto& data = *handle;
    data.admin_attempted_->On();
    data.admin_success_->On();
}

auto Server::SetHighest(const TransactionNumber& highest) -> bool
{
    auto handle = get_data();
    auto& data = *handle;

    if (highest >= data.highest_transaction_number_.load()) {
        data.highest_transaction_number_.store(highest);

        return true;
    }

    return false;
}

auto Server::SetPush(const bool on) -> void
{
    get_data()->enable_otx_push_.store(on);
}

auto Server::SetRevision(const std::uint64_t revision) -> void
{
    get_data()->revision_.store(revision);
}

auto Server::StaleNym() const -> bool
{
    assert_false(nullptr == Signer());

    return get_data()->revision_.load() < Signer()->Revision();
}

auto Server::Statement(const OTTransaction& owner, const PasswordPrompt& reason)
    const -> std::unique_ptr<Item>
{
    const TransactionNumbers empty;

    return Statement(owner, empty, reason);
}

auto Server::Statement(
    const OTTransaction& transaction,
    const TransactionNumbers& adding,
    const PasswordPrompt& reason) const -> std::unique_ptr<Item>
{
    return statement(*get_data(), transaction, adding, reason);
}

auto Server::Statement(
    const TransactionNumbers& adding,
    const TransactionNumbers& without,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<otx::context::TransactionStatement>
{
    return generate_statement(*get_data(), adding, without);
}

auto Server::statement(
    const Data& data,
    const OTTransaction& transaction,
    const TransactionNumbers& adding,
    const PasswordPrompt& reason) const -> std::unique_ptr<Item>
{
    std::unique_ptr<Item> output;

    assert_false(nullptr == Signer());

    if ((transaction.GetNymID() != Signer()->ID())) {
        LogError()()("Transaction has wrong owner.").Flush();

        return output;
    }

    // transaction is the depositPaymentPlan, activateSmartContract, or
    // marketOffer that triggered the need for this transaction statement.
    // Since it uses up a transaction number, I will be sure to remove that one
    // from my list before signing the list.
    output = api_.Factory().Internal().Session().Item(
        transaction,
        otx::itemType::transactionStatement,
        identifier::Account{});

    if (false == bool(output)) { return output; }

    const TransactionNumbers empty;
    auto statement = generate_statement(data, adding, empty);

    if (!statement) { return output; }

    switch (transaction.GetType()) {
        case otx::transactionType::cancelCronItem: {
            if (transaction.GetTransactionNum() > 0) {
                statement->Remove(transaction.GetTransactionNum());
            }
        } break;
        // Transaction Statements usually only have a transaction number in the
        // case of market offers and payment plans, in which case the number
        // should NOT be removed, and remains in play until final closure from
        // Cron.
        case otx::transactionType::marketOffer:
        case otx::transactionType::paymentPlan:
        case otx::transactionType::smartContract:
        default: {
        }
    }

    // What about cases where no number is being used? (Such as processNymbox)
    // Perhaps then if this function is even called, it's with a 0-number
    // transaction, in which case the above Removes probably won't hurt
    // anything.  Todo.

    output->SetAttachment(OTString(*statement));
    output->SignContract(*Signer(), reason);
    // OTTransactionType needs to weasel in a "date signed" variable.
    output->SaveContract();

    return output;
}

auto Server::ShouldRename(const UnallocatedCString& defaultName) const -> bool
{
    try {
        const auto contract = api_.Wallet().Internal().Server(server_id_);

        if (contract->Alias() != contract->EffectiveName()) { return true; }

        return defaultName == contract->EffectiveName();
    } catch (...) {
        LogError()()("Missing server contract.").Flush();

        return false;
    }
}

auto Server::start(
    Data& data,
    const Lock& decisionLock,
    const PasswordPrompt& reason,
    const api::session::Client& client,
    std::shared_ptr<Message> message,
    const ExtraArgs& args,
    const protobuf::DeliveryState state,
    const ActionType type,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox,
    UnallocatedSet<otx::context::ManagedNumber>* numbers) -> Server::QueueResult
{
    data.client_.store(&client);
    data.process_nymbox_.store(static_cast<bool>(type));
    data.pending_message_ = message;
    data.pending_args_ = args;
    data.pending_result_set_.store(false);
    data.pending_result_ = std::promise<DeliveryResult>();
    data.inbox_ = inbox;
    data.outbox_ = outbox;
    data.numbers_ = numbers;
    data.failure_counter_.store(0);
    update_state(data, state, reason);

    if (trigger(decisionLock)) {
        return std::make_unique<SendFuture>(data.pending_result_.get_future());
    }

    LogError()()("Failed to queue task").Flush();

    return {};
}

auto Server::state_machine(Data& data) noexcept -> bool
{
    const auto* pClient = data.client_.load();

    assert_false(nullptr == pClient);

    auto reason = api_.Factory().PasswordPrompt("Sending server message");
    const auto& client = *pClient;

    switch (data.state_.load()) {
        case protobuf::DELIVERTYSTATE_PENDINGSEND: {
            LogDetail()()("Attempting to send message").Flush();
            pending_send(data, client, reason);
        } break;
        case protobuf::DELIVERTYSTATE_NEEDNYMBOX: {
            LogDetail()()("Downloading nymbox").Flush();
            need_nymbox(data, client, reason);
        } break;
        case protobuf::DELIVERTYSTATE_NEEDBOXITEMS: {
            LogDetail()()("Downloading box items").Flush();
            need_box_items(data, client, reason);
        } break;
        case protobuf::DELIVERTYSTATE_NEEDPROCESSNYMBOX: {
            LogDetail()()("Processing nymbox").Flush();
            need_process_nymbox(data, client, reason);
        } break;
        case protobuf::DELIVERTYSTATE_IDLE:
        case protobuf::DELIVERTYSTATE_ERROR:
        default: {
            LogAbort()()("Unexpected state").Abort();
        }
    }

    const bool more = protobuf::DELIVERTYSTATE_IDLE != data.state_.load();
    const bool retry = failure_count_limit_ >= data.failure_counter_.load();

    if (false == more) {
        LogDetail()()("Delivery complete").Flush();
    } else if (false == retry) {
        LogError()()("Error limit exceeded").Flush();
        resolve_queue(
            data,
            DeliveryResult{otx::LastReplyStatus::NotSent, nullptr},
            reason,
            protobuf::DELIVERTYSTATE_IDLE);
    } else if (shutdown().load()) {
        LogError()()("Shutting down").Flush();
        resolve_queue(
            data,
            DeliveryResult{otx::LastReplyStatus::NotSent, nullptr},
            reason,
            protobuf::DELIVERTYSTATE_IDLE);
    } else {
        LogDetail()()("Continuing").Flush();

        return true;
    }

    return false;
}

auto Server::Type() const -> otx::ConsensusType
{
    return otx::ConsensusType::Server;
}

auto Server::UpdateHighest(
    const TransactionNumbers& numbers,
    TransactionNumbers& good,
    TransactionNumbers& bad) -> TransactionNumber
{
    return update_highest(*get_data(), numbers, good, bad);
}

auto Server::update_highest(
    Data& data,
    const TransactionNumbers& numbers,
    TransactionNumbers& good,
    TransactionNumbers& bad) -> TransactionNumber
{
    TransactionNumber output = 0;  // 0 is success.
    TransactionNumber highest = 0;
    TransactionNumber lowest = 0;

    scan_number_set(numbers, highest, lowest);

    const TransactionNumber oldHighest =
        data.highest_transaction_number_.load();

    validate_number_set(numbers, oldHighest, good, bad);

    if ((lowest > 0) && (lowest <= oldHighest)) {
        // Return the first invalid number
        output = lowest;
    }

    if (!good.empty()) {
        if (0 != oldHighest) {
            LogVerbose()()("Raising Highest Transaction Number from ")(
                oldHighest)(" to ")(highest)(".")
                .Flush();
        } else {
            LogVerbose()()(
                "Creating Highest Transaction Number entry for this server as "
                "'")(highest)("'.")
                .Flush();
        }

        data.highest_transaction_number_.store(highest);
    }

    return output;
}

auto Server::update_nymbox_hash(
    Data& data,
    const Message& reply,
    const UpdateHash which) -> bool
{
    if (false == reply.nymbox_hash_->Exists()) {
        LogError()()("No hash provided by notary").Flush();

        return false;
    }

    const auto hash =
        api_.Factory().IdentifierFromBase58(reply.nymbox_hash_->Bytes());
    set_remote_nymbox_hash(data, hash);

    if (UpdateHash::Both == which) { set_local_nymbox_hash(data, hash); }

    return true;
}

auto Server::update_remote_hash(Data& data, const Message& reply)
    -> identifier::Generic
{
    auto output = identifier::Generic{};
    const auto& input = reply.nymbox_hash_;

    if (input->Exists()) {
        output = api_.Factory().IdentifierFromBase58(input->Bytes());
        data.remote_nymbox_hash_ = output;
    }

    return output;
}

auto Server::UpdateRequestNumber(const PasswordPrompt& reason) -> RequestNumber
{
    bool notUsed{false};

    return UpdateRequestNumber(notUsed, reason);
}

auto Server::UpdateRequestNumber(bool& sendStatus, const PasswordPrompt& reason)
    -> RequestNumber
{
    auto handle = get_data();
    auto& data = *handle;
    const auto messageLock = Lock{data.message_lock_};

    return update_request_number(data, reason, messageLock, sendStatus);
}

auto Server::update_request_number(
    Data& data,
    const PasswordPrompt& reason,
    [[maybe_unused]] const Lock& messageLock,
    bool& sendStatus) -> RequestNumber
{
    sendStatus = false;

    auto request =
        initialize_server_command(otx::MessageType::getRequestNumber);

    if (false == bool(request)) {
        LogError()()("Failed to initialize server message.").Flush();

        return {};
    }

    request->request_num_ =
        String::Factory(std::to_string(FIRST_REQUEST_NUMBER).c_str());

    if (false == finalize_server_command(*request, reason)) {
        LogError()()("Failed to finalize server message.").Flush();

        return {};
    }

    const auto push = static_cast<opentxs::network::ServerConnection::Push>(
        data.enable_otx_push_.load());
    const auto response =
        data.connection_.Send(*this, data, *request, reason, push);
    const auto& status = response.first;
    const auto& reply = response.second;

    switch (status) {
        case client::SendResult::TIMEOUT: {
            LogError()()("Reply timeout.").Flush();

            return {};
        }
        case client::SendResult::INVALID_REPLY: {
            sendStatus = true;
            LogError()()("Invalid reply.").Flush();

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
            LogError()()("Unknown error.").Flush();

            return {};
        }
    }

    assert_false(nullptr == reply);

    const RequestNumber newNumber = reply->new_request_num_;
    add_acknowledged_number(data, newNumber);
    remove_acknowledged_number(data, *reply);
    data.request_number_ = newNumber;
    update_remote_hash(data, *reply);

    return newNumber;
}

auto Server::update_request_number(
    Data& data,
    const PasswordPrompt& reason,
    Message& command) -> bool
{
    command.request_num_ =
        String::Factory(std::to_string(data.request_number_++).c_str());

    return finalize_server_command(command, reason);
}

auto Server::UpdateRequestNumber(Message& command, const PasswordPrompt& reason)
    -> bool
{
    return update_request_number(*get_data(), reason, command);
}

auto Server::validate_number_set(
    const TransactionNumbers& input,
    const TransactionNumber limit,
    TransactionNumbers& good,
    TransactionNumbers& bad) -> void
{
    for (const auto& it : input) {
        if (it <= limit) {
            LogDetail()()(
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

auto Server::verify_blank(
    const Data& data,
    OTTransaction& blank,
    TransactionNumbers& output) const -> void
{
    for (const auto& number : extract_numbers(blank)) {
        if (verify_issued_number(data, number)) {
            LogConsole()()(
                "Attempted to accept a blank transaction number that I ALREADY "
                "HAD...(Skipping).")
                .Flush();
        } else if (verify_tentative_number(data, number)) {
            LogConsole()()(
                "Attempted to accept a blank transaction number that I ALREADY "
                "ACCEPTED (it's on my tentative list already; Skipping).")
                .Flush();
        } else if (number <= data.highest_transaction_number_.load()) {
            LogConsole()()(
                "Attempted to accept a blank transaction number that I've HAD "
                "BEFORE, or at least, is <= to ones I've had before. "
                "(Skipping...).")
                .Flush();
        } else {
            LogTrace()()("Accepting number ")(number).Flush();
            output.insert(number);
        }
    }
}

auto Server::verify_success(
    const Data& data,
    OTTransaction& blank,
    TransactionNumbers& output) const -> void
{
    for (const auto& number : extract_numbers(blank)) {
        if (false == verify_tentative_number(data, number)) {
            LogDetail()()("otx::transactionType::successNotice: This wasn't on "
                          "my tentative "
                          "list (")(
                number)("), I must have already processed it. (Or there was a "
                        "dropped message when I did, or the server is trying "
                        "to slip me an old number).")
                .Flush();
        } else {
            output.insert(number);
        }
    }
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
    for (const auto& number : get_data()->issued_transaction_numbers_) {
        const bool missing = (1 != statement.Issued().count(number));

        if (missing) {
            LogConsole()()("Issued transaction # ")(
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

auto Server::VerifyTentativeNumber(const TransactionNumber& number) const
    -> bool
{
    return verify_tentative_number(*get_data(), number);
}

auto Server::verify_tentative_number(
    const Data& data,
    const TransactionNumber& number) const -> bool
{
    return data.tentative_transaction_numbers_.contains(number);
}

Server::~Server()
{
    auto reason = api_.Factory().PasswordPrompt("Shutting down server context");
    Stop().get();
    auto handle = get_data();
    auto& data = *handle;
    const bool needPromise =
        (false == data.pending_result_set_.load()) &&
        (protobuf::DELIVERTYSTATE_IDLE != data.state_.load());

    if (needPromise) {
        resolve_queue(
            data,
            DeliveryResult{otx::LastReplyStatus::Unknown, nullptr},
            reason,
            protobuf::DELIVERTYSTATE_IDLE);
    }
}
}  // namespace opentxs::otx::context::implementation

#undef START_SERVER_CONTEXT
