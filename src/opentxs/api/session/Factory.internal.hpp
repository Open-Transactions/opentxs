// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/Factory.internal.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/CurrencyContract.hpp"
#include "internal/core/contract/SecurityContract.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/Object.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Hash;
class Header;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Script;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

namespace contract
{
namespace peer
{
class Reply;
class Request;
}  // namespace peer
}  // namespace contract

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
}  // namespace key

class Key;
}  // namespace asymmetric

namespace symmetric
{
class Key;
}  // namespace symmetric

class Parameters;
class SymmetricProvider;
}  // namespace crypto

namespace display
{
class Definition;
}  // namespace display

namespace identifier
{
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
namespace wot
{
class Claim;
class Verification;
}  // namespace wot

class Nym;
}  // namespace identity

namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network

namespace otx
{
namespace blind
{
class Mint;
class Purse;
}  // namespace blind

namespace context
{
class Server;
}  // namespace context
}  // namespace otx

namespace proto
{
class AsymmetricKey;
class BlockchainBlockHeader;
class BlockchainTransaction;
class Claim;
class ContactItem;
class PaymentCode;
class PeerObject;
class PeerReply;
class PeerRequest;
class Purse;
class SymmetricKey;
class UnitDefinition;
class Verification;
class VerificationItem;
}  // namespace proto

