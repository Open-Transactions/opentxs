// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Factory.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#if OT_CASH
#include "opentxs/blind/Mint.hpp"
#include "opentxs/blind/Purse.hpp"
#endif
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/recurring/OTAgreement.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include <array>

#include "Factory.hpp"

#define OT_METHOD "opentxs::api::implementation::Factory::"

namespace opentxs
{
api::Factory* Factory::FactoryAPI(const api::Core& api)
{
    return new api::implementation::Factory(api);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Factory::Factory(const api::Core& api)
    : api_(api)
{
}

std::unique_ptr<opentxs::Basket> Factory::Basket() const
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(api_));

    return basket;
}

std::unique_ptr<opentxs::Basket> Factory::Basket(
    std::int32_t nCount,
    std::int64_t lMinimumTransferAmount) const
{
    std::unique_ptr<opentxs::Basket> basket;
    basket.reset(new opentxs::Basket(api_, nCount, lMinimumTransferAmount));

    return basket;
}

std::unique_ptr<opentxs::Cheque> Factory::Cheque(
    const OTTransaction& receipt) const
{
    std::unique_ptr<opentxs::Cheque> output{new opentxs::Cheque{receipt.API()}};

    OT_ASSERT(output)

    auto serializedItem = String::Factory();
    receipt.GetReferenceString(serializedItem);
    std::unique_ptr<opentxs::Item> item{Item(
        serializedItem,
        receipt.GetRealNotaryID(),
        receipt.GetReferenceToNum())};

    OT_ASSERT(false != bool(item));

    auto serializedCheque = String::Factory();
    item->GetAttachment(serializedCheque);
    const auto loaded = output->LoadContractFromString(serializedCheque);

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load cheque.").Flush();
    }

    return output;
}

std::unique_ptr<opentxs::Cheque> Factory::Cheque() const
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(new opentxs::Cheque(api_));

    return cheque;
}

