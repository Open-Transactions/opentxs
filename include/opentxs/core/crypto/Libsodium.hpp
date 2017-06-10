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

#ifndef OPENTXS_CORE_CRYPTO_LIBSODIUM_HPP
#define OPENTXS_CORE_CRYPTO_LIBSODIUM_HPP

#include "opentxs/core/crypto/Crypto.hpp"
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/CryptoSymmetricNew.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"
#include "opentxs/core/Proto.hpp"

#include <cstddef>

namespace opentxs
{

class OTAsymmetricKey;
class Data;
class OTPassword;
class OTPasswordData;

class Libsodium
  : public Crypto
  , public CryptoAsymmetric
  , public CryptoSymmetricNew
  , public Ecdsa
  , public CryptoHash
{
    friend class CryptoEngine;

private:
    static const proto::SymmetricMode DEFAULT_MODE
        {proto::SMODE_CHACHA20POLY1305};

    void Cleanup_Override() const override {}
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const std::uint8_t* key,
        const std::size_t keySize,
        std::uint8_t* plaintext) const override;
    proto::SymmetricMode DefaultMode() const override { return DEFAULT_MODE; }
    bool Derive(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* salt,
        const std::size_t saltSize,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const proto::SymmetricKeyType type,
        std::uint8_t* output,
        std::size_t outputSize) const override;
    bool ECDH(
        const Data& publicKey,
        const OTPassword& seed,
        OTPassword& secret) const override;
    bool Encrypt(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* key,
        const std::size_t keySize,
        proto::Ciphertext& ciphertext) const override;
    bool ExpandSeed(
        const OTPassword& seed,
        OTPassword& privateKey,
        Data& publicKey) const;
    void Init_Override() const override;
    std::size_t IvSize(const proto::SymmetricMode mode) const override;
    std::size_t KeySize(const proto::SymmetricMode mode) const override;
    bool ScalarBaseMultiply(
        const OTPassword& seed,
        Data& publicKey) const override;
    std::size_t SaltSize(const proto::SymmetricKeyType type) const override;
    std::size_t TagSize(const proto::SymmetricMode mode) const override;

    Libsodium() = default;

public:
    bool Digest(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        std::uint8_t* output) const override;
    bool HMAC(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        const std::uint8_t* key,
        const size_t keySize,
        std::uint8_t* output) const override;
    bool RandomKeypair(
        OTPassword& privateKey,
        Data& publicKey) const override;
    bool Sign(
        const Data& plaintext,
        const OTAsymmetricKey& theKey,
        const proto::HashType hashType,
        Data& signature, // output
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr) const override;
    bool SeedToCurveKey(
        const OTPassword& seed,
        OTPassword& privateKey,
        Data& publicKey) const override;
    bool Verify(
        const Data& plaintext,
        const OTAsymmetricKey& theKey,
        const Data& signature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const override;

    virtual ~Libsodium() = default;
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
