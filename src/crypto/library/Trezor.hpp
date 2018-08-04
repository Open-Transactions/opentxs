// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_TREZOR_CRYPTO_HPP
#define OPENTXS_CORE_CRYPTO_TREZOR_CRYPTO_HPP

#include "opentxs/crypto/library/Trezor.hpp"

namespace opentxs::crypto::implementation
{

typedef std::shared_ptr<proto::AsymmetricKey> TrezorKey;

class Trezor final : virtual public crypto::Trezor<TrezorKey>,
                     virtual public EncodingProvider
#if OT_CRYPTO_WITH_BIP39
    ,
                     public Bip39
#endif
#if OT_CRYPTO_WITH_BIP32
    ,
                     public Bip32
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    ,
                     public AsymmetricProvider,
                     public EcdsaProvider
#endif
{
public:
#if OT_CRYPTO_WITH_BIP32

    typedef bool DerivationMode;
    static const DerivationMode DERIVE_PRIVATE = true;
    static const DerivationMode DERIVE_PUBLIC = false;
    
    std::shared_ptr<proto::AsymmetricKey> GetChild(
        const proto::AsymmetricKey& parent,
        const std::uint32_t index) const override;
    std::shared_ptr<proto::AsymmetricKey> GetHDKey(
        const EcdsaCurve& curve,
        const OTPassword& seed,
        proto::HDPath& path) const override;
    std::string SeedToFingerprint(
        const EcdsaCurve& curve,
        const OTPassword& seed) const override;
    std::shared_ptr<proto::AsymmetricKey> SeedToPrivateKey(
        const EcdsaCurve& curve,
        const OTPassword& seed) const override;
#endif
    std::string Base58CheckEncode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const override;
    bool Base58CheckDecode(const std::string&& input, RawData& output)
        const override;
    bool RIPEMD160(
        const std::uint8_t* input,
        const size_t inputSize,
        std::uint8_t* output) const override;

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    bool Sign(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const proto::HashType hashType,
        Data& signature,  // output
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr) const override;
    bool Verify(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const Data& signature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const override;
    bool ECDH(
        const Data& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const override;
#endif

    ~Trezor() = default;

private:
    friend Factory;

    const std::uint8_t KeyMax[32]{
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFC, 0x2F};

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1        
    bool ScalarBaseMultiply(const OTPassword& privateKey, Data& publicKey)
        const override;
    bool RandomKeypair(OTPassword& privateKey, Data& publicKey) const override;
    bool ValidPrivateKey(const OTPassword& key) const;
#endif

#if OT_CRYPTO_WITH_BIP39
    bool toWords(const OTPassword& seed, OTPassword& words) const override;
    void WordsToSeed(
        const OTPassword& words,
        OTPassword& seed,
        const OTPassword& passphrase) const override;
#endif

#if OT_CRYPTO_WITH_BIP32
    const curve_info* secp256k1_{nullptr};
#endif

    Trezor(const api::Native& native);
    Trezor() = delete;
    Trezor(const Trezor&) = delete;
    Trezor(Trezor&&) = delete;
    Trezor& operator=(const Trezor&) = delete;
    Trezor& operator=(Trezor&&) = delete;
};
}  // namespace opentxs::crypto::implementation
#endif  // OPENTXS_CORE_CRYPTO_TREZOR_CRYPTO_HPP
