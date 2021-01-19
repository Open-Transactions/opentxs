// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_FACTORY_HPP
#define OPENTXS_API_FACTORY_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/Primitives.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/Blockchain.hpp"
#endif                                   // OT_BLOCKCHAIN
#include "opentxs/blockchain/Types.hpp"  // IWYU pragma: keep
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/p2p/Address.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/String.hpp"
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
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/protobuf/CashEnums.pb.h"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/PeerEnums.pb.h"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace otx
{
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

class Secret;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class Factory : virtual public Primitives
{
public:
    OPENTXS_EXPORT virtual OTArmored Armored() const = 0;
    OPENTXS_EXPORT virtual OTArmored Armored(
        const std::string& input) const = 0;
    OPENTXS_EXPORT virtual OTArmored Armored(
        const opentxs::Data& input) const = 0;
    OPENTXS_EXPORT virtual OTArmored Armored(
        const opentxs::String& input) const = 0;
    OPENTXS_EXPORT virtual OTArmored Armored(
        const opentxs::crypto::Envelope& input) const = 0;
    OPENTXS_EXPORT virtual OTArmored Armored(
        const ProtobufType& input) const = 0;
    OPENTXS_EXPORT virtual OTString Armored(
        const ProtobufType& input,
        const std::string& header) const = 0;
    OPENTXS_EXPORT virtual OTAsymmetricKey AsymmetricKey(
        const NymParameters& params,
        const opentxs::PasswordPrompt& reason,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::Asymmetric::DefaultVersion) const = 0;
    OPENTXS_EXPORT virtual OTAsymmetricKey AsymmetricKey(
        const proto::AsymmetricKey& serialized) const = 0;
    OPENTXS_EXPORT virtual OTBailmentNotice BailmentNotice(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTBailmentNotice BailmentNotice(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTBailmentReply BailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTBailmentReply BailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTBailmentRequest BailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTBailmentRequest BailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) = 0;
#if OT_WITH_BASKETS
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Basket> Basket() const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Basket> Basket(
        std::int32_t nCount,
        std::int64_t lMinimumTransferAmount) const = 0;
    OPENTXS_EXPORT virtual OTBasketContract BasketContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTBasketContract BasketContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false) = 0;
#endif
#if OT_BLOCKCHAIN
    OPENTXS_EXPORT virtual auto BitcoinBlock(
        const opentxs::blockchain::Type chain,
        const ReadView bytes) const noexcept
        -> std::shared_ptr<const blockchain::block::bitcoin::Block> = 0;
    using Transaction_p =
        std::shared_ptr<const opentxs::blockchain::block::bitcoin::Transaction>;
    using AbortFunction = std::function<bool()>;
    OPENTXS_EXPORT virtual auto BitcoinBlock(
        const opentxs::blockchain::block::Header& previous,
        const Transaction_p generationTransaction,
        const std::uint32_t nBits,
        const std::vector<Transaction_p>& extraTransactions = {},
        const std::int32_t version = 1,
        const AbortFunction abort = {}) const noexcept
        -> std::shared_ptr<
            const opentxs::blockchain::block::bitcoin::Block> = 0;
    using OutputBuilder = std::tuple<
        blockchain::Amount,
        std::unique_ptr<const opentxs::blockchain::block::bitcoin::Script>,
        std::set<api::client::blockchain::Key>>;
    OPENTXS_EXPORT virtual auto BitcoinGenerationTransaction(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Height height,
        std::vector<OutputBuilder>&& outputs,
        const std::string& coinbase = {},
        const std::int32_t version = 1) const noexcept -> Transaction_p = 0;
    OPENTXS_EXPORT virtual auto BitcoinScriptNullData(
        const opentxs::blockchain::Type chain,
        const std::vector<ReadView>& data) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    OPENTXS_EXPORT virtual auto BitcoinScriptP2MS(
        const opentxs::blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        const std::vector<const opentxs::crypto::key::EllipticCurve*>&
            publicKeys) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    OPENTXS_EXPORT virtual auto BitcoinScriptP2PK(
        const opentxs::blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    OPENTXS_EXPORT virtual auto BitcoinScriptP2PKH(
        const opentxs::blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    OPENTXS_EXPORT virtual auto BitcoinScriptP2SH(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::bitcoin::Script& script)
        const noexcept -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> = 0;
    OPENTXS_EXPORT virtual auto BitcoinTransaction(
        const opentxs::blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction> = 0;
    OPENTXS_EXPORT virtual OTBlockchainAddress BlockchainAddress(
        const opentxs::blockchain::p2p::Protocol protocol,
        const opentxs::blockchain::p2p::Network network,
        const opentxs::Data& bytes,
        const std::uint16_t port,
        const opentxs::blockchain::Type chain,
        const Time lastConnected,
        const std::set<opentxs::blockchain::p2p::Service>& services,
        const bool incoming = false) const = 0;
    OPENTXS_EXPORT virtual OTBlockchainAddress BlockchainAddress(
        const opentxs::blockchain::p2p::Address::SerializedType& serialized)
        const = 0;
    using BlockHeaderP = std::unique_ptr<opentxs::blockchain::block::Header>;
    OPENTXS_EXPORT virtual BlockHeaderP BlockHeader(
        const proto::BlockchainBlockHeader& serialized) const = 0;
    OPENTXS_EXPORT virtual BlockHeaderP BlockHeader(
        const opentxs::blockchain::Type type,
        const Data& raw) const = 0;
    OPENTXS_EXPORT virtual BlockHeaderP BlockHeader(
        const opentxs::blockchain::block::Block& block) const = 0;
    OPENTXS_EXPORT virtual BlockHeaderP BlockHeaderForUnitTests(
        const opentxs::blockchain::block::Hash& hash,
        const opentxs::blockchain::block::Hash& parent,
        const opentxs::blockchain::block::Height height) const = 0;
#endif  // OT_BLOCKCHAIN
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Cheque> Cheque(
        const OTTransaction& receipt) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Cheque> Cheque() const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Cheque> Cheque(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const = 0;
    OPENTXS_EXPORT virtual OTConnectionReply ConnectionReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTConnectionReply ConnectionReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTConnectionRequest ConnectionRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const proto::ConnectionInfoType type,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTConnectionRequest ConnectionRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Contract> Contract(
        const String& strCronItem) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTCron> Cron() const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTCronItem> CronItem(
        const String& strCronItem) const = 0;
    OPENTXS_EXPORT virtual OTCurrencyContract CurrencyContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t power,
        const std::string& fraction,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTCurrencyContract CurrencyContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTData Data() const = 0;
    OPENTXS_EXPORT virtual OTData Data(const opentxs::Armored& input) const = 0;
    OPENTXS_EXPORT virtual OTData Data(const ProtobufType& input) const = 0;
    OPENTXS_EXPORT virtual OTData Data(
        const opentxs::network::zeromq::Frame& input) const = 0;
    OPENTXS_EXPORT virtual OTData Data(const std::uint8_t input) const = 0;
    OPENTXS_EXPORT virtual OTData Data(const std::uint32_t input) const = 0;
    OPENTXS_EXPORT virtual OTData Data(
        const std::string& input,
        const StringStyle mode) const = 0;
    OPENTXS_EXPORT virtual OTData Data(
        const std::vector<unsigned char>& input) const = 0;
    OPENTXS_EXPORT virtual OTData Data(
        const std::vector<std::byte>& input) const = 0;
    OPENTXS_EXPORT virtual OTData Data(const ReadView input) const = 0;
    OPENTXS_EXPORT virtual OTEnvelope Envelope() const noexcept = 0;
    OPENTXS_EXPORT virtual OTEnvelope Envelope(
        const opentxs::Armored& ciphertext) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTEnvelope Envelope(
        const opentxs::crypto::Envelope::SerializedType& serialized) const
        noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTIdentifier Identifier() const = 0;
    OPENTXS_EXPORT virtual OTIdentifier Identifier(
        const std::string& serialized) const = 0;
    OPENTXS_EXPORT virtual OTIdentifier Identifier(
        const opentxs::String& serialized) const = 0;
    OPENTXS_EXPORT virtual OTIdentifier Identifier(
        const opentxs::Contract& contract) const = 0;
    OPENTXS_EXPORT virtual OTIdentifier Identifier(
        const opentxs::Item& item) const = 0;
    OPENTXS_EXPORT virtual OTIdentifier Identifier(
        const ReadView bytes) const = 0;
    OPENTXS_EXPORT virtual OTIdentifier Identifier(
        const ProtobufType& proto) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const String& serialized) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const std::string& serialized) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const opentxs::Item& theOwner) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const String& strItem,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNumber) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Item> Item(
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const = 0;
    OPENTXS_EXPORT virtual OTKeypair Keypair(
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::KeyRole role,
        const opentxs::PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual OTKeypair Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey) const = 0;
    OPENTXS_EXPORT virtual OTKeypair Keypair(
        const proto::AsymmetricKey& serializedPubkey) const = 0;
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual OTKeypair Keypair(
        const std::string& fingerprint,
        const Bip32Index nym,
        const Bip32Index credset,
        const Bip32Index credindex,
        const EcdsaCurve& curve,
        const proto::KeyRole role,
        const opentxs::PasswordPrompt& reason) const = 0;
#endif  // OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Ledger> Ledger(
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Ledger> Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const = 0;
    
#if OT_WITH_MARKETS
    OPENTXS_EXPORT virtual std::unique_ptr<OTMarket> Market() const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTMarket> Market(
        const char* szFilename) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTMarket> Market(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const std::int64_t& lScale) const = 0;
#endif
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::Message> Message()
        const = 0;
#if OT_CASH
    OPENTXS_EXPORT virtual std::unique_ptr<blind::Mint> Mint() const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strInstrumentDefinitionID) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<blind::Mint> Mint(
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID) const = 0;
#endif
    OPENTXS_EXPORT virtual OTNymID NymID() const = 0;
    OPENTXS_EXPORT virtual OTNymID NymID(
        const std::string& serialized) const = 0;
    OPENTXS_EXPORT virtual OTNymID NymID(
        const opentxs::String& serialized) const = 0;
    OPENTXS_EXPORT virtual OTNymID NymIDFromPaymentCode(
        const std::string& serialized) const = 0;
#if OT_WITH_MARKETS
    OPENTXS_EXPORT virtual std::unique_ptr<OTOffer> Offer()
        const = 0;  // The constructor contains
                    // the 3 variables needed to
                    // identify any market.
    OPENTXS_EXPORT virtual std::unique_ptr<OTOffer> Offer(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const std::int64_t& MARKET_SCALE) const = 0;
#endif
    OPENTXS_EXPORT virtual OTOutbailmentReply OutbailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTOutbailmentReply OutbailmentReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTOutbailmentRequest OutbailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const std::uint64_t& amount,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTOutbailmentRequest OutbailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTPasswordPrompt PasswordPrompt(
        const std::string& text) const = 0;
    OPENTXS_EXPORT virtual OTPasswordPrompt PasswordPrompt(
        const opentxs::PasswordPrompt& rhs) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTPayment> Payment() const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTPayment> Payment(
        const String& strPayment) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTPayment> Payment(
        const opentxs::Contract& contract,
        const opentxs::PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual OTPaymentCode PaymentCode(
        const std::string& base58) const noexcept = 0;
    OPENTXS_EXPORT virtual OTPaymentCode PaymentCode(
        const proto::PaymentCode& serialized) const noexcept = 0;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual OTPaymentCode PaymentCode(
        const std::string& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const noexcept = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
#if OT_WITH_PAYMENT_PLANS
    OPENTXS_EXPORT virtual std::unique_ptr<OTPaymentPlan> PaymentPlan()
        const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTPaymentPlan> PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const opentxs::Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const opentxs::Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const = 0;
#endif
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& message) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment) const = 0;
#if OT_CASH
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse) const = 0;
#endif
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const OTPeerRequest request,
        const VersionNumber version) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& signerNym,
        const proto::PeerObject& serialized) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::PeerObject> PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual OTPeerReply PeerReply() const noexcept = 0;
    OPENTXS_EXPORT virtual OTPeerReply PeerReply(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTPeerRequest PeerRequest() const noexcept = 0;
    OPENTXS_EXPORT virtual OTPeerRequest PeerRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTZMQPipeline Pipeline(
        std::function<void(opentxs::network::zeromq::Message&)> callback)
        const = 0;
#if OT_CASH
    OPENTXS_EXPORT virtual std::unique_ptr<blind::Purse> Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type = proto::CASHTYPE_LUCRE) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<blind::Purse> Purse(
        const proto::Purse& serialized) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<blind::Purse> Purse(
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type = proto::CASHTYPE_LUCRE) const = 0;
#endif  // OT_CASH
    OPENTXS_EXPORT virtual OTReplyAcknowledgement ReplyAcknowledgement(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const proto::PeerRequestType type,
        const bool& ack,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTReplyAcknowledgement ReplyAcknowledgement(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTScriptable> Scriptable(
        const String& strCronItem) const = 0;
    OPENTXS_EXPORT virtual OTSecurityContract SecurityContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTSecurityContract SecurityContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTServerContract ServerContract() const
        noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTServerID ServerID() const = 0;
    OPENTXS_EXPORT virtual OTServerID ServerID(
        const std::string& serialized) const = 0;
    OPENTXS_EXPORT virtual OTServerID ServerID(
        const opentxs::String& serialized) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile() const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile(
        const String& LOCAL_SUBDIR,
        const String& FILE_NAME) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const String& FILE_NAME) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTSignedFile> SignedFile(
        const char* LOCAL_SUBDIR,
        const char* FILE_NAME) const = 0;
#if OT_WITH_SMART_CONTRACTS
    OPENTXS_EXPORT virtual std::unique_ptr<OTSmartContract> SmartContract()
        const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTSmartContract> SmartContract(
        const identifier::Server& NOTARY_ID) const = 0;
#endif
    OPENTXS_EXPORT virtual OTStoreSecret StoreSecret(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const proto::SecretType type,
        const std::string& primary,
        const std::string& secondary,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual OTStoreSecret StoreSecret(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false) = 0;
    /** Generate a blank, invalid key */
    OPENTXS_EXPORT virtual OTSymmetricKey SymmetricKey() const = 0;
    /** Derive a new, random symmetric key
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] password Optional key password information.
     *  \param[in] mode The symmetric algorithm for which to generate an
     *                  appropriate key
     */
    OPENTXS_EXPORT virtual OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const = 0;
    /** Instantiate a symmetric key from serialized form
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] serialized The symmetric key in protobuf form
     */
    OPENTXS_EXPORT virtual OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized) const = 0;
    /** Derive a symmetric key from a seed
     *
     *  \param[in] seed A binary or text seed to be expanded into a secret key
     *  \param[in] operations The number of iterations/operations the KDF should
     *                        perform
     *  \param[in] difficulty A type-specific difficulty parameter used by the
     *                        KDF.
     *  \param[in] size       The target number of bytes for the derived secret
     *                        key
     *  \param[in] type       The KDF to be used for the derivation process
     */
    OPENTXS_EXPORT virtual OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type) const = 0;
    /** Construct a symmetric key from an existing Secret
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] raw An existing, unencrypted binary or text secret
     */
    OPENTXS_EXPORT virtual OTSymmetricKey SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& raw,
        const opentxs::PasswordPrompt& reason) const = 0;
