// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/otx/blind/CashType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/CurrencyContract.hpp"
#include "opentxs/core/contract/SecurityContract.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/ConnectionRequest.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"      // TODO remove
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"  // TODO remove
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Factory;
}  // namespace internal
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
class Script;
class Transaction;
}  // namespace bitcoin

class Block;
class Header;
}  // namespace block
}  // namespace blockchain

namespace crypto
{
class SymmetricProvider;
}  // namespace crypto

namespace display
{
class Definition;
}  // namespace display

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace p2p
{
class Base;
}  // namespace p2p

namespace zeromq
{
class Frame;
class Message;
class Pipeline;
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
class PaymentCode;
class PeerObject;
class PeerReply;
class PeerRequest;
class Purse;
class SymmetricKey;
class UnitDefinition;
}  // namespace proto

class Basket;
class Cheque;
class Item;
class Ledger;
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
class Secret;
class PaymentCode;
}  // namespace opentxs

namespace opentxs::api::session
{
class OPENTXS_EXPORT Factory : virtual public api::Factory
{
public:
    virtual auto Armored() const -> OTArmored = 0;
    virtual auto Armored(const std::string& input) const -> OTArmored = 0;
    virtual auto Armored(const opentxs::Data& input) const -> OTArmored = 0;
    virtual auto Armored(const opentxs::String& input) const -> OTArmored = 0;
    virtual auto Armored(const opentxs::crypto::Envelope& input) const
        -> OTArmored = 0;
    virtual auto AsymmetricKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::Asymmetric::DefaultVersion) const
        -> OTAsymmetricKey = 0;
    OPENTXS_NO_EXPORT virtual auto AsymmetricKey(
        const proto::AsymmetricKey& serialized) const -> OTAsymmetricKey = 0;
    virtual auto BailmentNotice(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentNotice = 0;
    OPENTXS_NO_EXPORT virtual auto BailmentNotice(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTBailmentNotice = 0;
    virtual auto BailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentReply = 0;
    OPENTXS_NO_EXPORT virtual auto BailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTBailmentReply = 0;
    virtual auto BailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentRequest = 0;
    OPENTXS_NO_EXPORT virtual auto BailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTBailmentRequest = 0;
    virtual auto BailmentRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTBailmentRequest = 0;
    virtual auto Basket() const -> std::unique_ptr<opentxs::Basket> = 0;
    virtual auto Basket(
        std::int32_t nCount,
        const Amount& lMinimumTransferAmount) const
        -> std::unique_ptr<opentxs::Basket> = 0;
    virtual auto BasketContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& terms,
        const std::uint64_t weight,
        const core::UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTBasketContract = 0;
    OPENTXS_NO_EXPORT virtual auto BasketContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTBasketContract = 0;
#if OT_BLOCKCHAIN
    virtual auto BitcoinBlock(
        const opentxs::blockchain::Type chain,
        const ReadView bytes) const noexcept
        -> std::shared_ptr<
            const opentxs::blockchain::block::bitcoin::Block> = 0;
    using Transaction_p =
        std::shared_ptr<const opentxs::blockchain::block::bitcoin::Transaction>;
    using AbortFunction = std::function<bool()>;
    virtual auto BitcoinBlock(
        const opentxs::blockchain::block::Header& previous,
        const Transaction_p generationTransaction,
        const std::uint32_t nBits,
        const std::pmr::vector<Transaction_p>& extraTransactions = {},
        const std::int32_t version = 2,
        const AbortFunction abort = {}) const noexcept
        -> std::shared_ptr<
            const opentxs::blockchain::block::bitcoin::Block> = 0;
    using OutputBuilder = std::tuple<
        opentxs::blockchain::Amount,
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>,
        std::pmr::set<opentxs::blockchain::crypto::Key>>;
    virtual auto BitcoinGenerationTransaction(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Height height,
        std::pmr::vector<OutputBuilder>&& outputs,
        const std::string& coinbase = {},
        const std::int32_t version = 1) const noexcept -> Transaction_p = 0;
    virtual auto BitcoinScriptNullData(
        const opentxs::blockchain::Type chain,
        const std::pmr::vector<ReadView>& data) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    virtual auto BitcoinScriptP2MS(
        const opentxs::blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        const std::pmr::vector<const opentxs::crypto::key::EllipticCurve*>&
            publicKeys) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    virtual auto BitcoinScriptP2PK(
        const opentxs::blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    virtual auto BitcoinScriptP2PKH(
        const opentxs::blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    virtual auto BitcoinScriptP2SH(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::bitcoin::Script& script)
        const noexcept -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    virtual auto BitcoinScriptP2WPKH(
        const opentxs::blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    virtual auto BitcoinScriptP2WSH(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::bitcoin::Script& script)
        const noexcept -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    virtual auto BitcoinTransaction(
        const opentxs::blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration,
        const Time& time = Clock::now()) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction> = 0;
    virtual auto BlockchainAddress(
        const opentxs::blockchain::p2p::Protocol protocol,
        const opentxs::blockchain::p2p::Network network,
        const opentxs::Data& bytes,
        const std::uint16_t port,
        const opentxs::blockchain::Type chain,
        const Time lastConnected,
        const std::pmr::set<opentxs::blockchain::p2p::Service>& services,
        const bool incoming = false) const -> OTBlockchainAddress = 0;
    virtual auto BlockchainAddress(
        const opentxs::blockchain::p2p::Address::SerializedType& serialized)
        const -> OTBlockchainAddress = 0;
    virtual auto BlockchainSyncMessage(
        const opentxs::network::zeromq::Message& in) const noexcept
        -> std::unique_ptr<opentxs::network::p2p::Base> = 0;
    using BlockHeaderP = std::unique_ptr<opentxs::blockchain::block::Header>;
    OPENTXS_NO_EXPORT virtual auto BlockHeader(
        const proto::BlockchainBlockHeader& serialized) const
        -> BlockHeaderP = 0;
    virtual auto BlockHeader(const ReadView protobuf) const -> BlockHeaderP = 0;
    virtual auto BlockHeader(
        const opentxs::blockchain::Type type,
        const ReadView native) const -> BlockHeaderP = 0;
    virtual auto BlockHeader(const opentxs::blockchain::block::Block& block)
        const -> BlockHeaderP = 0;
    virtual auto BlockHeaderForUnitTests(
        const opentxs::blockchain::block::Hash& hash,
        const opentxs::blockchain::block::Hash& parent,
        const opentxs::blockchain::block::Height height) const
        -> BlockHeaderP = 0;
#endif  // OT_BLOCKCHAIN
    virtual auto Cheque(const OTTransaction& receipt) const
        -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto Cheque() const -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto Cheque(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<opentxs::Cheque> = 0;
    virtual auto ConnectionReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTConnectionReply = 0;
    OPENTXS_NO_EXPORT virtual auto ConnectionReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTConnectionReply = 0;
    virtual auto ConnectionRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const contract::peer::ConnectionInfoType type,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTConnectionRequest = 0;
    OPENTXS_NO_EXPORT virtual auto ConnectionRequest(
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
        const std::string& shortname,
        const std::string& terms,
        const core::UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTCurrencyContract = 0;
    OPENTXS_NO_EXPORT virtual auto CurrencyContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTCurrencyContract = 0;
    virtual auto Data() const -> OTData = 0;
    virtual auto Data(const opentxs::Armored& input) const -> OTData = 0;
    virtual auto Data(const opentxs::network::zeromq::Frame& input) const
        -> OTData = 0;
    virtual auto Data(const std::uint8_t input) const -> OTData = 0;
    virtual auto Data(const std::uint32_t input) const -> OTData = 0;
    virtual auto Data(const std::string& input, const StringStyle mode) const
        -> OTData = 0;
    virtual auto Data(const std::pmr::vector<unsigned char>& input) const
        -> OTData = 0;
    virtual auto Data(const std::pmr::vector<std::byte>& input) const
        -> OTData = 0;
    virtual auto Data(const ReadView input) const -> OTData = 0;
    virtual auto Envelope() const noexcept -> OTEnvelope = 0;
    virtual auto Envelope(const opentxs::Armored& ciphertext) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto Envelope(
        const opentxs::crypto::Envelope::SerializedType& serialized) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto Envelope(const opentxs::ReadView& serialized) const
        noexcept(false) -> OTEnvelope = 0;
    virtual auto Identifier() const -> OTIdentifier = 0;
    virtual auto Identifier(const opentxs::Contract& contract) const
        -> OTIdentifier = 0;
    virtual auto Identifier(const opentxs::Item& item) const
        -> OTIdentifier = 0;
    virtual auto Identifier(const opentxs::network::zeromq::Frame& bytes) const
        -> OTIdentifier = 0;
    virtual auto IdentifierFromBase58(const std::string& serialized) const
        -> OTIdentifier = 0;
    virtual auto IdentifierFromBase58(const opentxs::String& serialized) const
        -> OTIdentifier = 0;
    virtual auto IdentifierFromBytes(const ReadView bytes) const
        -> OTIdentifier = 0;
    OPENTXS_NO_EXPORT virtual auto InternalSession() const noexcept
        -> const internal::Factory& = 0;
    virtual auto Item(const String& serialized) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(const std::string& serialized) const
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
        const opentxs::Identifier& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const String& strItem,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNumber) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Item(
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> = 0;
    virtual auto Keypair(
        const opentxs::crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::crypto::key::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair = 0;
    OPENTXS_NO_EXPORT virtual auto Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey) const -> OTKeypair = 0;
    OPENTXS_NO_EXPORT virtual auto Keypair(
        const proto::AsymmetricKey& serializedPubkey) const -> OTKeypair = 0;
    virtual auto Keypair(
        const std::string& fingerprint,
        const Bip32Index nym,
        const Bip32Index credset,
        const Bip32Index credindex,
        const EcdsaCurve& curve,
        const opentxs::crypto::key::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair = 0;
    virtual auto Ledger(
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const -> std::unique_ptr<opentxs::Ledger> = 0;
    virtual auto Market() const -> std::unique_ptr<OTMarket> = 0;
    virtual auto Market(const char* szFilename) const
        -> std::unique_ptr<OTMarket> = 0;
    virtual auto Market(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const Amount& lScale) const -> std::unique_ptr<OTMarket> = 0;
    virtual auto Message() const -> std::unique_ptr<opentxs::Message> = 0;
    virtual auto Mint() const noexcept -> otx::blind::Mint = 0;
    virtual auto Mint(const otx::blind::CashType type) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const otx::blind::CashType type,
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const identifier::Server& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto Mint(
        const otx::blind::CashType type,
        const identifier::Server& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint = 0;
    virtual auto NymID() const -> OTNymID = 0;
    virtual auto NymID(const std::string& serialized) const -> OTNymID = 0;
    virtual auto NymID(const opentxs::String& serialized) const -> OTNymID = 0;
    virtual auto NymID(const opentxs::network::zeromq::Frame& bytes) const
        -> OTNymID = 0;
    virtual auto NymIDFromPaymentCode(const std::string& serialized) const
        -> OTNymID = 0;
    virtual auto Offer() const
        -> std::unique_ptr<OTOffer> = 0;  // The constructor
                                          // contains the 3
                                          // variables needed to
                                          // identify any market.
    virtual auto Offer(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const Amount& MARKET_SCALE) const -> std::unique_ptr<OTOffer> = 0;
    virtual auto OutbailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentReply = 0;
    OPENTXS_NO_EXPORT virtual auto OutbailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTOutbailmentReply = 0;
    virtual auto OutbailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const Amount& amount,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentRequest = 0;
    OPENTXS_NO_EXPORT virtual auto OutbailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTOutbailmentRequest = 0;
    virtual auto PasswordPrompt(const std::string& text) const
        -> OTPasswordPrompt = 0;
    virtual auto PasswordPrompt(const opentxs::PasswordPrompt& rhs) const
        -> OTPasswordPrompt = 0;
    virtual auto Payment() const -> std::unique_ptr<OTPayment> = 0;
    virtual auto Payment(const String& strPayment) const
        -> std::unique_ptr<OTPayment> = 0;
    virtual auto Payment(
        const opentxs::Contract& contract,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<OTPayment> = 0;
    virtual auto PaymentCode(const std::string& base58) const noexcept
        -> opentxs::PaymentCode = 0;
    OPENTXS_NO_EXPORT virtual auto PaymentCode(
        const proto::PaymentCode& serialized) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCode(const ReadView& serialized) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentCode(
        const std::string& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto PaymentPlan() const -> std::unique_ptr<OTPaymentPlan> = 0;
    virtual auto PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<OTPaymentPlan> = 0;
    virtual auto PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const opentxs::Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const opentxs::Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const
        -> std::unique_ptr<OTPaymentPlan> = 0;
    virtual auto PeerObject(const Nym_p& senderNym, const std::string& message)
        const -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const Nym_p& senderNym,
        const std::string& payment,
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
    OPENTXS_NO_EXPORT virtual auto PeerObject(
        const Nym_p& signerNym,
        const proto::PeerObject& serialized) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> = 0;
    virtual auto PeerReply() const noexcept -> OTPeerReply = 0;
    OPENTXS_NO_EXPORT virtual auto PeerReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTPeerReply = 0;
    virtual auto PeerReply(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTPeerReply = 0;
    virtual auto PeerRequest() const noexcept -> OTPeerRequest = 0;
    OPENTXS_NO_EXPORT virtual auto PeerRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTPeerRequest = 0;
    virtual auto PeerRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTPeerRequest = 0;
    virtual auto Pipeline(
        std::function<void(opentxs::network::zeromq::Message&&)> callback) const
        -> opentxs::network::zeromq::Pipeline = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const Amount& totalValue,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const Amount& totalValue,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    OPENTXS_NO_EXPORT virtual auto Purse(
        const proto::Purse& serialized) const noexcept -> otx::blind::Purse = 0;
    virtual auto Purse(
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto Purse(
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse = 0;
    virtual auto ReplyAcknowledgement(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const contract::peer::PeerRequestType type,
        const bool& ack,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTReplyAcknowledgement = 0;
    OPENTXS_NO_EXPORT virtual auto ReplyAcknowledgement(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTReplyAcknowledgement = 0;
    virtual auto Scriptable(const String& strCronItem) const
        -> std::unique_ptr<OTScriptable> = 0;
    virtual auto SecurityContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& terms,
        const core::UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTSecurityContract = 0;
    OPENTXS_NO_EXPORT virtual auto SecurityContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTSecurityContract = 0;
    virtual auto ServerContract() const noexcept(false) -> OTServerContract = 0;
    virtual auto ServerID() const -> OTServerID = 0;
    virtual auto ServerID(const std::string& serialized) const
        -> OTServerID = 0;
    virtual auto ServerID(const opentxs::String& serialized) const
        -> OTServerID = 0;
    virtual auto ServerID(const opentxs::network::zeromq::Frame& bytes) const
        -> OTServerID = 0;
    virtual auto SignedFile() const -> std::unique_ptr<OTSignedFile> = 0;
    virtual auto SignedFile(const String& LOCAL_SUBDIR, const String& FILE_NAME)
        const -> std::unique_ptr<OTSignedFile> = 0;
    virtual auto SignedFile(const char* LOCAL_SUBDIR, const String& FILE_NAME)
        const -> std::unique_ptr<OTSignedFile> = 0;
    virtual auto SignedFile(const char* LOCAL_SUBDIR, const char* FILE_NAME)
        const -> std::unique_ptr<OTSignedFile> = 0;
    virtual auto SmartContract() const -> std::unique_ptr<OTSmartContract> = 0;
    virtual auto SmartContract(const identifier::Server& NOTARY_ID) const
        -> std::unique_ptr<OTSmartContract> = 0;
    virtual auto StoreSecret(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const contract::peer::SecretType type,
        const std::string& primary,
        const std::string& secondary,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTStoreSecret = 0;
    OPENTXS_NO_EXPORT virtual auto StoreSecret(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTStoreSecret = 0;
    /** Generate a blank, invalid key */
    virtual auto SymmetricKey() const -> OTSymmetricKey = 0;
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
        const opentxs::PasswordPrompt& password,
        const opentxs::crypto::key::symmetric::Algorithm mode =
            opentxs::crypto::key::symmetric::Algorithm::Error) const
        -> OTSymmetricKey = 0;
    /** Instantiate a symmetric key from serialized form
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] serialized The symmetric key in protobuf form
     */
    OPENTXS_NO_EXPORT virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized) const -> OTSymmetricKey = 0;
    /** Derive a symmetric key from a seed
     *
     *  \param[in] seed A binary or text seed to be expanded into a secret key
     *  \param[in] salt
     *  \param[in] operations The number of iterations/operations the KDF should
     *                        perform
     *  \param[in] difficulty A type-specific difficulty parameter used by the
     *                        KDF.
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
        const opentxs::crypto::key::symmetric::Source type) const
        -> OTSymmetricKey = 0;
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const ReadView salt,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const std::size_t size,
        const opentxs::crypto::key::symmetric::Source type) const
        -> OTSymmetricKey = 0;
    /** Construct a symmetric key from an existing Secret
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] raw An existing, unencrypted binary or text secret
     */
    virtual auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& raw,
        const opentxs::PasswordPrompt& reason) const -> OTSymmetricKey = 0;
    virtual auto Trade() const -> std::unique_ptr<OTTrade> = 0;
    virtual auto Trade(
        const identifier::Server& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const opentxs::Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const opentxs::Identifier& currencyAcctId) const
        -> std::unique_ptr<OTTrade> = 0;
    virtual auto Transaction(const String& strCronItem) const
        -> std::unique_ptr<OTTransactionType> = 0;
    virtual auto Transaction(const opentxs::Ledger& theOwner) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> = 0;
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> = 0;
    // THIS factory only used when loading an abbreviated box receipt (inbox,
    // nymbox, or outbox receipt). The full receipt is loaded only after the
    // abbreviated ones are loaded, and verified against them.
    virtual auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
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
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
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
    virtual auto UnitID() const -> OTUnitID = 0;
    virtual auto UnitID(const std::string& serialized) const -> OTUnitID = 0;
    virtual auto UnitID(const opentxs::String& serialized) const
        -> OTUnitID = 0;
    virtual auto UnitID(const opentxs::network::zeromq::Frame& bytes) const
        -> OTUnitID = 0;
    virtual auto UnitDefinition() const noexcept -> OTUnitDefinition = 0;
    OPENTXS_NO_EXPORT virtual auto UnitDefinition(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTUnitDefinition = 0;

    OPENTXS_NO_EXPORT virtual auto InternalSession() noexcept
        -> internal::Factory& = 0;

    OPENTXS_NO_EXPORT ~Factory() override = default;

protected:
    Factory() = default;

private:
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;
};
}  // namespace opentxs::api::session
