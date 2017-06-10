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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/OTSymmetricKey.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/Crypto.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

extern "C" {
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#endif
}
#include <stdint.h>
#include <ostream>

namespace opentxs
{

// This class stores the iteration count, the salt, and the encrypted key.
// These are all generated or set when you call GenerateKey.

// Note: this calculates its ID based only on m_dataEncryptedKey,
// and does NOT include salt, IV, iteration count, etc when
// generating the hash for the ID.
//
void OTSymmetricKey::GetIdentifier(Identifier& theIdentifier) const
{
    //  const bool bCalc =
    theIdentifier.CalculateDigest(m_dataEncryptedKey);
}

void OTSymmetricKey::GetIdentifier(String& strIdentifier) const
{
    Identifier theIdentifier;
    const bool bCalc = theIdentifier.CalculateDigest(m_dataEncryptedKey);
    if (bCalc) theIdentifier.GetString(strIdentifier);
}

// Changes the passphrase on an existing symmetric key.
//
bool OTSymmetricKey::ChangePassphrase(const OTPassword& oldPassphrase,
                                      const OTPassword& newPassphrase)
{
    OT_ASSERT(m_uIterationCount > 1000);
    OT_ASSERT(m_bIsGenerated);

    // Todo: validate the passphrases exist or whatever?

    otInfo << "  Begin: " << __FUNCTION__
           << ": Changing password on symmetric key...\n";

    OTPassword theActualKey;

    if (!GetRawKeyFromPassphrase(oldPassphrase, theActualKey)) return false;

    Data dataIV, dataSalt;

    // NOTE: I can't randomize the IV because then anything that was
    // encrypted with this key before, will fail to decrypt. (Ruining
    // the whole point of changing the passphrase...)
    //
    // UPDATE: I think this is false. I think the IV is for the encryption of
    // the symmetric key itself, whereas the content has its own IV in
    // OTEnvelope.
    //
    if (!dataIV.Randomize(CryptoConfig::SymmetricIvSize())) {
        otErr << __FUNCTION__ << ": Failed generating iv for changing "
                                 "passphrase on a symmetric key. (Returning "
                                 "false.)\n";
        return false;
    }

    if (!dataSalt.Randomize(CryptoConfig::SymmetricSaltSize())) {
        otErr << __FUNCTION__ << ": Failed generating random salt for changing "
                                 "passphrase on a symmetric key. (Returning "
                                 "false.)\n";
        return false;
    }

    m_dataIV = dataIV;
    m_dataSalt = dataSalt;
    m_bHasHashCheck = false;

    m_dataHashCheck.Release();
    m_dataEncryptedKey.Release();

    // Generate the new derived key from the new passphrase.
    //
    std::unique_ptr<OTPassword> pNewDerivedKey(
        CalculateNewDerivedKeyFromPassphrase(newPassphrase)); // asserts
                                                              // already.

    // Below this point, pNewDerivedKey is NOT null. (And will be cleaned up
    // automatically.)

    //
    // Below this point, pNewDerivedKey contains a symmetric key derived from
    // the new salt, the iteration
    // count, and the new password that was passed in. We will store the salt
    // and iteration count inside this
    // OTSymmetricKey object, and we'll store an encrypted copy of the
    // ActualKey, encrypted to pNewDerivedKey.
    // We'll also store the new IV, which is used while encrypting the actual
    // key, and which must be used again
    // while decrypting it later.
    //
    // Encrypt theActualKey using pNewDerivedKey, which is clear/raw already.
    // (Both are OTPasswords.)
    // Put the result into the Data m_dataEncryptedKey.
    //
    const bool bEncryptedKey = OT::App().Crypto().AES().Encrypt(
        *pNewDerivedKey, // pNewDerivedKey is a symmetric key, in clear form.
                         // Used for encrypting theActualKey.
        reinterpret_cast<const char*>(
            theActualKey.getMemory_uint8()), // This is the Plaintext that's
                                             // being encrypted.
        theActualKey.getMemorySize(),
        m_dataIV,            // generated above.
        m_dataEncryptedKey); // OUTPUT. (Ciphertext.)
    m_bIsGenerated = bEncryptedKey;

    otInfo << "  End: " << __FUNCTION__
           << ": (Changing passphrase on symmetric key...) "
           << (m_bIsGenerated ? "SUCCESS" : "FAILED") << "\n";

    return m_bIsGenerated;
}

// Generates this OTSymmetricKey based on an OTPassword. The generated key is
// stored in encrypted form, based on a derived key from that password.
//

// Done:  Change pDerivedKey to ppDerivedKey, since you CANNOT derive a key
// BEFORE calling
// GenerateKey, since the salt and iteration count are both part of the
// derivation process!!

// ppDerivedKey: CALLER RESPONSIBLE TO DELETE.  (optional arg.)

// Output. If you want, I can pass this back to you.
bool OTSymmetricKey::GenerateKey(const OTPassword& thePassphrase,
                                 OTPassword** ppDerivedKey)
{
    OT_ASSERT(m_uIterationCount > 1000);
    OT_ASSERT(!m_bIsGenerated);
    //  OT_ASSERT(thePassphrase.isPassword());

    otInfo << "  Begin: " << __FUNCTION__
           << ": GENERATING keys and passwords...\n";

    if (!m_dataIV.Randomize(CryptoConfig::SymmetricIvSize())) {
        otErr << __FUNCTION__ << ": Failed generating iv for encrypting a "
                                 "symmetric key. (Returning false.)\n";
        return false;
    }

    if (!m_dataSalt.Randomize(CryptoConfig::SymmetricSaltSize())) {
        otErr << __FUNCTION__
              << ": Failed generating random salt. (Returning false.)\n";
        return false;
    }

    // Generate actual key (a randomized memory space.)
    // We will use the derived key for encrypting the actual key.
    //
    OTPassword theActualKey;

    {
        int32_t nRes =
            theActualKey.randomizeMemory(CryptoConfig::SymmetricKeySize());
        if (0 > nRes) {
            OT_FAIL;
        }
        uint32_t uRes =
            static_cast<uint32_t>(nRes); // we need an uint32_t value.

        if (CryptoConfig::SymmetricKeySize() != uRes) {
            otErr << __FUNCTION__
                  << ": Failed generating symmetric key. (Returning false.)\n";
            return false;
        }
    }
    // We didn't bother generating the derived key if the above three
    // randomizations failed.

    // Generate derived key from passphrase.
    //
    std::unique_ptr<OTPassword> pDerivedKey(
        CalculateNewDerivedKeyFromPassphrase(thePassphrase));

    OT_ASSERT(nullptr != pDerivedKey);

    // Below this point, pDerivedKey is NOT null. (And we only clean it up later
    // if we created it.)

    //
    // Below this point, pDerivedKey contains a symmetric key derived from the
    // salt, the iteration
    // count, and the password that was passed in. We will store the salt and
    // iteration count inside this
    // OTSymmetricKey object, and we'll store an encrypted copy of the
    // ActualKey, encrypted to pDerivedKey.
    // We'll also store the IV, which is generated while encrypting the actual
    // key, and which must be used
    // while decrypting it later.
    //
    // Encrypt theActualKey using pDerivedKey, which is clear/raw already. (Both
    // are OTPasswords.)
    // Put the result into the Data m_dataEncryptedKey.
    //
    const bool bEncryptedKey = OT::App().Crypto().AES().Encrypt(
        *pDerivedKey, // pDerivedKey is a symmetric key, in clear form. Used for
                      // encrypting theActualKey.
        reinterpret_cast<const char*>(
            theActualKey.getMemory_uint8()), // This is the Plaintext that's
                                             // being encrypted.
        theActualKey.getMemorySize(),
        m_dataIV,            // generated above.
        m_dataEncryptedKey); // OUTPUT. (Ciphertext.)
    m_bIsGenerated = bEncryptedKey;

    otInfo << "  End: " << __FUNCTION__
           << ": (GENERATING keys and passwords...) "
           << (m_bIsGenerated ? "SUCCESS" : "FAILED") << "\n";

    // return the pDerivedKey, if wanted.
    if (nullptr != ppDerivedKey) {
        *ppDerivedKey = pDerivedKey.release();
    }

    return m_bIsGenerated;
}

bool OTSymmetricKey::GenerateHashCheck(const OTPassword& thePassphrase)
{
    OT_ASSERT(m_uIterationCount > 1000);

    if (!m_bIsGenerated) {
        otErr << __FUNCTION__ << ": No Key Generated, run GenerateKey(), and "
                                 "this function will not be needed!";
        OT_FAIL;
    }

    if (HasHashCheck()) {
        otErr << __FUNCTION__
              << ": Already have a HashCheck, no need to create one!";
        return false;
    }

    OT_ASSERT(m_dataHashCheck.IsEmpty());

    OTPassword* pDerivedKey =
        CalculateNewDerivedKeyFromPassphrase(thePassphrase); // asserts already.

    if (nullptr ==
        pDerivedKey) // A pointerpointer was passed in... (caller will
                     // be responsible then, to delete.)
    {
        otErr << __FUNCTION__ << ": failed to calculate derived key";
        return false;
    }

    if (!HasHashCheck()) {
        otErr
            << __FUNCTION__
            << ": Still don't have a hash check (even after generating one)\n!"
               "this is bad. Will assert.";
        OT_FAIL;
    }

    return true;
}

/*
 To generate a symmetric key:

    1. First we generate the plain symmetric key itself using RAND_bytes().
    2. Then we generate the salt using RAND_bytes()
    3. Then we use thePassword and the salt to derive a key using PBKDF2.
    4. Then we encrypt the plain symmetric key using the derived key from
 PBKDF2.
    5. Then we store the salt and the encrypted symmetric key. (We discard the
 derived key.)
    6. (Use the plain symmetric key to encrypt the plaintext.)

 To use the symmetric key:

    1. We use thePassword from user input, and the stored salt, with PBKDF2 to
 derive a key.
    2. Use the derived key to decrypt the encrypted symmetric key.
    3. (Use the decrypted symmetric key to decrypt the ciphertext.)
 */

// Done:  add a "get Key" function which takes the OTPassword, generates the
// derived key using salt already on
// OTSymmetricKey object, then decrypts the encrypted symmetric key (using
// derived key) and returns clear symmetric
// key back as another OTPassword object.

// Assumes key is already generated. Tries to get the raw clear key from its
// encrypted form, via
// its passphrase being used to derive a key for that purpose.
//
// If returns true, theRawKeyOutput will contain the decrypted symmetric key, in
// an OTPassword object.
// Otherwise returns false if failure.
//

// The derived key is used for decrypting the actual symmetric key.
// It's called the derived key because it is derived from the passphrase.
//
// CALLER IS RESPONSIBLE TO DELETE.
//
OTPassword* OTSymmetricKey::CalculateDerivedKeyFromPassphrase(
    const OTPassword& thePassphrase, bool bCheckForHashCheck /*= true*/) const
{
    Data tmpDataHashCheck = m_dataHashCheck;

    if (bCheckForHashCheck) {
        if (!HasHashCheck()) {
            otErr << __FUNCTION__ << ": Unable to calculate derived key, as "
                                     "hash check is missing!";
            OT_FAIL;
        }
        OT_ASSERT(!tmpDataHashCheck.IsEmpty());
    }
    else {
        if (!HasHashCheck()) {
            otOut << __FUNCTION__ << ": Warning!! No hash check, ignoring... "
                                     "(since bCheckForHashCheck was set false)";
            OT_ASSERT(tmpDataHashCheck.IsEmpty());
        }
    }

    return OT::App().Crypto().AES().DeriveNewKey(
        thePassphrase, m_dataSalt, m_uIterationCount, tmpDataHashCheck);
}

// CALLER IS RESPONSIBLE TO DELETE.
OTPassword* OTSymmetricKey::CalculateNewDerivedKeyFromPassphrase(
    const OTPassword& thePassphrase)
{
    std::unique_ptr<OTPassword> pDerivedKey;

    if (!HasHashCheck()) {
        m_dataHashCheck.zeroMemory();

        pDerivedKey.reset(OT::App().Crypto().AES().DeriveNewKey(
            thePassphrase, m_dataSalt, m_uIterationCount, m_dataHashCheck));
    }
    else {
        otErr << __FUNCTION__
              << ": Calling Wrong function!! Hash check already exists!";
    }

    OT_ASSERT(pDerivedKey);
    OT_ASSERT(!m_dataHashCheck.IsEmpty());

    m_bHasHashCheck = true;

    return pDerivedKey.release();
}

// Assumes key is already generated. Tries to get the raw clear key from its
// encrypted form, via its passphrase being used to derive a key for that
// purpose.
//
bool OTSymmetricKey::GetRawKeyFromPassphrase(
    const OTPassword& thePassphrase, OTPassword& theRawKeyOutput,
    OTPassword* pDerivedKey) const // Optionally pass this, to save me the step.
{
    OT_ASSERT(m_bIsGenerated);

    std::unique_ptr<OTPassword> theDerivedAngel;

    if (nullptr == pDerivedKey) {
        // TODO, security: Do we have to create all these OTPassword objects on
        // the stack, just as a general practice? In which case I can't use this
        // factory how I'm using it now...
        pDerivedKey = CalculateDerivedKeyFromPassphrase(thePassphrase, false);

        theDerivedAngel.reset(pDerivedKey);
    }

    if (nullptr == pDerivedKey) { return false; }

    // Below this point, pDerivedKey contains a derived symmetric key, from the
    // salt, the iteration count, and the password that was passed in. The salt
    // and iteration count were both stored inside this OTSymmetricKey object
    // since this key was originally generated, and we store an encrypted copy
    // of the ActualKey already, as well-- it's encrypted to the Derived Key.
    // (We also store the IV from that encryption bit.)
    return GetRawKeyFromDerivedKey(*pDerivedKey, theRawKeyOutput);
}

// Assumes key is already generated. Tries to get the raw clear key from its
// encrypted form, via a derived key.
//
// If returns true, theRawKeyOutput will contain the decrypted symmetric key, in
// an OTPassword object.
// Otherwise returns false if failure.
//
bool OTSymmetricKey::GetRawKeyFromDerivedKey(const OTPassword& theDerivedKey,
                                             OTPassword& theRawKeyOutput) const
{
    OT_ASSERT(m_bIsGenerated);
    OT_ASSERT(theDerivedKey.isMemory());

    const char* szFunc = "OTSymmetricKey::GetRawKeyFromDerivedKey";

    // Decrypt theActualKey using theDerivedKey, which is clear/raw already.
    // (Both are OTPasswords.)
    // Put the result into theRawKeyOutput.
    //
    // theDerivedKey is a symmetric key, in clear form. Used here
    // for decrypting m_dataEncryptedKey into theRawKeyOutput.
    //
    otInfo
        << szFunc
        << ": *Begin) Attempting to recover actual key using derived key...\n";

        CryptoSymmetricDecryptOutput plaintext(theRawKeyOutput);
    const bool bDecryptedKey = OT::App().Crypto().AES().Decrypt(
        theDerivedKey, // We're using theDerivedKey to decrypt
                       // m_dataEncryptedKey.
        // Here's what we're trying to decrypt: the encrypted
        // form of the symmetric key.
        static_cast<const char*>(
            m_dataEncryptedKey.GetPointer()), // The Ciphertext.
        m_dataEncryptedKey.GetSize(),
        m_dataIV, // Created when *this symmetric key was generated. Both are
                  // already stored.
        plaintext); // OUTPUT. (Recovered plaintext of symmetric key.) You
                          // can pass OTPassword& OR Data& here (either
                          // will work.)

    otInfo << szFunc
           << ": (End) attempt to recover actual key using derived key...\n";
    return bDecryptedKey;
}

// The highest-level possible interface (used by the API)
//
// static  NOTE: this version circumvents the master key.
OTPassword* OTSymmetricKey::GetPassphraseFromUser(const String* pstrDisplay,
                                                  bool bAskTwice) // returns a
                                                                  // text
                                                                  // OTPassword,
                                                                  // or nullptr.
{
    // Caller MUST delete!

    OTPassword* pPassUserInput =
        OTPassword::CreateTextBuffer(); // already asserts.
    //  pPassUserInput->zeroMemory(); // This was causing the password to come
    // out blank.
    //
    // Below this point, pPassUserInput must be returned, or deleted. (Or it
    // will leak.)

    const char* szDisplay = "OTSymmetricKey::GetPassphraseFromUser";
    OTPasswordData thePWData((nullptr == pstrDisplay) ? szDisplay
                                                      : pstrDisplay->Get());
    // -------------------------------------------------------------------
    //
    // OLD SYSTEM! (NO MASTER KEY INVOLVEMENT.)
    //
    thePWData.setUsingOldSystem(); // So the cached key doesn't interfere, since
                                   // this is for a plain symmetric key.
    // -------------------------------------------------------------------
    const int32_t nCallback =
        souped_up_pass_cb(pPassUserInput->getPasswordWritable_char(),
                          pPassUserInput->getBlockSize(), bAskTwice ? 1 : 0,
                          static_cast<void*>(&thePWData));
    const uint32_t uCallback = static_cast<uint32_t>(nCallback);
    if ((nCallback > 0) && // Success retrieving the passphrase from the user.
        pPassUserInput->SetSize(uCallback)) {
        //      otOut << "%s: Retrieved passphrase (blocksize %d, actual size
        // %d) from user: %s\n", __FUNCTION__,
        //                     pPassUserInput->getBlockSize(), nCallback,
        // pPassUserInput->getPassword());
        return pPassUserInput; // Caller MUST delete!
    }
    else {
        delete pPassUserInput;
        pPassUserInput = nullptr;
        otOut
            << __FUNCTION__
            << ": Sorry, unable to retrieve passphrase from user. (Failure.)\n";
    }

    return nullptr;
}

// static
bool OTSymmetricKey::CreateNewKey(String& strOutput, const String* pstrDisplay,
                                  const OTPassword* pAlreadyHavePW)
{
    std::unique_ptr<OTPassword> pPassUserInput;

    if (nullptr == pAlreadyHavePW) {
        const char* szDisplay = "Creating new symmetric key.";
        const String strDisplay((nullptr == pstrDisplay) ? szDisplay
                                                         : pstrDisplay->Get());

        pPassUserInput.reset(OTSymmetricKey::GetPassphraseFromUser(
            &strDisplay, true)); // bAskTwice=false by default.
    }
    else
        pPassUserInput.reset(new OTPassword(*pAlreadyHavePW));

    bool bSuccess = false;

    if (pPassUserInput) // Success retrieving the passphrase from the
                        // user. (Now let's generate the key...)
    {
        otLog3 << __FUNCTION__
               << ": Calling OTSymmetricKey theKey.GenerateKey()...\n";
        OTSymmetricKey theKey(*pPassUserInput);
        const bool bGenerated = theKey.IsGenerated();
        //      otOut << "%s: Finished calling OTSymmetricKey
        // theKey.GenerateKey()...\n", __FUNCTION__);

        if (bGenerated && theKey.SerializeTo(strOutput))
            bSuccess = true;
        else
            otWarn << __FUNCTION__
                   << ": Sorry, unable to generate key. (Failure.)\n";
    }
    else
        otWarn
            << __FUNCTION__
            << ": Sorry, unable to retrieve password from user. (Failure.)\n";

    return bSuccess;
}

// static
bool OTSymmetricKey::Encrypt(const String& strKey, const String& strPlaintext,
                             String& strOutput, const String* pstrDisplay,
                             bool bBookends, const OTPassword* pAlreadyHavePW)
{
    if (!strKey.Exists() || !strPlaintext.Exists()) {
        otWarn << __FUNCTION__ << ": Nonexistent: either the key or the "
                                  "plaintext. Please supply. (Failure.)\n";
        return false;
    }

    OTSymmetricKey theKey;

    if (!theKey.SerializeFrom(strKey)) {
        otWarn << __FUNCTION__ << ": Failed trying to load symmetric key from "
                                  "string. (Returning false.)\n";
        return false;
    }

    // By this point, we know we have a plaintext and a symmetric Key.
    //
    return OTSymmetricKey::Encrypt(theKey, strPlaintext, strOutput, pstrDisplay,
                                   bBookends, pAlreadyHavePW);
}

// static
bool OTSymmetricKey::Encrypt(const OTSymmetricKey& theKey,
                             const String& strPlaintext, String& strOutput,
                             const String* pstrDisplay, bool bBookends,
                             const OTPassword* pAlreadyHavePW)
{
    if (!theKey.IsGenerated()) {
        otWarn << __FUNCTION__
               << ": Failure: theKey.IsGenerated() was false. (The calling "
                  "code probably should have checked that key already...)\n";
        return false;
    }

    if (!strPlaintext.Exists()) {
        otWarn << __FUNCTION__
               << ": Plaintext is empty. Please supply. (Failure.)\n";
        return false;
    }

    // By this point, we know we have a plaintext and a symmetric Key.
    //
    std::unique_ptr<OTPassword> pPassUserInput;

    if (nullptr == pAlreadyHavePW) {
        const char* szDisplay = "Password-protecting a plaintext.";
        const String strDisplay((nullptr == pstrDisplay) ? szDisplay
                                                         : pstrDisplay->Get());

        pPassUserInput.reset(OTSymmetricKey::GetPassphraseFromUser(
            &strDisplay)); // bAskTwice=false by default.
    }
    else
        pPassUserInput.reset(new OTPassword(*pAlreadyHavePW));

    OTASCIIArmor ascOutput;
    bool bSuccess = false;

    if (nullptr != pPassUserInput) // Success retrieving the passphrase from the
                                   // user. (Now let's encrypt...)
    {
        OTEnvelope theEnvelope;

        if (theEnvelope.Encrypt(strPlaintext,
                                const_cast<OTSymmetricKey&>(theKey),
                                *pPassUserInput) &&
            theEnvelope.GetCiphertext(ascOutput)) {
            bSuccess = true;

            if (bBookends) {
                return ascOutput.WriteArmoredString(
                    strOutput, "SYMMETRIC MSG", // todo hardcoding.
                    false);                     // bEscaped=false
            }
            else {
                strOutput.Set(ascOutput.Get());
            }
        }
        else {
            otWarn << __FUNCTION__ << ": Failed trying to encrypt. (Sorry.)\n";
        }
    }
    else
        otWarn
            << __FUNCTION__
            << ": Sorry, unable to retrieve passphrase from user. (Failure.)\n";

    return bSuccess;
}

// static
bool OTSymmetricKey::Decrypt(const String& strKey, String& strCiphertext,
                             String& strOutput, const String* pstrDisplay,
                             const OTPassword* pAlreadyHavePW)
{

    if (!strKey.Exists()) {
        otWarn
            << __FUNCTION__
            << ": Nonexistent: The symmetric key. Please supply. (Failure.)\n";
        return false;
    }

    OTSymmetricKey theKey;

    if (!theKey.SerializeFrom(strKey)) {
        otWarn << __FUNCTION__ << ": Failed trying to load symmetric key from "
                                  "string. (Returning false.)\n";
        return false;
    }

    // By this point, we know we have a ciphertext envelope and a symmetric Key.
    //
    return OTSymmetricKey::Decrypt(theKey, strCiphertext, strOutput,
                                   pstrDisplay, pAlreadyHavePW);
}

// static
bool OTSymmetricKey::Decrypt(const OTSymmetricKey& theKey,
                             const String& strCiphertext, String& strOutput,
                             const String* pstrDisplay,
                             const OTPassword* pAlreadyHavePW)
{
    if (!theKey.IsGenerated()) {
        otWarn << __FUNCTION__
               << ": Failure: theKey.IsGenerated() was false. (The calling "
                  "code probably should have checked for that...)\n";
        return false;
    }

    OTASCIIArmor ascArmor;
    const bool bLoadedArmor = OTASCIIArmor::LoadFromString(
        ascArmor, strCiphertext); // str_bookend="-----BEGIN" by default

    if (!bLoadedArmor || !ascArmor.Exists()) {
        otErr << __FUNCTION__ << ": Failure loading ciphertext envelope:\n\n"
              << strCiphertext << "\n\n";
        return false;
    }

    // By this point, we know we have a ciphertext envelope and a symmetric Key.
    //
    std::unique_ptr<OTPassword> pPassUserInput;

    if (nullptr == pAlreadyHavePW) {
        const char* szDisplay = "Decrypting a password-protected ciphertext.";
        const String strDisplay((nullptr == pstrDisplay) ? szDisplay
                                                         : pstrDisplay->Get());

        pPassUserInput.reset(OTSymmetricKey::GetPassphraseFromUser(
            &strDisplay)); // bAskTwice=false by default.
    }

    bool bSuccess = false;

    if (pPassUserInput || // Success retrieving the passphrase from the
        pAlreadyHavePW)   // user, or passphrase was provided out of scope.
    {
        OTEnvelope theEnvelope(ascArmor);

        if (theEnvelope.Decrypt(strOutput, theKey, pPassUserInput
                                                       ? *pPassUserInput
                                                       : *pAlreadyHavePW)) {
            bSuccess = true;
        }
        else {
            otWarn << __FUNCTION__ << ": Failed trying to decrypt. (Sorry.)\n";
        }
    }
    else
        otWarn
            << __FUNCTION__
            << ": Sorry, unable to retrieve passphrase from user. (Failure.)\n";

    return bSuccess;
}

bool OTSymmetricKey::SerializeTo(String& strOutput, bool bEscaped) const
{
    OTASCIIArmor ascOutput;

    if (SerializeTo(ascOutput))
        return ascOutput.WriteArmoredString(strOutput, "SYMMETRIC KEY",
                                            bEscaped);

    return false;
}

bool OTSymmetricKey::SerializeFrom(const String& strInput, bool bEscaped)
{
    OTASCIIArmor ascInput;

    if (strInput.Exists() &&
        ascInput.LoadFromString(const_cast<String&>(strInput), bEscaped,
                                "-----BEGIN OT ARMORED SYMMETRIC KEY")) {
        return SerializeFrom(ascInput);
    }

    return false;
}

bool OTSymmetricKey::SerializeTo(OTASCIIArmor& ascOutput) const
{
    Data theOutput;

    if (SerializeTo(theOutput)) {
        ascOutput.SetData(theOutput);
        return true;
    }

    return false;
}

bool OTSymmetricKey::SerializeFrom(const OTASCIIArmor& ascInput)
{
    Data theInput;

    if (ascInput.Exists() && ascInput.GetData(theInput)) {
        return SerializeFrom(theInput);
    }
    return false;
}

bool OTSymmetricKey::SerializeTo(Data& theOutput) const
{

    uint16_t from_bool_is_generated = (m_bIsGenerated ? 1 : 0);
    uint16_t n_is_generated = htons(from_bool_is_generated);
    uint16_t n_key_size_bits = htons(static_cast<uint16_t>(m_nKeySize));

    uint32_t n_iteration_count =
        htonl(static_cast<uint32_t>(m_uIterationCount));

    uint32_t n_salt_size = htonl(m_dataSalt.GetSize());
    uint32_t n_iv_size = htonl(m_dataIV.GetSize());
    uint32_t n_enc_key_size = htonl(m_dataEncryptedKey.GetSize());
    uint32_t n_hash_check_size = htonl(m_dataHashCheck.GetSize());

    otLog5 << __FUNCTION__
           << ": is_generated: " << static_cast<int32_t>(ntohs(n_is_generated))
           << "   key_size_bits: "
           << static_cast<int32_t>(ntohs(n_key_size_bits))
           << "   iteration_count: "
           << static_cast<int64_t>(ntohl(n_iteration_count))
           << "   \n  "
              "salt_size: " << static_cast<int64_t>(ntohl(n_salt_size))
           << "   iv_size: " << static_cast<int64_t>(ntohl(n_iv_size))
           << "   enc_key_size: " << static_cast<int64_t>(ntohl(n_enc_key_size))
           << "   \n";

    theOutput.Concatenate(reinterpret_cast<void*>(&n_is_generated),
                          static_cast<uint32_t>(sizeof(n_is_generated)));

    theOutput.Concatenate(reinterpret_cast<void*>(&n_key_size_bits),
                          static_cast<uint32_t>(sizeof(n_key_size_bits)));

    theOutput.Concatenate(reinterpret_cast<void*>(&n_iteration_count),
                          static_cast<uint32_t>(sizeof(n_iteration_count)));

    theOutput.Concatenate(reinterpret_cast<void*>(&n_salt_size),
                          static_cast<uint32_t>(sizeof(n_salt_size)));

    OT_ASSERT(nullptr != m_dataSalt.GetPointer());
    theOutput.Concatenate(m_dataSalt.GetPointer(), m_dataSalt.GetSize());

    theOutput.Concatenate(reinterpret_cast<void*>(&n_iv_size),
                          static_cast<uint32_t>(sizeof(n_iv_size)));

    OT_ASSERT(nullptr != m_dataIV.GetPointer());
    theOutput.Concatenate(m_dataIV.GetPointer(), m_dataIV.GetSize());

    theOutput.Concatenate(reinterpret_cast<void*>(&n_enc_key_size),
                          static_cast<uint32_t>(sizeof(n_enc_key_size)));

    OT_ASSERT(nullptr != m_dataEncryptedKey.GetPointer());
    theOutput.Concatenate(m_dataEncryptedKey.GetPointer(),
                          m_dataEncryptedKey.GetSize());

    theOutput.Concatenate(reinterpret_cast<void*>(&n_hash_check_size),
                          static_cast<uint32_t>(sizeof(n_hash_check_size)));

    OT_ASSERT(nullptr != m_dataHashCheck.GetPointer());
    theOutput.Concatenate(m_dataHashCheck.GetPointer(),
                          m_dataHashCheck.GetSize());

    return true;
}

// Notice I don't theInput.reset(), because what if this
// key was being read from a larger file containing several
// keys?  I should just continue reading from the current
// position, and let the CALLER reset first, if that's his
// intention.
//
bool OTSymmetricKey::SerializeFrom(Data& theInput)
{
    const char* szFunc = "OTSymmetricKey::SerializeFrom";

    uint32_t nRead = 0;

    // Read network-order "is generated" flag. (convert to host order)
    //
    uint16_t n_is_generated = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<uint8_t*>(&n_is_generated),
                  static_cast<uint32_t>(sizeof(n_is_generated))))) {
        otErr << szFunc << ": Error reading n_is_generated.\n";
        return false;
    }

    // convert from network to HOST endian.
    //
    uint16_t host_is_generated = ntohs(n_is_generated);

    if (1 == host_is_generated)
        m_bIsGenerated = true;
    else if (0 == host_is_generated)
        m_bIsGenerated = false;
    else {
        otErr << szFunc << ": Error: host_is_generated, Bad value: "
              << static_cast<int32_t>(host_is_generated)
              << ". (Expected 0 or 1.)\n";
        return false;
    }

    otLog5 << __FUNCTION__
           << ": is_generated: " << static_cast<int32_t>(host_is_generated)
           << " \n";

    // Read network-order "key size in bits". (convert to host order)
    //
    uint16_t n_key_size_bits = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<uint8_t*>(&n_key_size_bits),
                  static_cast<uint32_t>(sizeof(n_key_size_bits))))) {
        otErr << szFunc << ": Error reading n_key_size_bits.\n";
        return false;
    }

    // convert from network to HOST endian.

    m_nKeySize = ntohs(n_key_size_bits);

    otLog5 << __FUNCTION__
           << ": key_size_bits: " << static_cast<int32_t>(m_nKeySize) << " \n";

    // Read network-order "iteration count". (convert to host order)
    //
    uint32_t n_iteration_count = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<uint8_t*>(&n_iteration_count),
                  static_cast<uint32_t>(sizeof(n_iteration_count))))) {
        otErr << szFunc << ": Error reading n_iteration_count.\n";
        return false;
    }
    OT_ASSERT(nRead == static_cast<uint32_t>(sizeof(n_iteration_count)));

    // convert from network to HOST endian.

    m_uIterationCount = ntohl(n_iteration_count);

    otLog5 << __FUNCTION__
           << ": iteration_count: " << static_cast<int64_t>(m_uIterationCount)
           << " \n";

    // Read network-order "salt size". (convert to host order)
    //
    uint32_t n_salt_size = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<uint8_t*>(&n_salt_size),
                  static_cast<uint32_t>(sizeof(n_salt_size))))) {
        otErr << szFunc << ": Error reading n_salt_size.\n";
        return false;
    }
    OT_ASSERT(nRead == static_cast<uint32_t>(sizeof(n_salt_size)));

    // convert from network to HOST endian.

    const uint32_t lSaltSize = ntohl(n_salt_size);

    otLog5 << __FUNCTION__
           << ": salt_size value: " << static_cast<int64_t>(lSaltSize) << " \n";

    // Then read the Salt itself.
    //
    m_dataSalt.SetSize(lSaltSize);

    if (0 == (nRead = theInput.OTfread(static_cast<uint8_t*>(const_cast<void*>(
                                           m_dataSalt.GetPointer())),
                                       static_cast<uint32_t>(lSaltSize)))) {
        otErr << szFunc << ": Error reading salt for symmetric key.\n";
        return false;
    }
    otLog5 << __FUNCTION__
           << ": salt length actually read: " << static_cast<int64_t>(nRead)
           << " \n";
    OT_ASSERT(nRead == static_cast<uint32_t>(lSaltSize));

    // Read network-order "IV size". (convert to host order)
    //
    uint32_t n_iv_size = 0;

    if (0 ==
        (nRead = theInput.OTfread(reinterpret_cast<uint8_t*>(&n_iv_size),
                                  static_cast<uint32_t>(sizeof(n_iv_size))))) {
        otErr << szFunc << ": Error reading n_iv_size.\n";
        return false;
    }

    OT_ASSERT(nRead == static_cast<uint32_t>(sizeof(n_iv_size)));

    // convert from network to HOST endian.

    const uint32_t lIVSize = ntohl(n_iv_size);

    otLog5 << __FUNCTION__
           << ": iv_size value: " << static_cast<int64_t>(lIVSize) << " \n";

    // Then read the IV itself.
    //
    m_dataIV.SetSize(lIVSize);

    if (0 == (nRead = theInput.OTfread(static_cast<uint8_t*>(const_cast<void*>(
                                           m_dataIV.GetPointer())),
                                       static_cast<uint32_t>(lIVSize)))) {
        otErr << szFunc << ": Error reading IV for symmetric key.\n";
        return false;
    }

    otLog5 << __FUNCTION__
           << ": iv length actually read: " << static_cast<int64_t>(nRead)
           << " \n";

    OT_ASSERT(nRead == static_cast<uint32_t>(lIVSize));

    // Read network-order "encrypted key size". (convert to host order)
    //
    uint32_t n_enc_key_size = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<uint8_t*>(&n_enc_key_size),
                  static_cast<uint32_t>(sizeof(n_enc_key_size))))) {
        otErr << szFunc << ": Error reading n_enc_key_size.\n";
        return false;
    }
    OT_ASSERT(nRead == static_cast<uint32_t>(sizeof(n_enc_key_size)));

    // convert from network to HOST endian.

    const uint32_t lEncKeySize = ntohl(n_enc_key_size);

    otLog5 << __FUNCTION__
           << ": enc_key_size value: " << static_cast<int64_t>(lEncKeySize)
           << " \n";

    // Then read the Encrypted Key itself.
    //
    m_dataEncryptedKey.SetSize(lEncKeySize);

    if (0 == (nRead = theInput.OTfread(static_cast<uint8_t*>(const_cast<void*>(
                                           m_dataEncryptedKey.GetPointer())),
                                       static_cast<uint32_t>(lEncKeySize)))) {
        otErr << szFunc << ": Error reading encrypted symmetric key.\n";
        return false;
    }

    otLog5 << __FUNCTION__ << ": encrypted key length actually read: "
           << static_cast<int64_t>(nRead) << " \n";

    OT_ASSERT(nRead == static_cast<uint32_t>(lEncKeySize));

    // Read network-order "hash check size". (convert to host order)
    //
    uint32_t n_hash_check_size = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<uint8_t*>(&n_hash_check_size),
                  static_cast<uint32_t>(sizeof(n_hash_check_size))))) {
        otErr << szFunc << ": Error reading n_hash_check_size.\n";
        otErr
            << szFunc
            << ": Looks like we don't have a hash check yet! (will make one)\n";
        m_bHasHashCheck = false;
        return false;
    }
    OT_ASSERT(nRead == static_cast<uint32_t>(sizeof(n_hash_check_size)));

    // convert from network to HOST endian.

    const uint32_t lHashCheckSize = ntohl(n_hash_check_size);

    otLog5 << __FUNCTION__ << ": hash_check_size value: "
           << static_cast<int64_t>(lHashCheckSize) << " \n";

    // Then read the Hashcheck itself.
    //
    m_dataHashCheck.SetSize(lHashCheckSize);

    if (0 ==
        (nRead = theInput.OTfread(static_cast<uint8_t*>(const_cast<void*>(
                                      m_dataHashCheck.GetPointer())),
                                  static_cast<uint32_t>(lHashCheckSize)))) {
        otErr << szFunc << ": Error reading hash check data.\n";
        return false;
    }

    otLog5 << __FUNCTION__
           << ": hash check data actually read: " << static_cast<int64_t>(nRead)
           << " \n";

    OT_ASSERT(nRead == static_cast<uint32_t>(lHashCheckSize));

    m_bHasHashCheck = !m_dataHashCheck.IsEmpty();

    return true;
}