#if OT_WITH_MARKETS
    OPENTXS_EXPORT virtual std::unique_ptr<OTTrade> Trade() const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTTrade> Trade(
        const identifier::Server& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const opentxs::Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const opentxs::Identifier& currencyAcctId) const = 0;
#endif
    OPENTXS_EXPORT virtual std::unique_ptr<OTTransactionType> Transaction(
        const String& strCronItem) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        originType theOriginType = originType::not_applicable) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const = 0;
    // THIS factory only used when loading an abbreviated box receipt (inbox,
    // nymbox, or outbox receipt). The full receipt is loaded only after the
    // abbreviated ones are loaded, and verified against them.
    OPENTXS_EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
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
        const std::int64_t& lAdjustment,
        const std::int64_t& lDisplayValue,
        const std::int64_t& lClosingNum,
        const std::int64_t& lRequestNum,
        bool bReplyTransSuccess,
        NumList* pNumList = nullptr) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<OTTransaction> Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const = 0;
    OPENTXS_EXPORT virtual OTUnitID UnitID() const = 0;
    OPENTXS_EXPORT virtual OTUnitID UnitID(
        const std::string& serialized) const = 0;
    OPENTXS_EXPORT virtual OTUnitID UnitID(
        const opentxs::String& serialized) const = 0;
    OPENTXS_EXPORT virtual OTUnitDefinition UnitDefinition() const noexcept = 0;
    OPENTXS_EXPORT virtual OTUnitDefinition UnitDefinition(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false) = 0;

    ~Factory() override = default;

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
