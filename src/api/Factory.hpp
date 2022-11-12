// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identifier::Algorithm
// IWYU pragma: no_forward_declare opentxs::identifier::Type
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::ClaimType

#pragma once

#include <cstddef>
#include <iosfwd>
#include <string_view>

#include "internal/api/FactoryAPI.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Algorithm.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Type.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Factory;
}  // namespace api

namespace identifier
{
class Generic;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace proto
{
class HDPath;
class Identifier;
}  // namespace proto

class Cheque;
class Contract;
class Item;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::imp
{
using namespace std::literals;

class Factory final : public internal::Factory
{
public:
    auto Identifier(
        const identity::wot::claim::ClaimType type,
        const proto::HDPath& path,
        allocator_type alloc) const noexcept -> identifier::Generic final;
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
    auto id_from_preimage(
        const identifier::Algorithm type,
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
};
}  // namespace opentxs::api::imp
