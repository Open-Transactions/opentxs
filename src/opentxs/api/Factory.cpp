// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Factory.hpp"  // IWYU pragma: associated

#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api
{
Factory::Factory(internal::Factory* imp) noexcept
    : imp_(imp)
{
    assert_true(nullptr != imp_);
}

auto Factory::AccountIDFromHash(
    const ReadView bytes,
    identifier::AccountSubtype subtype,
    allocator_type alloc) const noexcept -> identifier::Account
{
    return imp_->AccountIDFromHash(bytes, subtype, alloc);
}

auto Factory::AccountIDFromHash(
    const ReadView bytes,
    identifier::AccountSubtype subtype,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Account
{
    return imp_->AccountIDFromHash(bytes, subtype, type, alloc);
}

auto Factory::AccountIDFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::Account
{
    return imp_->AccountIDFromBase58(base58, alloc);
}

auto Factory::AccountIDFromPreimage(
    const ReadView preimage,
    identifier::AccountSubtype subtype,
    allocator_type alloc) const noexcept -> identifier::Account
{
    return imp_->AccountIDFromPreimage(preimage, subtype, alloc);
}

auto Factory::AccountIDFromPreimage(
    const ReadView preimage,
    identifier::AccountSubtype subtype,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Account
{
    return imp_->AccountIDFromPreimage(preimage, subtype, type, alloc);
}

auto Factory::AccountIDFromProtobuf(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Account
{
    return imp_->AccountIDFromProtobuf(bytes, alloc);
}

auto Factory::AccountIDFromRandom(
    identifier::AccountSubtype subtype,
    allocator_type alloc) const noexcept -> identifier::Account
{
    return imp_->AccountIDFromRandom(subtype, alloc);
}

auto Factory::AccountIDFromRandom(
    identifier::AccountSubtype subtype,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Account
{
    return imp_->AccountIDFromRandom(subtype, type, alloc);
}

auto Factory::AccountIDFromZMQ(
    const opentxs::network::zeromq::Frame& frame,
    allocator_type alloc) const noexcept -> identifier::Account
{
    return imp_->AccountIDFromZMQ(frame, alloc);
}

auto Factory::AccountIDFromZMQ(const ReadView frame, allocator_type alloc)
    const noexcept -> identifier::Account
{
    return imp_->AccountIDFromZMQ(frame, alloc);
}

auto Factory::Amount(const opentxs::network::zeromq::Frame& zmq) const noexcept
    -> opentxs::Amount
{
    return imp_->Amount(zmq);
}

auto Factory::BlockchainAddress(
    const opentxs::network::blockchain::Protocol protocol,
    const opentxs::network::blockchain::Transport network,
    const ReadView bytes,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const Set<opentxs::network::blockchain::bitcoin::Service>& services)
    const noexcept -> opentxs::network::blockchain::Address
{
    return imp_->BlockchainAddress(
        protocol, network, bytes, port, chain, lastConnected, services);
}

auto Factory::BlockchainAddress(
    const opentxs::network::blockchain::Protocol protocol,
    const boost::asio::ip::address& address,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const Set<opentxs::network::blockchain::bitcoin::Service>& services)
    const noexcept -> opentxs::network::blockchain::Address
{
    return imp_->BlockchainAddress(
        protocol, address, port, chain, lastConnected, services);
}

auto Factory::BlockchainAddressZMQ(
    const opentxs::network::blockchain::Protocol protocol,
    const opentxs::network::blockchain::Transport subtype,
    const ReadView bytes,
    const blockchain::Type chain,
    const Time lastConnected,
    const Set<opentxs::network::blockchain::bitcoin::Service>& services,
    const ReadView key) const noexcept -> opentxs::network::blockchain::Address
{
    return imp_->BlockchainAddressZMQ(
        protocol, subtype, bytes, chain, lastConnected, services, key);
}

auto Factory::BlockchainAddressZMQ(
    const opentxs::network::blockchain::Protocol protocol,
    const boost::asio::ip::address& address,
    const blockchain::Type chain,
    const Time lastConnected,
    const Set<opentxs::network::blockchain::bitcoin::Service>& services,
    const ReadView key) const noexcept -> opentxs::network::blockchain::Address
{
    return imp_->BlockchainAddressZMQ(
        protocol, address, chain, lastConnected, services, key);
}

auto Factory::Data() const -> ByteArray { return imp_->Data(); }

auto Factory::Data(const opentxs::Armored& input) const -> ByteArray
{
    return imp_->Data(input);
}

auto Factory::Data(const opentxs::network::zeromq::Frame& input) const
    -> ByteArray
{
    return imp_->Data(input);
}

auto Factory::Data(const std::uint8_t input) const -> ByteArray
{
    return imp_->Data(input);
}

auto Factory::Data(const std::uint32_t input) const -> ByteArray
{
    return imp_->Data(input);
}

auto Factory::Data(const UnallocatedVector<unsigned char>& input) const
    -> ByteArray
{
    return imp_->Data(input);
}

auto Factory::Data(const UnallocatedVector<std::byte>& input) const -> ByteArray
{
    return imp_->Data(input);
}

auto Factory::DataFromBytes(const ReadView input) const -> ByteArray
{
    return imp_->DataFromBytes(input);
}

auto Factory::DataFromHex(const ReadView input) const -> ByteArray
{
    return imp_->DataFromHex(input);
}

auto Factory::IdentifierFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return imp_->IdentifierFromBase58(base58, alloc);
}

auto Factory::IdentifierFromHash(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Generic
{
    return imp_->IdentifierFromHash(bytes, alloc);
}

auto Factory::IdentifierFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return imp_->IdentifierFromHash(bytes, type, alloc);
}

auto Factory::IdentifierFromPreimage(
    const ReadView preimage,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return imp_->IdentifierFromPreimage(preimage, alloc);
}

auto Factory::IdentifierFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return imp_->IdentifierFromPreimage(preimage, type, alloc);
}

auto Factory::IdentifierFromProtobuf(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Generic
{
    return imp_->IdentifierFromProtobuf(bytes, alloc);
}

auto Factory::IdentifierFromRandom(allocator_type alloc) const noexcept
    -> identifier::Generic
{
    return imp_->IdentifierFromRandom(alloc);
}

auto Factory::IdentifierFromRandom(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return imp_->IdentifierFromRandom(type, alloc);
}

auto Factory::Internal() const noexcept -> const internal::Factory&
{
    return *imp_;
}

auto Factory::Internal() noexcept -> internal::Factory& { return *imp_; }

auto Factory::NotaryIDFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return imp_->NotaryIDFromBase58(base58, alloc);
}

auto Factory::NotaryIDFromHash(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Notary
{
    return imp_->NotaryIDFromHash(bytes, alloc);
}

auto Factory::NotaryIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return imp_->NotaryIDFromHash(bytes, type, alloc);
}

auto Factory::NotaryIDFromPreimage(
    const ReadView preimage,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return imp_->NotaryIDFromPreimage(preimage, alloc);
}

auto Factory::NotaryIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return imp_->NotaryIDFromPreimage(preimage, type, alloc);
}

auto Factory::NotaryIDFromProtobuf(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Notary
{
    return imp_->NotaryIDFromProtobuf(bytes, alloc);
}

auto Factory::NotaryIDFromRandom(allocator_type alloc) const noexcept
    -> identifier::Notary
{
    return imp_->NotaryIDFromRandom(alloc);
}

auto Factory::NotaryIDFromRandom(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return imp_->NotaryIDFromRandom(type, alloc);
}

auto Factory::NymIDFromHash(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Nym
{
    return imp_->NymIDFromHash(bytes, alloc);
}

auto Factory::NymIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Nym
{
    return imp_->NymIDFromHash(bytes, type, alloc);
}

auto Factory::NymIDFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::Nym
{
    return imp_->NymIDFromBase58(base58, alloc);
}

auto Factory::NymIDFromPreimage(const ReadView preimage, allocator_type alloc)
    const noexcept -> identifier::Nym
{
    return imp_->NymIDFromPreimage(preimage, alloc);
}

auto Factory::NymIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Nym
{
    return imp_->NymIDFromPreimage(preimage, type, alloc);
}

auto Factory::NymIDFromProtobuf(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Nym
{
    return imp_->NymIDFromProtobuf(bytes, alloc);
}

auto Factory::NymIDFromRandom(allocator_type alloc) const noexcept
    -> identifier::Nym
{
    return imp_->NymIDFromRandom(alloc);
}

auto Factory::NymIDFromRandom(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Nym
{
    return imp_->NymIDFromRandom(type, alloc);
}

auto Factory::Secret(const std::size_t bytes) const noexcept -> opentxs::Secret
{
    return imp_->Secret(bytes);
}

auto Factory::SecretFromBytes(const ReadView bytes) const noexcept
    -> opentxs::Secret
{
    return imp_->SecretFromBytes(bytes);
}

auto Factory::SecretFromText(const std::string_view text) const noexcept
    -> opentxs::Secret
{
    return imp_->SecretFromText(text);
}

auto Factory::SeedIDFromHash(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::HDSeed
{
    return imp_->SeedIDFromHash(bytes, alloc);
}

auto Factory::SeedIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::HDSeed
{
    return imp_->SeedIDFromHash(bytes, type, alloc);
}

auto Factory::SeedIDFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::HDSeed
{
    return imp_->SeedIDFromBase58(base58, alloc);
}

auto Factory::SeedIDFromPreimage(const ReadView preimage, allocator_type alloc)
    const noexcept -> identifier::HDSeed
{
    return imp_->SeedIDFromPreimage(preimage, alloc);
}

auto Factory::SeedIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::HDSeed
{
    return imp_->SeedIDFromPreimage(preimage, type, alloc);
}

auto Factory::SeedIDFromProtobuf(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::HDSeed
{
    return imp_->SeedIDFromProtobuf(bytes, alloc);
}

auto Factory::SeedIDFromRandom(allocator_type alloc) const noexcept
    -> identifier::HDSeed
{
    return imp_->SeedIDFromRandom(alloc);
}

auto Factory::SeedIDFromRandom(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::HDSeed
{
    return imp_->SeedIDFromRandom(type, alloc);
}

auto Factory::UnitIDFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return imp_->UnitIDFromBase58(base58, alloc);
}

auto Factory::UnitIDFromHash(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::UnitDefinition
{
    return imp_->UnitIDFromHash(bytes, alloc);
}

auto Factory::UnitIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return imp_->UnitIDFromHash(bytes, type, alloc);
}

auto Factory::UnitIDFromPreimage(const ReadView preimage, allocator_type alloc)
    const noexcept -> identifier::UnitDefinition
{
    return imp_->UnitIDFromPreimage(preimage, alloc);
}

auto Factory::UnitIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return imp_->UnitIDFromPreimage(preimage, type, alloc);
}

auto Factory::UnitIDFromProtobuf(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::UnitDefinition
{
    return imp_->UnitIDFromProtobuf(bytes, alloc);
}

auto Factory::UnitIDFromRandom(allocator_type alloc) const noexcept
    -> identifier::UnitDefinition
{
    return imp_->UnitIDFromRandom(alloc);
}

auto Factory::UnitIDFromRandom(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return imp_->UnitIDFromRandom(type, alloc);
}

Factory::~Factory()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api
