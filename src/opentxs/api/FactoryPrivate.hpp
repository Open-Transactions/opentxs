// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/Types.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

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
namespace session
{
namespace internal
{
class Factory;
}  // namespace internal
}  // namespace session

class Crypto;
class FactoryPrivate;  // IWYU pragma: keep
}  // namespace api

namespace crypto
{
class Envelope;
}  // namespace crypto

namespace protobuf
{
class BlockchainPeerAddress;
class HDPath;
class Identifier;
}  // namespace protobuf

class Cheque;
class Contract;
class Data;
class Item;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::FactoryPrivate final : public internal::Factory
{
public:
    auto AccountID(
        const identity::wot::claim::ClaimType type,
        const protobuf::HDPath& path,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountID(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::Account final;
    auto AccountID(const Contract& contract, alloc::Default alloc)
        const noexcept -> identifier::Account final;
    auto AccountIDConvertSafe(
        const identifier::Generic& in,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromBase58(
        const std::string_view base58,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Account final;
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromZMQ(
        const opentxs::network::zeromq::Frame& frame,
        alloc::Default alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromZMQ(const ReadView frame, alloc::Default alloc)
        const noexcept -> identifier::Account final;
    auto Amount(const opentxs::network::zeromq::Frame& zmq) const noexcept
        -> opentxs::Amount final;
    auto Armored() const -> OTArmored final;
    auto Armored(const UnallocatedCString& input) const -> OTArmored final;
    auto Armored(const opentxs::Data& input) const -> OTArmored final;
    auto Armored(const opentxs::String& input) const -> OTArmored final;
    auto Armored(const opentxs::crypto::Envelope& input) const
        -> OTArmored final;
    auto Armored(const protobuf::MessageType& input) const -> OTArmored final;
    auto Armored(
        const protobuf::MessageType& input,
        const UnallocatedCString& header) const -> OTString final;
    auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const noexcept -> opentxs::network::blockchain::Address final;
    auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const boost::asio::ip::address& address,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const noexcept -> opentxs::network::blockchain::Address final;
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
        -> opentxs::network::blockchain::Address final;
    auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const noexcept
        -> opentxs::network::blockchain::Address final;
    auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const boost::asio::ip::address& address,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const noexcept
        -> opentxs::network::blockchain::Address final;
    auto BlockchainAddress(const protobuf::BlockchainPeerAddress& serialized)
        const noexcept -> opentxs::network::blockchain::Address final;
    auto Data() const -> ByteArray final;
    auto Data(const opentxs::Armored& input) const -> ByteArray final;
    auto Data(const protobuf::MessageType& input) const -> ByteArray final;
    auto Data(const opentxs::network::zeromq::Frame& input) const
        -> ByteArray final;
    auto Data(const std::uint8_t input) const -> ByteArray final;
    auto Data(const std::uint32_t input) const -> ByteArray final;
    auto Data(const UnallocatedVector<unsigned char>& input) const
        -> ByteArray final;
    auto Data(const UnallocatedVector<std::byte>& input) const
        -> ByteArray final;
    auto DataFromBytes(ReadView input) const -> ByteArray final;
    auto DataFromHex(ReadView input) const -> ByteArray final;
    auto Identifier(const Cheque& cheque, alloc::Default alloc) const noexcept
        -> identifier::Generic final;
    auto Identifier(const Contract& contract, alloc::Default alloc)
        const noexcept -> identifier::Generic final;
    auto Identifier(const Item& item, alloc::Default alloc) const noexcept
        -> identifier::Generic final;
    auto Identifier(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromBase58(
        const std::string_view base58,
        alloc::Default alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(
        const protobuf::MessageType& proto,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(
        const protobuf::MessageType& proto,
        alloc::Default alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromRandom(alloc::Default alloc) const noexcept
        -> identifier::Generic final;
    auto IdentifierFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Generic final;
    auto NotaryID(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDConvertSafe(
        const identifier::Generic& in,
        alloc::Default alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromBase58(const std::string_view base58, alloc::Default alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(
        const protobuf::MessageType& proto,
        alloc::Default alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(
        const protobuf::MessageType& proto,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromRandom(alloc::Default alloc) const noexcept
        -> identifier::Notary final;
    auto NotaryIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Notary final;
    auto NymID(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDConvertSafe(const identifier::Generic& in, alloc::Default alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromBase58(const std::string_view base58, alloc::Default alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Nym final;
    auto NymIDFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::Nym final;
    auto NymIDFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromRandom(alloc::Default alloc) const noexcept
        -> identifier::Nym final;
    auto NymIDFromRandom(const identifier::Algorithm type, alloc::Default alloc)
        const noexcept -> identifier::Nym final;
    auto Secret(const std::size_t bytes) const noexcept
        -> opentxs::Secret final;
    auto SecretFromBytes(const ReadView bytes) const noexcept
        -> opentxs::Secret final;
    auto SecretFromText(const std::string_view text) const noexcept
        -> opentxs::Secret final;
    auto SeedIDFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::HDSeed final;
    auto SeedIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::HDSeed final;
    auto SeedIDFromBase58(const std::string_view base58, alloc::Default alloc)
        const noexcept -> identifier::HDSeed final;
    auto SeedIDFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::HDSeed final;
    auto SeedIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::HDSeed final;
    auto SeedIDFromProtobuf(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::HDSeed final;
    auto SeedIDFromRandom(alloc::Default alloc) const noexcept
        -> identifier::HDSeed final;
    auto SeedIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> identifier::HDSeed final;
    auto SeedID(const protobuf::Identifier& in, alloc::Default alloc = {})
        const noexcept -> identifier::HDSeed final;
    auto Self() const noexcept -> const api::Factory& final { return self_; }
    auto Session() const noexcept
        -> const api::session::internal::Factory& final;
    auto UnitID(const protobuf::Identifier& in, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDConvertSafe(const identifier::Generic& in, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromBase58(const std::string_view base58, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromHash(const ReadView bytes, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept
        -> identifier::UnitDefinition final;
    auto UnitIDFromPreimage(const ReadView preimage, alloc::Default alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept
        -> identifier::UnitDefinition final;
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
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromRandom(alloc::Default alloc) const noexcept
        -> identifier::UnitDefinition final;
    auto UnitIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept
        -> identifier::UnitDefinition final;

    auto Session() noexcept -> api::session::internal::Factory& final;
    auto Self() noexcept -> api::Factory& final { return self_; }

    FactoryPrivate(const api::Crypto& crypto) noexcept;
    FactoryPrivate() = delete;
    FactoryPrivate(const FactoryPrivate&) = delete;
    FactoryPrivate(FactoryPrivate&&) = delete;
    auto operator=(const FactoryPrivate&) -> FactoryPrivate& = delete;
    auto operator=(FactoryPrivate&&) -> FactoryPrivate& = delete;

    ~FactoryPrivate() final;

private:
    const api::Crypto& crypto_;
    api::Factory self_;

    template <typename IDType>
    static auto id_type() noexcept -> identifier::Type;

    template <typename IDType>
    auto id_from_base58(const std::string_view base58, alloc::Default alloc)
        const noexcept -> IDType;
    template <typename IDType>
    auto id_from_hash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_hash(
        const ReadView bytes,
        const identifier::Algorithm type,
        identifier::AccountSubtype accountSubtype,
        alloc::Default alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_preimage(
        const identifier::Algorithm type,
        const ReadView bytes,
        alloc::Default alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_preimage(
        const identifier::Algorithm type,
        identifier::AccountSubtype accountSubtype,
        const ReadView bytes,
        alloc::Default alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_preimage(
        const identifier::Algorithm type,
        const protobuf::MessageType& proto,
        alloc::Default alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_protobuf(
        const protobuf::Identifier& proto,
        alloc::Default alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_random(const identifier::Algorithm type, alloc::Default alloc)
        const noexcept -> IDType;
    template <typename IDType>
    auto id_from_random(
        const identifier::Algorithm type,
        identifier::AccountSubtype accountSubtype,
        alloc::Default alloc) const noexcept -> IDType;
};