std::unique_ptr<opentxs::Cheque> Factory::Cheque(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    std::unique_ptr<opentxs::Cheque> cheque;
    cheque.reset(
        new opentxs::Cheque(api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return cheque;
}

std::unique_ptr<opentxs::Contract> Factory::Contract(
    const opentxs::String& strInput) const
{

    using namespace opentxs;
    auto strContract = String::Factory(),
         strFirstLine = String::Factory();  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {

        std::unique_ptr<opentxs::Contract> pContract;

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is 36
                                                          // chars long.
        {
            pContract.reset(new OTSmartContract(api_));
            OT_ASSERT(false != bool(pContract));
        }

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED PAYMENT PLAN-----"))  // this string is 35
                                                         // chars long.
        {
            pContract.reset(new OTPaymentPlan(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED TRADE-----"))  // this string is 28
                                                         // chars long.
        {
            pContract.reset(new OTTrade(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED OFFER-----")) {
            pContract.reset(new OTOffer(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED INVOICE-----")) {
            pContract.reset(new opentxs::Cheque(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED VOUCHER-----")) {
            pContract.reset(new opentxs::Cheque(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED CHEQUE-----")) {
            pContract.reset(new opentxs::Cheque(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED MESSAGE-----")) {
            pContract.reset(new opentxs::Message(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains("-----BEGIN SIGNED MINT-----")) {
#if OT_CASH
            auto mint = Mint();
            pContract.reset(mint.release());
            OT_ASSERT(false != bool(pContract));
#endif  // OT_CASH
        } else if (strFirstLine->Contains("-----BEGIN SIGNED FILE-----")) {
            OT_ASSERT(false != bool(pContract));
        }

        // The string didn't match any of the options in the factory.
        //
        if (!pContract) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Object type not yet supported by class factory: ")(
                strFirstLine)
                .Flush();
            // Does the contract successfully load from the string passed in?
        } else if (!pContract->LoadContractFromString(strContract)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed loading contract from string (first line): ")(
                strFirstLine)
                .Flush();
        } else {
            return pContract;
        }
    }
    return nullptr;
}

std::unique_ptr<OTCron> Factory::Cron(const api::Core& server) const
{
    std::unique_ptr<opentxs::OTCron> cron;
    cron.reset(new opentxs::OTCron(server));

    return cron;
}

std::unique_ptr<OTCronItem> Factory::CronItem(const String& strCronItem) const
{
    std::array<char, 45> buf{};

    if (!strCronItem.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Empty string was passed in (returning nullptr).")
            .Flush();
        return nullptr;
    }

    auto strContract = String::Factory(strCronItem.Get());

    if (!strContract->DecodeIfArmored(false)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Input string apparently was encoded and "
            "then failed decoding. Contents: ")(strCronItem)(".")
            .Flush();
        return nullptr;
    }

    strContract->reset();  // for sgets
    bool bGotLine = strContract->sgets(buf.data(), 40);

    if (!bGotLine) return nullptr;

    auto strFirstLine = String::Factory(buf.data());
    // set the "file" pointer within this string back to index 0.
    strContract->reset();

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine->Exists() || strFirstLine->Contains("- -"))
        return nullptr;

    // By this point we know already that it's not escaped.
    // BUT it might still be ARMORED!

    std::unique_ptr<OTCronItem> pItem;
    // this string is 35 chars long.
    if (strFirstLine->Contains("-----BEGIN SIGNED PAYMENT PLAN-----")) {
        pItem.reset(new OTPaymentPlan(api_));
    }
    // this string is 28 chars long.
    else if (strFirstLine->Contains("-----BEGIN SIGNED TRADE-----")) {
        pItem.reset(new OTTrade(api_));
    }
    // this string is 36 chars long.
    else if (strFirstLine->Contains("-----BEGIN SIGNED SMARTCONTRACT-----")) {
        pItem.reset(new OTSmartContract(api_));
    } else {
        return nullptr;
    }

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) { return pItem; }

    return nullptr;
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const std::string& serialized) const
{
    return Item(String::Factory(serialized));
}

std::unique_ptr<opentxs::Item> Factory::Item(const String& serialized) const
{
    std::unique_ptr<opentxs::Item> output{new opentxs::Item(api_)};

    if (output) {
        const auto loaded = output->LoadContractFromString(serialized);

        if (false == loaded) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to deserialize.")
                .Flush();
            output.reset();
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.").Flush();
    }

    return output;
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const identifier::Nym& theNymID,
    const opentxs::Item& theOwner) const
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_, theNymID, theOwner));

    return item;
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner) const
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(api_, theNymID, theOwner));

    return item;
}

std::unique_ptr<opentxs::Item> Factory::Item(
    const identifier::Nym& theNymID,
    const OTTransaction& theOwner,
    itemType theType,
    const Identifier& pDestinationAcctID) const
{
    std::unique_ptr<opentxs::Item> item;
    item.reset(new opentxs::Item(
        api_, theNymID, theOwner, theType, pDestinationAcctID));

    return item;
}

// Sometimes I don't know user ID of the originator, or the account ID of the
// originator,
// until after I have loaded the item. It's simply impossible to set those
// values ahead
// of time, sometimes. In those cases, we set the values appropriately but then
// we need
// to verify that the user ID is actually the owner of the AccountID. TOdo that.
std::unique_ptr<opentxs::Item> Factory::Item(
    const String& strItem,
    const Identifier& theNotaryID,
    std::int64_t lTransactionNumber) const
{
    if (!strItem.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": strItem is empty. (Expected an "
                                           "item).")
            .Flush();
        return nullptr;
    }

    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(api_)};

    // So when it loads its own server ID, we can compare to this one.
    pItem->SetRealNotaryID(theNotaryID);

    // This loads up the purported account ID and the user ID.
    if (pItem->LoadContractFromString(strItem)) {
        const Identifier& ACCOUNT_ID = pItem->GetPurportedAccountID();
        pItem->SetRealAccountID(ACCOUNT_ID);  // I do this because it's all
                                              // we've got in this case. It's
                                              // what's in the
        // xml, so it must be right. If it's a lie, the signature will fail or
        // the
        // user will not show as the owner of that account. But remember, the
        // server
        // sent the message in the first place.

        pItem->SetTransactionNum(lTransactionNumber);

        if (pItem->VerifyContractID())  // this compares purported and real
                                        // account IDs, as well as server IDs.
        {
            return pItem;
        }
    }

    return nullptr;
}

// Let's say you have created a transaction, and you are creating an item to put
// into it.
// Well in that case, you don't care to verify that the real IDs match the
// purported IDs, since
// you are creating this item yourself, not verifying it from someone else.
// Use this function to create the new Item before you add it to your new
// Transaction.
std::unique_ptr<opentxs::Item> Factory::Item(
    const OTTransaction& theOwner,
    itemType theType,
    const Identifier& pDestinationAcctID) const
{
    std::unique_ptr<opentxs::Item> pItem{new opentxs::Item(
        theOwner.API(),
        theOwner.GetNymID(),
        theOwner,
        theType,
        pDestinationAcctID)};

    if (false != bool(pItem)) {
        pItem->SetPurportedAccountID(theOwner.GetPurportedAccountID());
        pItem->SetPurportedNotaryID(theOwner.GetPurportedNotaryID());
        return pItem;
    }
    return nullptr;
}

std::unique_ptr<opentxs::Ledger> Factory::Ledger(
    const Identifier& theAccountID,
    const Identifier& theNotaryID) const
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(api_, theAccountID, theNotaryID));

    return ledger;
}

std::unique_ptr<opentxs::Ledger> Factory::Ledger(
    const identifier::Nym& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID) const
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(
        new opentxs::Ledger(api_, theNymID, theAccountID, theNotaryID));

    return ledger;
}

std::unique_ptr<opentxs::Ledger> Factory::Ledger(
    const identifier::Nym& theNymID,
    const Identifier& theAcctID,
    const Identifier& theNotaryID,
    ledgerType theType,
    bool bCreateFile) const
{
    std::unique_ptr<opentxs::Ledger> ledger;
    ledger.reset(new opentxs::Ledger(api_, theNymID, theAcctID, theNotaryID));

    ledger->generate_ledger(
        theNymID, theAcctID, theNotaryID, theType, bCreateFile);

    return ledger;
}

std::unique_ptr<OTMarket> Factory::Market() const
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_));

    return market;
}

