// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/server/Notary.hpp"  // IWYU pragma: associated

#include <OTXEnums.pb.h>
#include <OTXPush.pb.h>
#include <Purse.pb.h>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <utility>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/blind/Factory.hpp"
#include "internal/otx/blind/Mint.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "internal/otx/blind/Token.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/basket/Basket.hpp"
#include "internal/otx/common/basket/BasketItem.hpp"
#include "internal/otx/common/cron/OTCron.hpp"
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/recurring/OTPaymentPlan.hpp"
#include "internal/otx/common/trade/OTOffer.hpp"
#include "internal/otx/common/trade/OTTrade.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/OTXPush.hpp"
#include "internal/serialization/protobuf/verify/Purse.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Exclusive.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "otx/server/Macros.hpp"
#include "otx/server/PayDividendVisitor.hpp"
#include "otx/server/Server.hpp"
#include "otx/server/ServerSettings.hpp"
#include "otx/server/Transactor.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::server
{
using listOfAccounts = UnallocatedVector<ExclusiveAccount>;

Notary::Notary(
    Server& server,
    const PasswordPrompt& reason,
    const opentxs::api::session::Notary& manager)
    : server_(server)
    , reason_(reason)
    , api_(manager)
    , notification_socket_(api_.Network().ZeroMQ().Internal().PushSocket(
          zmq::socket::Direction::Connect))
{
    const auto bound = notification_socket_->Start(
        api_.Endpoints().Internal().PushNotification().data());

    OT_ASSERT(bound);
}

Notary::Finalize::Finalize(
    const identity::Nym& signer,
    Item& item,
    Item& balanceItem,
    const PasswordPrompt& reason)
    : signer_(signer)
    , item_(item)
    , balance_item_(balanceItem)
    , reason_(reason)
{
}

Notary::Finalize::~Finalize()
{
    item_.SignContract(signer_, reason_);
    item_.SaveContract();
    balance_item_.SignContract(signer_, reason_);
    balance_item_.SaveContract();
}

void Notary::AddHashesToTransaction(
    OTTransaction& transaction,
    const Ledger& inbox,
    const Ledger& outbox,
    const identifier::Account& accounthash) const
{
    auto inboxHash = identifier::Generic{};
    inbox.CalculateInboxHash(inboxHash);
    transaction.SetInboxHash(inboxHash);

    auto outboxHash = identifier::Generic{};
    outbox.CalculateOutboxHash(outboxHash);
    transaction.SetOutboxHash(outboxHash);

    transaction.SetAccountHash(accounthash);
}

void Notary::cancel_cheque(
    const OTTransaction& input,
    const Cheque& cheque,
    const Item& depositItem,
    const String& serializedDepositItem,
    const Item& balanceItem,
    otx::context::Client& context,
    Account& account,
    Ledger& inbox,
    const Ledger& outbox,
    OTTransaction& output,
    bool& success,
    Item& responseItem,
    Item& responseBalanceItem)
{
    const auto& nymID = context.RemoteNym().ID();
    const auto strSenderNymID =
        String::Factory(cheque.GetSenderNymID(), api_.Crypto());
    const auto strRecipientNymID =
        String::Factory(cheque.GetRecipientNymID(), api_.Crypto());

    if (cheque.GetSenderNymID() != nymID) {
        LogError()(OT_PRETTY_CLASS())("Incorrect nym id (")(
            cheque.GetSenderNymID(), api_.Crypto())(").")
            .Flush();

        return;
    }

    if (cheque.GetAmount() != 0) {
        const auto unittype =
            api_.Wallet().Internal().CurrencyTypeBasedOnUnitType(
                cheque.GetInstrumentDefinitionID());
        LogError()(OT_PRETTY_CLASS())("Invalid amount (")(
            cheque.GetAmount(), unittype)(").")
            .Flush();

        return;
    }

    if (false == context.VerifyIssuedNumber(cheque.GetTransactionNum())) {
        LogError()(OT_PRETTY_CLASS())("Invalid transaction number (")(
            cheque.GetTransactionNum())(").")
            .Flush();

        return;
    }

    if (false == cheque.VerifySignature(context.RemoteNym())) {
        LogError()(OT_PRETTY_CLASS())("Invalid cheque signature.").Flush();

        return;
    }

    const auto validBalance = balanceItem.VerifyBalanceStatement(
        cheque.GetAmount(),
        context,
        inbox,
        outbox,
        account,
        input,
        UnallocatedSet<TransactionNumber>(),
        reason_);

    if (false == validBalance) {
        LogError()(OT_PRETTY_CLASS())("Invalid balance statement.").Flush();

        return;
    }

    responseBalanceItem.SetStatus(Item::acknowledgement);

    if (false == context.ConsumeAvailable(cheque.GetTransactionNum())) {
        LogError()(OT_PRETTY_CLASS())("Failed to consume transaction number.")
            .Flush();

        return;
    }

    TransactionNumber receiptNumber{0};
    server_.GetTransactor().issueNextTransactionNumber(receiptNumber);
    std::shared_ptr<OTTransaction> inboxTransaction{
        api_.Factory()
            .InternalSession()
            .Transaction(
                inbox,
                transactionType::chequeReceipt,
                originType::not_applicable,
                receiptNumber)
            .release()};

    OT_ASSERT(inboxTransaction);

    inboxTransaction->SetReferenceString(serializedDepositItem);
    inboxTransaction->SetReferenceToNum(depositItem.GetTransactionNum());
    inboxTransaction->SetAsCancelled();
    inboxTransaction->SignContract(server_.GetServerNym(), reason_);
    inboxTransaction->SaveContract();
    inbox.AddTransaction(inboxTransaction);
    inbox.ReleaseSignatures();
    inbox.SignContract(server_.GetServerNym(), reason_);
    inbox.SaveContract();
    account.SaveInbox(inbox);
    inboxTransaction->SaveBoxReceipt(inbox);
    responseItem.SetStatus(Item::acknowledgement);
    success = true;
    output.SetAsCancelled();
    LogDebug()(OT_PRETTY_CLASS())("Success cancelling cheque ")(
        cheque.GetTransactionNum())
        .Flush();

    auto accountHash = identifier::Account{};
    account.GetIdentifier(accountHash);
    AddHashesToTransaction(output, inbox, outbox, accountHash);
}

void Notary::deposit_cheque(
    const OTTransaction& input,
    const Item& depositItem,
    const String& serializedDepositItem,
    const Item& balanceItem,
    const Cheque& cheque,
    otx::context::Client& depositorContext,
    ExclusiveAccount& depositorAccount,
    Ledger& depositorInbox,
    const Ledger& depositorOutbox,
    OTTransaction& output,
    bool& success,
    Item& responseItem,
    Item& responseBalanceItem)
{
    const auto& nymID = depositorContext.RemoteNym().ID();
    const auto& sourceAccountID(cheque.GetSenderAcctID());
    const auto& senderNymID(cheque.GetSenderNymID());
    const auto& remitterAccountID(cheque.GetRemitterAcctID());
    const auto& remitterNymID(cheque.GetRemitterNymID());
    const bool isVoucher = cheque.HasRemitter();
    const bool cancelVoucher =
        (isVoucher && (nymID == cheque.GetRemitterNymID()));
    std::shared_ptr<Ledger> senderInbox{nullptr};
    std::shared_ptr<Ledger> senderOutbox{nullptr};
    std::shared_ptr<OTTransaction> inboxItem{nullptr};
    const api::session::internal::Wallet::AccountCallback push{
        [&](const Account& account) {
            this->send_push_notification(
                account, senderInbox, senderOutbox, inboxItem);
        }};
    const api::session::internal::Wallet::AccountCallback noPush{};
    ExclusiveAccount voucherAccount{};

    if (isVoucher) {
        voucherAccount = api_.Wallet().Internal().mutable_Account(
            sourceAccountID, reason_, (isVoucher) ? noPush : push);

        if (false == voucherAccount.get().VerifyOwner(server_.GetServerNym())) {
            LogError()(OT_PRETTY_CLASS())("Incorrect owner on voucher account.")
                .Flush();

            return;
        }
    }

    ExclusiveAccount senderAccount{};

    if (cancelVoucher) {
        deposit_cheque(
            input,
            depositItem,
            serializedDepositItem,
            balanceItem,
            cheque,
            true,
            true,
            remitterNymID,
            depositorContext,
            depositorAccount,
            depositorInbox,
            inboxItem,
            voucherAccount.get(),
            depositorContext,
            depositorAccount,
            depositorInbox,
            depositorOutbox,
            success,
            responseItem,
            responseBalanceItem);
    } else {
        {
            senderInbox.reset(
                api_.Factory()
                    .InternalSession()
                    .Ledger(
                        (isVoucher ? remitterNymID : senderNymID),
                        (isVoucher ? remitterAccountID : sourceAccountID),
                        depositorContext.Notary())
                    .release());

            OT_ASSERT(senderInbox);

            const auto inboxLoaded = senderInbox->LoadInbox();

            if (false == inboxLoaded) {
                LogError()(OT_PRETTY_CLASS())("Failed to load sender inbox.")
                    .Flush();

                return;
            }

            if (false == senderInbox->VerifyAccount(server_.GetServerNym())) {
                LogError()(OT_PRETTY_CLASS())("Failed to verify sender inbox.")
                    .Flush();

                return;
            }
        }

        {
            senderOutbox.reset(
                api_.Factory()
                    .InternalSession()
                    .Ledger(
                        (isVoucher ? remitterNymID : senderNymID),
                        (isVoucher ? remitterAccountID : sourceAccountID),
                        depositorContext.Notary())
                    .release());

            OT_ASSERT(senderOutbox);

            const auto outboxLoaded = senderOutbox->LoadOutbox();

            if (false == outboxLoaded) {
                LogError()(OT_PRETTY_CLASS())("Failed to load sender outbox.")
                    .Flush();

                return;
            }

            if (false == senderOutbox->VerifyAccount(server_.GetServerNym())) {
                LogError()(OT_PRETTY_CLASS())("Failed to verify sender outbox.")
                    .Flush();

                return;
            }
        }

        senderAccount = api_.Wallet().Internal().mutable_Account(
            ((isVoucher) ? remitterAccountID : sourceAccountID), reason_, push);
        auto senderContext = api_.Wallet().Internal().mutable_ClientContext(
            (isVoucher ? remitterNymID : senderNymID), reason_);

        if (!senderAccount.get().VerifyOwner(senderContext.get().RemoteNym())) {
            LogError()(OT_PRETTY_CLASS())("Incorrect owner on sender account.")
                .Flush();

            return;
        }

        deposit_cheque(
            input,
            depositItem,
            serializedDepositItem,
            balanceItem,
            cheque,
            isVoucher,
            false,
            ((isVoucher) ? remitterNymID : senderNymID),
            senderContext.get(),
            senderAccount,
            *senderInbox,
            inboxItem,
            ((isVoucher) ? voucherAccount.get() : senderAccount.get()),
            depositorContext,
            depositorAccount,
            depositorInbox,
            depositorOutbox,
            success,
            responseItem,
            responseBalanceItem);
    }

    auto accountHash = identifier::Account{};
    depositorAccount.get().GetIdentifier(accountHash);
    AddHashesToTransaction(
        output, depositorInbox, depositorOutbox, accountHash);

    if (success) {
        depositorAccount.Release();

        if (false == cancelVoucher) {
            if (isVoucher) {
                voucherAccount.Release();
            } else {
                senderAccount.Release();
            }
        }
    } else {
        depositorAccount.Abort();

        if (false == cancelVoucher) {
            if (isVoucher) {
                voucherAccount.Abort();
            } else {
                senderAccount.Abort();
            }
        }
    }
}

void Notary::deposit_cheque(
    const OTTransaction& input,
    const Item& depositItem,
    const String& serializedDepositItem,
    const Item& balanceItem,
    const Cheque& cheque,
    const bool isVoucher,
    const bool cancelling,
    const identifier::Nym& senderNymID,
    otx::context::Client& senderContext,
    Account& senderAccount,
    Ledger& senderInbox,
    std::shared_ptr<OTTransaction>& inboxItem,
    Account& sourceAccount,
    const otx::context::Client& depositorContext,
    Account& depositorAccount,
    const Ledger& depositorInbox,
    const Ledger& depositorOutbox,
    bool& success,
    Item& responseItem,
    Item& responseBalanceItem)
{
    const bool sameUnit = (cheque.GetInstrumentDefinitionID() ==
                           sourceAccount.GetInstrumentDefinitionID()) &&
                          (cheque.GetInstrumentDefinitionID() ==
                           depositorAccount.GetInstrumentDefinitionID());

    if (false == sameUnit) {
        LogError()(OT_PRETTY_CLASS())("Deposit account unit definition "
                                      "is incompatible with this cheque.")
            .Flush();

        return;
    }

    const auto& nymID = depositorContext.RemoteNym().ID();
    const auto& serverNymID = senderContext.Signer()->ID();

    if (isVoucher && (senderNymID != serverNymID)) {
        LogError()(OT_PRETTY_CLASS())("Invalid sender nym on voucher: ")(
            senderNymID, api_.Crypto())(".")
            .Flush();

        return;
    }

    const auto chequeNumber = cheque.GetTransactionNum();
    const auto validNumber = senderContext.VerifyIssuedNumber(chequeNumber);

    if (false == validNumber) {
        LogError()(OT_PRETTY_CLASS())("Invalid transaction number ")(
            chequeNumber)(".")
            .Flush();

        return;
    }

    if (false == cheque.VerifySignature(senderContext.RemoteNym())) {
        LogError()(OT_PRETTY_CLASS())("Invalid signature on cheque.").Flush();

        return;
    }

    bool validReceipient{false};

    if (cancelling) {
        validReceipient = true;
    } else {
        if (cheque.HasRecipient()) {
            validReceipient = (nymID == cheque.GetRecipientNymID());
        } else {
            validReceipient = true;
        }
    }

    if (false == validReceipient) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(nymID, api_.Crypto())(
            " is not allowed to deposit this cheque.")
            .Flush();

        return;
    }

    const auto amount = cheque.GetAmount();
    const auto validBalance = balanceItem.VerifyBalanceStatement(
        amount,
        depositorContext,
        depositorInbox,
        depositorOutbox,
        depositorAccount,
        input,
        UnallocatedSet<TransactionNumber>(),
        reason_);

    if (false == validBalance) {
        LogError()(OT_PRETTY_CLASS())(
            "Invalid balance agreement on deposit transaction.")
            .Flush();

        return;
    }

    responseBalanceItem.SetStatus(Item::acknowledgement);

    if (false == sourceAccount.Debit(amount)) {
        LogError()(OT_PRETTY_CLASS())("Failed debiting source account.")
            .Flush();

        return;
    }

    if (false == depositorAccount.Credit(amount)) {
        LogError()(OT_PRETTY_CLASS())("Failed crediting depositor account.")
            .Flush();

        return;
    }

    const bool consumed = senderContext.ConsumeAvailable(chequeNumber);

    if (false == consumed) {
        LogError()(OT_PRETTY_CLASS())("Failed to mark transaction number ")(
            chequeNumber)(" as used.")
            .Flush();

        return;
    }

    // This happens if the voucher is the result of a dividend payment
    if (isVoucher && (senderNymID == senderContext.Signer()->ID())) {
        // Server nyms never process the inbox of internal server accounts,
        // so this ensures the number is fully closed out.
        senderContext.ConsumeIssued(chequeNumber);
    }

    TransactionNumber receiptNumber{0};
    const auto issued =
        server_.GetTransactor().issueNextTransactionNumber(receiptNumber);

    if (false == issued) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to issue transaction number for cheque receipt.")
            .Flush();

        return;
    }

    inboxItem.reset(api_.Factory()
                        .InternalSession()
                        .Transaction(
                            senderInbox,
                            isVoucher ? transactionType::voucherReceipt
                                      : transactionType::chequeReceipt,
                            originType::not_applicable,
                            receiptNumber)
                        .release());

    OT_ASSERT(inboxItem);

    inboxItem->SetReferenceString(serializedDepositItem);
    inboxItem->SetReferenceToNum(depositItem.GetTransactionNum());
    inboxItem->SetNumberOfOrigin(chequeNumber);

    if (cancelling) { inboxItem->SetAsCancelled(); }

    inboxItem->SignContract(server_.GetServerNym(), reason_);
    inboxItem->SaveContract();
    senderInbox.AddTransaction(inboxItem);
    senderInbox.ReleaseSignatures();
    senderInbox.SignContract(server_.GetServerNym(), reason_);
    senderInbox.SaveContract();
    senderAccount.SaveInbox(senderInbox);
    inboxItem->SaveBoxReceipt(senderInbox);
    responseItem.SetStatus(Item::acknowledgement);
    success = true;
    LogDebug()(OT_PRETTY_CLASS())("Success processing cheque ")(chequeNumber)
        .Flush();
}

auto Notary::extract_cheque(
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID,
    const Item& item) const -> std::unique_ptr<Cheque>
{
    auto serialized = String::Factory();
    item.GetAttachment(serialized);
    auto cheque = api_.Factory().InternalSession().Cheque(serverID, unitID);

    OT_ASSERT(cheque);

    bool loadedCheque = cheque->LoadContractFromString(serialized);

    if (false == loadedCheque) {
        LogError()(OT_PRETTY_CLASS())("Failed to load cheque.").Flush();
        cheque.reset();

        return cheque;
    }

    if (serverID != cheque->GetNotaryID()) {
        LogError()(OT_PRETTY_CLASS())(
            "Cheque rejected due to incorrect notary ID "
            "(")(cheque->GetNotaryID(), api_.Crypto())(").")
            .Flush();
    }

    return cheque;
}

void Notary::NotarizeTransfer(
    otx::context::Client& context,
    ExclusiveAccount& theFromAccount,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    Ledger& inbox,
    Ledger& outbox,
    bool& bOutSuccess)
{
    // The outgoing transaction is an "atTransfer", that is, "a reply to the
    // transfer request"
    tranOut.SetType(transactionType::atTransfer);

    std::shared_ptr<Item> pItem = nullptr;
    std::shared_ptr<Item> pBalanceItem = nullptr;
    std::shared_ptr<Item> pResponseItem = nullptr;
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto& NOTARY_ID = context.Notary();
    const auto ACCOUNT_ID =
        server_.API().Factory().Internal().Identifier(theFromAccount.get());
    auto strNymID = String::Factory(NYM_ID, api_.Crypto()),
         strAccountID = String::Factory(ACCOUNT_ID, api_.Crypto());
    pResponseBalanceItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atBalanceStatement, identifier::Account{})
            .release());

    OT_ASSERT(false != bool(pResponseBalanceItem));

    pResponseBalanceItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseBalanceItem);  // the Transaction's destructor
                                            // will cleanup the item. It "owns"
                                            // it now.
    pResponseItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atTransfer, identifier::Account{})
            .release());

    OT_ASSERT(false != bool(pResponseItem));

    pResponseItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseItem);  // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    auto accountHash = identifier::Account{};

    if (false ==
        NYM_IS_ALLOWED(strNymID->Get(), ServerSettings::_transact_transfer)) {

        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (All acct-to-acct transfers are "
            "disallowed in server.cfg)")
            .Flush();
    } else if (
        nullptr ==
        (pBalanceItem = tranIn.GetItem(itemType::balanceStatement))) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())(
            "Expected Item::balanceStatement in trans #")(
            tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    }
    // For now, there should only be one of these transfer items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    else if (nullptr == (pItem = tranIn.GetItem(itemType::transfer))) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())("Expected Item::transfer in trans #")(
            tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    } else if (ACCOUNT_ID == pItem->GetDestinationAcctID()) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())("Failed attempt by user ")(
            strNymID.get())(" in trans #")(tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    } else {
        // The response item, as well as the inbox and outbox items, will
        // contain a copy
        // of the request item. So I save it into a string here so they can all
        // grab a copy of it
        // into their "in reference to" fields.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // IDFromAccount is the ID on the "from" Account that was passed in.
        // IDItemFromAccount is the "from" account ID on the transaction Item we
        // are currently examining.
        // IDItemToAccount is the "to" account ID on the transaction item we are
        // currently examining.
        auto IDFromAccount =
            server_.API().Factory().Internal().AccountID(theFromAccount.get());

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.

        pResponseItem->SetReferenceString(strInReferenceTo);  // the response
                                                              // item carries a
                                                              // copy of what
                                                              // it's responding
                                                              // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN RESPONSE
                                          // to
                                          // pItem and its Owner Transaction.
        pResponseItem->SetNumberOfOrigin(*pItem);

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem);  // the response item carries a copy of what it's
                              // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN RESPONSE
                                          // to
                                          // pItem and its Owner Transaction.
        pResponseBalanceItem->SetNumberOfOrigin(*pItem);

        // Set the ID on the To Account based on what the transaction request
        // said. (So we can load it up.)
        std::shared_ptr<Ledger> recipientInbox{nullptr};
        std::shared_ptr<Ledger> recipientOutbox{nullptr};
        std::shared_ptr<OTTransaction> inboxTransaction{nullptr};
        auto destinationAccount = api_.Wallet().Internal().mutable_Account(
            pItem->GetDestinationAcctID(),
            reason_,
            [&](const Account& account) {
                this->send_push_notification(
                    account, recipientInbox, recipientOutbox, inboxTransaction);
            });

        // Only accept transfers with positive amounts.
        if (0 > pItem->GetAmount()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failure: Attempt to transfer negative balance.")
                .Flush();
        }

        // I'm using the operator== because it exists.
        // If the ID on the "from" account that was passed in,
        // does not match the "Acct From" ID on this transaction item
        else if (!(IDFromAccount == pItem->GetPurportedAccountID())) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Error: 'From' "
                "account ID on the transaction does not match "
                "'from' account ID on the transaction item.")
                .Flush();
        }
        // ok so the IDs match. Does the destination account exist?
        else if (false == bool(destinationAccount)) {
            LogConsole()(OT_PRETTY_CLASS())("ERROR verifying "
                                            "existence of the 'to' account.")
                .Flush();
        }
        // Is the destination a legitimate other user's acct, or is it just an
        // internal server account?
        // (That is, stash accounts, voucher accounts, basket accounts, etc are
        // only used internally,
        // and may not be recipients to user transfers...)
        //
        else if (destinationAccount.get().IsInternalServerAcct()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failure: Destination "
                "account is used internally by the server, and is "
                "not a valid recipient for this transaction.")
                .Flush();
        }
        // Are both of the accounts of the same Asset Type?
        else if (!(theFromAccount.get().GetInstrumentDefinitionID() ==
                   destinationAccount.get().GetInstrumentDefinitionID())) {
            auto strFromInstrumentDefinitionID = String::Factory(
                     theFromAccount.get().GetInstrumentDefinitionID(),
                     api_.Crypto()),
                 strDestinationInstrumentDefinitionID = String::Factory(
                     destinationAccount.get().GetInstrumentDefinitionID(),
                     api_.Crypto());
            LogError()(OT_PRETTY_CLASS())(
                "ERROR - user attempted to transfer between accounts of 2 "
                "different instrument definitions in "
                "Notary::NotarizeTransfer: ")(
                strFromInstrumentDefinitionID.get())(" ")(
                strDestinationInstrumentDefinitionID.get())
                .Flush();
        }

        // This entire function can be divided into the top and bottom halves.
        // The top half is oriented around finding the "transfer" item (in the
        // "transfer" transaction)
        // and setting up the response item that will go into the response
        // transaction.
        // The bottom half is oriented, in the case of success, around creating
        // the necessary inbox
        // and outbox entries, and debiting the account, and basically
        // performing the actual transfer.
        else {
            // Okay then, everything checks out. Let's add this to the sender's
            // outbox and the recipient's inbox.
            // IF they can be loaded up from file, or generated, that is.

            // Load the inbox/outbox in case they already exist
            auto theFromOutbox{api_.Factory().InternalSession().Ledger(
                NYM_ID, IDFromAccount, NOTARY_ID)};
            recipientInbox.reset(
                api_.Factory()
                    .InternalSession()
                    .Ledger(pItem->GetDestinationAcctID(), NOTARY_ID)
                    .release());

            // Needed for push notifications
            {
                recipientOutbox.reset(
                    api_.Factory()
                        .InternalSession()
                        .Ledger(pItem->GetDestinationAcctID(), NOTARY_ID)
                        .release());

                OT_ASSERT(recipientOutbox);
            }

            OT_ASSERT(recipientInbox);

            bool bSuccessLoadingInbox = recipientInbox->LoadInbox();

            // Needed for push notifications
            {
                bSuccessLoadingInbox &= recipientOutbox->LoadOutbox();

                if (bSuccessLoadingInbox) {
                    bSuccessLoadingInbox &=
                        recipientOutbox->VerifyAccount(server_.GetServerNym());
                }
            }

            bool bSuccessLoadingOutbox = theFromOutbox->LoadOutbox();
            // ...or generate them otherwise...

            // NOTE:
            // 1. Any normal user had his inbox created at the same time as his
            // asset account was created.
            // 2. If there is an error now, we don't necessarily just want to
            // re-create (and overwrite) that file.
            // 3. Therefore I do not generate the ledger for safety reasons, per
            // 2.
            // 4. Also, what if an attempt is being made to transfer to an
            // account that isn't SUPPOSED to have
            //    an inbox? For example, a server voucher account (where backing
            // funds for vouchers are stored) does
            //    not have an inbox, and should not be able to receive
            // transfers. In that case, we definitely don't want
            //    to just "generate" an inbox here! Instead, we want it to fail.
            // In fact, I'm adding a check, above, for
            //    the account type. In fact, I'm adding a new method to
            // OTAccount where we can just ask it, for each
            //    transaction type, whether it can even be used for that purpose
            // in the first place.
            //    Update: appears OTAccount::IsInternalServerAcct already
            // basically fits the bill.

            if (true == bSuccessLoadingInbox) {
                bSuccessLoadingInbox =
                    recipientInbox->VerifyAccount(server_.GetServerNym());
            } else {
                LogError()(OT_PRETTY_CLASS())("Error loading 'to' inbox.")
                    .Flush();
            }

            if (true == bSuccessLoadingOutbox) {
                bSuccessLoadingOutbox =
                    theFromOutbox->VerifyAccount(server_.GetServerNym());
            } else {
                LogError()(OT_PRETTY_CLASS())("Error loading 'from' "
                                              "outbox.")
                    .Flush();
            }

            if (!bSuccessLoadingInbox || false == bSuccessLoadingOutbox) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR generating ledger in Notary::NotarizeTransfer.")
                    .Flush();
            } else {
                // Generate new transaction number for these new transactions
                // todo check this generation for failure (can it fail?)
                std::int64_t lNewTransactionNumber = 0;

                server_.GetTransactor().issueNextTransactionNumber(
                    lNewTransactionNumber);
                // I create TWO Outbox transactions -- one for the real outbox,
                // (theFromOutbox)
                // and one for pOutbox (used for verifying the balance
                // statement.)
                // pTEMPOutboxTransaction (here below) is that last one,
                // pOutbox.
                //
                auto pTEMPOutboxTransaction{
                    api_.Factory().InternalSession().Transaction(
                        outbox,
                        transactionType::pending,
                        originType::not_applicable,
                        lNewTransactionNumber)};

                OT_ASSERT(false != bool(pTEMPOutboxTransaction));

                auto pOutboxTransaction{
                    api_.Factory().InternalSession().Transaction(
                        *theFromOutbox,
                        transactionType::pending,
                        originType::not_applicable,
                        lNewTransactionNumber)};

                OT_ASSERT(false != bool(pOutboxTransaction));

                inboxTransaction.reset(api_.Factory()
                                           .InternalSession()
                                           .Transaction(
                                               *recipientInbox,
                                               transactionType::pending,
                                               originType::not_applicable,
                                               lNewTransactionNumber)
                                           .release());

                OT_ASSERT(inboxTransaction);

                // UPDATE: I am now issuing one new transaction number above,
                // instead of two. This is to make it easy
                // for the two to cross-reference each other. Later if I want to
                // remove the transaction from the inbox
                // and need to know the corresponding transaction # for the
                // outbox, it will be the same number.

                // I have to set this one up just like the one below.
                pTEMPOutboxTransaction->SetReferenceString(strInReferenceTo);
                pTEMPOutboxTransaction->SetReferenceToNum(
                    pItem->GetTransactionNum());
                pTEMPOutboxTransaction->SetNumberOfOrigin(*pItem);
                // the new transactions store a record of the item they're
                // referring to.
                pOutboxTransaction->SetReferenceString(strInReferenceTo);
                pOutboxTransaction->SetReferenceToNum(
                    pItem->GetTransactionNum());
                pOutboxTransaction->SetNumberOfOrigin(*pItem);

                // todo put these two together in a method.
                inboxTransaction->SetReferenceString(strInReferenceTo);
                inboxTransaction->SetReferenceToNum(pItem->GetTransactionNum());
                inboxTransaction->SetNumberOfOrigin(*pItem);

                // Now we have created 2 new transactions from the server to the
                // users' boxes
                // Let's sign them and add to their inbox / outbox.
                pOutboxTransaction->SignContract(
                    server_.GetServerNym(), reason_);
                inboxTransaction->SignContract(server_.GetServerNym(), reason_);

                pOutboxTransaction->SaveContract();
                inboxTransaction->SaveContract();
                // Meanwhile a copy of the outbox transaction is also added to
                // pOutbox. (It's just another copy of the outbox, but used
                // purely for verifying the balance statement, while a different
                // copy of the outbox is used for actually adding the receipt
                // and saving to the outbox file.)
                //
                pTEMPOutboxTransaction->SignContract(
                    server_.GetServerNym(), reason_);
                pTEMPOutboxTransaction->SaveContract();

                // No need to save a box receipt in this case, like we normally
                // would
                // when adding a transaction to a box.
                std::shared_ptr<OTTransaction> TEMPOutboxTransaction{
                    pTEMPOutboxTransaction.release()};
                outbox.AddTransaction(TEMPOutboxTransaction);

                // The balance item from the user, for the outbox transaction,
                // will not have
                // the correct transaction number (because I just generated it
                // above, so the user
                // could not possibly have known that when he sent his message.)
                // Currently it is
                // set to "1" in the user request, but I just put the actual
                // number in the pOutbox
                // above (since I now have the actual number.)
                //
                // So when the receipt is saved (the output transaction) it will
                // show the user's
                // signed request with "1" in the outbox, included in the
                // server's signed reply
                // with lNewTransactionNumber in the outbox to correspond to it.
                // The user saves
                // a copy of the same receipt, thus he will be unable to produce
                // a receipt signed
                // by the server, without producing the exact same thing.
                // ("1" in the request and lNewTransactionNumber in the signed
                // response.)
                //
                // This all means that the below call to
                // VerifyBalanceStatement() needs to verify
                // the number "1" on the user request, as lNewTransactionNumber
                // in pOutbox, in order
                // to handle this special case, since otherwise the verification
                // would fail.
                //
                if (!(pBalanceItem->VerifyBalanceStatement(
                        pItem->GetAmount() * (-1),  // My acct balance will be
                                                    // smaller as a result of
                                                    // this transfer.
                        context,
                        inbox,
                        outbox,
                        theFromAccount.get(),
                        tranIn,
                        UnallocatedSet<TransactionNumber>(),
                        reason_,
                        lNewTransactionNumber))) {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "ERROR verifying balance statement while performing "
                        "transfer. Acct ID: ")(strAccountID.get())
                        .Flush();
                } else {
                    pResponseBalanceItem->SetStatus(
                        Item::acknowledgement);  // the balance agreement (just
                                                 // above) was successful.
                    pResponseBalanceItem->SetNewOutboxTransNum(
                        lNewTransactionNumber);  // So the receipt will show
                                                 // that
                                                 // the client's "1" in the
                                                 // outbox is now actually "34"
                                                 // or whatever, issued by the
                                                 // server as part of
                                                 // successfully processing the
                                                 // transaction.

                    // Deduct the amount from the account...
                    // TODO an issuer account here, can go negative.
                    // whereas a regular account should not be allowed to go
                    // negative.
                    if (theFromAccount.get().Debit(
                            pItem->GetAmount())) {  // todo need to be able to
                                                    // "roll back" if anything
                                                    // inside this block fails.
                        // Here the transactions we just created are actually
                        // added to the ledgers.
                        std::shared_ptr<OTTransaction> outboxTransaction{
                            pOutboxTransaction.release()};
                        theFromOutbox->AddTransaction(outboxTransaction);
                        recipientInbox->AddTransaction(inboxTransaction);

                        // Release any signatures that were there before (They
                        // won't
                        // verify anymore anyway, since the content has
                        // changed.)
                        theFromOutbox->ReleaseSignatures();
                        recipientInbox->ReleaseSignatures();

                        // Sign them.
                        theFromOutbox->SignContract(
                            server_.GetServerNym(), reason_);
                        recipientInbox->SignContract(
                            server_.GetServerNym(), reason_);

                        // Save them internally
                        theFromOutbox->SaveContract();
                        recipientInbox->SaveContract();

                        // Save their internals (signatures and all) to file.
                        theFromAccount.get().SaveOutbox(*theFromOutbox);
                        destinationAccount.get().SaveInbox(*recipientInbox);

                        theFromAccount.get().GetIdentifier(accountHash);
                        theFromAccount.Release();
                        destinationAccount.Release();

                        // Now we can set the response item as an
                        // acknowledgement instead of the default (rejection)
                        // otherwise, if we never entered this block, then it
                        // would still be set to rejection, and the
                        // new items would never have been added to the
                        // inbox/outboxes, and those files, along with
                        // the account file, would never have had their
                        // signatures released, or been re-signed or
                        // re-saved back to file.  The debit failed, so all of
                        // those other actions would fail also.
                        // BUT... if the message comes back with
                        // acknowledgement--then all of these actions must have
                        // happened, and here is the server's signature to prove
                        // it.
                        // Otherwise you get no items and no signature. Just a
                        // rejection item in the response transaction.
                        pResponseItem->SetStatus(Item::acknowledgement);

                        bOutSuccess = true;  // The transfer was successful.

                        // Any inbox/nymbox/outbox ledger will only itself
                        // contain
                        // abbreviated versions of the receipts, including their
                        // hashes.
                        //
                        // The rest is stored separately, in the box receipt,
                        // which is created
                        // whenever a receipt is added to a box, and deleted
                        // after a receipt
                        // is removed from a box.
                        //
                        outboxTransaction->SaveBoxReceipt(*theFromOutbox);
                        inboxTransaction->SaveBoxReceipt(*recipientInbox);
                    } else {
                        theFromAccount.get().GetIdentifier(accountHash);
                        theFromAccount.Abort();
                        destinationAccount.Abort();
                        const auto unittype =
                            api_.Wallet()
                                .Internal()
                                .CurrencyTypeBasedOnUnitType(
                                    destinationAccount.get()
                                        .GetInstrumentDefinitionID());
                        LogError()(OT_PRETTY_CLASS())(
                            "Unable to debit account ")(strAccountID.get())(
                            " in the amount of: ")(pItem->GetAmount(), unittype)
                            .Flush();
                    }
                }
            }  // both boxes were successfully loaded or generated.
        }
    }

    // For the reply message.
    if (accountHash.empty() && theFromAccount) {
        theFromAccount.get().GetIdentifier(accountHash);
    }
    AddHashesToTransaction(tranOut, inbox, outbox, accountHash);

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_.GetServerNym(), reason_);
    pResponseItem->SaveContract();  // the signing was of no effect because I
                                    // forgot to save.

    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
}

