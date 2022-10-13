// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/crypto/key/symmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/crypto/key/symmetric/Source.hpp"

#pragma once

#include "opentxs/api/session/Crypto.hpp"

#include <memory>
#include <mutex>
#include <type_traits>

#include "internal/api/Crypto.hpp"
#include "internal/api/crypto/Null.hpp"
#include "internal/api/session/Crypto.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/key/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Config;
class Encode;
class Hash;
class Seed;
class Symmetric;
class Util;
}  // namespace crypto

namespace internal
{
class Crypto;
}  // namespace internal

namespace session
{
class Endpoints;
class Factory;
class Storage;
}  // namespace session

class Factory;
class Session;
}  // namespace api

namespace crypto
{
class Bip39;
class EcdsaProvider;
class OpenSSL;
class Secp256k1;
class Sodium;
class SymmetricProvider;
}  // namespace crypto

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class Crypto final : public internal::Crypto
{
public:
    auto Asymmetric() const noexcept -> const crypto::Asymmetric& final
    {
        return asymmetric_;
    }
    auto AsymmetricProvider(opentxs::crypto::key::asymmetric::Algorithm type)
        const noexcept -> const opentxs::crypto::AsymmetricProvider& final
    {
        return parent_.AsymmetricProvider(type);
    }
    auto BIP32() const noexcept -> const opentxs::crypto::Bip32& final
    {
        return parent_.BIP32();
    }
    auto BIP39() const noexcept -> const opentxs::crypto::Bip39& final
    {
        return parent_.BIP39();
    }
    auto Blockchain() const noexcept -> const crypto::Blockchain& final;
    auto Config() const noexcept -> const crypto::Config& final
    {
        return parent_.Config();
    }
    auto EllipticProvider(opentxs::crypto::key::asymmetric::Algorithm type)
        const noexcept -> const opentxs::crypto::EcdsaProvider& final
    {
        return parent_.EllipticProvider(type);
    }
    auto Encode() const noexcept -> const crypto::Encode& final
    {
        return parent_.Encode();
    }
    auto Hash() const noexcept -> const crypto::Hash& final
    {
        return parent_.Hash();
    }
    auto Libsecp256k1() const noexcept
        -> const opentxs::crypto::Secp256k1& final
    {
        return parent_.Libsecp256k1();
    }
    auto Libsodium() const noexcept -> const opentxs::crypto::Sodium& final
    {
        return parent_.Libsodium();
    }
    auto OpenSSL() const noexcept -> const opentxs::crypto::OpenSSL& final
    {
        return parent_.OpenSSL();
    }
    auto Seed() const noexcept -> const crypto::Seed& final { return seed_; }
    auto Symmetric() const noexcept -> const crypto::Symmetric& final
    {
        return symmetric_;
    }
    auto SymmetricProvider(opentxs::crypto::symmetric::Algorithm type)
        const noexcept -> const opentxs::crypto::SymmetricProvider& final
    {
        return parent_.SymmetricProvider(type);
    }
    auto SymmetricProvider(opentxs::crypto::symmetric::Source type)
        const noexcept -> const opentxs::crypto::SymmetricProvider& final
    {
        return parent_.SymmetricProvider(type);
    }
    auto Util() const noexcept -> const crypto::Util& final
    {
        return parent_.Util();
    }
    auto hasLibsecp256k1() const noexcept -> bool final
    {
        return parent_.hasLibsecp256k1();
    }
    auto hasOpenSSL() const noexcept -> bool final
    {
        return parent_.hasOpenSSL();
    }
    auto hasSodium() const noexcept -> bool final
    {
        return parent_.hasSodium();
    }

    auto Cleanup() noexcept -> void final;
    using internal::Crypto::Init;
    auto Init(const std::shared_ptr<const api::Factory>& factory) noexcept
        -> void final
    {
        parent_.Init(factory);
    }
    auto Init(
        const std::shared_ptr<const crypto::Blockchain>& blockchain) noexcept
        -> void final;
    auto PrepareShutdown() noexcept -> void final;

    Crypto(
        api::Crypto& parent,
        const api::Session& session,
        const api::session::Endpoints& endpoints,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const opentxs::network::zeromq::Context& zmq) noexcept;
    Crypto() = delete;
    Crypto(const Crypto&) = delete;
    Crypto(Crypto&&) = delete;
    auto operator=(const Crypto&) -> Crypto& = delete;
    auto operator=(Crypto&&) -> Crypto& = delete;

    ~Crypto() final;

private:
    mutable std::mutex lock_;
    const crypto::blank::Blockchain blank_blockchain_;
    std::shared_ptr<const crypto::Blockchain> blockchain_;
    api::internal::Crypto& parent_;
    const api::crypto::Asymmetric& asymmetric_;
    const api::crypto::Symmetric& symmetric_;
    std::unique_ptr<api::crypto::Seed> seed_p_;
    const api::crypto::Seed& seed_;
};
}  // namespace opentxs::api::session::imp
