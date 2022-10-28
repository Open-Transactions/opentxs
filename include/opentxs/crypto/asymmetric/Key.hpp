// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Role.hpp"

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
namespace internal
{
class Key;
}  // namespace internal

namespace key
{
class EllipticCurve;
class RSA;
}  // namespace key

class KeyPrivate;
}  // namespace asymmetric
}  // namespace crypto

class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric
{
class OPENTXS_EXPORT Key : virtual public opentxs::Allocated
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Key&;
    static auto DefaultVersion() noexcept -> VersionNumber;
    static auto MaxVersion() noexcept -> VersionNumber;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    auto asEllipticCurve() const noexcept -> const key::EllipticCurve&;
    auto asPublic(allocator_type alloc = {}) const noexcept -> Key;
    auto asRSA() const noexcept -> const key::RSA&;
    auto get_allocator() const noexcept -> allocator_type final;
    auto HasCapability(identity::NymCapability capability) const noexcept
        -> bool;
    auto HasPrivate() const noexcept -> bool;
    auto HasPublic() const noexcept -> bool;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Key&;
    [[nodiscard]] auto IsValid() const noexcept -> bool;
    auto PreferredHash() const noexcept -> crypto::HashType;
    auto PrivateKey(const PasswordPrompt& reason) const noexcept -> ReadView;
    auto PublicKey() const noexcept -> ReadView;
    auto Role() const noexcept -> asymmetric::Role;
    auto Sign(
        ReadView preimage,
        Writer&& output,
        crypto::HashType hash,
        const PasswordPrompt& reason) const noexcept -> bool;
    auto Type() const noexcept -> asymmetric::Algorithm;
    auto Verify(ReadView plaintext, ReadView sig) const noexcept -> bool;
    auto Version() const noexcept -> VersionNumber;

    auto asEllipticCurve() noexcept -> key::EllipticCurve&;
    auto asRSA() noexcept -> key::RSA&;
    [[nodiscard]] auto ErasePrivateData() noexcept -> bool;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Key&;
    auto swap(Key& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Key(KeyPrivate* imp) noexcept;
    Key(allocator_type alloc = {}) noexcept;
    Key(const Key& rhs, allocator_type alloc = {}) noexcept;
    Key(Key&& rhs) noexcept;
    Key(Key&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Key& rhs) noexcept -> Key&;
    auto operator=(Key&& rhs) noexcept -> Key&;

    ~Key() override;

protected:
    friend KeyPrivate;

    KeyPrivate* imp_;
};
}  // namespace opentxs::crypto::asymmetric