/// NotarizeWithdrawal supports two withdrawal types:
///
/// Item::withdrawVoucher    This is a bank voucher, like a cashier's check.
/// Funds are transferred to the bank, who then issues a cheque drawn on an
/// internal voucher account.
///
/// Item::withdrawal        This is a digital cash withdrawal, in the form of
/// untraceable, blinded tokens. Funds are transferred to the bank, who
/// blind-signs the tokens.
void Notary::NotarizeWithdrawal(
    otx::context::Client& context,
    ExclusiveAccount& theAccount,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    Ledger& inbox,
    Ledger& outbox,
    bool& bOutSuccess)
{
    // The outgoing transaction is an "atWithdrawal", that is, "a reply to the
    // withdrawal request"
    tranOut.SetType(transactionType::atWithdrawal);

    std::shared_ptr<Item> pItem = nullptr;
    std::shared_ptr<Item> pItemCash = nullptr;
    std::shared_ptr<Item> pItemVoucher = nullptr;
    std::shared_ptr<Item> pBalanceItem = nullptr;
    std::shared_ptr<Item> pResponseItem = nullptr;
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const auto& NOTARY_ID = context.Notary();
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto& NOTARY_NYM_ID = context.Signer()->ID();
    const auto ACCOUNT_ID =
        server_.API().Factory().Internal().AccountID(theAccount.get());
    const auto& INSTRUMENT_DEFINITION_ID =
        theAccount.get().GetInstrumentDefinitionID();
    const auto strNymID = String::Factory(NYM_ID, api_.Crypto()),
               strAccountID = String::Factory(ACCOUNT_ID, api_.Crypto()),
               strInstrumentDefinitionID =
                   String::Factory(INSTRUMENT_DEFINITION_ID, api_.Crypto());

    // Here we find out if we're withdrawing cash, or a voucher
    // (A voucher is a cashier's cheque aka banker's cheque).
    //
    itemType theReplyItemType = itemType::error_state;

    pItemVoucher = tranIn.GetItem(itemType::withdrawVoucher);

    if (false == bool(pItemVoucher)) {
        pItemCash = tranIn.GetItem(itemType::withdrawal);
        pItem = pItemCash;
        if (false != bool(pItem)) { theReplyItemType = itemType::atWithdrawal; }
    } else {
        pItem = pItemVoucher;
        theReplyItemType = itemType::atWithdrawVoucher;
    }
    pResponseItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, theReplyItemType, identifier::Account{})
            .release());
    pResponseItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseItem);  // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atBalanceStatement, identifier::Account{})
            .release());
    pResponseBalanceItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseBalanceItem);  // the Transaction's destructor
                                            // will cleanup the item. It "owns"
                                            // it now.
    auto accountHash = identifier::Account{};

    if (nullptr == pItem) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())(
            "Expected Item::withdrawal or Item::withdrawVoucher in trans "
            "#")(tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : "(ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    }
    // Below this point, we know that pItem is good, and that either
    // pItemVoucher OR pItemCash is good,
    // and that pItem points to the good one. Therefore next, let's verify
    // permissions:
    // This permission has to do with ALL withdrawals (cash or voucher)
    else if (!NYM_IS_ALLOWED(
                 strNymID->Get(), ServerSettings::_transact_withdrawal)) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (All withdrawals are disallowed in "
            "server.cfg")
            .Flush();
    }
    // This permission has to do with vouchers.
    else if (
        (nullptr != pItemVoucher) &&
        (false ==
         NYM_IS_ALLOWED(
             strNymID->Get(), ServerSettings::_transact_withdraw_voucher))) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (withdraw voucher is disallowed in "
            "server.cfg")
            .Flush();
    }
    // This permission has to do with cash.
    else if (
        (nullptr != pItemCash) &&
        (false ==
         NYM_IS_ALLOWED(
             strNymID->Get(), ServerSettings::_transact_withdraw_cash))) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (withdraw cash is disallowed in "
            "server.cfg")
            .Flush();
    }
    // Check for a balance agreement...
    //
    else if (
        nullptr ==
        (pBalanceItem = tranIn.GetItem(itemType::balanceStatement))) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())(
            "Expected Item::balanceStatement, but not found in trans "
            "#")(tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    } else if (pItem->GetType() == itemType::withdrawVoucher) {
        // The response item will contain a copy of the request item. So I save
        // it into a string
        // here so they can all grab a copy of it into their "in reference to"
        // fields.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // Server response item being added to server response transaction
        // (tranOut)
        // (They're getting SOME sort of response item.)

        pResponseItem->SetReferenceString(strInReferenceTo);  // the response
                                                              // item carries a
                                                              // copy of what
                                                              // it's responding
                                                              // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN RESPONSE
                                          // to
                                          // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem);  // the response item carries a copy of what it's
                              // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN RESPONSE
                                          // to
                                          // pItem and its Owner Transaction.

        // contains the server's funds to back vouchers of a specific instrument
        // definition
        ExclusiveAccount voucherReserveAccount;

        // I'm using the operator== because it exists.
        // If the ID on the "from" account that was passed in,
        // does not match the "Acct From" ID on this transaction item.
        //
        if (!(ACCOUNT_ID ==
              pItem->GetPurportedAccountID())) {  // TODO see if this is already
                                                  // verified by the caller
                                                  // function and if so, remove.
            LogConsole()(OT_PRETTY_CLASS())(
                "Error: Account ID does not match account ID on the withdrawal "
                "item.")
                .Flush();
        }
        // The server will already have a special account for issuing vouchers.
        // Actually, a list of them --
        // one for each instrument definition. Since this is the normal way of
        // doing
        // business, GetTransactor().getVoucherAccount() will
        // just create it if it doesn't already exist, and then return the
        // pointer. Therefore, a failure here
        // is a catastrophic failure!  Should never fail.
        //
        else if (
            (voucherReserveAccount = server_.GetTransactor().getVoucherAccount(
                 INSTRUMENT_DEFINITION_ID)) &&
            voucherReserveAccount) {
            auto strVoucherRequest = String::Factory(),
                 strItemNote = String::Factory();
            pItem->GetNote(strItemNote);
            pItem->GetAttachment(strVoucherRequest);

            auto VOUCHER_ACCOUNT_ID =
                server_.API().Factory().Internal().AccountID(
                    voucherReserveAccount.get());

            auto theVoucher{api_.Factory().InternalSession().Cheque(
                NOTARY_ID, INSTRUMENT_DEFINITION_ID)};
            auto theVoucherRequest{api_.Factory().InternalSession().Cheque(
                NOTARY_ID, INSTRUMENT_DEFINITION_ID)};

            bool bLoadContractFromString =
                theVoucherRequest->LoadContractFromString(strVoucherRequest);

            if (!bLoadContractFromString) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR loading voucher request "
                    "from string: ")(strVoucherRequest->Get())(".")
                    .Flush();
            } else if (!context.VerifyIssuedNumber(
                           theVoucherRequest->GetTransactionNum())) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed verifying transaction number on the voucher "
                    "(")(theVoucherRequest->GetTransactionNum())(
                    ") in withdrawal "
                    "request ")(tranIn.GetTransactionNum())(" for Nym: ")(
                    strNymID->Get())(".")
                    .Flush();
            } else if (
                INSTRUMENT_DEFINITION_ID !=
                theVoucherRequest->GetInstrumentDefinitionID()) {
                const auto strFoundInstrumentDefinitionID = String::Factory(
                    theVoucherRequest->GetInstrumentDefinitionID(),
                    api_.Crypto());
                LogError()(OT_PRETTY_CLASS())(
                    "Failed verifying instrument definition ID "
                    "(")(strInstrumentDefinitionID->Get())(
                    ") on the withdraw voucher request "
                    "(found: ")(strFoundInstrumentDefinitionID->Get())(
                    ") for "
                    "transaction"
                    " ")(tranIn.GetTransactionNum())(", voucher ")(
                    theVoucherRequest->GetTransactionNum())(". User: ")(
                    strNymID->Get())(".")
                    .Flush();
            } else if (!(pBalanceItem->VerifyBalanceStatement(
                           theVoucherRequest->GetAmount() *
                               (-1),  // My account's balance will go down by
                                      // this much.
                           context,
                           inbox,
                           outbox,
                           theAccount.get(),
                           tranIn,
                           UnallocatedSet<TransactionNumber>(),
                           reason_))) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR verifying balance statement while issuing voucher. "
                    "Acct ID:")(strAccountID.get())
                    .Flush();
            } else  // successfully loaded the voucher request from the
                    // string...
            {
                pResponseBalanceItem->SetStatus(
                    Item::acknowledgement);  // the transaction agreement was
                                             // successful.
                auto strChequeMemo = String::Factory();
                UnallocatedCString tmp;
                const auto& ref = theVoucherRequest->GetMemo();
                tmp.reserve(strItemNote->GetLength() + ref.GetLength());
                tmp.append(strItemNote->Get());
                tmp.append(ref.Get());
                strChequeMemo->Set(tmp.c_str());

                // 10 minutes ==    600 Seconds
                // 1 hour    ==     3600 Seconds
                // 1 day    ==    86400 Seconds
                // 30 days    ==  2592000 Seconds
                // 3 months ==  7776000 Seconds
                // 6 months == 15552000 Seconds

                const auto VALID_FROM = Clock::now();
                const auto VALID_TO =
                    VALID_FROM + std::chrono::hours(24 * 30 * 6);

                // UPDATE: We now use a transaction number owned by the
                // remitter, instead of the transaction server.
                //
                //                std::int64_t lNewTransactionNumber = 0;
                //                GetTransactor().issueNextTransactionNumberToNym(server_.nym_server_,
                // lNewTransactionNumber);
                // We save the transaction
                // number on the server Nym (normally we'd discard it) because
                const Amount lAmount =
                    theVoucherRequest->GetAmount();  // when the cheque is
                // deposited, the server nym,
                // as the owner of
                const auto& RECIPIENT_ID =
                    theVoucherRequest->GetRecipientNymID();  // the voucher
                                                             // account, needs
                                                             // to verify the
                                                             // transaction # on
                                                             // the
                // cheque (to prevent double-spending of cheques.)
                bool bIssueVoucher = theVoucher->IssueCheque(
                    lAmount,  // The amount of the cheque.
                    theVoucherRequest->GetTransactionNum(),  // Requiring a
                    // transaction number
                    // prevents
                    // double-spending of
                    // cheques.
                    VALID_FROM,  // The expiration date (valid from/to dates) of
                                 // the cheque
                    VALID_TO,  // Vouchers are automatically starting today and
                               // lasting 6 months.
                    VOUCHER_ACCOUNT_ID,  // The asset account the cheque is
                                         // drawn on.
                    NOTARY_NYM_ID,  // Nym ID of the sender (in this case the
                                    // server.)
                    strChequeMemo,  // Optional memo field. Includes item
                                    // note and request memo.
                    theVoucherRequest->HasRecipient() ? RECIPIENT_ID
                                                      : identifier::Nym());

                // IF we successfully created the voucher, AND the voucher
                // amount is greater than 0,
                // AND debited the user's account,
                // AND credited the server's voucher account,
                //
                // THEN save the accounts and return the voucher to the user.
                //
                if (bIssueVoucher && (lAmount > 0) &&
                    theAccount.get().Debit(theVoucherRequest->GetAmount())) {
                    if (false == voucherReserveAccount.get().Credit(
                                     theVoucherRequest->GetAmount())) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Failed crediting voucher reserve account.")
                            .Flush();

                        if (false == theAccount.get().Credit(
                                         theVoucherRequest->GetAmount())) {
                            LogError()(OT_PRETTY_CLASS())(
                                "Notary::NotarizeWithdrawal "
                                "(voucher): Failed crediting user "
                                "account.")
                                .Flush();
                        }

                        theAccount.get().GetIdentifier(accountHash);
                        theAccount.Abort();
                        voucherReserveAccount.Abort();
                    } else {
                        auto strVoucher = String::Factory();
                        theVoucher->SetAsVoucher(
                            NYM_ID, ACCOUNT_ID);  // All this does is set the
                                                  // voucher's internal contract
                                                  // string to
                        // "VOUCHER" instead of "CHEQUE". Plus it saves the
                        // remitter's IDs.
                        theVoucher->SignContract(
                            server_.GetServerNym(), reason_);
                        theVoucher->SaveContract();
                        theVoucher->SaveContractRaw(strVoucher);

                        pResponseItem->SetAttachment(strVoucher);
                        pResponseItem->SetStatus(Item::acknowledgement);

                        bOutSuccess = true;  // The withdrawal of a voucher was
                                             // successful.
                        // Release any signatures that were there before (They
                        // won't
                        // verify anymore anyway, since the content has
                        // changed.)
                        theAccount.get().GetIdentifier(accountHash);
                        theAccount.Release();
                        voucherReserveAccount.Release();
                    }
                }
                // else{} // TODO log that there was a problem with the amount

            }  // voucher request loaded successfully from string
        }      // GetTransactor().getVoucherAccount()
        else {
            LogError()(OT_PRETTY_CLASS())(
                "GetTransactor().getVoucherAccount() failed in "
                "NotarizeWithdrawal. "
                "Asset Type: ")(strInstrumentDefinitionID->Get())(".")
                .Flush();
        }
    }
    // WITHDRAW DIGITAL CASH (BLINDED TOKENS)
    //
    // For now, there should only be one of these withdrawal items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    //
    else if (pItem->GetType() == itemType::withdrawal) {
        // The response item will contain a copy of the request item. So I save
        // it into a string
        // here so they can all grab a copy of it into their "in reference to"
        // fields.
        //
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.
        //
        pResponseItem->SetReferenceString(strInReferenceTo);  // the response
                                                              // item carries a
                                                              // copy of what
                                                              // it's responding
                                                              // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN RESPONSE
                                          // to
                                          // pItem and its Owner Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem);  // the response item carries a copy of what it's
                              // responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN RESPONSE
                                          // to
                                          // pItem and its Owner Transaction.

        if (0 > pItem->GetAmount()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Attempt to withdraw a negative amount.")
                .Flush();
        }
        // If the ID on the "from" account that was passed in,
        // does not match the "Acct From" ID on this transaction item
        //
        else if (ACCOUNT_ID != pItem->GetPurportedAccountID()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Error: 'From' account ID on the transaction does "
                "not match 'from' account ID on the withdrawal "
                "item.")
                .Flush();
        } else {
            process_cash_withdrawal(
                tranIn,
                *pItem,
                *pBalanceItem,
                context,
                theAccount,
                accountHash,
                inbox,
                outbox,
                *pResponseItem,
                *pResponseBalanceItem,
                bOutSuccess);
        }

        // sign the response item before sending it back (it's already been
        // added to the transaction above)
        // Now, whether it was rejection or acknowledgement, it is set properly
        // and it is signed, and it
        // is owned by the transaction, who will take it from here.
        pResponseItem->SignContract(server_.GetServerNym(), reason_);
        pResponseItem->SaveContract();  // the signing was of no effect because
                                        // I
                                        // forgot to save.

        pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
        pResponseBalanceItem->SaveContract();
    } else {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())(
            "Expected Item::withdrawal or Item::withdrawVoucher in trans "
            "#")(tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    }

    // For the reply message.
    if (accountHash.empty() && theAccount) {
        theAccount.get().GetIdentifier(accountHash);
    }
    AddHashesToTransaction(tranOut, inbox, outbox, accountHash);

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_.GetServerNym(), reason_);
    pResponseItem->SaveContract();  // the signing was of no effect because I
                                    // forgot to save.

    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
}

/// NotarizePayDividend
///
/// Phase 1: Only the signer on the currency contract (the issuer) can pay a
/// dividend. He must pay the dividend in a currency of a DIFFERENT type. (Such
/// as, a dollar dividend for shares of Pepsi.) So this transaction is a
/// "dollar" transaction, using that example, and theAccount is a dollar
/// account. But then how do we know those dollars are being paid to _Pepsi_
/// shareholders? Because the instrument definition of the shares must be
/// attached to the Item::payDividend within tranIn--and also so must the
/// dividend payout amount, per share" be included, for the same reason. This
/// function gets the asset contract for the shares, and passes a functor to it,
/// so that it can iterate through all the Pepsi asset accounts and form/send a
/// payout voucher for each one (via the functor.) This function also verifies
/// that theNym is both signer on the asset contract for Pepsi shares (the
/// calling function has already verified that theNym is the signer on the
/// dollar account.)
///
/// Phase 2: voting groups, hierarchical entities with agents, oversight,
/// corporate asset accounts, etc.
void Notary::NotarizePayDividend(
    otx::context::Client& context,
    ExclusiveAccount& theSourceAccount,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    Ledger& inbox,
    Ledger& outbox,
    bool& bOutSuccess)
{
    // The outgoing transaction is an "atPayDividend", that is, "a reply to the
    // 'pay dividend' request"
    tranOut.SetType(transactionType::atPayDividend);
    // This pointer and the following one, are 2 pointers, as a vestige
    std::shared_ptr<Item> pItem = nullptr;
    // from the withdrawal code, which has two forms: voucher and cash.
    std::shared_ptr<Item> pItemPayDividend = nullptr;
    // The balance agreement item, which must be on any transaction.
    std::shared_ptr<Item> pBalanceItem = nullptr;
    // Server's response to pItem.
    std::shared_ptr<Item> pResponseItem = nullptr;
    // Server's response to pBalanceItem.
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;
    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();
    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const auto& NOTARY_ID = context.Notary();
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto SOURCE_ACCT_ID =
        server_.API().Factory().Internal().Identifier(theSourceAccount.get());
    const auto& PAYOUT_INSTRUMENT_DEFINITION_ID =
        theSourceAccount.get().GetInstrumentDefinitionID();
    const auto strNymID = String::Factory(NYM_ID, api_.Crypto());
    const auto strAccountID = String::Factory(SOURCE_ACCT_ID, api_.Crypto());
    const auto strInstrumentDefinitionID =
        String::Factory(PAYOUT_INSTRUMENT_DEFINITION_ID, api_.Crypto());
    // Make sure the appropriate item is attached.
    itemType theReplyItemType = itemType::error_state;
    pItemPayDividend = tranIn.GetItem(itemType::payDividend);

    if (nullptr != pItemPayDividend) {
        pItem = pItemPayDividend;
        theReplyItemType = itemType::atPayDividend;
    }
    //

    // Server response item being added to server response transaction (tranOut)
    // (They're getting SOME sort of response item.)
    pResponseItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, theReplyItemType, identifier::Account{})
            .release());
    pResponseItem->SetStatus(Item::rejection);
    // the Transaction's destructor will cleanup the item. It "owns" it now.
    tranOut.AddItem(pResponseItem);
    pResponseBalanceItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atBalanceStatement, identifier::Account{})
            .release());
    pResponseBalanceItem->SetStatus(Item::rejection);
    // the Transaction's destructor will cleanup the item. It "owns" it now.
    tranOut.AddItem(pResponseBalanceItem);

    auto accountHash = identifier::Account{};

    if (nullptr == pItem) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())("Expected Item::payDividend in trans #")(
            tranIn.GetTransactionNum())(": ")(
            strTemp->Exists()
                ? strTemp->Get()
                : " (ERROR SERIALIZING TRANSACTION INTO A STRING)")
            .Flush();
    }
    // Below this point, we know that pItem is good, and that pItemPayDividend
    // is good, and that pItem points to it. Therefore next, let's verify
    // permissions:
    //
    // This permission has to do with ALL withdrawals from an account (cash /
    // voucher / dividends)
    else if (!NYM_IS_ALLOWED(
                 strNymID->Get(), ServerSettings::_transact_withdrawal)) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (All withdrawals are disallowed in "
            "server.cfg, even for paying dividends with.)")
            .Flush();
    }
    // This permission has to do with paying dividends.
    else if (
        (nullptr != pItemPayDividend) &&
        (!NYM_IS_ALLOWED(
            strNymID->Get(), ServerSettings::_transact_pay_dividend))) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (payDividend is disallowed in "
            "server.cfg)")
            .Flush();
    }
    // Check for a balance agreement...
    else if (
        nullptr ==
        (pBalanceItem = tranIn.GetItem(itemType::balanceStatement))) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())(
            "Expected Item::balanceStatement, but not found in trans #")(
            tranIn.GetTransactionNum())(": ")(
            strTemp->Exists()
                ? strTemp->Get()
                : " (ERROR SERIALIZING TRANSACTION INTO A STRING)")
            .Flush();
    }
    // Superfluous by this point. Artifact of withdrawal code.
    else if (pItem->GetType() == itemType::payDividend) {
        // The response item will contain a copy of the request item. So I save
        // it into a string here so they can all grab a copy of it into their
        // "in reference to" fields.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // Make sure the response items know which transaction # they're in
        // response to, and have a copy of the original request-transaction.
        //
        // the response item carries a copy of what it's responding to.
        pResponseItem->SetReferenceString(strInReferenceTo);
        // This response item is IN RESPONSE to pItem and its Owner Transaction.
        pResponseItem->SetReferenceToNum(pItem->GetTransactionNum());
        // the response item carries a copy of what it's responding to.
        pResponseBalanceItem->SetReferenceString(strBalanceItem);
        // This response item is IN RESPONSE to pItem and its Owner Transaction.
        pResponseBalanceItem->SetReferenceToNum(pItem->GetTransactionNum());
        const Amount& lTotalCostOfDividend = pItem->GetAmount();
        auto theVoucherRequest{api_.Factory().InternalSession().Cheque()};

        OT_ASSERT(false != bool(theVoucherRequest));

        auto strVoucherRequest = String::Factory();
        // When paying a dividend, you create a voucher request (the same as in
        // withdrawVoucher). It's just for information
        auto strItemNote = String::Factory();
        // passing, since payDividend needs a few bits of info, and this is a
        // convenient way of passing it.
        pItem->GetAttachment(strVoucherRequest);
        pItem->GetNote(strItemNote);
        const bool bLoadContractFromString =
            theVoucherRequest->LoadContractFromString(strVoucherRequest);

        if (!bLoadContractFromString) {
            LogError()(OT_PRETTY_CLASS())(
                "ERROR loading dividend payout's "
                "voucher request from string: ")(strVoucherRequest.get())
                .Flush();
        } else if (theVoucherRequest->GetAmount() <= 0) {
            LogError()(OT_PRETTY_CLASS())(
                "ERROR expected >0 'payout per share' as 'amount' on request "
                "voucher: ")(strVoucherRequest.get())
                .Flush();
        } else {
            try {
                // the request voucher (sent from client) contains the payout
                // amount per share. Whereas pItem contains
                // lTotalCostOfDividend, which is the total cost (the payout
                // multiplied by number of shares.)
                //
                // already validated, just above.
                const Amount& lAmountPerShare = theVoucherRequest->GetAmount();
                const auto& SHARES_ISSUER_ACCT_ID =
                    theVoucherRequest->GetSenderAcctID();
                const auto strSharesIssuerAcct =
                    String::Factory(SHARES_ISSUER_ACCT_ID, api_.Crypto());
                // Get the asset contract for the shares type, stored in the
                // voucher request, inside pItem. (Make sure it's NOT the same
                // instrument definition as theSourceAccount.get().)
                const identifier::Generic& SHARES_INSTRUMENT_DEFINITION_ID =
                    theVoucherRequest->GetInstrumentDefinitionID();
                auto pSharesContract = api_.Wallet().Internal().UnitDefinition(
                    theVoucherRequest->GetInstrumentDefinitionID());
                auto sharesIssuerAccount =
                    api_.Wallet().Internal().mutable_Account(
                        SHARES_ISSUER_ACCT_ID, reason_);
                const auto& purportedID = context.RemoteNym().ID();

                if (pSharesContract->Type() != contract::UnitType::Security) {
                    const auto strSharesType = String::Factory(
                        SHARES_INSTRUMENT_DEFINITION_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())(
                        "FAILURE: Asset contract is not shares-based. Asset "
                        "type ID: ")(strSharesType.get())
                        .Flush();
                } else if (!(purportedID == pSharesContract->Signer()->ID())) {
                    const auto strSharesType = String::Factory(
                        SHARES_INSTRUMENT_DEFINITION_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())("ERROR only the issuer (")(
                        strNymID.get())(") of contract ")(strSharesType.get())(
                        ") may pay dividends.")
                        .Flush();
                } else if (!pSharesContract->Validate()) {
                    const auto strSharesType = String::Factory(
                        SHARES_INSTRUMENT_DEFINITION_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR unable to verify signature for Nym "
                        "(")(strNymID.get())(
                        ") on shares contract with instrument definition id: ")(
                        strSharesType.get())
                        .Flush();
                } else if (false == bool(sharesIssuerAccount)) {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR unable to find issuer account for shares: ")(
                        strSharesIssuerAcct.get())
                        .Flush();
                } else if (
                    PAYOUT_INSTRUMENT_DEFINITION_ID ==
                    SHARES_INSTRUMENT_DEFINITION_ID)  // these can't be the
                                                      // same
                {
                    const auto strSharesType = String::Factory(
                        PAYOUT_INSTRUMENT_DEFINITION_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR dividend payout attempted, using shares "
                        "instrument definition as payout type also. (It's "
                        "logically impossible for it to payout to itself, "
                        "using ITSELF as the instrument definition for the "
                        "payout): ")(strSharesType.get())
                        .Flush();
                } else if (!sharesIssuerAccount.get().VerifyAccount(
                               server_.GetServerNym())) {
                    const auto strIssuerAcctID =
                        String::Factory(SHARES_ISSUER_ACCT_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR failed trying to verify issuer account: ")(
                        strIssuerAcctID.get())
                        .Flush();
                } else if (!sharesIssuerAccount.get().VerifyOwner(
                               context.RemoteNym())) {
                    const auto strIssuerAcctID =
                        String::Factory(SHARES_ISSUER_ACCT_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR verifying signer's ownership of shares issuer "
                        "account (")(strIssuerAcctID.get())(
                        "), while trying to pay dividend from source "
                        "account: ")(strAccountID.get())
                        .Flush();
                }
                // Make sure the share issuer's account balance (number of
                // shares issued * (-1)), when multiplied by the dividend
                // "amount payout per share", equals the "total cost of
                // dividend" as expected based on the value from
                // pItem->GetAmount.
                //
                //
                else if (
                    (sharesIssuerAccount.get().GetBalance() * (-1) *
                     lAmountPerShare) != lTotalCostOfDividend) {
                    const auto strIssuerAcctID =
                        String::Factory(SHARES_ISSUER_ACCT_ID, api_.Crypto());
                    const auto unittype =
                        api_.Wallet().Internal().CurrencyTypeBasedOnUnitType(
                            sharesIssuerAccount.get()
                                .GetInstrumentDefinitionID());
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR: total payout of dividend as calculated (")(
                        (sharesIssuerAccount.get().GetBalance() * (-1) *
                         lAmountPerShare),
                        unittype)(") doesn't match client's request (")(
                        lTotalCostOfDividend,
                        unittype)(") for source acct: ")(strAccountID.get())
                        .Flush();
                } else if (
                    theSourceAccount.get().GetBalance() <
                    lTotalCostOfDividend) {
                    const auto strIssuerAcctID =
                        String::Factory(SHARES_ISSUER_ACCT_ID, api_.Crypto());
                    const auto unittype =
                        api_.Wallet().Internal().CurrencyTypeBasedOnUnitType(
                            sharesIssuerAccount.get()
                                .GetInstrumentDefinitionID());
                    LogError()(OT_PRETTY_CLASS())(
                        "FAILURE: not enough funds (")(
                        theSourceAccount.get().GetBalance(),
                        unittype)(") to cover total dividend payout (")(
                        lTotalCostOfDividend,
                        unittype)(") for source acct: ")(strAccountID.get())
                        .Flush();
                } else {
                    // Remove all the funds at once (so the balance agreement
                    // matches up.)
                    // Then, iterate through the asset accounts and use a
                    // functor to send a voucher to each one. (Or back to the
                    // issuer, for any that fail.)

                    // UPDATE: unfortunately the balance agreement will be a lie
                    // unless the complete amount is removed.
                    // Therefore, failures must be sent back to the issuer as
                    // individual receipts, containing the vouchers
                    // for any failures, so he can have a record of them, and so
                    // he can recover the funds.

                    // contains the server's funds to back vouchers of a
                    // specific instrument definition.
                    ExclusiveAccount voucherReserveAccount;
                    // If the ID on the "from" account that was passed in, does
                    // not match the "Acct From" ID on this transaction item...
                    //
                    // TODO see if this is already verified by the caller
                    // function and if so, remove. (I believe the item would
                    // have entirely failed to load, if the account ID, and
                    // other IDs, hadn't matched up with the transaction when we
                    // loaded it.)
                    if (SOURCE_ACCT_ID != pItem->GetPurportedAccountID()) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Error: Account ID does not match account ID on "
                            "the "
                            "'pay dividend' item.")
                            .Flush();
                    }
                    // The server will already have a special account for
                    // issuing vouchers. Actually, a list of them -- one for
                    // each instrument definition. Since this is the normal way
                    // of doing business, GetTransactor().getVoucherAccount()
                    // will just create it if it doesn't already exist, and then
                    // return the pointer. Therefore, a failure here is a
                    // catastrophic failure!  Should never fail.
                    //
                    else if (
                        (voucherReserveAccount =
                             server_.GetTransactor().getVoucherAccount(
                                 PAYOUT_INSTRUMENT_DEFINITION_ID)) &&
                        voucherReserveAccount) {
                        const auto VOUCHER_ACCOUNT_ID =
                            server_.API().Factory().Internal().AccountID(
                                voucherReserveAccount.get());

                        // This amount must be the total amount based on the
                        // amount issued. For example if 1000 shares of Pepsi
                        // were issued, and the dividend is $2 per share, then
                        // loading the issuer's account will show a balance of
                        // -1000, and I must have
                        // $2000 in my source account if I am going to pay this
                        // dividend.
                        //
                        // This $2000 is entirely removed from my account at
                        // once, and the below balance agreement must be for
                        // $2000. The vouchers are sent to the owners of each
                        // account, in amounts proportionate to the number of
                        // shares in the account. For any voucher that fails to
                        // be sent (for whatever reason) it is sent back to
                        // theNym instead.
                        //
                        if (!(pBalanceItem->VerifyBalanceStatement(
                                lTotalCostOfDividend * (-1),  // My account's
                                                              // balance will go
                                                              // down by this
                                                              // much.
                                context,
                                inbox,
                                outbox,
                                theSourceAccount.get(),
                                tranIn,
                                UnallocatedSet<TransactionNumber>(),
                                reason_))) {
                            LogError()(OT_PRETTY_CLASS())(
                                "ERROR verifying balance statement while "
                                "trying to pay dividend. Source Acct ID: ")(
                                strAccountID.get())
                                .Flush();
                        } else  // successfully verified the balance agreement.
                        {
                            pResponseBalanceItem->SetStatus(
                                Item::acknowledgement);  // the transaction
                                                         // agreement was
                                                         // successful.
                            // IF we successfully created the voucher, AND the
                            // voucher amount is greater than 0,
                            // AND debited the user's account,
                            // AND credited the server's voucher account,
                            //
                            // THEN save the accounts and pay the dividend out
                            // to the shareholders.
                            //
                            if ((lTotalCostOfDividend > 0) &&
                                theSourceAccount.get().Debit(
                                    lTotalCostOfDividend)  // todo: failsafe:
                                                           // update this code
                                                           // in case of
                                                           // problems in this
                                                           // sensitive area.
                                                           // need better funds
                                                           // transfer code.
                            ) {
                                const auto strVoucherAcctID = String::Factory(
                                    VOUCHER_ACCOUNT_ID, api_.Crypto());

                                if (false ==
                                    voucherReserveAccount.get().Credit(
                                        lTotalCostOfDividend))  // theVoucherRequest->GetAmount()))
                                {
                                    const auto unittype =
                                        api_.Wallet()
                                            .Internal()
                                            .CurrencyTypeBasedOnUnitType(
                                                voucherReserveAccount.get()
                                                    .GetInstrumentDefinitionID());
                                    LogError()(OT_PRETTY_CLASS())(
                                        "Failed crediting ")(
                                        lTotalCostOfDividend, unittype)(
                                        "units to voucher reserve account: ")(
                                        strVoucherAcctID.get())
                                        .Flush();
                                    // Since pVoucherReserveAcct->Credit failed,
                                    // we have to return the funds from
                                    // theSourceAccount.get().Debit (Credit them
                                    // back.)
                                    //
                                    if (false == theSourceAccount.get().Credit(
                                                     lTotalCostOfDividend)) {
                                        LogError()(OT_PRETTY_CLASS())(
                                            "Failed crediting back the user "
                                            "account, after taking his funds "
                                            "and failing to credit them to the "
                                            "voucher reserve account.")
                                            .Flush();
                                    }
                                } else  // By this point, we have taken the full
                                        // funds
                                        // and moved them to the voucher
                                {  // reserve account. So now, let's iterate all
                                   // the
                                    // accounts for that share type,
                                    // and send a voucher to the owner of each
                                    // one, to payout his dividend.

                                    // todo: determine whether I need to attach
                                    // anything here at all...
                                    pResponseItem->SetStatus(
                                        Item::acknowledgement);

                                    bOutSuccess = true;  // The paying of the
                                    // dividends was successful.
                                    //
                                    //
                                    // SAVE THE ACCOUNTS WITH THE NEW BALANCES
                                    // (FUNDS ARE MOVED)
                                    //
                                    // At this point, we save the accounts, so
                                    // that the funds transfer is solid before
                                    // we start mailing vouchers out to people.

                                    // Release any signatures that were there
                                    // before (They won't verify anymore anyway,
                                    // since the content has changed.)
                                    theSourceAccount.get().GetIdentifier(
                                        accountHash);
                                    theSourceAccount.Release();

                                    // We also need to save the Voucher cash
                                    // reserve account. (Any issued voucher
                                    // cheque is automatically backed by this
                                    // reserve account. If a cheque is
                                    // deposited, the funds come back out of
                                    // this account. If the cheque expires, then
                                    // after the expiry period, if it remains in
                                    // the account, it is now the property of
                                    // the transaction server.)
                                    voucherReserveAccount.Release();

                                    //
                                    // PAY THE SHAREHOLDERS
                                    //
                                    // Here's where we actually loop through the
                                    // asset accounts for the share type,
                                    // and send a voucher to the owner of each
                                    // one.
                                    PayDividendVisitor actionPayDividend(
                                        server_,
                                        NOTARY_ID,
                                        NYM_ID,
                                        PAYOUT_INSTRUMENT_DEFINITION_ID,
                                        VOUCHER_ACCOUNT_ID,
                                        strInReferenceTo,  // Memo for each
                                                           // voucher
                                                           // (containing
                                                           // original payout
                                                           // request pItem)
                                        lAmountPerShare);

                                    // Loops through all the accounts for a
                                    // given instrument definition
                                    // (PAYOUT_INSTRUMENT_DEFINITION_ID),
                                    // and triggers
                                    // actionPayDividend for each one. This
                                    // sends the owner nym for each, a voucher
                                    // drawn on VOUCHER_ACCOUNT_ID. (In the
                                    // amount of lAmountPerShare * number of
                                    // shares in account.)
                                    //
                                    const bool bForEachAcct =
                                        pSharesContract->VisitAccountRecords(
                                            api_.DataFolder().string(),
                                            actionPayDividend,
                                            reason_);  // <================
                                                       // pay all the
                                                       // dividends here.

                                    // TODO: Since the above line of code loops
                                    // through all the accounts and loads them
                                    // up, transforms them, and saves them
                                    // again, we cannot use our own loaded
                                    // accounts below this point. (They could
                                    // overwrite themselves.) theSourceAccount
                                    // especially, was passed in from above --
                                    // so how can we possible warn the caller
                                    // than he cannot save this account without
                                    // overwriting work we have done in this
                                    // function?
                                    //
                                    // Aside from any more elegant solution, the
                                    // only way to make it work in this case
                                    // would be to make a map or list of all the
                                    // accounts that are already loaded in
                                    // memory (such as theSourceAccount) and
                                    // PASS THEM IN to the above
                                    // VisitAccountRecords call. This way it
                                    // would have the option to use the "already
                                    // loaded" versions, where appropriate,
                                    // instead of loading them twice. (As it is,
                                    // theSourceAccount is not used below this
                                    // point, though we couldn't preven the
                                    // caller from using it.)
                                    //
                                    // Therefore we need to have some central
                                    // system where accounts can be loaded,
                                    // locked, saved, etc. So we cannot ever
                                    // overwrite ourselves BY DESIGN. (And the
                                    // same for other data types as well, like
                                    // Nyms.) Todo.
                                    //
                                    if (!bForEachAcct)  // todo failsafe. Handle
                                                        // this
                                                        // better.
                                    {
                                        LogError()(OT_PRETTY_CLASS())(
                                            "ERROR: After moving funds for "
                                            "dividend payment, there was some "
                                            "error when sending out the "
                                            "vouchers to the payout "
                                            "recipients.")
                                            .Flush();
                                    }
                                    //
                                    // REFUND ANY LEFTOVERS
                                    //
                                    const Amount lLeftovers =
                                        lTotalCostOfDividend -
                                        (actionPayDividend.GetAmountPaidOut() +
                                         actionPayDividend.GetAmountReturned());
                                    if (lLeftovers > 0) {
                                        // Of the total amount removed from the
                                        // sender's account, and after paying
                                        // all dividends, there was a leftover
                                        // amount that wasn't paid to anybody.
                                        // Therefore, we should pay it back to
                                        // the sender himself, now.
                                        //
                                        const auto unittype =
                                            api_.Wallet()
                                                .Internal()
                                                .CurrencyTypeBasedOnUnitType(
                                                    PAYOUT_INSTRUMENT_DEFINITION_ID);
                                        LogError()(OT_PRETTY_CLASS())(
                                            "After dividend payout, with ")(
                                            lTotalCostOfDividend, unittype)(
                                            " units removed initially, there "
                                            "were ")(lLeftovers, unittype)(
                                            " units remaining. (Returning them "
                                            "to sender...)")
                                            .Flush();
                                        auto theVoucher{
                                            api_.Factory().InternalSession().Cheque(
                                                NOTARY_ID,
                                                PAYOUT_INSTRUMENT_DEFINITION_ID)};
                                        const auto VALID_FROM = Clock::now();
                                        const auto VALID_TO =
                                            VALID_FROM +
                                            std::chrono::hours(24 * 30 * 6);
                                        std::int64_t lNewTransactionNumber = 0;
                                        const bool bGotNextTransNum =
                                            server_.GetTransactor()
                                                .issueNextTransactionNumberToNym(
                                                    context,
                                                    lNewTransactionNumber);
                                        // We save the
                                        // transaction
                                        // number on the server Nym (normally
                                        // we'd discard it) because when the
                                        // cheque is deposited, the server nym,
                                        // as the owner of the voucher account,
                                        // needs to verify the transaction # on
                                        // the cheque (to prevent
                                        // double-spending of cheques.)
                                        if (bGotNextTransNum) {
                                            const auto& NOTARY_NYM_ID =
                                                server_.GetServerNym().ID();
                                            const bool bIssueVoucher =
                                                theVoucher->IssueCheque(
                                                    lLeftovers,  // The amount
                                                                 // of the
                                                                 // cheque.
                                                    lNewTransactionNumber,  // Requiring
                                                                            // a
                                                                            // transaction
                                                                            // number
                                                                            // prevents
                                                                            // double-spending
                                                                            // of
                                                                            // cheques.
                                                    VALID_FROM,  // The
                                                                 // expiration
                                                                 // date (valid
                                                                 // from/to
                                                                 // dates) of
                                                                 // the cheque
                                                    VALID_TO,    // Vouchers are
                                                               // automatically
                                                    // starting today and
                                                    // lasting 6 months.
                                                    VOUCHER_ACCOUNT_ID,  // The
                                                                         // asset
                                                                         // account
                                                                         // the
                                                                         // cheque
                                                                         // is
                                                                         // drawn
                                                                         // on.
                                                    NOTARY_NYM_ID,  // Nym ID of
                                                                    // the
                                                                    // sender
                                                                    // (in this
                                                                    // case the
                                                                    // server
                                                                    // nym.)
                                                    strInReferenceTo,  // Optional
                                                                       // memo
                                                                       // field.
                                                                       // Includes
                                                                       // item
                                                                       // note
                                                                       // and
                                                                       // request
                                                                       // memo.
                                                    NYM_ID);

                                            // All account crediting / debiting
                                            // happens in the caller, in Server.
                                            //    (AND it happens only ONCE, to
                                            // cover ALL vouchers.)
                                            // Then in here, the voucher either
                                            // gets send to the recipient, or if
                                            // error, sent back home to the
                                            // issuer Nym. (ALL the funds are
                                            // removed, then the vouchers are
                                            // sent one way or the other.) Any
                                            // returned vouchers, obviously
                                            // serve to notify the dividend
                                            // payer of where the errors were
                                            // (as well as give him the
                                            // opportunity to get his money
                                            // back.)
                                            //
                                            bool bSent = false;
                                            if (bIssueVoucher) {
                                                theVoucher->SetAsVoucher(
                                                    NOTARY_NYM_ID,
                                                    VOUCHER_ACCOUNT_ID);  // All
                                                                          // this
                                                                          // does
                                                                          // is
                                                                          // set
                                                                          // the
                                                // voucher's
                                                // internal
                                                // contract
                                                // string
                                                theVoucher->SignContract(
                                                    server_.GetServerNym(),
                                                    reason_);  // to
                                                // "VOUCHER"
                                                // instead of
                                                // "CHEQUE".
                                                theVoucher->SaveContract();

                                                // Send the voucher to the
                                                // payments inbox of the
                                                // recipient.
                                                //
                                                const auto strVoucher =
                                                    String::Factory(
                                                        *theVoucher);
                                                auto thePayment{
                                                    api_.Factory()
                                                        .InternalSession()
                                                        .Payment(strVoucher)};

                                                // calls DropMessageToNymbox
                                                bSent =
                                                    server_.SendInstrumentToNym(
                                                        NOTARY_ID,
                                                        NOTARY_NYM_ID,  // sender
                                                                        // nym
                                                        NYM_ID,  // recipient
                                                                 // nym
                                                                 // (returning
                                                                 // to
                                                        // original sender.)
                                                        *thePayment,
                                                        "payDividend");  // todo:
                                                // hardcoding.
                                            }
                                            // If we didn't send it, then we
                                            // need to return the funds to where
                                            // they came from.
                                            //
                                            if (!bSent) {
                                                const auto
                                                    strPayoutInstrumentDefinitionID =
                                                        String::Factory(
                                                            PAYOUT_INSTRUMENT_DEFINITION_ID,
                                                            api_.Crypto()),
                                                    strSenderNymID =
                                                        String::Factory(
                                                            NYM_ID,
                                                            api_.Crypto());
                                                LogError()(OT_PRETTY_CLASS())(
                                                    "ERROR failed issuing "
                                                    "voucher (to return "
                                                    "leftovers back to the "
                                                    "dividend payout "
                                                    "initiator.) WAS TRYING TO "
                                                    "PAY ")(
                                                    lLeftovers,
                                                    unittype)(" of "
                                                              "instrument "
                                                              "definition ")(
                                                    strPayoutInstrumentDefinitionID
                                                        .get())(" to Nym ")(
                                                    strSenderNymID.get())
                                                    .Flush();
                                            }   // if !bSent
                                        } else  // !bGotNextTransNum
                                        {
                                            const auto
                                                strPayoutInstrumentDefinitionID =
                                                    String::Factory(
                                                        PAYOUT_INSTRUMENT_DEFINITION_ID,
                                                        api_.Crypto()),
                                                strRecipientNymID =
                                                    String::Factory(
                                                        NYM_ID, api_.Crypto());
                                            LogError()(OT_PRETTY_CLASS())(
                                                "ERROR!! Failed issuing next "
                                                "transaction number while "
                                                "trying to send a voucher "
                                                "(while returning leftover "
                                                "funds, after paying "
                                                "dividends.) WAS TRYING TO "
                                                "PAY ")(lLeftovers, unittype)(
                                                " of asset type ")(
                                                strPayoutInstrumentDefinitionID
                                                    .get())(" to Nym ")(
                                                strRecipientNymID.get())
                                                .Flush();
                                        }
                                    }
                                }  // else
                            }
                            // else{} // TODO log that there was a problem with
                            // the amount

                        }  // voucher request loaded successfully from string
                    }      // server_.GetTransactor().getVoucherAccount()
                    else {
                        LogError()(OT_PRETTY_CLASS())(
                            "server_.GetTransactor().getVoucherAccount() "
                            "failed. "
                            "Asset Type: ")(strInstrumentDefinitionID.get())
                            .Flush();
                    }
                }
            } catch (...) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR unable to find shares contract based on "
                    "instrument definition")
                    .Flush();
            }
        }
    } else {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())("Expected Item::payDividend in trans #")(
            tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    }

    // For the reply message.
    if (accountHash.empty() && theSourceAccount) {
        theSourceAccount.get().GetIdentifier(accountHash);
    }
    AddHashesToTransaction(tranOut, inbox, outbox, accountHash);

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_.GetServerNym(), reason_);
    pResponseItem->SaveContract();  // the signing was of no effect because I
                                    // forgot to save.

    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
}