std::unique_ptr<OTMarket> Factory::Market(const char* szFilename) const
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(api_, szFilename));

    return market;
}

std::unique_ptr<OTMarket> Factory::Market(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& CURRENCY_TYPE_ID,
    const std::int64_t& lScale) const
{
    std::unique_ptr<opentxs::OTMarket> market;
    market.reset(new opentxs::OTMarket(
        api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_TYPE_ID, lScale));

    return market;
}

std::unique_ptr<opentxs::Message> Factory::Message() const
{
    std::unique_ptr<opentxs::Message> message;
    message.reset(new opentxs::Message(api_));

    return message;
}

#if OT_CASH
std::unique_ptr<blind::Mint> Factory::Mint() const
{
    std::unique_ptr<blind::Mint> pMint;

#if OT_CASH_USING_LUCRE
    pMint.reset(opentxs::Factory::MintLucre(api_));

    OT_ASSERT(false != bool(pMint));

#else
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Open-Transactions isn't built with any digital cash algorithms, "
        "so it's impossible to instantiate a mint.")
        .Flush();
#endif

    return pMint;
}

std::unique_ptr<blind::Mint> Factory::Mint(
    const String& strNotaryID,
    const String& strInstrumentDefinitionID) const
{
    std::unique_ptr<blind::Mint> pMint;

#if OT_CASH_USING_LUCRE
    pMint.reset(opentxs::Factory::MintLucre(
        api_, strNotaryID, strInstrumentDefinitionID));

    OT_ASSERT(false != bool(pMint));
#else
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Open-Transactions isn't built with any digital cash algorithms, "
        "so it's impossible to instantiate a mint.")
        .Flush();
#endif

    return pMint;
}

std::unique_ptr<blind::Mint> Factory::Mint(
    const String& strNotaryID,
    const String& strServerNymID,
    const String& strInstrumentDefinitionID) const
{
    std::unique_ptr<blind::Mint> pMint;

#if OT_CASH_USING_LUCRE
    pMint.reset(opentxs::Factory::MintLucre(
        api_, strNotaryID, strServerNymID, strInstrumentDefinitionID));

    OT_ASSERT(false != bool(pMint));
#else
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Open-Transactions isn't built with any digital cash algorithms, "
        "so it's impossible to instantiate a mint.")
        .Flush();
#endif

    return pMint;
}
#endif
std::unique_ptr<OTOffer> Factory::Offer() const
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(api_));

    return offer;
}

std::unique_ptr<OTOffer> Factory::Offer(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& CURRENCY_ID,
    const std::int64_t& MARKET_SCALE) const
{
    std::unique_ptr<OTOffer> offer;
    offer.reset(new OTOffer(
        api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID, CURRENCY_ID, MARKET_SCALE));

    return offer;
}

