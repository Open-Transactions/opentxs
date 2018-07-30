// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_SECP256K1_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_SECP256K1_HPP

#if OT_CRYPTO_USING_LIBSECP256K1
namespace opentxs::crypto::implementation
{
class Secp256k1 final : virtual public crypto::Secp256k1,
                        public AsymmetricProvider,
                        public EcdsaProvider
{
public:
    bool RandomKeypair(OTPassword& privateKey, Data& publicKey) const override;
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

    void Init() override;

    ~Secp256k1();

private:
    friend Factory;

    static const int PrivateKeySize = 32;
    static const int PublicKeySize = 33;
    static bool Initialized_;

    secp256k1_context* context_{nullptr};
    const crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>& ecdsa_;
    const api::crypto::Util& ssl_;

    bool ParsePublicKey(const Data& input, secp256k1_pubkey& output) const;
    bool ECDH(
        const Data& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const override;
    bool DataToECSignature(
        const Data& inSignature,
        secp256k1_ecdsa_signature& outSignature) const;
    bool ScalarBaseMultiply(const OTPassword& privateKey, Data& publicKey)
        const override;

    Secp256k1(const api::crypto::Util& ssl, const crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>& ecdsa);
    Secp256k1() = delete;
};
}  // namespace opentxs::crypto::implementation
#endif  // OT_CRYPTO_USING_LIBSECP256K1
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_LIBRARY_SECP256K1_HPP