/// for depositing a cheque or cash.
void Notary::NotarizeDeposit(
    otx::context::Client& context,
    ExclusiveAccount& theAccount,
    OTTransaction& input,
    OTTransaction& output,
    Ledger& inbox,
    Ledger& outbox,
    bool& success)
{
    const auto& nymID = context.Signer()->ID();
    output.SetType(transactionType::atDeposit);
    std::shared_ptr<const Item> depositItem{nullptr};
    std::shared_ptr<const Item> balanceItem{
        input.GetItem(itemType::balanceStatement)};
    std::shared_ptr<Item> responseItem{nullptr};
    std::shared_ptr<Item> responseBalanceItem{nullptr};
    itemType type{itemType::error_state};
    bool permission = NYM_IS_ALLOWED(
        nymID.asBase58(api_.Crypto()), ServerSettings::_transact_deposit);

    if (input.GetItem(itemType::depositCheque)) {
        type = itemType::atDepositCheque;
        depositItem = input.GetItem(itemType::depositCheque);
        permission &= NYM_IS_ALLOWED(
            nymID.asBase58(api_.Crypto()),
            ServerSettings::_transact_deposit_cheque);
    } else if (input.GetItem(itemType::deposit)) {
        type = itemType::atDeposit;
        depositItem = input.GetItem(itemType::deposit);
        permission &= NYM_IS_ALLOWED(
            nymID.asBase58(api_.Crypto()),
            ServerSettings::_transact_deposit_cash);
    }

    responseItem.reset(api_.Factory()
                           .InternalSession()
                           .Item(output, type, identifier::Account{})
                           .release());
    responseItem->SetStatus(Item::rejection);
    output.AddItem(responseItem);
    responseBalanceItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(output, itemType::atBalanceStatement, identifier::Account{})
            .release());
    responseBalanceItem->SetStatus(Item::rejection);
    output.AddItem(responseBalanceItem);

    OT_ASSERT(responseItem);
    OT_ASSERT(responseBalanceItem);

    Finalize signer(
        server_.GetServerNym(), *responseItem, *responseBalanceItem, reason_);

    if (false == permission) {
        LogError()(OT_PRETTY_CLASS())(
            "Server configuration does not permit deposits.")
            .Flush();

        return;
    }

    if (false == bool(depositItem)) {
        LogError()(OT_PRETTY_CLASS())("Deposit transaction ")(
            input.GetTransactionNum())(" does not "
                                       "contain a "
                                       "deposit item.")
            .Flush();

        return;
    }

    if (false == bool(balanceItem)) {
        LogError()(OT_PRETTY_CLASS())("Deposit transaction ")(
            input.GetTransactionNum())(" does not "
                                       "contain a "
                                       "balance "
                                       "agreement "
                                       "item.")
            .Flush();

        return;
    }

    OT_ASSERT(depositItem);
    OT_ASSERT(balanceItem);

    switch (type) {
        case itemType::atDepositCheque: {
            process_cheque_deposit(
                input,
                *depositItem,
                *balanceItem,
                context,
                theAccount,
                output,
                inbox,
                outbox,
                success,
                *responseItem,
                *responseBalanceItem);
        } break;
        case itemType::atDeposit: {
            process_cash_deposit(
                input,
                *depositItem,
                *balanceItem,
                context,
                theAccount,
                output,
                inbox,
                outbox,
                success,
                *responseItem,
                *responseBalanceItem);
        } break;
        case itemType::transfer:
        case itemType::atTransfer:
        case itemType::acceptTransaction:
        case itemType::atAcceptTransaction:
        case itemType::acceptMessage:
        case itemType::atAcceptMessage:
        case itemType::acceptNotice:
        case itemType::atAcceptNotice:
        case itemType::acceptPending:
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
        case itemType::withdrawVoucher:
        case itemType::atWithdrawVoucher:
        case itemType::depositCheque:
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
            LogError()(OT_PRETTY_CLASS())("Invalid deposit item type.").Flush();

            break;
        }
    }
}

// DONE:  Need to make sure both parties have included TWO!!! transaction
// numbers, so both
// have the option to cancel later!  (And so the server can expire it later, and
// cover its own ass.)
//
// Note: still need to do something with those numbers upon closing. (cron
// expiration, and cancelCronItem.)
//

// DONE: The current version verifies that it's signed by both parties.
//  Fix it so that it loads the merchant's copy to verify recipient signature.

/// 1) The Merchant generates the payment plan, adds transaction numbers, and
/// signs. (All done via ProposePaymentPlan)
/// 2) Then the Customer uses ConfirmPaymentPlan to add his own numbers and
/// sign.
/// 3) Then the Customer must activate the payment plan. (Using a transaction
/// with the same number as the plan.)
void Notary::NotarizePaymentPlan(
    otx::context::Client& context,
    ExclusiveAccount& theDepositorAccount,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    bool& bOutSuccess)
{
    // The outgoing transaction is an "atPaymentPlan", that is, "a reply to the
    // paymentPlan request"
    tranOut.SetType(transactionType::atPaymentPlan);

    std::shared_ptr<Item> pItem = nullptr;
    std::shared_ptr<Item> pBalanceItem = nullptr;
    std::shared_ptr<Item> pResponseItem = nullptr;
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will definitely be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const auto& NOTARY_ID = context.Notary();
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto& DEPOSITOR_NYM_ID = NYM_ID;
    const auto DEPOSITOR_ACCT_ID =
        server_.API().Factory().Internal().Identifier(
            theDepositorAccount.get());
    const auto strNymID = String::Factory(NYM_ID, api_.Crypto());
    pItem = tranIn.GetItem(itemType::paymentPlan);
    pBalanceItem = tranIn.GetItem(itemType::transactionStatement);
    pResponseItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atPaymentPlan, identifier::Account{})
            .release());
    pResponseItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseItem);  // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.
    pResponseBalanceItem.reset(api_.Factory()
                                   .InternalSession()
                                   .Item(
                                       tranOut,
                                       itemType::atTransactionStatement,
                                       identifier::Account{})
                                   .release());
    pResponseBalanceItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseBalanceItem);  // the Transaction's destructor
                                            // will cleanup the item. It "owns"
                                            // it now.

    auto accountHash = identifier::Account{};

    if ((nullptr != pItem) &&
        (!NYM_IS_ALLOWED(
            strNymID->Get(), ServerSettings::_transact_payment_plan))) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (All payment "
            "plans are disallowed in "
            "server.cfg)")
            .Flush();
    }
    // For now, there should only be one of these paymentPlan items inside the
    // transaction. So we treat it that way... I either get it successfully or
    // not.
    else if ((nullptr == pItem) || (nullptr == pBalanceItem)) {
        LogError()(OT_PRETTY_CLASS())("Error, expected Item::paymentPlan and "
                                      "Item::transactionStatement.")
            .Flush();
    } else {
        if (DEPOSITOR_ACCT_ID != pItem->GetPurportedAccountID()) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Source account ID on the transaction does not match "
                "sender's account ID on the transaction item.")
                .Flush();
        } else if (!pBalanceItem->VerifyTransactionStatement(context, tranIn)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed verifying transaction statement.")
                .Flush();
        } else {
            pResponseBalanceItem->SetStatus(
                Item::acknowledgement);  // the transaction agreement was
                                         // successful.

            // The response item will contain a copy of the request item. So I
            // save it into a string here so it can be saved into the "in
            // reference to" field.
            pItem->SaveContractRaw(strInReferenceTo);
            pBalanceItem->SaveContractRaw(strBalanceItem);
            // Server response item being added to server response transaction
            // (tranOut) They're getting SOME sort of response item.
            //
            // the response item carries a copy of what it's responding to.
            pResponseItem->SetReferenceString(strInReferenceTo);
            // This response item is IN RESPONSE to pItem and its Owner
            // Transaction.
            pResponseItem->SetReferenceToNum(pItem->GetTransactionNum());
            // the response item carries a copy of what it's responding to.
            pResponseBalanceItem->SetReferenceString(strBalanceItem);
            // This response item is IN RESPONSE to pItem and its Owner
            // Transaction.
            pResponseBalanceItem->SetReferenceToNum(pItem->GetTransactionNum());
            // Also load up the Payment Plan from inside the transaction item.
            auto strPaymentPlan = String::Factory();
            pItem->GetAttachment(strPaymentPlan);
            auto pPlan = api_.Factory().InternalSession().PaymentPlan();

            OT_ASSERT(nullptr != pPlan);

            // If we failed to load the plan...
            if ((false == pPlan->LoadContractFromString(strPaymentPlan))) {
                LogError()(OT_PRETTY_CLASS())("ERROR loading payment plan from "
                                              "string: ")(strPaymentPlan.get())
                    .Flush();
            } else if (pPlan->GetNotaryID() != NOTARY_ID) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR bad server ID on payment plan.")
                    .Flush();
            } else if (
                pPlan->GetInstrumentDefinitionID() !=
                theDepositorAccount.get().GetInstrumentDefinitionID()) {
                const auto
                    strInstrumentDefinitionID1 = String::Factory(
                        pPlan->GetInstrumentDefinitionID(), api_.Crypto()),
                    strInstrumentDefinitionID2 = String::Factory(
                        theDepositorAccount.get().GetInstrumentDefinitionID(),
                        api_.Crypto());
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR wrong Instrument Definition ID "
                    "(")(strInstrumentDefinitionID1.get())(
                    ") on payment plan. "
                    "Expected:"
                    " ")(strInstrumentDefinitionID2.get())
                    .Flush();
            } else {
                // CANCELLING? OR ACTIVATING?
                // If he is cancelling the payment plan (from his outpayments
                // box, before it's even had a chance
                // to be activated by the sender) then the recipient (merchant)
                // will be the depositor. Otherwise,
                // if he is activating the payment plan (from his payments
                // inbox) then the sender (customer) will
                // be the depositor.

                auto theCancelerNymID = identifier::Nym{};
                const bool bCancelling =
                    (pPlan->IsCanceled() &&
                     pPlan->GetCancelerID(theCancelerNymID));
                const TransactionNumber lExpectedNum =
                    bCancelling ? 0 : pItem->GetTransactionNum();
                const TransactionNumber lFoundNum = pPlan->GetTransactionNum();
                const auto& FOUND_NYM_ID = bCancelling
                                               ? pPlan->GetRecipientNymID()
                                               : pPlan->GetSenderNymID();
                const auto& FOUND_ACCT_ID = bCancelling
                                                ? pPlan->GetRecipientAcctID()
                                                : pPlan->GetSenderAcctID();
                const TransactionNumber lFoundOpeningNum =
                    pPlan->GetOpeningNumber(FOUND_NYM_ID);
                const TransactionNumber lFoundClosingNum =
                    pPlan->GetClosingNumber(FOUND_ACCT_ID);

                if (lFoundNum != lExpectedNum) {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR bad main transaction number "
                        "while ")(bCancelling ? "cancelling" : "activating")(
                        " "
                        "p"
                        "a"
                        "y"
                        "m"
                        "e"
                        "n"
                        "t"
                        " "
                        "p"
                        "l"
                        "a"
                        "n"
                        " "
                        "(")(lFoundNum)("). Expected based on transaction: ")(
                        lExpectedNum)
                        .Flush();
                }

                if (lFoundOpeningNum != pItem->GetTransactionNum()) {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR bad transaction number "
                        "while ")(bCancelling ? "cancelling" : "activating")(
                        "p"
                        "a"
                        "y"
                        "m"
                        "e"
                        "n"
                        "t"
                        " "
                        "p"
                        "l"
                        "a"
                        "n"
                        " "
                        "(")(
                        lFoundOpeningNum)("). Expected based on transaction: ")(
                        pItem->GetTransactionNum())
                        .Flush();
                } else if (FOUND_NYM_ID != DEPOSITOR_NYM_ID) {
                    const auto strIDExpected =
                                   String::Factory(FOUND_NYM_ID, api_.Crypto()),
                               strIDDepositor = String::Factory(
                                   DEPOSITOR_NYM_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR wrong user ID "
                        "while ")(bCancelling ? "cancelling" : "activating")(
                        "p"
                        "a"
                        "y"
                        "m"
                        "e"
                        "n"
                        "t"
                        " "
                        "p"
                        "l"
                        "a"
                        "n"
                        "."
                        " "
                        "D"
                        "e"
                        "p"
                        "o"
                        "s"
                        "i"
                        "t"
                        "o"
                        "r"
                        ":"
                        " ")(strIDDepositor.get())(" Found on plan: ")(
                        strIDExpected.get())
                        .Flush();
                } else if (
                    bCancelling && (DEPOSITOR_NYM_ID != theCancelerNymID)) {
                    const auto strIDExpected = String::Factory(
                                   DEPOSITOR_NYM_ID, api_.Crypto()),
                               strIDDepositor = String::Factory(
                                   theCancelerNymID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR wrong canceler Nym ID while canceling payment "
                        "plan. Depositor: ")(strIDExpected.get())(
                        " Canceler:"
                        " ")(strIDDepositor.get())
                        .Flush();
                } else if (FOUND_ACCT_ID != DEPOSITOR_ACCT_ID) {
                    const auto strAcctID1 = String::Factory(
                                   FOUND_ACCT_ID, api_.Crypto()),
                               strAcctID2 = String::Factory(
                                   DEPOSITOR_ACCT_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())("ERROR wrong Acct ID "
                                                  "(")(strAcctID1.get())(
                        ") "
                        "while"
                        " ")(bCancelling ? "cancelling" : "activating")(
                        " payment plan. Expected: ")(strAcctID2.get())
                        .Flush();
                }
                // If we're activating the plan (versus cancelling) then the
                // transaction number opens
                // the payment plan, but there must also be a closing number for
                // closing it.
                else if (
                    !bCancelling &&  // If activating and:
                    ((pPlan->GetCountClosingNumbers() < 1) ||
                     // ...if there aren't enough closing numbers... ...or
                     // the official closing # isn't available for use on
                     // theNym.
                     !context.VerifyIssuedNumber(lFoundClosingNum))) {
                    // We don't check opening number here, since
                    // NotarizeTransaction already did.
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR: the Closing "
                        "number ")(lFoundClosingNum)(" wasn't "
                                                     "available "
                                                     "for use "
                                                     "while "
                                                     "activating "
                                                     "a payment "
                                                     "plan.")
                        .Flush();
                } else if (
                    bCancelling &&  // If cancelling and:
                    ((pPlan->GetRecipientCountClosingNumbers() < 2) ||
                     !context.VerifyIssuedNumber(lFoundClosingNum))) {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR: the Closing number wasn't available for use "
                        "while cancelling a payment plan.")
                        .Flush();
                } else  // The plan is good (so far.)
                {
                    // The RECIPIENT_ACCT_ID is the ID on the "To" Account.
                    // (When doing a transfer, normally 2nd acct is the Payee.)
                    const auto& RECIPIENT_ACCT_ID = pPlan->GetRecipientAcctID();
                    auto rContext =
                        api_.Wallet().Internal().mutable_ClientContext(
                            pPlan->GetRecipientNymID(), reason_);

                    if (!bCancelling &&
                        (DEPOSITOR_ACCT_ID == RECIPIENT_ACCT_ID))  // ACTIVATING
                    {
                        LogError()(OT_PRETTY_CLASS())(
                            "Error: Source account ID matches Recipient "
                            "account ID on attempted Payment Plan "
                            "notarization.")
                            .Flush();
                    }
                    // Unless you are cancelling...
                    else if (
                        bCancelling &&
                        (DEPOSITOR_ACCT_ID != RECIPIENT_ACCT_ID))  // CANCELLING
                    {
                        LogError()(OT_PRETTY_CLASS())(
                            "Error: Source account ID doesn't match "
                            "Recipient account ID on attempted Payment Plan "
                            "cancellation.")
                            .Flush();
                    } else if (
                        !bCancelling &&
                        !pPlan->VerifyAgreement(rContext.get(), context)) {
                        LogError()(OT_PRETTY_CLASS())(
                            "ERROR verifying Sender and Recipient on Payment "
                            "Plan (against merchant and customer copies.)")
                            .Flush();
                    }
                    // This is now done above, in VerifyAgreement().
                    // We only have it here now in cases of cancellation (where
                    // VerifyAgreement isn't called.)
                    else if (
                        bCancelling &&
                        !pPlan->VerifySignature(*rContext.get().Signer())) {
                        LogConsole()(OT_PRETTY_CLASS())(
                            "ERROR verifying Recipient's signature on "
                            "Payment Plan.")
                            .Flush();
                    } else {
                        // Verify that BOTH of the Recipient's transaction
                        // numbers (opening and
                        // closing) are available for use.
                        //
                        // These three blocks are only checked if we are
                        // activating, not
                        // cancelling. Why? Because if we're canceling, then we
                        // ALREADY checked
                        // these things above. But if we're activating, that
                        // means we checked
                        // the sender above only, and thus we still need to
                        // check the recipient.
                        //
                        if (!bCancelling &&
                            pPlan->GetRecipientCountClosingNumbers() < 2) {
                            LogError()(OT_PRETTY_CLASS())(
                                "ERROR verifying Recipient's Opening and "
                                "Closing number on a Payment Plan (he should "
                                "have two numbers, but he doesn't.)")
                                .Flush();
                        } else if (
                            !bCancelling &&
                            !rContext.get().VerifyIssuedNumber(
                                pPlan->GetRecipientOpeningNum())) {
                            LogError()(OT_PRETTY_CLASS())(
                                "ERROR verifying Recipient's opening "
                                "transaction number on a payment plan.")
                                .Flush();
                        } else if (
                            !bCancelling &&
                            !rContext.get().VerifyIssuedNumber(
                                pPlan->GetRecipientClosingNum())) {
                            LogError()(OT_PRETTY_CLASS())(
                                "ERROR verifying Recipient's Closing "
                                "transaction number on a Payment Plan.")
                                .Flush();
                        } else {
                            // Load up the recipient ACCOUNT and validate it.
                            //
                            Account* pRecipientAcct{nullptr};
                            ExclusiveAccount recipientAccount{};

                            if (!bCancelling)  // ACTIVATING
                            {
                                recipientAccount =
                                    api_.Wallet().Internal().mutable_Account(
                                        RECIPIENT_ACCT_ID, reason_);
                                pRecipientAcct = &recipientAccount.get();
                            } else  // CANCELLING
                            {
                                pRecipientAcct = &theDepositorAccount.get();
                            }
                            //
                            if (nullptr == pRecipientAcct) {
                                LogError()(OT_PRETTY_CLASS())(
                                    "ERROR loading Recipient account.")
                                    .Flush();
                            } else if (!pRecipientAcct->VerifyOwner(
                                           rContext.get().RemoteNym())) {
                                LogError()(OT_PRETTY_CLASS())(
                                    "ERROR verifying ownership of the "
                                    "recipient account.")
                                    .Flush();
                            } else if (pRecipientAcct->IsInternalServerAcct()) {
                                LogError()(OT_PRETTY_CLASS())(
                                    "Failed: recipient account is an "
                                    "internal server account (currently "
                                    "prohibited.)")
                                    .Flush();
                            }
                            // Are both of the accounts of the same Asset Type?
                            // VERY IMPORTANT!!
                            else if (
                                pRecipientAcct->GetInstrumentDefinitionID() !=
                                theDepositorAccount.get()
                                    .GetInstrumentDefinitionID()) {
                                auto strSourceInstrumentDefinitionID =
                                         String::Factory(
                                             theDepositorAccount.get()
                                                 .GetInstrumentDefinitionID(),
                                             api_.Crypto()),
                                     strRecipInstrumentDefinitionID =
                                         String::Factory(
                                             pRecipientAcct
                                                 ->GetInstrumentDefinitionID(),
                                             api_.Crypto());
                                LogError()(OT_PRETTY_CLASS())(
                                    "ERROR - user attempted "
                                    "to ")(bCancelling ? "cancel" : "activate")(
                                    " a payment plan between dissimilar "
                                    "instrument definitions: ")(
                                    strSourceInstrumentDefinitionID.get())(
                                    ", ")(strRecipInstrumentDefinitionID.get())
                                    .Flush();
                            }
                            // Does it verify? I call VerifySignature here since
                            // VerifyContractID
                            // was already called in LoadExistingAccount().
                            else if (!pRecipientAcct->VerifySignature(
                                         server_.GetServerNym())) {
                                LogError()(OT_PRETTY_CLASS())(
                                    "ERROR verifying signature on the "
                                    "Recipient account.")
                                    .Flush();
                            }
                            // This one is superfluous, but I'm leaving it.
                            // (pPlan and pRecip are
                            // both already matches to a 3rd value: source acct
                            // instrument
                            // definition ID.)
                            else if (
                                pRecipientAcct->GetInstrumentDefinitionID() !=
                                pPlan->GetInstrumentDefinitionID()) {
                                const auto
                                    strInstrumentDefinitionID1 =
                                        String::Factory(
                                            pPlan->GetInstrumentDefinitionID(),
                                            api_.Crypto()),
                                    strInstrumentDefinitionID2 =
                                        String::Factory(
                                            pRecipientAcct
                                                ->GetInstrumentDefinitionID(),
                                            api_.Crypto());
                                LogError()(OT_PRETTY_CLASS())(
                                    "ERROR wrong Asset Type ID "
                                    "(")(strInstrumentDefinitionID2.get())(
                                    ") on "
                                    "Recipient"
                                    " Acct. "
                                    "Expected "
                                    "per "
                                    "Plan:"
                                    " ")(strInstrumentDefinitionID1.get())
                                    .Flush();
                            }
                            // At this point I feel pretty confident that the
                            // Payment Plan is a
                            // valid request from both parties. I have both
                            // users AND both accounts
                            // and validated against the Payment Plan,
                            // signatures and all. The only
                            // other possibility is that the merchant is
                            // canceling the payment plan
                            // before the customer had a chance to
                            // confirm/activate it.
                            else {
                                // If activating, add it to Cron...
                                //
                                // We add the payment plan to the server's Cron
                                // object, which does
                                // regular processing. That object will take
                                // care of processing the
                                // payment plan according to its terms.
                                //
                                // NOTE: FYI, inside AddCronItem, since this is
                                // a new CronItem, a Cron
                                // Receipt will be saved with the User's
                                // signature on it, containing the
                                // Cron Item from the user's original request.
                                // After that, the item is
                                // stored internally to Cron itself, and signed
                                // by the server--and
                                // changes over time as cron processes. (The
                                // original receipt can always
                                // be loaded when necessary.)
                                //
                                std::shared_ptr<OTPaymentPlan> plan{
                                    pPlan.release()};
                                if (!bCancelling &&
                                    server_.Cron().AddCronItem(
                                        plan,
                                        true,
                                        Clock::now()))  // bSaveReceipt=true
                                {
                                    // todo need to be able to "roll back" if
                                    // anything inside this block fails.

                                    // Now we can set the response item as an
                                    // acknowledgement instead of the
                                    // default (rejection)
                                    pResponseItem->SetStatus(
                                        Item::acknowledgement);

                                    bOutSuccess = true;  // The payment plan
                                                         // activation was
                                                         // successful.
                                    LogDetail()(OT_PRETTY_CLASS())(
                                        "Successfully added payment plan to "
                                        "Cron object.")
                                        .Flush();

                                    // Server side, the Nym stores a list of all
                                    // open cron item numbers. (So
                                    // we know if there is still stuff open on
                                    // Cron for that Nym, and we
                                    // know what it is.)
                                    context.OpenCronItem(
                                        plan->GetTransactionNum());
                                    context.OpenCronItem(plan->GetClosingNum());

                                    // This just marks the Closing number so I
                                    // can't USE it again. (Since
                                    // I'm using it as the closing number for
                                    // this cron item now.) I'm still
                                    // RESPONSIBLE for the number until
                                    // RemoveIssuedNumber() is called. If
                                    // we didn't call this here, then I could
                                    // come back later and USE THE
                                    // NUMBER AGAIN! (Bad!)
                                    // server_.GetTransactor().removeTransactionNumber
                                    // was
                                    // already called for
                                    // tranIn->GetTransactionNum() (That's the
                                    // opening
                                    // number.)
                                    //
                                    // Here's the closing number:
                                    context.ConsumeAvailable(
                                        plan->GetClosingNum());
                                    // RemoveIssuedNum will be called for that
                                    // original transaction number
                                    // when the finalReceipt is created.
                                    // RemoveIssuedNum will be called for
                                    // the Closing number when the finalReceipt
                                    // is accepted.

                                    context.OpenCronItem(
                                        plan->GetRecipientOpeningNum());
                                    context.OpenCronItem(
                                        plan->GetRecipientClosingNum());

                                    // For recipient, I also remove the opening
                                    // and closing numbers as
                                    // AVAILABLE FOR USE. But they aren't
                                    // removed as ISSUED until later...
                                    // RemoveIssuedNum is called for the
                                    // Recipient's opening number
                                    // onFinalReceipt, and it's called for the
                                    // Recipient's closing number
                                    // when that final receipt is closed out.
                                    context.ConsumeAvailable(
                                        plan->GetRecipientOpeningNum());
                                    context.ConsumeAvailable(
                                        plan->GetRecipientClosingNum());

                                    // Send success notice to other parties. (So
                                    // they can deal with their payments
                                    // inbox and outpayments box, where pending
                                    // copies of the instrument may still
                                    // be waiting.)
                                    //
                                    std::int64_t lOtherNewTransNumber = 0;
                                    server_.GetTransactor()
                                        .issueNextTransactionNumber(
                                            lOtherNewTransNumber);

                                    if (false == plan->SendNoticeToAllParties(
                                                     api_,
                                                     true,  // bSuccessMsg=true
                                                     server_.GetServerNym(),
                                                     NOTARY_ID,
                                                     lOtherNewTransNumber,
                                                     // Each party has its own
                                                     // opening number. Handled
                                                     // internally.
                                                     strPaymentPlan,
                                                     reason_,
                                                     strPaymentPlan,
                                                     String::Factory())) {
                                        LogError()(OT_PRETTY_CLASS())(
                                            "Failed notifying parties while "
                                            "trying to activate payment "
                                            "plan: ")(plan->GetOpeningNum())
                                            .Flush();
                                    }
                                } else {
                                    if (bCancelling) {
                                        tranOut.SetAsCancelled();
                                        LogError()(OT_PRETTY_CLASS())(
                                            "Canceling a payment plan before "
                                            "it was ever activated. (At user's "
                                            "request.)")
                                            .Flush();
                                    } else {
                                        LogError()(OT_PRETTY_CLASS())(
                                            "Unable to add payment plan to "
                                            "Cron. (Failed activating payment "
                                            "plan.)")
                                            .Flush();
                                    }

                                    // Send a failure notice to the other
                                    // parties.
                                    //
                                    // DROP REJECTION NOTICE HERE TO ALL
                                    // PARTIES....
                                    // SO THEY CAN CLAW BACK THEIR TRANSACTION
                                    // #s....
                                    //
                                    std::int64_t lOtherNewTransNumber = 0;
                                    server_.GetTransactor()
                                        .issueNextTransactionNumber(
                                            lOtherNewTransNumber);

                                    if (false == plan->SendNoticeToAllParties(
                                                     api_,
                                                     false,
                                                     server_.GetServerNym(),
                                                     NOTARY_ID,
                                                     lOtherNewTransNumber,
                                                     // Each party has its own
                                                     // opening number. Handled
                                                     // internally.
                                                     strPaymentPlan,
                                                     reason_,
                                                     strPaymentPlan,
                                                     String::Factory())) {
                                        // NOTE: A party may deliberately try to
                                        // activate a payment plan without
                                        // signing it. (As a way of rejecting
                                        // it.) This will cause rejection
                                        // notices to go to all the other
                                        // parties, allowing them to harvest
                                        // back
                                        // their closing numbers. Since that is
                                        // expected to happen, that means
                                        // if you have 2 parties, and the 2nd
                                        // one "activates" it (without
                                        // signing), then this piece of code
                                        // here will DEFINITELY fail to send
                                        // the rejection notice to the first
                                        // party (since the 2nd one hadn't
                                        // even signed the thing yet.)
                                        //
                                        // (Since we expect that to normally
                                        // happen, we don't log an error here.)

                                        //                                      OTLog::vOutput(0,
                                        // "%s: Failed notifying all parties
                                        // about failed activation of payment
                                        // plan: %ld.\n", ,
                                        // plan->GetTransactionNum());
                                    }
                                }  // Failure adding Cron Item.
                            }
                        }
                    }  // If recipient Nym successfully loaded from storage.
                }  // If Payment Plan successfully loaded from Transaction Item.
            }      // else
        }
    }

    std::unique_ptr<Ledger> pInbox(
        theDepositorAccount.get().LoadInbox(server_.GetServerNym()));
    std::unique_ptr<Ledger> pOutbox(
        theDepositorAccount.get().LoadOutbox(server_.GetServerNym()));

    theDepositorAccount.get().GetIdentifier(accountHash);
    AddHashesToTransaction(tranOut, *pInbox, *pOutbox, accountHash);

    // sign the response item before sending it back (it's already been added to
    // the transaction above)
    // Now, whether it was rejection or acknowledgement, it is set properly and
    // it is signed, and it
    // is owned by the transaction, who will take it from here.
    pResponseItem->SignContract(server_.GetServerNym(), reason_);
    pResponseItem->SaveContract();  // the signing was of no effect because I
                                    // forgot to save.

    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
}