std::unique_ptr<OTPayment> Factory::Payment() const
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_));

    return payment;
}

std::unique_ptr<OTPayment> Factory::Payment(const String& strPayment) const
{
    std::unique_ptr<OTPayment> payment;
    payment.reset(new OTPayment(api_, strPayment));

    return payment;
}

std::unique_ptr<OTPayment> Factory::Payment(
    const opentxs::Contract& contract) const
{
    auto payment = Factory::Payment(String::Factory(contract));

    if (payment) { payment->SetTempValues(); }

    return payment;
}

#if OT_CRYPTO_WITH_BIP39
OTPaymentCode Factory::PaymentCode(const std::string& base58) const
{
    return opentxs::PaymentCode::Factory(api_.Crypto(), api_.Seeds(), base58);
}

OTPaymentCode Factory::PaymentCode(const proto::PaymentCode& serialized) const
{
    return opentxs::PaymentCode::Factory(
        api_.Crypto(), api_.Seeds(), serialized);
}

OTPaymentCode Factory::PaymentCode(
    const std::string& seed,
    const std::uint32_t nym,
    const std::uint8_t version,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream) const
{
    return opentxs::PaymentCode::Factory(
        api_.Crypto(),
        api_.Seeds(),
        seed,
        nym,
        version,
        bitmessage,
        bitmessageVersion,
        bitmessageStream);
}
#endif

std::unique_ptr<OTPaymentPlan> Factory::PaymentPlan() const
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(api_));

    return paymentplan;
}

std::unique_ptr<OTPaymentPlan> Factory::PaymentPlan(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(
        new OTPaymentPlan(api_, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    return paymentplan;
}

std::unique_ptr<OTPaymentPlan> Factory::PaymentPlan(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& SENDER_ACCT_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const Identifier& RECIPIENT_ACCT_ID,
    const identifier::Nym& RECIPIENT_NYM_ID) const
{
    std::unique_ptr<OTPaymentPlan> paymentplan;
    paymentplan.reset(new OTPaymentPlan(
        api_,
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        RECIPIENT_ACCT_ID,
        RECIPIENT_NYM_ID));

    return paymentplan;
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const ConstNym& senderNym,
    [[maybe_unused]] const std::string& message) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const ConstNym& senderNym,
    [[maybe_unused]] const std::string& payment,
    [[maybe_unused]] const bool isPayment) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

#if OT_CASH
std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const ConstNym& senderNym,
    [[maybe_unused]] const std::shared_ptr<blind::Purse> purse) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}
#endif

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const std::shared_ptr<const PeerRequest> request,
    [[maybe_unused]] const std::shared_ptr<const PeerReply> reply,
    [[maybe_unused]] const std::uint32_t& version) const

{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const std::shared_ptr<const PeerRequest> request,
    [[maybe_unused]] const std::uint32_t& version) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const ConstNym& signerNym,
    [[maybe_unused]] const proto::PeerObject& serialized) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    [[maybe_unused]] const ConstNym& recipientNym,
    [[maybe_unused]] const Armored& encrypted) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Peer objects are only supported in client sessions")
        .Flush();

    return {};
}

#if OT_CASH
std::unique_ptr<blind::Purse> Factory::Purse(
    const ServerContext& context,
    const identifier::UnitDefinition& unit,
    const blind::Mint& mint,
    const Amount totalValue,
    const proto::CashType type) const
{
    return std::unique_ptr<blind::Purse>(
        opentxs::Factory::Purse(api_, context, type, mint, totalValue));
}

std::unique_ptr<blind::Purse> Factory::Purse(
    const proto::Purse& serialized) const
{
    return std::unique_ptr<blind::Purse>(
        opentxs::Factory::Purse(api_, serialized));
}

std::unique_ptr<blind::Purse> Factory::Purse(
    const Nym& owner,
    const identifier::Server& server,
    const identifier::UnitDefinition& unit,
    const proto::CashType type) const
{
    return std::unique_ptr<blind::Purse>(
        opentxs::Factory::Purse(api_, owner, server, unit, type));
}
#endif  // OT_CASH

