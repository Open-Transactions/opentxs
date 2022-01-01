// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/key/asymmetric/Role.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "Proto.hpp"
#include "crypto/Seed.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStrength.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/Enums.pb.h"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Seed;
class Symmetric;
}  // namespace crypto

namespace session
{
class Factory;
class Storage;
}  // namespace session

class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class HD;
class Secp256k1;
}  // namespace key

class Bip32;
class Bip39;
}  // namespace crypto

namespace proto
{
class HDPath;
class Seed;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::crypto::imp
{
class Seed final : public internal::Seed
{
public:
    auto AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto AccountChildKey(
        const ReadView& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto AccountKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto AllowedSeedTypes() const noexcept
        -> const std::pmr::map<opentxs::crypto::SeedStyle, std::string>& final;
    auto AllowedLanguages(const opentxs::crypto::SeedStyle type) const noexcept
        -> const std::pmr::map<opentxs::crypto::Language, std::string>& final;
    auto AllowedSeedStrength(
        const opentxs::crypto::SeedStyle type) const noexcept -> const
        std::pmr::map<opentxs::crypto::SeedStrength, std::string>& final;
    auto Bip32Root(const std::string& seedID, const PasswordPrompt& reason)
        const -> std::string final;
    auto DefaultSeed() const -> std::string final;
    auto GetHDKey(
        const std::string& seedID,
        const EcdsaCurve& curve,
        const std::pmr::vector<Bip32Index>& path,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto GetHDKey(
        const std::string& seedID,
        const EcdsaCurve& curve,
        const std::pmr::vector<Bip32Index>& path,
        const opentxs::crypto::key::asymmetric::Role role,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto GetHDKey(
        const std::string& seedID,
        const EcdsaCurve& curve,
        const std::pmr::vector<Bip32Index>& path,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto GetHDKey(
        const std::string& seedID,
        const EcdsaCurve& curve,
        const std::pmr::vector<Bip32Index>& path,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto GetOrCreateDefaultSeed(
        std::string& seedID,
        opentxs::crypto::SeedStyle& type,
        opentxs::crypto::Language& lang,
        Bip32Index& index,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const -> OTSecret final;
    auto GetPaymentCode(
        const std::string& seedID,
        const Bip32Index nym,
        const std::uint8_t version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto GetSeed(
        const std::string& seedID,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> OTSecret final;
    auto GetStorageKey(const std::string& seedID, const PasswordPrompt& reason)
        const -> OTSymmetricKey final;
    auto ImportRaw(const Secret& entropy, const PasswordPrompt& reason) const
        -> std::string final;
    auto ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const PasswordPrompt& reason) const -> std::string final;
    auto LongestWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> std::size_t final;
    auto NewSeed(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const -> std::string final;
    auto Passphrase(const std::string& seedID, const PasswordPrompt& reason)
        const -> std::string final;
    auto SeedDescription(std::string seedID) const noexcept
        -> std::string final;
    auto UpdateIndex(
        const std::string& seedID,
        const Bip32Index index,
        const PasswordPrompt& reason) const -> bool final;
    auto ValidateWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const std::string_view word) const noexcept
        -> std::pmr::vector<std::string> final;
    auto WordCount(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::SeedStrength strength) const noexcept
        -> std::size_t final;
    auto Words(const std::string& seedID, const PasswordPrompt& reason) const
        -> std::string final;

    Seed(
        const api::Session& api,
        const api::session::Factory& factory,
        const api::crypto::Asymmetric& asymmetric,
        const api::crypto::Symmetric& symmetric,
        const api::session::Storage& storage,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39);

    ~Seed() final = default;

private:
    using SeedMap = std::pmr::map<std::string, opentxs::crypto::Seed>;

    const api::Session& api_;  // WARNING do not access during construction
    const api::session::Factory& factory_;
    const api::crypto::Symmetric& symmetric_;
    const api::crypto::Asymmetric& asymmetric_;
    const api::session::Storage& storage_;
    const opentxs::crypto::Bip32& bip32_;
    const opentxs::crypto::Bip39& bip39_;
    mutable std::mutex seed_lock_;
    mutable SeedMap seeds_;

    auto get_seed(
        const Lock& lock,
        const std::string& seedID,
        const PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::Seed&;
    auto new_seed(
        const Lock& lock,
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const noexcept -> std::string;

    Seed() = delete;
    Seed(const Seed&) = delete;
    Seed(Seed&&) = delete;
    auto operator=(const Seed&) -> Seed& = delete;
    auto operator=(Seed&&) -> Seed& = delete;
};
}  // namespace opentxs::api::crypto::imp
