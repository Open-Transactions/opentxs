/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_CRYPTO_LIBSECP256K1_HPP
#define OPENTXS_CORE_CRYPTO_LIBSECP256K1_HPP

#include "opentxs/core/crypto/Crypto.hpp"
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/Proto.hpp"

extern "C" {
#include "secp256k1.h"
}

namespace opentxs
{

class CryptoEngine;
class OTAsymmetricKey;
class Data;
class OTPassword;
class OTPasswordData;
class Nym;
class CryptoUtil;

class Libsecp256k1 : public Crypto, public CryptoAsymmetric, public Ecdsa
{
    friend class CryptoEngine;

private:
    static const int PrivateKeySize = 32;
    static const int PublicKeySize = 33;

    secp256k1_context* context_{nullptr};
    Ecdsa& ecdsa_;
    CryptoUtil& ssl_;

    bool ParsePublicKey(const Data& input, secp256k1_pubkey& output) const;
    void Init_Override() const override;
    void Cleanup_Override() const override {};
    bool ECDH(
        const Data& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const override;
    bool DataToECSignature(
        const Data& inSignature,
        secp256k1_ecdsa_signature& outSignature) const;
    bool ScalarBaseMultiply(
        const OTPassword& privateKey,
        Data& publicKey) const override;

    Libsecp256k1() = delete;
    explicit Libsecp256k1(CryptoUtil& ssl, Ecdsa& ecdsa);

public:
    bool RandomKeypair(
        OTPassword& privateKey,
        Data& publicKey) const override;
    bool Sign(
        const Data& plaintext,
        const OTAsymmetricKey& theKey,
        const proto::HashType hashType,
        Data& signature,  // output
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr) const override;
    bool Verify(
        const Data& plaintext,
        const OTAsymmetricKey& theKey,
        const Data& signature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const override;

    virtual ~Libsecp256k1();
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
