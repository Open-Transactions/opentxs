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

#include "opentxs/core/crypto/OTEnvelope.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/Crypto.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/Letter.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
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
bool OTEnvelope::GetCiphertext(OTASCIIArmor& theArmoredText) const
{
    if (!ciphertext_) { return false; }

    return theArmoredText.SetData(*ciphertext_, true);
}

bool OTEnvelope::SetCiphertext(const OTASCIIArmor& theArmoredText)
{
    ciphertext_.reset(new Data);

    OT_ASSERT(ciphertext_);

    return theArmoredText.GetData(*ciphertext_, true);
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
    Data theIV;

    if (!theIV.Randomize(CryptoConfig::SymmetricIvSize())) {
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

    Data theCipherText;

    const bool bEncrypted = OT::App().Crypto().AES().Encrypt(
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
    ciphertext_.reset(new Data);

    OT_ASSERT(ciphertext_);

    // Write the ENVELOPE TYPE (network order version.)
    //
    // 0 == Error
    // 1 == Asymmetric Key  (other functions -- Seal / Open.)
    // 2 == Symmetric Key   (this function -- Encrypt / Decrypt.)
    // Anything else: error.

    // Calculate "network-order" version of envelope type 2.
    uint16_t env_type_n = htons(static_cast<uint16_t>(2));

    ciphertext_->Concatenate(reinterpret_cast<void*>(&env_type_n),
                               // (uint32_t here is the 2nd parameter to
                               // Concatenate, and has nothing to do with
                               // env_type_n being uint16_t)
                               static_cast<uint32_t>(sizeof(env_type_n)));

    // Write IV size (in network-order)
    //
    uint32_t ivlen =
        CryptoConfig::SymmetricIvSize(); // Length of IV for this cipher...
    OT_ASSERT(ivlen >= theIV.GetSize());
    uint32_t ivlen_n = htonl(
        theIV.GetSize()); // Calculate "network-order" version of iv length.

    ciphertext_->Concatenate(reinterpret_cast<void*>(&ivlen_n),
                               static_cast<uint32_t>(sizeof(ivlen_n)));

    // Write the IV itself.
    //
    ciphertext_->Concatenate(theIV.GetPointer(), theIV.GetSize());

    // Write the Ciphertext.
    //
    ciphertext_->Concatenate(theCipherText.GetPointer(),
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

    OT_ASSERT(ciphertext_);

    ciphertext_->reset();    // Reset the fread position on this object to 0.

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

    if (0 == (nRead = ciphertext_->OTfread(
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
        CryptoConfig::SymmetricIvSize(); // I believe this is a max length, so
                                           // it may not match the actual length
                                           // of the IV.

    // Read the IV SIZE (network order version -- convert to host version.)
    //
    uint32_t iv_size_n = 0;

    if (0 == (nRead = ciphertext_->OTfread(
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
    Data theIV;
    theIV.SetSize(iv_size_host_order);

    if (0 == (nRead = ciphertext_->OTfread(
                  static_cast<uint8_t*>(const_cast<void*>(theIV.GetPointer())),
                  static_cast<uint32_t>(iv_size_host_order)))) {
        otErr << szFunc << ": Error reading initialization vector.\n";
        return false;
    }
    nRunningTotal += nRead;
    OT_ASSERT(nRead == static_cast<uint32_t>(iv_size_host_order));

    OT_ASSERT(nRead <= max_iv_length);

    // We create an Data object to store the ciphertext itself, which
    // begins AFTER the end of the IV.
    // So we see pointer + nRunningTotal as the starting point for the
    // ciphertext.
    // the size of the ciphertext, meanwhile, is the size of the entire thing,
    // MINUS nRunningTotal.
    //
    Data theCipherText(
        static_cast<const void*>(
            static_cast<const uint8_t*>(ciphertext_->GetPointer()) +
            nRunningTotal),
        ciphertext_->GetSize() - nRunningTotal);

    // Now we've got all the pieces together, let's try to decrypt it...
    //
    Data thePlaintext; // for output.
    CryptoSymmetricDecryptOutput plaintext(thePlaintext);

    const bool bDecrypted = OT::App().Crypto().AES().Decrypt(
        theRawSymmetricKey, // The symmetric key, in clear form.
        static_cast<const char*>(
            theCipherText.GetPointer()), // This is the Ciphertext.
        theCipherText.GetSize(),
        theIV, plaintext); // OUTPUT. (Recovered plaintext.) You can pass
                              // OTPassword& OR Data& here (either will
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

EXPORT bool OTEnvelope::Seal(const setOfNyms& recipients,
                 const String& theInput)
{
    mapOfAsymmetricKeys recipientKeys;

    for (auto& it : recipients) {
        recipientKeys.insert(std::pair<std::string, OTAsymmetricKey*>(
            "",const_cast<OTAsymmetricKey*>(&(it->GetPublicEncrKey()))));
    }

    if (!recipientKeys.empty()) {
        return Seal(recipientKeys, theInput);
    } else {
        return false;
    }
}

bool OTEnvelope::Seal(const Nym& theRecipient, const String& theInput)
{
    return Seal(theRecipient.GetPublicEncrKey(), theInput);
}

bool OTEnvelope::Seal(const OTAsymmetricKey& RecipPubKey,
                      const String& theInput)
{
    mapOfAsymmetricKeys recipientKeys;
    recipientKeys.insert(std::pair<std::string, OTAsymmetricKey*>(
        "",const_cast<OTAsymmetricKey*>(&RecipPubKey)));

    return Seal(recipientKeys, theInput);
}

bool OTEnvelope::Seal(const mapOfAsymmetricKeys& recipientKeys,
                      const String& theInput)
{
    OT_ASSERT_MSG(!recipientKeys.empty(),
                  "OTEnvelope::Seal: ASSERT: RecipPubKeys.size() > 0");

    ciphertext_.reset(new Data);

    OT_ASSERT(ciphertext_);

    return Letter::Seal(recipientKeys, theInput, *ciphertext_);
}

bool OTEnvelope::Open(
    const Nym& theRecipient,
    String& theOutput,
    const OTPasswordData* pPWData)
{
    if (!ciphertext_) { return false; }

    if (nullptr == pPWData) {
        OTPasswordData password("Decrypt this document.");

        return Letter::Open(*ciphertext_, theRecipient, password, theOutput);
    } else {
        return Letter::Open(*ciphertext_, theRecipient, *pPWData, theOutput);
    }
}

OTEnvelope::OTEnvelope(const OTASCIIArmor& theArmoredText)
{
    SetCiphertext(theArmoredText);
}
} // namespace opentxs
