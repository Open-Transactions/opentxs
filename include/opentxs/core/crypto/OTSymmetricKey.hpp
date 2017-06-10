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

#ifndef OPENTXS_CORE_CRYPTO_OTSYMMETRICKEY_HPP
#define OPENTXS_CORE_CRYPTO_OTSYMMETRICKEY_HPP

#include "opentxs/core/Data.hpp"

#include <stdint.h>

namespace opentxs
{

class Identifier;
class OTASCIIArmor;
class OTPassword;
class String;

class OTSymmetricKey
{
private:
    // GetKey asserts if this is false; GenerateKey asserts if it's true.
    bool m_bIsGenerated{false};
    // If a hash-check fo the Derived Key has been made yet.
    bool m_bHasHashCheck{false};
    // The size, in bits. For example, 128 bit key, 256 bit key, etc.
    uint32_t m_nKeySize{0};
    // Stores the iteration count, which should probably be at least 2000.
    // (Number of iterations used while generating key from passphrase.)
    uint32_t m_uIterationCount{0};
    // Stores the SALT (which is used with the password for generating /
    // retrieving the key from m_dataEncryptedKey)
    Data m_dataSalt;
    // Stores the IV used internally for encrypting / decrypting the actual key
    // (using the derived key) from m_dataEncryptedKey.
    Data m_dataIV;
    // Stores only encrypted version of symmetric key.
    Data m_dataEncryptedKey;
    Data m_dataHashCheck;

public:
    // The highest-level possible interface (used by the API)

    // Caller must delete.
    EXPORT static OTPassword* GetPassphraseFromUser(
        const String* pstrDisplay = nullptr,
        bool bAskTwice = false); // returns a text OTPassword, or nullptr.

    // If you already have the passphrase, you can pass it in as an optional
    // arg.
    // That way if you have to use it 100 times in a row, the user doesn't
    // actually have
    // to TYPE it 100 times in a row.
    //
    EXPORT static bool CreateNewKey(String& strOutput,
                                    const String* pstrDisplay = nullptr,
                                    const OTPassword* pAlreadyHavePW = nullptr);
    EXPORT static bool Encrypt(const String& strKey, const String& strPlaintext,
                               String& strOutput,
                               const String* pstrDisplay = nullptr,
                               bool bBookends = true,
                               const OTPassword* pAlreadyHavePW = nullptr);

    EXPORT static bool Decrypt(const String& strKey, String& strCiphertext,
                               String& strOutput,
                               const String* pstrDisplay = nullptr,
                               const OTPassword* pAlreadyHavePW = nullptr);

    EXPORT static bool Encrypt(const OTSymmetricKey& theKey,
                               const String& strPlaintext, String& strOutput,
                               const String* pstrDisplay = nullptr,
                               bool bBookends = true,
                               const OTPassword* pAlreadyHavePW = nullptr);

    EXPORT static bool Decrypt(const OTSymmetricKey& theKey,
                               const String& strCiphertext, String& strOutput,
                               const String* pstrDisplay = nullptr,
                               const OTPassword* pAlreadyHavePW = nullptr);

    EXPORT bool SerializeTo(Data& theOutput) const;
    EXPORT bool SerializeFrom(Data& theInput);

    EXPORT bool SerializeTo(OTASCIIArmor& ascOutput) const;
    EXPORT bool SerializeFrom(const OTASCIIArmor& ascInput);

    EXPORT bool SerializeTo(String& strOutput, bool bEscaped = false) const;
    EXPORT bool SerializeFrom(const String& strInput, bool bEscaped = false);
    inline bool IsGenerated() const
    {
        return m_bIsGenerated;
    }
    inline bool HasHashCheck() const
    {
        return m_bHasHashCheck;
    }
    EXPORT void GetIdentifier(Identifier& theIdentifier) const;
    EXPORT void GetIdentifier(String& strIdentifier) const;
    // The derived key is used for decrypting the actual symmetric key.
    // It's called the derived key because it is derived from the passphrase.
    //

    // Must have a hash-check already!
    EXPORT OTPassword* CalculateDerivedKeyFromPassphrase(
        const OTPassword& thePassphrase, bool bCheckForHashCheck = true) const;

    // Must not have a hash-check yet!
    EXPORT OTPassword* CalculateNewDerivedKeyFromPassphrase(
        const OTPassword& thePassphrase); // not const!

    // Assumes key is already generated. Tries to get the raw clear key from its
    // encrypted form, via its passphrase being used to derive a key for that
    // purpose.
    //
    EXPORT bool GetRawKeyFromPassphrase(
        const OTPassword& thePassphrase, OTPassword& theRawKeyOutput,
        OTPassword* pDerivedKey = nullptr) const;

    // Assumes key is already generated. Tries to get the raw clear key
    // from its encrypted form, via a derived key.
    //
    EXPORT bool GetRawKeyFromDerivedKey(const OTPassword& theDerivedKey,
                                        OTPassword& theRawKeyOutput) const;
    // Generates this OTSymmetricKey based on an OTPassword. The generated key
    // is
    // stored in encrypted form, based on a derived key from that password.
    //
    EXPORT bool GenerateKey(const OTPassword& thePassphrase,
                            OTPassword** ppDerivedKey = nullptr); // If you
                                                                  // want, I
    // can pass this
    // back to you.
    // Changes the passphrase on an existing symmetric key.
    //
    EXPORT bool ChangePassphrase(const OTPassword& oldPassphrase,
                                 const OTPassword& newPassphrase);
    // For old SymmetricKey's that do not yet have a hash-check.
    // This will generate a hash check for them.
    //
    EXPORT bool GenerateHashCheck(const OTPassword& thePassphrase);

    EXPORT OTSymmetricKey();
    EXPORT OTSymmetricKey(const OTPassword& thePassword);

    EXPORT virtual ~OTSymmetricKey();
    EXPORT virtual void Release();

    EXPORT void Release_SymmetricKey();
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTSYMMETRICKEY_HPP
