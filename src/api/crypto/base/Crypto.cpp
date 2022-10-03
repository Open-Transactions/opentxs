// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "api/crypto/base/Crypto.hpp"  // IWYU pragma: associated

#include "2_Factory.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/crypto/Crypto.hpp"
#include "internal/crypto/Factory.hpp"
#include "internal/crypto/library/Factory.hpp"
#include "internal/crypto/library/Null.hpp"
#include "internal/crypto/library/OpenSSL.hpp"
#include "internal/crypto/library/Secp256k1.hpp"
#include "internal/crypto/library/Sodium.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/crypto/key/symmetric/Source.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto CryptoAPI(const api::Settings& settings) noexcept
    -> std::unique_ptr<api::Crypto>
{
    using ReturnType = api::imp::Crypto;

    return std::make_unique<ReturnType>(settings);
}
}  // namespace opentxs::factory

namespace opentxs::api::imp
{
const opentxs::crypto::blank::EcdsaProvider Crypto::blank_{};

Crypto::Crypto(const api::Settings& settings) noexcept
    : config_(factory::CryptoConfig(settings))
    , sodium_(factory::Sodium(*this))
    , ssl_(factory::OpenSSL())
    , util_(*sodium_)
    , secp256k1_(factory::Secp256k1(*this, util_))
    , bip39_p_(opentxs::Factory::Bip39(*this))
    , bip32_(factory::Bip32(*this))
    , bip39_(*bip39_p_)
    , encode_(factory::Encode(*this))
    , hash_(factory::Hash(*encode_, *ssl_, *sodium_, *ssl_, *ssl_, *sodium_))
    , asymmetric_map_([this] {
        auto out = AMap{};
        out.emplace(AType::ED25519, sodium_.get());

        if (ssl_) { out.emplace(AType::Legacy, ssl_.get()); }

        if (secp256k1_) { out.emplace(AType::Secp256k1, secp256k1_.get()); }

        return out;
    }())
    , elliptic_map_([this] {
        auto out = EMap{};
        out.emplace(AType::ED25519, sodium_.get());

        if (secp256k1_) { out.emplace(AType::Secp256k1, secp256k1_.get()); }

        return out;
    }())
    , symmetric_algorithm_map_([this] {
        auto out = SaMap{};
        out.emplace(SaType::ChaCha20Poly1305, sodium_.get());

        return out;
    }())
    , symmetric_source_map_([this] {
        auto out = SsMap{};
        out.emplace(SsType::Argon2i, sodium_.get());
        out.emplace(SsType::Argon2id, sodium_.get());

        return out;
    }())
{
    Init();
}

auto Crypto::AsymmetricProvider(
    opentxs::crypto::key::asymmetric::Algorithm type) const noexcept
    -> const opentxs::crypto::AsymmetricProvider&
{
    try {

        return *asymmetric_map_.at(type);
    } catch (...) {

        return blank_;
    }
}

auto Crypto::BIP32() const noexcept -> const opentxs::crypto::Bip32&
{
    return bip32_;
}

auto Crypto::BIP39() const noexcept -> const opentxs::crypto::Bip39&
{
    return bip39_;
}

auto Crypto::Cleanup() noexcept -> void
{
    hash_.reset();
    encode_.reset();
    secp256k1_.reset();
    sodium_.reset();
    config_.reset();
}

auto Crypto::Config() const noexcept -> const crypto::Config&
{
    OT_ASSERT(config_);

    return *config_;
}

auto Crypto::EllipticProvider(opentxs::crypto::key::asymmetric::Algorithm type)
    const noexcept -> const opentxs::crypto::EcdsaProvider&
{
    try {

        return *elliptic_map_.at(type);
    } catch (...) {

        return blank_;
    }
}

auto Crypto::Encode() const noexcept -> const crypto::Encode&
{
    OT_ASSERT(encode_);

    return *encode_;
}

auto Crypto::Hash() const noexcept -> const crypto::Hash&
{
    OT_ASSERT(hash_);

    return *hash_;
}

auto Crypto::hasLibsecp256k1() const noexcept -> bool
{
    return secp256k1_.operator bool();
}

auto Crypto::hasOpenSSL() const noexcept -> bool
{
    return ssl_.operator bool();
}

auto Crypto::hasSodium() const noexcept -> bool
{
    return sodium_.operator bool();
}

auto Crypto::Init() noexcept -> void
{
    LogDetail()(OT_PRETTY_CLASS())("Setting up crypto libraries...").Flush();
    Init_Sodium();
    Init_OpenSSL();
    Init_Libsecp256k1();
}

auto Crypto::Init(const std::shared_ptr<const api::Factory>& factory) noexcept
    -> void
{
    bip32_.Internal().Init(factory);
}

auto Crypto::Init_Sodium() noexcept -> void { OT_ASSERT(sodium_); }

auto Crypto::Libsodium() const noexcept -> const opentxs::crypto::Sodium&
{
    return *sodium_;
}

auto Crypto::SymmetricProvider(opentxs::crypto::key::symmetric::Algorithm type)
    const noexcept -> const opentxs::crypto::SymmetricProvider&
{
    try {

        return *symmetric_algorithm_map_.at(type);
    } catch (...) {
        static const auto blank = opentxs::crypto::blank::SymmetricProvider{};

        return blank;
    }
}

auto Crypto::SymmetricProvider(opentxs::crypto::key::symmetric::Source type)
    const noexcept -> const opentxs::crypto::SymmetricProvider&
{
    try {

        return *symmetric_source_map_.at(type);
    } catch (...) {
        static const auto blank = opentxs::crypto::blank::SymmetricProvider{};

        return blank;
    }
}

auto Crypto::Util() const noexcept -> const crypto::Util& { return util_; }

Crypto::~Crypto() { Cleanup(); }
}  // namespace opentxs::api::imp
