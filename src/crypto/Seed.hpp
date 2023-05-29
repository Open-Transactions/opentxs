// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Ciphertext.pb.h>
#include <cs_plain_guarded.h>

#include "internal/crypto/Seed.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Seed.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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

class Session;
}  // namespace api

namespace crypto
{
class Bip32;
class Bip39;
}  // namespace crypto

namespace proto
{
class Seed;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::crypto::Seed::Imp final : public internal::Seed
{
public:
    const SeedStyle type_;
    const Language lang_;
    const Secret words_;
    const Secret phrase_;
    const Secret entropy_;
    const identifier::Generic id_;
    const api::session::Storage* const storage_;
    const proto::Ciphertext encrypted_words_;
    const proto::Ciphertext encrypted_phrase_;
    const proto::Ciphertext encrypted_entropy_;

    auto Index() const noexcept -> Bip32Index;

    auto IncrementIndex(const Bip32Index index) noexcept -> bool final;

    Imp(const api::Session& api) noexcept;
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
    Imp(const api::Session& api,
        const opentxs::crypto::Bip32& bip32,
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
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

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

    const api::Session& api_;
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
};
