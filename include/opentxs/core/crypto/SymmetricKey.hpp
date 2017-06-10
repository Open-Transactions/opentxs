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

#ifndef OPENTXS_CORE_CRYPTO_SYMMETRICKEY_HPP
#define OPENTXS_CORE_CRYPTO_SYMMETRICKEY_HPP

#include "opentxs/core/Proto.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{

class CryptoSymmetricNew;
class OTAsymmetricKeyEC;
class Data;
class OTPassword;
class OTPasswordData;
class String;

class SymmetricKey
{
private:
    /// The library providing the underlying crypto algorithms
    const CryptoSymmetricNew& engine_;
    const std::uint32_t version_{0};
    const proto::SymmetricKeyType type_{proto::SKEYTYPE_ERROR};
    /// Size of the plaintext key in bytes;
    std::size_t key_size_{0};
    std::unique_ptr<std::string> salt_;
    std::uint64_t operations_{0};
    std::uint64_t difficulty_{0};

    /// The unencrypted, fully-derived version of the key which is provided to
    /// encryption functions.
    std::unique_ptr<OTPassword> plaintext_key_;
    /// The encrypted form of the plaintext key
    std::unique_ptr<proto::Ciphertext> encrypted_key_;

    bool Allocate(
        const std::size_t size,
        Data& container);
    bool Allocate(
        const std::size_t size,
        std::string& container);
    bool Allocate(
        const std::size_t size,
        OTPassword& container,
        const bool text = false);
    bool Decrypt(
        const proto::Ciphertext& input,
        const OTPasswordData& keyPassword,
        std::uint8_t* plaintext);
    bool Encrypt(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* iv,
        const std::size_t ivSize,
        const proto::SymmetricMode mode,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool text = false);
    bool EncryptKey(
        const OTPassword& plaintextKey,
        const OTPasswordData& keyPassword,
        const proto::SymmetricKeyType type = proto::SKEYTYPE_ARGON2);
    bool GetPassword(
        const OTPasswordData& keyPassword,
        OTPassword& password);

    SymmetricKey(const CryptoSymmetricNew& engine);
    SymmetricKey(
        const CryptoSymmetricNew& engine,
        const proto::SymmetricKey serialized);
    SymmetricKey(
        const CryptoSymmetricNew& engine,
        const OTPassword& seed,
        const std::string& salt,
        const std::size_t size,
        const std::uint64_t operations = 3,
        const std::uint64_t difficulty = 8388608,
        const proto::SymmetricKeyType type = proto::SKEYTYPE_ARGON2);
    SymmetricKey() = delete;
    SymmetricKey(const SymmetricKey&) = delete;
    SymmetricKey& operator=(const SymmetricKey&) = delete;

public:
    /** Derive a new, random symmetric key
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] keyPassword Optional key password information.
     *  \param[in] mode The symmetric algorithm for which to generate an
     *                  appropriate key
     */
    static std::unique_ptr<SymmetricKey> Factory(
        const CryptoSymmetricNew& engine,
        const OTPasswordData& password,
        const proto::SymmetricMode mode = proto::SMODE_ERROR);

    /** Instantiate a symmetric key from serialized form
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] serialized The symmetric key in protobuf form
     */
    static std::unique_ptr<SymmetricKey> Factory(
        const CryptoSymmetricNew& engine,
        const proto::SymmetricKey serialized);

    /** Derive a symmetric key from a seed
     *
     *  \param[in] seed A binary or text seed to be expanded into a secret key
     *  \param[in] salt A nonce to be used for encrypting the derived key
     *  \param[in] operations The number of iterations/operations the KDF should
     *                        perform
     *  \param[in] difficulty A type-specific difficulty parameter used by the
     *                        KDF.
     *  \param[in] size       The target number of bytes for the derived secret
     *                        key
     *  \param[in] type       The KDF to be used for the derivation process
     */
    static std::unique_ptr<SymmetricKey> Factory(
        const CryptoSymmetricNew& engine,
        const OTPassword& seed,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const std::size_t size,
        const proto::SymmetricKeyType type);

    /** Construct a symmetric key from an existing OTPassword
     *
     *  \param[in] engine A reference to the crypto library to be bound to the
     *                    instance
     *  \param[in] raw An existing, unencrypted binary or text secret
     */
    static std::unique_ptr<SymmetricKey> Factory(
        const CryptoSymmetricNew& engine,
        const OTPassword& raw);

    bool ChangePassword(
        const OTPasswordData& oldPassword,
        const OTPassword& newPassword);

    /** Decrypt ciphertext using the symmetric key
     *
     *  \param[in] ciphertext The data to be decrypted
     *  \param[in] keyPassword The password needed to decrypt the key
     *  \param[out] plaintext The encrypted output
     */
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        Data& plaintext);
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        OTPassword& plaintext);

    /** Encrypt plaintext using the symmetric key
     *
     *  \param[in] plaintext The data to be encrypted
     *  \param[in] iv Nonce for the encryption operation
     *  \param[in] keyPassword The password needed to decrypt the key
     *  \param[out] ciphertext The encrypted output
     *  \param[in] attachKey Set to true if the serialized key should be
     *                       embedded in the ciphertext
     *  \param[in] mode The symmetric algorithm to use for encryption
     */
    bool Encrypt(
        const String& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR);
    bool Encrypt(
        const OTPassword& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR);

    bool Serialize(proto::SymmetricKey& output) const;

    bool Unlock(const OTPasswordData& keyPassword);

    ~SymmetricKey() = default;
};
} // namespace opentxs
#endif // OPENTXS_CORE_CRYPTO_SYMMETRICKEY_HPP