void Notary::NotarizeSmartContract(
    otx::context::Client& context,
    ExclusiveAccount& theActivatingAccount,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    bool& bOutSuccess)
{
    // The outgoing transaction is an "atSmartContract", that is, "a reply to
    // the smartContract request"
    tranOut.SetType(transactionType::atSmartContract);

    std::shared_ptr<Item> pItem = nullptr;
    std::shared_ptr<Item> pBalanceItem = nullptr;
    std::shared_ptr<Item> pResponseItem = nullptr;
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will definitely be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();

    // Grab the actual server ID from this object, and use it as the server ID
    // here.
    const auto& NOTARY_ID = context.Notary();
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto& NOTARY_NYM_ID = context.Signer()->ID();
    const auto& ACTIVATOR_NYM_ID = NYM_ID;
    const auto ACTIVATOR_ACCT_ID = server_.API().Factory().Internal().AccountID(
        theActivatingAccount.get());
    const auto strNymID = String::Factory(NYM_ID, api_.Crypto());
    pItem = tranIn.GetItem(itemType::smartContract);
    pBalanceItem = tranIn.GetItem(itemType::transactionStatement);
    pResponseItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atSmartContract, identifier::Account{})
            .release());
    pResponseItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseItem);  // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.
    pResponseBalanceItem.reset(api_.Factory()
                                   .InternalSession()
                                   .Item(
                                       tranOut,
                                       itemType::atTransactionStatement,
                                       identifier::Account{})
                                   .release());
    pResponseBalanceItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseBalanceItem);  // the Transaction's destructor
                                            // will cleanup the item. It "owns"
                                            // it now.

    auto accountHash = identifier::Account{};

    if ((nullptr != pItem) &&
        (false ==
         NYM_IS_ALLOWED(
             strNymID->Get(), ServerSettings::_transact_smart_contract))) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (All smart "
            "contracts are disallowed "
            "in server.cfg)")
            .Flush();
    }
    // For now, there should only be one of these smartContract items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    else if ((nullptr == pItem) || (nullptr == pBalanceItem)) {
        LogError()(OT_PRETTY_CLASS())("Error, expected Item::smartContract and "
                                      "Item::transactionStatement.")
            .Flush();
    } else {
        if (ACTIVATOR_ACCT_ID != pItem->GetPurportedAccountID()) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Source account ID on the transaction does not match "
                "activator's account ID on the transaction item.")
                .Flush();
        } else if (!pBalanceItem->VerifyTransactionStatement(context, tranIn)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed verifying transaction statement.")
                .Flush();
        } else {
            pResponseBalanceItem->SetStatus(
                Item::acknowledgement);  // the transaction agreement was
                                         // successful.

            // The response item will contain a copy of the request item. So I
            // save it into a string
            // here so it can be saved into the "in reference to" field.
            pItem->SaveContractRaw(strInReferenceTo);
            pBalanceItem->SaveContractRaw(strBalanceItem);

            // Server response item being added to server response transaction
            // (tranOut)
            // They're getting SOME sort of response item.

            pResponseItem->SetReferenceString(
                strInReferenceTo);  // the response item carries a copy of what
                                    // it's responding to.
            pResponseItem->SetReferenceToNum(
                pItem->GetTransactionNum());  // This response item is IN
                                              // RESPONSE to pItem and its Owner
                                              // Transaction.

            pResponseBalanceItem->SetReferenceString(
                strBalanceItem);  // the response item carries a copy of what
                                  // it's responding to.
            pResponseBalanceItem->SetReferenceToNum(
                pItem->GetTransactionNum());  // This response item is IN
                                              // RESPONSE to pItem and its Owner
                                              // Transaction.

            // Also load up the smart contract from inside the transaction item.
            auto strContract = String::Factory();
            pItem->GetAttachment(strContract);
            auto pContract{
                api_.Factory().InternalSession().SmartContract(NOTARY_ID)};
            OT_ASSERT(false != bool(pContract));

            // If we failed to load the smart contract...
            if ((false == pContract->LoadContractFromString(strContract))) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR loading smart contract from string: ")(
                    strContract.get())
                    .Flush();
            } else if (pContract->GetNotaryID() != NOTARY_ID) {
                const auto strWrongID =
                    String::Factory(pContract->GetNotaryID(), api_.Crypto());
                LogError()(OT_PRETTY_CLASS())("ERROR bad server ID "
                                              "(")(strWrongID.get())(
                    ") on smart "
                    "contract. "
                    "Expected:"
                    " ")(server_.GetServerID(), api_.Crypto())
                    .Flush();
            } else {
                // CANCELING, or ACTIVATING?
                //
                auto theCancelerNymID = identifier::Nym{};
                const bool bCancelling =
                    (pContract->IsCanceled() &&
                     pContract->GetCancelerID(theCancelerNymID));
                const std::int64_t lFoundNum = pContract->GetTransactionNum();
                const std::int64_t lExpectedNum = pItem->GetTransactionNum();
                std::int64_t lFoundOpeningNum = 0;
                std::int64_t lFoundClosingNum = 0;

                auto FOUND_NYM_ID = identifier::Nym{};
                auto FOUND_ACCT_ID = identifier::Generic{};

                if (!bCancelling)  // ACTIVATING
                {
                    LogError()(OT_PRETTY_CLASS())(
                        "Attempting to activate smart contract...")
                        .Flush();
                    lFoundOpeningNum = pContract->GetOpeningNum();
                    lFoundClosingNum = pContract->GetClosingNum();

                    FOUND_NYM_ID = pContract->GetSenderNymID();
                    FOUND_ACCT_ID = pContract->GetSenderAcctID();
                } else  // CANCELING
                {
                    LogError()(OT_PRETTY_CLASS())(
                        "Attempting to cancel smart contract...")
                        .Flush();
                    lFoundOpeningNum = pContract->GetOpeningNumber(
                        theCancelerNymID);  // See if there's an opening
                                            // number for the canceling
                                            // Nym.
                    lFoundClosingNum = pContract->GetClosingNumber(
                        ACTIVATOR_ACCT_ID);  // See if there's a closing
                                             // number for the current
                                             // account.

                    if (lFoundOpeningNum > 0) {
                        FOUND_NYM_ID = theCancelerNymID;
                    }
                    if (lFoundClosingNum > 0) {
                        FOUND_ACCT_ID = ACTIVATOR_ACCT_ID;
                    }
                }

                if (lFoundNum != lExpectedNum) {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR bad main opening transaction "
                        "number ")(lFoundNum)("on smart contract. "
                                              "Found: ")(lExpectedNum)(" Expect"
                                                                       "ed:"
                                                                       " ")(
                        lExpectedNum)("FYI, pItem->GetTransactionNum() is ")(
                        pItem->GetTransactionNum())
                        .Flush();
                } else if (lFoundOpeningNum != lExpectedNum) {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR bad opening transaction number on smart "
                        "contract. Found: ")(lFoundOpeningNum)("  "
                                                               "Expected:"
                                                               " ")(
                        lExpectedNum)
                        .Flush();
                } else if (FOUND_NYM_ID != ACTIVATOR_NYM_ID) {
                    const auto strWrongID =
                        String::Factory(ACTIVATOR_NYM_ID, api_.Crypto());
                    const auto strRightID =
                        String::Factory(FOUND_NYM_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())("ERROR wrong user ID (")(
                        strWrongID.get())(
                        ") used "
                        "while"
                        " ")(bCancelling ? "canceling" : "activating")(
                        "smart contract. Expected from contract: ")(
                        strRightID.get())
                        .Flush();
                } else if (FOUND_ACCT_ID != ACTIVATOR_ACCT_ID) {
                    const auto strSenderAcctID = String::Factory(
                                   FOUND_ACCT_ID, api_.Crypto()),
                               strActivatorAcctID = String::Factory(
                                   ACTIVATOR_ACCT_ID, api_.Crypto());
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR wrong asset Acct ID used "
                        "(")(strActivatorAcctID.get())(") to ")(
                        bCancelling ? "cancel" : "activate")(
                        "smart contract. Expected from contract: ")(
                        strSenderAcctID.get())
                        .Flush();
                }
                // The transaction number opens the smart contract, but
                // there must also be a closing number for closing it.
                else if (
                    (pContract->GetCountClosingNumbers() <
                     1) ||  // the transaction number was verified
                            // before we entered this function, so only
                            // the closing # is left...
                    !context.VerifyIssuedNumber(lFoundClosingNum))
                // Verify that it can still be USED (not closed...)
                {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR: the Closing "
                        "number ")(lFoundClosingNum)(" wasn't "
                                                     "available "
                                                     "for use "
                                                     "while"
                                                     " ")(
                        bCancelling ? "canceling"
                                    : "activating")(" a smart contract.")
                        .Flush();
                }
                // NOTE: since theNym has ALREADY been substituted for
                // the Server's Nym by this point, if indeed they are
                // the same Nym, then I could probably just ALLOW the
                // server to be a party to a smart contract. It will
                // definitely be on the "list of nyms that are already
                // loaded" due to the substitution. So really it's just
                // a matter of security review, and the below block
                // could be commented out (or not.)  ALSO: If I'm going
                // to enforce this, then I need to do it for ALL
                // parties, not just the activator!
                else if (
                    (pContract->GetSenderNymID() == NOTARY_NYM_ID) ||
                    (nullptr != pContract->FindPartyBasedOnNymAsAgent(
                                    server_.GetServerNym()))) {
                    LogError()(OT_PRETTY_CLASS())(
                        "** SORRY ** but the server itself is NOT ALLOWED to "
                        "be a party to any smart contracts. (Pending security "
                        "review.)")
                        .Flush();
                }
                //
                // VERIFY SMART CONTRACT
                /*
                  -- Loop through all parties and load up the
                  authorizing agent's Nym, if not already loaded, for
                  each.
                  -- Verify each party, that the authorizing agent is
                  good, and verify his signature on the party's copy of
                  the contract.
                  -- Definitely during this, need to make sure that the
                  contents of the signed version match the contents of
                  the main version, for each signer.
                  -- Verify that the authorizing agent actually has the
                  opening transaction # for the party issued to him.
                  -- EVEN IF VERIFICATION FAILS HALFWAY THOUGH, REMOVE
                  that opening transaction # for each-and-every agent.
                  (So he can't use it twice--leaving it as issued, but
                  no longer as "available to be used on another
                  transaction".) Otherwise, if verification failed
                  halfway through, with half of the parties having their
                  opening numbers already burned, and the other half
                  not, then it would be impossible to tell, based on the
                  failed message itself, which group YOU are in, and
                  therefore whether YOU need to harvest that number
                  back or not (in order to avoid going out-of-sync.)
                  THEREFORE WE BURN ALL OPENING NUMBERS so the client
                  API can just assume the opening number is burned, if
                  the transaction ran at all. (And, as normal, if the
                  transaction did NOT run at all, e.g. if the message
                  failed before the transaction had a chance to run,
                  then all opening numbers are still good, for all
                  parties--including the activator.)

                  -- NOTE: this means, if it succeeds, the opening
                  numbers are marked as IN USE (RemoveTransactionNum but
                  NOT RemoveIssuedNum.) But if it FAILS, then we also
                  need to RemoveIssuedNum... So I'm adding that to
                  VerifySmartContract.

                  -- Next, loop through all the asset accounts...
                  -- For each, get a pointer to the authorized agent and
                  verify the CLOSING number for that asset acct. (AND
                  mark that number as "used but still issued.") Again,
                  do this for ALL asset accounts on the smart contract,
                  even if some of them fail the verification process.
                  (It's also okay to skip the accounts for
                  parties who failed verification.) If anything fails,
                  then at the very end, add the closing numbers back
                  again as "available for use" on those nyms.

                  -- Since we're looping through all the agents, and
                  looping through all the asset accounts, and checking
                  the agent for each asset account, then we might as
                  well make sure that each agent is
                  a legit agent for the party, and that each account has
                  a legit agent lording over it.
                */
                else if (
                    bCancelling &&
                    !pContract->VerifySignature(context.RemoteNym())) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Failed verifying canceler signature while canceling "
                        "smart contract.")
                        .Flush();
                }

                // We let it run through the verifier here, even if we
                // are cancelling. The reason is because this is where
                // the various opening/closing numbers are
                // burned/reserved/etc. So even cancellation needs this
                // part done.
                //
                else if (!pContract->VerifySmartContract(
                             context.RemoteNym(),
                             theActivatingAccount.get(),
                             server_.GetServerNym(),
                             reason_,
                             true))  // bBurnTransNo=false by default,
                                     // but here we pass TRUE.
                {
                    if (bCancelling) {
                        tranOut.SetAsCancelled();
                        LogError()(OT_PRETTY_CLASS())(
                            "Canceling a smart contract before it was ever "
                            "even activated (at user's request.)")
                            .Flush();
                    } else {
                        LogError()(OT_PRETTY_CLASS())(
                            "This smart contract has FAILED to verify.")
                            .Flush();
                    }

                    /*

                      ------ TODO: Smart Contracts -----------

                      Done:  Whenever a party confirms a smart contract
                      (sending it on to the next party) then a copy of
                      the smart contract should go into that party's
                      paymentOutbox. Same thing if the party is the last
                      one in the chain, and has activated it on to the
                      server. A copy sits in the paymentOutbox until
                      that smart contract is either successfully
                      activated, or FAILS to activate.

                      If a smart contract activates,
                      OTAgreement::DropServerNoticeToNymbox already
                      sends an 'acknowledgment' notice to all parties.

                      Done: If a smart contract fails to activate, it
                      should ALSO send a notice ('rejection') to all
                      parties.

                      TODO: When a party receives a rejection notice in
                      his Nymbox for a certain smart contract, he looks
                      up that same smart contract in his paymentOutbox,
                      HARVESTS THE CLOSING NUMBERS, and
                      then moves the notice from his outpayments box to
                      his recordBox. NOTE: the notice might be in his
                      payments inbox (sometimes) instead of his
                      outpayments box. Possibly even both. How so? See
                      below. Point being: Need to check both, at this
                      point.

                      Until this is added, then clients will go out of
                      sync on rejected smart contracts. (Not the kind of
                      out-of-sync where they can't do any transactions,
                      but rather, the kind where they have certain
                      numbers signed out forever but then never use them
                      on anything because their client thinks those
                      numbers were already used on a smart contract
                      somewhere, and without the above code they would
                      never have clawed back those numbers.)

                      MORE DETAILS:

                      *** When I send a smart contract on to the next
                      party, remember it's sitting in my payments inbox
                      at first. When I confirm it, a copy goes into my
                      outpayments box. Then when I actually SEND it, a
                      copy goes into my outpayments box AGAIN. (This is
                      already smart enough to remove this first copy,
                      when this happens.) If I activate it (rather than
                      sending it on, perhaps I'm the last one) then it's
                      already in my outpayments box from the
                      confirmation.

                      BUT WHEN DO I REMOVE IT FROM THE payments *INBOX*
                      ? Answer: when the successful server reply is
                      received from the sendNymInstrument. What if I
                      don't send it to another user? Perhaps I activate
                      it. In that case, whether the activation succeeds
                      or fails, I will get an acknowledgment (or
                      rejection) notice in my Nymbox. Therefore I can
                      harvest the numbers back when that notice is
                      received (or not.) That will be from my
                      outpayments box. But removing it from my INBOX
                      should happen when I get the server response to
                      the activation (just as when I get the server
                      response to sendNymInstrument.)
                      If I never tried to activate it, and never tried
                      to send it to the next party, and never discarded
                      it, then it should remain in my inbox, until I
                      choose to do one of those things.


                      *** The sent contract remains in the outPayments
                      box until: A. Activated. When: When the
                      acknowledgment of activation is received through
                      the Nymbox. B. Failed activation. When: Rejection
                      of activation received through the Nymbox. C.
                      Expiration. Expired notices may be harvested from
                      the outpayments box. After all, they were
                      apparently never activated or even attempted,
                      since either of those actions
                      would have resulted in a rejection notice which
                      would have already removed the outpayments box. So
                      the transaction numbers can be harvested. BUT make
                      sure you have latest version of
                      files first, so you know for sure that the
                      contract never really was activated or attempted.

                      *** What if the incoming smart contract is
                      discarded, instead of confirmed? This means it
                      never goes into my outbox in the first place. It's
                      in my inbox, then I discard it. Then what? In one
                      scenario, the user simply throws it away. He
                      removes it from the box and never notifies anyone.
                      This is physically possible so we must consider
                      it. In that case, it's still sitting in other
                      people's outboxes, and will eventually expire, and
                      then those people will just harvest back their
                      transaction numbers. It's kind of rude not to
                      notify them, but everything will still be OKAY.
                      They also still have the power, since it hasn't
                      been activated, to "false activate" it, which will
                      fail since it's not fully-confirmed yet, and then
                      the rejection notice will come through and remove
                      it from their outboxes. All parties are notified
                      in that case. The polite thing to do, instead of
                      just discarding it, would be for me to do the same
                      (false-activate it) meaning I activate it, but
                      without signing it. And possibly putting some
                      other "This must fail" indicator on the message,
                      so the server doesn't waste a lot of time figuring
                      that out. Then the failure causes all the parties
                      who DID sign it, to get a rejection notification,
                      and I can remove it from my payments inbox at that
                      time, when they are all removing it from their
                      outboxes.

                      *** What if the incoming contract is discarded
                      AFTER it was confirmed? From the outbox, meaning
                      it hasn't been activated yet. Perhaps I sent
                      someone a cheque that hasn't been cashed yet.
                      Perhaps I sent someone a signed smart contract but
                      they haven't activated it yet. Therefore I still
                      have a chance to cancel it. I can't just discard
                      it, since they can still deposit their copy
                      whenever they want. But if I RUN IT THROUGH, then
                      it will be INVALIDATED thereafter -- and if I beat
                      them to the punch, then it will work. Of course,
                      if they activate it, then I will get an activation
                      notice, which will automatically remove it from my
                      outbox. So I beat them to the punch, by activating
                      / depositing it myself, which fails, and then we
                      both get rejection notices. That removes it from
                      my outbox, as well as the inbox of the guy who I
                      had been stuck waiting on in the first place.


                      WHY WAS IT in whichever box it was in? (Just
                      curious.) Well... If inbox, because I discarded it
                      without confirming, yet wanted to be nice and let
                      people who had, harvest their numbers back.
                      (Otherwise they'd still eventually expire.) If
                      outpayments box, because I activated it (so it's
                      in that box) and it's just legitimately a failed
                      attempt on my part, or because I confirmed it and
                      sent to the next guy, and he hasn't activated it
                      yet, and I've changed my mind and wish to cancel
                      it. Either way, once I do, I will get the notice
                      (as will any other parties) and then it will be
                      removed from that box (and placed in the records
                      box.) Another scenario: It's removed from my inbox
                      when some other confirming party "false activates"
                      it in order to cancel it and remove it from his
                      outbox. He HAD been sitting there waiting on me,
                      while the notice sat in my inbox. But now that
                      it's been invalidated at the server, I will get a
                      rejection notice from the server which should
                      remove the one that was sitting in my inbox, to
                      the record box.


                      ACTIONS:

                      -- When successful "sendNymInstrument" server
                      reply is received, remove that instrument from
                      payments inbox. (If it's there
                      -- it can be.)

                      -- When party receives notice that smart contract
                      has been activated, remove the instrument from
                      outpayments box. (If it's there
                      -- it can be.)

                      -- When party receives notice that smart contract
                      has failed activation attempt, then remove the
                      instrument from payments inbox AND outpayments
                      box. (If there -- could be for either.)

                      Does this cover all cases?

                      -- Any _sent_ instrument will properly be removed
                      from the payments inbox.
                      -- It will go into the outpayments box. Once it
                      activates, it will be removed again from that box.
                      (For all parties.)
                      -- If it fails to activate, or if a party discards
                      it from inbox (through a deliberate failed
                      activation) or if a party discards it from the
                      outbox (through a deliberate failed activation)
                      either way, it will be removed from both boxes.
                      -- If it expires while sitting in my inbox, my
                      high-level API is responsible to remove it and
                      harvest the numbers.
                      -- It if expires while sitting in my outbox, my
                      high-level API is responsible to remove it and
                      harvest the numbers.

                      It can be sent, discarded (from outbox, as a
                      scramble-to-discard-it-before-next-guy-deposits
                      it), discarded (from inbox, when I decide I won't
                      sign it), it can be ignored until expiration
                      (either box), and it can legitimately activate or
                      fail to activate, and either way, all the parties
                      who confirmed it will get a notice and harvest (if
                      necessary.)

                      THIS SEEMS TO COVER ALL CASES!

                      One more thing, just noticed: Whether success or
                      fail, the opening AND closing numbers are marked
                      as "used but not closed" on the Nym's record. We
                      do this above for all Nyms just doing
                      verification, since we can't fail halfway through
                      and have
                      inconsistent results between them. (All or
                      nothing.)

                      THERFORE: 1. When the Nyms receive "SUCCESS"
                      activating the smart contract, they ALL know that
                      the opening AND closing numbers are marked off as
                      used, and can only be closed thereafter through
                      the final receipt. 2. When the Nyms receive FAILED
                      activating the smart contract... Do they harvest
                      the numbers back? NOT if they were all marked off
                      already! So next, if failure here I need to mark
                      them all as closed, right? Since we failed? And
                      then the client side, when he gets the notice, he
                      needs to mark them as closed as well. (NOT harvest
                      them.) We could alternately mark them all as
                      "still
                      available" on the server side (or all closing
                      numbers anyway) and mark all the opening numbers
                      as closed. But whatever we do, the client side
                      needs to do the same thing. The only time we
                      harvest ALL the numbers is then when we haven't
                      even sent it to the server yet? Otherwise we mark
                      them
                      as "used and not closed" (if success) or if
                      failure, we mark them as:  ????? Perhaps all still
                      open except the main opening one? Or perhaps all
                      the closing numbers are still available and the
                      opening numbers are burned? Or just ALL numbers
                      are burned? Which? Why?

                      NOTE: I found the answer in the comments in
                      OTSmartContract::VerifySmartContract. (And there
                      are very good reasons involved for why I went the
                      way that I did. Read it for those reasons.)
                      Conclusion:

                      If there is a failed activation attempt, then all
                      parties get a notice, and all parties can CLOSE
                      the opening number, which was burned, and they can
                      HARVEST the closing numbers, which were made new
                      again.

                      But if the activation attempt succeeded, then all
                      parties get a notice, and all parties will
                      continue as they were: with the opening AND
                      closing numbers marked as "Still issued but in
                      use." Their opening numbers will not close until
                      the smart contract is deactivated, and their
                      closing numbers will not close until their final
                      receipts have been closed. You might ask, "Then
                      why send the notice, if the transaction numbers
                      are already set up correctly on the client side?"
                      The answer is, because the client still does
                      things based on that notice. Like for example, it
                      removes the confirmed copy of that smart contract
                      from its outpayments box.

                    */

                    // DROP REJECTION NOTICE HERE TO ALL PARTIES....
                    // SO THEY CAN CLAW BACK THEIR TRANSACTION #s....
                    //
                    std::int64_t lNewTransactionNumber = 0;
                    server_.GetTransactor().issueNextTransactionNumber(
                        lNewTransactionNumber);

                    if (false == pContract->SendNoticeToAllParties(
                                     false,
                                     server_.GetServerNym(),
                                     NOTARY_ID,
                                     lNewTransactionNumber,
                                     // // Each party has its own
                                     // opening number. Handled
                                     // internally.
                                     strContract,
                                     reason_,
                                     strContract,
                                     String::Factory())) {
                        // NOTE: A party may deliberately try to
                        // activate a smart contract without signing it.
                        // (As a way of rejecting it.) This will cause
                        // rejection notices to go to all the other
                        // parties, allowing them to harvest back their
                        // closing numbers. Since that is expected to
                        // happen, that means if you have 5 parties, and
                        // the 3rd one "activates" the contract, then
                        // this piece of code here will DEFINITELY fail
                        // to send the rejection notice to the last 2
                        // parties (since they hadn't even signed the
                        // contract yet.)
                        //
                        // (Since we expect that to normally happen, we
                        // don't log an error here.)
                    }
                }  // smart contract is no good.

                // The smart contract is good...
                //
                // NOTIFY ALL PARTIES and ACTIVATE.
                //
                // This is important to notify first, because the hooks
                // in OTSmartContract::onActivate() could very
                // potentially trigger MORE receipts, and we want to
                // make sure the activation receipt comes first.
                //
                else {
                    std::int64_t lNewTransactionNumber = 0;
                    server_.GetTransactor().issueNextTransactionNumber(
                        lNewTransactionNumber);

                    std::shared_ptr<OTSmartContract> contract{
                        pContract.release()};
                    if (false == contract->SendNoticeToAllParties(
                                     true,
                                     server_.GetServerNym(),
                                     NOTARY_ID,
                                     lNewTransactionNumber,
                                     // // Each party has its own
                                     // opening number. Handled
                                     // internally.
                                     strContract,
                                     reason_,
                                     strContract,
                                     String::Factory())) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Failed notifying parties while trying to "
                            "activate smart contract: ")(
                            contract->GetTransactionNum())
                            .Flush();
                    }
                    // Add it to Cron...
                    else if (server_.Cron().AddCronItem(
                                 contract, true, Clock::now())) {
                        // We add the smart contract to the server's
                        // Cron object, which does regular processing.
                        // That object will take care of processing the
                        // smart contract according to its terms.
                        //
                        // NOTE: FYI, inside AddCronItem, since this is
                        // a new CronItem, a Cron Receipt will be saved
                        // with the User's signature on it, containing
                        // the Cron Item from the user's
                        // original request. After that, the item is
                        // stored internally to Cron itself, and signed
                        // by the server--and changes over time as cron
                        // processes. (The original receipt
                        // can always be loaded when necessary.)
                        //

                        // Now we can set the response item as an
                        // acknowledgement instead of rejection (the
                        // default)
                        pResponseItem->SetStatus(Item::acknowledgement);
                        bOutSuccess = true;  // The smart contract
                                             // activation was successful.
                        LogError()(OT_PRETTY_CLASS())(
                            "Successfully added smart contract to Cron "
                            "object.")
                            .Flush();
                    }  // If smart contract verified.
                    else {
                        LogError()(OT_PRETTY_CLASS())(
                            "Unable to add smart contract to Cron object.")
                            .Flush();
                    }
                }  // contract verifies, activate it.
            }      // else
                   // If the smart contract WAS successfully added to Cron,
            // then we don't need to delete it here, since Cron owns it
            // now, and will deal with cleaning it up at the right time.
            //            if ((nullptr != pContract) &&
            //                (pResponseItem->GetStatus() !=
            //                Item::acknowledgement)) { delete
            //                pContract; pContract = nullptr;
            //            }
        }
    }

    std::unique_ptr<Ledger> pInbox(
        theActivatingAccount.get().LoadInbox(server_.GetServerNym()));
    std::unique_ptr<Ledger> pOutbox(
        theActivatingAccount.get().LoadOutbox(server_.GetServerNym()));

    theActivatingAccount.get().GetIdentifier(accountHash);
    AddHashesToTransaction(tranOut, *pInbox, *pOutbox, accountHash);

    // sign the response item before sending it back (it's already been
    // added to the transaction above) Now, whether it was rejection or
    // acknowledgement, it is set properly and it is signed, and it is owned
    // by the transaction, who will take it from here.
    pResponseItem->SignContract(server_.GetServerNym(), reason_);
    pResponseItem->SaveContract();  // the signing was of no effect because
                                    // I forgot to save.

    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
}

// DONE: The code inside here is just a copy of payment plan.
// Make it into a REAL notarizeCancelCronItem so it actually works.
//
// Cancel a market offer.
// (DONE:  NEED TO CHANGE THIS INTO A TRANSACTION, INSTEAD OF A MESSAGE...)
// Will become "Cancel Cron Item"
//
// DONE: This needs to be "CANCEL CRON ITEM" and it should make use of
// CLOSING NUMBERS that should SHOULD ALREADY be available in the CRON
// ITEMS!
//
// Basically it allows you to cancel payment plans OR market offers, and
// places the appropriate cancellation receipts (preferably through
// polymorphism, versus some huge 'if' block here...
//
// When cancelling it uses the closing numbers provided in the cron items.
// Then code the expiration part in OTCron Item or wherever, which should
// use the SAME closing numbers.
//
void Notary::NotarizeCancelCronItem(
    otx::context::Client& context,
    ExclusiveAccount& theAssetAccount,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    bool& bOutSuccess)
{
    // The outgoing transaction is an "atCancelCronItem", that is, "a reply
    // to the cancelCronItem request"
    tranOut.SetType(transactionType::atCancelCronItem);

    std::shared_ptr<Item> pItem = nullptr;
    std::shared_ptr<Item> pBalanceItem = nullptr;
    std::shared_ptr<Item> pResponseItem = nullptr;
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will definitely be bundled in our reply to the user as well.
    // Therefore, let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();

    // Grab the actual server ID from this object, and use it as the server
    // ID here.
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto strNymID = String::Factory(NYM_ID, api_.Crypto());
    pBalanceItem = tranIn.GetItem(itemType::transactionStatement);
    pResponseItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atCancelCronItem, identifier::Account{})
            .release());
    pResponseItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseItem);  // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem.reset(api_.Factory()
                                   .InternalSession()
                                   .Item(
                                       tranOut,
                                       itemType::atTransactionStatement,
                                       identifier::Account{})
                                   .release());
    pResponseBalanceItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseBalanceItem);  // the Transaction's destructor
                                            // will cleanup the item. It
                                            // "owns" it now.
    auto accountHash = identifier::Account{};

    if (!NYM_IS_ALLOWED(
            strNymID->Get(), ServerSettings::_transact_cancel_cron_item)) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (CancelCronItem messages are "
            "disallowed in server.cfg)")
            .Flush();
    } else if (nullptr == pBalanceItem) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())(
            "Expected transaction statement in trans #")(
            tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    }
    // For now, there should only be one of these cancelCronItem items
    // inside the transaction. So we treat it that way... I either get it
    // successfully or not.
    else if (nullptr != (pItem = tranIn.GetItem(itemType::cancelCronItem))) {
        // The response item will contain a copy of the request item. So I
        // save it into a string here so it can be saved into the "in
        // reference to" field.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // ASSET_ACCT_ID is the ID on the "from" Account that was passed in.
        //
        const auto ASSET_ACCT_ID =
            server_.API().Factory().Internal().Identifier(
                theAssetAccount.get());

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.

        pResponseItem->SetReferenceString(strInReferenceTo);  // the response
                                                              // item carries a
                                                              // copy of what
                                                              // it's responding
                                                              // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN
                                          // RESPONSE to pItem and its Owner
                                          // Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem);  // the response item carries a copy of what
                              // it's responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN
                                          // RESPONSE to pItem and its Owner
                                          // Transaction.

        if (!(pBalanceItem->VerifyTransactionStatement(context, tranIn))) {
            LogError()(OT_PRETTY_CLASS())(
                "ERROR verifying transaction statement in "
                "NotarizeCancelCronItem.")
                .Flush();
        } else {
            pResponseBalanceItem->SetStatus(
                Item::acknowledgement);  // the transaction agreement was
                                         // successful.

            const std::int64_t lReferenceToNum = pItem->GetReferenceToNum();

            // I'm using the operator== because it exists. (Although now I
            // believe != exists also)
            // If the ID on the "from" account that was passed in,
            // does not match the "Acct From" ID on this transaction item
            if (!(ASSET_ACCT_ID == pItem->GetPurportedAccountID())) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Error: Asset account ID on the transaction "
                    "does not match asset account "
                    "ID on the transaction item.")
                    .Flush();
            } else  // LET'S SEE IF WE CAN REMOVE IT THEN...
            {
                auto pCronItem =
                    server_.Cron().GetItemByValidOpeningNum(lReferenceToNum);

                // Check for the closing number here (that happens in
                // OTCronItem, since it's polymorphic.)

                bool bSuccess = false;

                if (false != bool(pCronItem) &&
                    // see if theNym has right to remove the cronItem from
                    // processing.
                    (pCronItem->CanRemoveItemFromCron(context))) {
                    bSuccess = server_.Cron().RemoveCronItem(
                        pCronItem->GetTransactionNum(),
                        api_.Wallet().Nym(context.RemoteNym().ID()),
                        reason_);
                }

                // If we were just successful in removing the offer from the
                // market, that means a finalReceipt was
                // just dropped into the inboxes for the relevant asset
                // accounts. Once I process that receipt out of my
                // inbox, (which will require my processing out all related
                // marketReceipts) then the closing number
                // will be removed from my list of responsibility.

                if (bSuccess) {
                    // Now we can set the response item as an
                    // acknowledgement instead of the default (rejection)
                    pResponseItem->SetStatus(Item::acknowledgement);

                    bOutSuccess =
                        true;  // The "cancel cron item" was successful.
                    LogDetail()(OT_PRETTY_CLASS())(
                        "Successfully removed Cron Item from Cron object, "
                        "based on ID: ")(
                        (pCronItem) ? pCronItem->GetTransactionNum()
                                    : lReferenceToNum)
                        .Flush();
                    // Any transaction numbers that need to be cleared,
                    // happens inside RemoveCronItem().
                } else {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "Unable to remove Cron Item from Cron object.")
                        .Flush();
                }
            }
        }  // transaction statement verified.
    } else {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())(
            "Error, expected Item::cancelCronItem in "
            "Notary::NotarizeCancelCronItem for trans #")(
            tranIn.GetTransactionNum())(" : ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION FROM STRING)")
            .Flush();
    }

    std::unique_ptr<Ledger> pInbox(
        theAssetAccount.get().LoadInbox(server_.GetServerNym()));
    std::unique_ptr<Ledger> pOutbox(
        theAssetAccount.get().LoadOutbox(server_.GetServerNym()));

    theAssetAccount.get().GetIdentifier(accountHash);
    AddHashesToTransaction(tranOut, *pInbox, *pOutbox, accountHash);

    // sign the response item before sending it back (it's already been
    // added to the transaction above) Now, whether it was rejection or
    // acknowledgement, it is set properly and it is signed, and it is owned
    // by the transaction, who will take it from here.
    pResponseItem->SignContract(server_.GetServerNym(), reason_);
    pResponseItem->SaveContract();  // the signing was of no effect because
                                    // I forgot to save.

    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
}

