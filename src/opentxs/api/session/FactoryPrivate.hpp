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
#include <optional>
#include <span>
#include <string_view>
#include <utility>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/BasketContract.hpp"
#include "internal/core/contract/CurrencyContract.hpp"
#include "internal/core/contract/SecurityContract.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/reply/Bailment.hpp"
#include "opentxs/core/contract/peer/reply/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/reply/Connection.hpp"
#include "opentxs/core/contract/peer/reply/Faucet.hpp"
#include "opentxs/core/contract/peer/reply/Outbailment.hpp"
#include "opentxs/core/contract/peer/reply/StoreSecret.hpp"
#include "opentxs/core/contract/peer/reply/Verification.hpp"
#include "opentxs/core/contract/peer/request/Bailment.hpp"
#include "opentxs/core/contract/peer/request/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/request/Connection.hpp"
#include "opentxs/core/contract/peer/request/Faucet.hpp"
#include "opentxs/core/contract/peer/request/Outbailment.hpp"
#include "opentxs/core/contract/peer/request/StoreSecret.hpp"
#include "opentxs/core/contract/peer/request/Verification.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/Types.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/Verification.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace asio
{
namespace ip
{
class address;
}  // namespace ip
}  // namespace asio
}  // namespace boost

namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

namespace session
{
class FactoryPrivate;  // IWYU pragma: keep
}  // namespace session
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

namespace protobuf
{
class AsymmetricKey;
class BlockchainBlockHeader;
class BlockchainPeerAddress;
class BlockchainTransaction;
class Claim;
class ContactItem;
class HDPath;
class Identifier;
class PaymentCode;
class PeerObject;
class PeerReply;
class PeerRequest;
class Purse;
class SymmetricKey;
class UnitDefinition;
class Verification;
class VerificationItem;
}  // namespace protobuf

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

