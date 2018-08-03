// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_ECDSAPROVIDER_HPP
#define OPENTXS_CRYPTO_LIBRARY_ECDSAPROVIDER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

namespace opentxs
{
namespace crypto
{
class EcdsaProvider
{
public:
    EXPORT static bool DecryptPrivateKey(
        const proto::Ciphertext& encryptedKey,
        const OTPasswordData& password,
        OTPassword& plaintextKey);
    EXPORT static bool DecryptPrivateKey(
        const proto::Ciphertext& encryptedKey,
        const proto::Ciphertext& encryptedChaincode,
        const OTPasswordData& password,
        OTPassword& key,
        OTPassword& chaincode);
    EXPORT static bool EncryptPrivateKey(
        const OTPassword& plaintextKey,
        const OTPasswordData& password,
        proto::Ciphertext& encryptedKey);
    EXPORT static bool EncryptPrivateKey(
        const OTPassword& key,
        const OTPassword& chaincode,
        const OTPasswordData& password,
        proto::Ciphertext& encryptedKey,
        proto::Ciphertext& encryptedChaincode);

    EXPORT virtual bool AsymmetricKeyToECPrivatekey(
        const crypto::key::EllipticCurve& asymmetricKey,
        const OTPasswordData& passwordData,
        OTPassword& privkey) const = 0;
    EXPORT virtual bool DecryptSessionKeyECDH(
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        const OTPasswordData& password,
        crypto::key::Symmetric& sessionKey) const = 0;
    EXPORT virtual bool ECPrivatekeyToAsymmetricKey(
        const OTPassword& privkey,
        const OTPasswordData& passwordData,
        crypto::key::EllipticCurve& asymmetricKey) const = 0;
    EXPORT virtual bool ECPubkeyToAsymmetricKey(
        const Data& pubkey,
        crypto::key::EllipticCurve& asymmetricKey) const = 0;
    EXPORT virtual bool EncryptSessionKeyECDH(
        const crypto::key::EllipticCurve& privateKey,
        const crypto::key::EllipticCurve& publicKey,
        const OTPasswordData& passwordData,
        crypto::key::Symmetric& sessionKey,
        OTPassword& newKeyPassword) const = 0;
    EXPORT virtual bool ExportECPrivatekey(
        const OTPassword& privkey,
        const OTPasswordData& password,
        crypto::key::EllipticCurve& asymmetricKey) const = 0;
    EXPORT virtual bool ImportECPrivatekey(
        const proto::Ciphertext& asymmetricKey,
        const OTPasswordData& password,
        OTPassword& privkey) const = 0;
    EXPORT virtual bool PrivateToPublic(
        const proto::AsymmetricKey& privateKey,
        proto::AsymmetricKey& publicKey) const = 0;
    EXPORT virtual bool PrivateToPublic(
        const proto::Ciphertext& privateKey,
        Data& publicKey) const = 0;
    EXPORT virtual bool RandomKeypair(OTPassword& privateKey, Data& publicKey)
        const = 0;
    EXPORT virtual bool SeedToCurveKey(
        const OTPassword& seed,
        OTPassword& privateKey,
        Data& publicKey) const = 0;
    /*virtual bool ECDH(
        const Data& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const = 0;*/

    EXPORT virtual ~EcdsaProvider() = default;

protected:
    EcdsaProvider() = default;

private:
    EcdsaProvider(const EcdsaProvider&) = delete;
    EcdsaProvider(EcdsaProvider&&) = delete;
    EcdsaProvider& operator=(const EcdsaProvider&) = delete;
    EcdsaProvider& operator=(EcdsaProvider&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif  // OPENTXS_CRYPTO_LIBRARY_ECDSAPROVIDER_HPP