/// a user is exchanging in or out of a basket.  (Ex. He's trading 2 gold
/// and 3 silver for 10 baskets, or vice-versa.)
void Notary::NotarizeExchangeBasket(
    otx::context::Client& context,
    ExclusiveAccount& theAccount,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    Ledger& inbox,
    Ledger& outbox,
    bool& bOutSuccess)
{
    // The outgoing transaction is an "atExchangeBasket", that is, "a reply
    // to the exchange basket request"
    tranOut.SetType(transactionType::atExchangeBasket);

    std::shared_ptr<Item> pItem = tranIn.GetItem(itemType::exchangeBasket);
    std::shared_ptr<Item> pBalanceItem =
        tranIn.GetItem(itemType::balanceStatement);
    std::shared_ptr<Item> pResponseItem = nullptr;
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto& BASKET_CONTRACT_ID =
        theAccount.get().GetInstrumentDefinitionID();
    const auto ACCOUNT_ID =
        server_.API().Factory().Internal().Identifier(theAccount.get());
    const auto strNymID = String::Factory(NYM_ID, api_.Crypto());

    pResponseItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atExchangeBasket, identifier::Account{})
            .release());
    pResponseItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseItem);  // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atBalanceStatement, identifier::Account{})
            .release());
    pResponseBalanceItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseBalanceItem);  // the Transaction's destructor
                                            // will cleanup the item. It
                                            // "owns" it now.
    bool bSuccess = false;
    auto accountHash = identifier::Account{};

    if (!NYM_IS_ALLOWED(
            strNymID->Get(), ServerSettings::_transact_exchange_basket)) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (All basket "
            "exchanges are disallowed "
            "in server.cfg)")
            .Flush();
    } else if (nullptr == pItem) {
        LogConsole()(OT_PRETTY_CLASS())("No exchangeBasket item found on "
                                        "this transaction.")
            .Flush();
    } else if (nullptr == pBalanceItem) {
        LogConsole()(OT_PRETTY_CLASS())(
            "No Balance "
            "Agreement item found on this transaction.")
            .Flush();
    } else {
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        pResponseItem->SetReferenceString(strInReferenceTo);  // the response
                                                              // item carries a
                                                              // copy of what
                                                              // it's responding
                                                              // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN
                                          // RESPONSE to pItem and its Owner
                                          // Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem);  // the response item carries a copy of what
                              // it's responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pBalanceItem->GetTransactionNum());  // This response item is IN
                                                 // RESPONSE to tranIn's
                                                 // balance agreement
        // Now after all that setup, we do the balance agreement!
        if (false == pBalanceItem->VerifyBalanceStatement(
                         0,  // the one balance agreement that doesn't
                             // change any balances.
                         context,  // Could have been a transaction
                                   // agreement.
                         inbox,    // Still could be, in fact....
                         outbox,
                         theAccount.get(),
                         tranIn,
                         UnallocatedSet<TransactionNumber>(),
                         reason_)) {
            LogError()(OT_PRETTY_CLASS())("ERROR verifying balance statement.")
                .Flush();

        } else  // BALANCE AGREEMENT WAS SUCCESSFUL.......
        {
            pResponseBalanceItem->SetStatus(
                Item::acknowledgement);  // the balance agreement was
                                         // successful.
            // Set up some account pointer lists for later...
            listOfAccounts listUserAccounts, listServerAccounts;
            UnallocatedList<Ledger*> listInboxes;

            // Here's the request from the user.
            auto strBasket = String::Factory();
            auto theRequestBasket{api_.Factory().InternalSession().Basket()};

            OT_ASSERT(false != bool(theRequestBasket));

            pItem->GetAttachment(strBasket);
            Amount lTransferAmount = 0;

            // Now we have the Contract ID from the basket account,
            // we can get a pointer to its asset contract...

            auto BASKET_ACCOUNT_ID = identifier::Account{};
            ExclusiveAccount basketAccount{};
            bool bLookup =
                server_.GetTransactor().lookupBasketAccountIDByContractID(
                    BASKET_CONTRACT_ID, BASKET_ACCOUNT_ID);

            if (!bLookup) {
                LogError()(OT_PRETTY_CLASS())(
                    "Notary::NotarizeExchangeBasket: Asset type is not a "
                    "basket currency.")
                    .Flush();
            } else if (
                !strBasket->Exists() ||
                !theRequestBasket->LoadContractFromString(strBasket) ||
                !theRequestBasket->VerifySignature(context.RemoteNym())) {
                LogError()(OT_PRETTY_CLASS())(
                    "Expected verifiable basket object to be attached to "
                    "exchangeBasket item.")
                    .Flush();
            } else if (
                theRequestBasket->GetRequestAccountID() !=
                theAccount.get().GetPurportedAccountID()) {
                LogError()(OT_PRETTY_CLASS())(
                    "User's main account ID according to request basket "
                    "doesn't match theAccount.get().")
                    .Flush();
            } else if (!context.VerifyIssuedNumber(
                           theRequestBasket->GetClosingNum())) {
                LogError()(OT_PRETTY_CLASS())(
                    "Closing number used for User's main account receipt was "
                    "not available for use...")
                    .Flush();
            } else {  // Load the basket account and make sure it exists.
                basketAccount = api_.Wallet().Internal().mutable_Account(
                    BASKET_ACCOUNT_ID, reason_);

                if (false == bool(basketAccount)) {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR loading the basket account")
                        .Flush();
                }
                // Does it verify?
                // I call VerifySignature here since VerifyContractID was
                // already called in LoadExistingAccount().
                else if (!basketAccount.get().VerifySignature(
                             server_.GetServerNym())) {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR verifying signature on the basket account")
                        .Flush();
                } else {
                    try {
                        // Now we get a pointer to its asset contract...
                        const auto basket =
                            api_.Wallet().Internal().BasketContract(
                                BASKET_CONTRACT_ID);
                        // Now let's load up the actual basket, from the actual
                        // asset contract.
                        std::int64_t currencies = basket->Currencies().size();
                        const Amount& weight = basket->Weight();

                        if (currencies == theRequestBasket->Count() &&
                            weight == theRequestBasket->GetMinimumTransfer()) {
                            // Let's make sure that the same asset account
                            // doesn't appear twice on the request.
                            //
                            UnallocatedSet<identifier::Generic> setOfAccounts;
                            setOfAccounts.insert(
                                theRequestBasket->GetRequestAccountID());

                            bool bFoundSameAcctTwice = false;

                            for (std::int32_t i = 0;
                                 i < theRequestBasket->Count();
                                 i++) {
                                BasketItem* item = theRequestBasket->At(i);
                                OT_ASSERT(nullptr != item);
                                auto it_account =
                                    setOfAccounts.find(item->sub_account_id_);

                                if (setOfAccounts.end() !=
                                    it_account)  // The account appears twice!!
                                {
                                    const auto strSubID = String::Factory(
                                        item->sub_account_id_, api_.Crypto());
                                    LogError()(OT_PRETTY_CLASS())(
                                        "Failed: Sub-account ID found TWICE "
                                        "on same basket exchange "
                                        "request: ")(strSubID.get())
                                        .Flush();
                                    bFoundSameAcctTwice = true;
                                    break;
                                }
                                setOfAccounts.insert(item->sub_account_id_);
                            }
                            if (!bFoundSameAcctTwice)  // Let's do it!
                            {
                                // Loop through the request AND the actual
                                // basket TOGETHER...
                                for (std::int32_t i = 0;
                                     i < theRequestBasket->Count();
                                     i++) {

                                    BasketItem* pRequestItem =
                                        theRequestBasket->At(i);
                                    const auto requestContractID =
                                        String::Factory(
                                            pRequestItem->sub_contract_id_,
                                            api_.Crypto());
                                    const auto requestAccountID =
                                        String::Factory(
                                            pRequestItem->sub_account_id_,
                                            api_.Crypto());

                                    if (basket->Currencies().find(
                                            requestContractID->Get()) ==
                                        basket->Currencies().end()) {
                                        LogError()(OT_PRETTY_CLASS())(
                                            "Error: expected instrument "
                                            "definition IDs to match")
                                            .Flush();
                                        bSuccess = false;
                                        break;
                                    }

                                    const auto serverAccountID =
                                        String::Factory(
                                            basket->Currencies()
                                                .at(requestContractID->Get())
                                                .first);
                                    const auto subWeight =
                                        basket->Currencies()
                                            .at(requestContractID->Get())
                                            .second;

                                    if (serverAccountID->Compare(
                                            requestAccountID)) {
                                        LogError()(OT_PRETTY_CLASS())(
                                            "Error: VERY strange to have these "
                                            "account ID's match")
                                            .Flush();
                                        bSuccess = false;
                                        break;
                                    } else if (
                                        !context.VerifyIssuedNumber(
                                            pRequestItem
                                                ->closing_transaction_no_)) {
                                        LogError()(OT_PRETTY_CLASS())(
                                            "Error: Basket sub-currency "
                                            "closing number didn't verify")
                                            .Flush();
                                        bSuccess = false;
                                        break;
                                    } else  // if equal
                                    {
                                        // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
                                        bSuccess = true;

                                        // Load up the two accounts and perform
                                        // the exchange...
                                        auto tempUserAccount =
                                            api_.Wallet()
                                                .Internal()
                                                .mutable_Account(
                                                    pRequestItem
                                                        ->sub_account_id_,
                                                    reason_);

                                        if (false == bool(tempUserAccount)) {
                                            LogError()(OT_PRETTY_CLASS())(
                                                "ERROR loading a user's "
                                                "asset account")
                                                .Flush();
                                            bSuccess = false;
                                            tempUserAccount.Abort();
                                            break;
                                        }

                                        auto tempServerAccount =
                                            api_.Wallet()
                                                .Internal()
                                                .mutable_Account(
                                                    server_.API()
                                                        .Factory()
                                                        .AccountIDFromBase58(
                                                            serverAccountID
                                                                ->Bytes()),
                                                    reason_);

                                        if (false == bool(tempServerAccount)) {
                                            LogError()(OT_PRETTY_CLASS())(
                                                "ERROR loading a basket "
                                                "sub-account")
                                                .Flush();
                                            bSuccess = false;
                                            tempUserAccount.Abort();
                                            tempServerAccount.Abort();
                                            break;
                                        }
                                        // Load up the inbox for the user's sub
                                        // account, so we can drop the receipt.
                                        //
                                        auto pSubInbox =
                                            tempUserAccount.get().LoadInbox(
                                                server_.GetServerNym());

                                        if (false == bool(pSubInbox)) {
                                            LogError()(OT_PRETTY_CLASS())(
                                                "Error loading or verifying "
                                                "sub-inbox.")
                                                .Flush();
                                            bSuccess = false;
                                            tempUserAccount.Abort();
                                            tempServerAccount.Abort();
                                            break;
                                        }

                                        // I'm preserving these points, to be
                                        // deleted at the end.
                                        // They won't be saved until after ALL
                                        // debits/credits were successful.
                                        // Once ALL exchanges are done, THEN it
                                        // loops through and saves / deletes
                                        // all the accounts.
                                        listUserAccounts.emplace_back(
                                            std::move(tempUserAccount));
                                        auto& userAccount =
                                            *listUserAccounts.rbegin();

                                        listServerAccounts.emplace_back(
                                            std::move(tempServerAccount));
                                        auto& serverAccount =
                                            *listServerAccounts.rbegin();
                                        listInboxes.push_back(pSubInbox.get());

                                        // Do they verify?
                                        // I call VerifySignature here since
                                        // VerifyContractID was already called
                                        // in LoadExistingAccount().
                                        if (userAccount.get()
                                                .GetInstrumentDefinitionID() !=
                                            server_.API()
                                                .Factory()
                                                .UnitIDFromBase58(
                                                    requestContractID
                                                        ->Bytes())) {
                                            LogError()(OT_PRETTY_CLASS())(
                                                "ERROR verifying instrument "
                                                "definition on a user's "
                                                "account.")
                                                .Flush();
                                            bSuccess = false;
                                            break;
                                        } else {
                                            // the amount being transferred
                                            // between these two accounts is the
                                            // minimum transfer amount for the
                                            // sub-account on the basket,
                                            // multiplied by
                                            lTransferAmount =
                                                (subWeight *
                                                 theRequestBasket
                                                     ->GetTransferMultiple());

                                            // user is performing exchange IN
                                            if (theRequestBasket
                                                    ->GetExchangingIn()) {
                                                if (userAccount.get().Debit(
                                                        lTransferAmount)) {
                                                    if (serverAccount.get()
                                                            .Credit(
                                                                lTransferAmount)) {
                                                        bSuccess = true;
                                                    } else {  // the server
                                                              // credit failed.
                                                        LogError()(
                                                            OT_PRETTY_CLASS())(
                                                            "Failure "
                                                            "crediting server "
                                                            "acct.")
                                                            .Flush();

                                                        // Since we debited the
                                                        // user's acct already,
                                                        // let's put that back.
                                                        if (false ==
                                                            userAccount.get().Credit(
                                                                lTransferAmount)) {
                                                            LogError()(
                                                                OT_PRETTY_CLASS())(
                                                                "Failure "
                                                                "crediting "
                                                                "back user "
                                                                "account.")
                                                                .Flush();
                                                        }
                                                        bSuccess = false;
                                                        break;
                                                    }
                                                } else {
                                                    LogConsole()(
                                                        OT_PRETTY_CLASS())(
                                                        "Unable to Debit "
                                                        "user account.")
                                                        .Flush();
                                                    bSuccess = false;
                                                    break;
                                                }
                                            } else  // user is peforming
                                                    // exchange OUT
                                            {
                                                if (serverAccount.get().Debit(
                                                        lTransferAmount)) {
                                                    if (userAccount.get().Credit(
                                                            lTransferAmount)) {
                                                        bSuccess = true;
                                                    } else {  // the user credit
                                                              // failed.
                                                        LogError()(
                                                            OT_PRETTY_CLASS())(
                                                            "Failure "
                                                            "crediting user "
                                                            "account.")
                                                            .Flush();

                                                        // Since we debited the
                                                        // server's acct
                                                        // already, let's put
                                                        // that back.
                                                        if (false ==
                                                            serverAccount.get().Credit(
                                                                lTransferAmount)) {
                                                            LogError()(
                                                                OT_PRETTY_CLASS())(
                                                                "Failure "
                                                                "crediting "
                                                                "back server "
                                                                "account.")
                                                                .Flush();
                                                        }
                                                        bSuccess = false;
                                                        break;
                                                    }
                                                } else {
                                                    LogConsole()(
                                                        OT_PRETTY_CLASS())(
                                                        "Unable to Debit "
                                                        "server account.")
                                                        .Flush();
                                                    bSuccess = false;
                                                    break;
                                                }
                                            }
                                            // Drop the receipt -- accounts were
                                            // debited and credited properly.
                                            //
                                            if (bSuccess) {  // need to be able
                                                             // to "roll back"
                                                             // if anything
                                                             // inside this
                                                             // block fails.
                                                // update: actually does pretty
                                                // good roll-back as it is. The
                                                // debits and credits don't save
                                                // unless everything is a
                                                // success.

                                                // Generate new transaction
                                                // number (for putting the
                                                // basketReceipt in the
                                                // exchanger's inbox.) todo
                                                // check this generation for
                                                // failure (can it fail?)
                                                std::int64_t
                                                    lNewTransactionNumber = 0;

                                                server_.GetTransactor()
                                                    .issueNextTransactionNumber(
                                                        lNewTransactionNumber);

                                                auto pInboxTransaction{
                                                    api_.Factory()
                                                        .InternalSession()
                                                        .Transaction(
                                                            *pSubInbox,
                                                            transactionType::
                                                                basketReceipt,
                                                            originType::
                                                                not_applicable,
                                                            lNewTransactionNumber)};

                                                OT_ASSERT(
                                                    false !=
                                                    bool(pInboxTransaction));

                                                auto pItemInbox =
                                                    api_.Factory()
                                                        .InternalSession()
                                                        .Item(
                                                            *pInboxTransaction,
                                                            itemType::
                                                                basketReceipt,
                                                            identifier::
                                                                Account{});

                                                // these may be unnecessary,
                                                // I'll have to check
                                                // CreateItemFromTransaction.
                                                // I'll leave em.
                                                OT_ASSERT(
                                                    false != bool(pItemInbox));

                                                pItemInbox->SetStatus(
                                                    Item::acknowledgement);
                                                pItemInbox->SetAmount(
                                                    theRequestBasket
                                                            ->GetExchangingIn()
                                                        ? lTransferAmount * (-1)
                                                        : lTransferAmount);

                                                pItemInbox->SignContract(
                                                    server_.GetServerNym(),
                                                    reason_);
                                                pItemInbox->SaveContract();

                                                std::shared_ptr<Item> itemInbox{
                                                    pItemInbox.release()};
                                                pInboxTransaction->AddItem(
                                                    itemInbox);  // Add the
                                                                 // inbox item
                                                                 // to the inbox
                                                // transaction, so
                                                // we can add to
                                                // the inbox
                                                // ledger.

                                                pInboxTransaction
                                                    ->SetNumberOfOrigin(*pItem);

                                                // The "exchangeBasket request"
                                                // Item is saved as the "In
                                                // Reference To" field
                                                // on the inbox basketReceipt
                                                // transaction.
                                                // todo put these two together
                                                // in a method.
                                                pInboxTransaction
                                                    ->SetReferenceString(
                                                        strInReferenceTo);
                                                pInboxTransaction
                                                    ->SetReferenceToNum(
                                                        pItem
                                                            ->GetTransactionNum());
                                                // Here is the number the user
                                                // wishes
                                                // to sign-off by accepting this
                                                // receipt.
                                                pInboxTransaction->SetClosingNum(
                                                    pRequestItem
                                                        ->closing_transaction_no_);

                                                // Now we have created a new
                                                // transaction from the server
                                                // to the sender's inbox (for a
                                                // receipt).
                                                // Let's sign and save it...
                                                pInboxTransaction->SignContract(
                                                    server_.GetServerNym(),
                                                    reason_);
                                                pInboxTransaction
                                                    ->SaveContract();

                                                // Here the transaction we just
                                                // created is actually added to
                                                // the exchanger's inbox.
                                                std::shared_ptr<OTTransaction>
                                                    inboxTransaction{
                                                        pInboxTransaction
                                                            .release()};
                                                pSubInbox->AddTransaction(
                                                    inboxTransaction);
                                                inboxTransaction
                                                    ->SaveBoxReceipt(
                                                        *pSubInbox);
                                            }
                                        }  // User and Server sub-accounts are
                                        // good.
                                    }  // pBasketItem and pRequestItem are good.
                                }      // for (loop through basketitems)
                                // Load up the two main accounts and perform the
                                // exchange...
                                // (Above we did the sub-accounts for server and
                                // user. Now we do the main accounts for server
                                // and user.)
                                //

                                // At this point, if we have successfully
                                // debited / credited the sub-accounts. then we
                                // need to debit and credit the user's main
                                // basket account and the server's basket issuer
                                // account.
                                if (bSuccess && basketAccount) {
                                    lTransferAmount =
                                        (theRequestBasket
                                             ->GetMinimumTransfer() *
                                         theRequestBasket
                                             ->GetTransferMultiple());

                                    // Load up the two accounts and perform the
                                    // exchange...
                                    // user is performing exchange IN
                                    if (theRequestBasket->GetExchangingIn()) {
                                        if (basketAccount.get().Debit(
                                                lTransferAmount)) {
                                            if (theAccount.get().Credit(
                                                    lTransferAmount)) {
                                                bSuccess = true;
                                            } else {
                                                LogError()(OT_PRETTY_CLASS())(
                                                    "Failed crediting user "
                                                    "basket account.")
                                                    .Flush();

                                                if (false ==
                                                    basketAccount.get().Credit(
                                                        lTransferAmount)) {
                                                    LogError()(
                                                        OT_PRETTY_CLASS())(
                                                        "Failed crediting "
                                                        "back basket issuer "
                                                        "account.")
                                                        .Flush();
                                                }

                                                bSuccess = false;
                                            }
                                        } else {
                                            bSuccess = false;
                                            LogConsole()(OT_PRETTY_CLASS())(
                                                "Unable to Debit basket "
                                                "issuer account")
                                                .Flush();
                                        }
                                    } else  // user is peforming exchange OUT
                                    {
                                        if (theAccount.get().Debit(
                                                lTransferAmount)) {
                                            if (basketAccount.get().Credit(
                                                    lTransferAmount)) {
                                                bSuccess = true;
                                            } else {
                                                LogError()(OT_PRETTY_CLASS())(
                                                    "Failed crediting basket "
                                                    "issuer account.")
                                                    .Flush();

                                                if (false ==
                                                    theAccount.get().Credit(
                                                        lTransferAmount)) {
                                                    LogError()(
                                                        OT_PRETTY_CLASS())(
                                                        "Failed crediting "
                                                        "back user basket "
                                                        "account.")
                                                        .Flush();
                                                }

                                                bSuccess = false;
                                            }
                                        } else {
                                            bSuccess = false;
                                            LogConsole()(OT_PRETTY_CLASS())(
                                                "Unable to Debit user basket "
                                                "account")
                                                .Flush();
                                        }
                                    }

                                    // Drop the receipt -- accounts were debited
                                    // and credited properly.
                                    //
                                    if (bSuccess) {  // need to be able to "roll
                                        // back" if anything inside this
                                        // block fails.
                                        // update: actually does pretty good
                                        // roll-back as it is. The debits and
                                        // credits
                                        // don't save unless everything is a
                                        // success.

                                        // Generate new transaction number (for
                                        // putting the basketReceipt in the
                                        // exchanger's inbox.)
                                        // todo check this generation for
                                        // failure (can it fail?)
                                        std::int64_t lNewTransactionNumber = 0;

                                        server_.GetTransactor()
                                            .issueNextTransactionNumber(
                                                lNewTransactionNumber);

                                        auto pInboxTransaction{
                                            api_.Factory()
                                                .InternalSession()
                                                .Transaction(
                                                    inbox,
                                                    transactionType::
                                                        basketReceipt,
                                                    originType::not_applicable,
                                                    lNewTransactionNumber)};

                                        OT_ASSERT(
                                            false != bool(pInboxTransaction));

                                        auto pItemInbox =
                                            api_.Factory()
                                                .InternalSession()
                                                .Item(
                                                    *pInboxTransaction,
                                                    itemType::basketReceipt,
                                                    identifier::Account{});

                                        // these may be unnecessary, I'll have
                                        // to check CreateItemFromTransaction.
                                        // I'll leave em.
                                        OT_ASSERT(false != bool(pItemInbox));

                                        pItemInbox->SetStatus(
                                            Item::acknowledgement);  // the
                                                                     // default.
                                        pItemInbox->SetAmount(
                                            theRequestBasket->GetExchangingIn()
                                                ? lTransferAmount
                                                : lTransferAmount * (-1));

                                        pItemInbox->SignContract(
                                            server_.GetServerNym(), reason_);
                                        pItemInbox->SaveContract();

                                        std::shared_ptr<Item> itemInbox{
                                            pItemInbox.release()};
                                        pInboxTransaction->AddItem(
                                            itemInbox);  // Add the inbox item
                                                         // to the inbox
                                                         // transaction, so we
                                                         // can add to the inbox
                                                         // ledger.

                                        pInboxTransaction->SetNumberOfOrigin(
                                            *pItem);

                                        // The exchangeBasket request Item is
                                        // saved as a "in reference to" field,
                                        // on the inbox basketReceipt
                                        // transaction. todo put these two
                                        // together in a method.
                                        pInboxTransaction->SetReferenceString(
                                            strInReferenceTo);
                                        pInboxTransaction->SetReferenceToNum(
                                            pItem->GetTransactionNum());
                                        pInboxTransaction->SetClosingNum(
                                            theRequestBasket
                                                ->GetClosingNum());  // So the
                                                                     // exchanger
                                                                     // can
                                                                     // sign-off
                                                                     // on this
                                                                     // closing
                                                                     // num by
                                                                     // accepting
                                                                     // the
                                        // basket receipt
                                        // on his main
                                        // basket
                                        // account.

                                        // Now we have created a new transaction
                                        // from the server to the sender's inbox
                                        // Let's sign and save it...
                                        pInboxTransaction->SignContract(
                                            server_.GetServerNym(), reason_);
                                        pInboxTransaction->SaveContract();

                                        // Here the transaction we just created
                                        // is actually added to the source
                                        // acct's inbox.
                                        std::shared_ptr<OTTransaction>
                                            inboxTransaction{
                                                pInboxTransaction.release()};
                                        inbox.AddTransaction(inboxTransaction);
                                        inboxTransaction->SaveBoxReceipt(inbox);
                                    }
                                } else {
                                    LogError()(OT_PRETTY_CLASS())(
                                        "Error loading or verifying user's "
                                        "main basket account")
                                        .Flush();
                                    bSuccess = false;
                                }

                                // At this point, we have hopefully
                                // credited/debited ALL the relevant accounts So
                                // now, let's Save them ALL to disk..

                                for (auto& account : listUserAccounts) {
                                    OT_ASSERT(account);

                                    if (bSuccess) {
                                        account.Release();
                                    } else {
                                        account.Abort();
                                    }
                                }

                                for (auto& account : listServerAccounts) {
                                    OT_ASSERT(account);

                                    if (bSuccess) {
                                        account.Release();
                                    } else {
                                        account.Abort();
                                    }
                                }

                                // empty the list of inboxes (and save to disk,
                                // if everything was successful.)
                                while (!listInboxes.empty()) {
                                    Ledger* pTempInbox = listInboxes.front();
                                    if (nullptr == pTempInbox) { OT_FAIL; }
                                    listInboxes.pop_front();

                                    if (true == bSuccess) {
                                        pTempInbox->ReleaseSignatures();
                                        pTempInbox->SignContract(
                                            server_.GetServerNym(), reason_);
                                        pTempInbox->SaveContract();
                                        pTempInbox->SaveInbox();
                                    }

                                    delete pTempInbox;
                                    pTempInbox = nullptr;
                                }
                                if (true == bSuccess) {
                                    inbox.ReleaseSignatures();
                                    inbox.SignContract(
                                        server_.GetServerNym(), reason_);
                                    inbox.SaveContract();
                                    theAccount.get().SaveInbox(inbox);
                                    theAccount.get().GetIdentifier(accountHash);
                                    theAccount.Release();
                                    basketAccount.Release();

                                    // Remove my ability to use the "closing"
                                    // numbers in the future.
                                    // (Since I'm using them to do this
                                    // exchange...)
                                    //
                                    for (std::int32_t i = 0;
                                         i < theRequestBasket->Count();
                                         i++) {
                                        BasketItem* pRequestItem =
                                            theRequestBasket->At(i);

                                        OT_ASSERT(nullptr != pRequestItem);

                                        // This just removes the number so I
                                        // can't USE it. I'm still RESPONSIBLE
                                        // for the number until
                                        // RemoveIssuedNumber() is called.
                                        context.ConsumeAvailable(
                                            pRequestItem
                                                ->closing_transaction_no_);
                                    }

                                    context.ConsumeAvailable(
                                        theRequestBasket->GetClosingNum());
                                    pResponseItem->SetStatus(
                                        Item::
                                            acknowledgement);  // the
                                                               // exchangeBasket
                                                               // was
                                                               // successful.

                                    bOutSuccess = true;  // The exchangeBasket
                                                         // was successful.
                                } else {
                                    theAccount.get().GetIdentifier(accountHash);
                                    theAccount.Abort();
                                    basketAccount.Abort();
                                }
                            }  // Let's do it!
                        } else {
                            LogError()(OT_PRETTY_CLASS())(
                                "Error finding asset contract for basket, or "
                                " loading the basket from it, or verifying  "
                                "the signature on that basket, or the request  "
                                "basket didn't match actual basket.")
                                .Flush();
                        }
                    } catch (...) {
                        // TODO handle error
                    }
                }  // pBasket exists and signature verifies
            }      // theRequestBasket loaded properly.
        }          // else (balance agreement verified.)
    }              // Balance Agreement item found.

    // For the reply message.
    if (accountHash.empty() && theAccount) {
        theAccount.get().GetIdentifier(accountHash);
    }
    AddHashesToTransaction(tranOut, inbox, outbox, accountHash);

    // I put this here so it's signed/saved whether the balance agreement
    // itself was successful OR NOT.
    pResponseItem->SignContract(server_.GetServerNym(), reason_);
    pResponseItem->SaveContract();

    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
}

// DONE:  Make sure a CLOSING TRANSACTION number is provided, and recorded
// for use later in cron!

void Notary::NotarizeMarketOffer(
    otx::context::Client& context,
    ExclusiveAccount& theAssetAccount,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    bool& bOutSuccess)
{
    // The outgoing transaction is an "atMarketOffer", that is, "a reply to
    // the marketOffer request"
    tranOut.SetType(transactionType::atMarketOffer);

    std::shared_ptr<Item> pItem = nullptr;
    std::shared_ptr<Item> pBalanceItem = nullptr;
    std::shared_ptr<Item> pResponseItem = nullptr;
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will definitely be bundled in our reply to the user as well.
    // Therefore, let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();

    // Grab the actual server ID from this object, and use it as the server
    // ID here.
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto& NOTARY_ID = context.Notary();
    const auto strNymID = String::Factory(NYM_ID, api_.Crypto());

    pItem = tranIn.GetItem(itemType::marketOffer);
    pBalanceItem = tranIn.GetItem(itemType::transactionStatement);
    pResponseItem.reset(
        api_.Factory()
            .InternalSession()
            .Item(tranOut, itemType::atMarketOffer, identifier::Account{})
            .release());
    pResponseItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseItem);  // the Transaction's destructor will
                                     // cleanup the item. It "owns" it now.

    pResponseBalanceItem.reset(api_.Factory()
                                   .InternalSession()
                                   .Item(
                                       tranOut,
                                       itemType::atTransactionStatement,
                                       identifier::Account{})
                                   .release());
    pResponseBalanceItem->SetStatus(Item::rejection);  // the default.
    tranOut.AddItem(pResponseBalanceItem);  // the Transaction's destructor
                                            // will cleanup the item. It
                                            // "owns" it now.
    auto accountHash = identifier::Account{};

    if (!NYM_IS_ALLOWED(
            strNymID->Get(), ServerSettings::_transact_market_offer)) {
        LogError()(OT_PRETTY_CLASS())("User ")(strNymID.get())(
            " cannot do this transaction (All market "
            "offers are disallowed in "
            "server.cfg)")
            .Flush();
    } else if (nullptr == pBalanceItem) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())(
            "Expected transaction statement in trans #")(
            tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    } else if (nullptr == pItem) {
        auto strTemp = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())("Expected Item::marketOffer in trans #")(
            tranIn.GetTransactionNum())(": ")(
            strTemp->Exists() ? strTemp->Get()
                              : " (ERROR LOADING TRANSACTION INTO STRING)")
            .Flush();
    }
    // For now, there should only be one of these marketOffer items inside
    // the transaction. So we treat it that way... I either get it
    // successfully or not.
    else {
        // The response item will contain a copy of the request item. So I
        // save it into a string here so it can be saved into the "in
        // reference to" field.
        pItem->SaveContractRaw(strInReferenceTo);
        pBalanceItem->SaveContractRaw(strBalanceItem);

        // ASSET_ACCT_ID is the ID on the "from" Account that was passed in.
        // The CURRENCY_ACCT_ID is the ID on the "To" Account. (When doing a
        // transfer, normally 2nd acct is the Payee.)
        const auto ASSET_ACCT_ID =
            server_.API().Factory().Internal().Identifier(
                theAssetAccount.get());
        const auto& CURRENCY_ACCT_ID = pItem->GetDestinationAcctID();

        // Server response item being added to server response transaction
        // (tranOut)
        // They're getting SOME sort of response item.

        pResponseItem->SetReferenceString(strInReferenceTo);  // the response
                                                              // item carries a
                                                              // copy of what
                                                              // it's responding
                                                              // to.
        pResponseItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN
                                          // RESPONSE to pItem and its Owner
                                          // Transaction.

        pResponseBalanceItem->SetReferenceString(
            strBalanceItem);  // the response item carries a copy of what
                              // it's responding to.
        pResponseBalanceItem->SetReferenceToNum(
            pItem->GetTransactionNum());  // This response item is IN
                                          // RESPONSE to pItem and its Owner
                                          // Transaction.

        if (!pBalanceItem->VerifyTransactionStatement(context, tranIn)) {
            LogError()(OT_PRETTY_CLASS())(
                "ERROR verifying transaction statement")
                .Flush();
        } else {
            pResponseBalanceItem->SetStatus(
                Item::acknowledgement);  // the transaction agreement was
                                         // successful.

            // Load up the currency account and validate it.
            ExclusiveAccount currencyAccount =
                api_.Wallet().Internal().mutable_Account(
                    CURRENCY_ACCT_ID, reason_);
            // Also load up the Trade from inside the transaction item.
            auto strOffer = String::Factory();
            auto theOffer{api_.Factory().InternalSession().Offer()};

            OT_ASSERT(false != bool(theOffer));

            auto strTrade = String::Factory();
            pItem->GetAttachment(strTrade);
            auto pTrade = api_.Factory().InternalSession().Trade();

            OT_ASSERT(false != bool(pTrade));

            // First load the Trade up (from the string that was passed in
            // on the transaction item.)
            bool bLoadContractFromString =
                pTrade->LoadContractFromString(strTrade);

            // If failed to load the trade...
            if (!bLoadContractFromString) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR loading trade from string: ")(strTrade.get())
                    .Flush();
            }
            // I'm using the operator== because it exists. (Although now I
            // believe != exists also)
            // If the ID on the "from" account that was passed in,
            // does not match the "Acct From" ID on this transaction item
            else if (!(ASSET_ACCT_ID == pItem->GetPurportedAccountID())) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Error: Asset account ID on the transaction "
                    "does not match asset account ID on the "
                    "transaction item.")
                    .Flush();
            }
            // ok so the IDs match. Does the currency account exist?
            else if (false == bool(currencyAccount)) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "ERROR verifying existence of the currency "
                    "account in Notary::NotarizeMarketOffer.")
                    .Flush();
            } else if (!currencyAccount.get().VerifyContractID()) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "ERROR verifying Contract ID on the currency "
                    "account in Notary::NotarizeMarketOffer.")
                    .Flush();
            } else if (!currencyAccount.get().VerifyOwner(
                           context.RemoteNym())) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "ERROR verifying ownership of the currency "
                    "account in Notary::NotarizeMarketOffer.")
                    .Flush();
            }
            // Are both of the accounts of the same Asset Type?
            else if (
                theAssetAccount.get().GetInstrumentDefinitionID() ==
                currencyAccount.get().GetInstrumentDefinitionID()) {
                auto strInstrumentDefinitionID = String::Factory(
                         theAssetAccount.get().GetInstrumentDefinitionID(),
                         api_.Crypto()),
                     strCurrencyTypeID = String::Factory(
                         currencyAccount.get().GetInstrumentDefinitionID(),
                         api_.Crypto());
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR - user attempted to trade between identical "
                    "instrument definitions: ")(strInstrumentDefinitionID.get())
                    .Flush();
            }
            // Does it verify?
            // I call VerifySignature here since VerifyContractID was
            // already called in LoadExistingAccount().
            else if (!currencyAccount.get().VerifySignature(
                         server_.GetServerNym())) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "ERROR verifying signature on the Currency "
                    "account in Notary::NotarizeMarketOffer.")
                    .Flush();
            } else if (!pTrade->VerifySignature(context.RemoteNym())) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "ERROR verifying signature on the Trade in "
                    "Notary::NotarizeMarketOffer")
                    .Flush();
            } else if (
                pTrade->GetTransactionNum() != pItem->GetTransactionNum()) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "ERROR bad transaction number on trade in "
                    "Notary::NotarizeMarketOffer.")
                    .Flush();
            }
            // The transaction number opens the market offer, but there must
            // also be a closing number for closing it.
            else if (
                (pTrade->GetCountClosingNumbers() < 2) ||
                // Verify that it can still be USED
                !context.VerifyIssuedNumber(pTrade->GetAssetAcctClosingNum()) ||
                !context.VerifyIssuedNumber(
                    pTrade->GetCurrencyAcctClosingNum())) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "ERROR needed 2 valid closing transaction numbers in "
                    "Notary::NotarizeMarketOffer.")
                    .Flush();
            } else if (pTrade->GetNotaryID() != NOTARY_ID) {
                const auto strID1 = String::Factory(
                               pTrade->GetNotaryID(), api_.Crypto()),
                           strID2 = String::Factory(NOTARY_ID, api_.Crypto());
                LogError()(OT_PRETTY_CLASS())("ERROR wrong Notary ID (")(
                    strID1.get())(") on trade. "
                                  "Expected: ")(strID2.get())
                    .Flush();
            } else if (pTrade->GetSenderNymID() != NYM_ID) {
                const auto strID1 = String::Factory(
                               pTrade->GetSenderNymID(), api_.Crypto()),
                           strID2 = String::Factory(NYM_ID, api_.Crypto());
                LogError()(OT_PRETTY_CLASS())("ERROR wrong Nym ID (")(
                    strID1.get())(") on trade. "
                                  "Expected: ")(strID2.get())
                    .Flush();
            } else if (
                pTrade->GetInstrumentDefinitionID() !=
                theAssetAccount.get().GetInstrumentDefinitionID()) {
                const auto
                    strInstrumentDefinitionID1 = String::Factory(
                        pTrade->GetInstrumentDefinitionID(), api_.Crypto()),
                    strInstrumentDefinitionID2 = String::Factory(
                        theAssetAccount.get().GetInstrumentDefinitionID(),
                        api_.Crypto());
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR wrong Instrument Definition ID "
                    "(")(strInstrumentDefinitionID1.get())(
                    ") on trade. "
                    "Expected:"
                    " ")(strInstrumentDefinitionID2.get())
                    .Flush();
            } else if (pTrade->GetSenderAcctID() != ASSET_ACCT_ID) {
                const auto strAcctID1 = String::Factory(
                               pTrade->GetSenderAcctID(), api_.Crypto()),
                           strAcctID2 =
                               String::Factory(ASSET_ACCT_ID, api_.Crypto());
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR wrong asset Acct ID "
                    "(")(strAcctID1.get())(") on trade. "
                                           "Expected:"
                                           " ")(strAcctID2.get())
                    .Flush();
            } else if (
                pTrade->GetCurrencyID() !=
                currencyAccount.get().GetInstrumentDefinitionID()) {
                const auto
                    strID1 =
                        String::Factory(pTrade->GetCurrencyID(), api_.Crypto()),
                    strID2 = String::Factory(
                        currencyAccount.get().GetInstrumentDefinitionID(),
                        api_.Crypto());
                LogError()(OT_PRETTY_CLASS())("ERROR wrong Currency Type ID (")(
                    strID1.get())(") on trade. "
                                  "Expected:"
                                  " ")(strID2.get())
                    .Flush();
            } else if (pTrade->GetCurrencyAcctID() != CURRENCY_ACCT_ID) {
                const auto strID1 = String::Factory(
                               pTrade->GetCurrencyAcctID(), api_.Crypto()),
                           strID2 =
                               String::Factory(CURRENCY_ACCT_ID, api_.Crypto());
                LogError()(OT_PRETTY_CLASS())("ERROR wrong Currency Acct ID (")(
                    strID1.get())(") on trade. "
                                  "Expected:"
                                  " ")(strID2.get())
                    .Flush();
            }
            // If the Trade successfully verified, but I couldn't get the
            // offer out of it, then it actually DIDN'T successfully load
            // still.  :-(
            else if (!pTrade->GetOfferString(strOffer)) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR getting offer string from trade: ")(strTrade.get())
                    .Flush();
            } else if (!theOffer->LoadContractFromString(strOffer)) {
                LogError()(OT_PRETTY_CLASS())(
                    "ERROR loading offer from string: ")(strTrade.get())
                    .Flush();
            }
            // ...And then we use that same Nym to verify the signature on
            // the offer.
            else if (!theOffer->VerifySignature(context.RemoteNym())) {
                LogError()(OT_PRETTY_CLASS())("ERROR verifying offer signature")
                    .Flush();
            } else if (!pTrade->VerifyOffer(*theOffer)) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "FAILED verifying offer for Trade")
                    .Flush();
            } else if (
                theOffer->GetScale() < ServerSettings::GetMinMarketScale()) {
                LogError()(OT_PRETTY_CLASS())("FAILED verifying Offer, "
                                              "SCALE: ")(theOffer->GetScale())(
                    ". (Minimum is ")(ServerSettings::GetMinMarketScale())(".)")
                    .Flush();
            } else if (
                static_cast<std::int64_t>((context.OpenCronItems() / 3)) >=
                OTCron::GetCronMaxItemsPerNym()) {
                // NOTE:
                // We divided by 3 since this set contains THREE numbers for
                // each active market offer.
                // It's kind of a hack, since it may NOT be three numbers
                // for other cron items such as payment plans and smart
                // contracts. But it's a good enough approximation for now.
                //
                LogConsole()(OT_PRETTY_CLASS())(
                    "FAILED adding offer to market: "
                    "NYM HAS TOO MANY ACTIVE OFFERS ALREADY. See "
                    "'max_items_per_nym' setting in the config "
                    "file.")
                    .Flush();
            }
            // At this point I feel pretty confident that the Trade is a
            // valid request from the user. The top half of this function is
            // oriented around finding the "marketOffer" item (in the
            // "marketOffer" transaction) and setting up the response item
            // that will go into the response transaction. It also retrieves
            // the Trade object and fully validates it.
            //
            // Next all we need to do is add it to the market...
            else {
                // We don't actually add the trade to a market here.
                // Instead, we add it to the server's Cron object. That
                // object will take care of processing the offer on and off
                // of any market.
                //
                // NOTE: FYI, inside AddCronItem, since this is a new
                // CronItem, a Cron Receipt will be saved with the User's
                // signature on it, containing the Cron Item from the user's
                // original request. After that, the item is stored
                // internally to Cron itself, and signed by the server--and
                // changes over time as cron processes. (The original
                // receipt can always be loaded when necessary.)
                //
                std::shared_ptr<OTTrade> trade{pTrade.release()};
                if (server_.Cron().AddCronItem(
                        trade,
                        true,
                        Clock::now()))  // bSaveReceipt=true
                {
                    // todo need to be able to "roll back" if anything
                    // inside this block fails.

                    // Now we can set the response item as an
                    // acknowledgement instead of the default (rejection)
                    pResponseItem->SetStatus(Item::acknowledgement);

                    bOutSuccess = true;  // The offer was successfully
                                         // placed on the market.

                    LogVerbose()(OT_PRETTY_CLASS())(
                        "Successfully added Trade to Cron object.")
                        .Flush();

                    // Server side, the Nym stores a list of all open cron
                    // item numbers. (So we know if there is still stuff
                    // open on Cron for that Nym, and we know what it is.)
                    context.OpenCronItem(trade->GetTransactionNum());
                    context.OpenCronItem(trade->GetAssetAcctClosingNum());
                    context.OpenCronItem(trade->GetCurrencyAcctClosingNum());

                    // This just removes the Closing number so he can't USE
                    // it again. (Since he's using it as the closing number
                    // for this cron item now.) He's still RESPONSIBLE for
                    // the number until RemoveIssuedNumber() is called. If
                    // we didn't call this here, then he could come back
                    // later and USE THE NUMBER AGAIN! (Bad!) You might ask,
                    // why not remove the Opening number as well as the
                    // Closing numbers? The answer is, we already did,
                    // before we got here. (Otherwise we wouldn't have even
                    // gotten this far.)
                    //
                    context.ConsumeAvailable(trade->GetAssetAcctClosingNum());
                    context.ConsumeAvailable(
                        trade->GetCurrencyAcctClosingNum());
                    // RemoveIssuedNum will be called for the original
                    // transaction number when the finalReceipt is created.
                    // RemoveIssuedNum will be called for the Closing number
                    // when the finalReceipt is accepted.
                } else {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "Unable to add trade to Cron object. ")
                        .Flush();
                }
            }
        }  // transaction statement verified.
    }

    std::unique_ptr<Ledger> pInbox(
        theAssetAccount.get().LoadInbox(server_.GetServerNym()));
    std::unique_ptr<Ledger> pOutbox(
        theAssetAccount.get().LoadOutbox(server_.GetServerNym()));

    theAssetAccount.get().GetIdentifier(accountHash);
    AddHashesToTransaction(tranOut, *pInbox, *pOutbox, accountHash);

    // sign the response item before sending it back (it's already been
    // added to the transaction above) Now, whether it was rejection or
    // acknowledgement, it is set properly and it is signed, and it is owned
    // by the transaction, who will take it from here.
    pResponseItem->SignContract(server_.GetServerNym(), reason_);
    pResponseItem->SaveContract();  // the signing was of no effect because
                                    // I forgot to save. (fixed.)

    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
}

