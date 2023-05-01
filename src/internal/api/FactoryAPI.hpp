// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
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

namespace opentxs::api::internal
{
class Factory : virtual public api::Factory
{
public:
    virtual auto AccountID(
        const proto::Identifier& in,
        allocator_type alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto AccountIDConvertSafe(
        const identifier::Generic& in,
        allocator_type alloc = {}) const noexcept -> identifier::Account = 0;
    virtual auto Identifier(
        const identity::wot::claim::ClaimType type,
        const proto::HDPath& path,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
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
