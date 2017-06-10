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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRIC_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRIC_HPP

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"

#include <memory>
#include <mutex>
#include <tuple>

namespace opentxs
{

class Data;
class OTPassword;
class OTPasswordData;
class Data;

typedef std::tuple<String, String, String, String, std::shared_ptr<OTEnvelope> > symmetricEnvelope;
typedef std::shared_ptr<OTPassword> BinarySecret;
// Sometimes I want to decrypt into an OTPassword (for encrypted symmetric
// keys being decrypted) and sometimes I want to decrypt into an Data
// (For most other types of data.) This class allows me to do it either way
// without duplicating the static Decrypt() function, by wrapping both
// types.
//
class CryptoSymmetricDecryptOutput
{
private:
    OTPassword* m_pPassword{nullptr};
    Data* m_pPayload{nullptr};

    CryptoSymmetricDecryptOutput();

public:
    EXPORT ~CryptoSymmetricDecryptOutput();

    EXPORT CryptoSymmetricDecryptOutput(const CryptoSymmetricDecryptOutput& rhs);

    EXPORT CryptoSymmetricDecryptOutput(OTPassword& thePassword);
    EXPORT CryptoSymmetricDecryptOutput(Data& thePayload);

    EXPORT void swap(CryptoSymmetricDecryptOutput& other);

    EXPORT CryptoSymmetricDecryptOutput& operator=(
        CryptoSymmetricDecryptOutput other); // passed by value.

    EXPORT bool Concatenate(const void* pAppendData,
                            uint32_t lAppendSize) const;

    EXPORT void Release(); // Someday make this virtual, if we ever subclass it.
    EXPORT void Release_Envelope_Decrypt_Output() const;
};

class CryptoSymmetric
{
public:
    enum Mode: int32_t {
        ERROR_MODE,
        AES_128_CBC,
        AES_256_CBC,
        AES_256_ECB,
        AES_128_GCM,
        AES_256_GCM
    };

    static String ModeToString(const Mode Mode);

    static Mode StringToMode(const String& Mode);

    static uint32_t KeySize(const Mode Mode);
    static uint32_t IVSize(const Mode Mode);
    static uint32_t TagSize(const Mode Mode);

    // InstantiateBinarySecret
    // (To instantiate a text secret, just do this: OTPassword thePass;)
    //
    virtual OTPassword* InstantiateBinarySecret() const = 0;
    virtual BinarySecret InstantiateBinarySecretSP() const = 0;
    // KEY DERIVATION
    //
    // DeriveNewKey derives a 128-bit symmetric key from a passphrase.
    //
    // The OTPassword* returned is the actual derived key. (The result.)
    //
    // However, you would not use it directly for symmetric-key crypto, but
    // instead you'd use the OTSymmetricKey class. This is because you still
    // need an object to manage everything about the symmetric key. It stores
    // the salt and the iteration count, as well as ONLY the ENCRYPTED version
    // of the symmetric key, which is a completely random number and is only
    // decrypted briefly for specific operations. The derived key (below) is
    // what we use for briefly decrypting that actual (random) symmetric key.
    //
    // Therefore this function is mainly used INSIDE OTSymmetricKey as part of
    // its internal operations.
    //
    // userPassword argument contains the user's password which is used to
    // derive the key. Presumably you already obtained this passphrase...
    // Then the derived key is returned, or nullptr if failure. CALLER
    // IS RESPONSIBLE TO DELETE!
    // Todo: return a smart pointer here.
    //
    virtual OTPassword* DeriveNewKey(const OTPassword& userPassword,
                                     const Data& dataSalt,
                                     uint32_t uIterations,
                                     Data& dataCheckHash) const = 0;

    // ENCRYPT / DECRYPT
    //
    // Symmetric (secret key) encryption / decryption
    //
    virtual bool Encrypt(
        const OTPassword& theRawSymmetricKey, // The symmetric key, in clear
                                              // form.
        const char* szInput,                  // This is the Plaintext.
        uint32_t lInputLength,
        const Data& theIV, // (We assume this IV is already generated and
                             // passed in.)
        Data& theEncryptedOutput) const = 0; // OUTPUT. (Ciphertext.)
    virtual bool Encrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const char* plaintext,
        uint32_t plaintextLength,
        Data& ciphertext) const = 0;
    virtual bool Encrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const char* plaintext,
        uint32_t plaintextLength,
        Data& ciphertext) const = 0;
    virtual bool Encrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const char* plaintext,
        uint32_t plaintextLength,
        Data& ciphertext,
        Data& tag) const = 0;

    virtual bool Decrypt(const OTPassword& theRawSymmetricKey, // The symmetric
                                                               // key, in clear
                                                               // form.
                         const char* szInput, // This is the Ciphertext.
                         uint32_t lInputLength,
                         const Data& theIV, // (We assume this IV is
                                              // already generated and passed
                                              // in.)
                         CryptoSymmetricDecryptOutput& theDecryptedOutput)
        const = 0; // OUTPUT. (Recovered plaintext.) You can pass OTPassword& OR
                   // Data& here (either will work.)
    virtual bool Decrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const char* ciphertext,
        uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput& plaintext) const = 0;
    virtual bool Decrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const char* ciphertext,
        uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput& plaintext) const = 0;
    virtual bool Decrypt(
        const CryptoSymmetric::Mode cipher,
        const OTPassword& key,
        const Data& iv,
        const Data& tag,
        const char* ciphertext,
        const uint32_t ciphertextLength,
        CryptoSymmetricDecryptOutput& plaintext) const = 0;
};

} // namespace opentxs

/*
 ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-5v2/pkcs5v2_1.pdf

 4.2 Iteration count

 An iteration count has traditionally served the purpose of increasing
 the cost of producing keys from a password, thereby also increasing
 the difficulty of attack. For the methods in this document, a minimum
 of 1000 iterations is recommended. This will increase the cost of
 exhaustive search for passwords significantly, without a noticeable
 impact in the cost of deriving individual keys.

 Time the KDF on your systems and see how many iterations are practical
 without signficant resource impact on legitimate use cases. If it is
 practical to make the count configurable, do so, otherwise hard-code
 a sensible value that is at least 1000.

 The iteration count is a multiplier on the CPU cost of brute-force
 dictionary attacks. If you are not sure that users are choosing "strong"
 passwords (they rarely do), you want to make dictionary attacks difficult
 by making individual password->key calculations sufficiently slow thereby
 limiting the throughput of brute-force attacks.

 If no legitimate system is computing multiple password-based keys per
 second, you could set the iteration count to consume 10-100ms of CPU
 on 2008 processors, this is likely much more than 1000 iterations.
 At a guess 1000 iterations will be O(1ms) per key on a typical modern CPU.
 This is based on "openssl speed sha1" reporting:

 type             16 bytes     64 bytes    256 bytes   1024 bytes   8192 bytes
 sha1             18701.67k    49726.06k   104600.90k   141349.84k   157502.27k

 or about 10^6 16-byte SHA1 ops per second, doing 1000 iterations of HMAC
 is approximately 2000 SHA1 ops and so should take about 2ms. In many
 applications 10,000 or even 100,000 may be practical provided no
 legitimate "actor" needs to perform password->key comptutations at
 a moderately high rate.

 */

/*
 int32_t PKCS5_PBKDF2_HMAC_SHA1    (
    const void*     password,
    size_t          password_len,

    const void*     salt,
    size_t          salt_len,

    uint64_t     iter,

    size_t          keylen,
    void*          key
)
*/

#endif // OPENTXS_CORE_CRYPTO_CRYPTOSYMMETRIC_HPP
