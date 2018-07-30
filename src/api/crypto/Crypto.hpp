// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_IMPLEMENTATION_CRYPTO_HPP
#define OPENTXS_API_CRYPTO_IMPLEMENTATION_CRYPTO_HPP

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <map>
#include <memory>
#include <mutex>

namespace opentxs::api::implementation
{
class Crypto : virtual public opentxs::api::Crypto
{
public:
    EXPORT const OTCachedKey& DefaultKey() const override;
    EXPORT Editor<OTCachedKey> mutable_DefaultKey() const override;
    EXPORT const OTCachedKey& CachedKey(const Identifier& id) const override;
    EXPORT const OTCachedKey& CachedKey(
        const OTCachedKey& source) const override;
    EXPORT const OTCachedKey& LoadDefaultKey(
        const Armored& serialized) const override;
    EXPORT void SetTimeout(const std::chrono::seconds& timeout) const override;
    EXPORT void SetSystemKeyring(const bool useKeyring) const override;

    // Encoding function interface
    EXPORT const crypto::Encode& Encode() const override;

    // Hash function interface
    EXPORT const crypto::Hash& Hash() const override;

    // Utility class for misc OpenSSL-provided functions
    EXPORT const crypto::Util& Util() const override;

    // Asymmetric encryption engines
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    EXPORT const opentxs::crypto::AsymmetricProvider& ED25519() const override;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    EXPORT const opentxs::crypto::AsymmetricProvider& RSA() const override;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    EXPORT const opentxs::crypto::AsymmetricProvider& SECP256K1()
        const override;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

    // Symmetric encryption engines
    EXPORT const crypto::Symmetric& Symmetric() const override;

#if OT_CRYPTO_SUPPORTED_ALGO_AES
    EXPORT const opentxs::crypto::LegacySymmetricProvider& AES() const override;
#endif  // OT_CRYPTO_SUPPORTED_ALGO_AES
#if OT_CRYPTO_WITH_BIP32
    EXPORT const opentxs::crypto::Bip32<std::shared_ptr<proto::AsymmetricKey>>& BIP32() const override;
#endif  // OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_WITH_BIP39
    EXPORT const opentxs::crypto::Bip39& BIP39() const override;
#endif  // OT_CRYPTO_WITH_BIP39

    OTSymmetricKey GetStorageKey(std::string& seed) const override;

    ~Crypto();

private:
    friend Factory;

    const api::Native& native_;
    mutable std::mutex cached_key_lock_;
    mutable std::unique_ptr<OTCachedKey> primary_key_;
    mutable std::map<OTIdentifier, std::unique_ptr<OTCachedKey>> cached_keys_;
#if OT_CRYPTO_USING_TREZOR
    std::unique_ptr<opentxs::crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>> bitcoincrypto_;
#endif  // OT_CRYPTO_USING_TREZOR
    std::unique_ptr<opentxs::crypto::Sodium> ed25519_;
#if OT_CRYPTO_USING_OPENSSL
    std::unique_ptr<opentxs::crypto::OpenSSL> ssl_;
#endif  // OT_CRYPTO_USING_OPENSSL
    const api::crypto::Util& util_;
#if OT_CRYPTO_USING_LIBSECP256K1
    std::unique_ptr<opentxs::crypto::Secp256k1> secp256k1_;
#endif  // OT_CRYPTO_USING_LIBSECP256K1
    std::unique_ptr<crypto::Encode> encode_;
    std::unique_ptr<crypto::Hash> hash_;
    std::unique_ptr<crypto::Symmetric> symmetric_;

    void init_default_key(const Lock& lock) const;

    void Init();
    void Cleanup();

    Crypto(const api::Native& native);
    Crypto() = delete;
    Crypto(const Crypto&) = delete;
    Crypto(Crypto&&) = delete;
    Crypto& operator=(const Crypto&) = delete;
    Crypto& operator=(Crypto&&) = delete;
};
}  // namespace opentxs::api::implementation
#endif  // OPENTXS_API_CRYPTO_IMPLEMENTATION_CRYPTO_HPP
