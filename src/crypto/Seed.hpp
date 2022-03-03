// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <memory>
#include <mutex>

#include "internal/crypto/Seed.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/Seed.hpp"
#include "opentxs/crypto/SeedStrength.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/Ciphertext.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Symmetric;
}  // namespace crypto

namespace session
{
class Factory;
class Storage;
}  // namespace session

class Factory;
class Session;
}  // namespace api

namespace crypto
{
class Bip32;
class Bip39;
}  // namespace crypto

namespace proto
{
class Ciphertext;
class Seed;
}  // namespace proto

class Identifier;
class PasswordPrompt;
class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::crypto::Seed::Imp final : public internal::Seed
{
public:
    const SeedStyle type_;
    const Language lang_;
    const OTSecret words_;
    const OTSecret phrase_;
    const OTSecret entropy_;
    const OTIdentifier id_;
    const api::session::Storage* const storage_;
    const proto::Ciphertext encrypted_words_;
    const proto::Ciphertext encrypted_phrase_;
    const proto::Ciphertext encrypted_entropy_;

    auto Index() const noexcept -> Bip32Index;

    auto IncrementIndex(const Bip32Index index) noexcept -> bool final;

    Imp() noexcept;
    Imp(const api::Factory& factory) noexcept;
    Imp(const api::Session& api,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const Language lang,
        const SeedStrength strength,
        const PasswordPrompt& reason) noexcept(false);
    Imp(const api::Session& api,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const SeedStyle type,
        const Language lang,
        const Secret& words,
        const Secret& passphrase,
        const PasswordPrompt& reason) noexcept(false);
    Imp(const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const Secret& entropy,
        const PasswordPrompt& reason) noexcept(false);
    Imp(const api::Session& api,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const proto::Seed& proto,
        const PasswordPrompt& reason) noexcept(false);
    Imp(const Imp& rhs) noexcept;

    ~Imp() final = default;

private:
    static constexpr auto default_version_ = VersionNumber{4u};
    static constexpr auto no_passphrase_{""};

    struct MutableData {
        VersionNumber version_;
        Bip32Index index_;

        MutableData(VersionNumber version, Bip32Index index) noexcept
            : version_(version)
            , index_(index)
        {
        }
        MutableData() noexcept
            : MutableData(default_version_, 0)
        {
        }
        MutableData(const MutableData& rhs) noexcept
            : MutableData(rhs.version_, rhs.index_)
        {
        }
    };
    using Guarded = libguarded::plain_guarded<MutableData>;
    using SerializeType = proto::Seed;

    // TODO switch to shared_guarded after
    // https://github.com/copperspice/cs_libguarded/pull/18 is merged upstream
    mutable Guarded data_;

    static auto encrypt(
        const SeedStyle type,
        const api::crypto::Symmetric& symmetric,
        const Secret& entropy,
        const Secret& words,
        const Secret& phrase,
        proto::Ciphertext& cwords,
        proto::Ciphertext& cphrase,
        const PasswordPrompt& reason) noexcept(false) -> proto::Ciphertext;

    auto save() const noexcept -> bool;
    auto save(const MutableData& data) const noexcept -> bool;

    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
