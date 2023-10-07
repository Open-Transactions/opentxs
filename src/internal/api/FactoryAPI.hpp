// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/Armored.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace google
{
namespace protobuf
{
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace opentxs
{
namespace crypto
{
class Envelope;
}  // namespace crypto

namespace identifier
{
class Generic;
class HDSeed;
}  // namespace identifier

namespace proto
{
class BlockchainPeerAddress;
class HDPath;
class Identifier;
}  // namespace proto

class ByteArray;
class Cheque;
class Data;
class String;
class Contract;
class Item;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::internal
{
class Factory : virtual public api::Factory
{
public:
    virtual auto AccountID(
        const identity::wot::claim::ClaimType type,
        const proto::HDPath& path,
        allocator_type alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountID(
        const proto::Identifier& in,
        allocator_type alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountID(const Contract& contract, allocator_type alloc = {})
        const noexcept -> identifier::Account = 0;
    virtual auto AccountIDConvertSafe(
        const identifier::Generic& in,
        allocator_type alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto Armored() const -> OTArmored = 0;
    virtual auto Armored(const UnallocatedCString& input) const
        -> OTArmored = 0;
    virtual auto Armored(const opentxs::Data& input) const -> OTArmored = 0;
    virtual auto Armored(const opentxs::String& input) const -> OTArmored = 0;
    virtual auto Armored(const opentxs::crypto::Envelope& input) const
        -> OTArmored = 0;
    virtual auto Armored(const google::protobuf::MessageLite& input) const
        -> OTArmored = 0;
    virtual auto Armored(
        const google::protobuf::MessageLite& input,
        const UnallocatedCString& header) const -> OTString = 0;
    using api::Factory::BlockchainAddress;
    virtual auto BlockchainAddress(
        const proto::BlockchainPeerAddress& serialized) const noexcept
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
    using api::Factory::Data;
    virtual auto Data(const google::protobuf::MessageLite& input) const
        -> ByteArray = 0;
    virtual auto Identifier(const Cheque& cheque, allocator_type alloc = {})
        const noexcept -> identifier::Generic = 0;
    virtual auto Identifier(const Contract& contract, allocator_type alloc = {})
        const noexcept -> identifier::Generic = 0;
    virtual auto Identifier(const Item& item, allocator_type alloc = {})
        const noexcept -> identifier::Generic = 0;
    virtual auto Identifier(
        const proto::Identifier& in,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
    using api::Factory::IdentifierFromPreimage;
    virtual auto IdentifierFromPreimage(
        const ProtobufType& proto,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromPreimage(
        const ProtobufType& proto,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto NotaryID(
        const proto::Identifier& in,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDConvertSafe(
        const identifier::Generic& in,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    using api::Factory::NotaryIDFromPreimage;
    virtual auto NotaryIDFromPreimage(
        const ProtobufType& proto,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromPreimage(
        const ProtobufType& proto,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NymID(const proto::Identifier& in, allocator_type alloc = {})
        const noexcept -> identifier::Nym = 0;
    virtual auto NymIDConvertSafe(
        const identifier::Generic& in,
        allocator_type alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto SeedID(const proto::Identifier& in, allocator_type alloc = {})
        const noexcept -> identifier::HDSeed = 0;
    virtual auto UnitID(const proto::Identifier& in, allocator_type alloc = {})
        const noexcept -> identifier::UnitDefinition = 0;
    virtual auto UnitIDConvertSafe(
        const identifier::Generic& in,
        allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    using api::Factory::UnitIDFromPreimage;
    virtual auto UnitIDFromPreimage(
        const ProtobufType& proto,
        allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromPreimage(
        const ProtobufType& proto,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    auto Internal() const noexcept -> const Factory& final { return *this; }

    auto Internal() noexcept -> Factory& final { return *this; }

    ~Factory() override = default;
};
}  // namespace opentxs::api::internal
