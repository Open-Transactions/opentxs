// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Factory;
}  // namespace api

namespace crypto
{
class AsymmetricProvider;
class EcdsaProvider;
class OpenSSL;
class Secp256k1;
class Sodium;
class SymmetricProvider;
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::internal
{
class Crypto : virtual public api::Crypto
{
public:
    virtual auto AsymmetricProvider(opentxs::crypto::asymmetric::Algorithm type)
        const noexcept -> const opentxs::crypto::AsymmetricProvider& = 0;
    virtual auto EllipticProvider(opentxs::crypto::asymmetric::Algorithm type)
        const noexcept -> const opentxs::crypto::EcdsaProvider& = 0;
    auto Internal() const noexcept -> const Crypto& final { return *this; }
    virtual auto Libsecp256k1() const noexcept
        -> const opentxs::crypto::Secp256k1& = 0;
    virtual auto Libsodium() const noexcept
        -> const opentxs::crypto::Sodium& = 0;
    virtual auto OpenSSL() const noexcept
        -> const opentxs::crypto::OpenSSL& = 0;
    virtual auto SymmetricProvider(opentxs::crypto::symmetric::Algorithm type)
        const noexcept -> const opentxs::crypto::SymmetricProvider& = 0;
    virtual auto SymmetricProvider(opentxs::crypto::symmetric::Source type)
        const noexcept -> const opentxs::crypto::SymmetricProvider& = 0;
    virtual auto hasLibsecp256k1() const noexcept -> bool = 0;
    virtual auto hasOpenSSL() const noexcept -> bool = 0;
    virtual auto hasSodium() const noexcept -> bool = 0;

    virtual auto Init(
        const std::shared_ptr<const api::Factory>& factory) noexcept
        -> void = 0;
    auto Internal() noexcept -> Crypto& final { return *this; }

    ~Crypto() override = default;
};
}  // namespace opentxs::api::internal