OTSymmetricKey::OTSymmetricKey()
    : m_bIsGenerated(false)
    , m_bHasHashCheck(false)
    , m_nKeySize(CryptoConfig::SymmetricKeySize() * 8)
    , // 128 (in bits)
    m_uIterationCount(CryptoConfig::IterationCount())
{
}

OTSymmetricKey::OTSymmetricKey(const OTPassword& thePassword)
    : m_bIsGenerated(false)
    , m_bHasHashCheck(false)
    , m_nKeySize(CryptoConfig::SymmetricKeySize() * 8)
    , // 128 (in bits)
    m_uIterationCount(CryptoConfig::IterationCount())
{
    //  const bool bGenerated =
    GenerateKey(thePassword);
}

OTSymmetricKey::~OTSymmetricKey()
{
    Release_SymmetricKey();
}

void OTSymmetricKey::Release_SymmetricKey()
{
    m_bIsGenerated = false;
    m_uIterationCount = CryptoConfig::IterationCount();
    m_nKeySize = CryptoConfig::SymmetricKeySize() * 8; // 128 (in bits)

    m_dataSalt.Release();
    m_dataIV.Release();
    m_dataEncryptedKey.Release();
}

void OTSymmetricKey::Release()
{
    Release_SymmetricKey();

    // no call to ot_super::Release() here, since this is a base class
    // (currently with no children...)
}

} // namespace opentxs
