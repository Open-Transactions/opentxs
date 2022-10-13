// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/key/asymmetric/Role.hpp"

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
namespace internal
{
class Seed;
}  // namespace internal
}  // namespace crypto
}  // namespace api

namespace crypto
{
namespace key
{
class HD;
class Secp256k1;
}  // namespace key

class Seed;
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier

class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto
{
/**
 The api::crypto::Seed API contains various seed-related crypto functions.
 */
class OPENTXS_EXPORT Seed
{
public:
    virtual auto AccountChildKey(
        const ReadView& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto AllowedSeedTypes() const noexcept -> const
        UnallocatedMap<opentxs::crypto::SeedStyle, UnallocatedCString>& = 0;
    virtual auto AllowedLanguages(
        const opentxs::crypto::SeedStyle type) const noexcept -> const
        UnallocatedMap<opentxs::crypto::Language, UnallocatedCString>& = 0;
    virtual auto AllowedSeedStrength(
        const opentxs::crypto::SeedStyle type) const noexcept -> const
        UnallocatedMap<opentxs::crypto::SeedStrength, UnallocatedCString>& = 0;
    virtual auto Bip32Root(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const -> UnallocatedCString = 0;
    virtual auto DefaultSeed() const
        -> std::pair<UnallocatedCString, std::size_t> = 0;
    virtual auto GetHDKey(
        const UnallocatedCString& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetHDKey(
        const UnallocatedCString& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const opentxs::crypto::key::asymmetric::Role,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetHDKey(
        const UnallocatedCString& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetHDKey(
        const UnallocatedCString& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const opentxs::crypto::key::asymmetric::Role,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetPaymentCode(
        const UnallocatedCString& seedID,
        const Bip32Index nym,
        const std::uint8_t version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> = 0;
    virtual auto GetStorageKey(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const -> OTSymmetricKey = 0;
    virtual auto GetSeed(
        const UnallocatedCString& seedID,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> Secret = 0;
    virtual auto GetSeed(
        const identifier::Generic& id,
        const PasswordPrompt& reason) const noexcept
        -> opentxs::crypto::Seed = 0;
    virtual auto ImportRaw(const Secret& entropy, const PasswordPrompt& reason)
        const -> UnallocatedCString = 0;
    virtual auto ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const PasswordPrompt& reason,
        const std::string_view comment = {}) const -> UnallocatedCString = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Seed& = 0;
    virtual auto LongestWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept -> std::size_t = 0;
    virtual auto NewSeed(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason,
        const std::string_view comment = {}) const -> UnallocatedCString = 0;
    virtual auto Passphrase(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const -> UnallocatedCString = 0;
    virtual auto SeedDescription(UnallocatedCString seedID) const noexcept
        -> UnallocatedCString = 0;
    virtual auto SetDefault(const identifier::Generic& id) const noexcept
        -> bool = 0;
    virtual auto SetSeedComment(
        const identifier::Generic& id,
        const std::string_view comment) const noexcept -> bool = 0;
    virtual auto ValidateWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const std::string_view word) const noexcept
        -> UnallocatedVector<UnallocatedCString> = 0;
    virtual auto WordCount(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::SeedStrength strength) const noexcept
        -> std::size_t = 0;
    virtual auto Words(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const -> UnallocatedCString = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Seed& = 0;

    Seed(const Seed&) = delete;
    Seed(Seed&&) = delete;
    auto operator=(const Seed&) -> Seed& = delete;
    auto operator=(Seed&&) -> Seed& = delete;

    OPENTXS_NO_EXPORT virtual ~Seed() = default;

protected:
    Seed() = default;
};
}  // namespace opentxs::api::crypto
