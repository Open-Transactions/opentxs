// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

/// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
namespace internal
{
class Asymmetric;
}  // namespace internal
}  // namespace crypto
}  // namespace api

namespace crypto
{
namespace asymmetric
{
namespace key
{
class HD;
class Secp256k1;
}  // namespace key

class Key;
}  // namespace asymmetric

class Parameters;
}  // namespace crypto

class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto
{
/**
 The api::crypto::Asymmetric API is used for instantiating asymmetric keys and
 related crypto objects.
 */
class OPENTXS_EXPORT Asymmetric
{
public:
    virtual auto InstantiateKey(
        const opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::crypto::SeedID& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto InstantiateKey(
        const opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::crypto::SeedID& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto InstantiateKey(
        const opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::crypto::SeedID& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto InstantiateKey(
        const opentxs::crypto::asymmetric::Algorithm type,
        const opentxs::crypto::SeedID& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const noexcept
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Asymmetric& = 0;
    virtual auto NewHDKey(
        const opentxs::crypto::SeedID& seedID,
        const Secret& seed,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto NewHDKey(
        const opentxs::crypto::SeedID& seedID,
        const Secret& seed,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto NewHDKey(
        const opentxs::crypto::SeedID& seedID,
        const Secret& seed,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto NewHDKey(
        const opentxs::crypto::SeedID& seedID,
        const Secret& seed,
        const opentxs::crypto::EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto NewKey(
        const opentxs::crypto::Parameters& params,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto NewKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto NewKey(
        const opentxs::crypto::Parameters& params,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto NewKey(
        const opentxs::crypto::Parameters& params,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::Key = 0;
    virtual auto NewSecp256k1Key(
        const opentxs::crypto::SeedID& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto NewSecp256k1Key(
        const opentxs::crypto::SeedID& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto NewSecp256k1Key(
        const opentxs::crypto::SeedID& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto NewSecp256k1Key(
        const opentxs::crypto::SeedID& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Asymmetric& = 0;

    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    auto operator=(const Asymmetric&) -> Asymmetric& = delete;
    auto operator=(Asymmetric&&) -> Asymmetric& = delete;

    OPENTXS_NO_EXPORT virtual ~Asymmetric() = default;

protected:
    Asymmetric() = default;
};
}  // namespace opentxs::api::crypto
