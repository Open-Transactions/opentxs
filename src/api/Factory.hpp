// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "internal/api/Api.hpp"
#include "internal/api/crypto/Crypto.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Primitives.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/Secret.hpp"
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
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/crypto/Types.hpp"
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
namespace blind
{
class Mint;
class Purse;
}  // namespace blind

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
class Script;
}  // namespace bitcoin

class Block;
class Header;
}  // namespace block
}  // namespace blockchain

namespace crypto
{
namespace key
{
class EllipticCurve;
class Secp256k1;
}  // namespace key

class SymmetricProvider;
}  // namespace crypto

namespace identity
{
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
class Contract;
class Message;
class NumList;
class NymParameters;
class OTCron;
class OTCronItem;
class OTMarket;
class OTOffer;
class OTPassword;
class OTPayment;
class OTPaymentPlan;
class OTScriptable;
class OTSignedFile;
class OTSmartContract;
class OTTrade;
class OTTransactionType;
class PeerObject;
}  // namespace opentxs

namespace opentxs::api::implementation
{
class Factory : virtual public api::internal::Factory
{
public:
    auto Armored() const -> OTArmored final;
    auto Armored(const std::string& input) const -> OTArmored final;
    auto Armored(const opentxs::Data& input) const -> OTArmored final;
    auto Armored(const opentxs::String& input) const -> OTArmored final;
    auto Armored(const opentxs::crypto::Envelope& input) const
        -> OTArmored final;
    auto Armored(const ProtobufType& input) const -> OTArmored final;
    auto Armored(const ProtobufType& input, const std::string& header) const
        -> OTString final;
    auto Asymmetric() const -> const api::crypto::internal::Asymmetric& final
    {
        return asymmetric_;
    }
    auto AsymmetricKey(
        const NymParameters& params,
        const opentxs::PasswordPrompt& reason,
        const proto::KeyRole role,
        const VersionNumber version) const -> OTAsymmetricKey final;
    auto AsymmetricKey(const proto::AsymmetricKey& serialized) const
        -> OTAsymmetricKey final;
    auto BailmentNotice(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const opentxs::Identifier& requestID,
        const std::string& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentNotice final;
    auto BailmentNotice(const Nym_p& nym, const proto::PeerRequest& serialized)
        const noexcept(false) -> OTBailmentNotice final;
    auto BailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentReply final;
    auto BailmentReply(const Nym_p& nym, const proto::PeerReply& serialized)
        const noexcept(false) -> OTBailmentReply final;
    auto BailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentRequest final;
    auto BailmentRequest(const Nym_p& nym, const proto::PeerRequest& serialized)
        const noexcept(false) -> OTBailmentRequest final;
#if OT_WITH_BASKETS
    auto Basket() const -> std::unique_ptr<opentxs::Basket> final;
    auto Basket(std::int32_t nCount, std::int64_t lMinimumTransferAmount) const
        -> std::unique_ptr<opentxs::Basket> final;
    auto BasketContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version) const noexcept(false)
        -> OTBasketContract final;
    auto BasketContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTBasketContract final;
#endif
#if OT_BLOCKCHAIN
    auto BitcoinBlock(
        const opentxs::blockchain::Type chain,
        const ReadView bytes) const noexcept
        -> std::shared_ptr<
            const opentxs::blockchain::block::bitcoin::Block> override
    {
        return {};
    }
    auto BitcoinBlock(
        [[maybe_unused]] const opentxs::blockchain::block::Header& previous,
        [[maybe_unused]] const Transaction_p generationTransaction,
        [[maybe_unused]] const std::uint32_t nBits,
        [[maybe_unused]] const std::vector<Transaction_p>& extraTransactions,
        [[maybe_unused]] const std::int32_t version,
        [[maybe_unused]] const AbortFunction abort) const noexcept
        -> std::shared_ptr<
            const opentxs::blockchain::block::bitcoin::Block> override
    {
        return {};
    }
    auto BitcoinGenerationTransaction(
        [[maybe_unused]] const opentxs::blockchain::Type chain,
        [[maybe_unused]] const opentxs::blockchain::block::Height height,
        [[maybe_unused]] std::vector<OutputBuilder>&& outputs,
        [[maybe_unused]] const std::string& coinbase,
        [[maybe_unused]] const std::int32_t version) const noexcept
        -> Transaction_p override
    {
        return {};
    }
    auto BitcoinScriptNullData(
        const opentxs::blockchain::Type chain,
        const std::vector<ReadView>& data) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> final;
    auto BitcoinScriptP2MS(
        const opentxs::blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        const std::vector<const opentxs::crypto::key::EllipticCurve*>&
            publicKeys) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> final;
    auto BitcoinScriptP2PK(
        const opentxs::blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> final;
    auto BitcoinScriptP2PKH(
        const opentxs::blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> final;
    auto BitcoinScriptP2SH(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::bitcoin::Script& script)
        const noexcept -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> final;
    auto BitcoinTransaction(
        const opentxs::blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction> override
    {
        return {};
    }
    auto BlockchainAddress(
        const opentxs::blockchain::p2p::Protocol protocol,
        const opentxs::blockchain::p2p::Network network,
        const opentxs::Data& bytes,
        const std::uint16_t port,
        const opentxs::blockchain::Type chain,
        const Time lastConnected,
        const std::set<opentxs::blockchain::p2p::Service>& services,
        const bool incoming) const -> OTBlockchainAddress final;
    auto BlockchainAddress(
        const opentxs::blockchain::p2p::Address::SerializedType& serialized)
        const -> OTBlockchainAddress final;
    auto BlockHeader(const proto::BlockchainBlockHeader& serialized) const
        -> BlockHeaderP override
    {
        return {};
    }
    auto BlockHeader(
        const opentxs::blockchain::Type type,
        const opentxs::Data& raw) const -> BlockHeaderP override
    {
        return {};
    }
    auto BlockHeader(const opentxs::blockchain::block::Block& block) const
        -> BlockHeaderP override
    {
        return {};
    }
    auto BlockHeaderForUnitTests(
        const opentxs::blockchain::block::Hash& hash,
        const opentxs::blockchain::block::Hash& parent,
        const opentxs::blockchain::block::Height height) const
        -> BlockHeaderP override
    {
        return {};
    }
#endif  // OT_BLOCKCHAIN
    auto Cheque(const OTTransaction& receipt) const
        -> std::unique_ptr<opentxs::Cheque> final;
    auto Cheque() const -> std::unique_ptr<opentxs::Cheque> final;
    auto Cheque(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<opentxs::Cheque> final;
    auto ConnectionReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTConnectionReply final;
    auto ConnectionReply(const Nym_p& nym, const proto::PeerReply& serialized)
        const noexcept(false) -> OTConnectionReply final;
    auto ConnectionRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const proto::ConnectionInfoType type,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTConnectionRequest final;
    auto ConnectionRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTConnectionRequest final;
    auto Contract(const String& strCronItem) const
        -> std::unique_ptr<opentxs::Contract> final;
    auto Cron() const -> std::unique_ptr<OTCron> override;
    auto CronItem(const String& strCronItem) const
        -> std::unique_ptr<OTCronItem> final;
    auto CurrencyContract(
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
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTCurrencyContract final;
    auto CurrencyContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTCurrencyContract final;
    auto Data() const -> OTData final;
    auto Data(const opentxs::Armored& input) const -> OTData final;
    auto Data(const ProtobufType& input) const -> OTData final;
    auto Data(const opentxs::network::zeromq::Frame& input) const
        -> OTData final;
    auto Data(const std::uint8_t input) const -> OTData final;
    auto Data(const std::uint32_t input) const -> OTData final;
    auto Data(const std::string& input, const StringStyle mode) const
        -> OTData final;
    auto Data(const std::vector<unsigned char>& input) const -> OTData final;
    auto Data(const std::vector<std::byte>& input) const -> OTData final;
    auto Data(ReadView input) const -> OTData final;
    auto Envelope() const noexcept -> OTEnvelope final;
    auto Envelope(const opentxs::Armored& ciphertext) const noexcept(false)
        -> OTEnvelope final;
    auto Envelope(const opentxs::crypto::Envelope::SerializedType& serialized)
        const noexcept(false) -> OTEnvelope final;
    auto Identifier() const -> OTIdentifier final;
    auto Identifier(const std::string& serialized) const -> OTIdentifier final;
    auto Identifier(const opentxs::String& serialized) const
        -> OTIdentifier final;
    auto Identifier(const opentxs::Contract& contract) const
        -> OTIdentifier final;
    auto Identifier(const opentxs::Item& item) const -> OTIdentifier final;
    auto Identifier(const ReadView bytes) const -> OTIdentifier final;
    auto Identifier(const ProtobufType& proto) const -> OTIdentifier final;
    auto Item(const String& serialized) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(const std::string& serialized) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(const identifier::Nym& theNymID, const opentxs::Item& theOwner)
        const -> std::unique_ptr<opentxs::Item> final;
    auto Item(const identifier::Nym& theNymID, const OTTransaction& theOwner)
        const -> std::unique_ptr<opentxs::Item> final;
    auto Item(
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(
        const String& strItem,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNumber) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Keypair(
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::KeyRole role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair final;
    auto Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey) const -> OTKeypair final;
    auto Keypair(const proto::AsymmetricKey& serializedPubkey) const
        -> OTKeypair final;
#if OT_CRYPTO_WITH_BIP32
    auto Keypair(
        const std::string& fingerprint,
        const Bip32Index nym,
        const Bip32Index credset,
        const Bip32Index credindex,
        const EcdsaCurve& curve,
        const proto::KeyRole role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto Ledger(
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> final;
    auto Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> final;
    auto Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAcctID,
        const identifier::Server& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const
        -> std::unique_ptr<opentxs::Ledger> final;
#if OT_WITH_MARKETS
    auto Market() const -> std::unique_ptr<OTMarket> final;
    auto Market(const char* szFilename) const
        -> std::unique_ptr<OTMarket> final;
    virtual auto Market(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const std::int64_t& lScale) const -> std::unique_ptr<OTMarket> final;
#endif
    auto Message() const -> std::unique_ptr<opentxs::Message> final;
#if OT_CASH
    auto Mint() const -> std::unique_ptr<blind::Mint> final;
    auto Mint(
        const String& strNotaryID,
        const String& strInstrumentDefinitionID) const
        -> std::unique_ptr<blind::Mint> final;
    auto Mint(
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID) const
        -> std::unique_ptr<blind::Mint> final;
#endif
    auto NymID() const -> OTNymID final;
    auto NymID(const std::string& serialized) const -> OTNymID final;
    auto NymID(const opentxs::String& serialized) const -> OTNymID final;
    auto NymIDFromPaymentCode(const std::string& serialized) const
        -> OTNymID final;
#if OT_WITH_MARKETS
    auto Offer() const -> std::unique_ptr<OTOffer> final;
    auto Offer(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const std::int64_t& MARKET_SCALE) const
        -> std::unique_ptr<OTOffer> final;
#endif
    auto OutbailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentReply final;
    auto OutbailmentReply(const Nym_p& nym, const proto::PeerReply& serialized)
        const noexcept(false) -> OTOutbailmentReply final;
    auto OutbailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Server& serverID,
        const std::uint64_t& amount,
        const std::string& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentRequest final;
    auto OutbailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTOutbailmentRequest final;
    auto PasswordPrompt(const std::string& text) const
        -> OTPasswordPrompt final;
    auto PasswordPrompt(const opentxs::PasswordPrompt& rhs) const
        -> OTPasswordPrompt final
    {
        return PasswordPrompt(rhs.GetDisplayString());
    }
    auto Payment() const -> std::unique_ptr<OTPayment> final;
    auto Payment(const String& strPayment) const
        -> std::unique_ptr<OTPayment> final;
    auto Payment(
        const opentxs::Contract& contract,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<OTPayment> final;
    auto PaymentCode(const std::string& base58) const noexcept
        -> OTPaymentCode final;
    auto PaymentCode(const proto::PaymentCode& serialized) const noexcept
        -> OTPaymentCode final;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    auto PaymentCode(
        const std::string& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage,
        const std::uint8_t bitmessageVersion,
        const std::uint8_t bitmessageStream) const noexcept
        -> OTPaymentCode final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
#if OT_WITH_PAYMENT_PLANS
    auto PaymentPlan() const -> std::unique_ptr<OTPaymentPlan> final;
    auto PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<OTPaymentPlan> final;
    auto PaymentPlan(
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const opentxs::Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const opentxs::Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const
        -> std::unique_ptr<OTPaymentPlan> final;
#endif
    auto PeerObject(const Nym_p& senderNym, const std::string& message) const
        -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerObject(
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment) const
        -> std::unique_ptr<opentxs::PeerObject> override;
#if OT_CASH
    auto PeerObject(
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse) const
        -> std::unique_ptr<opentxs::PeerObject> override;
#endif
    auto PeerObject(
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerObject(const OTPeerRequest request, const VersionNumber version)
        const -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerObject(const Nym_p& signerNym, const proto::PeerObject& serialized)
        const -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerReply() const noexcept -> OTPeerReply final;
    auto PeerReply(const Nym_p& nym, const proto::PeerReply& serialized) const
        noexcept(false) -> OTPeerReply final;
    auto PeerRequest() const noexcept -> OTPeerRequest final;
    auto PeerRequest(const Nym_p& nym, const proto::PeerRequest& serialized)
        const noexcept(false) -> OTPeerRequest final;
    auto Pipeline(
        std::function<void(opentxs::network::zeromq::Message&)> callback) const
        -> OTZMQPipeline final;
#if OT_CASH
    auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const blind::Mint& mint,
        const Amount totalValue,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type) const
        -> std::unique_ptr<blind::Purse> final;
    auto Purse(const proto::Purse& serialized) const
        -> std::unique_ptr<blind::Purse> final;
    auto Purse(
        const identity::Nym& owner,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason,
        const proto::CashType type) const
        -> std::unique_ptr<blind::Purse> final;
#endif  // OT_CASH
    auto ReplyAcknowledgement(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Server& server,
        const proto::PeerRequestType type,
        const bool& ack,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTReplyAcknowledgement final;
    auto ReplyAcknowledgement(
        const Nym_p& nym,
        const proto::PeerReply& serialized) const noexcept(false)
        -> OTReplyAcknowledgement final;
    auto Scriptable(const String& strCronItem) const
        -> std::unique_ptr<OTScriptable> final;
    auto Secret(const std::size_t bytes) const noexcept -> OTSecret final
    {
        return primitives_.Secret(bytes);
    }
    auto SecretFromBytes(const ReadView bytes) const noexcept -> OTSecret final
    {
        return primitives_.SecretFromBytes(bytes);
    }
    auto SecretFromText(std::string_view text) const noexcept -> OTSecret final
    {
        return primitives_.SecretFromText(text);
    }
    auto SecurityContract(
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const proto::ContactItemType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTSecurityContract final;
    auto SecurityContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTSecurityContract final;
    auto ServerContract() const noexcept(false) -> OTServerContract final;
    auto ServerID() const -> OTServerID final;
    auto ServerID(const std::string& serialized) const -> OTServerID final;
    auto ServerID(const opentxs::String& serialized) const -> OTServerID final;
    auto SignedFile() const -> std::unique_ptr<OTSignedFile> final;
    auto SignedFile(const String& LOCAL_SUBDIR, const String& FILE_NAME) const
        -> std::unique_ptr<OTSignedFile> final;
    auto SignedFile(const char* LOCAL_SUBDIR, const String& FILE_NAME) const
        -> std::unique_ptr<OTSignedFile> final;
    auto SignedFile(const char* LOCAL_SUBDIR, const char* FILE_NAME) const
        -> std::unique_ptr<OTSignedFile> final;
#if OT_WITH_SMART_CONTRACTS
    auto SmartContract() const -> std::unique_ptr<OTSmartContract> final;
    auto SmartContract(const identifier::Server& NOTARY_ID) const
        -> std::unique_ptr<OTSmartContract> final;
#endif
    auto StoreSecret(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const proto::SecretType type,
        const std::string& primary,
        const std::string& secondary,
        const identifier::Server& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTStoreSecret final;
    auto StoreSecret(const Nym_p& nym, const proto::PeerRequest& serialized)
        const noexcept(false) -> OTStoreSecret final;
    auto Symmetric() const -> const api::crypto::Symmetric& final
    {
        return symmetric_;
    }
    auto SymmetricKey() const -> OTSymmetricKey final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        const proto::SymmetricMode mode) const -> OTSymmetricKey final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized) const -> OTSymmetricKey final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type) const -> OTSymmetricKey final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& raw,
        const opentxs::PasswordPrompt& reason) const -> OTSymmetricKey final;
#if OT_WITH_MARKETS
    auto Trade() const -> std::unique_ptr<OTTrade> final;
    auto Trade(
        const identifier::Server& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const opentxs::Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const opentxs::Identifier& currencyAcctId) const
        -> std::unique_ptr<OTTrade> final;
#endif
    auto Transaction(const String& strCronItem) const
        -> std::unique_ptr<OTTransactionType> final;
    auto Transaction(const opentxs::Ledger& theOwner) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
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
        NumList* pNumList = nullptr) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Server& theNotaryID,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const opentxs::Ledger& theOwner,
        transactionType theType,
        originType theOriginType = originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> final;
    auto UnitID() const -> OTUnitID final;
    auto UnitID(const std::string& serialized) const -> OTUnitID final;
    auto UnitID(const opentxs::String& serialized) const -> OTUnitID final;
    auto UnitDefinition() const noexcept -> OTUnitDefinition final;
    auto UnitDefinition(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTUnitDefinition final;

    ~Factory() override = default;

protected:
    const api::internal::Core& api_;
    const api::Primitives& primitives_;
    std::unique_ptr<const api::crypto::internal::Asymmetric> pAsymmetric_;
    const api::crypto::internal::Asymmetric& asymmetric_;
    std::unique_ptr<const api::crypto::Symmetric> pSymmetric_;
    const api::crypto::Symmetric& symmetric_;

    Factory(const api::internal::Core& api);

private:
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto instantiate_secp256k1(const ReadView key) const noexcept
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1>;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;
};
}  // namespace opentxs::api::implementation