/// If the server receives a notarizeTransaction command, it will be
/// accompanied by a payload
/// containing a ledger to be notarized.  UserCmdNotarizeTransaction will
/// loop through that ledger, and for each transaction within, it calls THIS
/// method.
/// TODO think about error reporting here and sending a message back to
/// user.
void Notary::NotarizeTransaction(
    otx::context::Client& context,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    bool& bOutSuccess)
{
    struct Cleanup {
        OTTransaction& transaction_;
        const identity::Nym& server_;
        const PasswordPrompt& reason_;

        Cleanup(
            OTTransaction& transaction,
            const identity::Nym& server,
            const PasswordPrompt& reason)
            : transaction_(transaction)
            , server_(server)
            , reason_(reason)
        {
        }

        ~Cleanup()
        {
            transaction_.SignContract(server_, reason_);
            transaction_.SaveContract();
        }
    };

    const auto& serverNym = server_.GetServerNym();
    Cleanup cleanup(tranOut, serverNym, reason_);
    const auto lTransactionNumber = tranIn.GetTransactionNum();
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto strIDNym = String::Factory(NYM_ID, api_.Crypto());
    auto theFromAccount = api_.Wallet().Internal().mutable_Account(
        tranIn.GetPurportedAccountID(), reason_);
    std::unique_ptr<Ledger> pInbox(theFromAccount.get().LoadInbox(serverNym));
    std::unique_ptr<Ledger> pOutbox(theFromAccount.get().LoadOutbox(serverNym));

    if (false == bool(pInbox)) {
        LogError()(OT_PRETTY_CLASS())("Error loading or verifying inbox.")
            .Flush();

        return;
    } else {
        auto inboxHash = identifier::Generic{};
        pInbox->CalculateInboxHash(inboxHash);

        if (tranIn.GetInboxHash() != inboxHash) {
            LogError()(OT_PRETTY_CLASS())("Inbox hash mismatch. Local inbox "
                                          "hash: ")(inboxHash, api_.Crypto())(
                " Remote inbox "
                "hash: ")(tranIn.GetInboxHash(), api_.Crypto())(".")
                .Flush();
        }
    }

    if (false == bool(pOutbox)) {
        LogError()(OT_PRETTY_CLASS())("Error loading or verifying outbox.")
            .Flush();

        return;
    } else {
        auto outboxHash = identifier::Generic{};
        pOutbox->CalculateOutboxHash(outboxHash);

        if (tranIn.GetOutboxHash() != outboxHash) {
            LogError()(OT_PRETTY_CLASS())(
                "Outbox hash mismatch. Local outbox hash: ")(
                outboxHash, api_.Crypto())(" Remote outbox hash: ")(
                tranIn.GetOutboxHash(), api_.Crypto())(".")
                .Flush();
        }
    }

    auto accountHash = identifier::Account{};
    theFromAccount.get().ConsensusHash(context, accountHash, reason_);

    if (tranIn.GetAccountHash() != accountHash) {
        LogError()(OT_PRETTY_CLASS())(
            "Account hash mismatch. Local account hash: ")(
            accountHash, api_.Crypto())(" Remote account hash: ")(
            tranIn.GetAccountHash(), api_.Crypto())(".")
            .Flush();
    }

    auto& inbox = *pInbox;
    auto& outbox = *pOutbox;

    // Make sure the Account ID loaded from the file matches the one we just
    // set and used as the filename.
    if (!theFromAccount.get().VerifyContractID()) {
        // this should never happen. How did the wrong ID get into the
        // account file, if the right ID is on the filename itself? and vice
        // versa.
        const auto strIDAcct =
            String::Factory(tranIn.GetPurportedAccountID(), api_.Crypto());
        LogError()(OT_PRETTY_CLASS())("Error verifying account ID:")(
            strIDAcct.get())
            .Flush();
    }
    // Make sure the nymID loaded up in the account as its actual owner
    // matches the nym who was passed in to this function requesting a
    // transaction on this account... otherwise any asshole could do
    // transactions on your account, no?
    else if (!theFromAccount.get().VerifyOwner(context.RemoteNym())) {
        const auto idAcct =
            server_.API().Factory().Internal().Identifier(theFromAccount.get());
        const auto strIDAcct = String::Factory(idAcct, api_.Crypto());
        LogError()(OT_PRETTY_CLASS())(
            "Error verifying account ownership... "
            "Nym: ")(strIDNym.get())("  Acct: ")(strIDAcct.get())
            .Flush();
    }
    // Make sure I, the server, have signed this file.
    else if (!theFromAccount.get().VerifySignature(serverNym)) {
        const auto idAcct =
            server_.API().Factory().Internal().Identifier(theFromAccount.get());
        const auto strIDAcct = String::Factory(idAcct, api_.Crypto());
        LogError()(OT_PRETTY_CLASS())(
            "Error verifying server signature on "
            "account: ")(strIDAcct.get())(" for Nym: ")(strIDNym.get())
            .Flush();
    }
    // No need to call VerifyAccount() here since the above calls go above
    // and beyond that method.
    else if (!context.VerifyIssuedNumber(lTransactionNumber)) {
        const auto idAcct =
            server_.API().Factory().Internal().Identifier(theFromAccount.get());
        const auto strIDAcct = String::Factory(idAcct, api_.Crypto());
        // The user may not submit a transaction using a number he's already
        // used before.
        LogError()(OT_PRETTY_CLASS())("Error verifying transaction "
                                      "number ")(lTransactionNumber)(" on user "
                                                                     "Nym: ")(
            strIDNym.get())(" A"
                            "cc"
                            "ou"
                            "nt"
                            ":"
                            " ")(strIDAcct.get())
            .Flush();
    }

    // The items' acct and server ID were already checked in
    // VerifyContractID() when they were loaded. Now this checks a little
    // deeper, to verify ownership, signatures, and transaction number on
    // each item.  That way those things don't have to be checked for
    // security over and over
    // again in the subsequent calls.
    //
    else if (!tranIn.VerifyItems(context.RemoteNym(), reason_)) {
        const auto idAcct =
            server_.API().Factory().Internal().Identifier(theFromAccount.get());
        const auto strIDAcct = String::Factory(idAcct, api_.Crypto());
        LogError()(OT_PRETTY_CLASS())("Error verifying transaction items. "
                                      "Trans: ")(lTransactionNumber)(" Nym: ")(
            strIDNym.get())(" "
                            "A"
                            "c"
                            "c"
                            "o"
                            "u"
                            "n"
                            "t"
                            ":"
                            " ")(strIDAcct.get())
            .Flush();
    }

    // any other security stuff?
    // Todo do I need to verify the server ID here as well?
    else {
        // We don't want any transaction number being used twice. (The
        // number, at this point, is STILL issued to the user, who is still
        // responsible for that number and must continue signing for it. All
        // this means here is that the user no longer has the number on his
        // AVAILABLE list. Removal from issued list happens separately.)
        if (!context.ConsumeAvailable(lTransactionNumber)) {
            LogError()(OT_PRETTY_CLASS())(
                "Error removing transaction number (as available) "
                "from user nym in Notary::NotarizeTransaction.")
                .Flush();
        } else {
            itemType theReplyItemType = itemType::error_state;

            switch (tranIn.GetType()) {
                // TRANSFER (account to account)
                // Alice sends a signed request to the server asking it to
                // transfer from her account ABC to the inbox of account
                // DEF. A copy will also remain in her outbox until canceled
                // or accepted.
                case transactionType::transfer:
                    LogConsole()("    Notarizing transfer").Flush();
                    NotarizeTransfer(
                        context,
                        theFromAccount,
                        tranIn,
                        tranOut,
                        inbox,
                        outbox,
                        bOutSuccess);
                    theReplyItemType = itemType::atTransfer;
                    break;

                // PROCESS INBOX (currently, all incoming transfers must be
                // accepted.)
                // Bob sends a signed request to the server asking it to
                // reject some of his inbox items and/or accept some into
                // his account DEF.
                case transactionType::processInbox:
                    LogConsole()("    Notarizing process inbox").Flush();
                    NotarizeProcessInbox(
                        context,
                        theFromAccount,
                        tranIn,
                        tranOut,
                        inbox,
                        outbox,
                        bOutSuccess);
                    //                    theReplyItemType =
                    //                    Item::atProcessInbox;
                    // // Nonexistent, and here, unused.
                    // (There is a processInbox message that carries that
                    // transaction...)
                    break;

                // WITHDRAWAL (cash or voucher)
                // Alice sends a signed request to the server asking it to
                // debit her account ABC and then issue her a purse full of
                // blinded cash tokens
                // --OR-- a voucher (a cashier's cheque, made out to any
                // recipient's
                // Nym ID, or made out to a blank recipient, just like a
                // blank cheque.)
                case transactionType::withdrawal: {
                    auto pItemVoucher =
                        tranIn.GetItem(itemType::withdrawVoucher);
                    auto pItemCash = tranIn.GetItem(itemType::withdrawal);

                    if (false != bool(pItemCash)) {
                        theReplyItemType = itemType::atWithdrawal;
                        LogConsole()("    Notarizing withdrawal (cash)")
                            .Flush();
                    } else if (false != bool(pItemVoucher)) {
                        theReplyItemType = itemType::atWithdrawVoucher;
                        LogConsole()("    Notarizing withdrawal (voucher)")
                            .Flush();
                    }
                    NotarizeWithdrawal(
                        context,
                        theFromAccount,
                        tranIn,
                        tranOut,
                        inbox,
                        outbox,
                        bOutSuccess);
                } break;

                // DEPOSIT    (cash or cheque)
                // Bob sends a signed request to the server asking it to
                // deposit into his account ABC. He includes with his
                // request a signed cheque made out to Bob's user ID (or
                // blank), --OR-- a purse full of tokens.
                case transactionType::deposit:
                    LogConsole()("    Notarizing deposit").Flush();
                    NotarizeDeposit(
                        context,
                        theFromAccount,
                        tranIn,
                        tranOut,
                        inbox,
                        outbox,
                        bOutSuccess);
                    theReplyItemType = itemType::atDeposit;
                    break;

                // PAY DIVIDEND
                // Bob sends a signed request to the server asking it to pay
                // all shareholders of a given instrument definition at the
                // rate of $X per share, where X and $ are both
                // configurable.
                case transactionType::payDividend:
                    LogConsole()("    Notarizing pay dividend").Flush();
                    NotarizePayDividend(
                        context,
                        theFromAccount,
                        tranIn,
                        tranOut,
                        inbox,
                        outbox,
                        bOutSuccess);
                    theReplyItemType = itemType::atPayDividend;
                    break;

                // MARKET OFFER
                // Bob sends a signed request to the server asking it to put
                // an offer on the market. He includes with his request a
                // signed trade listing the relevant information, instrument
                // definitions and account IDs.
                case transactionType::marketOffer:
                    LogConsole()("    Notarizing market offer").Flush();
                    NotarizeMarketOffer(
                        context, theFromAccount, tranIn, tranOut, bOutSuccess);
                    theReplyItemType = itemType::atMarketOffer;
                    break;

                // PAYMENT PLAN
                // Bob sends a signed request to the server asking it to
                // make regular payments to Alice. (BOTH Alice AND Bob must
                // have signed the same contract.)
                case transactionType::paymentPlan:
                    LogConsole()("    Notarizing payment plan").Flush();
                    NotarizePaymentPlan(
                        context, theFromAccount, tranIn, tranOut, bOutSuccess);
                    theReplyItemType = itemType::atPaymentPlan;
                    break;

                // SMART CONTRACT
                // Bob sends a signed request to the server asking it to
                // activate a
                // smart contract.
                // Bob is the authorizing agent for one of the parties, all
                // of whom have signed it, and have provided transaction #s
                // for it.
                case transactionType::smartContract: {
                    LogConsole()("    Notarizing smart contract").Flush();

                    // For all transaction numbers used on cron items, we
                    // keep track of them in the GetSetOpenCronItems. This
                    // will be removed again below, if the transaction
                    // fails.
                    context.OpenCronItem(lTransactionNumber);
                    NotarizeSmartContract(
                        context, theFromAccount, tranIn, tranOut, bOutSuccess);
                    theReplyItemType = itemType::atSmartContract;
                } break;

                // CANCEL CRON ITEM
                // (Cron items: market offers, payment plans...) Bob sends a
                // signed request to the server asking it to cancel a
                // REGULARLY PROCESSING CONTRACT that he had previously
                // created.
                case transactionType::cancelCronItem: {
                    LogConsole()("    Notarizing cancelCronItem").Flush();
                    NotarizeCancelCronItem(
                        context, theFromAccount, tranIn, tranOut, bOutSuccess);
                    theReplyItemType = itemType::atCancelCronItem;
                } break;

                // EXCHANGE BASKET
                // Bob sends a signed request to the server asking it to
                // exchange
                // funds
                // in or out of a basket currency. (From-or-to his main
                // basket account and his various sub-accounts for each
                // member currency in the basket.)
                case transactionType::exchangeBasket:
                    LogConsole()("    Notarizing exchange basket").Flush();
                    NotarizeExchangeBasket(
                        context,
                        theFromAccount,
                        tranIn,
                        tranOut,
                        inbox,
                        outbox,
                        bOutSuccess);
                    theReplyItemType = itemType::atExchangeBasket;
                    break;

                default:
                    LogError()(OT_PRETTY_CLASS())("Error, unexpected type: ")(
                        tranIn.GetTypeString())
                        .Flush();
                    break;
            }

            // Where appropriate, remove a transaction number from my issued
            // list
            // (the list of numbers I must sign for in every balance
            // agreement.)
            bool bIsCronItem = false;

            switch (tranIn.GetType()) {
                case transactionType::marketOffer:
                case transactionType::paymentPlan:
                case transactionType::smartContract:
                    bIsCronItem = true;
                    [[fallthrough]];
                case transactionType::transfer: {
                    // If success, then Issued number stays on Nym's issued
                    // list until the transfer, paymentPlan, marketOffer, or
                    // smart contract is entirely closed and removed. In the
                    // case of transfer, that's when the transfer receipt is
                    // accepted. In the case of markets and paymentplans,
                    // that's when they've been entirely removed from Cron
                    // (many intermediary receipts might occur before that
                    // happens.) At that time, a final receipt is issued
                    // with a closing transaction number (to make sure the
                    // user closes all of the related market receipts.)
                    //
                    // But if failure, then Issued number is immediately
                    // removed.
                    // (It already can't be used again, and there's no
                    // receipt to clear later, thus no reason to save it...)
                    {
                        auto pItem = tranOut.GetItem(theReplyItemType);

                        if (false != bool(pItem)) {
                            if (Item::rejection == pItem->GetStatus()) {
                                // If this is a cron item, then we need to
                                // remove it from the list of open cron
                                // items as well.
                                if (bIsCronItem) {
                                    context.CloseCronItem(lTransactionNumber);
                                }

                                if (!context.ConsumeIssued(
                                        lTransactionNumber)) {
                                    const auto strNymID =
                                        String::Factory(NYM_ID, api_.Crypto());
                                    LogError()(OT_PRETTY_CLASS())(
                                        "Error removing issued "
                                        "number")(lTransactionNumber)(" from "
                                                                      "user "
                                                                      "nym:"
                                                                      " ")(
                                        strNymID.get())
                                        .Flush();
                                }
                            }
                        }
                    }
                } break;
                // In the case of the below transaction types, the
                // transaction number is removed from the Nym's issued list
                // SUCCESS OR FAIL. (It's closed either way.)
                //
                case transactionType::processInbox:
                case transactionType::payDividend:
                case transactionType::withdrawal:
                case transactionType::deposit:
                case transactionType::cancelCronItem:
                case transactionType::exchangeBasket: {
                    if (!context.ConsumeIssued(lTransactionNumber)) {
                        const auto strNymID =
                            String::Factory(NYM_ID, api_.Crypto());
                        LogError()(OT_PRETTY_CLASS())("Error removing issued "
                                                      "number ")(
                            lTransactionNumber)(" from user "
                                                "nym: ")(strNymID.get())
                            .Flush();
                    }
                } break;
                default:
                    LogError()(OT_PRETTY_CLASS())("Error, unexpected type:")(
                        tranIn.GetTypeString())
                        .Flush();
                    break;
            }
        }
    }
}

/// The client may send multiple transactions in the ledger when he calls
/// processNymbox. This function will be called for each of those. Each
/// processNymbox transaction may contain multiple items accepting or
/// rejecting certain transactions. The server acknowledges and notarizes
/// those transactions accordingly. (And each of those transactions must be
/// accepted or rejected in whole.)
//
// The processNymbox TRANSACTION has a series of TRANSACTION ITEMS. One is
// the transaction statement (which is like a balance agreement, except
// there's no balance, since there's no asset account.) The rest are *items*
// IN REFERENCE TO some *transaction* in my Nymbox (signing to accept it.)
// At this point you can't really reject Nymbox receipts, just like you
// can't reject inbox receipts. Why not? Haven't coded it yet. So your items
// on your processNymbox transaction can only accept things (notices, new
// transaction numbers,
auto Notary::NotarizeProcessNymbox(
    otx::context::Client& context,
    OTTransaction& tranIn,
    OTTransaction& tranOut,
    bool& bOutSuccess) -> bool
{
    // The outgoing transaction is an "atProcessNymbox", that is, "a reply
    // to the process nymbox request"
    tranOut.SetType(transactionType::atProcessNymbox);
    std::shared_ptr<Item> pItem = nullptr;
    std::shared_ptr<Item> pBalanceItem =
        tranIn.GetItem(itemType::transactionStatement);
    std::shared_ptr<Item> pResponseItem = nullptr;
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;

    // Grab the actual server ID from this object, and use it as the server
    // ID here.
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto& NOTARY_ID = context.Notary();
    UnallocatedSet<TransactionNumber> newNumbers;
    auto theNymbox{
        api_.Factory().InternalSession().Ledger(NYM_ID, NYM_ID, NOTARY_ID)};

    OT_ASSERT(false != bool(theNymbox));

    auto strNymID = String::Factory(NYM_ID, api_.Crypto());
    bool bSuccessLoadingNymbox = theNymbox->LoadNymbox();

    if (true == bSuccessLoadingNymbox) {
        bSuccessLoadingNymbox =
            theNymbox->VerifyAccount(server_.GetServerNym());
    }

    pResponseBalanceItem.reset(api_.Factory()
                                   .InternalSession()
                                   .Item(
                                       tranOut,
                                       itemType::atTransactionStatement,
                                       identifier::Account{})
                                   .release());
    pResponseBalanceItem->SetStatus(Item::rejection);  // the default.
    // the Transaction's destructor will cleanup the item. It "owns" it now.
    tranOut.AddItem(pResponseBalanceItem);
    bool bNymboxHashRegenerated = false;
    // In case the Nymbox hash is updated, we will have the updated version
    // here
    auto NYMBOX_HASH = identifier::Generic{};

    if (!bSuccessLoadingNymbox) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed loading or verifying Nymbox for user: ")(strNymID.get())
            .Flush();
    } else if (nullptr == pBalanceItem) {
        const auto strTransaction = String::Factory(tranIn);
        LogError()(OT_PRETTY_CLASS())(
            "No Transaction Agreement item found on this "
            "transaction ")(tranIn.GetTransactionNum())(
            " ("
            "required):"
            " ")(strTransaction.get())
            .Flush();
    } else {
        auto strBalanceItem = String::Factory();
        pBalanceItem->SaveContractRaw(strBalanceItem);
        // the response item carries a copy of what it's responding to.
        pResponseBalanceItem->SetReferenceString(strBalanceItem);
        // This response item is IN RESPONSE to tranIn's balance agreement
        pResponseBalanceItem->SetReferenceToNum(
            pBalanceItem->GetTransactionNum());

        // The incoming transaction accepts various messages and transaction
        // numbers. So when it's all finished, my list of transaction
        // numbers will be higher.
        //
        // I would like to not even process the whole giant loop below, if I
        // can verify here now that the transaction agreement is wrong.
        //
        // Thus I will actually loop through the acceptTransaction items in
        // tranIn, and then for each one, I'll lookup the ACTUAL transaction
        // in the nymbox, and get its ACTUAL value. (And store them all up
        // on a temp nym.)
        //
        // The ones being accepted will therefore be added to my Nym, so the
        // Transaction Statement will be signed as if that is already the
        // case. (So they'll match.)
        //
        // I need to add them all to the Nym, verify the transaction
        // statement, and then remove them again. (which is why I stored
        // them on a temp Nym
        // :-) Then if it succeeds for real, at the bottom of this function,
        // I'll go ahead and add them properly (so it adds them to both
        // lists.)
        bool bSuccessFindingAllTransactions = true;

        for (auto& it : tranIn.GetItemList()) {
            pItem = it;

            OT_ASSERT_MSG(
                nullptr != pItem, "Pointer should not have been nullptr.");

            if (pItem->GetType() == itemType::acceptTransaction) {
                auto pTransaction =
                    theNymbox->GetTransaction(pItem->GetReferenceToNum());

                if ((nullptr != pTransaction) &&
                    (pTransaction->GetType() ==
                     transactionType::blank))  // The user is referencing a
                                               // blank in the nymbox, which
                                               // indeed is actually there.
                {
                    bSuccessFindingAllTransactions = true;
                    NumList listNumbersNymbox, listNumbersUserItem;
                    pItem->GetNumList(listNumbersUserItem);
                    pTransaction->GetNumList(listNumbersNymbox);

                    // MAKE SURE THEY MATCH. (Otherwise user could be
                    // signing numbers that differ from the actual ones in
                    // the Nymbox.)
                    if (!listNumbersNymbox.Verify(listNumbersUserItem)) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Failed verifying: The numbers on the actual "
                            "blank "
                            "transaction in the nymbox do not match the "
                            "list "
                            "of numbers sent over by the user.")
                            .Flush();
                    } else {
                        // INSTEAD of merely adding the TRANSACTION NUMBER
                        // of the blank to the Nym, we actually add an
                        // entire list of numbers retrieved from the blank,
                        // including its main number.
                        UnallocatedSet<TransactionNumber> theNumbers;
                        listNumbersNymbox.Output(theNumbers);

                        // Looping through the transaction numbers on the
                        // Nymbox blank transaction. (There's probably 20 of
                        // them.)
                        for (const auto& number : theNumbers) {
                            // (We don't add it if it's already there.)
                            if (!context.VerifyIssuedNumber(number)) {
                                newNumbers.insert(number);
                            } else {
                                LogError()(OT_PRETTY_CLASS())(
                                    "tried to add an issued trans "
                                    "#(")(number)("to a nym who ALREADY had "
                                                  "that number")
                                    .Flush();
                            }
                        }
                    }
                } else {
                    bSuccessFindingAllTransactions = false;
                    break;
                }
            }
        }

        // NOTICE: We're adding up all the new transaction numbers being
        // added. (Item::acceptTransaction)... but we're NOT bothering
        // with the ones being REMOVED (Item::acceptFinalReceipt) here in
        // NotarizeProecessNymbox. Why not? BECAUSE THEY WERE ALREADY
        // REMOVED. They were removed when the Cron Item expired, or was
        // canceled. The finalReceipt notice that went into the Nymbox was
        // ONLY A COURTESY
        // -- the NUMBER was ALREADY REMOVED. Thus, we don't need to remove
        // it now, although we DO need to add the new transaction numbers
        // (acceptTransaction).
        //
        // (Of course, I will still remove the finalReceipt from the Nymbox.
        // I just don't have to juggle any transaction numbers on the NYM as
        // a result of this.)
        if (!bSuccessFindingAllTransactions) {
            LogError()(OT_PRETTY_CLASS())(
                "transactions in processNymbox message do not match actual "
                "nymbox.")
                .Flush();
        }
        // VERIFY TRANSACTION STATEMENT!
        else if (!pBalanceItem->VerifyTransactionStatement(
                     context, tranIn, newNumbers, false)) {
            LogError()(OT_PRETTY_CLASS())(
                "ERROR verifying transaction statement")
                .Flush();
        } else {
            // TRANSACTION AGREEMENT WAS SUCCESSFUL.......
            pResponseBalanceItem->SetStatus(Item::acknowledgement);

            // THE ABOVE LOOP WAS JUST A TEST RUN
            //
            // (TO **VERIFY TRANSACTION STATEMENT** BEFORE WE BOTHERED TO
            // RUN THIS LOOP BELOW...) (AND ALSO SO WE COULD GET THE LIST OF
            // NUMBERS FOR THE STATEMENT ONTO TEMP NYM.)

            // loop through the items that make up the incoming transaction,
            // and add them to the Nym, and remove them from the Nymbox, as
            // appropriate.
            for (auto& it : tranIn.GetItemList()) {
                pItem = it;
                OT_ASSERT_MSG(
                    nullptr != pItem, "Pointer should not have been nullptr.");

                // We already handled this one (if we're even in this block
                // in the first place.)
                if (itemType::transactionStatement == pItem->GetType()) {
                    continue;
                }

                // If the client sent an accept item then let's process it.
                if ((Item::request == pItem->GetStatus()) &&
                    ((itemType::acceptFinalReceipt ==
                      pItem->GetType()) ||  // Clearing out a finalReceipt
                                            // notice.
                     (itemType::acceptTransaction ==
                      pItem->GetType()) ||  // Accepting new transaction
                                            // number.
                     (itemType::acceptMessage ==
                      pItem->GetType()) ||  // Accepted
                                            // message.
                     (itemType::acceptNotice ==
                      pItem->GetType())  // Accepted server notification.
                     )) {
                    auto strInReferenceTo = String::Factory();

                    // The response item will contain a copy of the "accept"
                    // request.
                    // So I'm just setting aside a copy now for those
                    // purposes later.
                    pItem->SaveContractRaw(strInReferenceTo);
                    itemType theReplyItemType;

                    switch (pItem->GetType()) {
                        case itemType::acceptFinalReceipt: {
                            theReplyItemType = itemType::atAcceptFinalReceipt;
                        } break;
                        case itemType::acceptTransaction: {
                            theReplyItemType = itemType::atAcceptTransaction;
                        } break;
                        case itemType::acceptMessage: {
                            theReplyItemType = itemType::atAcceptMessage;
                        } break;
                        case itemType::acceptNotice: {
                            theReplyItemType = itemType::atAcceptNotice;
                        } break;
                        default: {
                            LogError()(OT_PRETTY_CLASS())(
                                "Should never happen.")
                                .Flush();
                        }
                            continue;
                    }

                    // Server response item being added to server response
                    // transaction (tranOut) They're getting SOME sort of
                    // response item.
                    pResponseItem.reset(api_.Factory()
                                            .InternalSession()
                                            .Item(
                                                tranOut,
                                                theReplyItemType,
                                                identifier::Account{})
                                            .release());
                    // the default.
                    pResponseItem->SetStatus(Item::rejection);
                    // the response item carries a copy of what it's
                    // responding to.
                    pResponseItem->SetReferenceString(strInReferenceTo);
                    // // This was just 0 every time, since Nymbox needs no
                    // transaction numbers. So the reference was useless.
                    // I'm hoping to change it to this and make sure nothing
                    // breaks. ReferenceNum actually means you can match it
                    // up against the request items, and also, that is where
                    // THEY store it.
                    pResponseItem->SetReferenceToNum(
                        pItem->GetReferenceToNum());
                    // the Transaction's destructor will cleanup the item.
                    // It "owns" it now.
                    tranOut.AddItem(pResponseItem);
                    std::shared_ptr<OTTransaction> pServerTransaction = nullptr;

                    if ((nullptr !=
                         (pServerTransaction = theNymbox->GetTransaction(
                              pItem->GetReferenceToNum()))) &&
                        ((transactionType::finalReceipt ==
                          pServerTransaction->GetType()) ||  // finalReceipt
                                                             // (notice that
                                                             // an opening
                                                             // num was
                                                             // closed.)
                         (transactionType::blank ==
                          pServerTransaction->GetType()) ||  // new
                                                             // transaction
                         // number waiting to
                         // be picked up.
                         (transactionType::message ==
                          pServerTransaction->GetType()) ||  // message in
                                                             // the nymbox
                         (transactionType::replyNotice ==
                          pServerTransaction->GetType()) ||  // replyNotice
                                                             // containing a
                                                             // server
                         // reply to a previous request.
                         // (Some replies are so important,
                         // this is used to make sure users
                         // get them.)
                         (transactionType::successNotice ==
                          pServerTransaction->GetType()) ||  // successNotice
                                                             // that you signed
                                                             // out a
                                                             // transaction#.
                         (transactionType::notice ==
                          pServerTransaction->GetType()) ||  // server
                                                             // notification,
                                                             // in the
                                                             // nymbox
                         (transactionType::instrumentNotice ==
                          pServerTransaction->GetType())  // A financial
                         // instrument sent from
                         // another user.
                         // (Nymbox=>PaymentInbox)
                         )) {
                        // the accept item will come with the transaction
                        // number that it's referring to. So we'll just look
                        // up that transaction in the nymbox, and now that
                        // it's been accepted, we'll process it.

                        // At this point, pItem points to the client's
                        // attempt to accept pServerTransaction and
                        // pServerTransaction is the server's created
                        // transaction in my nymbox that might have a
                        // message or transaction number on it I might find
                        // useful.

                        // What are we doing in this code?
                        //
                        // I need to accept various items that are sitting
                        // in my nymbox, such as:
                        //
                        // -- transaction numbers waiting to be accepted
                        // (they cannot be rejected.)
                        //
                        // -- messages waiting to be accepted (they cannot
                        // be rejected.)
                        //

                        // The below block only executes for ACCEPTING a
                        // MESSAGE
                        if ((itemType::acceptMessage == pItem->GetType()) &&
                            (transactionType::message ==
                             pServerTransaction->GetType())) {
                            // pItem contains the current user's attempt to
                            // accept the
                            // ['message'] located in pServerTransaction.
                            // Now we have the user's item and the item he
                            // is trying to accept.
                            pServerTransaction->DeleteBoxReceipt(
                                *theNymbox);  // faster.
                            theNymbox->RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theNymbox->ReleaseSignatures();
                            theNymbox->SignContract(
                                server_.GetServerNym(), reason_);
                            theNymbox->SaveContract();
                            theNymbox->SaveNymbox();

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(Item::acknowledgement);
                        }  // its type is Item::aacceptMessage

                        // The below block only executes for ACCEPTING a
                        // NOTICE
                        else if (
                            (itemType::acceptNotice == pItem->GetType()) &&
                            ((transactionType::notice ==
                              pServerTransaction->GetType()) ||
                             (transactionType::replyNotice ==
                              pServerTransaction->GetType()) ||
                             (transactionType::successNotice ==
                              pServerTransaction->GetType()) ||
                             (transactionType::instrumentNotice ==
                              pServerTransaction->GetType()))) {
                            // pItem contains the current user's attempt to
                            // accept the
                            // ['notice'] or replyNotice or successNotice or
                            // instrumentNotice
                            // located in pServerTransaction.
                            // Now we have the user's item and the item he
                            // is trying to accept.

                            pServerTransaction->DeleteBoxReceipt(
                                *theNymbox);  // faster.
                            theNymbox->RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theNymbox->ReleaseSignatures();
                            theNymbox->SignContract(
                                server_.GetServerNym(), reason_);
                            theNymbox->SaveContract();
                            theNymbox->SaveNymbox();

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(Item::acknowledgement);

                        }  // its type is Item::acceptNotice

                        // The below block only executes for ACCEPTING a
                        // TRANSACTION NUMBER
                        // It also places a success notice into the Nymbox,
                        // to solve sync issues. (We'll make SURE the client
                        // got the notice! Probably should do this for cash
                        // withdrawals as well...)
                        else if (
                            (itemType::acceptTransaction == pItem->GetType()) &&
                            (transactionType::blank ==
                             pServerTransaction->GetType())) {
                            // Add the success notice to the Nymbox, so if
                            // the Nym fails to see the server reply, he can
                            // still get his transaction # later, from the
                            // notice, instead of going out of sync.
                            //
                            auto lSuccessNoticeTransNum = TransactionNumber{};
                            bool bGotNextTransNum =
                                server_.GetTransactor()
                                    .issueNextTransactionNumber(
                                        lSuccessNoticeTransNum);

                            if (!bGotNextTransNum) {
                                LogError()(OT_PRETTY_CLASS())(
                                    "Error getting next transaction "
                                    "number in "
                                    "Notary::NotarizeProcessNymbox "
                                    "for transactionType::blank (for "
                                    "the successNotice).")
                                    .Flush();
                            } else {
                                // Drop SUCCESS NOTICE in the Nymbox
                                //
                                auto pSuccessNotice{
                                    api_.Factory()
                                        .InternalSession()
                                        .Transaction(
                                            *theNymbox,
                                            transactionType::successNotice,
                                            originType::not_applicable,
                                            lSuccessNoticeTransNum)};

                                if (nullptr != pSuccessNotice)  // The above has
                                                                // an OT_ASSERT
                                                                // within, but I
                                                                // just like to
                                                                // check my
                                                                // pointers.
                                {
                                    // If I accepted blank trans#10, then
                                    // this successNotice is in reference to
                                    // #10.
                                    //
                                    pSuccessNotice->SetReferenceToNum(
                                        pServerTransaction
                                            ->GetTransactionNum());

                                    // Contains a copy of the Item where I
                                    // actually accepted the blank
                                    // transaction
                                    // #.
                                    // (which generated the notice in the
                                    // first place...)
                                    //
                                    pSuccessNotice->SetReferenceString(
                                        strInReferenceTo);

                                    NumList theOutput;
                                    pServerTransaction->GetNumList(
                                        theOutput);  // now theOutput
                                                     // contains the numlist
                                                     // from the
                                    // server-side nymbox's copy
                                    // of the blank. (containing
                                    // 20 transaction #s)

                                    pSuccessNotice->AddNumbersToTransaction(
                                        theOutput);  // Now we add those
                                                     // numbers to the
                                                     // success notice. That
                                                     // way client can add
                                                     // those numbers to his
                                                     // issued and
                                                     // transaction lists.

                                    pSuccessNotice->SignContract(
                                        server_.GetServerNym(), reason_);
                                    pSuccessNotice->SaveContract();

                                    std::shared_ptr<OTTransaction>
                                        successNotice{pSuccessNotice.release()};
                                    theNymbox->AddTransaction(
                                        successNotice);  // Add the
                                                         // successNotice
                                                         // to the nymbox.
                                                         // It takes
                                                         // ownership.

                                    successNotice->SaveBoxReceipt(*theNymbox);
                                }
                            }
                            // pItem contains the current user's attempt to
                            // accept the
                            // transaction number located in
                            // pServerTransaction. Now we have the user's
                            // item and the item he is trying to accept.

                            // Here we remove the blank transaction that was
                            // just accepted.
                            //
                            pServerTransaction->DeleteBoxReceipt(
                                *theNymbox);  // faster.
                            theNymbox->RemoveTransaction(
                                pServerTransaction->GetTransactionNum());
                            theNymbox->ReleaseSignatures();
                            theNymbox->SignContract(
                                server_.GetServerNym(), reason_);
                            theNymbox->SaveContract();
                            theNymbox->SaveNymbox(NYMBOX_HASH);

                            bNymboxHashRegenerated = true;

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(Item::acknowledgement);
                        }

                        // The below block only executes for CLEARING a
                        // finalReceipt
                        // (an OPENING TRANSACTION NUMBER was already
                        // removed), and this was a notice that that had
                        // occurred. The client has seen the notice and is
                        // now clearing it from the box.
                        else if (
                            (itemType::acceptFinalReceipt ==
                             pItem->GetType()) &&
                            (transactionType::finalReceipt ==
                             pServerTransaction->GetType())) {
                            // pItem contains the current user's attempt to
                            // clear the
                            // finalReceipt located in pServerTransaction.
                            // Now we have the user's item and the item he
                            // is trying to accept.

                            pServerTransaction->DeleteBoxReceipt(
                                *theNymbox);  // faster.
                            theNymbox->RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            theNymbox->ReleaseSignatures();
                            theNymbox->SignContract(
                                server_.GetServerNym(), reason_);
                            theNymbox->SaveContract();
                            theNymbox->SaveNymbox(NYMBOX_HASH);

                            bNymboxHashRegenerated = true;

                            // Now we can set the response item as an
                            // acknowledgement instead of the default
                            // (rejection)
                            pResponseItem->SetStatus(Item::acknowledgement);
                        }
                    } else {
                        LogError()(OT_PRETTY_CLASS())(
                            "Error finding original Nymbox transaction that "
                            "client is trying to accept: ")(
                            pItem->GetReferenceToNum())
                            .Flush();
                    }

                    // sign the response item before sending it back (it's
                    // already been added to the transaction above)
                    // Now, whether it was rejection or acknowledgement, it
                    // is set properly and it is signed, and it is owned by
                    // the transaction, who will take it from here.
                    pResponseItem->ReleaseSignatures();
                    pResponseItem->SignContract(
                        server_.GetServerNym(), reason_);
                    pResponseItem->SaveContract();
                } else {
                    const std::int32_t nStatus = pItem->GetStatus();
                    auto strItemType = String::Factory();
                    pItem->GetTypeString(strItemType);
                    LogError()(OT_PRETTY_CLASS())(
                        "Error, unexpected item type "
                        "(")(strItemType.get())(") and/or status "
                                                "(")(nStatus)(")")
                        .Flush();
                }
            }
        }
    }

    pResponseBalanceItem->ReleaseSignatures();
    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
    tranOut.ReleaseSignatures();
    tranOut.SignContract(server_.GetServerNym(), reason_);
    tranOut.SaveContract();

    if (bNymboxHashRegenerated) {
        auto clientContext = api_.Wallet().Internal().mutable_ClientContext(
            context.RemoteNym().ID(), reason_);
        clientContext.get().SetLocalNymboxHash(NYMBOX_HASH);
    }

    auto strPath = String::Factory();

    // On the server side, response will only have chance to succeed if
    // balance agreement succeeds first. Therefore, you will never see
    // successful response but failed balance, since it would stop at the
    // balance and response itself would remain failed with no chance of
    // changing.
    //
    // Thus, "success" must be when balance succeeded and transaction
    // succeeded, and "failure" must be when balance succeeded but
    // transaction failed.
    //
    // If NEITHER succeeded, then there is no point recording it to a file,
    // now is there?
    if ((nullptr != pResponseBalanceItem) &&
        (Item::acknowledgement == pResponseBalanceItem->GetStatus())) {
        if (tranOut.GetSuccess()) {
            // Transaction agreement was a success, AND process nymbox was a
            // success. Therefore, add any new issued numbers to theNym, and
            // save.
            context.AcceptIssuedNumbers(newNumbers);  // TODO: capture
                                                      // return
            bOutSuccess = true;  // the processNymbox was successful.
            strPath->Set(
                api::Legacy::GetFilenameSuccess(strNymID->Get()).c_str());
        } else {
            strPath->Set(api::Legacy::GetFilenameFail(strNymID->Get()).c_str());
        }

        const char* szFoldername = server_.API().Internal().Legacy().Receipt();
        tranOut.SaveContract(szFoldername, strPath->Get());
    }

    return bNymboxHashRegenerated;
}