std::unique_ptr<OTScriptable> Factory::Scriptable(const String& strInput) const
{
    std::array<char, 45> buf{};

    if (!strInput.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Input string is empty.")
            .Flush();
        return nullptr;
    }

    auto strContract = String::Factory(strInput.Get());

    if (!strContract->DecodeIfArmored(false))  // bEscapedIsAllowed=true
                                               // by default.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Input string apparently was encoded and "
            "then failed decoding. Contents: ")(strInput)(".")
            .Flush();
        return nullptr;
    }

    // At this point, strContract contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)
    //
    strContract->reset();  // for sgets
    bool bGotLine = strContract->sgets(buf.data(), 40);

    if (!bGotLine) return nullptr;

    std::unique_ptr<OTScriptable> pItem;

    auto strFirstLine = String::Factory(buf.data());
    strContract->reset();  // set the "file" pointer within this string back to
                           // index 0.

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine->Exists() || strFirstLine->Contains("- -"))
        return nullptr;

    // There are actually two factories that load smart contracts. See
    // OTCronItem.
    //
    else if (strFirstLine->Contains(
                 "-----BEGIN SIGNED SMARTCONTRACT-----"))  // this string is 36
                                                           // chars long.
    {
        pItem.reset(new OTSmartContract(api_));
        OT_ASSERT(false != bool(pItem));
    }

    // The string didn't match any of the options in the factory.
    if (false == bool(pItem)) return nullptr;

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract)) return pItem;

    return nullptr;
}

std::unique_ptr<OTSignedFile> Factory::SignedFile() const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_));

    return signedfile;
}

std::unique_ptr<OTSignedFile> Factory::SignedFile(
    const String& LOCAL_SUBDIR,
    const String& FILE_NAME) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}
std::unique_ptr<OTSignedFile> Factory::SignedFile(
    const char* LOCAL_SUBDIR,
    const String& FILE_NAME) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

std::unique_ptr<OTSignedFile> Factory::SignedFile(
    const char* LOCAL_SUBDIR,
    const char* FILE_NAME) const
{
    std::unique_ptr<OTSignedFile> signedfile;
    signedfile.reset(new OTSignedFile(api_, LOCAL_SUBDIR, FILE_NAME));

    return signedfile;
}

std::unique_ptr<OTSmartContract> Factory::SmartContract() const
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_));

    return smartcontract;
}

std::unique_ptr<OTSmartContract> Factory::SmartContract(
    const Identifier& NOTARY_ID) const
{
    std::unique_ptr<OTSmartContract> smartcontract;
    smartcontract.reset(new OTSmartContract(api_, NOTARY_ID));

    return smartcontract;
}

std::unique_ptr<OTTrade> Factory::Trade() const
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(api_));

    return trade;
}

std::unique_ptr<OTTrade> Factory::Trade(
    const Identifier& notaryID,
    const Identifier& instrumentDefinitionID,
    const Identifier& assetAcctId,
    const identifier::Nym& nymID,
    const Identifier& currencyId,
    const Identifier& currencyAcctId) const
{
    std::unique_ptr<OTTrade> trade;
    trade.reset(new OTTrade(
        api_,
        notaryID,
        instrumentDefinitionID,
        assetAcctId,
        nymID,
        currencyId,
        currencyAcctId));

    return trade;
}

