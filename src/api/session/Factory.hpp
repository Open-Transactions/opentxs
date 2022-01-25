// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Factory.hpp"
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
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/BasketContract.hpp"
#include "opentxs/core/contract/CurrencyContract.hpp"
#include "opentxs/core/contract/SecurityContract.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Unit.hpp"
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
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/crypto/key/symmetric/Source.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"

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
namespace key
{
class EllipticCurve;
class Secp256k1;
}  // namespace key

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
class Nym;
}  // namespace identity

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
class Identifier;
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

namespace opentxs::api::session::imp
{
class Factory : virtual public internal::Factory
{
public:
    auto Armored() const -> OTArmored final;
    auto Armored(const UnallocatedCString& input) const -> OTArmored final;
    auto Armored(const opentxs::Data& input) const -> OTArmored final;
    auto Armored(const opentxs::String& input) const -> OTArmored final;
    auto Armored(const opentxs::crypto::Envelope& input) const
        -> OTArmored final;
    auto Armored(const ProtobufType& input) const -> OTArmored final;
    auto Armored(const ProtobufType& input, const UnallocatedCString& header)
        const -> OTString final;
    auto Asymmetric() const -> const api::crypto::Asymmetric& final
    {
        return asymmetric_;
    }
    auto AsymmetricKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version) const -> OTAsymmetricKey final;
    auto AsymmetricKey(const proto::AsymmetricKey& serialized) const
        -> OTAsymmetricKey final;
    auto BailmentNotice(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const opentxs::Identifier& requestID,
        const UnallocatedCString& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentNotice final;
    auto BailmentNotice(const Nym_p& nym, const proto::PeerRequest& serialized)
        const noexcept(false) -> OTBailmentNotice final;
    auto BailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentReply final;
    auto BailmentReply(const Nym_p& nym, const proto::PeerReply& serialized)
        const noexcept(false) -> OTBailmentReply final;
    auto BailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& server,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentRequest final;
    auto BailmentRequest(const Nym_p& nym, const proto::PeerRequest& serialized)
        const noexcept(false) -> OTBailmentRequest final;
    auto BailmentRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTBailmentRequest final;
    auto Basket() const -> std::unique_ptr<opentxs::Basket> final;
    auto Basket(std::int32_t nCount, const Amount& lMinimumTransferAmount) const
        -> std::unique_ptr<opentxs::Basket> final;
    auto BasketContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const std::uint64_t weight,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTBasketContract final;
    auto BasketContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTBasketContract final;
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
        [[maybe_unused]] const UnallocatedVector<Transaction_p>&
            extraTransactions,
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
        [[maybe_unused]] UnallocatedVector<OutputBuilder>&& outputs,
        [[maybe_unused]] const UnallocatedCString& coinbase,
        [[maybe_unused]] const std::int32_t version) const noexcept
        -> Transaction_p override
    {
        return {};
    }
    auto BitcoinScriptNullData(
        const opentxs::blockchain::Type chain,
        const UnallocatedVector<ReadView>& data) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> final;
    auto BitcoinScriptP2MS(
        const opentxs::blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        const UnallocatedVector<const opentxs::crypto::key::EllipticCurve*>&
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
    auto BitcoinScriptP2WPKH(
        const opentxs::blockchain::Type chain,
        const opentxs::crypto::key::EllipticCurve& publicKey) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> final;
    auto BitcoinScriptP2WSH(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::bitcoin::Script& script)
        const noexcept -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Script> final;
    auto BitcoinTransaction(
        const opentxs::blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration,
        const Time& time) const noexcept
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
        const UnallocatedSet<opentxs::blockchain::p2p::Service>& services,
        const bool incoming) const -> OTBlockchainAddress final;
    auto BlockchainAddress(
        const opentxs::blockchain::p2p::Address::SerializedType& serialized)
        const -> OTBlockchainAddress final;
    auto BlockchainSyncMessage(const opentxs::network::zeromq::Message& in)
        const noexcept -> std::unique_ptr<opentxs::network::p2p::Base> final;
    auto BlockHeader(const proto::BlockchainBlockHeader& serialized) const
        -> BlockHeaderP override
    {
        return {};
    }
    auto BlockHeader(const ReadView protobuf) const -> BlockHeaderP override
    {
        return {};
    }
    auto BlockHeader(
        const opentxs::blockchain::Type type,
        const ReadView native) const -> BlockHeaderP override
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
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<opentxs::Cheque> final;
    auto ConnectionReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Notary& server,
        const bool ack,
        const UnallocatedCString& url,
        const UnallocatedCString& login,
        const UnallocatedCString& password,
        const UnallocatedCString& key,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTConnectionReply final;
    auto ConnectionReply(const Nym_p& nym, const proto::PeerReply& serialized)
        const noexcept(false) -> OTConnectionReply final;
    auto ConnectionRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        const contract::peer::ConnectionInfoType type,
        const identifier::Notary& server,
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
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
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
    auto Data(const UnallocatedCString& input, const StringStyle mode) const
        -> OTData final;
    auto Data(const UnallocatedVector<unsigned char>& input) const
        -> OTData final;
    auto Data(const UnallocatedVector<std::byte>& input) const -> OTData final;
    auto Data(ReadView input) const -> OTData final;
    auto Envelope() const noexcept -> OTEnvelope final;
    auto Envelope(const opentxs::Armored& ciphertext) const noexcept(false)
        -> OTEnvelope final;
    auto Envelope(const opentxs::crypto::Envelope::SerializedType& serialized)
        const noexcept(false) -> OTEnvelope final;
    auto Envelope(const opentxs::ReadView& serialized) const noexcept(false)
        -> OTEnvelope final;
    auto Identifier() const -> OTIdentifier final;
    auto Identifier(const UnallocatedCString& serialized) const
        -> OTIdentifier final;
    auto Identifier(const opentxs::String& serialized) const
        -> OTIdentifier final;
    auto Identifier(const opentxs::Contract& contract) const
        -> OTIdentifier final;
    auto Identifier(const opentxs::Item& item) const -> OTIdentifier final;
    auto Identifier(const ReadView bytes) const -> OTIdentifier final;
    auto Identifier(const ProtobufType& proto) const -> OTIdentifier final;
    auto Identifier(const opentxs::network::zeromq::Frame& bytes) const
        -> OTIdentifier final;
    auto Identifier(const proto::Identifier& in) const noexcept
        -> OTIdentifier final;
    auto Item(const String& serialized) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(const UnallocatedCString& serialized) const
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
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNumber) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(
        const OTTransaction& theOwner,
        itemType theType,
        const opentxs::Identifier& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Keypair(
        const opentxs::crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::crypto::key::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair final;
    auto Keypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey) const -> OTKeypair final;
    auto Keypair(const proto::AsymmetricKey& serializedPubkey) const
        -> OTKeypair final;
    auto Keypair(
        const UnallocatedCString& fingerprint,
        const Bip32Index nym,
        const Bip32Index credset,
        const Bip32Index credindex,
        const EcdsaCurve& curve,
        const opentxs::crypto::key::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair final;
    auto Ledger(
        const opentxs::Identifier& theAccountID,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> final;
    auto Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> final;
    auto Ledger(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAcctID,
        const identifier::Notary& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const
        -> std::unique_ptr<opentxs::Ledger> final;
    auto Market() const -> std::unique_ptr<OTMarket> final;
    auto Market(const char* szFilename) const
        -> std::unique_ptr<OTMarket> final;
    virtual auto Market(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const Amount& lScale) const -> std::unique_ptr<OTMarket> final;
    auto Message() const -> std::unique_ptr<opentxs::Message> final;
    auto Mint() const noexcept -> otx::blind::Mint final;
    auto Mint(const otx::blind::CashType type) const noexcept
        -> otx::blind::Mint final;
    auto Mint(
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint final;
    auto Mint(
        const otx::blind::CashType type,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint final;
    auto Mint(
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint final;
    auto Mint(
        const otx::blind::CashType type,
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit) const noexcept
        -> otx::blind::Mint final;
    auto NymID() const -> OTNymID final;
    auto NymID(const UnallocatedCString& serialized) const -> OTNymID final;
    auto NymID(const opentxs::String& serialized) const -> OTNymID final;
    auto NymID(const opentxs::network::zeromq::Frame& bytes) const
        -> OTNymID final;
    auto NymID(const proto::Identifier& in) const noexcept -> OTNymID final;
    auto NymID(const opentxs::Identifier& in) const noexcept -> OTNymID final;
    auto NymIDFromPaymentCode(const UnallocatedCString& serialized) const
        -> OTNymID final;
    auto Offer() const -> std::unique_ptr<OTOffer> final;
    auto Offer(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const Amount& MARKET_SCALE) const -> std::unique_ptr<OTOffer> final;
    auto OutbailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Notary& server,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentReply final;
    auto OutbailmentReply(const Nym_p& nym, const proto::PeerReply& serialized)
        const noexcept(false) -> OTOutbailmentReply final;
    auto OutbailmentRequest(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const Amount& amount,
        const UnallocatedCString& terms,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTOutbailmentRequest final;
    auto OutbailmentRequest(
        const Nym_p& nym,
        const proto::PeerRequest& serialized) const noexcept(false)
        -> OTOutbailmentRequest final;
    auto PasswordPrompt(const UnallocatedCString& text) const
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
    auto PaymentCode(const UnallocatedCString& base58) const noexcept
        -> opentxs::PaymentCode final;
    auto PaymentCode(const proto::PaymentCode& serialized) const noexcept
        -> opentxs::PaymentCode final;
    auto PaymentCode(const ReadView& serialized) const noexcept
        -> opentxs::PaymentCode final;
    auto PaymentCode(
        const UnallocatedCString& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const opentxs::PasswordPrompt& reason,
        const bool bitmessage,
        const std::uint8_t bitmessageVersion,
        const std::uint8_t bitmessageStream) const noexcept
        -> opentxs::PaymentCode final;
    auto PaymentPlan() const -> std::unique_ptr<OTPaymentPlan> final;
    auto PaymentPlan(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<OTPaymentPlan> final;
    auto PaymentPlan(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const opentxs::Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const opentxs::Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID) const
        -> std::unique_ptr<OTPaymentPlan> final;
    auto PeerObject(const Nym_p& senderNym, const UnallocatedCString& message)
        const -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerObject(
        const Nym_p& senderNym,
        const UnallocatedCString& payment,
        const bool isPayment) const
        -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerObject(const Nym_p& senderNym, otx::blind::Purse&& purse) const
        -> std::unique_ptr<opentxs::PeerObject> override;
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
    auto PeerReply(const Nym_p& nym, const ReadView& view) const noexcept(false)
        -> OTPeerReply final;
    auto PeerRequest() const noexcept -> OTPeerRequest final;
    auto PeerRequest(const Nym_p& nym, const proto::PeerRequest& serialized)
        const noexcept(false) -> OTPeerRequest final;
    auto PeerRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTPeerRequest final;
    auto Pipeline(
        std::function<void(opentxs::network::zeromq::Message&&)> callback) const
        -> opentxs::network::zeromq::Pipeline final;
    auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const Amount& totalValue,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse final;
    auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const Amount& totalValue,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse final;
    auto Purse(const proto::Purse& serialized) const noexcept
        -> otx::blind::Purse final;
    auto Purse(
        const identity::Nym& owner,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse final;
    auto Purse(
        const identity::Nym& owner,
        const identifier::Notary& server,
        const identifier::UnitDefinition& unit,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse final;
    auto ReplyAcknowledgement(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const opentxs::Identifier& request,
        const identifier::Notary& server,
        const contract::peer::PeerRequestType type,
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
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const opentxs::PasswordPrompt& reason,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement) const noexcept(false)
        -> OTSecurityContract final;
    auto SecurityContract(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTSecurityContract final;
    auto ServerContract() const noexcept(false) -> OTServerContract final;
    auto ServerID() const -> OTNotaryID final;
    auto ServerID(const UnallocatedCString& serialized) const
        -> OTNotaryID final;
    auto ServerID(const opentxs::String& serialized) const -> OTNotaryID final;
    auto ServerID(const opentxs::network::zeromq::Frame& bytes) const
        -> OTNotaryID final;
    auto ServerID(const proto::Identifier& in) const noexcept
        -> OTNotaryID final;
    auto ServerID(const opentxs::Identifier& in) const noexcept
        -> OTNotaryID final;
    auto ServerID(const google::protobuf::MessageLite& proto) const
        -> OTIdentifier final;
    auto SignedFile() const -> std::unique_ptr<OTSignedFile> final;
    auto SignedFile(const String& LOCAL_SUBDIR, const String& FILE_NAME) const
        -> std::unique_ptr<OTSignedFile> final;
    auto SignedFile(const char* LOCAL_SUBDIR, const String& FILE_NAME) const
        -> std::unique_ptr<OTSignedFile> final;
    auto SignedFile(const char* LOCAL_SUBDIR, const char* FILE_NAME) const
        -> std::unique_ptr<OTSignedFile> final;
    auto SmartContract() const -> std::unique_ptr<OTSmartContract> final;
    auto SmartContract(const identifier::Notary& NOTARY_ID) const
        -> std::unique_ptr<OTSmartContract> final;
    auto StoreSecret(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const contract::peer::SecretType type,
        const UnallocatedCString& primary,
        const UnallocatedCString& secondary,
        const identifier::Notary& server,
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
        const opentxs::crypto::key::symmetric::Algorithm mode) const
        -> OTSymmetricKey final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized) const -> OTSymmetricKey final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const opentxs::crypto::key::symmetric::Source type) const
        -> OTSymmetricKey final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const ReadView salt,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const std::size_t size,
        const opentxs::crypto::key::symmetric::Source type) const
        -> OTSymmetricKey final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& raw,
        const opentxs::PasswordPrompt& reason) const -> OTSymmetricKey final;
    auto Trade() const -> std::unique_ptr<OTTrade> final;
    auto Trade(
        const identifier::Notary& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const opentxs::Identifier& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const opentxs::Identifier& currencyAcctId) const
        -> std::unique_ptr<OTTrade> final;
    auto Transaction(const String& strCronItem) const
        -> std::unique_ptr<OTTransactionType> final;
    auto Transaction(const opentxs::Ledger& theOwner) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Notary& theNotaryID,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
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
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const opentxs::Identifier& theAccountID,
        const identifier::Notary& theNotaryID,
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
    auto UnitID(const UnallocatedCString& serialized) const -> OTUnitID final;
    auto UnitID(const opentxs::String& serialized) const -> OTUnitID final;
    auto UnitID(const opentxs::network::zeromq::Frame& bytes) const
        -> OTUnitID final;
    auto UnitID(const proto::Identifier& in) const noexcept -> OTUnitID final;
    auto UnitID(const opentxs::Identifier& in) const noexcept -> OTUnitID final;
    auto UnitID(const google::protobuf::MessageLite& proto) const
        -> OTIdentifier final;
    auto UnitDefinition() const noexcept -> OTUnitDefinition final;
    auto UnitDefinition(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTUnitDefinition final;

    ~Factory() override = default;

protected:
    const api::Session& api_;
    const api::Factory& primitives_;
    std::unique_ptr<const api::crypto::Asymmetric> pAsymmetric_;
    const api::crypto::Asymmetric& asymmetric_;
    std::unique_ptr<const api::crypto::Symmetric> pSymmetric_;
    const api::crypto::Symmetric& symmetric_;

    Factory(const api::Session& api);

private:
    auto instantiate_secp256k1(const ReadView key, const ReadView chaincode)
        const noexcept -> std::unique_ptr<opentxs::crypto::key::Secp256k1>;

    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;
};
}  // namespace opentxs::api::session::imp
