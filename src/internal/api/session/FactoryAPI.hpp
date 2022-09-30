// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/FactoryAPI.hpp"

#include "internal/otx/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace google
{
namespace protobuf
{
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Symmetric;
}  // namespace crypto
}  // namespace api

namespace identifier
{
class Generic;
class Nym;
class Notary;
class Unit;
}  // namespace identifier

namespace proto
{
class AsymmetricKey;
class BlockchainBlockHeader;
class HDPath;
class PaymentCode;
class PeerObject;
class PeerReply;
class PeerRequest;
class Purse;
class SymmetricKey;
class UnitDefinition;
}  // namespace proto

class Amount;
class Basket;
class ByteArray;
class Cheque;
class Contract;
class Item;
class Ledger;
class Message;
class NumList;
class OTCron;
class OTCronItem;
class OTMarket;
class OTOffer;
class OTPayment;
class OTPaymentPlan;
class OTScriptable;
class OTSignedFile;
class OTSmartContract;
class OTTrade;
class OTTransaction;
class OTTransactionType;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class Factory : virtual public api::session::Factory,
                virtual public api::internal::Factory
{
public:
    using session::Factory::Armored;
    virtual auto Armored(const google::protobuf::MessageLite& input) const
        -> OTArmored = 0;
    virtual auto Armored(
        const google::protobuf::MessageLite& input,
        const UnallocatedCString& header) const -> OTString = 0;
    virtual auto Asymmetric() const -> const api::crypto::Asymmetric& = 0;
    using session::Factory::AsymmetricKey;
    virtual auto AsymmetricKey(const proto::AsymmetricKey& serialized) const
        -> OTAsymmetricKey = 0;
    using session::Factory::BailmentNotice;
    virtual auto BailmentNotice(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTBailmentNotice = 0;
    using session::Factory::BailmentReply;
    virtual auto BailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTBailmentReply = 0;
    using session::Factory::BailmentRequest;
    virtual auto BailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTBailmentRequest = 0;
    virtual auto Basket() const -> std::unique_ptr<opentxs::Basket> = 0;
    virtual auto Basket(
        std::int32_t nCount,
        const Amount& lMinimumTransferAmount) const
        -> std::unique_ptr<opentxs::Basket> = 0;
    using session::Factory::BasketContract;
    virtual auto BasketContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTBasketContract = 0;
    using session::Factory::BlockHeader;
    virtual auto BlockHeader(const proto::BlockchainBlockHeader& serialized)
        const -> BlockHeaderP = 0;
    virtual auto Cheque(const OTTransaction& receipt) const
        -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto Cheque() const -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto Cheque(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<opentxs::Cheque> = 0;
    using session::Factory::ConnectionReply;
    virtual auto ConnectionReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTConnectionReply = 0;
    using session::Factory::ConnectionRequest;
    virtual auto ConnectionRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTConnectionRequest = 0;
    virtual auto Contract(const String& strCronItem) const
        -> std::unique_ptr<opentxs::Contract> = 0;
    virtual auto Cron() const -> std::unique_ptr<OTCron> = 0;
    virtual auto CronItem(const String& strCronItem) const
        -> std::unique_ptr<OTCronItem> = 0;
    using session::Factory::CurrencyContract;
    virtual auto CurrencyContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTCurrencyContract = 0;
    using session::Factory::Data;
    virtual auto Data(const google::protobuf::MessageLite& input) const
        -> ByteArray = 0;
    auto InternalSession() const noexcept -> const Factory& final
    {
        return *this;
    }
    virtual auto Item(const String& serialized) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(const UnallocatedCString& serialized) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const identifier::Nym& theNymID,
        const opentxs::Item& theOwner) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const identifier::Generic& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const String& strItem,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNumber) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const OTTransaction& theOwner,
        itemType theType,
        const identifier::Generic& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> = 0;
    using session::Factory::Keypair;
    virtual auto Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey) const -> OTKeypair = 0;
    virtual auto Keypair(const proto::AsymmetricKey& serializedPubkey) const
        -> OTKeypair = 0;
    virtual auto Ledger(
        const identifier::Generic& theAccountID,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Generic& theAccountID,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Generic& theAcctID,
        const identifier::Notary& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Market() const -> std::unique_ptr<OTMarket> = 0;
    virtual auto Market(const char* szFilename) const
        -> std::unique_ptr<OTMarket> = 0;
    virtual auto Market(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const Amount& lScale) const -> std::unique_ptr<OTMarket> = 0;
    virtual auto Message() const -> std::unique_ptr<opentxs::Message> = 0;
    using session::Factory::OutbailmentReply;
    virtual auto OutbailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTOutbailmentReply = 0;
    using session::Factory::OutbailmentRequest;
    virtual auto OutbailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTOutbailmentRequest = 0;
    virtual auto Offer() const
        -> std::unique_ptr<OTOffer> = 0;  // The constructor
                                          // contains the 3
                                          // variables needed to
                                          // identify any market.
    virtual auto Offer(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const Amount& MARKET_SCALE) const -> std::unique_ptr<OTOffer> = 0;
    virtual auto Payment() const -> std::unique_ptr<OTPayment> = 0;
    virtual auto Payment(const String& strPayment) const
        -> std::unique_ptr<OTPayment> = 0;
    virtual auto Payment(
        const opentxs::Contract& contract,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<OTPayment> = 0;
    using session::Factory::PaymentCode;
    virtual auto PaymentCode(const proto::PaymentCode& serialized)
        const noexcept -> opentxs::PaymentCode = 0;
    virtual auto PaymentPlan() const -> std::unique_ptr<OTPaymentPlan> = 0;
    virtual auto PaymentPlan(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<OTPaymentPlan> = 0;
    virtual auto PaymentPlan(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::Generic& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const identifier::Generic& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const
        -> std::unique_ptr<OTPaymentPlan> = 0;
    using session::Factory::PeerObject;
    virtual auto PeerObject(
        const Nym_p& signerNym,
        const proto::PeerObject& serialized) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    using session::Factory::PeerReply;
    virtual auto PeerReply(const Nym_p& nym, const proto::PeerReply& serialized)
        const noexcept(false) -> OTPeerReply = 0;
    using session::Factory::PeerRequest;
    virtual auto PeerRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTPeerRequest = 0;
    using session::Factory::Purse;
    virtual auto Purse(const proto::Purse& serialized) const noexcept
        -> otx::blind::Purse = 0;
    using session::Factory::ReplyAcknowledgement;
    virtual auto ReplyAcknowledgement(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTReplyAcknowledgement = 0;
    using session::Factory::SecurityContract;
    virtual auto SecurityContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTSecurityContract = 0;
    virtual auto Scriptable(const String& strCronItem) const
        -> std::unique_ptr<OTScriptable> = 0;
    virtual auto SignedFile() const -> std::unique_ptr<OTSignedFile> = 0;
    virtual auto SignedFile(const String& LOCAL_SUBDIR, const String& FILE_NAME)
        const -> std::unique_ptr<OTSignedFile> = 0;
    virtual auto SignedFile(const char* LOCAL_SUBDIR, const String& FILE_NAME)
        const -> std::unique_ptr<OTSignedFile> = 0;
    virtual auto SignedFile(const char* LOCAL_SUBDIR, const char* FILE_NAME)
        const -> std::unique_ptr<OTSignedFile> = 0;
    virtual auto SmartContract() const -> std::unique_ptr<OTSmartContract> = 0;
    virtual auto SmartContract(const identifier::Notary& NOTARY_ID) const
        -> std::unique_ptr<OTSmartContract> = 0;
    using session::Factory::StoreSecret;
    virtual auto StoreSecret(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTStoreSecret = 0;
    virtual auto Symmetric() const -> const api::crypto::Symmetric& = 0;
    using session::Factory::SymmetricKey;
    /** Instantiate a symmetric key from serialized form
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] serialized The symmetric key in protobuf form
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized) const -> OTSymmetricKey = 0;
    virtual auto Trade() const -> std::unique_ptr<OTTrade> = 0;
    virtual auto Trade(
        const identifier::Notary& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const identifier::Generic& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const identifier::Generic& currencyAcctId) const
        -> std::unique_ptr<OTTrade> = 0;
    virtual auto Transaction(const String& strCronItem) const
        -> std::unique_ptr<OTTransactionType> = 0;
    virtual auto Transaction(const opentxs::Ledger& theOwner) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Generic& theAccountID,
        const identifier::Notary& theNotaryID,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Generic& theAccountID,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> = 0;
    // THIS factory only used when loading an abbreviated box receipt (inbox,
    // nymbox, or outbox receipt). The full receipt is loaded only after the
    // abbreviated ones are loaded, and verified against them.
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Generic& theAccountID,
        const identifier::Notary& theNotaryID,
        const std::int64_t& lNumberOfOrigin,
        originType theOriginType,
        const std::int64_t& lTransactionNum,
        const std::int64_t& lInRefTo,
        const std::int64_t& lInRefDisplay,
        const Time the_DATE_SIGNED,
        transactionType theType,
        const String& strHash,
        const Amount& lAdjustment,
        const Amount& lDisplayValue,
        const std::int64_t& lClosingNum,
        const std::int64_t& lRequestNum,
        bool bReplyTransSuccess,
        NumList* pNumList = nullptr) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Generic& theAccountID,
        const identifier::Notary& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> = 0;
    using session::Factory::UnitDefinition;
    virtual auto UnitDefinition(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTUnitDefinition = 0;

    auto InternalSession() noexcept -> Factory& final { return *this; }

    ~Factory() override = default;
};
}  // namespace opentxs::api::session::internal
