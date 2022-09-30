// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Factory;
}  // namespace internal
}  // namespace api

namespace identifier
{
class Generic;
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api
{
/**
 The top-level Factory API, used for instantiating secrets.
 A Secret is a piece of data, similar to a string or byte vector.
 But secrets, unlike normal strings or byte vectors, have additional secrecy
 requirements. They are used to store, for example, private keys. They have
 additional security requirements such as wiping their memory to zero when
 destructed.
 */
class OPENTXS_EXPORT Factory
{
public:
    using allocator_type = alloc::Default;

    virtual auto IdentifierFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromHash(
        const ReadView bytes,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromPreimage(
        const ReadView preimage,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
    virtual auto IdentifierFromRandom(allocator_type alloc = {}) const noexcept
        -> identifier::Generic = 0;
    virtual auto IdentifierFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Generic = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Factory& = 0;
    virtual auto NotaryIDFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromHash(
        const ReadView bytes,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromPreimage(
        const ReadView preimage,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NotaryIDFromRandom(allocator_type alloc = {}) const noexcept
        -> identifier::Notary = 0;
    virtual auto NotaryIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Notary = 0;
    virtual auto NymIDFromHash(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromPreimage(
        const ReadView preimage,
        allocator_type alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto NymIDFromRandom(allocator_type alloc = {}) const noexcept
        -> identifier::Nym = 0;
    virtual auto NymIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept -> identifier::Nym = 0;
    virtual auto Secret(const std::size_t bytes) const noexcept -> OTSecret = 0;
    virtual auto SecretFromBytes(const ReadView bytes) const noexcept
        -> OTSecret = 0;
    virtual auto SecretFromText(const std::string_view text) const noexcept
        -> OTSecret = 0;
    virtual auto UnitIDFromBase58(
        const std::string_view base58,
        allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromHash(const ReadView bytes, allocator_type alloc = {})
        const noexcept -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromHash(
        const ReadView bytes,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromPreimage(
        const ReadView preimage,
        allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromPreimage(
        const ReadView preimage,
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromRandom(allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;
    virtual auto UnitIDFromRandom(
        const identifier::Algorithm type,
        allocator_type alloc = {}) const noexcept
        -> identifier::UnitDefinition = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Factory& = 0;

    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;

    OPENTXS_NO_EXPORT virtual ~Factory() = default;

protected:
    Factory() noexcept = default;
};
}  // namespace opentxs::api
