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

#ifndef OPENTXS_CORE_CRYPTO_OTCRYPTOOPENSSL_HPP
#define OPENTXS_CORE_CRYPTO_OTCRYPTOOPENSSL_HPP

#if OT_CRYPTO_USING_OPENSSL

#include "opentxs/core/crypto/Crypto.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#endif
#include "opentxs/core/crypto/CryptoHash.hpp"
#if OT_CRYPTO_SUPPORTED_ALGO_AES
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#endif
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>
#include <memory>
#include <mutex>
#include <set>
#include <string>

namespace opentxs
{

class OTAsymmetricKey;
class Data;
class Identifier;
class OTPassword;
class OTPasswordData;
class Data;
class Nym;
class Settings;
class OTSignature;

class OpenSSL : public Crypto
#if OT_CRYPTO_SUPPORTED_KEY_RSA
  , public CryptoAsymmetric
#endif
#if OT_CRYPTO_SUPPORTED_ALGO_AES
  , public CryptoSymmetric
#endif
  , public CryptoUtil
  , public CryptoHash
{
private:
    friend class CryptoEngine;

    class OpenSSLdp;

    std::unique_ptr<OpenSSLdp> dp_;

    bool ArgumentCheck(
        const bool encrypt,
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const Data& tag,
        const char* input,
        const uint32_t inputLength,
        bool& AEAD,
        bool& ECB) const;
    void Cleanup_Override() const override;
    bool GetPasswordFromConsole(
        OTPassword& theOutput,
        const char* szPrompt) const override;
    void Init_Override() const override;

    OpenSSL();

public:
    static std::mutex* s_arrayMutex;

    // (To instantiate a text secret, just do this: OTPassword thePass;)
    OTPassword* InstantiateBinarySecret() const override;
    BinarySecret InstantiateBinarySecretSP() const override;

    // RANDOM NUMBERS
    bool RandomizeMemory(
        uint8_t* szDestination,
        uint32_t nNewSize) const override;

    OTPassword* DeriveNewKey(
        const OTPassword& userPassword,
        const Data& dataSalt,
        uint32_t uIterations,
        Data& dataCheckHash) const override;

    // ENCRYPT / DECRYPT
    // Symmetric (secret key) encryption / decryption
    bool Encrypt(
        const OTPassword& theRawSymmetricKey, // The symmetric key, in clear
                                              // form.
        const char* szInput,                  // This is the Plaintext.
        uint32_t lInputLength,
        const Data& theIV, // (We assume this IV is already generated and
                             // passed in.)
        Data& theEncryptedOutput) const override;
    bool Encrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const char* plaintext,
        uint32_t plaintextLength,
        Data& ciphertext) const override;
    bool Encrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const char* plaintext,
        uint32_t plaintextLength,
        Data& ciphertext) const override;
    bool Encrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const char* plaintext,
        uint32_t plaintextLength,
        Data& ciphertext,
        Data& tag) const override;
    bool Decrypt(
        const OTPassword& theRawSymmetricKey,
        const char* szInput,
        uint32_t lInputLength,
        const Data& theIV,
        CryptoSymmetricDecryptOutput& theDecryptedOutput) const override;
    bool Decrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const char* ciphertext,
        uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput& plaintext) const override;
    bool Decrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const char* ciphertext,
        uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput& plaintext) const override;
    bool Decrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const Data& tag,
        const char* ciphertext,
        const uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput& plaintext) const override;

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

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    // SIGN / VERIFY
    // Sign or verify using the Asymmetric Key itself.
    bool Sign(
        const Data& plaintext,
        const OTAsymmetricKey& theKey,
        const proto::HashType hashType,
        Data& signature, // output
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr) const override;
    bool Verify(
        const Data& plaintext,
        const OTAsymmetricKey& theKey,
        const Data& signature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const override;

    // Session key operations (used by opentxs::Letter)
    // Asymmetric (public key) encryption / decryption
    bool EncryptSessionKey(
        const mapOfAsymmetricKeys& RecipPubKeys,
        Data& plaintext,
        Data& dataOutput) const;
    bool DecryptSessionKey(
        Data& dataInput,
        const Nym& theRecipient,
        Data& plaintext,
        const OTPasswordData* pPWData = nullptr) const;
#endif // OT_CRYPTO_SUPPORTED_KEY_RSA

    void thread_setup() const;
    void thread_cleanup() const;

    ~OpenSSL();
};
} // namespace opentxs
#endif // OT_CRYPTO_USING_OPENSSL
#endif // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
