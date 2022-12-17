// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/HD.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
// IWYU pragma: no_include "opentxs/crypto/symmetric/Key.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string_view>
#include <utility>

#include "internal/api/crypto/Seed.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Seed.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Symmetric;
}  // namespace crypto

namespace session
{
class Endpoints;
class Factory;
class Storage;
}  // namespace session

class Session;
}  // namespace api

namespace crypto
{
class Bip32;
class Bip39;
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class HDPath;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
        -> opentxs::crypto::asymmetric::key::HD final;
    auto AccountChildKey(
        const ReadView& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto AccountKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto AllowedSeedTypes() const noexcept -> const
        UnallocatedMap<opentxs::crypto::SeedStyle, UnallocatedCString>& final;
    auto AllowedLanguages(const opentxs::crypto::SeedStyle type) const noexcept
        -> const
        UnallocatedMap<opentxs::crypto::Language, UnallocatedCString>& final;
    auto AllowedSeedStrength(const opentxs::crypto::SeedStyle type)
        const noexcept -> const UnallocatedMap<
            opentxs::crypto::SeedStrength,
            UnallocatedCString>& final;
    auto Bip32Root(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const -> UnallocatedCString final;
    auto DefaultSeed() const
        -> std::pair<UnallocatedCString, std::size_t> final;
    auto GetHDKey(
        const UnallocatedCString& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto GetHDKey(
        const UnallocatedCString& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const opentxs::crypto::asymmetric::Role role,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto GetHDKey(
        const UnallocatedCString& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto GetHDKey(
        const UnallocatedCString& seedID,
        const opentxs::crypto::EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const opentxs::crypto::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::asymmetric::key::HD final;
    auto GetOrCreateDefaultSeed(
        UnallocatedCString& seedID,
        opentxs::crypto::SeedStyle& type,
        opentxs::crypto::Language& lang,
        Bip32Index& index,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const -> Secret final;
    auto GetPaymentCode(
        const UnallocatedCString& seedID,
        const Bip32Index nym,
        const std::uint8_t version,
        const PasswordPrompt& reason,
        alloc::Default alloc) const
        -> opentxs::crypto::asymmetric::key::Secp256k1 final;
    auto GetSeed(
        const UnallocatedCString& seedID,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> Secret final;
    auto GetSeed(const identifier::Generic& id, const PasswordPrompt& reason)
        const noexcept -> opentxs::crypto::Seed final;
    auto GetStorageKey(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const
        -> opentxs::crypto::symmetric::Key final;
    auto ImportRaw(const Secret& entropy, const PasswordPrompt& reason) const
        -> UnallocatedCString final;
    auto ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const PasswordPrompt& reason,
        const std::string_view comment) const -> UnallocatedCString final;
    auto LongestWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> std::size_t final;
    auto NewSeed(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason,
        const std::string_view comment) const -> UnallocatedCString final;
    auto Passphrase(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const -> UnallocatedCString final;
    auto SeedDescription(UnallocatedCString seedID) const noexcept
        -> UnallocatedCString final;
    auto SetDefault(const identifier::Generic& id) const noexcept -> bool final;
    auto SetSeedComment(
        const identifier::Generic& id,
        const std::string_view comment) const noexcept -> bool final;
    auto UpdateIndex(
        const UnallocatedCString& seedID,
        const Bip32Index index,
        const PasswordPrompt& reason) const -> bool final;
    auto ValidateWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const std::string_view word) const noexcept
        -> UnallocatedVector<UnallocatedCString> final;
    auto WordCount(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::SeedStrength strength) const noexcept
        -> std::size_t final;
    auto Words(const UnallocatedCString& seedID, const PasswordPrompt& reason)
        const -> UnallocatedCString final;

    Seed(
        const api::Session& api,
        const api::session::Endpoints& endpoints,
        const api::session::Factory& factory,
        const api::crypto::Asymmetric& asymmetric,
        const api::crypto::Symmetric& symmetric,
        const api::session::Storage& storage,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const opentxs::network::zeromq::Context& zmq);
    Seed() = delete;
    Seed(const Seed&) = delete;
    Seed(Seed&&) = delete;
    auto operator=(const Seed&) -> Seed& = delete;
    auto operator=(Seed&&) -> Seed& = delete;

    ~Seed() final;

private:
    using SeedMap = UnallocatedMap<UnallocatedCString, opentxs::crypto::Seed>;

    const api::Session& api_;  // WARNING do not access during construction
    const api::session::Factory& factory_;
    const api::crypto::Symmetric& symmetric_;
    const api::crypto::Asymmetric& asymmetric_;
    const api::session::Storage& storage_;
    const opentxs::crypto::Bip32& bip32_;
    const opentxs::crypto::Bip39& bip39_;
    const OTZMQPublishSocket socket_;
    mutable std::mutex seed_lock_;
    mutable SeedMap seeds_;

    auto get_seed(
        const Lock& lock,
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::Seed&;
    auto new_seed(
        const Lock& lock,
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const opentxs::crypto::SeedStrength strength,
        const std::string_view comment,
        const PasswordPrompt& reason) const noexcept -> UnallocatedCString;
    auto new_seed(
        const Lock& lock,
        const std::string_view comment,
        opentxs::crypto::Seed&& seed) const noexcept -> UnallocatedCString;
    auto publish(const identifier::Generic& id) const noexcept -> void;
    auto publish(const UnallocatedCString& id) const noexcept -> void;
};
}  // namespace opentxs::api::crypto::imp
