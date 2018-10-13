// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Crypto.hpp"

#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#if OT_CRYPTO_USING_TREZOR || OT_CRYPTO_USING_LIBBITCOIN
#include "opentxs/core/crypto/OTCachedKey.hpp"
#endif
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#if OT_CRYPTO_USING_LIBBITCOIN
#include "opentxs/crypto/library/Bitcoin.hpp"
#endif
#if OT_CRYPTO_USING_OPENSSL
#include "opentxs/crypto/library/OpenSSL.hpp"
#endif
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/Secp256k1.hpp"
#endif
#include "opentxs/crypto/library/Ripemd160.hpp"
#include "opentxs/crypto/library/Sodium.hpp"
#if OT_CRYPTO_USING_TREZOR
#include "opentxs/crypto/library/Trezor.hpp"
#endif
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/Bip32.hpp"
#endif
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/crypto/Bip39.hpp"
#endif

#include <functional>
#include <ostream>
#include <vector>

extern "C" {
#ifdef _WIN32
#else
#include <sys/resource.h>
#endif
}

#define OT_METHOD "opentxs::Crypto::"

template class opentxs::Pimpl<opentxs::crypto::key::Symmetric>;

namespace opentxs
{
api::Crypto* Factory::Crypto(const api::Settings& settings)
{
    return new api::implementation::Crypto(settings);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Crypto::Crypto(const api::Settings& settings)
    : cached_key_lock_()
    , primary_key_(nullptr)
    , cached_keys_()
    , config_(opentxs::Factory::CryptoConfig(settings))
#if OT_CRYPTO_USING_LIBBITCOIN
    , bitcoin_(opentxs::Factory::Bitcoin(*this))
#endif
#if OT_CRYPTO_USING_TREZOR
    , trezor_(opentxs::Factory::Trezor(*this))
#endif
    , sodium_(opentxs::Factory::Sodium(*this))
#if OT_CRYPTO_USING_OPENSSL
    , ssl_(opentxs::Factory::OpenSSL(*this))
#endif
    , util_(*sodium_)
#if OT_CRYPTO_USING_LIBBITCOIN
    , secp256k1_helper_(*bitcoin_)
    , base58_(*bitcoin_)
    , ripemd160_(*bitcoin_)
    , bip32_(*bitcoin_)
    , bip39_(*bitcoin_)
#elif OT_CRYPTO_USING_TREZOR
    , secp256k1_helper_(*trezor_)
    , base58_(*trezor_)
    , ripemd160_(*trezor_)
    , bip32_(*trezor_)
    , bip39_(*trezor_)
#endif
#if OT_CRYPTO_USING_LIBSECP256K1
    , secp256k1_(opentxs::Factory::Secp256k1(*this, util_, secp256k1_helper_))
    , secp256k1_provider_(*secp256k1_)
#elif OT_CRYPTO_USING_LIBBITCOIN
    , secp256k1_provider_(*bitcoin_)
#elif OT_CRYPTO_USING_TREZOR
    , secp256k1_provider_(*trezor_)
#endif
    , encode_(opentxs::Factory::Encode(base58_))
    , hash_(opentxs::Factory::Hash(
          *encode_,
          *ssl_,
          *sodium_
#if OT_CRYPTO_USING_TREZOR || OT_CRYPTO_USING_LIBBITCOIN
          ,
          ripemd160_
#endif  // OT_CRYPTO_USING_TREZOR
          ))
    , symmetric_(opentxs::Factory::Symmetric(*sodium_))
{
#if OT_CRYPTO_USING_LIBBITCOIN
    OT_ASSERT(bitcoin_)
#endif
#if OT_CRYPTO_USING_TREZOR
    OT_ASSERT(trezor_)
#endif
    OT_ASSERT(sodium_)
#if OT_CRYPTO_USING_OPENSSL
    OT_ASSERT(ssl_)
#endif
#if OT_CRYPTO_USING_LIBSECP256K1
    OT_ASSERT(secp256k1_)
#endif

    Init();
}

#if OT_CRYPTO_SUPPORTED_ALGO_AES
const opentxs::crypto::LegacySymmetricProvider& Crypto::AES() const
{
    OT_ASSERT(nullptr != ssl_);

    return *ssl_;
}
#endif

#if OT_CRYPTO_WITH_BIP32
const opentxs::crypto::Bip32& Crypto::BIP32() const { return bip32_; }
#endif

#if OT_CRYPTO_WITH_BIP39
const opentxs::crypto::Bip39& Crypto::BIP39() const { return bip39_; }
#endif

const OTCachedKey& Crypto::CachedKey(const Identifier& id) const
{
    Lock lock(cached_key_lock_);

    auto& output = cached_keys_[id];

    if (false == bool(output)) {
        OT_FAIL_MSG("This function is broken, this never should have "
                    "happened.");

        output.reset(new OTCachedKey(*this, OT_MASTER_KEY_TIMEOUT));
    }

    OT_ASSERT(output);

    return *output;
}

const OTCachedKey& Crypto::CachedKey(const OTCachedKey& source) const
{
    Lock lock(cached_key_lock_);
    const auto id = Identifier::Factory(source);
    auto& output = cached_keys_[id];

    if (false == bool(output)) {
        auto serialized = Armored::Factory();
        const bool haveSerialized = source.SerializeTo(serialized);

        OT_ASSERT(haveSerialized);

        output.reset(new OTCachedKey(*this, serialized));
    }

    OT_ASSERT(output);

    return *output;
}

void Crypto::Cleanup()
{
    primary_key_.reset();
    cached_keys_.clear();
    symmetric_.reset();
    hash_.reset();
    encode_.reset();
#if OT_CRYPTO_USING_LIBSECP256K1
    secp256k1_.reset();
#endif
#if OT_CRYPTO_USING_OPENSSL
    ssl_->Cleanup();
#endif
    sodium_.reset();
#if OT_CRYPTO_USING_TREZOR
    trezor_.reset();
#endif
#if OT_CRYPTO_USING_LIBBITCOIN
    bitcoin_.reset();
#endif
    config_.reset();
}

const crypto::Config& Crypto::Config() const
{
    OT_ASSERT(config_);

    return *config_;
}

const OTCachedKey& Crypto::DefaultKey() const
{
    Lock lock(cached_key_lock_);

    init_default_key(lock);

    OT_ASSERT(primary_key_);

    return *primary_key_;
}

Editor<OTCachedKey> Crypto::mutable_DefaultKey() const
{
    OT_ASSERT(primary_key_);

    std::function<void(OTCachedKey*, Lock&)> callback = [&](OTCachedKey*,
                                                            Lock&) -> void {};

    return Editor<OTCachedKey>(cached_key_lock_, primary_key_.get(), callback);
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
const opentxs::crypto::AsymmetricProvider& Crypto::ED25519() const
{
    OT_ASSERT(sodium_);

    return *sodium_;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

const crypto::Encode& Crypto::Encode() const
{
    OT_ASSERT(encode_);

    return *encode_;
}

const crypto::Hash& Crypto::Hash() const
{
    OT_ASSERT(hash_);

    return *hash_;
}

void Crypto::Init()
{
    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Setting up rlimits, and crypto libraries...")
        .Flush();

// Here is a security measure intended to make it more difficult to
// capture a core dump. (Not used in debug mode, obviously.)
//
#if !defined(PREDEF_MODE_DEBUG) && defined(PREDEF_PLATFORM_UNIX)
    struct rlimit rlim;
    getrlimit(RLIMIT_CORE, &rlim);
    rlim.rlim_max = rlim.rlim_cur = 0;
    if (setrlimit(RLIMIT_CORE, &rlim)) {
        OT_FAIL_MSG("Crypto::Init: ASSERT: setrlimit failed. (Used for "
                    "preventing core dumps.)\n");
    }
#endif

#if OT_CRYPTO_USING_OPENSSL
    ssl_->Init();
#endif
#if OT_CRYPTO_USING_LIBSECP256K1
    // WARNING: The below call to secp256k1_->Init() DEPENDS on the fact
    // that the above call to ssl_->Init() happened FIRST.
    secp256k1_->Init();
#endif
}

void Crypto::init_default_key(const Lock&) const
{
    if (false == bool(primary_key_)) {
        primary_key_.reset(new OTCachedKey(*this, OT_MASTER_KEY_TIMEOUT));
    }
}

const OTCachedKey& Crypto::LoadDefaultKey(const Armored& serialized) const
{
    Lock lock(cached_key_lock_);

    init_default_key(lock);

    OT_ASSERT(primary_key_);

    primary_key_->SetCachedKey(serialized);

    return *primary_key_;
}

#if OT_CRYPTO_SUPPORTED_KEY_RSA
const opentxs::crypto::AsymmetricProvider& Crypto::RSA() const
{
    OT_ASSERT(nullptr != ssl_);

    return *ssl_;
}
#endif

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
const opentxs::crypto::AsymmetricProvider& Crypto::SECP256K1() const
{
    return secp256k1_provider_;
}
#endif

void Crypto::SetTimeout(const std::chrono::seconds& timeout) const
{
    Lock lock(cached_key_lock_);

    init_default_key(lock);

    OT_ASSERT(primary_key_);

    primary_key_->SetTimeoutSeconds(timeout.count());
}

void Crypto::SetSystemKeyring(const bool useKeyring) const
{
    Lock lock(cached_key_lock_);

    init_default_key(lock);

    OT_ASSERT(primary_key_);

    primary_key_->UseSystemKeyring(useKeyring);
}

const crypto::Symmetric& Crypto::Symmetric() const
{
    OT_ASSERT(symmetric_);

    return *symmetric_;
}

const crypto::Util& Crypto::Util() const { return util_; }

OTSymmetricKey Crypto::GetStorageKey(const proto::AsymmetricKey& raw) const
{
    OTPassword keySource{};
    auto sessionKey =
        Symmetric().Key(raw.encryptedkey().key(), raw.encryptedkey().mode());
    OTPasswordData blank(__FUNCTION__);
    const bool decrypted =
        sessionKey->Decrypt(raw.encryptedkey(), blank, keySource);

    if (false == decrypted) {
        return opentxs::crypto::key::Symmetric::Factory();
    }

    return Symmetric().Key(keySource);
}

Crypto::~Crypto() { Cleanup(); }
}  // namespace opentxs::api::implementation