std::unique_ptr<OTTransactionType> Factory::Transaction(
    const String& strInput) const
{
    auto strContract = String::Factory(),
         strFirstLine = String::Factory();  // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {
        std::unique_ptr<OTTransactionType> pContract;

        if (strFirstLine->Contains(
                "-----BEGIN SIGNED TRANSACTION-----"))  // this string is 34
                                                        // chars long.
        {
            pContract.reset(new OTTransaction(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED TRANSACTION ITEM-----"))  // this
                                                                    // string is
                                                                    // 39 chars
                                                                    // long.
        {
            pContract.reset(new opentxs::Item(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED LEDGER-----"))  // this string is 29
                                                          // chars long.
        {
            pContract.reset(new opentxs::Ledger(api_));
            OT_ASSERT(false != bool(pContract));
        } else if (strFirstLine->Contains(
                       "-----BEGIN SIGNED ACCOUNT-----"))  // this string is 30
                                                           // chars long.
        {
            OT_FAIL;
        }

        // The string didn't match any of the options in the factory.
        //

        //        const char* szFunc = "OTTransactionType::TransactionFactory";
        // The string didn't match any of the options in the factory.
        if (nullptr == pContract) {
            LogNormal(OT_METHOD)(__FUNCTION__)(  //<< szFunc
                ": Object type not yet supported by class factory: ")(
                strFirstLine)
                .Flush();
            return nullptr;
        }

        // This causes pItem to load ASSUMING that the PurportedAcctID and
        // PurportedNotaryID are correct.
        // The object is still expected to be internally consistent with its
        // sub-items, regarding those IDs,
        // but the big difference is that it will SET the Real Acct and Real
        // Notary IDs based on the purported
        // values. This way you can load a transaction without knowing the
        // account in advance.
        //
        pContract->SetLoadInsecure();

        // Does the contract successfully load from the string passed in?
        if (pContract->LoadContractFromString(strContract)) {
            // NOTE: this already happens in OTTransaction::ProcessXMLNode and
            // OTLedger::ProcessXMLNode.
            // Specifically, it happens when m_bLoadSecurely is set to false.
            //
            //          pContract->SetRealNotaryID(pItem->GetPurportedNotaryID());
            //          pContract->SetRealAccountID(pItem->GetPurportedAccountID());

            return pContract;
        } else {
            LogNormal(OT_METHOD)(__FUNCTION__)(  //<< szFunc
                ": Failed loading contract from string (first line): ")(
                strFirstLine)
                .Flush();
        }
    }

    return nullptr;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const opentxs::Ledger& theOwner) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(api_, theOwner));

    return transaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const identifier::Nym& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    originType theOriginType) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_, theNymID, theAccountID, theNotaryID, theOriginType));

    return transaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const identifier::Nym& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    std::int64_t lTransactionNum,
    originType theOriginType) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_,
        theNymID,
        theAccountID,
        theNotaryID,
        lTransactionNum,
        theOriginType));

    return transaction;
}
// THIS factory only used when loading an abbreviated box receipt
// (inbox, nymbox, or outbox receipt).
// The full receipt is loaded only after the abbreviated ones are loaded,
// and verified against them.
std::unique_ptr<OTTransaction> Factory::Transaction(
    const identifier::Nym& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    const std::int64_t& lNumberOfOrigin,
    originType theOriginType,
    const std::int64_t& lTransactionNum,
    const std::int64_t& lInRefTo,
    const std::int64_t& lInRefDisplay,
    time64_t the_DATE_SIGNED,
    transactionType theType,
    const String& strHash,
    const std::int64_t& lAdjustment,
    const std::int64_t& lDisplayValue,
    const std::int64_t& lClosingNum,
    const std::int64_t& lRequestNum,
    bool bReplyTransSuccess,
    NumList* pNumList) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_,
        theNymID,
        theAccountID,
        theNotaryID,
        lNumberOfOrigin,
        theOriginType,
        lTransactionNum,
        lInRefTo,
        lInRefDisplay,
        the_DATE_SIGNED,
        theType,
        strHash,
        lAdjustment,
        lDisplayValue,
        lClosingNum,
        lRequestNum,
        bReplyTransSuccess,
        pNumList));

    return transaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const opentxs::Ledger& theOwner,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const
{
    auto pTransaction = Transaction(
        theOwner.GetNymID(),
        theOwner.GetPurportedAccountID(),
        theOwner.GetPurportedNotaryID(),
        theType,
        theOriginType,
        lTransactionNum);
    if (false != bool(pTransaction)) pTransaction->SetParent(theOwner);

    return pTransaction;
}

std::unique_ptr<OTTransaction> Factory::Transaction(
    const identifier::Nym& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    std::int64_t lTransactionNum /*=0*/) const
{
    std::unique_ptr<OTTransaction> transaction;
    transaction.reset(new OTTransaction(
        api_,
        theNymID,
        theAccountID,
        theNotaryID,
        lTransactionNum,
        theOriginType));
    OT_ASSERT(false != bool(transaction));

    transaction->m_Type = theType;

    // Since we're actually generating this transaction, then we can go ahead
    // and set the purported account and server IDs (we have already set the
    // real ones in the constructor). Now both sets are fill with matching data.
    // No need to security check the IDs since we are creating this transaction
    // versus loading and inspecting it.
    transaction->SetPurportedAccountID(theAccountID);
    transaction->SetPurportedNotaryID(theNotaryID);

    return transaction;
}
}  // namespace opentxs::api::implementation
