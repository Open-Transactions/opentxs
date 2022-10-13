// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/key/asymmetric/Role.hpp"
// IWYU pragma: no_include "opentxs/crypto/key/symmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/crypto/key/symmetric/Source.hpp"

#pragma once

#include <Enums.pb.h>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class Asymmetric;
class Ed25519;
class Keypair;
class RSA;
class Secp256k1;
}  // namespace key

class AsymmetricProvider;
class EcdsaProvider;
class Parameters;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class HDPath;
}  // namespace proto

class Data;
class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto Ed25519Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>;
auto Ed25519Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>;
auto Ed25519Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>;
auto Keypair() noexcept -> std::unique_ptr<crypto::key::Keypair>;
auto Keypair(
    const api::Session& api,
    const opentxs::crypto::key::asymmetric::Role role,
    std::unique_ptr<crypto::key::Asymmetric> publicKey,
    std::unique_ptr<crypto::key::Asymmetric> privateKey) noexcept(false)
    -> std::unique_ptr<crypto::key::Keypair>;
auto RSAKey(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& input) noexcept
    -> std::unique_ptr<crypto::key::RSA>;
auto RSAKey(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::key::asymmetric::Role input,
    const VersionNumber version,
    const crypto::Parameters& options,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::RSA>;
auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const Data& publicKey,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
auto Secp256k1Key(
    const api::Session& api,
    const ReadView key,
    const ReadView chaincode) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
}  // namespace opentxs::factory
