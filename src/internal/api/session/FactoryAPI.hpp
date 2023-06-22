// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/FactoryAPI.hpp"

#include <functional>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/CurrencyContract.hpp"
#include "internal/core/contract/SecurityContract.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/BailmentNotice.hpp"
#include "internal/core/contract/peer/BailmentReply.hpp"
#include "internal/core/contract/peer/BailmentRequest.hpp"
#include "internal/core/contract/peer/ConnectionReply.hpp"
#include "internal/core/contract/peer/ConnectionRequest.hpp"
#include "internal/core/contract/peer/FaucetReply.hpp"
#include "internal/core/contract/peer/FaucetRequest.hpp"
#include "internal/core/contract/peer/NoticeAcknowledgement.hpp"
#include "internal/core/contract/peer/OutBailmentReply.hpp"
#include "internal/core/contract/peer/OutBailmentRequest.hpp"
#include "internal/core/contract/peer/PeerObject.hpp"
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/core/contract/peer/StoreSecret.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/otx/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Types.hpp"

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

namespace blockchain
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace blockchain

namespace crypto
{
namespace asymmetric
{
class Key;
}  // namespace asymmetric

namespace symmetric
{
class Key;
}  // namespace symmetric

class SymmetricProvider;
}  // namespace crypto

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
class BlockchainPeerAddress;
class BlockchainTransaction;
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
    virtual auto Asymmetric() const -> const api::crypto::Asymmetric& = 0;
    using session::Factory::AsymmetricKey;
    virtual auto AsymmetricKey(const proto::AsymmetricKey& serialized) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto BailmentNotice(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const identifier::Generic& requestID,
        const UnallocatedCString& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentNotice = 0;
    virtual auto BailmentNotice(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTBailmentNotice = 0;
    virtual auto BailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentReply = 0;
    virtual auto BailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTBailmentReply = 0;
    virtual auto BailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentRequest = 0;
    virtual auto BailmentRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTBailmentRequest = 0;
    virtual auto BailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTBailmentRequest = 0;
    virtual auto Basket() const -> std::unique_ptr<opentxs::Basket> = 0;
    virtual auto Basket(
        std::int32_t nCount,
        const Amount& lMinimumTransferAmount) const
        -> std::unique_ptr<opentxs::Basket> = 0;
    virtual auto BasketContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const std::uint64_t weight,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTBasketContract = 0;
    virtual auto BasketContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTBasketContract = 0;
    using AbortFunction = std::function<bool()>;
    virtual auto BitcoinBlock(
        const blockchain::block::Header& previous,
        blockchain::block::Transaction generationTransaction,
        std::uint32_t nBits,
        std::span<blockchain::block::Transaction> extraTransactions,
        std::int32_t version,
        AbortFunction abort,
        alloc::Default alloc) const noexcept -> blockchain::block::Block = 0;
    using session::Factory::BlockchainAddress;
    virtual auto BlockchainAddress(
        const proto::BlockchainPeerAddress& serialized) const
        -> opentxs::network::blockchain::Address = 0;
    using session::Factory::BlockchainTransaction;
    virtual auto BlockchainTransaction(
        const proto::BlockchainTransaction& serialized,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction = 0;
    virtual auto BlockHeader(
        const proto::BlockchainBlockHeader& proto,
        alloc::Default alloc) const noexcept -> blockchain::block::Header = 0;
    virtual auto BlockHeaderForUnitTests(
        const blockchain::block::Hash& hash,
        const blockchain::block::Hash& parent,
        const blockchain::block::Height height,
        alloc::Default alloc) const noexcept -> blockchain::block::Header = 0;
    virtual auto Cheque(const OTTransaction& receipt) const
        -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto Cheque() const -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto Cheque(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto ConnectionReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const bool ack,
        const UnallocatedCString& url,
        const UnallocatedCString& login,
        const UnallocatedCString& password,
        const UnallocatedCString& key,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTConnectionReply = 0;
    virtual auto ConnectionReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTConnectionReply = 0;
    virtual auto ConnectionRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const contract::peer::ConnectionInfoType type,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTConnectionRequest = 0;
    virtual auto ConnectionRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTConnectionRequest = 0;
    virtual auto Contract(const String& strCronItem) const
        -> std::unique_ptr<opentxs::Contract> = 0;
    virtual auto Cron() const -> std::unique_ptr<OTCron> = 0;
    virtual auto CronItem(const String& strCronItem) const
        -> std::unique_ptr<OTCronItem> = 0;
    virtual auto CurrencyContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTCurrencyContract = 0;
    virtual auto CurrencyContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTCurrencyContract = 0;
    virtual auto Envelope() const noexcept -> OTEnvelope = 0;
    virtual auto Envelope(const opentxs::Armored& ciphertext) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto Envelope(
        const opentxs::crypto::Envelope::SerializedType& serialized) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto Envelope(const opentxs::ReadView& serialized) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto FaucetReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const blockchain::block::Transaction& transaction,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTFaucetReply = 0;
    virtual auto FaucetReply(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTFaucetReply = 0;
    virtual auto FaucetReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTFaucetReply = 0;
    virtual auto FaucetRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        opentxs::UnitType unit,
        std::string_view address,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTFaucetRequest = 0;
    virtual auto FaucetRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTFaucetRequest = 0;
    virtual auto FaucetRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTFaucetRequest = 0;
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
        const identifier::Account& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const String& strItem,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNumber) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const OTTransaction& theOwner,
        itemType theType,
        const identifier::Account& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Keypair(
        const opentxs::crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::crypto::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair = 0;
    virtual auto Keypair(
        const UnallocatedCString& fingerprint,
        const Bip32Index nym,
        const Bip32Index credset,
        const Bip32Index credindex,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair = 0;
    virtual auto Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey) const -> OTKeypair = 0;
    virtual auto Keypair(const proto::AsymmetricKey& serializedPubkey) const
        -> OTKeypair = 0;
    virtual auto Ledger(
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Account& theAcctID,
        const identifier::Notary& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Nym& nymAsAccount,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Nym& nymAsAccount,
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
    virtual auto OutbailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentReply = 0;
    virtual auto OutbailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTOutbailmentReply = 0;
    virtual auto OutbailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const Amount& amount,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentRequest = 0;
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
        const identifier::Account& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const identifier::Account& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const
        -> std::unique_ptr<OTPaymentPlan> = 0;
    virtual auto PeerObject(
        const Nym_p& senderNym,
        const UnallocatedCString& message) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const Nym_p& senderNym,
        const UnallocatedCString& payment,
        const bool isPayment) const -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(const Nym_p& senderNym, otx::blind::Purse&& purse)
        const -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const OTPeerRequest request,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const Nym_p& signerNym,
        const proto::PeerObject& serialized) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerReply() const noexcept -> OTPeerReply = 0;
    virtual auto PeerReply(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTPeerReply = 0;
    virtual auto PeerReply(const Nym_p& nym, const proto::PeerReply& serialized)
        const noexcept(false) -> OTPeerReply = 0;
    virtual auto PeerRequest() const noexcept -> OTPeerRequest = 0;
    virtual auto PeerRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTPeerRequest = 0;
    virtual auto PeerRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTPeerRequest = 0;
    using session::Factory::Purse;
    virtual auto Purse(const proto::Purse& serialized) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto ReplyAcknowledgement(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const identifier::Notary& server,
        const contract::peer::PeerRequestType type,
        const bool& ack,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTReplyAcknowledgement = 0;
    virtual auto ReplyAcknowledgement(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTReplyAcknowledgement = 0;
    virtual auto SecurityContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTSecurityContract = 0;
    virtual auto SecurityContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTSecurityContract = 0;
    virtual auto ServerContract() const noexcept(false) -> OTServerContract = 0;
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
    virtual auto StoreSecret(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const contract::peer::SecretType type,
        const UnallocatedCString& primary,
        const UnallocatedCString& secondary,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTStoreSecret = 0;
    virtual auto StoreSecret(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTStoreSecret = 0;
    virtual auto Symmetric() const -> const api::crypto::Symmetric& = 0;
    /** Derive a new, random symmetric key
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] password Optional key password information.
     *  \param[in] mode The symmetric algorithm for which to generate an
     *                  appropriate key
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::crypto::symmetric::Algorithm mode,
        const opentxs::PasswordPrompt& password,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    /** Derive a symmetric key from a seed
     *
     *  \param[in] engine     Symmetric provider compatible with the specified
     *                        key type
     *  \param[in] seed       A binary or text seed to be expanded into a secret
     *                        key
     *  \param[in] operations The number of iterations/operations the KDF should
     *                        perform
     *  \param[in] difficulty A type-specific difficulty parameter used by the
     *                        KDF
     *  \param[in] size       The target number of bytes for the derived secret
     *                        key
     *  \param[in] type       The KDF to be used for the derivation process
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const opentxs::crypto::symmetric::Source type,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    /** Derive a symmetric key from a seed
     *
     *  \param[in] engine     Symmetric provider compatible with the specified
     *                        key type
     *  \param[in] seed       A binary or text seed to be expanded into a secret
     *                        key
     *  \param[in] salt       Extra data to pass to the KDF
     *  \param[in] operations The number of iterations/operations the KDF should
     *                        perform
     *  \param[in] difficulty A type-specific difficulty parameter used by the
     *                        KDF
     *  \param[in] parallel   A type-specific difficulty parameter used by some
     *                        KDFs
     *  \param[in] size       The target number of bytes for the derived secret
     *                        key
     *  \param[in] type       The KDF to be used for the derivation process
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const ReadView salt,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const std::size_t size,
        const opentxs::crypto::symmetric::Source type,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    /** Construct a symmetric key from an existing Secret
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] raw An existing, unencrypted binary or text secret
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& raw,
        const opentxs::PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    /** Instantiate a symmetric key from serialized form
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] serialized The symmetric key in protobuf form
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized,
        alloc::Default alloc) const -> opentxs::crypto::symmetric::Key = 0;
    virtual auto Trade() const -> std::unique_ptr<OTTrade> = 0;
    virtual auto Trade(
        const identifier::Notary& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const identifier::Account& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const identifier::Account& currencyAcctId) const
        -> std::unique_ptr<OTTrade> = 0;
    virtual auto Transaction(const String& strCronItem) const
        -> std::unique_ptr<OTTransactionType> = 0;
    virtual auto Transaction(const opentxs::Ledger& theOwner) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> = 0;
    // THIS factory only used when loading an abbreviated box receipt (inbox,
    // nymbox, or outbox receipt). The full receipt is loaded only after the
    // abbreviated ones are loaded, and verified against them.
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
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
        const identifier::Account& theAccountID,
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
    virtual auto UnitDefinition() const noexcept -> OTUnitDefinition = 0;
    virtual auto UnitDefinition(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTUnitDefinition = 0;

    auto InternalSession() noexcept -> Factory& final { return *this; }

    ~Factory() override = default;
};
}  // namespace opentxs::api::session::internal