class Amount;
class Armored;
class Basket;
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
class PasswordPrompt;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class Factory : virtual public api::internal::Factory
{
public:
    virtual auto Asymmetric() const -> const api::crypto::Asymmetric& = 0;
    virtual auto AsymmetricKey(
        VersionNumber version,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        VersionNumber version,
        opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::Secret& key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        VersionNumber version,
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        VersionNumber version,
        opentxs::crypto::asymmetric::Role role,
        opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::Secret& key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(const proto::AsymmetricKey& serialized)
        const noexcept -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::Secret& key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto AsymmetricKey(
        opentxs::crypto::asymmetric::Role role,
        opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::Secret& key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto BailmentNoticeReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        bool value,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::BailmentNotice = 0;
    virtual auto BailmentNoticeRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& notary,
        const identifier::Generic& inReferenceToRequest,
        std::string_view description,
        const opentxs::Amount& amount,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::BailmentNotice = 0;
    virtual auto BailmentReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        std::string_view terms,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Bailment = 0;
    virtual auto BailmentRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& notary,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Bailment = 0;
    virtual auto Basket() const -> std::unique_ptr<opentxs::Basket> = 0;
    virtual auto Basket(
        std::int32_t nCount,
        const opentxs::Amount& lMinimumTransferAmount) const
        -> std::unique_ptr<opentxs::Basket> = 0;
    virtual auto BasketContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const std::uint64_t weight,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const opentxs::Amount& redemptionIncrement) const noexcept(false)
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
    virtual auto BitcoinScriptNullData(
        const blockchain::Type chain,
        std::span<const ReadView> data,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2MS(
        const blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2PK(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2PKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2SH(
        const blockchain::Type chain,
        const blockchain::protocol::bitcoin::base::block::Script& script,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2WPKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BitcoinScriptP2WSH(
        const blockchain::Type chain,
        const blockchain::protocol::bitcoin::base::block::Script& script,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script = 0;
    virtual auto BlockHeader(
        const proto::BlockchainBlockHeader& proto,
        alloc::Default alloc) const noexcept -> blockchain::block::Header = 0;
    virtual auto BlockHeaderForUnitTests(
        const blockchain::block::Hash& hash,
        const blockchain::block::Hash& parent,
        const blockchain::block::Height height,
        alloc::Default alloc) const noexcept -> blockchain::block::Header = 0;
    virtual auto BlockHeaderFromNative(
        const blockchain::Type type,
        const ReadView bytes,
        alloc::Default alloc) const noexcept -> blockchain::block::Header = 0;
    virtual auto BlockHeaderFromProtobuf(
        const ReadView bytes,
        alloc::Default alloc) const noexcept -> blockchain::block::Header = 0;
    virtual auto BlockchainBlock(
        const blockchain::Type chain,
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept
        -> blockchain::block::Block = 0;
    virtual auto BlockchainSyncMessage(
        const opentxs::network::zeromq::Message& in) const noexcept
        -> std::unique_ptr<opentxs::network::otdht::Base> = 0;
    virtual auto BlockchainTransaction(
        const blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration,
        const Time time,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction = 0;
    virtual auto BlockchainTransaction(
        const blockchain::Type chain,
        const blockchain::block::Height height,
        std::span<blockchain::OutputBuilder> outputs,
        ReadView coinbase,
        std::int32_t version,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction = 0;
    virtual auto BlockchainTransaction(
        const proto::BlockchainTransaction& serialized,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction = 0;
    virtual auto Cheque() const -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto Cheque(const OTTransaction& receipt) const
        -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto Cheque(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto Claim(ReadView serialized, alloc::Strategy alloc = {})
        const noexcept -> identity::wot::Claim = 0;
    virtual auto Claim(
        const identity::wot::Claimant& claimant,
        const identity::wot::claim::SectionType section,
        const proto::ContactItem& proto,
        alloc::Strategy alloc = {}) const noexcept -> identity::wot::Claim = 0;
    virtual auto Claim(
        const identity::wot::Claimant& claimant,
        identity::wot::claim::SectionType section,
        identity::wot::claim::ClaimType type,
        ReadView value,
        ReadView subtype,
        std::span<const identity::wot::claim::Attribute> attributes,
        Time start,
        Time stop,
        VersionNumber version,
        alloc::Strategy alloc) const noexcept -> identity::wot::Claim = 0;
    virtual auto Claim(
        const identity::Nym& claimant,
        identity::wot::claim::SectionType section,
        identity::wot::claim::ClaimType type,
        ReadView value,
        ReadView subtype,
        std::span<const identity::wot::claim::Attribute> attributes,
        Time start,
        Time stop,
        VersionNumber version,
        alloc::Strategy alloc) const noexcept -> identity::wot::Claim = 0;
    virtual auto Claim(const proto::Claim& proto, alloc::Strategy alloc = {})
        const noexcept -> identity::wot::Claim = 0;
    virtual auto ConnectionReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        bool accepted,
        std::string_view url,
        std::string_view login,
        std::string_view password,
        std::string_view key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Connection = 0;
    virtual auto ConnectionRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const contract::peer::ConnectionInfoType kind,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Connection = 0;
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
        const opentxs::Amount& redemptionIncrement) const noexcept(false)
        -> OTCurrencyContract = 0;
    virtual auto CurrencyContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTCurrencyContract = 0;
    virtual auto Envelope() const noexcept -> OTEnvelope = 0;
    virtual auto Envelope(const opentxs::Armored& ciphertext) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto Envelope(const opentxs::ReadView& serialized) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto Envelope(
        const opentxs::crypto::Envelope::SerializedType& serialized) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto FaucetReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        const blockchain::block::Transaction& transaction,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Faucet = 0;
    virtual auto FaucetRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        opentxs::UnitType unit,
        std::string_view address,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Faucet = 0;
    virtual auto Item(
        const OTTransaction& theOwner,
        otx::itemType theType,
        const identifier::Account& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(const String& serialized) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const String& strItem,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNumber) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(const UnallocatedCString& serialized) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        otx::itemType theType,
        const identifier::Account& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const identifier::Nym& theNymID,
        const opentxs::Item& theOwner) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Keypair(
        const opentxs::crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::crypto::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair = 0;
    virtual auto Keypair(
        const opentxs::crypto::SeedID& fingerprint,
        const opentxs::crypto::Bip32Index nym,
        const opentxs::crypto::Bip32Index credset,
        const opentxs::crypto::Bip32Index credindex,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair = 0;
    virtual auto Keypair(const proto::AsymmetricKey& serializedPubkey) const
        -> OTKeypair = 0;
    virtual auto Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey) const -> OTKeypair = 0;
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
        otx::ledgerType theType,
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
        otx::ledgerType theType,
        bool bCreateFile = false) const -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Market() const -> std::unique_ptr<OTMarket> = 0;
    virtual auto Market(const char* szFilename) const
        -> std::unique_ptr<OTMarket> = 0;
    virtual auto Market(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const opentxs::Amount& lScale) const -> std::unique_ptr<OTMarket> = 0;
    virtual auto Message() const -> std::unique_ptr<opentxs::Message> = 0;
    virtual auto Mint() const noexcept -> otx::blind::Mint = 0;
    virtual auto Mint(
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(const otx::blind::CashType type) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const otx::blind::CashType type,
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const otx::blind::CashType type,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto NymIDFromPaymentCode(
        const UnallocatedCString& serialized) const -> identifier::Nym = 0;
    virtual auto Offer() const -> std::unique_ptr<OTOffer> = 0;
    virtual auto Offer(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const opentxs::Amount& MARKET_SCALE) const
        -> std::unique_ptr<OTOffer> = 0;
    virtual auto OutbailmentReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        std::string_view description,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Outbailment = 0;
    virtual auto OutbailmentRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& notary,
        const opentxs::Amount& amount,
        std::string_view instructions,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Outbailment = 0;
    virtual auto PasswordPrompt(const opentxs::PasswordPrompt& rhs) const
        -> opentxs::PasswordPrompt = 0;
    virtual auto PasswordPrompt(std::string_view text) const
        -> opentxs::PasswordPrompt = 0;
    virtual auto Payment() const -> std::unique_ptr<OTPayment> = 0;
    virtual auto Payment(const String& strPayment) const
        -> std::unique_ptr<OTPayment> = 0;
    virtual auto Payment(
        const opentxs::Contract& contract,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<OTPayment> = 0;
    virtual auto PaymentCode(
        const opentxs::crypto::SeedID& seed,
        const opentxs::crypto::Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCode(const proto::PaymentCode& serialized)
        const noexcept -> opentxs::PaymentCode = 0;
    virtual auto PaymentCodeFromBase58(const ReadView base58) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCodeFromProtobuf(const ReadView proto) const noexcept
        -> opentxs::PaymentCode = 0;
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
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
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
        const Nym_p& signerNym,
        const proto::PeerObject& serialized) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const contract::peer::Request& request,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const contract::peer::Request& request,
        const contract::peer::Reply& reply,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerReply(ReadView bytes, alloc::Strategy alloc = {})
        const noexcept -> contract::peer::Reply = 0;
    virtual auto PeerReply(
        const opentxs::network::zeromq::Frame& bytes,
        alloc::Strategy alloc = {}) const noexcept -> contract::peer::Reply = 0;
    virtual auto PeerReply(
        const proto::PeerReply& proto,
        alloc::Strategy alloc = {}) const noexcept -> contract::peer::Reply = 0;
    virtual auto PeerRequest(ReadView bytes, alloc::Strategy alloc = {})
        const noexcept -> contract::peer::Request = 0;
    virtual auto PeerRequest(
        const opentxs::network::zeromq::Frame& bytes,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::Request = 0;
    virtual auto PeerRequest(
        const proto::PeerRequest& proto,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::Request = 0;
    virtual auto Purse(
        const identity::Nym& owner,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const identity::Nym& owner,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const opentxs::Amount& totalValue,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const opentxs::Amount& totalValue,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(const proto::Purse& serialized) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Scriptable(const String& strCronItem) const
        -> std::unique_ptr<OTScriptable> = 0;
    virtual auto SecurityContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const opentxs::Amount& redemptionIncrement) const noexcept(false)
        -> OTSecurityContract = 0;
    virtual auto SecurityContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTSecurityContract = 0;
    virtual auto ServerContract() const noexcept(false) -> OTServerContract = 0;
    auto Session() const noexcept
        -> const api::session::internal::Factory& final
    {
        return *this;
    }
    virtual auto SessionPublic() const noexcept
        -> const api::session::Factory& = 0;
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
    virtual auto StoreSecretReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        bool value,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::StoreSecret = 0;
    virtual auto StoreSecretRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const contract::peer::SecretType kind,
        std::span<const std::string_view> data,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::StoreSecret = 0;
    virtual auto Symmetric() const -> const api::crypto::Symmetric& = 0;
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& raw,
        const opentxs::PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
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
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const opentxs::crypto::symmetric::Source type,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::crypto::symmetric::Algorithm mode,
        const opentxs::PasswordPrompt& password,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key = 0;
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
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        const std::int64_t& lNumberOfOrigin,
        otx::originType theoriginType,
        const std::int64_t& lTransactionNum,
        const std::int64_t& lInRefTo,
        const std::int64_t& lInRefDisplay,
        const Time the_DATE_SIGNED,
        otx::transactionType theType,
        const String& strHash,
        const opentxs::Amount& lAdjustment,
        const opentxs::Amount& lDisplayValue,
        const std::int64_t& lClosingNum,
        const std::int64_t& lRequestNum,
        bool bReplyTransSuccess,
        NumList* pNumList = nullptr) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        otx::originType theoriginType = otx::originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNum,
        otx::originType theoriginType = otx::originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        otx::transactionType theType,
        otx::originType theoriginType = otx::originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(const opentxs::Ledger& theOwner) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const opentxs::Ledger& theOwner,
        otx::transactionType theType,
        otx::originType theoriginType = otx::originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto UnitDefinition() const noexcept -> OTUnitDefinition = 0;
    virtual auto UnitDefinition(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTUnitDefinition = 0;
    virtual auto Verification(ReadView serialized, alloc::Strategy alloc = {})
        const noexcept -> identity::wot::Verification = 0;
    virtual auto Verification(
        const identifier::Nym& verifier,
        const opentxs::PasswordPrompt& reason,
        identity::wot::ClaimID claim,
        identity::wot::verification::Type value,
        Time start = {},
        Time stop = {},
        std::span<const identity::wot::VerificationID> superscedes = {},
        alloc::Strategy alloc = {}) const noexcept
        -> identity::wot::Verification = 0;
    virtual auto Verification(
        const identifier::Nym& verifier,
        const proto::VerificationItem& proto,
        alloc::Strategy alloc = {}) const noexcept
        -> identity::wot::Verification = 0;
    virtual auto Verification(
        const identity::Nym& verifier,
        const opentxs::PasswordPrompt& reason,
        identity::wot::ClaimID claim,
        identity::wot::verification::Type value,
        Time start = {},
        Time stop = {},
        std::span<const identity::wot::VerificationID> superscedes = {},
        alloc::Strategy alloc = {}) const noexcept
        -> identity::wot::Verification = 0;
    virtual auto Verification(
        const proto::Verification& proto,
        alloc::Strategy alloc = {}) const noexcept
        -> identity::wot::Verification = 0;
    virtual auto VerificationReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        const std::optional<identity::wot::Verification>& response,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::reply::Verification = 0;
    virtual auto VerificationRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identity::wot::Claim& claim,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc = {}) const noexcept
        -> contract::peer::request::Verification = 0;

    auto Session() noexcept -> api::session::internal::Factory& final
    {
        return *this;
    }
    virtual auto SessionPublic() noexcept -> api::session::Factory& = 0;

    ~Factory() override = default;
};
}  // namespace opentxs::api::session::internal
