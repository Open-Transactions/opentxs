// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "internal/api/FactoryAPI.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace crypto
{
class Envelope;
}  // namespace crypto

namespace proto
{
class BlockchainPeerAddress;
class HDPath;
class Identifier;
}  // namespace proto

class Cheque;
class Contract;
class Data;
class Item;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::imp
{
using namespace std::literals;

class Factory final : public internal::Factory
{
public:
    auto AccountID(
        const identity::wot::claim::ClaimType type,
        const proto::HDPath& path,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountID(const proto::Identifier& in, allocator_type alloc)
        const noexcept -> identifier::Account final;
    auto AccountID(const Contract& contract, allocator_type alloc)
        const noexcept -> identifier::Account final;
    auto AccountIDConvertSafe(
        const identifier::Generic& in,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromBase58(
        const std::string_view base58,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Account final;
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromZMQ(
        const opentxs::network::zeromq::Frame& frame,
        allocator_type alloc) const noexcept -> identifier::Account final;
    auto AccountIDFromZMQ(const ReadView frame, allocator_type alloc)
        const noexcept -> identifier::Account final;
    auto Amount(const opentxs::network::zeromq::Frame& zmq) const noexcept
        -> opentxs::Amount final;
    auto Armored() const -> OTArmored final;
    auto Armored(const UnallocatedCString& input) const -> OTArmored final;
    auto Armored(const opentxs::Data& input) const -> OTArmored final;
    auto Armored(const opentxs::String& input) const -> OTArmored final;
    auto Armored(const opentxs::crypto::Envelope& input) const
        -> OTArmored final;
    auto Armored(const ProtobufType& input) const -> OTArmored final;
    auto Armored(const ProtobufType& input, const UnallocatedCString& header)
        const -> OTString final;
    auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
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
    auto BlockchainAddress(const proto::BlockchainPeerAddress& serialized)
        const noexcept -> opentxs::network::blockchain::Address final;
    auto Data() const -> ByteArray final;
    auto Data(const opentxs::Armored& input) const -> ByteArray final;
    auto Data(const ProtobufType& input) const -> ByteArray final;
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
    auto Identifier(const Cheque& cheque, allocator_type alloc) const noexcept
        -> identifier::Generic final;
    auto Identifier(const Contract& contract, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto Identifier(const Item& item, allocator_type alloc) const noexcept
        -> identifier::Generic final;
    auto Identifier(const proto::Identifier& in, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromBase58(
        const std::string_view base58,
        allocator_type alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromHash(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(const ReadView preimage, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(
        const ProtobufType& proto,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Generic final;
    auto IdentifierFromPreimage(const ProtobufType& proto, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Generic final;
    auto IdentifierFromRandom(allocator_type alloc) const noexcept
        -> identifier::Generic final;
    auto IdentifierFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Generic final;
    auto NotaryID(const proto::Identifier& in, allocator_type alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDConvertSafe(
        const identifier::Generic& in,
        allocator_type alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromBase58(const std::string_view base58, allocator_type alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromHash(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(const ReadView preimage, allocator_type alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(const ProtobufType& proto, allocator_type alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromPreimage(
        const ProtobufType& proto,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Notary final;
    auto NotaryIDFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Notary final;
    auto NotaryIDFromRandom(allocator_type alloc) const noexcept
        -> identifier::Notary final;
    auto NotaryIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Notary final;
    auto NymID(const proto::Identifier& in, allocator_type alloc) const noexcept
        -> identifier::Nym final;
    auto NymIDConvertSafe(const identifier::Generic& in, allocator_type alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromBase58(const std::string_view base58, allocator_type alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromHash(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Nym final;
    auto NymIDFromPreimage(const ReadView preimage, allocator_type alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::Nym final;
    auto NymIDFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::Nym final;
    auto NymIDFromRandom(allocator_type alloc) const noexcept
        -> identifier::Nym final;
    auto NymIDFromRandom(const identifier::Algorithm type, allocator_type alloc)
        const noexcept -> identifier::Nym final;
    auto Secret(const std::size_t bytes) const noexcept
        -> opentxs::Secret final;
    auto SecretFromBytes(const ReadView bytes) const noexcept
        -> opentxs::Secret final;
    auto SecretFromText(const std::string_view text) const noexcept
        -> opentxs::Secret final;
    auto SeedIDFromHash(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::HDSeed final;
    auto SeedIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::HDSeed final;
    auto SeedIDFromBase58(const std::string_view base58, allocator_type alloc)
        const noexcept -> identifier::HDSeed final;
    auto SeedIDFromPreimage(const ReadView preimage, allocator_type alloc)
        const noexcept -> identifier::HDSeed final;
    auto SeedIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::HDSeed final;
    auto SeedIDFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::HDSeed final;
    auto SeedIDFromRandom(allocator_type alloc) const noexcept
        -> identifier::HDSeed final;
    auto SeedIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> identifier::HDSeed final;
    auto SeedID(const proto::Identifier& in, allocator_type alloc = {})
        const noexcept -> identifier::HDSeed final;
    auto UnitID(const proto::Identifier& in, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDConvertSafe(const identifier::Generic& in, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromBase58(const std::string_view base58, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromHash(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept
        -> identifier::UnitDefinition final;
    auto UnitIDFromPreimage(const ReadView preimage, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept
        -> identifier::UnitDefinition final;
    auto UnitIDFromPreimage(const ProtobufType& proto, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromPreimage(
        const ProtobufType& proto,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept
        -> identifier::UnitDefinition final;
    auto UnitIDFromProtobuf(const ReadView bytes, allocator_type alloc)
        const noexcept -> identifier::UnitDefinition final;
    auto UnitIDFromRandom(allocator_type alloc) const noexcept
        -> identifier::UnitDefinition final;
    auto UnitIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept
        -> identifier::UnitDefinition final;

    Factory(const api::Crypto& crypto) noexcept;
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;

    ~Factory() final = default;

private:
    const api::Crypto& crypto_;

    template <typename IDType>
    static auto id_type() noexcept -> identifier::Type;

    template <typename IDType>
    auto id_from_base58(const std::string_view base58, allocator_type alloc)
        const noexcept -> IDType;
    template <typename IDType>
    auto id_from_hash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_hash(
        const ReadView bytes,
        const identifier::Algorithm type,
        identifier::AccountSubtype accountSubtype,
        allocator_type alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_preimage(
        const identifier::Algorithm type,
        const ReadView bytes,
        allocator_type alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_preimage(
        const identifier::Algorithm type,
        identifier::AccountSubtype accountSubtype,
        const ReadView bytes,
        allocator_type alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_preimage(
        const identifier::Algorithm type,
        const ProtobufType& proto,
        allocator_type alloc) const noexcept -> IDType;
    template <typename IDType>
    auto id_from_protobuf(const proto::Identifier& proto, allocator_type alloc)
        const noexcept -> IDType;
    template <typename IDType>
    auto id_from_random(const identifier::Algorithm type, allocator_type alloc)
        const noexcept -> IDType;
    template <typename IDType>
    auto id_from_random(
        const identifier::Algorithm type,
        identifier::AccountSubtype accountSubtype,
        allocator_type alloc) const noexcept -> IDType;
};
}  // namespace opentxs::api::imp
