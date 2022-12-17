// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/crypto/asymmetric/key/EllipticCurve.hpp"
// IWYU pragma: no_include "internal/crypto/asymmetric/key/RSA.hpp"

#pragma once

#include <functional>

#include "internal/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
namespace key
{
class Ed25519Private;
class EllipticCurve;
class EllipticCurvePrivate;
class HDPrivate;
class RSA;
class RSAPrivate;
class Secp256k1Private;
}  // namespace key

class Key;
}  // namespace asymmetric

}  // namespace crypto
class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric
{
class KeyPrivate : virtual public internal::Key,
                   public opentxs::implementation::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> KeyPrivate*;
    static auto Reset(asymmetric::Key& key) noexcept -> void;

    [[nodiscard]] virtual auto asEllipticCurvePrivate() const noexcept
        -> const key::EllipticCurvePrivate*;
    [[nodiscard]] virtual auto asEllipticCurvePublic() const noexcept
        -> const key::EllipticCurve&;
    [[nodiscard]] virtual auto asEd25519Private() const noexcept
        -> const key::Ed25519Private*;
    [[nodiscard]] virtual auto asHDPrivate() const noexcept
        -> const key::HDPrivate*;
    [[nodiscard]] virtual auto asPublic(allocator_type alloc) const noexcept
        -> asymmetric::Key;
    [[nodiscard]] virtual auto asRSAPrivate() const noexcept
        -> const key::RSAPrivate*;
    [[nodiscard]] virtual auto asRSAPublic() const noexcept -> const key::RSA&;
    [[nodiscard]] virtual auto asSecp256k1Private() const noexcept
        -> const key::Secp256k1Private*;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> KeyPrivate*;
    [[nodiscard]] virtual auto get_deleter() const noexcept
        -> std::function<void(KeyPrivate*)>;
    [[nodiscard]] virtual auto HasCapability(
        identity::NymCapability capability) const noexcept -> bool;
    [[nodiscard]] virtual auto HasPrivate() const noexcept -> bool;
    [[nodiscard]] virtual auto HasPublic() const noexcept -> bool;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] virtual auto PreferredHash() const noexcept
        -> crypto::HashType;
    [[nodiscard]] virtual auto PrivateKey(
        const PasswordPrompt& reason) const noexcept -> ReadView;
    [[nodiscard]] virtual auto PublicKey() const noexcept -> ReadView;
    [[nodiscard]] virtual auto Role() const noexcept -> asymmetric::Role;
    using internal::Key::Sign;
    [[nodiscard]] virtual auto Sign(
        ReadView preimage,
        Writer&& output,
        crypto::HashType hash,
        const PasswordPrompt& reason) const noexcept -> bool;
    [[nodiscard]] virtual auto Type() const noexcept -> asymmetric::Algorithm;
    using internal::Key::Verify;
    [[nodiscard]] virtual auto Verify(ReadView plaintext, ReadView sig)
        const noexcept -> bool;
    [[nodiscard]] virtual auto Version() const noexcept -> VersionNumber;

    [[nodiscard]] virtual auto asEd25519Private() noexcept
        -> key::Ed25519Private*;
    [[nodiscard]] virtual auto asEllipticCurvePrivate() noexcept
        -> key::EllipticCurvePrivate*;
    [[nodiscard]] virtual auto asEllipticCurvePublic() noexcept
        -> key::EllipticCurve&;
    [[nodiscard]] virtual auto asHDPrivate() noexcept -> key::HDPrivate*;
    [[nodiscard]] virtual auto asRSAPrivate() noexcept -> key::RSAPrivate*;
    [[nodiscard]] virtual auto asSecp256k1Private() noexcept
        -> key::Secp256k1Private*;
    virtual auto asRSAPublic() noexcept -> key::RSA&;
    [[nodiscard]] virtual auto ErasePrivateData() noexcept -> bool;

    KeyPrivate(allocator_type alloc) noexcept;
    KeyPrivate() = delete;
    KeyPrivate(const KeyPrivate& rhs, allocator_type alloc) noexcept;
    KeyPrivate(const KeyPrivate&) = delete;
    KeyPrivate(KeyPrivate&&) = delete;
    auto operator=(const KeyPrivate&) -> KeyPrivate& = delete;
    auto operator=(KeyPrivate&&) -> KeyPrivate& = delete;

    ~KeyPrivate() override;
};
}  // namespace opentxs::crypto::asymmetric
