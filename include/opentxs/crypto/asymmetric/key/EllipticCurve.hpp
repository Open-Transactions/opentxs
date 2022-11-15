// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::HashType

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
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
namespace key
{
class Ed25519;
class HD;
class Secp256k1;
}  // namespace key

class KeyPrivate;
}  // namespace asymmetric
}  // namespace crypto

class PasswordPrompt;
class Secret;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key
{
class OPENTXS_EXPORT EllipticCurve : public Key
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> EllipticCurve&;
    static auto DefaultVersion() noexcept -> VersionNumber;
    static auto MaxVersion() noexcept -> VersionNumber;

    auto asEd25519() const noexcept -> const key::Ed25519&;
    auto asHD() const noexcept -> const asymmetric::key::HD&;
    auto asSecp256k1() const noexcept -> const key::Secp256k1&;
    auto IncrementPrivate(
        const Secret& scalar,
        const PasswordPrompt& reason,
        allocator_type alloc = {}) const noexcept -> EllipticCurve;
    auto IncrementPublic(const Secret& scalar, allocator_type alloc = {})
        const noexcept -> EllipticCurve;
    auto SignDER(
        const ReadView preimage,
        const crypto::HashType hash,
        Writer&& output,
        const PasswordPrompt& reason) const noexcept -> bool;

    auto asEd25519() noexcept -> key::Ed25519&;
    auto asHD() noexcept -> asymmetric::key::HD&;
    auto asSecp256k1() noexcept -> key::Secp256k1&;

    OPENTXS_NO_EXPORT EllipticCurve(KeyPrivate* imp) noexcept;
    EllipticCurve(allocator_type alloc = {}) noexcept;
    EllipticCurve(const EllipticCurve& rhs, allocator_type alloc = {}) noexcept;
    EllipticCurve(EllipticCurve&& rhs) noexcept;
    EllipticCurve(EllipticCurve&& rhs, allocator_type alloc) noexcept;
    auto operator=(const EllipticCurve& rhs) noexcept -> EllipticCurve&;
    auto operator=(EllipticCurve&& rhs) noexcept -> EllipticCurve&;

    ~EllipticCurve() override;
};
}  // namespace opentxs::crypto::asymmetric::key
