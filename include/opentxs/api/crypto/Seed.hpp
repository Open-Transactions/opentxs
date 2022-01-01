// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/key/asymmetric/Role.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/Types.hpp"

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
}  // namespace crypto

class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace opentxs::api::crypto
{
class OPENTXS_EXPORT Seed
{
public:
    virtual auto AccountChildKey(
        const ReadView& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto AllowedSeedTypes() const noexcept
        -> const std::pmr::map<opentxs::crypto::SeedStyle, std::string>& = 0;
    virtual auto AllowedLanguages(
        const opentxs::crypto::SeedStyle type) const noexcept
        -> const std::pmr::map<opentxs::crypto::Language, std::string>& = 0;
    virtual auto AllowedSeedStrength(
        const opentxs::crypto::SeedStyle type) const noexcept
        -> const std::pmr::map<opentxs::crypto::SeedStrength, std::string>& = 0;
    virtual auto Bip32Root(
        const std::string& seedID,
        const PasswordPrompt& reason) const -> std::string = 0;
    virtual auto DefaultSeed() const -> std::string = 0;
    virtual auto GetHDKey(
        const std::string& seedID,
        const EcdsaCurve& curve,
        const std::pmr::vector<Bip32Index>& path,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetHDKey(
        const std::string& seedID,
        const EcdsaCurve& curve,
        const std::pmr::vector<Bip32Index>& path,
        const opentxs::crypto::key::asymmetric::Role,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetHDKey(
        const std::string& seedID,
        const EcdsaCurve& curve,
        const std::pmr::vector<Bip32Index>& path,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetHDKey(
        const std::string& seedID,
        const EcdsaCurve& curve,
        const std::pmr::vector<Bip32Index>& path,
        const opentxs::crypto::key::asymmetric::Role,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetOrCreateDefaultSeed(
        std::string& seedID,
        opentxs::crypto::SeedStyle& type,
        opentxs::crypto::Language& lang,
        Bip32Index& index,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const -> OTSecret = 0;
    virtual auto GetPaymentCode(
        const std::string& seedID,
        const Bip32Index nym,
        const std::uint8_t version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> = 0;
    virtual auto GetStorageKey(
        const std::string& seedID,
        const PasswordPrompt& reason) const -> OTSymmetricKey = 0;
    virtual auto GetSeed(
        const std::string& seedID,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> OTSecret = 0;
    virtual auto ImportRaw(const Secret& entropy, const PasswordPrompt& reason)
        const -> std::string = 0;
    virtual auto ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const PasswordPrompt& reason) const -> std::string = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Seed& = 0;
    virtual auto LongestWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept -> std::size_t = 0;
    virtual auto NewSeed(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const -> std::string = 0;
    virtual auto Passphrase(
        const std::string& seedID,
        const PasswordPrompt& reason) const -> std::string = 0;
    virtual auto SeedDescription(std::string seedID) const noexcept
        -> std::string = 0;
    virtual auto UpdateIndex(
        const std::string& seedID,
        const Bip32Index index,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto ValidateWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const std::string_view word) const noexcept
        -> std::pmr::vector<std::string> = 0;
    virtual auto WordCount(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::SeedStrength strength) const noexcept
        -> std::size_t = 0;
    virtual auto Words(const std::string& seedID, const PasswordPrompt& reason)
        const -> std::string = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Seed& = 0;

    OPENTXS_NO_EXPORT virtual ~Seed() = default;

protected:
    Seed() = default;

private:
    Seed(const Seed&) = delete;
    Seed(Seed&&) = delete;
    auto operator=(const Seed&) -> Seed& = delete;
    auto operator=(Seed&&) -> Seed& = delete;
};
}  // namespace opentxs::api::crypto
