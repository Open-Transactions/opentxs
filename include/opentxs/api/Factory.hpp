// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

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
class Factory;
}  // namespace internal

class Factory;  // IWYU pragma: keep
}  // namespace api

namespace identifier
{
class Account;
class Generic;
class HDSeed;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain

namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

class Amount;
class Armored;
class ByteArray;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/**
 The top-level Factory API, used for instantiating secrets.
 A Secret is a piece of data, similar to a string or byte vector.
 But secrets, unlike normal strings or byte vectors, have additional secrecy
 requirements. They are used to store, for example, private keys. They have
 additional security requirements such as wiping their memory to zero when
 destructed.
 */
class OPENTXS_EXPORT opentxs::api::Factory
{
public:
    using allocator_type = alloc::Default;

    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        allocator_type alloc = {}) const noexcept -> identifier::Account;
    auto AccountIDFromHash(
        const ReadView bytes,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Account;
    auto AccountIDFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept -> identifier::Account;
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        allocator_type alloc = {}) const noexcept -> identifier::Account;
    auto AccountIDFromPreimage(
        const ReadView preimage,
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Account;
    auto AccountIDFromProtobuf(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::Account;
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        allocator_type alloc = {}) const noexcept -> identifier::Account;
    auto AccountIDFromRandom(
        identifier::AccountSubtype subtype,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Account;
    auto AccountIDFromZMQ(
        const opentxs::network::zeromq::Frame& frame,
        allocator_type alloc = {}) const noexcept -> identifier::Account;
    auto AccountIDFromZMQ(const ReadView frame, allocator_type alloc = {})
        const noexcept -> identifier::Account;
    auto Amount(const opentxs::network::zeromq::Frame& zmq) const noexcept
        -> opentxs::Amount;
    auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const noexcept -> opentxs::network::blockchain::Address;
    auto BlockchainAddress(
        const opentxs::network::blockchain::Protocol protocol,
        const boost::asio::ip::address& address,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services)
        const noexcept -> opentxs::network::blockchain::Address;
    auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const opentxs::network::blockchain::Transport subtype,
        const ReadView bytes,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const noexcept
        -> opentxs::network::blockchain::Address;
    auto BlockchainAddressZMQ(
        const opentxs::network::blockchain::Protocol protocol,
        const boost::asio::ip::address& address,
        const blockchain::Type chain,
        const Time lastConnected,
        const Set<opentxs::network::blockchain::bitcoin::Service>& services,
        const ReadView key) const noexcept
        -> opentxs::network::blockchain::Address;
    auto Data() const -> ByteArray;
    auto Data(const opentxs::Armored& input) const -> ByteArray;
    auto Data(const opentxs::network::zeromq::Frame& input) const -> ByteArray;
    auto Data(const std::uint8_t input) const -> ByteArray;
    auto Data(const std::uint32_t input) const -> ByteArray;
    auto Data(const UnallocatedVector<unsigned char>& input) const -> ByteArray;
    auto Data(const UnallocatedVector<std::byte>& input) const -> ByteArray;
    auto DataFromBytes(const ReadView input) const -> ByteArray;
    auto DataFromHex(const ReadView input) const -> ByteArray;
    auto IdentifierFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept -> identifier::Generic;
    auto IdentifierFromHash(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::Generic;
    auto IdentifierFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Generic;
    auto IdentifierFromPreimage(
        const ReadView preimage,
        allocator_type alloc = {}) const noexcept -> identifier::Generic;
    auto IdentifierFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Generic;
    auto IdentifierFromProtobuf(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::Generic;
    auto IdentifierFromRandom(allocator_type alloc = {}) const noexcept
        -> identifier::Generic;
    auto IdentifierFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Generic;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Factory&;
    auto NotaryIDFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept -> identifier::Notary;
    auto NotaryIDFromHash(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::Notary;
    auto NotaryIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Notary;
    auto NotaryIDFromPreimage(
        const ReadView preimage,
        allocator_type alloc = {}) const noexcept -> identifier::Notary;
    auto NotaryIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Notary;
    auto NotaryIDFromProtobuf(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::Notary;
    auto NotaryIDFromRandom(allocator_type alloc = {}) const noexcept
        -> identifier::Notary;
    auto NotaryIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Notary;
    auto NymIDFromHash(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::Nym;
    auto NymIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Nym;
    auto NymIDFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept -> identifier::Nym;
    auto NymIDFromPreimage(const ReadView preimage, allocator_type alloc = {})
        const noexcept -> identifier::Nym;
    auto NymIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Nym;
    auto NymIDFromProtobuf(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::Nym;
    auto NymIDFromRandom(allocator_type alloc = {}) const noexcept
        -> identifier::Nym;
    auto NymIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Nym;
    auto Secret(const std::size_t bytes) const noexcept -> opentxs::Secret;
    auto SecretFromBytes(const ReadView bytes) const noexcept
        -> opentxs::Secret;
    auto SecretFromText(const std::string_view text) const noexcept
        -> opentxs::Secret;
    auto SeedIDFromHash(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::HDSeed;
    auto SeedIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::HDSeed;
    auto SeedIDFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept -> identifier::HDSeed;
    auto SeedIDFromPreimage(const ReadView preimage, allocator_type alloc = {})
        const noexcept -> identifier::HDSeed;
    auto SeedIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::HDSeed;
    auto SeedIDFromProtobuf(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::HDSeed;
    auto SeedIDFromRandom(allocator_type alloc = {}) const noexcept
        -> identifier::HDSeed;
    auto SeedIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::HDSeed;
    auto UnitIDFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept -> identifier::UnitDefinition;
    auto UnitIDFromHash(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::UnitDefinition;
    auto UnitIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::UnitDefinition;
    auto UnitIDFromPreimage(const ReadView preimage, allocator_type alloc = {})
        const noexcept -> identifier::UnitDefinition;
    auto UnitIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::UnitDefinition;
    auto UnitIDFromProtobuf(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::UnitDefinition;
    auto UnitIDFromRandom(allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition;
    auto UnitIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::UnitDefinition;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Factory&;

    Factory(internal::Factory* imp) noexcept;
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;

    OPENTXS_NO_EXPORT virtual ~Factory();

protected:
    friend internal::Factory;

    internal::Factory* imp_;
};
