// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/Key.hpp"
// IWYU pragma: no_include "opentxs/crypto/symmetric/Key.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/FactoryAPI.hpp"
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
#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/core/contract/peer/StoreSecret.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
}  // namespace key
}  // namespace asymmetric

class Parameters;
class SymmetricProvider;
}  // namespace crypto

namespace display
{
class Definition;
}  // namespace display

namespace identity
{
class Nym;
}  // namespace identity

namespace network
{
namespace otdht
{
class Base;
}  // namespace otdht

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
class BlockchainPeerAddress;
class BlockchainTransaction;
class HDPath;
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
class Data;
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
class OTTransactionType;
class PeerObject;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class Factory : virtual public internal::Factory
{
public:
    auto AccountID(
        const identity::wot::claim::ClaimType type,
        const proto::HDPath& path,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountID(type, path, std::move(alloc));
    }
    auto AccountID(const proto::Identifier& in, allocator_type alloc)
        const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountID(in, std::move(alloc));
    }
    auto AccountID(const opentxs::Contract& contract, allocator_type alloc)
        const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountID(contract, std::move(alloc));
    }
    auto AccountIDConvertSafe(
        const identifier::Generic& in,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountIDConvertSafe(in, std::move(alloc));
    }
    auto AccountIDFromBase58(
        const std::string_view base58,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromBase58(base58, std::move(alloc));
    }
    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromHash(bytes, subtype, std::move(alloc));
    }
    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromHash(
            bytes, subtype, type, std::move(alloc));
    }
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromPreimage(
            preimage, subtype, std::move(alloc));
    }
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromPreimage(
            preimage, subtype, type, std::move(alloc));
    }
    auto AccountIDFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromProtobuf(bytes, std::move(alloc));
    }
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromRandom(subtype, std::move(alloc));
    }
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromRandom(subtype, type, std::move(alloc));
    }
    auto AccountIDFromZMQ(
        const opentxs::network::zeromq::Frame& frame,
        allocator_type alloc) const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountIDFromZMQ(frame, std::move(alloc));
    }
    auto AccountIDFromZMQ(const ReadView frame, allocator_type alloc)
        const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountIDFromZMQ(frame, std::move(alloc));
    }
    auto Armored() const -> OTArmored final
    {
        return parent_.Internal().Armored();
    }
    auto Armored(const UnallocatedCString& input) const -> OTArmored final
    {
        return parent_.Internal().Armored(input);
    }
    auto Armored(const opentxs::Data& input) const -> OTArmored final
    {
        return parent_.Internal().Armored(input);
    }
    auto Armored(const opentxs::String& input) const -> OTArmored final
    {
        return parent_.Internal().Armored(input);
    }
    auto Armored(const opentxs::crypto::Envelope& input) const
        -> OTArmored final
    {
        return parent_.Internal().Armored(input);
    }
    auto Armored(const ProtobufType& input) const -> OTArmored final
    {
        return parent_.Internal().Armored(input);
    }
    auto Armored(const ProtobufType& input, const UnallocatedCString& header)
        const -> OTString final
    {
        return parent_.Internal().Armored(input, header);
    }
    auto Asymmetric() const -> const api::crypto::Asymmetric& final
    {
        return asymmetric_;
    }
    auto AsymmetricKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        VersionNumber version,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        VersionNumber version,
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(const proto::AsymmetricKey& serialized) const
        -> opentxs::crypto::asymmetric::Key final;
    auto BailmentNotice(
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& serverID,
        const identifier::Generic& requestID,
        const UnallocatedCString& txid,
        const Amount& amount,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTBailmentNotice final;
    auto BailmentNotice(const Nym_p& nym, const proto::PeerRequest& serialized)
        const noexcept(false) -> OTBailmentNotice final;
    auto BailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
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
    auto BitcoinBlock(
        const blockchain::block::Header& previous,
        blockchain::block::Transaction generationTransaction,
        std::uint32_t nBits,
        std::span<blockchain::block::Transaction> extraTransactions,
        std::int32_t version,
        AbortFunction abort,
        alloc::Default alloc) const noexcept -> blockchain::block::Block final;
    auto BitcoinScriptNullData(
        const blockchain::Type chain,
        std::span<const ReadView> data,
        alloc::Default alloc) const noexcept
        -> blockchain::bitcoin::block::Script final;
    auto BitcoinScriptP2MS(
        const blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
        alloc::Default alloc) const noexcept
        -> blockchain::bitcoin::block::Script final;
    auto BitcoinScriptP2PK(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::bitcoin::block::Script final;
    auto BitcoinScriptP2PKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::bitcoin::block::Script final;
    auto BitcoinScriptP2SH(
        const blockchain::Type chain,
        const blockchain::bitcoin::block::Script& script,
        alloc::Default alloc) const noexcept
        -> blockchain::bitcoin::block::Script final;
    auto BitcoinScriptP2WPKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::bitcoin::block::Script final;
    auto BitcoinScriptP2WSH(
        const blockchain::Type chain,
        const blockchain::bitcoin::block::Script& script,
        alloc::Default alloc) const noexcept
        -> blockchain::bitcoin::block::Script final;
    auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const -> opentxs::network::blockchain::Address final;
    auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const
        -> opentxs::network::blockchain::Address final;
    auto BlockchainAddress(const proto::BlockchainPeerAddress& serialized) const
        -> opentxs::network::blockchain::Address final;
    auto BlockchainBlock(
        const blockchain::Type chain,
        const ReadView bytes,
        alloc::Default alloc) const noexcept -> blockchain::block::Block final;
    auto BlockchainSyncMessage(const opentxs::network::zeromq::Message& in)
        const noexcept -> std::unique_ptr<opentxs::network::otdht::Base> final;
    auto BlockchainTransaction(
        const blockchain::Type chain,
        const ReadView bytes,
        const bool isGeneration,
        const Time time,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction final;
    auto BlockchainTransaction(
        const blockchain::Type chain,
        const blockchain::block::Height height,
        std::span<blockchain::OutputBuilder> outputs,
        ReadView coinbase,
        std::int32_t version,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction final;
    auto BlockchainTransaction(
        const proto::BlockchainTransaction& serialized,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction final;
    auto BlockHeader(
        const proto::BlockchainBlockHeader& proto,
        alloc::Default alloc) const noexcept -> blockchain::block::Header final;
    auto BlockHeaderForUnitTests(
        const blockchain::block::Hash& hash,
        const blockchain::block::Hash& parent,
        const blockchain::block::Height height,
        alloc::Default alloc) const noexcept -> blockchain::block::Header final;
    auto BlockHeaderFromNative(
        const blockchain::Type type,
        const ReadView bytes,
        alloc::Default alloc) const noexcept -> blockchain::block::Header final;
    auto BlockHeaderFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> blockchain::block::Header final;
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
        const identifier::Generic& request,
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
    auto Data() const -> ByteArray final { return parent_.Data(); }
    auto Data(const opentxs::Armored& input) const -> ByteArray final
    {
        return parent_.Data(input);
    }
    auto Data(const ProtobufType& input) const -> ByteArray final
    {
        return parent_.Internal().Data(input);
    }
    auto Data(const opentxs::network::zeromq::Frame& input) const
        -> ByteArray final
    {
        return parent_.Data(input);
    }
    auto Data(const std::uint8_t input) const -> ByteArray final
    {
        return parent_.Data(input);
    }
    auto Data(const std::uint32_t input) const -> ByteArray final
    {
        return parent_.Data(input);
    }
    auto Data(const UnallocatedVector<unsigned char>& input) const
        -> ByteArray final
    {
        return parent_.Data(input);
    }
    auto Data(const UnallocatedVector<std::byte>& input) const
        -> ByteArray final
    {
        return parent_.Data(input);
    }
    auto DataFromBytes(ReadView input) const -> ByteArray final
    {
        return parent_.DataFromBytes(input);
    }
    auto DataFromHex(ReadView input) const -> ByteArray final
    {
        return parent_.DataFromHex(input);
    }
    auto Envelope() const noexcept -> OTEnvelope final;
    auto Envelope(const opentxs::Armored& ciphertext) const noexcept(false)
        -> OTEnvelope final;
    auto Envelope(const opentxs::crypto::Envelope::SerializedType& serialized)
        const noexcept(false) -> OTEnvelope final;
    auto Envelope(const opentxs::ReadView& serialized) const noexcept(false)
        -> OTEnvelope final;
    auto FaucetReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
        const blockchain::block::Transaction& transaction,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTFaucetReply final;
    auto FaucetReply(const Nym_p& nym, const proto::PeerReply& serialized) const
        noexcept(false) -> OTFaucetReply final;
    auto FaucetReply(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTFaucetReply final;
    auto FaucetRequest(
        const Nym_p& nym,
        const identifier::Nym& recipient,
        opentxs::UnitType unit,
        std::string_view address,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> OTFaucetRequest final;
    auto FaucetRequest(const Nym_p& nym, const proto::PeerRequest& serialized)
        const noexcept(false) -> OTFaucetRequest final;
    auto FaucetRequest(const Nym_p& nym, const ReadView& view) const
        noexcept(false) -> OTFaucetRequest final;
    auto Identifier(const opentxs::Contract& contract, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto Identifier(const opentxs::Cheque& cheque, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto Identifier(const opentxs::Item& item, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto Identifier(const proto::Identifier& in, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromBase58(
        const std::string_view base58,
        allocator_type alloc) const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromBase58(base58, std::move(alloc));
    }
    auto IdentifierFromHash(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromHash(bytes, std::move(alloc));
    }
    auto IdentifierFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromHash(bytes, type, std::move(alloc));
    }
    auto IdentifierFromPreimage(const ReadView preimage, allocator_type alloc)
        const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromPreimage(preimage, std::move(alloc));
    }
    auto IdentifierFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromPreimage(preimage, type, std::move(alloc));
    }
    auto IdentifierFromPreimage(const ProtobufType& proto, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(
        const ProtobufType& proto,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromProtobuf(bytes, std::move(alloc));
    }
    auto IdentifierFromRandom(allocator_type alloc) const noexcept
        -> identifier::Generic final
    {
        return parent_.IdentifierFromRandom(std::move(alloc));
    }
    auto IdentifierFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromRandom(type, std::move(alloc));
    }
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
        const identifier::Account& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(
        const String& strItem,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNumber) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(
        const OTTransaction& theOwner,
        itemType theType,
        const identifier::Account& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Keypair(
        const opentxs::crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::crypto::asymmetric::Role role,
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
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair final;
    auto Ledger(
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> final;
    auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> final;
    auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Account& theAcctID,
        const identifier::Notary& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false) const
        -> std::unique_ptr<opentxs::Ledger> final;
    auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Nym& nymAsAccount,
        const identifier::Notary& theNotaryID) const
        -> std::unique_ptr<opentxs::Ledger> final;
    auto Ledger(
        const identifier::Nym& theNymID,
        const identifier::Nym& nymAsAccount,
        const identifier::Notary& theNotaryID,
        ledgerType theType,
        bool bCreateFile) const -> std::unique_ptr<opentxs::Ledger> final;
    auto Market() const -> std::unique_ptr<OTMarket> final;
    auto Market(const char* szFilename) const
        -> std::unique_ptr<OTMarket> final;
    auto Market(
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
    auto NotaryID(const proto::Identifier& in, allocator_type alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDConvertSafe(
        const identifier::Generic& in,
        allocator_type alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromBase58(const std::string_view base58, allocator_type alloc)
        const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromBase58(base58, std::move(alloc));
    }
    auto NotaryIDFromHash(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromHash(bytes, std::move(alloc));
    }
    auto NotaryIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromHash(bytes, type, std::move(alloc));
    }
    auto NotaryIDFromPreimage(const ReadView preimage, allocator_type alloc)
        const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromPreimage(preimage, std::move(alloc));
    }
    auto NotaryIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromPreimage(preimage, type, std::move(alloc));
    }
    auto NotaryIDFromPreimage(
        const ProtobufType& proto,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(const ProtobufType& proto, allocator_type alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromProtobuf(bytes, std::move(alloc));
    }
    auto NotaryIDFromRandom(allocator_type alloc) const noexcept
        -> identifier::Notary final
    {
        return parent_.NotaryIDFromRandom(std::move(alloc));
    }
    auto NotaryIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromRandom(type, std::move(alloc));
    }
    auto NymID(const proto::Identifier& in, allocator_type alloc) const noexcept
        -> identifier::Nym final
    {
        return parent_.Internal().NymID(in, std::move(alloc));
    }
    auto NymIDConvertSafe(const identifier::Generic& in, allocator_type alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.Internal().NymIDConvertSafe(in, std::move(alloc));
    }
    auto NymIDFromBase58(const std::string_view base58, allocator_type alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromBase58(base58, std::move(alloc));
    }
    auto NymIDFromHash(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromHash(bytes, std::move(alloc));
    }
    auto NymIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromHash(bytes, type, std::move(alloc));
    }
    auto NymIDFromPaymentCode(const UnallocatedCString& serialized) const
        -> identifier::Nym final;
    auto NymIDFromPreimage(const ReadView preimage, allocator_type alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromPreimage(preimage, std::move(alloc));
    }
    auto NymIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromPreimage(preimage, type, std::move(alloc));
    }
    auto NymIDFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromProtobuf(bytes, std::move(alloc));
    }
    auto NymIDFromRandom(allocator_type alloc) const noexcept
        -> identifier::Nym final
    {
        return parent_.NymIDFromRandom(std::move(alloc));
    }
    auto NymIDFromRandom(const identifier::Algorithm type, allocator_type alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromRandom(type, std::move(alloc));
    }
    auto Offer() const -> std::unique_ptr<OTOffer> final;
    auto Offer(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const Amount& MARKET_SCALE) const -> std::unique_ptr<OTOffer> final;
    auto OutbailmentReply(
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const identifier::Generic& request,
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
    auto PasswordPrompt(std::string_view text) const
        -> opentxs::PasswordPrompt final;
    auto PasswordPrompt(const opentxs::PasswordPrompt& rhs) const
        -> opentxs::PasswordPrompt final;
    auto Payment() const -> std::unique_ptr<OTPayment> final;
    auto Payment(const String& strPayment) const
        -> std::unique_ptr<OTPayment> final;
    auto Payment(
        const opentxs::Contract& contract,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<OTPayment> final;
    auto PaymentCode(const proto::PaymentCode& serialized) const noexcept
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
    auto PaymentCodeFromBase58(const ReadView base58) const noexcept
        -> opentxs::PaymentCode final;
    auto PaymentCodeFromProtobuf(const ReadView proto) const noexcept
        -> opentxs::PaymentCode final;
    auto PaymentPlan() const -> std::unique_ptr<OTPaymentPlan> final;
    auto PaymentPlan(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID) const
        -> std::unique_ptr<OTPaymentPlan> final;
    auto PaymentPlan(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::Account& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const identifier::Account& RECIPIENT_ACCT_ID,
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
        const identifier::Generic& request,
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
    auto Secret(const std::size_t bytes) const noexcept -> opentxs::Secret final
    {
        return parent_.Secret(bytes);
    }
    auto SecretFromBytes(const ReadView bytes) const noexcept
        -> opentxs::Secret final
    {
        return parent_.SecretFromBytes(bytes);
    }
    auto SecretFromText(std::string_view text) const noexcept
        -> opentxs::Secret final
    {
        return parent_.SecretFromText(text);
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
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::crypto::symmetric::Algorithm mode,
        const opentxs::PasswordPrompt& password,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::PasswordPrompt& password,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const opentxs::crypto::symmetric::Source type,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& seed,
        const ReadView salt,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::uint64_t parallel,
        const std::size_t size,
        const opentxs::crypto::symmetric::Source type,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const opentxs::Secret& raw,
        const opentxs::PasswordPrompt& reason,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key final;
    auto SymmetricKey(
        const opentxs::crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized,
        alloc::Default alloc) const noexcept
        -> opentxs::crypto::symmetric::Key final;
    auto Trade() const -> std::unique_ptr<OTTrade> final;
    auto Trade(
        const identifier::Notary& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const identifier::Account& assetAcctId,
        const identifier::Nym& nymID,
        const identifier::UnitDefinition& currencyId,
        const identifier::Account& currencyAcctId) const
        -> std::unique_ptr<OTTrade> final;
    auto Transaction(const String& strCronItem) const
        -> std::unique_ptr<OTTransactionType> final;
    auto Transaction(const opentxs::Ledger& theOwner) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
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
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
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
    auto UnitID(const proto::Identifier& in, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDConvertSafe(const identifier::Generic& in, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromBase58(const std::string_view base58, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromBase58(base58, std::move(alloc));
    }
    auto UnitIDFromHash(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromHash(bytes, std::move(alloc));
    }
    auto UnitIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromHash(bytes, type, std::move(alloc));
    }
    auto UnitIDFromPreimage(const ReadView preimage, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromPreimage(preimage, std::move(alloc));
    }
    auto UnitIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromPreimage(preimage, type, std::move(alloc));
    }
    auto UnitIDFromPreimage(const ProtobufType& proto, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromPreimage(
        const ProtobufType& proto,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept
        -> identifier::UnitDefinition final;
    auto UnitIDFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromProtobuf(bytes, std::move(alloc));
    }
    auto UnitIDFromRandom(allocator_type alloc) const noexcept
        -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromRandom(std::move(alloc));
    }
    auto UnitIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromRandom(type, std::move(alloc));
    }
    auto UnitDefinition() const noexcept -> OTUnitDefinition final;
    auto UnitDefinition(
        const Nym_p& nym,
        const proto::UnitDefinition serialized) const noexcept(false)
        -> OTUnitDefinition final;

    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;

    ~Factory() override;

protected:
    const api::Session& api_;
    const api::Factory& parent_;
    std::unique_ptr<const api::crypto::Asymmetric> p_asymmetric_;
    const api::crypto::Asymmetric& asymmetric_;
    std::unique_ptr<const api::crypto::Symmetric> p_symmetric_;
    const api::crypto::Symmetric& symmetric_;

    Factory(const api::Session& api, const api::Factory& parent);
};
}  // namespace opentxs::api::session::imp
