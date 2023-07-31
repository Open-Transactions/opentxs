// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

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
namespace asymmetric
{
namespace key
{
class HD;
class Secp256k1;
}  // namespace key
}  // namespace asymmetric

namespace symmetric
{
class Key;
}  // namespace symmetric

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
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto AllowedLanguages(
        const opentxs::crypto::SeedStyle type) const noexcept -> const
        UnallocatedMap<opentxs::crypto::Language, std::string_view>& = 0;
    virtual auto AllowedSeedStrength(
        const opentxs::crypto::SeedStyle type) const noexcept -> const
        UnallocatedMap<opentxs::crypto::SeedStrength, std::string_view>& = 0;
    virtual auto AllowedSeedTypes() const noexcept -> const
        UnallocatedMap<opentxs::crypto::SeedStyle, std::string_view>& = 0;
    virtual auto Bip32Root(
        const opentxs::crypto::SeedID& seedID,
        const PasswordPrompt& reason) const -> UnallocatedCString = 0;
    virtual auto DefaultSeed() const
        -> std::pair<opentxs::crypto::SeedID, std::size_t> = 0;
    virtual auto GetHDKey(
        const opentxs::crypto::SeedID& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto GetHDKey(
        const opentxs::crypto::SeedID& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const opentxs::crypto::asymmetric::Role,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto GetHDKey(
        const opentxs::crypto::SeedID& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto GetHDKey(
        const opentxs::crypto::SeedID& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const opentxs::crypto::asymmetric::Role,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD = 0;
    virtual auto GetPaymentCode(
        const opentxs::crypto::SeedID& seedID,
        const Bip32Index nym,
        const std::uint8_t version,
        const PasswordPrompt& reason,
        alloc::Default alloc = {}) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 = 0;
    virtual auto GetStorageKey(
        const opentxs::crypto::SeedID& seedID,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::symmetric::Key = 0;
    virtual auto GetSeed(
        const opentxs::crypto::SeedID& seedID,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> Secret = 0;
    virtual auto GetSeed(
        const opentxs::crypto::SeedID& id,
        const PasswordPrompt& reason) const noexcept
        -> opentxs::crypto::Seed = 0;
    virtual auto ImportRaw(
        const Secret& entropy,
        const PasswordPrompt& reason,
        std::string_view comment = {},
        Time created = {}) const -> opentxs::crypto::SeedID = 0;
    virtual auto ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const PasswordPrompt& reason,
        std::string_view comment = {},
        Time created = {}) const -> opentxs::crypto::SeedID = 0;
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
        const std::string_view comment = {}) const
        -> opentxs::crypto::SeedID = 0;
    virtual auto Passphrase(
        const opentxs::crypto::SeedID& seedID,
        const PasswordPrompt& reason) const -> UnallocatedCString = 0;
    virtual auto SeedDescription(const opentxs::crypto::SeedID& seedID)
        const noexcept -> UnallocatedCString = 0;
    virtual auto SetDefault(const opentxs::crypto::SeedID& id) const noexcept
        -> bool = 0;
    virtual auto SetSeedComment(
        const opentxs::crypto::SeedID& id,
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
        const opentxs::crypto::SeedID& seedID,
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
