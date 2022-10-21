// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/asymmetric/Key.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
namespace internal
{
namespace key
{
class Ed25519;
class HD;
class Secp256k1;
}  // namespace key
}  // namespace internal
}  // namespace asymmetric

class EcdsaProvider;
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::internal::key
{
class EllipticCurve : virtual public Key
{
public:
    static auto Blank() noexcept -> EllipticCurve&;

    virtual auto asEd25519() const noexcept -> const key::Ed25519&;
    virtual auto asHD() const noexcept -> const key::HD&;
    virtual auto asSecp256k1() const noexcept -> const key::Secp256k1&;
    virtual auto ECDSA() const noexcept -> const crypto::EcdsaProvider&;

    virtual auto asEd25519() noexcept -> key::Ed25519&;
    virtual auto asHD() noexcept -> key::HD&;
    virtual auto asSecp256k1() noexcept -> key::Secp256k1&;

    EllipticCurve(const EllipticCurve&) = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    auto operator=(const EllipticCurve& rhs) noexcept
        -> EllipticCurve& = delete;
    auto operator=(EllipticCurve&& rhs) noexcept -> EllipticCurve& = delete;

    ~EllipticCurve() override = default;

protected:
    EllipticCurve() = default;
};
}  // namespace opentxs::crypto::asymmetric::internal::key