/// The client may send multiple transactions in the ledger when he calls
/// processInbox. This function will be called for each of those. Each may
/// contain multiple items accepting or rejecting certain transactions. The
/// server acknowledges and notarizes those transactions accordingly. (And
/// each of those transactions must be accepted or rejected in whole.)
void Notary::NotarizeProcessInbox(
    otx::context::Client& context,
    ExclusiveAccount& theAccount,
    OTTransaction& processInbox,
    OTTransaction& processInboxResponse,
    Ledger& inbox,
    Ledger& outbox,
    bool& bOutSuccess)
{
    // The outgoing transaction is an "atProcessInbox", that is, "a reply to
    // the process inbox request"
    processInboxResponse.SetType(transactionType::atProcessInbox);

    std::shared_ptr<Item> pItem = nullptr;
    std::shared_ptr<Item> pBalanceItem =
        processInbox.GetItem(itemType::balanceStatement);
    std::shared_ptr<Item> pResponseItem = nullptr;
    std::shared_ptr<Item> pResponseBalanceItem = nullptr;

    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();

    // Grab the actual server ID from this object, and use it as the server
    // ID here.
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto& NOTARY_ID = context.Notary();
    const auto ACCOUNT_ID =
        server_.API().Factory().Internal().AccountID(theAccount.get());
    const UnallocatedCString strNymID(
        String::Factory(NYM_ID, api_.Crypto())->Get());
    UnallocatedSet<TransactionNumber> closedNumbers, closedCron;
    pResponseBalanceItem.reset(api_.Factory()
                                   .InternalSession()
                                   .Item(
                                       processInboxResponse,
                                       itemType::atBalanceStatement,
                                       identifier::Account{})
                                   .release());
    pResponseBalanceItem->SetStatus(Item::rejection);  // the default.
    // the Transaction's destructor will cleanup the item. It "owns" it now.
    processInboxResponse.AddItem(pResponseBalanceItem);

    bool bSuccessFindingAllTransactions{true};
    Amount lTotalBeingAccepted{0};
    UnallocatedList<TransactionNumber> theListOfInboxReceiptsBeingRemoved{};
    bool bVerifiedBalanceStatement{false};
    const bool allowed =
        NYM_IS_ALLOWED(strNymID, ServerSettings::_transact_process_inbox);

    auto accountHash = identifier::Account{};

    if (false == allowed) {
        LogError()(OT_PRETTY_CLASS())("User ")(
            strNymID)(" is not allowed to perform processInbox "
                      "requests.")
            .Flush();

        goto send_message;
    }

    if (nullptr == pBalanceItem) {
        LogError()(OT_PRETTY_CLASS())(
            "No Balance Agreement item found on this transaction.")
            .Flush();

        goto send_message;
    }

    pBalanceItem->SaveContractRaw(strBalanceItem);
    // the response item carries a copy of what it's responding to.
    pResponseBalanceItem->SetReferenceString(strBalanceItem);
    // This response item is IN RESPONSE to processInbox's balance agreement
    pResponseBalanceItem->SetReferenceToNum(pBalanceItem->GetTransactionNum());
    pResponseBalanceItem->SetNumberOfOrigin(*pBalanceItem);

    // This transaction accepts various incoming pending transfers. So when
    // it's all done, my balance will be higher. AND pending inbox items
    // will be removed from my inbox.
    //
    // I would like to not even process the whole giant loop below, if I can
    // verify here now that the balance agreement is wrong.
    //
    // Thus I will actually loop through the acceptPending items in
    // processInbox,
    // and then for each one, I'll lookup the ACTUAL transaction in the
    // inbox, and get its ACTUAL value. (And total them all up.)
    //
    // The total of those, (WITHOUT the user having to tell me what it will
    // be, since I'm looking them all up), should equal the difference in
    // the account balance! Meaning the current balance plus that total will
    // be the expected NEW balance, according to this balance agreement --
    // if it wants to be approved, that is.

    // To make sure each inbox item refers to a different number. (If two of
    // them refer to the same number, that's bad and is not allowed. You
    // can't process the same inbox item twice simultaneously! Or even at
    // all.)

    for (const auto& pProcessInboxItem : processInbox.GetItemList()) {
        if (false == bool(pProcessInboxItem)) {
            LogError()(OT_PRETTY_CLASS())("Invalid transaction").Flush();
            bSuccessFindingAllTransactions = false;

            break;
        }

        const auto& item = *pProcessInboxItem;
        std::shared_ptr<OTTransaction> pServerTransaction;

        switch (item.GetType()) {
            case itemType::balanceStatement: {
                continue;
            }
            case itemType::acceptCronReceipt:
            case itemType::acceptFinalReceipt:
            case itemType::acceptBasketReceipt:
            case itemType::disputeCronReceipt:
            case itemType::disputeFinalReceipt:
            case itemType::disputeBasketReceipt: {
                pServerTransaction =
                    inbox.GetTransaction(item.GetReferenceToNum());
            } break;
            // Accept an incoming (pending) transfer.
            case itemType::acceptPending:
            // Accept a chequeReceipt, voucherReceipt, or transferReceipt.
            case itemType::acceptItemReceipt:
            case itemType::rejectPending:
            case itemType::disputeItemReceipt: {
                pServerTransaction =
                    inbox.GetTransaction(item.GetReferenceToNum());
            } break;
            default: {
                auto strItemType = String::Factory();
                item.GetTypeString(strItemType);
                itemType nItemType = item.GetType();
                bSuccessFindingAllTransactions = false;
                LogError()(OT_PRETTY_CLASS())(" Wrong item type: ")(
                    strItemType->Exists() ? strItemType->Get() : "")("(")(
                    static_cast<std::int32_t>(nItemType))(")")
                    .Flush();
            } break;
        }

        if (false == bool(pServerTransaction)) {
            const auto strAccountID =
                String::Factory(ACCOUNT_ID, api_.Crypto());
            LogError()(OT_PRETTY_CLASS())(
                "Unable to find or process inbox transaction being accepted "
                "by user: ")(strNymID)(" for account: ")(strAccountID.get())
                .Flush();
            bSuccessFindingAllTransactions = false;
            break;
        } else if (
            pServerTransaction->GetReceiptAmount(reason_) != item.GetAmount()) {
            const auto& INSTRUMENT_DEFINITION_ID =
                theAccount.get().GetInstrumentDefinitionID();
            const auto unittype =
                api_.Wallet().Internal().CurrencyTypeBasedOnUnitType(
                    INSTRUMENT_DEFINITION_ID);
            LogError()(OT_PRETTY_CLASS())("Receipt amounts don't "
                                          "match: ")(
                pServerTransaction->GetReceiptAmount(reason_), unittype)(
                " and ")(item.GetAmount(), unittype)(". Nym ")(strNymID)
                .Flush();
            bSuccessFindingAllTransactions = false;
            break;
        }

        // BELOW THIS POINT, WE KNOW THAT pServerTransaction was FOUND (and
        // validated.)
        auto& serverTransaction = *pServerTransaction;
        const auto closingNum = serverTransaction.GetClosingNum();

        switch (item.GetType()) {
            case itemType::acceptCronReceipt: {
                bSuccessFindingAllTransactions = true;
            } break;
            case itemType::acceptFinalReceipt: {
                bSuccessFindingAllTransactions = true;

                // Need to ERROR OUT here, if the number of cron receipts
                // (related to this finalReceipt) in the inbox isn't equal
                // to the number being accepted in this processInbox
                // transaction. (You can't close the final receipt unless
                // you close all the others as well.)

                // IN THIS CASE: If user is accepting a finalReceipt, that
                // means all the OTHER receipts related to it (sharing the
                // same "in reference to") must ALSO be cleared from the
                // inbox along with it! That's the whole point of the
                // finalReceipt -- to make sure all related receipts are
                // cleared, when IT is.
                //
                // So let's see if the number of related receipts on this
                // process inbox (processInbox) matches the number of
                // related receipts in the actual inbox (pInbox), as found
                // by the finalReceipt's (pServerTransaction) "in reference
                // to" value, which should be the same as on the related
                // receipts.
                //
                // (Below) processInbox is the processInbox transaction.
                // Each item on it is "in ref to" a DIFFERENT receipt, even
                // though, if they are marketReceipts, all of THOSE receipts
                // are "in ref to" the original transaction#. I need to loop
                // through all items on processInbox (processInbox request)
                // For each, look it up on the inbox. (Each item will be "in
                // reference to" the original transaction.) ONCE THE INBOX
                // RECEIPT IS FOUND, if *IT* is "in reference to"
                // serverTransaction.GetReferenceToNum(), Then increment
                // the count for the transaction.  COMPARE *THAT* to
                // theInbox.GetCount and we're golden!!

                // we'll store them here, and disallow duplicates, to make
                // sure they are all unique IDs (no repeats.)
                UnallocatedSet<std::int64_t> setOfRefNumbers;

                for (auto& it : processInbox.GetItemList()) {
                    auto pItemPointer = it;
                    OT_ASSERT_MSG(
                        false != bool(pItemPointer),
                        "Pointer should not have been nullptr.");

                    auto pTransPointer =
                        inbox.GetTransaction(pItemPointer->GetReferenceToNum());

                    if ((false != bool(pTransPointer)) &&
                        (pTransPointer->GetReferenceToNum() ==
                         serverTransaction.GetReferenceToNum())) {
                        setOfRefNumbers.insert(
                            pItemPointer->GetReferenceToNum());
                    }
                }

                if (inbox.GetTransactionCountInRefTo(
                        serverTransaction.GetReferenceToNum()) !=
                    static_cast<std::int32_t>(setOfRefNumbers.size())) {
                    LogError()(OT_PRETTY_CLASS())(
                        "User tried to close a finalReceipt, without also "
                        "closing all related receipts. (Those that share the "
                        "IN REF TO number.)")
                        .Flush();
                    bSuccessFindingAllTransactions = false;
                    break;
                }
                // Upon success, these numbers will be removed from the
                // Nym's additional record of "cron item IDs".
                //
                // Server side stores a list of open cron items on each Nym.
                // The closing transaction number on the final receipt
                // SHOULD be on that list.
                //
                // If we FOUND it on the Nym, then we add it to the list to
                // be removed from Nym's open cron items. (If it wasn't
                // there before, then we wouldn't want to "re-add" it, now
                // would we?)
                const bool found = context.VerifyCronItem(closingNum);

                if (found) {
                    // Schedule to remove GetClosingNum() from server-side
                    // list of Nym's open cron items. (By adding it to
                    // closedCron.)
                    closedCron.insert(closingNum);
                } else {
                    LogError()(OT_PRETTY_CLASS())(
                        "expected to find closingNum "
                        "(")(closingNum)("Nym's (")(strNymID)(") "
                                                              "li"
                                                              "st"
                                                              " o"
                                                              "f "
                                                              "op"
                                                              "en"
                                                              " c"
                                                              "ro"
                                                              "n "
                                                              "it"
                                                              "em"
                                                              "s."
                                                              " ("
                                                              "Ma"
                                                              "yb"
                                                              "e "
                                                              "he"
                                                              " d"
                                                              "id"
                                                              "n'"
                                                              "t "
                                                              "se"
                                                              "e "
                                                              "th"
                                                              "e "
                                                              "no"
                                                              "ti"
                                                              "ce"
                                                              " i"
                                                              "n "
                                                              "hi"
                                                              "s "
                                                              "Ny"
                                                              "mb"
                                                              "ox"
                                                              " y"
                                                              "et"
                                                              "."
                                                              ")")
                        .Flush();
                }  // else error log.
                [[fallthrough]];
            }
            // ---- COUNT is correct and closing num is on list of open cron
            // items. (FINAL RECEIPT FALLS THROUGH HERE!!! no break)
            case itemType::acceptBasketReceipt: {
                // IF it's actually there on theNym, then schedule it for
                // removal. (Otherwise we'd end up improperly re-adding it.)
                const bool verified = context.VerifyIssuedNumber(closingNum);

                if (verified) {
                    closedNumbers.insert(closingNum);
                    LogDetail()(OT_PRETTY_CLASS())(
                        "Closing "
                        "acceptBasketReceipt or acceptFinalReceipt "
                        "number ")(closingNum)
                        .Flush();
                } else {
                    bSuccessFindingAllTransactions = false;
                    LogError()(OT_PRETTY_CLASS())(
                        "basket or final receipt, trying to 'remove' an "
                        "issued number (")(closingNum)(") that already wasn't "
                                                       "on Nym's issued list. "
                                                       "(So what "
                                                       "is this in the inbox, "
                                                       "then?)")
                        .Flush();
                }

            } break;
            case itemType::acceptPending: {
                // IF I'm accepting a pending transfer, then add the amount
                // to my counter of total amount being accepted.
                lTotalBeingAccepted +=
                    serverTransaction.GetReceiptAmount(reason_);
                bSuccessFindingAllTransactions = true;
            } break;
            case itemType::acceptItemReceipt: {
                bSuccessFindingAllTransactions = true;
                // If I'm accepting an item receipt (which will remove my
                // responsibility for that item) then add it to the temp Nym
                // (which is a list of transaction numbers that will be
                // removed from my responsibility if all is successful.)
                // Also remove all the Temp Nym numbers from theNym, so we
                // can verify the Balance Statement AS IF they were already
                // removed.
                //
                // What number do I remove here? the user is accepting a
                // transfer receipt, which is in reference to the
                // recipient's acceptPending. THAT item is in reference to
                // my original transfer (or contains a cheque with my
                // original number.) (THAT's the # I need.)
                auto strOriginalItem = String::Factory();
                serverTransaction.GetReferenceString(strOriginalItem);

                auto pOriginalItem{api_.Factory().InternalSession().Item(
                    strOriginalItem,
                    NOTARY_ID,
                    serverTransaction.GetReferenceToNum())};

                if (false != bool(pOriginalItem)) {
                    // If pOriginalItem is acceptPending, that means the
                    // client is accepting the transfer receipt from the
                    // server, (from his inbox), which has the recipient's
                    // acceptance inside of the client's transfer as the
                    // original item. This means the transfer that the
                    // client originally sent is now finally closed!
                    //
                    // If it's a depositCheque, that means the client is
                    // accepting the cheque receipt from the server, (from
                    // his inbox) which has the recipient's deposit inside
                    // of it as the original item. This means that the
                    // cheque that the client originally wrote is now
                    // finally closed!
                    //
                    // In both cases, the "original item" itself is not from
                    // the client, but from the recipient! Therefore, the
                    // number on that item is useless for removing numbers
                    // from the client's list of issued numbers. Rather, I
                    // need to load that original cheque, or pending
                    // transfer, from WITHIN the original item, in order to
                    // get THAT number, to remove it from the client's
                    // issued list. (Whether for real, or for setting up
                    // dummy data in order to verify the balance agreement.)
                    // *sigh*

                    // client is accepting a cheque receipt, which has a
                    // depositCheque (from the recipient) as the original
                    // item within.
                    if (itemType::depositCheque == pOriginalItem->GetType()) {
                        // Get the cheque from the Item and load it up into
                        // a Cheque object.
                        auto strCheque = String::Factory();
                        pOriginalItem->GetAttachment(strCheque);
                        auto theCheque{
                            api_.Factory().InternalSession().Cheque()};

                        OT_ASSERT(false != bool(theCheque));

                        if (false ==
                            ((strCheque->GetLength() > 2) &&
                             theCheque->LoadContractFromString(strCheque))) {
                            LogError()(OT_PRETTY_CLASS())(
                                "ERROR loading cheque from "
                                "string: ")(strCheque.get())
                                .Flush();
                            bSuccessFindingAllTransactions = false;
                        }
                        // Since the client wrote the cheque, and he is now
                        // accepting the cheque receipt, he can be cleared
                        // for that transaction number...
                        else {
                            const auto number = theCheque->GetTransactionNum();
                            // IF it's actually there on theNym, then
                            // schedule it for removal. (Otherwise we'd end
                            // up improperly re-adding it.)
                            const bool verified =
                                context.VerifyIssuedNumber(number);

                            if (verified) {
                                closedNumbers.insert(number);
                                LogDetail()(OT_PRETTY_CLASS())(
                                    "Closing "
                                    "depositCheque number ")(number)
                                    .Flush();
                            } else {
                                bSuccessFindingAllTransactions = false;
                                LogError()(OT_PRETTY_CLASS())(
                                    "cheque receipt, trying to 'remove' an "
                                    "issued number (")(number)(") that already "
                                                               "wasn't on "
                                                               "Nym's issued "
                                                               "list. (So what "
                                                               "is this in the "
                                                               "inbox, "
                                                               "then?)")
                                    .Flush();
                            }
                        }
                    }
                    // client is accepting a transfer receipt, which has
                    // an acceptPending from the recipient as the original
                    // item within, (which is in reference to the client's
                    // outoing original transfer.)
                    else if (
                        itemType::acceptPending == pOriginalItem->GetType()) {
                        const auto number = pOriginalItem->GetNumberOfOrigin();
                        // IF it's actually there on theNym, then schedule
                        // it for removal. (Otherwise we'd end up improperly
                        // re-adding it.)
                        const bool verified =
                            context.VerifyIssuedNumber(number);

                        if (verified) {
                            closedNumbers.insert(number);
                            LogDetail()(OT_PRETTY_CLASS())(
                                "Closing "
                                "acceptPending number ")(number)
                                .Flush();
                        } else {
                            bSuccessFindingAllTransactions = false;
                            LogError()(OT_PRETTY_CLASS())(
                                "transfer receipt, trying to 'remove' an "
                                "issued number (")(
                                pOriginalItem->GetReferenceToNum())(
                                ") that already wasn't on Nym's issued list. "
                                "(So what is this in the inbox, then?)")
                                .Flush();
                        }
                    } else {
                        auto strOriginalItemType = String::Factory();
                        pOriginalItem->GetTypeString(strOriginalItemType);
                        LogError()(OT_PRETTY_CLASS())(
                            "Original item has wrong type, while accepting "
                            "item receipt: ")(strOriginalItemType.get())
                            .Flush();
                        bSuccessFindingAllTransactions = false;
                    }
                } else {
                    LogError()(OT_PRETTY_CLASS())(
                        "Unable to load original item from string while "
                        "accepting item receipt: ")(strOriginalItem.get())
                        .Flush();
                    bSuccessFindingAllTransactions = false;
                }
            } break;
            default:
                LogError()(OT_PRETTY_CLASS())(
                    "Wrong item type in "
                    "Notary::NotarizeProcessInbox. (2nd notice).")
                    .Flush();
                bSuccessFindingAllTransactions = false;
                break;
        }

        // I'll also go ahead and remove each transaction from pInbox, and
        // pass said inbox into the VerifyBalanceAgreement call... (So it
        // can simulate as if the inbox was already processed, and the total
        // is already calculated, and if it succeeds, then we can allow the
        // giant loop below to do it all for real.) (I'm not saving this
        // copy of the inbox anyway--there's another one below.)
        if (bSuccessFindingAllTransactions) {
            // WE'RE REMOVING THE TRANSACTIONS FROM AN INBOX COPY, IN ORDER
            // TO VERIFY THE BALANCE AGREEMENT (WITH THAT INBOX COPY SET UP
            // AS THOUGH THE TRANSACTION HAD ALREADY BEEN A SUCCESS.) I'm
            // not ACTUALLY removing though, until AFTER the loop (in case
            // the rest of the loop needs the data still, in that inbox.) So
            // we save in a list, and remove AFTER the loop.
            theListOfInboxReceiptsBeingRemoved.push_back(
                serverTransaction.GetTransactionNum());
        }
        // If there was an error above, then we don't want to keep looping.
        // We want the below error block.
        else {
            break;
        }
    }

    if (false == bSuccessFindingAllTransactions) {
        LogError()(OT_PRETTY_CLASS())("Transactions in processInbox "
                                      "message do not match actual inbox.")
            .Flush();

        goto send_message;
    }

    // Remove certain receipts (determined in the big loop above) from the
    // inbox copy, to see if it will verify in the balance agreement.
    while (!theListOfInboxReceiptsBeingRemoved.empty()) {
        std::int64_t lTemp = theListOfInboxReceiptsBeingRemoved.front();
        theListOfInboxReceiptsBeingRemoved.pop_front();

        // Notice I don't call DeleteBoxReceipt(lTemp) here like I
        // normally would when calling RemoveTransaction(lTemp), since
        // this is only a copy of my inbox and not the real thing.
        if (false == inbox.RemoveTransaction(lTemp)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed removing receipt from Inbox "
                "copy: ")(lTemp)("Meaning the client probably "
                                 "has an old copy of his inbox. "
                                 "We "
                                 "don't even see the receipt "
                                 "that he still thinks he has.")
                .Flush();
        }
    }

    // FINALLY after all that setup, we can do the balance agreement!!
    bVerifiedBalanceStatement = pBalanceItem->VerifyBalanceStatement(
        lTotalBeingAccepted,
        context,
        inbox,
        outbox,
        theAccount.get(),
        processInbox,
        closedNumbers,
        reason_);

    if (false == bVerifiedBalanceStatement) {
        LogError()(OT_PRETTY_CLASS())(
            "Error validating balance statement for "
            "transaction ")(processInbox.GetTransactionNum())(".")
            .Flush();

        goto send_message;
    }

    // BALANCE AGREEMENT WAS SUCCESSFUL.......
    pResponseBalanceItem->SetStatus(Item::acknowledgement);

    // THE ABOVE LOOP WAS JUST A TEST RUN (TO VERIFY BALANCE
    // AGREEMENT BEFORE WE BOTHERED TO RUN THIS LOOP BELOW...)

    // loop through the items that make up the incoming transaction
    for (auto& pProcessInboxItem : processInbox.GetItemList()) {
        OT_ASSERT(nullptr != pProcessInboxItem);

        // We already handled this one (if we're even in this block
        // in the first place.)
        if (itemType::balanceStatement == pProcessInboxItem->GetType()) {
            continue;
        }

        // If the client sent an accept item, (or reject/dispute)
        // then let's process it.
        const bool validType =
            (Item::request == pProcessInboxItem->GetStatus()) &&
            ((itemType::acceptCronReceipt ==
              pProcessInboxItem->GetType()) ||  // Accepting notice of market
                                                // trade or payment processing.
                                                // (Original in Cron Receipt.)
             //                       (OTitemType::disputeCronReceipt
             (itemType::acceptItemReceipt ==
              pProcessInboxItem->GetType()) ||  // Accepted
                                                // item
                                                // receipt
                                                // (cheque,
                                                // transfer)
             (itemType::acceptPending ==
              pProcessInboxItem->GetType()) ||  // Accepting
                                                // notice of
                                                // pending
                                                // transfer
             (itemType::acceptFinalReceipt ==
              pProcessInboxItem->GetType()) ||  // Accepting
                                                // finalReceipt
             (itemType::acceptBasketReceipt ==
              pProcessInboxItem->GetType())  // Accepting
                                             // basketReceipt
            );

        if (false == validType) {
            auto strItemType = String::Factory();
            pProcessInboxItem->GetTypeString(strItemType);
            LogError()(OT_PRETTY_CLASS())("Error, unexpected Item::itemType: ")(
                strItemType.get())
                .Flush();

            continue;
        }

        // The response item will contain a copy of the "accept"
        // request.
        // So I'm just setting aside a copy now for those
        // purposes later.
        strInReferenceTo->Release();
        pProcessInboxItem->SaveContractRaw(strInReferenceTo);

        itemType theReplyItemType;
        switch (pProcessInboxItem->GetType()) {
            case itemType::acceptPending:
                theReplyItemType = itemType::atAcceptPending;
                break;
            case itemType::rejectPending:
                theReplyItemType = itemType::atRejectPending;
                break;
            case itemType::acceptCronReceipt:
                theReplyItemType = itemType::atAcceptCronReceipt;
                break;
            case itemType::disputeCronReceipt:
                theReplyItemType = itemType::atDisputeCronReceipt;
                break;
            case itemType::acceptItemReceipt:
                theReplyItemType = itemType::atAcceptItemReceipt;
                break;
            case itemType::disputeItemReceipt:
                theReplyItemType = itemType::atDisputeItemReceipt;
                break;
            case itemType::acceptFinalReceipt:
                theReplyItemType = itemType::atAcceptFinalReceipt;
                break;
            case itemType::disputeFinalReceipt:
                theReplyItemType = itemType::atDisputeFinalReceipt;
                break;
            case itemType::acceptBasketReceipt:
                theReplyItemType = itemType::atAcceptBasketReceipt;
                break;
            case itemType::disputeBasketReceipt:
                theReplyItemType = itemType::atDisputeBasketReceipt;
                break;
            default:
                LogError()(OT_PRETTY_CLASS())("Should never happen.").Flush();
                theReplyItemType =
                    itemType::error_state;  // should never happen
                                            // based on above 'if'
                                            // statement.
                break;  // saving this anyway just cause it's
                        // cleaner.
        }

        // Server response item being added to server response
        // transaction (processInboxResponse)
        // They're getting SOME sort of response item.

        pResponseItem.reset(api_.Factory()
                                .InternalSession()
                                .Item(
                                    processInboxResponse,
                                    theReplyItemType,
                                    identifier::Account{})
                                .release());
        pResponseItem->SetStatus(Item::rejection);  // the default.
        pResponseItem->SetReferenceString(
            strInReferenceTo);  // the response item carries a
                                // copy of what it's responding
                                // to.
        pResponseItem->SetReferenceToNum(
            pProcessInboxItem->GetTransactionNum());
        pResponseItem->SetNumberOfOrigin(*pProcessInboxItem);

        processInboxResponse.AddItem(pResponseItem);  // the Transaction's
                                                      // destructor will
        // cleanup the item. It
        // "owns" it now.

        // Need to load the Inbox first, in order to look up the
        // transaction that
        // the client is accepting. This is possible because the
        // client has included
        // the transaction number.  I'll just look it up in his
        // inbox and then
        // process it.
        // theAcctID is the ID on the client Account that was
        // passed in.
        auto theInbox{api_.Factory().InternalSession().Ledger(
            NYM_ID, ACCOUNT_ID, NOTARY_ID)};

        OT_ASSERT(false != bool(theInbox));

        std::shared_ptr<OTTransaction> pServerTransaction = nullptr;

        if (!theInbox->LoadInbox()) {
            LogError()(OT_PRETTY_CLASS())(
                "Error loading inbox during processInbox.")
                .Flush();
        } else if (false == theInbox->VerifyAccount(server_.GetServerNym())) {
            LogError()(OT_PRETTY_CLASS())(
                "Error verifying inbox during processInbox.")
                .Flush();
        }
        //
        // Warning! In the case of a
        // transactionType::paymentReceipt or
        // transactionType::marketReceipt,
        // the "in reference to" string will NOT contain an
        // Item at all, but an OTPaymentPlan or
        // an OTTrade!! Also, a paymentReceipt might be for a
        // smart contract, in which case there's
        // a smartcontract inside, instead of a payment plan! I
        // handle these cases first, here:
        else if (  // MARKET RECEIPT, or PAYMENT RECEIPT.....
            ((itemType::acceptCronReceipt ==
              pProcessInboxItem->GetType())  // This is checked
                                             // above, but just
             // keeping this safe.
             )  // especially in case this block moves
            // or is used elsewhere.
            && (nullptr != (pServerTransaction = theInbox->GetTransaction(
                                pProcessInboxItem->GetReferenceToNum()))) &&
            ((transactionType::paymentReceipt ==
              pServerTransaction->GetType()) ||
             (transactionType::marketReceipt ==
              pServerTransaction->GetType()))) {
            // pItem contains the current user's attempt to
            // accept the Receipt
            // represented by pServerTransaction. Therefore we
            // have the user's
            // item AND the receipt he is trying to accept.

            pServerTransaction->DeleteBoxReceipt(*theInbox);  // faster.
            theInbox->RemoveTransaction(
                pServerTransaction->GetTransactionNum());

            theInbox->ReleaseSignatures();
            theInbox->SignContract(server_.GetServerNym(), reason_);
            theInbox->SaveContract();
            theAccount.get().SaveInbox(*theInbox);

            // Now we can set the response item as an
            // acknowledgement instead of the default
            // (rejection)
            pResponseItem->SetStatus(Item::acknowledgement);
        } else if (  // FINAL RECEIPT
            ((itemType::acceptFinalReceipt ==
              pProcessInboxItem->GetType())  // This is checked
                                             // above, but just
             // keeping this safe.
             )  // especially in case this block moves
            // or is used elsewhere.
            && (nullptr != (pServerTransaction = theInbox->GetTransaction(
                                pProcessInboxItem->GetReferenceToNum()))) &&
            ((transactionType::finalReceipt ==
              pServerTransaction->GetType()))) {
            // pItem contains the current user's attempt to
            // accept the Receipt
            // represented by pServerTransaction. Therefore we
            // have the user's
            // item AND the receipt he is trying to accept.

            pServerTransaction->DeleteBoxReceipt(*theInbox);  // faster.
            theInbox->RemoveTransaction(
                pServerTransaction->GetTransactionNum());

            theInbox->ReleaseSignatures();
            theInbox->SignContract(server_.GetServerNym(), reason_);
            theInbox->SaveContract();
            theAccount.get().SaveInbox(*theInbox);

            // Now we can set the response item as an
            // acknowledgement instead of the default
            // (rejection)
            pResponseItem->SetStatus(Item::acknowledgement);
        } else if (  // BASKET RECEIPT
            ((itemType::acceptBasketReceipt ==
              pProcessInboxItem->GetType())  // This is checked
                                             // above, but just
             // keeping this safe.
             )  // especially in case this block moves
            // or is used elsewhere.
            && (nullptr != (pServerTransaction = theInbox->GetTransaction(
                                pProcessInboxItem->GetReferenceToNum()))) &&
            ((transactionType::basketReceipt ==
              pServerTransaction->GetType()))) {
            // pItem contains the current user's attempt to
            // accept the Receipt
            // represented by pServerTransaction. Therefore we
            // have the user's
            // item AND the receipt he is trying to accept.

            pServerTransaction->DeleteBoxReceipt(*theInbox);  // faster.
            theInbox->RemoveTransaction(
                pServerTransaction->GetTransactionNum());

            theInbox->ReleaseSignatures();
            theInbox->SignContract(server_.GetServerNym(), reason_);
            theInbox->SaveContract();
            theAccount.get().SaveInbox(*theInbox);

            // Now we can set the response item as an
            // acknowledgement instead of the default
            // (rejection)
            pResponseItem->SetStatus(Item::acknowledgement);
        }

        // Careful here.  I'm looking up the original
        // transaction number (1, say) which is stored
        // in my inbox as a "in reference to" on transaction
        // number 41. (Which is a pending transaction
        // or receipt
        // that the server created in my inbox, and only REFERS
        // to the original transaction, but is not
        // the original transaction in and of itself.)
        //
        // In other words, in this case below, I am looking for
        // the transaction in the Inbox
        // that REFERS to the same transaction that the accept
        // item REFERS to. That process, necessary
        // for pending transactions and cheque receipts, is NOT
        // the case above, with receipts from cron.
        else if (
            ((itemType::acceptItemReceipt ==
              pProcessInboxItem->GetType())  // acceptItemReceipt
                                             // includes
                                             // checkReceipt and
                                             // transferReceipts.
             || (itemType::acceptPending ==
                 pProcessInboxItem->GetType())  // acceptPending
                                                // includes
                                                // checkReceipts.
                                                // Because they
                                                // are
             ) &&
            (nullptr != (pServerTransaction = theInbox->GetTransaction(
                             pProcessInboxItem->GetReferenceToNum()))) &&
            ((transactionType::pending ==
              pServerTransaction->GetType()) ||  // pending
                                                 // transfer.
             (transactionType::transferReceipt ==
              pServerTransaction->GetType()) ||  // transfer
                                                 // receipt.
             (transactionType::voucherReceipt ==
              pServerTransaction->GetType()) ||  // voucher
                                                 // receipt.
             (transactionType::chequeReceipt ==
              pServerTransaction->GetType())  // cheque
                                              // receipt is
                                              // down here
                                              // in the
                                              // pending
                                              // section,
             )  // because this is where an Item is loaded
            // up (since it
            )  // originated with a deposit transaction, not
               // a cron receipt.)
        {
            // The accept item will come with the transaction
            // number that
            // it's referring to. So we'll just look up that
            // transaction
            // in the inbox, and now that it's been accepted,
            // we'll process it.

            // At this point, pItem points to the client's
            // attempt to accept pServerTransaction
            // and pServerTransaction is the server's created
            // transaction in my inbox that contains
            // the original item (from the sender) as the
            // "referenced to" object. So let's extract
            // it.
            auto strOriginalItem = String::Factory();
            pServerTransaction->GetReferenceString(strOriginalItem);

            auto pOriginalItem{api_.Factory().InternalSession().Item(
                strOriginalItem,
                NOTARY_ID,
                pServerTransaction->GetReferenceToNum())};

            if (false != bool(pOriginalItem)) {

                // What are we doing in this code?
                //
                // I need to accept various items that are
                // sitting in my inbox, such as:
                //
                // -- transfers waiting to be accepted (or
                // rejected.)
                //
                // -- cheque deposit receipts waiting to be
                // accepted (they cannot be rejected.)
                //
                // -- transfer receipts waiting to be accepted
                // (they cannot be rejected.)

                //
                // ONLY in the case of pending transfers also do
                // I need to mess around with my account,
                // and the sender's inbox and outbox. In the
                // other cases, I merely need to remove
                // the item from my inbox.
                // Although when 'accepting the reject', I do
                // need to take the money back into
                // my inbox...

                // The depositCheque request Item is saved as
                // a "in reference to" field
                // on the inbox chequeReceipt transaction.

                // Therefore, if I am processing an
                // acceptPending item from the client,
                // for accepting a chequeReceipt Transaction
                // that's in his inbox, and
                // the original item (that the receipt is for)
                // is a depositCheque,
                // then I can go ahead and clear it from his
                // inbox.

                // The below block only executes for ACCEPTING a
                // CHEQUE deposit receipt, or
                // for 'Accepting an ACCEPT.'
                //
                // I can't 'Accept a REJECT' without also
                // transferring the rejected money back into
                // my own account. And that means fiddling with
                // my account, and that means it will
                // be in a different block of code than this
                // one.
                //
                // Whereas with accepting a cheque deposit
                // receipt, or accepting an accepted transfer
                // notice,
                // in both of those cases, my account balance
                // doesn't change at all. I just need to accept
                // those notices in order to get them out of my
                // inbox. So that's the simplest case, and it's
                // handled by THIS block of code:
                //
                if ((itemType::acceptItemReceipt ==
                     pProcessInboxItem->GetType()) &&
                    (((transactionType::transferReceipt ==
                       pServerTransaction->GetType()) &&
                      (itemType::acceptPending == pOriginalItem->GetType())) ||
                     (((transactionType::chequeReceipt ==
                        pServerTransaction->GetType()) ||
                       (transactionType::voucherReceipt ==
                        pServerTransaction->GetType())) &&
                      (itemType::depositCheque ==
                       pOriginalItem->GetType())))) {  // (The
                                                       // funds
                                                       // are
                    // already
                    // paid
                    // out...)
                    // pItem contains the current user's attempt
                    // to accept the
                    // ['depositCheque' OR 'acceptPending']
                    // located in theOriginalItem.
                    // Now we have the user's item and the item
                    // he is trying to accept.

                    pServerTransaction->DeleteBoxReceipt(*theInbox);  // faster.
                    theInbox->RemoveTransaction(
                        pServerTransaction->GetTransactionNum());
                    theInbox->ReleaseSignatures();
                    theInbox->SignContract(server_.GetServerNym(), reason_);
                    theInbox->SaveContract();
                    theAccount.get().SaveInbox(*theInbox);

                    // Now we can set the response item as an
                    // acknowledgement instead of the default
                    // (rejection)
                    pResponseItem->SetStatus(Item::acknowledgement);

                    // Don't I need to remove from
                    // responsibility list?
                    // No, because that is done at the bottom of
                    // the function.
                    //
                }  // its type is Item::acceptPending or
                // Item::depositCheque

                // TODO: 'Accept a REJECT' -- NEED TO PERFORM
                // THE TRANSFER OF FUNDS BACK TO THE SENDER'S
                // ACCOUNT WHEN TRANSFER IS REJECTED.

                // The below block only executes for ACCEPTING a
                // TRANSFER
                else if (
                    (transactionType::pending ==
                     pServerTransaction->GetType()) &&
                    (itemType::transfer == pOriginalItem->GetType())) {
                    // pItem contains the current user's attempt
                    // to accept the transfer located in
                    // theOriginalItem.
                    // Now we have both items.
                    const auto& IDFromAccount =
                        pOriginalItem->GetPurportedAccountID();
                    const auto& IDToAccount =
                        pOriginalItem->GetDestinationAcctID();

                    // I'm using the operator== because it
                    // exists.
                    // If the ID on the "To" account from the
                    // original transaction does not
                    // match the Acct ID of the client trying to
                    // accept the transaction...
                    if (!(ACCOUNT_ID == IDToAccount)) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Error: Destination account ID on "
                            "the transaction does not match "
                            "account ID of client transaction "
                            "item.")
                            .Flush();
                    }

                    // The 'from' outbox is loaded to remove the
                    // outgoing transfer, since it has been
                    // accepted.
                    // The 'from' inbox is loaded in order to
                    // put a notice of this acceptance for the
                    // sender's records.
                    auto theFromOutbox{api_.Factory().InternalSession().Ledger(
                        IDFromAccount, NOTARY_ID)};  // Sender's
                                                     // *OUTBOX*
                    auto theFromInbox{api_.Factory().InternalSession().Ledger(
                        IDFromAccount, NOTARY_ID)};  // Sender's
                                                     // *INBOX*

                    OT_ASSERT(false != bool(theFromOutbox));
                    OT_ASSERT(false != bool(theFromInbox));

                    bool bSuccessLoadingInbox = theFromInbox->LoadInbox();
                    bool bSuccessLoadingOutbox = theFromOutbox->LoadOutbox();

                    // THE FROM INBOX -- We are adding an item
                    // here (acceptance of transfer),
                    // so we will create this inbox if we have
                    // to, so we can add that record to it.

                    if (true == bSuccessLoadingInbox) {
                        bSuccessLoadingInbox =
                            theFromInbox->VerifyAccount(server_.GetServerNym());
                    } else {
                        LogError()(OT_PRETTY_CLASS())("ERROR missing 'from' "
                                                      "inbox in "
                                                      "Notary::"
                                                      "NotarizeProcessInbox.")
                            .Flush();
                    }
                    // THE FROM OUTBOX -- We are removing an
                    // item, so this outbox SHOULD already
                    // exist.

                    if (true == bSuccessLoadingOutbox) {
                        bSuccessLoadingOutbox = theFromOutbox->VerifyAccount(
                            server_.GetServerNym());
                    } else {  // If it does not already exist, that
                        // is an error condition. For now, log
                        // and fail.
                        LogError()(OT_PRETTY_CLASS())("ERROR missing 'from' "
                                                      "outbox in "
                                                      "Notary::"
                                                      "NotarizeProcessInbox.")
                            .Flush();
                    }
                    if (!bSuccessLoadingInbox ||
                        false == bSuccessLoadingOutbox) {
                        LogError()(OT_PRETTY_CLASS())("ERROR loading 'from' "
                                                      "inbox or outbox in "
                                                      "Notary::"
                                                      "NotarizeProcessInbox.")
                            .Flush();
                    } else {
                        // Generate a new transaction number for
                        // the sender's inbox (to notice him of
                        // acceptance.)
                        std::int64_t lNewTransactionNumber = 0;
                        server_.GetTransactor().issueNextTransactionNumber(
                            lNewTransactionNumber);

                        // Generate a new transaction... (to
                        // notice the sender of acceptance.)
                        auto pInboxTransaction{
                            api_.Factory().InternalSession().Transaction(
                                *theFromInbox,
                                transactionType::transferReceipt,
                                originType::not_applicable,
                                lNewTransactionNumber)};

                        OT_ASSERT(false != bool(pInboxTransaction));

                        // Here we give the sender (by dropping
                        // into his inbox) a copy of my
                        // acceptItem (for
                        // his transfer), including the
                        // transaction number of my acceptance
                        // of his transfer.
                        //
                        pInboxTransaction->SetReferenceString(strInReferenceTo);
                        pInboxTransaction->SetReferenceToNum(
                            pProcessInboxItem->GetTransactionNum());  // Right
                                                                      // now
                                                                      // this
                                                                      // has
                                                                      // the
                        // 'accept
                        // the
                        // transfer'
                        // transaction
                        // number.
                        // It could be changed to the original
                        // transaction number, as a better
                        // receipt for the original sender.
                        // TODO? Decisions....

                        pInboxTransaction->SetNumberOfOrigin(
                            *pProcessInboxItem);

                        // Now we have created a new transaction
                        // from the server to the sender's inbox
                        // Let's sign it and add to his inbox.
                        pInboxTransaction->ReleaseSignatures();
                        pInboxTransaction->SignContract(
                            server_.GetServerNym(), reason_);
                        pInboxTransaction->SaveContract();

                        // At this point I have theInbox ledger,
                        // theFromOutbox ledger, theFromINBOX
                        // ledger,
                        // and theAccount.get().  So I should remove
                        // the appropriate item from each
                        // ledger, and
                        // add the acceptance to the sender's
                        // inbox, and credit the account....

                        // First try to credit the amount to the
                        // account...
                        if (theAccount.get().Credit(
                                pOriginalItem->GetAmount())) {
                            // Add a transfer receipt to the
                            // sender's inbox, containing the
                            // "accept" transaction as the ref
                            // string.
                            // (to notify him that his transfer
                            // was accepted; once he accepts it,
                            // the trans #can be removed from
                            // his issued list.)
                            //
                            std::shared_ptr<OTTransaction> inboxTransaction{
                                pInboxTransaction.release()};
                            theFromInbox->AddTransaction(inboxTransaction);

                            // The original item carries the
                            // transaction number that the
                            // original
                            // sender used to generate the
                            // transfer in the first place. This
                            // is the number
                            // by which that transaction is
                            // available in the sender's outbox.
                            //
                            // Then ANOTHER transaction was
                            // created, by the server, in order
                            // to put
                            // a pending transfer into the
                            // recipient's inbox. This has its
                            // own transaction
                            // number, generated by the server
                            // at that time.
                            //
                            // So we remove the original
                            // transfer from the sender's outbox
                            // using the
                            // transaction number on the
                            // original item, and we remove the
                            // pending transfer
                            // from the recipient's inbox using
                            // the transaction number from the
                            // pending
                            // transaction.

                            // UPDATE: These two transactions
                            // correspond to each other, so I am
                            // now creating
                            // them with the same transaction
                            // number. As you can see, this
                            // makes them easy
                            // to remove as well.

                            pServerTransaction->DeleteBoxReceipt(
                                *theFromOutbox);  // faster.
                            theFromOutbox->RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            pServerTransaction->DeleteBoxReceipt(
                                *theInbox);  // faster.
                            theInbox->RemoveTransaction(
                                pServerTransaction->GetTransactionNum());

                            // NOTICE BTW, warning: Notice that
                            // the box receipts are marked for
                            // deletion
                            // the instant they are removed from
                            // their respective boxes.
                            // Meanwhile, the client
                            // may not have actually DOWNLOADED
                            // those box receipts. Once they are
                            // ACTUALLY
                            // deleted, then client will never
                            // have the chance. It's assumed
                            // that client doesn't
                            // care, since the receipts are
                            // already out of his box.

                            theFromInbox->ReleaseSignatures();
                            theFromOutbox->ReleaseSignatures();

                            theFromInbox->SignContract(
                                server_.GetServerNym(), reason_);
                            theFromOutbox->SignContract(
                                server_.GetServerNym(), reason_);

                            theFromInbox->SaveContract();
                            theFromOutbox->SaveContract();

                            theFromInbox->SaveInbox();
                            theFromOutbox->SaveOutbox();

                            // Release any signatures that were
                            // there before (Old ones won't
                            // verify anymore anyway, since the
                            // content has changed.)
                            theInbox->ReleaseSignatures();
                            theInbox->SignContract(
                                server_.GetServerNym(), reason_);
                            theInbox->SaveContract();
                            theAccount.get().SaveInbox(*theInbox);

                            // Now we can set the response item
                            // as an acknowledgement instead of
                            // the default (rejection)
                            // otherwise, if we never entered
                            // this block, then it would still
                            // be set to rejection, and the
                            // new items would never have been
                            // added to the inbox/outboxes, and
                            // those files, along with
                            // the account file, would never
                            // have had their signatures
                            // released, or been re-signed or
                            // re-saved back to file.  The debit
                            // failed, so all of those other
                            // actions would fail also.
                            // BUT... if the message comes back
                            // with acknowledgement--then all of
                            // these actions must have
                            // happened, and here is the
                            // server's signature to prove it.
                            // Otherwise you get no items and no
                            // signature. Just a rejection item
                            // in the response transaction.
                            pResponseItem->SetStatus(Item::acknowledgement);

                            // This goes with the call above to
                            // theFromInbox->AddTransaction().
                            // Adding a receipt to any box, for
                            // real, requires saving the box
                            // receipt
                            // as well. (Which is stored in a
                            // separate file.)
                            //
                            inboxTransaction->SaveBoxReceipt(*theFromInbox);
                        } else {
                            theAccount.get().GetIdentifier(accountHash);
                            theAccount.Abort();
                            LogError()(OT_PRETTY_CLASS())(
                                "Unable to credit account in "
                                "Notary::"
                                "NotarizeProcessInbox.")
                                .Flush();
                        }
                    }  // outbox was successfully loaded
                }      // its type is Item::transfer
            }          // loaded original item from string
            else {
                LogError()(OT_PRETTY_CLASS())(
                    "Error loading original item from "
                    "inbox transaction.")
                    .Flush();
            }
        } else {
            LogError()(OT_PRETTY_CLASS())(
                "Error finding original receipt or transfer that client is "
                "trying to accept: ")(pProcessInboxItem->GetReferenceToNum())
                .Flush();
        }

        // sign the response item before sending it back (it's
        // already been added to the transaction above)
        // Now, whether it was rejection or acknowledgement, it
        // is set properly and it is signed, and it
        // is owned by the transaction, who will take it from
        // here.
        pResponseItem->SignContract(server_.GetServerNym(), reason_);
        pResponseItem->SaveContract();
    }  // for LOOP (each item)

    // For the reply message.
    if (accountHash.empty() && theAccount) {
        theAccount.get().GetIdentifier(accountHash);
    }
    AddHashesToTransaction(processInboxResponse, inbox, outbox, accountHash);