class opentxs::api::session::FactoryPrivate : virtual public internal::Factory
{
public:
    auto AccountID(
        const identity::wot::claim::ClaimType type,
        const protobuf::HDPath& path,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountID(type, path, std::move(alloc));
    }
    auto AccountID(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountID(in, std::move(alloc));
    }
    auto AccountID(const opentxs::Contract& contract, alloc::Default alloc)
        const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountID(contract, std::move(alloc));
    }
    auto AccountIDConvertSafe(
        const identifier::Generic& in,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountIDConvertSafe(in, std::move(alloc));
    }
    auto AccountIDFromBase58(
        const std::string_view base58,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromBase58(base58, std::move(alloc));
    }
    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromHash(bytes, subtype, std::move(alloc));
    }
    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromHash(
            bytes, subtype, type, std::move(alloc));
    }
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromPreimage(
            preimage, subtype, std::move(alloc));
    }
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromPreimage(
            preimage, subtype, type, std::move(alloc));
    }
    auto AccountIDFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromProtobuf(bytes, std::move(alloc));
    }
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromRandom(subtype, std::move(alloc));
    }
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.AccountIDFromRandom(subtype, type, std::move(alloc));
    }
    auto AccountIDFromZMQ(
        const opentxs::network::zeromq::Frame& frame,
        alloc::Default alloc) const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountIDFromZMQ(frame, std::move(alloc));
    }
    auto AccountIDFromZMQ(const ReadView frame, alloc::Default alloc)
        const noexcept -> identifier::Account final
    {
        return parent_.Internal().AccountIDFromZMQ(frame, std::move(alloc));
    }
    auto Amount(const opentxs::network::zeromq::Frame& zmq) const noexcept
        -> opentxs::Amount final
    {
        return parent_.Amount(zmq);
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
    auto Armored(const protobuf::MessageType& input) const -> OTArmored final
    {
        return parent_.Internal().Armored(input);
    }
    auto Armored(
        const protobuf::MessageType& input,
        const UnallocatedCString& header) const -> OTString final
    {
        return parent_.Internal().Armored(input, header);
    }
    auto Asymmetric() const -> const api::crypto::Asymmetric& final
    {
        return asymmetric_;
    }
    auto AsymmetricKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        VersionNumber version,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        VersionNumber version,
        opentxs::crypto::asymmetric::Role role,
        const opentxs::crypto::Parameters& params,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(const protobuf::AsymmetricKey& serialized) const noexcept
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::Secret& key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        VersionNumber version,
        opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::Secret& key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        opentxs::crypto::asymmetric::Role role,
        opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::Secret& key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> opentxs::crypto::asymmetric::Key final;
    auto AsymmetricKey(
        VersionNumber version,
        opentxs::crypto::asymmetric::Role role,
        opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::Secret& key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> opentxs::crypto::asymmetric::Key final;
    auto BailmentNoticeReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        bool value,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::reply::BailmentNotice final;
    auto BailmentNoticeRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& notary,
        const identifier::Generic& inReferenceToRequest,
        std::string_view description,
        const opentxs::Amount& amount,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::request::BailmentNotice final;
    auto BailmentReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        std::string_view terms,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::reply::Bailment final;
    auto BailmentRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identifier::UnitDefinition& unit,
        const identifier::Notary& notary,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::request::Bailment final;
    auto Basket() const -> std::unique_ptr<opentxs::Basket> final;
    auto Basket(
        std::int32_t nCount,
        const opentxs::Amount& lMinimumTransferAmount) const
        -> std::unique_ptr<opentxs::Basket> final;
    auto BasketContract(
        const Nym_p& nym,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const std::uint64_t weight,
        const UnitType unitOfAccount,
        const VersionNumber version,
        const display::Definition& displayDefinition,
        const opentxs::Amount& redemptionIncrement) const noexcept(false)
        -> OTBasketContract final;
    auto BasketContract(
        const Nym_p& nym,
        const protobuf::UnitDefinition serialized) const noexcept(false)
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
        -> blockchain::protocol::bitcoin::base::block::Script final;
    auto BitcoinScriptP2MS(
        const blockchain::Type chain,
        const std::uint8_t M,
        const std::uint8_t N,
        std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script final;
    auto BitcoinScriptP2PK(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script final;
    auto BitcoinScriptP2PKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script final;
    auto BitcoinScriptP2SH(
        const blockchain::Type chain,
        const blockchain::protocol::bitcoin::base::block::Script& script,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script final;
    auto BitcoinScriptP2WPKH(
        const blockchain::Type chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script final;
    auto BitcoinScriptP2WSH(
        const blockchain::Type chain,
        const blockchain::protocol::bitcoin::base::block::Script& script,
        alloc::Default alloc) const noexcept
        -> blockchain::protocol::bitcoin::base::block::Script final;
    auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const noexcept -> opentxs::network::blockchain::Address final
    {
        return parent_.BlockchainAddress(
            protocol, network, bytes, port, chain, lastConnected, services);
    }
    auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const boost::asio::ip::address& address,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const noexcept -> opentxs::network::blockchain::Address final
    {
        return parent_.Internal().BlockchainAddress(
            protocol, address, port, chain, lastConnected, services);
    }
    auto BlockchainAddress(const protobuf::BlockchainPeerAddress& serialized)
        const noexcept -> opentxs::network::blockchain::Address final
    {
        return parent_.Internal().BlockchainAddress(serialized);
    }
    auto BlockchainAddressIncoming(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const opentxs::network::blockchain::Transport subtype,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView cookie) const noexcept
        -> opentxs::network::blockchain::Address final
    {
        return parent_.Internal().BlockchainAddressIncoming(
            protocol,
            network,
            subtype,
            bytes,
            port,
            chain,
            lastConnected,
            services,
            cookie);
    }
    auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport subtype,
        const ReadView bytes,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const noexcept
        -> opentxs::network::blockchain::Address final
    {
        return parent_.BlockchainAddressZMQ(
            protocol, subtype, bytes, chain, lastConnected, services, key);
    }
    auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const boost::asio::ip::address& address,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const noexcept
        -> opentxs::network::blockchain::Address final
    {
        return parent_.BlockchainAddressZMQ(
            protocol, address, chain, lastConnected, services, key);
    }
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
        const protobuf::BlockchainTransaction& serialized,
        alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction final;
    auto BlockHeader(
        const protobuf::BlockchainBlockHeader& proto,
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
    auto Claim(
        const identity::wot::Claimant& claimant,
        identity::wot::claim::SectionType section,
        identity::wot::claim::ClaimType type,
        ReadView value,
        ReadView subtype,
        std::span<const identity::wot::claim::Attribute> attributes,
        Time start,
        Time stop,
        VersionNumber version,
        alloc::Strategy alloc) const noexcept -> identity::wot::Claim final;
    auto Claim(
        const identity::Nym& claimant,
        identity::wot::claim::SectionType section,
        identity::wot::claim::ClaimType type,
        ReadView value,
        ReadView subtype,
        std::span<const identity::wot::claim::Attribute> attributes,
        Time start,
        Time stop,
        VersionNumber version,
        alloc::Strategy alloc) const noexcept -> identity::wot::Claim final;
    auto Claim(ReadView serialized, alloc::Strategy alloc) const noexcept
        -> identity::wot::Claim final;
    auto Claim(
        const identity::wot::Claimant& claimant,
        const identity::wot::claim::SectionType section,
        const protobuf::ContactItem& proto,
        alloc::Strategy alloc) const noexcept -> identity::wot::Claim final;
    auto Claim(const protobuf::Claim& proto, alloc::Strategy alloc)
        const noexcept -> identity::wot::Claim final;
    auto ConnectionReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        bool accepted,
        std::string_view url,
        std::string_view login,
        std::string_view password,
        std::string_view key,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::reply::Connection final;
    auto ConnectionRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const contract::peer::ConnectionInfoType kind,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::request::Connection final;
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
        const opentxs::Amount& redemptionIncrement) const noexcept(false)
        -> OTCurrencyContract final;
    auto CurrencyContract(
        const Nym_p& nym,
        const protobuf::UnitDefinition serialized) const noexcept(false)
        -> OTCurrencyContract final;
    auto Data() const -> ByteArray final { return parent_.Data(); }
    auto Data(const opentxs::Armored& input) const -> ByteArray final
    {
        return parent_.Data(input);
    }
    auto Data(const protobuf::MessageType& input) const -> ByteArray final
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
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        const blockchain::block::Transaction& transaction,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::reply::Faucet final;
    auto FaucetRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        opentxs::UnitType unit,
        std::string_view address,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::request::Faucet final;
    auto Identifier(const opentxs::Contract& contract, alloc::Default alloc)
        const noexcept -> identifier::Generic final;
    auto Identifier(const opentxs::Cheque& cheque, alloc::Default alloc)
        const noexcept -> identifier::Generic final;
    auto Identifier(const opentxs::Item& item, alloc::Default alloc)
        const noexcept -> identifier::Generic final;
    auto Identifier(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromBase58(
        const std::string_view base58,
        alloc::Default alloc) const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromBase58(base58, std::move(alloc));
    }
    auto IdentifierFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromHash(bytes, std::move(alloc));
    }
    auto IdentifierFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromHash(bytes, type, std::move(alloc));
    }
    auto IdentifierFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromPreimage(preimage, std::move(alloc));
    }
    auto IdentifierFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromPreimage(preimage, type, std::move(alloc));
    }
    auto IdentifierFromPreimage(
        const protobuf::MessageType& proto,
        alloc::Default alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(
        const protobuf::MessageType& proto,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Generic final
    {
        return parent_.IdentifierFromProtobuf(bytes, std::move(alloc));
    }
    auto IdentifierFromRandom(alloc::Default alloc) const noexcept
        -> identifier::Generic final
    {
        return parent_.IdentifierFromRandom(std::move(alloc));
    }
    auto IdentifierFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Generic final
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
        otx::itemType theType,
        const identifier::Account& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(
        const String& strItem,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNumber) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Item(
        const OTTransaction& theOwner,
        otx::itemType theType,
        const identifier::Account& pDestinationAcctID) const
        -> std::unique_ptr<opentxs::Item> final;
    auto Keypair(
        const opentxs::crypto::Parameters& nymParameters,
        const VersionNumber version,
        const opentxs::crypto::asymmetric::Role role,
        const opentxs::PasswordPrompt& reason) const -> OTKeypair final;
    auto Keypair(
        const protobuf::AsymmetricKey& serializedPubkey,
        const protobuf::AsymmetricKey& serializedPrivkey) const
        -> OTKeypair final;
    auto Keypair(const protobuf::AsymmetricKey& serializedPubkey) const
        -> OTKeypair final;
    auto Keypair(
        const opentxs::crypto::SeedID& fingerprint,
        const opentxs::crypto::Bip32Index nym,
        const opentxs::crypto::Bip32Index credset,
        const opentxs::crypto::Bip32Index credindex,
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
        otx::ledgerType theType,
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
        otx::ledgerType theType,
        bool bCreateFile) const -> std::unique_ptr<opentxs::Ledger> final;
    auto Market() const -> std::unique_ptr<OTMarket> final;
    auto Market(const char* szFilename) const
        -> std::unique_ptr<OTMarket> final;
    auto Market(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_TYPE_ID,
        const opentxs::Amount& lScale) const -> std::unique_ptr<OTMarket> final;
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
    auto NotaryID(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDConvertSafe(
        const identifier::Generic& in,
        alloc::Default alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromBase58(const std::string_view base58, alloc::Default alloc)
        const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromBase58(base58, std::move(alloc));
    }
    auto NotaryIDFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromHash(bytes, std::move(alloc));
    }
    auto NotaryIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromHash(bytes, type, std::move(alloc));
    }
    auto NotaryIDFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromPreimage(preimage, std::move(alloc));
    }
    auto NotaryIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromPreimage(preimage, type, std::move(alloc));
    }
    auto NotaryIDFromPreimage(
        const protobuf::MessageType& proto,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(
        const protobuf::MessageType& proto,
        alloc::Default alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromProtobuf(bytes, std::move(alloc));
    }
    auto NotaryIDFromRandom(alloc::Default alloc) const noexcept
        -> identifier::Notary final
    {
        return parent_.NotaryIDFromRandom(std::move(alloc));
    }
    auto NotaryIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Notary final
    {
        return parent_.NotaryIDFromRandom(type, std::move(alloc));
    }
    auto NymID(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.Internal().NymID(in, std::move(alloc));
    }
    auto NymIDConvertSafe(const identifier::Generic& in, alloc::Default alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.Internal().NymIDConvertSafe(in, std::move(alloc));
    }
    auto NymIDFromBase58(const std::string_view base58, alloc::Default alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromBase58(base58, std::move(alloc));
    }
    auto NymIDFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromHash(bytes, std::move(alloc));
    }
    auto NymIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromHash(bytes, type, std::move(alloc));
    }
    auto NymIDFromPaymentCode(const UnallocatedCString& serialized) const
        -> identifier::Nym final;
    auto NymIDFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromPreimage(preimage, std::move(alloc));
    }
    auto NymIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromPreimage(preimage, type, std::move(alloc));
    }
    auto NymIDFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromProtobuf(bytes, std::move(alloc));
    }
    auto NymIDFromRandom(alloc::Default alloc) const noexcept
        -> identifier::Nym final
    {
        return parent_.NymIDFromRandom(std::move(alloc));
    }
    auto NymIDFromRandom(const identifier::Algorithm type, alloc::Default alloc)
        const noexcept -> identifier::Nym final
    {
        return parent_.NymIDFromRandom(type, std::move(alloc));
    }
    auto Offer() const -> std::unique_ptr<OTOffer> final;
    auto Offer(
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const identifier::UnitDefinition& CURRENCY_ID,
        const opentxs::Amount& MARKET_SCALE) const
        -> std::unique_ptr<OTOffer> final;
    auto OutbailmentReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        std::string_view description,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::reply::Outbailment final;
    auto OutbailmentRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identifier::UnitDefinition& unitID,
        const identifier::Notary& notary,
        const opentxs::Amount& amount,
        std::string_view instructions,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::request::Outbailment final;
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
    auto PaymentCode(const protobuf::PaymentCode& serialized) const noexcept
        -> opentxs::PaymentCode final;
    auto PaymentCode(
        const opentxs::crypto::SeedID& seed,
        const opentxs::crypto::Bip32Index nym,
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
        const contract::peer::Request& request,
        const contract::peer::Reply& reply,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerObject(
        const contract::peer::Request& request,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerObject(
        const Nym_p& signerNym,
        const protobuf::PeerObject& serialized) const
        -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> override;
    auto PeerReply(ReadView bytes, alloc::Strategy alloc) const noexcept
        -> contract::peer::Reply final;
    auto PeerReply(
        const opentxs::network::zeromq::Frame& bytes,
        alloc::Strategy alloc) const noexcept -> contract::peer::Reply final;
    auto PeerReply(const protobuf::PeerReply& proto, alloc::Strategy alloc)
        const noexcept -> contract::peer::Reply final;
    auto PeerRequest(ReadView bytes, alloc::Strategy alloc) const noexcept
        -> contract::peer::Request final;
    auto PeerRequest(
        const opentxs::network::zeromq::Frame& bytes,
        alloc::Strategy alloc) const noexcept -> contract::peer::Request final;
    auto PeerRequest(const protobuf::PeerRequest& proto, alloc::Strategy alloc)
        const noexcept -> contract::peer::Request final;
    auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const opentxs::Amount& totalValue,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse final;
    auto Purse(
        const otx::context::Server& context,
        const identifier::UnitDefinition& unit,
        const otx::blind::Mint& mint,
        const opentxs::Amount& totalValue,
        const otx::blind::CashType type,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> otx::blind::Purse final;
    auto Purse(const protobuf::Purse& serialized) const noexcept
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
        const opentxs::Amount& redemptionIncrement) const noexcept(false)
        -> OTSecurityContract final;
    auto SecurityContract(
        const Nym_p& nym,
        const protobuf::UnitDefinition serialized) const noexcept(false)
        -> OTSecurityContract final;
    auto SeedID(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::HDSeed final
    {
        return parent_.Internal().SeedID(in, std::move(alloc));
    }
    auto SeedIDFromBase58(const std::string_view base58, alloc::Default alloc)
        const noexcept -> identifier::HDSeed final
    {
        return parent_.SeedIDFromBase58(base58, std::move(alloc));
    }
    auto SeedIDFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::HDSeed final
    {
        return parent_.SeedIDFromHash(bytes, std::move(alloc));
    }
    auto SeedIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::HDSeed final
    {
        return parent_.SeedIDFromHash(bytes, type, std::move(alloc));
    }
    auto SeedIDFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::HDSeed final
    {
        return parent_.SeedIDFromPreimage(preimage, std::move(alloc));
    }
    auto SeedIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::HDSeed final
    {
        return parent_.SeedIDFromPreimage(preimage, type, std::move(alloc));
    }
    auto SeedIDFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::HDSeed final
    {
        return parent_.SeedIDFromProtobuf(bytes, std::move(alloc));
    }
    auto SeedIDFromRandom(alloc::Default alloc) const noexcept
        -> identifier::HDSeed final
    {
        return parent_.SeedIDFromRandom(std::move(alloc));
    }
    auto SeedIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::HDSeed final
    {
        return parent_.SeedIDFromRandom(type, std::move(alloc));
    }
    auto Self() const noexcept -> const api::Factory& final { return self_; }
    auto ServerContract() const noexcept(false) -> OTServerContract final;
    auto SessionPublic() const noexcept -> const api::session::Factory& final
    {
        return self_;
    }
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
    auto StoreSecretReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        bool value,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::reply::StoreSecret final;
    auto StoreSecretRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const contract::peer::SecretType kind,
        std::span<const std::string_view> data,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::request::StoreSecret final;
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
        const protobuf::SymmetricKey serialized,
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
        otx::originType theOriginType = otx::originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        std::int64_t lTransactionNum,
        otx::originType theOriginType = otx::originType::not_applicable) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        const std::int64_t& lNumberOfOrigin,
        otx::originType theOriginType,
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
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const identifier::Nym& theNymID,
        const identifier::Account& theAccountID,
        const identifier::Notary& theNotaryID,
        otx::transactionType theType,
        otx::originType theOriginType = otx::originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> final;
    auto Transaction(
        const opentxs::Ledger& theOwner,
        otx::transactionType theType,
        otx::originType theOriginType = otx::originType::not_applicable,
        std::int64_t lTransactionNum = 0) const
        -> std::unique_ptr<OTTransaction> final;
    auto UnitID(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDConvertSafe(const identifier::Generic& in, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromBase58(const std::string_view base58, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromBase58(base58, std::move(alloc));
    }
    auto UnitIDFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromHash(bytes, std::move(alloc));
    }
    auto UnitIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromHash(bytes, type, std::move(alloc));
    }
    auto UnitIDFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromPreimage(preimage, std::move(alloc));
    }
    auto UnitIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromPreimage(preimage, type, std::move(alloc));
    }
    auto UnitIDFromPreimage(
        const protobuf::MessageType& proto,
        alloc::Default alloc) const noexcept
        -> identifier::UnitDefinition final;
    auto UnitIDFromPreimage(
        const protobuf::MessageType& proto,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept
        -> identifier::UnitDefinition final;
    auto UnitIDFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromProtobuf(bytes, std::move(alloc));
    }
    auto UnitIDFromRandom(alloc::Default alloc) const noexcept
        -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromRandom(std::move(alloc));
    }
    auto UnitIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::UnitDefinition final
    {
        return parent_.UnitIDFromRandom(type, std::move(alloc));
    }
    auto UnitDefinition() const noexcept -> OTUnitDefinition final;
    auto UnitDefinition(
        const Nym_p& nym,
        const protobuf::UnitDefinition serialized) const noexcept(false)
        -> OTUnitDefinition final;
    auto Verification(
        const identifier::Nym& verifier,
        const opentxs::PasswordPrompt& reason,
        identity::wot::ClaimID claim,
        identity::wot::verification::Type value,
        Time start,
        Time stop,
        std::span<const identity::wot::VerificationID> superscedes,
        alloc::Strategy alloc) const noexcept
        -> identity::wot::Verification final;
    auto Verification(
        const identity::Nym& verifier,
        const opentxs::PasswordPrompt& reason,
        identity::wot::ClaimID claim,
        identity::wot::verification::Type value,
        Time start,
        Time stop,
        std::span<const identity::wot::VerificationID> superscedes,
        alloc::Strategy alloc) const noexcept
        -> identity::wot::Verification final;
    auto Verification(ReadView serialized, alloc::Strategy alloc) const noexcept
        -> identity::wot::Verification final;
    auto Verification(
        const identifier::Nym& verifier,
        const protobuf::VerificationItem& proto,
        alloc::Strategy alloc) const noexcept
        -> identity::wot::Verification final;
    auto Verification(
        const protobuf::Verification& proto,
        alloc::Strategy alloc) const noexcept
        -> identity::wot::Verification final;
    auto VerificationReply(
        const Nym_p& responder,
        const identifier::Nym& initiator,
        const identifier::Generic& inReferenceToRequest,
        const std::optional<identity::wot::Verification>& response,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::reply::Verification final;
    auto VerificationRequest(
        const Nym_p& initiator,
        const identifier::Nym& responder,
        const identity::wot::Claim& claim,
        const opentxs::PasswordPrompt& reason,
        alloc::Strategy alloc) const noexcept
        -> contract::peer::request::Verification final;

    auto Self() noexcept -> api::Factory& final { return self_; }
    auto SessionPublic() noexcept -> api::session::Factory& final
    {
        return self_;
    }

    FactoryPrivate() = delete;
    FactoryPrivate(const FactoryPrivate&) = delete;
    FactoryPrivate(FactoryPrivate&&) = delete;
    auto operator=(const FactoryPrivate&) -> FactoryPrivate& = delete;
    auto operator=(FactoryPrivate&&) -> FactoryPrivate& = delete;

    ~FactoryPrivate() override;

protected:
    const api::internal::Session& api_;
    const api::Factory& parent_;
    std::unique_ptr<const api::crypto::Asymmetric> p_asymmetric_;
    const api::crypto::Asymmetric& asymmetric_;
    std::unique_ptr<const api::crypto::Symmetric> p_symmetric_;
    const api::crypto::Symmetric& symmetric_;

    FactoryPrivate(
        const api::internal::Session& api,
        const api::Factory& parent);

private:
    api::session::Factory self_;
};
