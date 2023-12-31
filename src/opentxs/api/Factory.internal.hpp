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
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/identifier/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
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
namespace internal
{
class Factory;  // IWYU pragma: keep
}  // namespace internal

namespace session
{
namespace internal
{
class Factory;
}  // namespace internal
}  // namespace session
}  // namespace api

namespace crypto
{
class Envelope;
}  // namespace crypto

namespace identifier
{
class Generic;
class HDSeed;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

namespace protobuf
{
class BlockchainPeerAddress;
class HDPath;
class Identifier;
}  // namespace protobuf

class ByteArray;
class Cheque;
class Contract;
class Data;
class Item;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::internal::Factory
{
public:
    static auto Detach(api::Factory& self) noexcept -> void;

    virtual auto AccountID(const Contract& contract, alloc::Default alloc = {})
        const noexcept -> identifier::Account = 0;
    virtual auto AccountID(
        const identity::wot::claim::ClaimType type,
        const protobuf::HDPath& path,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountID(
        const protobuf::Identifier& in,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDConvertSafe(
        const identifier::Generic& in,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromBase58(
        const std::string_view base58,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromProtobuf(
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromZMQ(
        const ReadView frame,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDFromZMQ(
        const opentxs::network::zeromq::Frame& frame,
        alloc::Default alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto Amount(const opentxs::network::zeromq::Frame& zmq)
        const noexcept -> opentxs::Amount = 0;
    virtual auto Armored() const -> OTArmored = 0;
    virtual auto Armored(const UnallocatedCString& input) const
        -> OTArmored = 0;
    virtual auto Armored(const google::protobuf::MessageLite& input) const
        -> OTArmored = 0;
    virtual auto Armored(
        const google::protobuf::MessageLite& input,
        const UnallocatedCString& header) const -> OTString = 0;
    virtual auto Armored(const opentxs::Data& input) const -> OTArmored = 0;
    virtual auto Armored(const opentxs::String& input) const -> OTArmored = 0;
    virtual auto Armored(const opentxs::crypto::Envelope& input) const
        -> OTArmored = 0;
    virtual auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const boost::asio::ip::address& address,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const noexcept -> opentxs::network::blockchain::Address = 0;
    virtual auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const noexcept -> opentxs::network::blockchain::Address = 0;
    virtual auto BlockchainAddress(
        const protobuf::BlockchainPeerAddress& serialized) const noexcept
        -> opentxs::network::blockchain::Address = 0;
    virtual auto BlockchainAddressIncoming(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const opentxs::network::blockchain::Transport subtype,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView cookie) const noexcept
        -> opentxs::network::blockchain::Address = 0;
    virtual auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const boost::asio::ip::address& address,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const noexcept
        -> opentxs::network::blockchain::Address = 0;
    virtual auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport subtype,
        const ReadView bytes,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const noexcept
        -> opentxs::network::blockchain::Address = 0;
    virtual auto Data() const -> ByteArray = 0;
    virtual auto Data(const UnallocatedVector<std::byte>& input) const
        -> ByteArray = 0;
    virtual auto Data(const UnallocatedVector<unsigned char>& input) const
        -> ByteArray = 0;
    virtual auto Data(const google::protobuf::MessageLite& input) const
        -> ByteArray = 0;
    virtual auto Data(const opentxs::Armored& input) const -> ByteArray = 0;
    virtual auto Data(const opentxs::network::zeromq::Frame& input) const
        -> ByteArray = 0;
    virtual auto Data(const std::uint32_t input) const -> ByteArray = 0;
    virtual auto Data(const std::uint8_t input) const -> ByteArray = 0;
    virtual auto DataFromBytes(const ReadView input) const -> ByteArray = 0;
    virtual auto DataFromHex(const ReadView input) const -> ByteArray = 0;
    virtual auto Identifier(const Cheque& cheque, alloc::Default alloc = {})
        const noexcept -> identifier::Generic = 0;
    virtual auto Identifier(const Contract& contract, alloc::Default alloc = {})
        const noexcept -> identifier::Generic = 0;
    virtual auto Identifier(const Item& item, alloc::Default alloc = {})
        const noexcept -> identifier::Generic = 0;
    virtual auto Identifier(
        const protobuf::Identifier& in,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromBase58(
        const std::string_view base58,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromHash(
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromPreimage(
        const protobuf::MessageType& proto,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromPreimage(
        const protobuf::MessageType& proto,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromPreimage(
        const ReadView preimage,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromProtobuf(
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromRandom(alloc::Default alloc = {}) const noexcept
        -> identifier::Generic = 0;
    virtual auto IdentifierFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto NotaryID(
        const protobuf::Identifier& in,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDConvertSafe(
        const identifier::Generic& in,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromBase58(
        const std::string_view base58,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromHash(
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromPreimage(
        const protobuf::MessageType& proto,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromPreimage(
        const protobuf::MessageType& proto,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromPreimage(
        const ReadView preimage,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromProtobuf(
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromRandom(alloc::Default alloc = {}) const noexcept
        -> identifier::Notary = 0;
    virtual auto NotaryIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NymID(
        const protobuf::Identifier& in,
        alloc::Default alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDConvertSafe(
        const identifier::Generic& in,
        alloc::Default alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromBase58(
        const std::string_view base58,
        alloc::Default alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromHash(const ReadView bytes, alloc::Default alloc = {})
        const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromPreimage(
        const ReadView preimage,
        alloc::Default alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromProtobuf(
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromRandom(alloc::Default alloc = {}) const noexcept
        -> identifier::Nym = 0;
    virtual auto NymIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto Secret(const std::size_t bytes) const noexcept
        -> opentxs::Secret = 0;
    virtual auto SecretFromBytes(const ReadView bytes) const noexcept
        -> opentxs::Secret = 0;
    virtual auto SecretFromText(const std::string_view text) const noexcept
        -> opentxs::Secret = 0;
    virtual auto SeedID(
        const protobuf::Identifier& in,
        alloc::Default alloc = {}) const noexcept -> identifier::HDSeed = 0;
    virtual auto SeedIDFromBase58(
        const std::string_view base58,
        alloc::Default alloc = {}) const noexcept -> identifier::HDSeed = 0;
    virtual auto SeedIDFromHash(const ReadView bytes, alloc::Default alloc = {})
        const noexcept -> identifier::HDSeed = 0;
    virtual auto SeedIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::HDSeed = 0;
    virtual auto SeedIDFromPreimage(
        const ReadView preimage,
        alloc::Default alloc = {}) const noexcept -> identifier::HDSeed = 0;
    virtual auto SeedIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::HDSeed = 0;
    virtual auto SeedIDFromProtobuf(
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept -> identifier::HDSeed = 0;
    virtual auto SeedIDFromRandom(alloc::Default alloc = {}) const noexcept
        -> identifier::HDSeed = 0;
    virtual auto SeedIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept -> identifier::HDSeed = 0;
    virtual auto Self() const noexcept -> const api::Factory& = 0;
    virtual auto Session() const noexcept
        -> const api::session::internal::Factory& = 0;
    virtual auto UnitID(
        const protobuf::Identifier& in,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDConvertSafe(
        const identifier::Generic& in,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromBase58(
        const std::string_view base58,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromHash(const ReadView bytes, alloc::Default alloc = {})
        const noexcept -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromPreimage(
        const protobuf::MessageType& proto,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromPreimage(
        const protobuf::MessageType& proto,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromPreimage(
        const ReadView preimage,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromProtobuf(
        const ReadView bytes,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromRandom(alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromRandom(
        const identifier::Algorithm type,
        alloc::Default alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;

    virtual auto Self() noexcept -> api::Factory& = 0;
    virtual auto Session() noexcept -> api::session::internal::Factory& = 0;

    virtual ~Factory() = default;
};