send_message:
    theAccount.Release();
    // I put this here so it's signed/saved whether the balance agreement
    // itself was successful OR NOT. (Or whether it even existed or not.)
    pResponseBalanceItem->ReleaseSignatures();
    pResponseBalanceItem->SignContract(server_.GetServerNym(), reason_);
    pResponseBalanceItem->SaveContract();
    processInboxResponse.ReleaseSignatures();
    processInboxResponse.SignContract(server_.GetServerNym(), reason_);
    processInboxResponse.SaveContract();
    // SAVE THE RECEIPT TO LOCAL STORAGE (for dispute resolution.)
    auto strPath = String::Factory();

    // On the server side, response will only have chance to succeed if
    // balance agreement succeeds first. Therefore, you will never see
    // successful response but failed balance, since it would stop at the
    // balance and response itself would remain failed with no chance of
    // changing.
    //
    // Thus, "success" must be when balance succeeded and transaction
    // succeeded, and "failure" must be when balance succeeded but
    // transaction failed.
    //
    // If NEITHER succeeded, then there is no point recording it to a file,
    // now is there?
    const auto strAcctID = String::Factory(ACCOUNT_ID, api_.Crypto());

    if (processInboxResponse.GetSuccess()) {
        // Balance agreement was a success, AND process inbox was a success.
        // Therefore, remove any relevant issued numbers from theNym (those
        // he's now officially no longer responsible for), and save.
        for (const auto& number : closedNumbers) {
            context.ConsumeIssued(number);
        }

        // The Nym (server side) stores a list of all opening and closing
        // cron #s. So when the number is released from the Nym, we also
        // take it off that list.
        for (const auto& number : closedCron) { context.CloseCronItem(number); }

        bOutSuccess = true;  // the processInbox was successful.
        strPath->Set(api::Legacy::GetFilenameSuccess(strAcctID->Get()).c_str());
    } else {
        strPath->Set(api::Legacy::GetFilenameFail(strAcctID->Get()).c_str());
    }

    const char* szFoldername = server_.API().Internal().Legacy().Receipt();

    // Save the receipt. (My outgoing transaction including the client's
    // signed request that triggered it.)
    processInboxResponse.SaveContract(szFoldername, strPath->Get());
}

void Notary::process_cash_deposit(
    const OTTransaction& input,
    const Item& depositItem,
    const Item& balanceItem,
    otx::context::Client& context,
    ExclusiveAccount& depositorAccount,
    OTTransaction& output,
    Ledger& inbox,
    Ledger& outbox,
    bool& success,
    Item& responseItem,
    Item& responseBalanceItem)
{
    // The incoming transaction may be sent to inboxes and outboxes, and it
    // will probably be bundled in our reply to the user as well. Therefore,
    // let's grab it as a string.
    auto strInReferenceTo = String::Factory();
    auto strBalanceItem = String::Factory();
    const auto& NOTARY_ID = context.Notary();
    const auto& NYM_ID = context.RemoteNym().ID();
    const auto ACCOUNT_ID =
        server_.API().Factory().Internal().Identifier(depositorAccount.get());
    const auto& INSTRUMENT_DEFINITION_ID =
        depositorAccount.get().GetInstrumentDefinitionID();
    const auto strNymID = String::Factory(NYM_ID, api_.Crypto()),
               strAccountID = String::Factory(ACCOUNT_ID, api_.Crypto());
    ExclusiveAccount pMintCashReserveAcct{};

    // BELOW -- DEPOSIT CASH
    // For now, there should only be one of these deposit items inside the
    // transaction.
    // So we treat it that way... I either get it successfully or not.
    //
    // Deposit (the transaction) now supports deposit (the item) and
    // depositCheque (the item)
    // The response item, as well as the inbox and outbox items, will
    // contain a copy
    // of the request item. So I save it into a string here so they can all
    // grab a copy of it
    // into their "in reference to" fields.
    depositItem.SaveContractRaw(strInReferenceTo);
    balanceItem.SaveContractRaw(strBalanceItem);

    // Server response item being added to server response transaction
    // (output)
    // They're getting SOME sort of response item.

    responseItem.SetReferenceString(strInReferenceTo);  // the response
                                                        // item carries a
                                                        // copy of what
                                                        // it's responding
                                                        // to.
    responseItem.SetReferenceToNum(
        depositItem.GetTransactionNum());  // This response item is IN
                                           // RESPONSE to pItem and its
                                           // Owner Transaction.

    responseBalanceItem.SetReferenceString(
        strBalanceItem);  // the response item carries a copy of what it's
                          // responding to.
    responseBalanceItem.SetReferenceToNum(
        depositItem.GetTransactionNum());  // This response item is IN
                                           // RESPONSE to pItem and its
                                           // Owner Transaction.
    auto accountHash = identifier::Account{};

    // If the ID on the "from" account that was passed in,
    // does not match the "Acct From" ID on this transaction item
    if (ACCOUNT_ID != depositItem.GetPurportedAccountID()) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: 'From' account ID on the transaction does not match "
            "'from' account ID on the deposit item.")
            .Flush();
    } else {
        auto rawPurse = ByteArray{};
        depositItem.GetAttachment(rawPurse);
        const auto serializedPurse = proto::Factory<proto::Purse>(rawPurse);

        if (false == proto::Validate(serializedPurse, VERBOSE)) {
            LogError()(OT_PRETTY_CLASS())("Invalid purse").Flush();
        } else {
            auto purse =
                api_.Factory().InternalSession().Purse(serializedPurse);

            if (false == bool(purse)) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to instantiate request purse")
                    .Flush();
            } else {
                if (false == purse.Unlock(*context.Signer(), reason_)) {
                    LogError()(OT_PRETTY_CLASS())("Failed to decrypt purse")
                        .Flush();
                } else if (
                    false == balanceItem.VerifyBalanceStatement(
                                 purse.Value(),
                                 context,
                                 inbox,
                                 outbox,
                                 depositorAccount.get(),
                                 input,
                                 UnallocatedSet<TransactionNumber>(),
                                 reason_)) {
                    LogError()(OT_PRETTY_CLASS())(
                        "ERROR verifying balance statement while depositing "
                        "cash. Acct ID: ")(strAccountID.get())
                        .Flush();
                } else if (INSTRUMENT_DEFINITION_ID != purse.Unit()) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Incorrect unit definition ID on purse")
                        .Flush();
                } else if (NOTARY_ID != purse.Notary()) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Incorrect notary ID on purse")
                        .Flush();
                } else {
                    responseBalanceItem.SetStatus(Item::acknowledgement);
                    bool bSuccess{false};
                    auto token = purse.Pop();

                    while (token) {
                        bSuccess = process_token_deposit(
                            pMintCashReserveAcct,
                            depositorAccount.get(),
                            token);

                        if (bSuccess) {
                            token = purse.Pop();
                        } else {
                            token = {};
                        }
                    }

                    if (bSuccess) {
                        depositorAccount.get().GetIdentifier(accountHash);
                        depositorAccount.Release();
                        // We also need to save the Mint's cash reserve. (Any
                        // cash issued by the Mint is automatically backed by
                        // this reserve account. If cash is deposited, it comes
                        // back out of this account. If the cash expires, then
                        // after the expiry period, if it remains in the
                        // account, it is now the property of the transaction
                        // server.)
                        pMintCashReserveAcct.Release();
                        responseItem.SetStatus(Item::acknowledgement);
                        success = true;  // The cash deposit was successful.
                        LogDetail()(OT_PRETTY_CLASS())(
                            "SUCCESS -- crediting account from cash "
                            "deposit.")
                            .Flush();

                        // TODO:  Right here, again, I need to save the receipt
                        // from the new balance agreement, since we have
                        // "ultimate success".  Also need to save the Nym, since
                        // he had a transaction number removed in the above call
                        // to VerifyBalanceAgreement. If we failed here, then we
                        // wouldn't WANT to save, since that number should stay
                        // on him! Same reason we don't save the accounts if
                        // anything goes wrong.
                    } else {
                        depositorAccount.get().GetIdentifier(accountHash);
                        depositorAccount.Abort();
                        pMintCashReserveAcct.Abort();
                    }
                }
            }
        }
    }

    if (accountHash.empty() && depositorAccount) {
        depositorAccount.get().GetIdentifier(accountHash);
    }

    AddHashesToTransaction(output, inbox, outbox, accountHash);
}

void Notary::process_cash_withdrawal(
    const OTTransaction& requestTransaction,
    const Item& requestItem,
    const Item& balanceItem,
    otx::context::Client& context,
    ExclusiveAccount& account,
    identifier::Account& accountHash,
    Ledger& inbox,
    Ledger& outbox,
    Item& responseItem,
    Item& responseBalanceItem,
    bool& success)
{
    const auto& unit = account.get().GetInstrumentDefinitionID();
    const auto& accountID = requestItem.GetPurportedAccountID();
    bool bSuccess{false};
    std::shared_ptr<otx::blind::Mint> pMint{nullptr};
    ExclusiveAccount pMintCashReserveAcct{};

    auto rawPurse = ByteArray{};
    requestItem.GetAttachment(rawPurse);
    const auto serializedPurse = proto::Factory<proto::Purse>(rawPurse);

    if (false == proto::Validate(serializedPurse, VERBOSE)) {
        LogError()(OT_PRETTY_CLASS())("Invalid purse").Flush();

        return;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Serialized purse is valid").Flush();
    }

    auto requestPurse = factory::Purse(api_, serializedPurse);

    if (false == bool(requestPurse)) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate request purse")
            .Flush();

        return;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Request purse instantiated").Flush();
    }

    if (false == requestPurse.Unlock(*context.Signer(), reason_)) {
        LogError()(OT_PRETTY_CLASS())("Failed to decrypt purse").Flush();

        return;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Request purse unlocked").Flush();
    }

    auto replyPurse =
        factory::Purse(api_, requestPurse, context.RemoteNym(), reason_);

    if (false == replyPurse) {
        LogError()(OT_PRETTY_CLASS())("Failed to instantiate reply purse")
            .Flush();

        return;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Reply purse instantiated").Flush();
    }

    if (false == replyPurse.AddNym(*context.Signer(), reason_)) {
        LogError()(OT_PRETTY_CLASS())("Failed to encrypt reply purse").Flush();

        return;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Reply purse encrypted").Flush();
    }

    const auto verifiedBalance = balanceItem.VerifyBalanceStatement(
        requestPurse.Value() * (-1),
        context,
        inbox,
        outbox,
        account.get(),
        requestTransaction,
        UnallocatedSet<TransactionNumber>(),
        reason_);

    if (false == verifiedBalance) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to verify balance statement for account ")(
            accountID, api_.Crypto())
            .Flush();

        return;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Balance statement verified").Flush();
    }

    responseBalanceItem.SetStatus(Item::acknowledgement);
    auto token = requestPurse.Pop();

    while (token) {
        bSuccess = process_token_withdrawal(
            unit,
            context,
            pMintCashReserveAcct,
            account.get(),
            replyPurse,
            std::move(token));

        if (bSuccess) {
            token = requestPurse.Pop();
        } else {
            break;
        }
    }

    if (bSuccess) {
        // Add the digital cash token to the response message
        responseItem.SetAttachment([&] {
            auto proto = proto::Purse{};
            replyPurse.Internal().Serialize(proto);

            return server_.API().Factory().Internal().Data(proto);
        }());
        responseItem.SetStatus(Item::acknowledgement);
        success = true;  // The cash withdrawal was successful.
        account.get().GetIdentifier(accountHash);
        account.Release();

        // We also need to save the Mint's cash reserve. (Any cash issued by the
        // Mint is automatically backed by this reserve account. If cash is
        // deposited, it comes back out of this account. If the cash expires,
        // then after the expiry period, if it remains in the account, it is now
        // the property of the transaction server.)
        pMintCashReserveAcct.Release();

        // Notice if there is any failure in the above loop, then we will never
        // enter this block. Therefore the account will never be saved with the
        // new debited balance, and the output purse will never be added to the
        // response item.  No tokens will be returned to the user and the
        // account will not be saved, thus retaining the original balance.
        //
        // Only if everything is successful do we enter this block, save the
        // output purse onto the response, and save the newly-debited account
        // back to disk.
    }
}

void Notary::process_cheque_deposit(
    const OTTransaction& input,
    const Item& depositItem,
    const Item& balanceItem,
    otx::context::Client& context,
    ExclusiveAccount& depositorAccount,
    OTTransaction& output,
    Ledger& inbox,
    Ledger& outbox,
    bool& success,
    Item& responseItem,
    Item& responseBalanceItem)
{
    const auto& serverID = context.Notary();
    const auto accountID =
        server_.API().Factory().Internal().Identifier(depositorAccount.get());
    const auto& unitID = depositorAccount.get().GetInstrumentDefinitionID();
    auto serializedItem = String::Factory();
    auto serializedBalanceItem = String::Factory();
    depositItem.SaveContractRaw(serializedItem);
    balanceItem.SaveContractRaw(serializedBalanceItem);
    responseItem.SetReferenceString(serializedItem);
    responseItem.SetReferenceToNum(depositItem.GetTransactionNum());
    responseBalanceItem.SetReferenceString(serializedBalanceItem);
    responseBalanceItem.SetReferenceToNum(depositItem.GetTransactionNum());

    if (accountID != depositItem.GetPurportedAccountID()) {
        LogError()(OT_PRETTY_CLASS())("Wrong account ID on deposit item.")
            .Flush();

        return;
    }

    const auto cheque = extract_cheque(serverID, unitID, depositItem);

    if (false == bool(cheque)) { return; }

    OT_ASSERT(cheque);

    if (accountID == cheque->GetSenderAcctID()) {
        cancel_cheque(
            input,
            *cheque,
            depositItem,
            serializedItem,
            balanceItem,
            context,
            depositorAccount.get(),
            inbox,
            outbox,
            output,
            success,
            responseItem,
            responseBalanceItem);
        depositorAccount.Release();
    } else {
        deposit_cheque(
            input,
            depositItem,
            serializedItem,
            balanceItem,
            *cheque,
            context,
            depositorAccount,
            inbox,
            outbox,
            output,
            success,
            responseItem,
            responseBalanceItem);
    }
}

void Notary::send_push_notification(
    const Account& account,
    const std::shared_ptr<const Ledger>& inbox,
    const std::shared_ptr<const Ledger>& outbox,
    const std::shared_ptr<const OTTransaction>& item) const
{
    OT_ASSERT(inbox);
    OT_ASSERT(outbox);

    if (false == bool(item)) {
        LogError()(OT_PRETTY_CLASS())("No transaction item.").Flush();

        return;
    }

    auto inboxHash = identifier::Generic{};
    auto outboxHash = identifier::Generic{};
    auto serializedAccount = String::Factory();
    auto serializedInbox = String::Factory();
    auto serializedOutbox = String::Factory();
    auto serializedItem = String::Factory();
    account.SaveContractRaw(serializedAccount);
    inbox->SaveContractRaw(serializedInbox);
    inbox->CalculateInboxHash(inboxHash);
    outbox->SaveContractRaw(serializedOutbox);
    outbox->CalculateOutboxHash(outboxHash);
    item->SaveContractRaw(serializedItem);
    auto message = zmq::Message{};
    message.AddFrame(account.GetNymID().asBase58(api_.Crypto()));
    proto::OTXPush push;
    push.set_version(OTX_PUSH_VERSION);
    push.set_type(proto::OTXPUSH_INBOX);
    push.set_accountid(
        server_.API().Factory().Internal().Identifier(account).asBase58(
            api_.Crypto()));
    push.set_itemid(item->GetTransactionNum());
    push.set_account(serializedAccount->Get());
    push.set_inbox(serializedInbox->Get());
    push.set_inboxhash(inboxHash.asBase58(api_.Crypto()));
    push.set_outbox(serializedOutbox->Get());
    push.set_outboxhash(outboxHash.asBase58(api_.Crypto()));
    push.set_item(serializedItem->Get());

    if (false == proto::Validate(push, VERBOSE)) {
        LogError()(OT_PRETTY_CLASS())("Unable to send push notification.")
            .Flush();

        return;
    }

    message.Internal().AddFrame(push);
    notification_socket_->Send(std::move(message));
}

auto Notary::process_token_deposit(
    ExclusiveAccount& reserveAccount,
    Account& depositAccount,
    otx::blind::Token& token) -> bool
{
    if (std::numeric_limits<std::uint32_t>::max() < token.Series()) {
        LogError()(OT_PRETTY_CLASS())("invalid series (")(token.Series())(")")
            .Flush();

        return false;
    }

    const auto amount = token.Value();
    auto& mint = api_.GetPrivateMint(
        token.Unit(), static_cast<std::uint32_t>(token.Series()));

    if (!mint) {
        LogError()(OT_PRETTY_CLASS())("Unable to get or load Mint.").Flush();

        return false;
    }

    reserveAccount =
        api_.Wallet().Internal().mutable_Account(mint.AccountID(), reason_);

    if (false == bool(reserveAccount)) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to get cash reserve account for Mint.")
            .Flush();

        return false;
    }

    if (false == verify_token(mint, token)) { return false; }

    if (false == reserveAccount.get().Debit(amount)) {
        LogError()(OT_PRETTY_CLASS())(
            "Error debiting the mint cash reserve account.")
            .Flush();

        return false;
    }

    if (false == depositAccount.Credit(amount)) {
        LogError()(OT_PRETTY_CLASS())("Error "
                                      "crediting the user's asset "
                                      "account...")
            .Flush();

        if (false == reserveAccount.get().Credit(amount)) {
            LogError()(OT_PRETTY_CLASS())("Failure crediting-back "
                                          "mint's cash reserve account "
                                          "while depositing cash.")
                .Flush();
        }

        return false;
    }

    // Spent token database. This is where the call is made to add the token to
    // the spent token database.
    const auto spent = token.Internal().MarkSpent(reason_);

    if (false == spent) {
        LogError()(OT_PRETTY_CLASS())("Failed recording token as spent...")
            .Flush();

        if (false == reserveAccount.get().Credit(amount)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failure crediting-back mint's cash reserve account while "
                "depositing cash.")
                .Flush();
        }

        if (false == depositAccount.Debit(amount)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failure debiting-back user's asset account while depositing "
                "cash.")
                .Flush();
        }

        return false;
    }

    LogDetail()(OT_PRETTY_CLASS())("Success crediting account with cash token.")
        .Flush();

    return true;
}

auto Notary::process_token_withdrawal(
    const identifier::UnitDefinition& unit,
    otx::context::Client& context,
    ExclusiveAccount& reserveAccount,
    Account& account,
    otx::blind::Purse& replyPurse,
    otx::blind::Token&& token) -> bool
{
    const auto series = token.Series();

    if (std::numeric_limits<std::uint32_t>::max() < series) {
        LogError()(OT_PRETTY_CLASS())("invalid series (")(series)("): ")(
            unit, api_.Crypto())
            .Flush();

        return false;
    }

    const auto value = token.Value();
    auto& mint = api_.GetPrivateMint(unit, static_cast<std::uint32_t>(series));

    if (false == bool(mint)) {
        LogError()(OT_PRETTY_CLASS())("Unable to find Mint (series ")(
            series)("): ")(unit, api_.Crypto())
            .Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Mint loaded").Flush();
    }

    reserveAccount =
        api_.Wallet().Internal().mutable_Account(mint.AccountID(), reason_);

    if (false == bool(reserveAccount)) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to find cash reserve account for Mint "
            "(series ")(series)("): ")(unit, api_.Crypto())
            .Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Reserve account loaded").Flush();
    }

    // Mints expire halfway into their token expiration period. So if a mint
    // creates tokens valid from Jan 1 through Jun 1, then the Mint itself
    // expires Mar 1. That's when the next series Mint is phased in to start
    // issuing tokens, even though the server continues redeeming the first
    // series tokens until June.
    if (mint.Expired()) {
        LogError()(OT_PRETTY_CLASS())(
            "User attempting attempting withdrawal with an expired mint "
            "(series ")(series)("): ")(unit, api_.Crypto())
            .Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Mint is valid").Flush();
    }

    const auto signedToken =
        mint.Internal().SignToken(*context.Signer(), token, reason_);

    if (false == signedToken) {
        LogError()(OT_PRETTY_CLASS())("Failed to sign token").Flush();

        return false;
    } else {
        LogInsane()(OT_PRETTY_CLASS())("Token signed").Flush();
    }

    if (false == replyPurse.Push(std::move(token), reason_)) {
        LogError()(OT_PRETTY_CLASS())("Failed to push token to reply purse")
            .Flush();

        return false;
    }

    // Deduct the amount from the account...
    // TODO need to be able to "roll back" if anything inside this block fails.
    if (account.Debit(value)) {
        // Credit the server's cash account for this instrument definition in
        // the same amount that was debited. When the token is deposited again,
        // Debit that same server cash account and deposit in the depositor's
        // acct. Why, you might ask? Because if the token expires, the money
        // will stay in the bank's cash account instead of being lost (and
        // screwing up the overall issuer balance, with the issued money
        // disappearing forever.) The bank knows that once the series expires,
        // whatever funds are left in that cash account are for the bank to
        // keep. They can be transferred to another account and kept, instead of
        // being lost.
        if (false == reserveAccount.get().Credit(value)) {
            LogError()(OT_PRETTY_CLASS())(
                "Error crediting mint cash reserve account...")
                .Flush();

            // Reverse the account debit (even though we're not going to save it
            // anyway.)
            if (false == account.Credit(value)) {
                LogError()(OT_PRETTY_CLASS())(
                    "failed crediting user account back.")
                    .Flush();
            }

            return false;
        }
    } else {
        const auto& INSTRUMENT_DEFINITION_ID =
            account.GetInstrumentDefinitionID();
        const auto unittype =
            api_.Wallet().Internal().CurrencyTypeBasedOnUnitType(
                INSTRUMENT_DEFINITION_ID);
        LogError()(OT_PRETTY_CLASS())("Unable to debit account ")(
            account.GetPurportedAccountID(),
            api_.Crypto())(" in the amount of: ")(value, unittype)
            .Flush();

        return false;
    }

    return true;
}

auto Notary::verify_token(otx::blind::Mint& mint, otx::blind::Token& token)
    -> bool
{
    // This call to VerifyToken verifies the token's Series and From/To dates
    // against the mint's, and also verifies that the CURRENT date is inside
    // that valid date range.
    //
    // It also verifies the Lucre coin data itself against the key for that
    // series and denomination. (The signed and unblinded Lucre coin is finally
    // verified in Lucre using the appropriate Mint private key.)
    if (false ==
        mint.Internal().VerifyToken(server_.GetServerNym(), token, reason_)) {
        LogError()(OT_PRETTY_CLASS())("Failed to verofy token").Flush();

        return false;
    }

    // Lookup the token in the SPENT TOKEN DATABASE, and make sure
    // that it hasn't already been spent...
    const auto spent = token.IsSpent(reason_);

    if (spent) {
        LogError()(OT_PRETTY_CLASS())("Token is already spent").Flush();

        return false;
    } else {
        LogDebug()(OT_PRETTY_CLASS())("SUCCESS verifying token...").Flush();

        return true;
    }
}
}  // namespace opentxs::server
