// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_FACTORY_HPP
#define OPENTXS_API_FACTORY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include "opentxs/core/util/Common.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{
namespace api
{
class Factory
{
public:
    EXPORT virtual std::unique_ptr<opentxs::Basket> Basket() const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Basket> Basket(
        std::int32_t nCount,
        std::int64_t lMinimumTransferAmount) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::Cheque> Cheque(
        const OTTransaction& receipt) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Cheque> Cheque() const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Cheque> Cheque(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::Contract> Contract(
        const String& strCronItem) const = 0;

    EXPORT virtual std::unique_ptr<OTCron> Cron(
        const api::Core& server) const = 0;

    EXPORT virtual std::unique_ptr<OTCronItem> CronItem(
        const String& strCronItem) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const String& serialized) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const std::string& serialized) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const opentxs::Item& theOwner) const = 0;  // From owner we can get acct
                                                   // ID, server ID, and
                                                   // transaction Num
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner) const = 0;  // From owner we can get acct
                                                   // ID, server ID, and
                                                   // transaction Num
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const Identifier& pDestinationAcctID) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const String& strItem,
        const Identifier& theNotaryID,
        std::int64_t lTransactionNumber) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const OTTransaction& theOwner,
        itemType theType,
        const Identifier& pDestinationAcctID) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::Ledger> Ledger(
        const Identifier& theAccountID,
        const Identifier& theNotaryID) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const Identifier& theAcctID,
        const Identifier& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const = 0;

    EXPORT virtual std::unique_ptr<OTMarket> Market() const = 0;
    EXPORT virtual std::unique_ptr<OTMarket> Market(
        const char* szFilename) const = 0;
    EXPORT virtual std::unique_ptr<OTMarket> Market(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& CURRENCY_TYPE_ID,
        const std::int64_t& lScale) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::Message> Message() const = 0;

#if OT_CASH
    EXPORT virtual std::unique_ptr<blind::Mint> Mint() const = 0;
    EXPORT virtual std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strInstrumentDefinitionID) const = 0;
    EXPORT virtual std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID) const = 0;
#endif

    EXPORT virtual std::unique_ptr<OTOffer> Offer()
        const = 0;  // The constructor contains
                    // the 3 variables needed to
                    // identify any market.
    EXPORT virtual std::unique_ptr<OTOffer> Offer(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& CURRENCY_ID,
        const std::int64_t& MARKET_SCALE) const = 0;

    EXPORT virtual std::unique_ptr<OTPayment> Payment() const = 0;
    EXPORT virtual std::unique_ptr<OTPayment> Payment(
        const String& strPayment) const = 0;
    EXPORT virtual std::unique_ptr<OTPayment> Payment(
        const opentxs::Contract& contract) const = 0;

#if OT_CRYPTO_WITH_BIP39
    EXPORT virtual OTPaymentCode PaymentCode(
        const std::string& base58) const = 0;
    EXPORT virtual OTPaymentCode PaymentCode(
        const proto::PaymentCode& serialized) const = 0;
    EXPORT virtual OTPaymentCode PaymentCode(
        const std::string& seed,
        const std::uint32_t nym,
        const std::uint8_t version,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const = 0;
#endif

    EXPORT virtual std::unique_ptr<OTPaymentPlan> PaymentPlan() const = 0;
    EXPORT virtual std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const = 0;
    EXPORT virtual std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const = 0;

    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& senderNym,
        const std::string& message) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& senderNym,
        const std::string& payment,
        const bool isPayment) const = 0;
#if OT_CASH
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& senderNym,
        const std::shared_ptr<blind::Purse> purse) const = 0;
#endif
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const std::shared_ptr<const PeerRequest> request,
        const std::shared_ptr<const PeerReply> reply,
        const std::uint32_t& version) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const std::shared_ptr<const PeerRequest> request,
        const std::uint32_t& version) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& signerNym,
        const proto::PeerObject& serialized) const = 0;
    EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& recipientNym,
        const Armored& encrypted) const = 0;

#if OT_CASH
    EXPORT virtual std::unique_ptr<blind::Purse> Purse(
        const ServerContext& context,
        const identifier::UnitDefinition& unit,
        const blind::Mint& mint,
        const Amount totalValue,
        const proto::CashType type = proto::CASHTYPE_LUCRE) const = 0;
    EXPORT virtual std::unique_ptr<blind::Purse> Purse(
        const proto::Purse& serialized) const = 0;
    EXPORT virtual std::unique_ptr<blind::Purse> Purse(
        const Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const proto::CashType type = proto::CASHTYPE_LUCRE) const = 0;
#endif  // OT_CASH

    EXPORT virtual std::unique_ptr<OTScriptable> Scriptable(
        const String& strCronItem) const = 0;

    EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile() const = 0;
    EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile(
        const String& LOCAL_SUBDIR,
        const String& FILE_NAME) const = 0;
    EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const String& FILE_NAME) const = 0;
    EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const char* FILE_NAME) const = 0;

    EXPORT virtual std::unique_ptr<OTSmartContract> SmartContract() const = 0;
    EXPORT virtual std::unique_ptr<OTSmartContract> SmartContract(
        const Identifier& NOTARY_ID) const = 0;

    EXPORT virtual std::unique_ptr<OTTrade> Trade() const = 0;
    EXPORT virtual std::unique_ptr<OTTrade> Trade(
        const Identifier& notaryID,
        const Identifier& instrumentDefinitionID,
        const Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const Identifier& currencyId,
        const Identifier& currencyAcctId) const = 0;

    EXPORT virtual std::unique_ptr<OTTransactionType> Transaction(
        const String& strCronItem) const = 0;

    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner) const = 0;
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        originType theOriginType = originType::not_applicable) const = 0;
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const = 0;
    // THIS factory only used when loading an abbreviated box receipt
    // (inbox, nymbox, or outbox receipt).
    // The full receipt is loaded only after the abbreviated ones are loaded,
    // and verified against them.
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
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
        NumList* pNumList = nullptr) const = 0;
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const = 0;
    EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const = 0;

    EXPORT virtual ~Factory() = default;

protected:
    Factory() = default;

private:
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
