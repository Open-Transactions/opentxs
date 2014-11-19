/************************************************************
 *
 *  OTEnvelope.cpp
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

#include <opentxs/core/crypto/OTEnvelope.hpp>

#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/crypto/OTSymmetricKey.hpp>

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

// Presumably this Envelope contains encrypted data (in binary form.)
// If you would like an ASCII-armored version of that data, just call this
// function.
// Should be called "Get Binary Envelope Encrypted Contents Into Ascii-Armored
// Form"
//
bool OTEnvelope::GetAsciiArmoredData(OTASCIIArmor& theArmoredText,
                                     bool bLineBreaks) const
{
    return theArmoredText.SetData(m_dataContents, bLineBreaks);
}

// Should be called "Set This Envelope's binary ciphertext data, from an
// ascii-armored input string."
//
// Let's say you just retrieved the ASCII-armored contents of an encrypted
// envelope.
// Perhaps someone sent it to you, and you just read it out of his message.
// And let's say you want to get those contents back into binary form in an
// Envelope object again, so that they can be decrypted and extracted back as
// plaintext. Fear not, just call this function.
//
bool OTEnvelope::SetAsciiArmoredData(const OTASCIIArmor& theArmoredText,
                                     bool bLineBreaks)
{
    return theArmoredText.GetData(m_dataContents, bLineBreaks);
}

bool OTEnvelope::GetAsBookendedString(
    String& strArmorWithBookends, // output (if successful.)
    bool bEscaped) const
{
    OTASCIIArmor theArmoredText;
    // This function will base64 ENCODE m_dataContents, and then
    // Set() that as the string contents on theArmoredText.
    const bool bSetData = theArmoredText.SetData(
        m_dataContents, true); // bLineBreaks=true (by default anyway.)

    if (bSetData) {
        const bool bWritten = theArmoredText.WriteArmoredString(
            strArmorWithBookends, "ENVELOPE", // todo hardcoded
            bEscaped);
        if (!bWritten)
            otErr << __FUNCTION__ << ": Failed while calling: "
                                     "theArmoredText.WriteArmoredString\n";
        else
            return true;
    }
    else
        otErr << __FUNCTION__
              << ": Failed while calling: "
                 "theArmoredText.SetData(m_dataContents, true)\n";

    return false;
}

bool OTEnvelope::SetFromBookendedString(
    const String& strArmorWithBookends, // input
    bool bEscaped)
{
    OTASCIIArmor theArmoredText;
    const bool bLoaded = theArmoredText.LoadFromString(
        const_cast<String&>(strArmorWithBookends),
        bEscaped); // std::string str_override="-----BEGIN");

    if (bLoaded) {
        // This function will base64 DECODE theArmoredText's string contents
        // and return them as binary in m_dataContents
        const bool bGotData =
            theArmoredText.GetData(m_dataContents, true); // bLineBreaks = true

        if (!bGotData)
            otErr << __FUNCTION__ << ": Failed while calling: "
                                     "theArmoredText.GetData\n";
        else
            return true;
    }
    else
        otErr << __FUNCTION__ << ": Failed while calling: "
                                 "theArmoredText.LoadFromString\n";

    return false;
}

// Encrypt theInput as envelope using symmetric crypto, using a random AES key
// that's
// kept encrypted in an OTSymmetricKey (encrypted using another key derived from
// thePassword.)

bool OTEnvelope::Encrypt(const String& theInput, OTSymmetricKey& theKey,
                         const OTPassword& thePassword)
{
    OT_ASSERT(
        (thePassword.isPassword() && (thePassword.getPasswordSize() > 0)) ||
        (thePassword.isMemory() && (thePassword.getMemorySize() > 0)));
    OT_ASSERT(theInput.Exists());

    // Generate a random initialization vector.
    //
    OTData theIV;

    if (!theIV.Randomize(OTCryptoConfig::SymmetricIvSize())) {
        otErr << __FUNCTION__ << ": Failed trying to randomly generate IV.\n";
        return false;
    }

    // If the symmetric key hasn't already been generated, we'll just do that
    // now...
    // (The passphrase is used to derive another key that is used to encrypt the
    // actual symmetric key, and to access it later.)
    //
    if ((false == theKey.IsGenerated()) &&
        (false == theKey.GenerateKey(thePassword))) {
        otErr << __FUNCTION__
              << ": Failed trying to generate symmetric key using password.\n";
        return false;
    }

    if (!theKey.HasHashCheck()) {
        if (!theKey.GenerateHashCheck(thePassword)) {
            otErr << __FUNCTION__
                  << ": Failed trying to generate hash check using password.\n";
            return false;
        }
    }

    OT_ASSERT(theKey.HasHashCheck());

    OTPassword theRawSymmetricKey;

    if (false ==
        theKey.GetRawKeyFromPassphrase(thePassword, theRawSymmetricKey)) {
        otErr << __FUNCTION__ << ": Failed trying to retrieve raw symmetric "
                                 "key using password.\n";
        return false;
    }

    OTData theCipherText;

    const bool bEncrypted = OTCrypto::It()->Encrypt(
        theRawSymmetricKey,       // The symmetric key, in clear form.
        theInput.Get(),           // This is the Plaintext.
        theInput.GetLength() + 1, // for null terminator
        theIV,                    // Initialization vector.
        theCipherText);           // OUTPUT. (Ciphertext.)

    //
    // Success?
    //
    if (!bEncrypted) {
        otErr << __FUNCTION__ << ": (static) call failed to encrypt. Wrong "
                                 "key? (Returning false.)\n";
        return false;
    }

    // This is where the envelope final contents will be placed,
    // including the envelope type, the size of the IV, the IV
    // itself, and the ciphertext.
    //
    m_dataContents.Release();

    // Write the ENVELOPE TYPE (network order version.)
    //
    // 0 == Error
    // 1 == Asymmetric Key  (other functions -- Seal / Open.)
    // 2 == Symmetric Key   (this function -- Encrypt / Decrypt.)
    // Anything else: error.

    // Calculate "network-order" version of envelope type 2.
    uint16_t env_type_n = htons(static_cast<uint16_t>(2));

    m_dataContents.Concatenate(reinterpret_cast<void*>(&env_type_n),
                               // (uint32_t here is the 2nd parameter to
                               // Concatenate, and has nothing to do with
                               // env_type_n being uint16_t)
                               static_cast<uint32_t>(sizeof(env_type_n)));

    // Write IV size (in network-order)
    //
    uint32_t ivlen =
        OTCryptoConfig::SymmetricIvSize(); // Length of IV for this cipher...
    OT_ASSERT(ivlen >= theIV.GetSize());
    uint32_t ivlen_n = htonl(
        theIV.GetSize()); // Calculate "network-order" version of iv length.

    m_dataContents.Concatenate(reinterpret_cast<void*>(&ivlen_n),
                               static_cast<uint32_t>(sizeof(ivlen_n)));

    // Write the IV itself.
    //
    m_dataContents.Concatenate(theIV.GetPointer(), theIV.GetSize());

    // Write the Ciphertext.
    //
    m_dataContents.Concatenate(theCipherText.GetPointer(),
                               theCipherText.GetSize());

    // We don't write the size of the ciphertext before the ciphertext itself,
    // since the decryption is able to deduce the size based on the total
    // envelope
    // size minus the other pieces. We might still want to add that size here,
    // however.
    // (for security / safety reasons.)

    return true;
}

bool OTEnvelope::Decrypt(String& theOutput, const OTSymmetricKey& theKey,
                         const OTPassword& thePassword)
{
    const char* szFunc = "OTEnvelope::Decrypt";

    OT_ASSERT(
        (thePassword.isPassword() && (thePassword.getPasswordSize() > 0)) ||
        (thePassword.isMemory() && (thePassword.getMemorySize() > 0)));
    OT_ASSERT(theKey.IsGenerated());

    OTPassword theRawSymmetricKey;

    if (false ==
        theKey.GetRawKeyFromPassphrase(thePassword, theRawSymmetricKey)) {
        otErr << szFunc << ": Failed trying to retrieve raw symmetric key "
                           "using password. (Wrong password?)\n";
        return false;
    }

    uint32_t nRead = 0;
    uint32_t nRunningTotal = 0;

    m_dataContents.reset(); // Reset the fread position on this object to 0.

    //
    // Read the ENVELOPE TYPE (as network order version -- and convert to host
    // version.)
    //
    // 0 == Error
    // 1 == Asymmetric Key  (this function -- Seal / Open)
    // 2 == Symmetric Key   (other functions -- Encrypt / Decrypt use this.)
    // Anything else: error.
    //
    uint16_t env_type_n = 0;

    if (0 == (nRead = m_dataContents.OTfread(
                  reinterpret_cast<uint8_t*>(&env_type_n),
                  static_cast<uint32_t>(sizeof(env_type_n))))) {
        otErr << szFunc << ": Error reading Envelope Type. Expected "
                           "asymmetric(1) or symmetric (2).\n";
        return false;
    }
    nRunningTotal += nRead;
    OT_ASSERT(nRead == static_cast<uint32_t>(sizeof(env_type_n)));

    // convert that envelope type from network to HOST endian.
    //
    const uint16_t env_type = ntohs(env_type_n);
    //  nRunningTotal += env_type;    // NOPE! Just because envelope type is 1
    // or 2, doesn't mean we add 1 or 2 extra bytes to the length here. Nope!

    if (2 != env_type) {
        const uint32_t l_env_type = static_cast<uint32_t>(env_type);
        otErr << szFunc << ": Error: Expected Envelope for Symmetric key (type "
                           "2) but instead found type: " << l_env_type << ".\n";
        return false;
    }

    // Read network-order IV size (and convert to host version)
    //
    const uint32_t max_iv_length =
        OTCryptoConfig::SymmetricIvSize(); // I believe this is a max length, so
                                           // it may not match the actual length
                                           // of the IV.

    // Read the IV SIZE (network order version -- convert to host version.)
    //
    uint32_t iv_size_n = 0;

    if (0 == (nRead = m_dataContents.OTfread(
                  reinterpret_cast<uint8_t*>(&iv_size_n),
                  static_cast<uint32_t>(sizeof(iv_size_n))))) {
        otErr << szFunc << ": Error reading IV Size.\n";
        return false;
    }
    nRunningTotal += nRead;
    OT_ASSERT(nRead == static_cast<uint32_t>(sizeof(iv_size_n)));

    // convert that iv size from network to HOST endian.
    //
    const uint32_t iv_size_host_order = ntohl(iv_size_n);

    if (iv_size_host_order > max_iv_length) {
        otErr << szFunc << ": Error: iv_size ("
              << static_cast<int64_t>(iv_size_host_order)
              << ") is larger than max_iv_length ("
              << static_cast<int64_t>(max_iv_length) << ").\n";
        return false;
    }
    //  nRunningTotal += iv_size_host_order; // Nope!

    // Then read the IV (initialization vector) itself.
    //
    OTData theIV;
    theIV.SetSize(iv_size_host_order);

    if (0 == (nRead = m_dataContents.OTfread(
                  static_cast<uint8_t*>(const_cast<void*>(theIV.GetPointer())),
                  static_cast<uint32_t>(iv_size_host_order)))) {
        otErr << szFunc << ": Error reading initialization vector.\n";
        return false;
    }
    nRunningTotal += nRead;
    OT_ASSERT(nRead == static_cast<uint32_t>(iv_size_host_order));

    OT_ASSERT(nRead <= max_iv_length);

    // We create an OTData object to store the ciphertext itself, which
    // begins AFTER the end of the IV.
    // So we see pointer + nRunningTotal as the starting point for the
    // ciphertext.
    // the size of the ciphertext, meanwhile, is the size of the entire thing,
    // MINUS nRunningTotal.
    //
    OTData theCipherText(
        static_cast<const void*>(
            static_cast<const uint8_t*>(m_dataContents.GetPointer()) +
            nRunningTotal),
        m_dataContents.GetSize() - nRunningTotal);

    // Now we've got all the pieces together, let's try to decrypt it...
    //
    OTData thePlaintext; // for output.

    const bool bDecrypted = OTCrypto::It()->Decrypt(
        theRawSymmetricKey, // The symmetric key, in clear form.
        static_cast<const char*>(
            theCipherText.GetPointer()), // This is the Ciphertext.
        theCipherText.GetSize(),
        theIV, thePlaintext); // OUTPUT. (Recovered plaintext.) You can pass
                              // OTPassword& OR OTData& here (either will
                              // work.)

    // theOutput is where we'll put the decrypted data.
    //
    theOutput.Release();

    if (bDecrypted) {

        // Make sure it's null-terminated...
        //
        uint32_t nIndex = thePlaintext.GetSize() - 1;
        (static_cast<uint8_t*>(
            const_cast<void*>(thePlaintext.GetPointer())))[nIndex] = '\0';

        // Set it into theOutput (to return the plaintext to the caller)
        //
        theOutput.Set(static_cast<const char*>(thePlaintext.GetPointer()));
    }

    return bDecrypted;
}

// RSA / AES

bool OTEnvelope::Seal(const OTPseudonym& theRecipient, const String& theInput)
{
    String strNymID;
    mapOfAsymmetricKeys theKeys;
    theRecipient.GetIdentifier(strNymID);
    theKeys.insert(std::pair<std::string, OTAsymmetricKey*>(
        strNymID.Get(),
        const_cast<OTAsymmetricKey*>(&(theRecipient.GetPublicEncrKey()))));

    return Seal(theKeys, theInput);
}

bool OTEnvelope::Seal(setOfNyms& theRecipients, const String& theInput)
{
    mapOfAsymmetricKeys RecipPubKeys;

    // Loop through theRecipients, and add the public key of each one to a set
    // of keys.
    for (auto& it : theRecipients) {
        OTPseudonym* pNym = it;
        OT_ASSERT_MSG(nullptr != pNym,
                      "OTEnvelope::Seal: Assert: nullptr pseudonym pointer.");

        String strNymID;
        pNym->GetIdentifier(strNymID);
        RecipPubKeys.insert(std::pair<std::string, OTAsymmetricKey*>(
            strNymID.Get(),
            const_cast<OTAsymmetricKey*>(&(pNym->GetPublicEncrKey()))));
    }

    if (RecipPubKeys.empty()) return false;

    return Seal(RecipPubKeys, theInput);
}

bool OTEnvelope::Seal(const OTAsymmetricKey& RecipPubKey,
                      const String& theInput)
{
    mapOfAsymmetricKeys theKeys;
    theKeys.insert(std::pair<std::string, OTAsymmetricKey*>(
        "", // Normally the NymID goes here, but we don't know what it is, in
            // this case.
        const_cast<OTAsymmetricKey*>(&RecipPubKey)));

    return Seal(theKeys, theInput);
}

// Seal up as envelope (Asymmetric, using public key and then AES key.)

bool OTEnvelope::Seal(mapOfAsymmetricKeys& RecipPubKeys, const String& theInput)
{
    OT_ASSERT_MSG(!RecipPubKeys.empty(),
                  "OTEnvelope::Seal: ASSERT: RecipPubKeys.size() > 0");

    return OTCrypto::It()->Seal(RecipPubKeys, theInput, m_dataContents);
}

// RSA / AES

bool OTEnvelope::Open(const OTPseudonym& theRecipient, String& theOutput,
                      const OTPasswordData* pPWData)
{
    return OTCrypto::It()->Open(m_dataContents, theRecipient, theOutput,
                                pPWData);
}

// DONE: Fix OTEnvelope so we can seal to multiple recipients simultaneously.
// DONE: Fix OTEnvelope so it supports symmetric crypto as well as asymmetric.

// DONE: Remove the Nym stored inside the purse, and replace with a
// session key, just as envelopes will support a session key.

// TODO: Make sure OTEnvelope / OTCrypto_OpenSSL is safe with zeroing memory
// wherever needed.

// Todo: Once envelopes support multiple recipient Nyms, then make a habit of
// encrypting
// to the user's key AND server's key, when sending.

// Hmm this might be better than a session key, since we don't have to worry
// about keeping track
// of the session key for LATER, since envelopes generate a session key already.
// BUT: That means we do it already, and that means we wouldn't get any speed
// benefit.
// Transport protocol should have session key already built-in -- hmm what if
// going over email or
// some insecure channel?
// Solution: Make it always encrypted to public key (as it already is now) with
// session key automatically
// (as already) by virtue of using OpenSSL envelope. This will, of course,
// generate a new session key for
// EACH envelope, so we will STILL add the protocol of initiating sessions,
// purely to reduce CPU cycles
// during each session. This means we'll have the same protocol as before but
// just faster (in a way.)
//

// We just read some encrypted (and armored) data, and we want to put it in
// an envelope so that it can be opened. So we can just directly set the
// armored string here, and it will be decoded into the original binary,
// inside this envelope. That way we can decrypt it (symmetric), or open it
// (asymmetric) and get the original plaintext that was sent.
//
OTEnvelope::OTEnvelope(const OTASCIIArmor& theArmoredText)
{
    SetAsciiArmoredData(theArmoredText);
}

OTEnvelope::OTEnvelope()
{
}

OTEnvelope::~OTEnvelope()
{
}

} // namespace opentxs
