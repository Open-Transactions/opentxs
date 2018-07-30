// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Crypto.hpp"

#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#if OT_CRYPTO_USING_TREZOR
#include "opentxs/core/crypto/OTCachedKey.hpp"
#endif
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#if OT_CRYPTO_USING_OPENSSL
#include "opentxs/crypto/library/OpenSSL.hpp"
#endif
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/Secp256k1.hpp"
#endif
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
api::Crypto* Factory::Crypto(const api::Native& native)
{
    return new api::implementation::Crypto(native);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Crypto::Crypto(const api::Native& native)
    : native_(native)
    , cached_key_lock_()
    , primary_key_(nullptr)
    , cached_keys_()
#if OT_CRYPTO_USING_TREZOR
    , bitcoincrypto_(Factory::Trezor(native_))
#endif
    , ed25519_(Factory::Sodium())
#if OT_CRYPTO_USING_OPENSSL
    , ssl_(Factory::OpenSSL())
#endif
    , util_(*ed25519_)
#if OT_CRYPTO_USING_LIBSECP256K1
    , secp256k1_(Factory::Secp256k1(util_, *bitcoincrypto_))
#endif
{
#if OT_CRYPTO_USING_TREZOR
    OT_ASSERT(bitcoincrypto_)
#endif
    OT_ASSERT(ed25519_)
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
const opentxs::crypto::Bip32<std::shared_ptr<proto::AsymmetricKey>>& Crypto::BIP32() const
{
    OT_ASSERT(nullptr != bitcoincrypto_);

    return *bitcoincrypto_;
}
#endif

#if OT_CRYPTO_WITH_BIP39
const opentxs::crypto::Bip39& Crypto::BIP39() const
{
    OT_ASSERT(nullptr != bitcoincrypto_);

    return *bitcoincrypto_;
}
#endif

const OTCachedKey& Crypto::CachedKey(const Identifier& id) const
{
    Lock lock(cached_key_lock_);

    auto& output = cached_keys_[id];

    if (false == bool(output)) {
        OT_FAIL_MSG("This function is broken, this never should have "
                    "happened.");

        output.reset(new OTCachedKey(OT_MASTER_KEY_TIMEOUT));
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
        Armored serialized{};
        const bool haveSerialized = source.SerializeTo(serialized);

        OT_ASSERT(haveSerialized);

        output.reset(new OTCachedKey(serialized));
    }

    OT_ASSERT(output);

    return *output;
}

void Crypto::Cleanup()
{
#if OT_CRYPTO_USING_OPENSSL
    ssl_->Cleanup();
#endif
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
    OT_ASSERT(nullptr != ed25519_);

    return *ed25519_;
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
#if OT_CRYPTO_USING_LIBSECP256K1
    secp256k1_.reset(Factory::Secp256k1(util_, *bitcoincrypto_));
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    encode_.reset(Factory::Encode(*bitcoincrypto_));
    hash_.reset(Factory::Hash(
        *encode_,
        *ssl_,
        *ed25519_
#if OT_CRYPTO_USING_TREZOR
        ,
        *bitcoincrypto_
#endif  // OT_CRYPTO_USING_TREZOR
        ));
    symmetric_.reset(Factory::Symmetric(*ed25519_));

    otWarn << OT_METHOD << __FUNCTION__
           << ": Setting up rlimits, and crypto libraries...\n";

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
        primary_key_.reset(new OTCachedKey(OT_MASTER_KEY_TIMEOUT));
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
#if OT_CRYPTO_USING_LIBSECP256K1
    OT_ASSERT(nullptr != secp256k1_);

    return *secp256k1_;
#else
    OT_ASSERT(nullptr != bitcoincrypto_);

    return *bitcoincrypto_;
#endif
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

OTSymmetricKey Crypto::GetStorageKey(__attribute__((unused))
                                     std::string& seed) const
{
#if OT_CRYPTO_WITH_BIP39
    auto serialized = BIP32().GetStorageKey(seed);

    if (false == bool(serialized)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load encryption key."
              << std::endl;

        return opentxs::crypto::key::Symmetric::Factory();
    }

    OTPassword keySource{};
    auto sessionKey = Symmetric().Key(
        serialized->encryptedkey().key(), serialized->encryptedkey().mode());
    OTPasswordData blank(__FUNCTION__);
    const bool decrypted =
        sessionKey->Decrypt(serialized->encryptedkey(), blank, keySource);

    if (false == decrypted) {
        return opentxs::crypto::key::Symmetric::Factory();
    }

    return Symmetric().Key(keySource);
#else
    return opentxs::crypto::key::Symmetric::Factory();
#endif
}

Crypto::~Crypto() { Cleanup(); }
}  // namespace opentxs::api::implementation
