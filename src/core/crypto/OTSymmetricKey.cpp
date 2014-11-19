/************************************************************
 *
 *  OTSymmetricKey.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/crypto/OTSymmetricKey.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/Identifier.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>

#include <memory>

extern "C" {
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#endif
}

// for htons
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wold-style-cast"
#ifndef __clang__
#pragma GCC diagnostic warning "-Wuseless-cast"
#endif
#endif

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

    ot_data_t dataIV, dataSalt;

    // NOTE: I can't randomize the IV because then anything that was
    // encrypted with this key before, will fail to decrypt. (Ruining
    // the whole point of changing the passphrase...)
    //
    // UPDATE: I think this is false. I think the IV is for the encryption of
    // the symmetric key itself, whereas the content has its own IV in
    // OTEnvelope.
    //
    dataIV.resize(OTCryptoConfig::SymmetricIvSize());
    if (!OTCrypto::It()->RandomizeMemory(dataIV.data(), dataIV.size())) {
        otErr << __FUNCTION__ << ": Failed generating iv for changing "
                                 "passphrase on a symmetric key. (Returning "
                                 "false.)\n";
        return false;
    }

    dataSalt.resize(OTCryptoConfig::SymmetricIvSize());
    if (!OTCrypto::It()->RandomizeMemory(dataSalt.data(), dataSalt.size())) {
        otErr << __FUNCTION__ << ": Failed generating random salt for changing "
                                 "passphrase on a symmetric key. (Returning "
                                 "false.)\n";
        return false;
    }

    m_dataIV = dataIV;
    m_dataSalt = dataSalt;

    m_dataHashCheck.clear();
    m_dataEncryptedKey.clear();

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
    // Put the result into the OTData m_dataEncryptedKey.
    //
    const bool bEncryptedKey = OTCrypto::It()->Encrypt(
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

    m_dataIV.resize(OTCryptoConfig::SymmetricIvSize());
    if (!OTCrypto::It()->RandomizeMemory(m_dataIV.data(), m_dataIV.size())) {
        otErr << __FUNCTION__ << ": Failed generating iv for encrypting a "
                                 "symmetric key. (Returning false.)\n";
        return false;
    }

    m_dataSalt.resize(OTCryptoConfig::SymmetricIvSize());
    if (!OTCrypto::It()->RandomizeMemory(m_dataSalt.data(),
                                         m_dataSalt.size())) {
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
            theActualKey.randomizeMemory(OTCryptoConfig::SymmetricKeySize());
        if (0 > nRes) {
            OT_FAIL;
        }
        uint32_t uRes =
            static_cast<uint32_t>(nRes); // we need an uint32_t value.

        if (OTCryptoConfig::SymmetricKeySize() != uRes) {
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
    // Put the result into the OTData m_dataEncryptedKey.
    //
    const bool bEncryptedKey = OTCrypto::It()->Encrypt(
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

    OT_ASSERT(m_dataHashCheck.empty());

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
    //  OT_ASSERT(m_bIsGenerated);
    //  OT_ASSERT(thePassphrase.isPassword());
    OTPassword* pDerivedKey = nullptr;

    ot_data_t tmpDataHashCheck = m_dataHashCheck;

    if (bCheckForHashCheck) {
        if (!HasHashCheck()) {
            otErr << __FUNCTION__ << ": Unable to calculate derived key, as "
                                     "hash check is missing!";
            OT_FAIL;
        }
        OT_ASSERT(!tmpDataHashCheck.empty());
    }
    else {
        if (!HasHashCheck()) {
            otOut << __FUNCTION__ << ": Warning!! No hash check, ignoring... "
                                     "(since bCheckForHashCheck was set false)";
            OT_ASSERT(tmpDataHashCheck.empty());
        }
    }

    pDerivedKey = OTCrypto::It()->DeriveNewKey(
        thePassphrase, m_dataSalt, m_uIterationCount, tmpDataHashCheck);

    return pDerivedKey; // can be null
}

// CALLER IS RESPONSIBLE TO DELETE.
OTPassword* OTSymmetricKey::CalculateNewDerivedKeyFromPassphrase(
    const OTPassword& thePassphrase)
{
    //  OT_ASSERT(m_bIsGenerated);
    //  OT_ASSERT(thePassphrase.isPassword());
    OTPassword* pDerivedKey = nullptr;

    if (!HasHashCheck()) {
        m_dataHashCheck.clear();

        pDerivedKey = OTCrypto::It()->DeriveNewKey(
            thePassphrase, m_dataSalt, m_uIterationCount, m_dataHashCheck);
    }
    else {
        otErr << __FUNCTION__
              << ": Calling Wrong function!! Hash check already exists!";
    }

    OT_ASSERT(nullptr != pDerivedKey);
    OT_ASSERT(!m_dataHashCheck.empty());

    return pDerivedKey;
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
    //  OT_ASSERT(thePassphrase.isPassword());

    std::unique_ptr<OTPassword> theDerivedAngel;

    if (nullptr == pDerivedKey) {
        // todo, security: Do we have to create all these OTPassword objects on
        // the stack, just
        // as a general practice? In which case I can't use this factory how I'm
        // using it now...
        //

        pDerivedKey = CalculateDerivedKeyFromPassphrase(
            thePassphrase, false); // asserts already.

        theDerivedAngel.reset(pDerivedKey);
    }
    // Below this point, pDerivedKey is NOT null. And we only clean it up if we
    // created it.

    //
    // Below this point, pDerivedKey contains a derived symmetric key, from the
    // salt, the iteration
    // count, and the password that was passed in. The salt and iteration count
    // were both stored inside this
    // OTSymmetricKey object since this key was originally generated, and we
    // store an encrypted copy of the
    // ActualKey already, as well-- it's encrypted to the Derived Key. (We also
    // store the IV from that
    // encryption bit.)
    //
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

    const bool bDecryptedKey = OTCrypto::It()->Decrypt(
        theDerivedKey, // We're using theDerivedKey to decrypt
                       // m_dataEncryptedKey.

        // Here's what we're trying to decrypt: the encrypted
        // form of the symmetric key.
        //
        reinterpret_cast<const char*>(
            m_dataEncryptedKey.data()), // The Ciphertext.
        m_dataEncryptedKey.size(),
        m_dataIV, // Created when *this symmetric key was generated. Both are
                  // already stored.
        theRawKeyOutput); // OUTPUT. (Recovered plaintext of symmetric key.) You
                          // can pass OTPassword& OR OTData& here (either
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
    thePWData.setUsingOldSystem(); // So the cached key doesn't interfere, since
                                   // this is for a plain symmetric key.

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
        pPassUserInput.reset(const_cast<OTPassword*>(pAlreadyHavePW));

    bool bSuccess = false;

    if (nullptr != pPassUserInput) // Success retrieving the passphrase from the
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
        pPassUserInput.reset(const_cast<OTPassword*>(pAlreadyHavePW));

    OTASCIIArmor ascOutput;
    bool bSuccess = false;

    if (nullptr != pPassUserInput) // Success retrieving the passphrase from the
                                   // user. (Now let's encrypt...)
    {
        OTEnvelope theEnvelope;

        if (theEnvelope.Encrypt(strPlaintext,
                                const_cast<OTSymmetricKey&>(theKey),
                                *pPassUserInput) &&
            theEnvelope.GetAsciiArmoredData(ascOutput)) {
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
    ot_data_t theOutput;

    if (SerializeTo(theOutput)) {
        ascOutput.SetData(theOutput);
        return true;
    }

    return false;
}

bool OTSymmetricKey::SerializeFrom(const OTASCIIArmor& ascInput)
{
    ot_data_t theInput;

    if (ascInput.Exists() && ascInput.GetData(theInput)) {
        return SerializeFrom(theInput);
    }
    return false;
}

bool OTSymmetricKey::SerializeTo(ot_data_t& theOutput) const
{
    theOutput.clear();

    // Is Generated
    OTData::appendData<uint16_t>(htons(m_bIsGenerated ? 1 : 0), theOutput);

    // Key Size (bits)
    OTData::appendData<uint16_t>(htons(m_nKeySize), theOutput);

    // Iteration Count
    OTData::appendData<uint32_t>(htonl(m_uIterationCount), theOutput);

    // Salt
    OTData::appendData<uint32_t>(htonl(m_dataSalt.size()), theOutput);
    theOutput.insert(theOutput.end(), m_dataSalt.begin(), m_dataSalt.end());

    // IV
    OTData::appendData<uint32_t>(htonl(m_dataIV.size()), theOutput);
    theOutput.insert(theOutput.end(), m_dataIV.begin(), m_dataIV.end());

    // Encrypted Key
    OTData::appendData<uint32_t>(htonl(m_dataEncryptedKey.size()), theOutput);
    theOutput.insert(theOutput.end(), m_dataEncryptedKey.begin(),
                     m_dataEncryptedKey.end());

    // Check
    OTData::appendData<uint32_t>(htonl(m_dataHashCheck.size()), theOutput);
    theOutput.insert(theOutput.end(), m_dataHashCheck.begin(),
                     m_dataHashCheck.end());

    return true;
}

bool OTSymmetricKey::SerializeFrom(ot_data_t& theInput)
{
    auto input_it = theInput.cbegin();

    // Is Generated
    auto is_generated =
        ntohs(OTData::readData<uint16_t>(&input_it, theInput.end()));

    if (1 == is_generated)
        m_bIsGenerated = true;
    else if (0 == is_generated)
        m_bIsGenerated = false;
    else {
        otErr << __FUNCTION__ << ": Error: host_is_generated, Bad value: "
              << static_cast<int32_t>(is_generated) << ". (Expected 0 or 1.)\n";
        return false;
    }

    // Key Size (bits)
    m_nKeySize = ntohs(OTData::readData<uint16_t>(&input_it, theInput.end()));
    if (0 >= m_nKeySize) return false;

    // Iteration Count
    m_uIterationCount =
        ntohl(OTData::readData<uint32_t>(&input_it, theInput.end()));

    // Salt
    m_dataSalt.resize(
        ntohl(OTData::readData<uint32_t>(&input_it, theInput.end())));
    OTData::readDataVector(&input_it, theInput.end(), m_dataSalt);

    // IV
    m_dataIV.resize(
        ntohl(OTData::readData<uint32_t>(&input_it, theInput.end())));
    OTData::readDataVector(&input_it, theInput.end(), m_dataIV);

    // Encrypted Key
    m_dataEncryptedKey.resize(
        ntohl(OTData::readData<uint32_t>(&input_it, theInput.end())));
    OTData::readDataVector(&input_it, theInput.end(), m_dataEncryptedKey);

    // Check
    m_dataHashCheck.resize(
        ntohl(OTData::readData<uint32_t>(&input_it, theInput.end())));
    OTData::readDataVector(&input_it, theInput.end(), m_dataHashCheck);

    return true;
}

OTSymmetricKey::OTSymmetricKey()
    : m_bIsGenerated(false)
    , m_nKeySize(OTCryptoConfig::SymmetricKeySize() * 8)
    , // 128 (in bits)
    m_uIterationCount(OTCryptoConfig::IterationCount())
{
}

OTSymmetricKey::OTSymmetricKey(const OTPassword& thePassword)
    : m_bIsGenerated(false)
    , m_nKeySize(OTCryptoConfig::SymmetricKeySize() * 8)
    , // 128 (in bits)
    m_uIterationCount(OTCryptoConfig::IterationCount())
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
    m_uIterationCount = OTCryptoConfig::IterationCount();
    m_nKeySize = OTCryptoConfig::SymmetricKeySize() * 8; // 128 (in bits)

    m_dataSalt.clear();
    m_dataIV.clear();
    m_dataEncryptedKey.clear();
}

void OTSymmetricKey::Release()
{
    Release_SymmetricKey();

    // no call to ot_super::Release() here, since this is a base class
    // (currently with no children...)
}

} // namespace opentxs
