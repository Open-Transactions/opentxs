// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::implementation
{
class Factory : virtual public opentxs::api::Factory
{
public:
    std::unique_ptr<opentxs::Basket> Basket() const override;
    std::unique_ptr<opentxs::Basket> Basket(
        std::int32_t nCount,
        std::int64_t lMinimumTransferAmount) const override;

    std::unique_ptr<opentxs::Cheque> Cheque(
        const OTTransaction& receipt) const override;
    std::unique_ptr<opentxs::Cheque> Cheque() const override;
    std::unique_ptr<opentxs::Cheque> Cheque(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;

    std::unique_ptr<opentxs::Contract> Contract(
        const String& strCronItem) const override;

    std::unique_ptr<OTCron> Cron(const api::Core& server) const override;

    std::unique_ptr<OTCronItem> CronItem(
        const String& strCronItem) const override;

    std::unique_ptr<opentxs::Item> Item(
        const String& serialized) const override;
    std::unique_ptr<opentxs::Item> Item(
        const std::string& serialized) const override;
    std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const opentxs::Item& theOwner) const override;  // From owner we can get
                                                        // acct ID, server ID,
                                                        // and transaction Num
    std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner) const override;  // From owner we can get
                                                        // acct ID, server ID,
                                                        // and transaction Num
    std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const Identifier& pDestinationAcctID) const override;
    std::unique_ptr<opentxs::Item> Item(
        const String& strItem,
        const Identifier& theNotaryID,
        std::int64_t lTransactionNumber) const override;
    std::unique_ptr<opentxs::Item> Item(
        const OTTransaction& theOwner,
        itemType theType,
        const Identifier& pDestinationAcctID) const override;

    std::unique_ptr<opentxs::Ledger> Ledger(
        const Identifier& theAccountID,
        const Identifier& theNotaryID) const override;
    std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID) const override;
    std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const Identifier& theAcctID,
        const Identifier& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const override;

    std::unique_ptr<OTMarket> Market() const override;
    std::unique_ptr<OTMarket> Market(const char* szFilename) const override;
    virtual std::unique_ptr<OTMarket> Market(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& CURRENCY_TYPE_ID,
        const std::int64_t& lScale) const override;

    std::unique_ptr<opentxs::Message> Message() const override;

#if OT_CASH
    std::unique_ptr<blind::Mint> Mint() const override;
    std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strInstrumentDefinitionID) const override;
    std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID) const override;
#endif

    std::unique_ptr<OTOffer> Offer() const override;  // The constructor
                                                      // contains the 3
                                                      // variables needed to
                                                      // identify any market.
    std::unique_ptr<OTOffer> Offer(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& CURRENCY_ID,
        const std::int64_t& MARKET_SCALE) const override;

    std::unique_ptr<OTPayment> Payment() const override;
    std::unique_ptr<OTPayment> Payment(const String& strPayment) const override;
    std::unique_ptr<OTPayment> Payment(
        const opentxs::Contract& contract) const override;

#if OT_CRYPTO_WITH_BIP39
    OTPaymentCode PaymentCode(const std::string& base58) const override;
    OTPaymentCode PaymentCode(
        const proto::PaymentCode& serialized) const override;
    OTPaymentCode PaymentCode(
        const std::string& seed,
        const std::uint32_t nym,
        const std::uint8_t version,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const override;
#endif

    std::unique_ptr<OTPaymentPlan> PaymentPlan() const override;
    std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const override;
    std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const override;

    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& senderNym,
        const std::string& message) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& senderNym,
        const std::string& payment,
        const bool isPayment) const override;
#if OT_CASH
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& senderNym,
        const std::shared_ptr<blind::Purse> purse) const override;
#endif
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const std::shared_ptr<const PeerRequest> request,
        const std::shared_ptr<const PeerReply> reply,
        const std::uint32_t& version) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const std::shared_ptr<const PeerRequest> request,
        const std::uint32_t& version) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& signerNym,
        const proto::PeerObject& serialized) const override;
    std::unique_ptr<opentxs::PeerObject> PeerObject(
        const ConstNym& recipientNym,
        const Armored& encrypted) const override;

#if OT_CASH
    std::unique_ptr<blind::Purse> Purse(
        const ServerContext& context,
        const identifier::UnitDefinition& unit,
        const blind::Mint& mint,
        const Amount totalValue,
        const proto::CashType type) const override;
    std::unique_ptr<blind::Purse> Purse(
        const proto::Purse& serialized) const override;
    std::unique_ptr<blind::Purse> Purse(
        const Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const proto::CashType type) const override;

#endif  // OT_CASH

    std::unique_ptr<OTScriptable> Scriptable(
        const String& strCronItem) const override;

    std::unique_ptr<OTSignedFile> SignedFile() const override;
    std::unique_ptr<OTSignedFile> SignedFile(
        const String& LOCAL_SUBDIR,
        const String& FILE_NAME) const override;
    std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const String& FILE_NAME) const override;
    std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const char* FILE_NAME) const override;

    std::unique_ptr<OTSmartContract> SmartContract() const override;
    std::unique_ptr<OTSmartContract> SmartContract(
        const Identifier& NOTARY_ID) const override;

    std::unique_ptr<OTTrade> Trade() const override;
    std::unique_ptr<OTTrade> Trade(
        const Identifier& notaryID,
        const Identifier& instrumentDefinitionID,
        const Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const Identifier& currencyId,
        const Identifier& currencyAcctId) const override;

    std::unique_ptr<OTTransactionType> Transaction(
        const String& strCronItem) const override;

    std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        originType theOriginType = originType::not_applicable) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const override;
    // THIS factory only used when loading an abbreviated box receipt
    // (inbox, nymbox, or outbox receipt).
    // The full receipt is loaded only after the abbreviated ones are loaded,
    // and verified against them.
    std::unique_ptr<OTTransaction> Transaction(
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
        NumList* pNumList = nullptr) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const override;
    std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const override;

    ~Factory() override = default;

protected:
    const api::Core& api_;

    Factory(const api::Core& api);

private:
    friend opentxs::Factory;

    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace opentxs::api::implementation
