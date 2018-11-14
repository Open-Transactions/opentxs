// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/crypto/key/LegacySymmetric.hpp"

#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/CryptoSymmetricDecryptOutput.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/library/LegacySymmetricProvider.hpp"
#include "opentxs/OT.hpp"

#include "internal/api/Internal.hpp"
#include "LegacySymmetricNull.hpp"

extern "C" {
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#endif
}

#include <atomic>
#include <cstdint>
#include <ostream>

#include "LegacySymmetric.hpp"

#define OT_METHOD "opentxs::crypto::key::LegacySymmetric::"

namespace opentxs::crypto::key
{
OTLegacySymmetricKey LegacySymmetric::Blank()
{
    return OTLegacySymmetricKey{new implementation::LegacySymmetricNull};
}

OTLegacySymmetricKey LegacySymmetric::Factory(const api::Crypto& crypto)
{
    return OTLegacySymmetricKey{new implementation::LegacySymmetric(crypto)};
}

OTLegacySymmetricKey LegacySymmetric::Factory(
    const api::Crypto& crypto,
    const OTPassword& thePassword)
{
    return OTLegacySymmetricKey{
        new implementation::LegacySymmetric(crypto, thePassword)};
}

bool LegacySymmetric::CreateNewKey(
    const api::Crypto& crypto,
    String& strOutput,
    const String& pstrDisplay,
    const OTPassword* pAlreadyHavePW)
{
    std::unique_ptr<OTPassword> pPassUserInput;

    if (nullptr == pAlreadyHavePW) {
        const char* szDisplay = "Creating new symmetric key.";
        const auto strDisplay = String::Factory(
            (!pstrDisplay.Exists()) ? szDisplay : pstrDisplay.Get());

        pPassUserInput.reset(GetPassphraseFromUser(
            strDisplay, true));  // bAskTwice=false by default.
    } else
        pPassUserInput.reset(new OTPassword(*pAlreadyHavePW));

    bool bSuccess = false;

    if (pPassUserInput)  // Success retrieving the passphrase from the
                         // user. (Now let's generate the key...)
    {
        LogDebug(OT_METHOD)(__FUNCTION__)(
            ": Calling LegacySymmetric theKey.GenerateKey()...")
            .Flush();
        implementation::LegacySymmetric theKey(crypto, *pPassUserInput);
        const bool bGenerated = theKey.IsGenerated();

        if (bGenerated && theKey.SerializeTo(strOutput))
            bSuccess = true;
        else
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Sorry, unable to generate key. (Failure).")
                .Flush();
    } else
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Sorry, unable to retrieve password from user. (Failure).")
            .Flush();

    return bSuccess;
}

bool LegacySymmetric::Decrypt(
    const api::Crypto& crypto,
    const String& strKey,
    String& strCiphertext,
    String& strOutput,
    const String& pstrDisplay,
    const OTPassword* pAlreadyHavePW)
{

    if (!strKey.Exists()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Nonexistent: The symmetric key. Please supply. (Failure).")
            .Flush();
        return false;
    }

    implementation::LegacySymmetric theKey(crypto);

    if (!theKey.SerializeFrom(strKey)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load symmetric key from "
            "string. (Returning false).")
            .Flush();
        return false;
    }

    // By this point, we know we have a ciphertext envelope and a symmetric Key.
    //
    return Decrypt(
        theKey, strCiphertext, strOutput, pstrDisplay, pAlreadyHavePW);
}

bool LegacySymmetric::Decrypt(
    const LegacySymmetric& theKey,
    const String& strCiphertext,
    String& strOutput,
    const String& pstrDisplay,
    const OTPassword* pAlreadyHavePW)
{
    if (!theKey.IsGenerated()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure: theKey.IsGenerated() was false. (The calling "
            "code probably should have checked for that...).")
            .Flush();
        return false;
    }

    auto ascArmor = Armored::Factory();
    const bool bLoadedArmor = Armored::LoadFromString(
        ascArmor, strCiphertext);  // str_bookend="-----BEGIN" by default

    if (!bLoadedArmor || !ascArmor->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure loading ciphertext envelope: ")(strCiphertext)(".")
            .Flush();
        return false;
    }

    // By this point, we know we have a ciphertext envelope and a symmetric Key.
    //
    std::unique_ptr<OTPassword> pPassUserInput;

    if (nullptr == pAlreadyHavePW) {
        const char* szDisplay = "Decrypting a password-protected ciphertext.";
        const auto strDisplay = String::Factory(
            (!pstrDisplay.Exists()) ? szDisplay : pstrDisplay.Get());

        pPassUserInput.reset(
            GetPassphraseFromUser(strDisplay));  // bAskTwice=false
                                                 // by default.
    }

    bool bSuccess = false;

    if (pPassUserInput ||  // Success retrieving the passphrase from the
        pAlreadyHavePW)    // user, or passphrase was provided out of scope.
    {
        OTEnvelope theEnvelope(ascArmor);

        if (theEnvelope.Decrypt(
                strOutput,
                theKey,
                pPassUserInput ? *pPassUserInput : *pAlreadyHavePW)) {
            bSuccess = true;
        } else {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to decrypt. (Sorry).")
                .Flush();
        }
    } else
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Sorry, unable to retrieve passphrase from user. (Failure).")
            .Flush();

    return bSuccess;
}

// static
bool LegacySymmetric::Encrypt(
    const api::Crypto& crypto,
    const String& strKey,
    const String& strPlaintext,
    String& strOutput,
    const String& pstrDisplay,
    bool bBookends,
    const OTPassword* pAlreadyHavePW)
{
    if (!strKey.Exists() || !strPlaintext.Exists()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Nonexistent: either the key or the "
            "plaintext. Please supply. (Failure).")
            .Flush();
        return false;
    }

    implementation::LegacySymmetric theKey(crypto);

    if (!theKey.SerializeFrom(strKey)) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load symmetric key from "
            "string. (Returning false).")
            .Flush();
        return false;
    }

    // By this point, we know we have a plaintext and a symmetric Key.
    //
    return Encrypt(
        theKey,
        strPlaintext,
        strOutput,
        pstrDisplay,
        bBookends,
        pAlreadyHavePW);
}

// static
bool LegacySymmetric::Encrypt(
    const LegacySymmetric& theKey,
    const String& strPlaintext,
    String& strOutput,
    const String& pstrDisplay,
    bool bBookends,
    const OTPassword* pAlreadyHavePW)
{
    if (!theKey.IsGenerated()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Failure: theKey.IsGenerated() was false. (The calling "
            "code probably should have checked that key already...).")
            .Flush();
        return false;
    }

    if (!strPlaintext.Exists()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Plaintext is empty. Please supply. (Failure).")
            .Flush();
        return false;
    }

    // By this point, we know we have a plaintext and a symmetric Key.
    //
    std::unique_ptr<OTPassword> pPassUserInput;

    if (nullptr == pAlreadyHavePW) {
        const char* szDisplay = "Password-protecting a plaintext.";
        const auto strDisplay = String::Factory(
            (!pstrDisplay.Exists()) ? szDisplay : pstrDisplay.Get());

        pPassUserInput.reset(
            GetPassphraseFromUser(strDisplay));  // bAskTwice=false
                                                 // by default.
    } else
        pPassUserInput.reset(new OTPassword(*pAlreadyHavePW));

    auto ascOutput = Armored::Factory();
    bool bSuccess = false;

    if (nullptr != pPassUserInput)  // Success retrieving the passphrase from
                                    // the user. (Now let's encrypt...)
    {
        OTEnvelope theEnvelope;

        if (theEnvelope.Encrypt(
                strPlaintext,
                const_cast<LegacySymmetric&>(theKey),
                *pPassUserInput) &&
            theEnvelope.GetCiphertext(ascOutput)) {
            bSuccess = true;

            if (bBookends) {
                return ascOutput->WriteArmoredString(
                    strOutput,
                    "SYMMETRIC MSG",  // todo hardcoding.
                    false);           // bEscaped=false
            } else {
                strOutput.Set(ascOutput->Get());
            }
        } else {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to encrypt. (Sorry).")
                .Flush();
        }
    } else
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Sorry, unable to retrieve passphrase from user. (Failure).")
            .Flush();

    return bSuccess;
}

// The highest-level possible interface (used by the API)
//
// NOTE: this version circumvents the master key.
OTPassword* LegacySymmetric::GetPassphraseFromUser(
    const String& pstrDisplay,
    bool bAskTwice)  // returns a
                     // text
                     // OTPassword,
                     // or nullptr.
{
    OTPassword* pPassUserInput =
        OTPassword::CreateTextBuffer();  // already asserts.
    //  pPassUserInput->zeroMemory(); // This was causing the password to come
    // out blank.
    //
    // Below this point, pPassUserInput must be returned, or deleted. (Or it
    // will leak.)

    const char* szDisplay = "LegacySymmetric::GetPassphraseFromUser";
    OTPasswordData thePWData(
        (!pstrDisplay.Exists()) ? szDisplay : pstrDisplay.Get());
    // -------------------------------------------------------------------
    //
    // OLD SYSTEM! (NO MASTER KEY INVOLVEMENT.)
    //
    thePWData.setUsingOldSystem();  // So the cached key doesn't interfere,
                                    // since
                                    // this is for a plain symmetric key.
    // -------------------------------------------------------------------
    const auto& native = dynamic_cast<const api::internal::Native&>(OT::App());
    auto* callback = native.GetInternalPasswordCallback();
    const std::int32_t nCallback = (*callback)(
        pPassUserInput->getPasswordWritable_char(),
        pPassUserInput->getBlockSize(),
        bAskTwice ? 1 : 0,
        static_cast<void*>(&thePWData));
    const std::uint32_t uCallback = static_cast<uint32_t>(nCallback);
    if ((nCallback > 0) &&  // Success retrieving the passphrase from the user.
        pPassUserInput->SetSize(uCallback)) {
        //      otOut << "%s: Retrieved passphrase (blocksize %d, actual size
        //      %d) from "
        //               "user: %s\n", __FUNCTION__,
        //               pPassUserInput->getBlockSize(), nCallback,
        //               pPassUserInput->getPassword());
        return pPassUserInput;  // Caller MUST delete!
    } else {
        delete pPassUserInput;
        pPassUserInput = nullptr;
        LogNormal(OT_METHOD)(__FUNCTION__)(": Sorry, unable to retrieve "
                                           "passphrase from user. (Failure).")
            .Flush();
    }

    return nullptr;
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
LegacySymmetric::LegacySymmetric(const api::Crypto& crypto)
    : crypto_(crypto)
    , m_bIsGenerated(false)
    , has_hash_check_(Flag::Factory(false))
    , m_nKeySize(crypto_.Config().SymmetricKeySize() * 8)
    , m_uIterationCount(crypto_.Config().IterationCount())
    , salt_(Data::Factory())
    , iv_(Data::Factory())
    , encrypted_key_(Data::Factory())
    , hash_check_(Data::Factory())
{
}

LegacySymmetric::LegacySymmetric(
    const api::Crypto& crypto,
    const OTPassword& thePassword)
    : LegacySymmetric(crypto)
{
    GenerateKey(thePassword);
}

LegacySymmetric::LegacySymmetric(const LegacySymmetric& rhs)
    : key::LegacySymmetric()
    , Lockable()
    , crypto_(rhs.crypto_)
    , m_bIsGenerated(rhs.m_bIsGenerated)
    , has_hash_check_(Flag::Factory(rhs.has_hash_check_.get()))
    , m_nKeySize(rhs.m_nKeySize)
    , m_uIterationCount(rhs.m_uIterationCount)
    , salt_(Data::Factory(rhs.salt_))
    , iv_(Data::Factory(rhs.salt_))
    , encrypted_key_(Data::Factory(rhs.encrypted_key_))
    , hash_check_(Data::Factory(rhs.hash_check_))
{
}

LegacySymmetric* LegacySymmetric::clone() const
{
    return new LegacySymmetric(*this);
}

void LegacySymmetric::GetIdentifier(Identifier& theIdentifier) const
{
    Lock lock(lock_);
    theIdentifier.CalculateDigest(encrypted_key_.get());
}

void LegacySymmetric::GetIdentifier(String& strIdentifier) const
{
    Lock lock(lock_);
    auto theIdentifier = Identifier::Factory();
    const bool bCalc = theIdentifier->CalculateDigest(encrypted_key_.get());

    if (bCalc) { theIdentifier->GetString(strIdentifier); }
}

// Changes the passphrase on an existing symmetric key.
//
bool LegacySymmetric::ChangePassphrase(
    const OTPassword& oldPassphrase,
    const OTPassword& newPassphrase)
{
    Lock lock(lock_);
    OT_ASSERT(m_uIterationCount > 1000);
    OT_ASSERT(m_bIsGenerated);

    // Todo: validate the passphrases exist or whatever?
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Begin: ")(
        ": Changing password on symmetric key... ")
        .Flush();
    OTPassword theActualKey;

    if (!get_raw_key_from_passphrase(lock, oldPassphrase, theActualKey)) {

        return false;
    }

    auto dataIV = Data::Factory();
    auto dataSalt = Data::Factory();

    // NOTE: I can't randomize the IV because then anything that was
    // encrypted with this key before, will fail to decrypt. (Ruining
    // the whole point of changing the passphrase...)
    //
    // UPDATE: I think this is false. I think the IV is for the encryption of
    // the symmetric key itself, whereas the content has its own IV in
    // OTEnvelope.
    //
    if (!dataIV->Randomize(crypto_.Config().SymmetricIvSize())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed generating iv for changing "
            "passphrase on a symmetric key. (Returning "
            "false).")
            .Flush();
        return false;
    }

    if (!dataSalt->Randomize(crypto_.Config().SymmetricSaltSize())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed generating random salt for changing "
            "passphrase on a symmetric key. (Returning "
            "false).")
            .Flush();
        return false;
    }

    iv_ = dataIV;
    salt_ = dataSalt;
    has_hash_check_->Off();
    hash_check_->Release();
    encrypted_key_->Release();

    // Generate the new derived key from the new passphrase.
    std::unique_ptr<OTPassword> pNewDerivedKey(
        calculate_new_derived_key_from_passphrase(lock, newPassphrase));

    // Below this point, pNewDerivedKey is NOT null. (And will be cleaned up
    // automatically.)

    //
    // Below this point, pNewDerivedKey contains a symmetric key derived from
    // the new salt, the iteration
    // count, and the new password that was passed in. We will store the salt
    // and iteration count inside this
    // LegacySymmetric object, and we'll store an encrypted copy of the
    // ActualKey, encrypted to pNewDerivedKey.
    // We'll also store the new IV, which is used while encrypting the actual
    // key, and which must be used again
    // while decrypting it later.
    //
    // Encrypt theActualKey using pNewDerivedKey, which is clear/raw already.
    // (Both are OTPasswords.)
    // Put the result into the encrypted_key_.
    //
    const bool bEncryptedKey = crypto_.AES().Encrypt(
        *pNewDerivedKey,  // pNewDerivedKey is a symmetric key, in clear form.
                          // Used for encrypting theActualKey.
        reinterpret_cast<const char*>(
            theActualKey.getMemory_uint8()),  // This is the Plaintext that's
                                              // being encrypted.
        theActualKey.getMemorySize(),
        iv_,              // generated above.
        encrypted_key_);  // OUTPUT. (Ciphertext.)
    m_bIsGenerated = bEncryptedKey;

    LogVerbose(OT_METHOD)(__FUNCTION__)(": End: ")(
        ": (Changing passphrase on symmetric key...) ")(
        m_bIsGenerated ? "SUCCESS" : "FAILED")
        .Flush();

    return m_bIsGenerated;
}

// Generates this LegacySymmetric based on an OTPassword. The generated key is
// stored in encrypted form, based on a derived key from that password.
//

// Done:  Change pDerivedKey to ppDerivedKey, since you CANNOT derive a key
// BEFORE calling
// GenerateKey, since the salt and iteration count are both part of the
// derivation process!!

// ppDerivedKey: CALLER RESPONSIBLE TO DELETE.  (optional arg.)

// Output. If you want, I can pass this back to you.
bool LegacySymmetric::GenerateKey(
    const OTPassword& thePassphrase,
    OTPassword** ppDerivedKey)
{
    OT_ASSERT(m_uIterationCount > 1000);
    OT_ASSERT(!m_bIsGenerated);

    Lock lock(lock_);
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Begin: ")(
        ": GENERATING keys and passwords... ")
        .Flush();

    if (!iv_->Randomize(crypto_.Config().SymmetricIvSize())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed generating iv for encrypting a "
            "symmetric key. (Returning false).")
            .Flush();
        return false;
    }

    if (!salt_->Randomize(crypto_.Config().SymmetricSaltSize())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed generating random salt. (Returning false).")
            .Flush();
        return false;
    }

    // Generate actual key (a randomized memory space.)
    // We will use the derived key for encrypting the actual key.
    //
    OTPassword theActualKey;

    {
        std::int32_t nRes =
            theActualKey.randomizeMemory(crypto_.Config().SymmetricKeySize());
        if (0 > nRes) { OT_FAIL; }
        std::uint32_t uRes =
            static_cast<std::uint32_t>(nRes);  // we need an uint32_t value.

        if (crypto_.Config().SymmetricKeySize() != uRes) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed generating symmetric key. (Returning false).")
                .Flush();
            return false;
        }
    }
    // We didn't bother generating the derived key if the above three
    // randomizations failed.

    // Generate derived key from passphrase.
    //
    std::unique_ptr<OTPassword> pDerivedKey(
        calculate_new_derived_key_from_passphrase(lock, thePassphrase));

    OT_ASSERT(nullptr != pDerivedKey);

    // Below this point, pDerivedKey is NOT null. (And we only clean it up later
    // if we created it.)

    //
    // Below this point, pDerivedKey contains a symmetric key derived from the
    // salt, the iteration
    // count, and the password that was passed in. We will store the salt and
    // iteration count inside this
    // LegacySymmetric object, and we'll store an encrypted copy of the
    // ActualKey, encrypted to pDerivedKey.
    // We'll also store the IV, which is generated while encrypting the actual
    // key, and which must be used
    // while decrypting it later.
    //
    // Encrypt theActualKey using pDerivedKey, which is clear/raw already. (Both
    // are OTPasswords.)
    // Put the result into the encrypted_key_.
    //
    const bool bEncryptedKey = crypto_.AES().Encrypt(
        *pDerivedKey,  // pDerivedKey is a symmetric key, in clear form. Used
                       // for
                       // encrypting theActualKey.
        reinterpret_cast<const char*>(
            theActualKey.getMemory_uint8()),  // This is the Plaintext that's
                                              // being encrypted.
        theActualKey.getMemorySize(),
        iv_,              // generated above.
        encrypted_key_);  // OUTPUT. (Ciphertext.)
    m_bIsGenerated = bEncryptedKey;

    LogVerbose(OT_METHOD)(__FUNCTION__)(": End: ")(
        ": (GENERATING keys and passwords...) ")(
        m_bIsGenerated ? "SUCCESS" : "FAILED")
        .Flush();

    // return the pDerivedKey, if wanted.
    if (nullptr != ppDerivedKey) { *ppDerivedKey = pDerivedKey.release(); }

    return m_bIsGenerated;
}

bool LegacySymmetric::GenerateHashCheck(const OTPassword& thePassphrase)
{
    OT_ASSERT(m_uIterationCount > 1000);

    Lock lock(lock_);

    if (!m_bIsGenerated) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No Key Generated, run GenerateKey(), and "
            "this function will not be needed!")
            .Flush();
        OT_FAIL;
    }

    if (HasHashCheck()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Already have a HashCheck, no need to create one!")
            .Flush();
        return false;
    }

    OT_ASSERT(hash_check_->empty());

    OTPassword* pDerivedKey = calculate_new_derived_key_from_passphrase(
        lock,
        thePassphrase);  // asserts already.

    if (nullptr == pDerivedKey)  // A pointerpointer was passed in... (caller
                                 // will be responsible then, to delete.)
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate derived key.")
            .Flush();
        return false;
    }

    if (!HasHashCheck()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Still don't have a hash check (even after generating one)!"
            " This is bad! Will assert.")
            .Flush();
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
// LegacySymmetric object, then decrypts the encrypted symmetric key (using
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
OTPassword* LegacySymmetric::calculate_derived_key_from_passphrase(
    const Lock& lock,
    const OTPassword& thePassphrase,
    bool bCheckForHashCheck) const
{
    OT_ASSERT(verify_lock(lock))

    auto tmpDataHashCheck = Data::Factory(hash_check_.get());

    if (bCheckForHashCheck) {
        if (!HasHashCheck()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unable to calculate derived key because "
                "hash check is missing!")
                .Flush();
            OT_FAIL;
        }
        OT_ASSERT(false == hash_check_->empty());
        OT_ASSERT(false == tmpDataHashCheck->empty());
    } else {
        if (!HasHashCheck()) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Warning!! No hash check, ignoring... "
                "(since bCheckForHashCheck was set false).")
                .Flush();
            OT_ASSERT(tmpDataHashCheck->empty());
        }
    }

    return crypto_.AES().DeriveNewKey(
        thePassphrase, salt_.get(), m_uIterationCount, tmpDataHashCheck);
}

OTPassword* LegacySymmetric::CalculateDerivedKeyFromPassphrase(
    const OTPassword& thePassphrase,
    bool bCheckForHashCheck) const
{
    Lock lock(lock_);

    return calculate_derived_key_from_passphrase(
        lock, thePassphrase, bCheckForHashCheck);
}

OTPassword* LegacySymmetric::calculate_new_derived_key_from_passphrase(
    const Lock& lock,
    const OTPassword& thePassphrase)
{
    OT_ASSERT(verify_lock(lock))

    std::unique_ptr<OTPassword> pDerivedKey;

    if (false == HasHashCheck()) {
        hash_check_ = Data::Factory();
        pDerivedKey.reset(crypto_.AES().DeriveNewKey(
            thePassphrase, salt_, m_uIterationCount, hash_check_));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Calling Wrong function!! Hash check already exists!")
            .Flush();
    }

    OT_ASSERT(pDerivedKey);
    OT_ASSERT(false == hash_check_->empty());

    has_hash_check_->On();

    return pDerivedKey.release();
}

OTPassword* LegacySymmetric::CalculateNewDerivedKeyFromPassphrase(
    const OTPassword& thePassphrase)
{
    Lock lock(lock_);

    return calculate_new_derived_key_from_passphrase(lock, thePassphrase);
}

bool LegacySymmetric::get_raw_key_from_passphrase(
    const Lock& lock,
    const OTPassword& thePassphrase,
    OTPassword& theRawKeyOutput,
    OTPassword* pDerivedKey) const
{
    OT_ASSERT(verify_lock(lock))
    OT_ASSERT(m_bIsGenerated)

    std::unique_ptr<OTPassword> theDerivedAngel;

    if (nullptr == pDerivedKey) {
        // TODO, security: Do we have to create all these OTPassword objects on
        // the stack, just as a general practice? In which case I can't use this
        // factory how I'm using it now...
        pDerivedKey =
            calculate_derived_key_from_passphrase(lock, thePassphrase, false);
        theDerivedAngel.reset(pDerivedKey);
    }

    if (nullptr == pDerivedKey) { return false; }

    // Below this point, pDerivedKey contains a derived symmetric key, from the
    // salt, the iteration count, and the password that was passed in. The salt
    // and iteration count were both stored inside this LegacySymmetric object
    // since this key was originally generated, and we store an encrypted copy
    // of the ActualKey already, as well-- it's encrypted to the Derived Key.
    // (We also store the IV from that encryption bit.)
    return get_raw_key_from_derived_key(lock, *pDerivedKey, theRawKeyOutput);
}

bool LegacySymmetric::GetRawKeyFromPassphrase(
    const OTPassword& thePassphrase,
    OTPassword& theRawKeyOutput,
    OTPassword* pDerivedKey) const
{
    Lock lock(lock_);

    return get_raw_key_from_passphrase(
        lock, thePassphrase, theRawKeyOutput, pDerivedKey);
}

bool LegacySymmetric::get_raw_key_from_derived_key(
    const Lock& lock,
    const OTPassword& theDerivedKey,
    OTPassword& theRawKeyOutput) const
{
    OT_ASSERT(verify_lock(lock))
    OT_ASSERT(m_bIsGenerated);
    OT_ASSERT(theDerivedKey.isMemory());

    // Decrypt theActualKey using theDerivedKey, which is clear/raw already.
    // (Both are OTPasswords.)
    // Put the result into theRawKeyOutput.
    //
    // theDerivedKey is a symmetric key, in clear form. Used here
    // for decrypting encrypted_key_ into theRawKeyOutput.
    //
    LogVerbose(OT_METHOD)(__FUNCTION__)(
        ": Begin) Attempting to recover actual key using derived key... ")
        .Flush();

    CryptoSymmetricDecryptOutput plaintext(theRawKeyOutput);
    const bool bDecryptedKey = crypto_.AES().Decrypt(
        theDerivedKey,  // We're using theDerivedKey to decrypt
                        // encrypted_key_.
        // Here's what we're trying to decrypt: the encrypted
        // form of the symmetric key.
        static_cast<const char*>(encrypted_key_->data()),  // The
                                                           // Ciphertext.
        encrypted_key_->size(),
        iv_.get(),   // Created when *this symmetric key was generated. Both are
                     // already stored.
        plaintext);  // OUTPUT. (Recovered plaintext of symmetric key.) You
                     // can pass OTPassword& OR Data& here (either
                     // will work.)

    LogVerbose(OT_METHOD)(__FUNCTION__)(
        ": (End) attempt to recover actual key using derived key... ")
        .Flush();
    return bDecryptedKey;
}

bool LegacySymmetric::GetRawKeyFromDerivedKey(
    const OTPassword& theDerivedKey,
    OTPassword& theRawKeyOutput) const
{
    Lock lock(lock_);

    return get_raw_key_from_derived_key(lock, theDerivedKey, theRawKeyOutput);
}

// Notice I don't theInput.reset(), because what if this
// key was being read from a larger file containing several
// keys?  I should just continue reading from the current
// position, and let the CALLER reset first, if that's his
// intention.
//
bool LegacySymmetric::serialize_from(const Lock& lock, Data& theInput)
{
    OT_ASSERT(verify_lock(lock))

    std::uint32_t nRead = 0;

    // Read network-order "is generated" flag. (convert to host order)
    //
    uint16_t n_is_generated = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(&n_is_generated),
                  static_cast<std::uint32_t>(sizeof(n_is_generated))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading n_is_generated.")
            .Flush();
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
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: host_is_generated, Bad value: ")(
            static_cast<std::int32_t>(host_is_generated))(
            ". (Expected 0 or 1).")
            .Flush();
        return false;
    }

    LogInsane(OT_METHOD)(__FUNCTION__)(": is_generated: ")(host_is_generated)
        .Flush();

    // Read network-order "key size in bits". (convert to host order)
    uint16_t n_key_size_bits = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(&n_key_size_bits),
                  static_cast<std::uint32_t>(sizeof(n_key_size_bits))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading n_key_size_bits.")
            .Flush();
        return false;
    }

    // convert from network to HOST endian.

    m_nKeySize = ntohs(n_key_size_bits);
    LogInsane(OT_METHOD)(__FUNCTION__)(": key_size_bits: ")(m_nKeySize).Flush();

    // Read network-order "iteration count". (convert to host order)
    std::uint32_t n_iteration_count = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(&n_iteration_count),
                  static_cast<std::uint32_t>(sizeof(n_iteration_count))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading n_iteration_count.")
            .Flush();
        return false;
    }
    OT_ASSERT(nRead == static_cast<std::uint32_t>(sizeof(n_iteration_count)));

    // convert from network to HOST endian.

    m_uIterationCount = ntohl(n_iteration_count);
    LogInsane(OT_METHOD)(__FUNCTION__)(": iteration_count: ")(m_uIterationCount)
        .Flush();

    // Read network-order "salt size". (convert to host order)
    std::uint32_t n_salt_size = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(&n_salt_size),
                  static_cast<std::uint32_t>(sizeof(n_salt_size))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading n_salt_size.")
            .Flush();
        return false;
    }
    OT_ASSERT(nRead == static_cast<std::uint32_t>(sizeof(n_salt_size)));

    // convert from network to HOST endian.

    const std::uint32_t lSaltSize = ntohl(n_salt_size);
    LogInsane(OT_METHOD)(__FUNCTION__)(": salt_size value: ")(lSaltSize)
        .Flush();

    // Then read the Salt itself.
    salt_->SetSize(lSaltSize);

    if (0 == (nRead = theInput.OTfread(
                  static_cast<std::uint8_t*>(const_cast<void*>(salt_->data())),
                  static_cast<std::uint32_t>(lSaltSize)))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error reading salt for symmetric key.")
            .Flush();
        return false;
    }

    LogInsane(OT_METHOD)(__FUNCTION__)(": salt length actually read: ")(nRead)
        .Flush();

    OT_ASSERT(nRead == static_cast<std::uint32_t>(lSaltSize));

    // Read network-order "IV size". (convert to host order)
    //
    std::uint32_t n_iv_size = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(&n_iv_size),
                  static_cast<std::uint32_t>(sizeof(n_iv_size))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading n_iv_size.")
            .Flush();
        return false;
    }

    OT_ASSERT(nRead == static_cast<std::uint32_t>(sizeof(n_iv_size)));

    // convert from network to HOST endian.

    const std::uint32_t lIVSize = ntohl(n_iv_size);
    LogInsane(OT_METHOD)(__FUNCTION__)(": iv_size value: ")(lIVSize).Flush();

    // Then read the IV itself.
    iv_->SetSize(lIVSize);

    if (0 == (nRead = theInput.OTfread(
                  static_cast<std::uint8_t*>(const_cast<void*>(iv_->data())),
                  static_cast<std::uint32_t>(lIVSize)))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error reading IV for symmetric key.")
            .Flush();
        return false;
    }

    LogInsane(OT_METHOD)(__FUNCTION__)(": iv length actually read: ")(nRead)
        .Flush();

    OT_ASSERT(nRead == static_cast<std::uint32_t>(lIVSize));

    // Read network-order "encrypted key size". (convert to host order)
    //
    std::uint32_t n_enc_key_size = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(&n_enc_key_size),
                  static_cast<std::uint32_t>(sizeof(n_enc_key_size))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading n_enc_key_size.")
            .Flush();
        return false;
    }
    OT_ASSERT(nRead == static_cast<std::uint32_t>(sizeof(n_enc_key_size)));

    // convert from network to HOST endian.

    const std::uint32_t lEncKeySize = ntohl(n_enc_key_size);
    LogInsane(OT_METHOD)(__FUNCTION__)(": enc_key_size value: ")(lEncKeySize)
        .Flush();

    // Then read the Encrypted Key itself.
    //
    encrypted_key_->SetSize(lEncKeySize);

    if (0 == (nRead = theInput.OTfread(
                  static_cast<std::uint8_t*>(
                      const_cast<void*>(encrypted_key_->data())),
                  static_cast<std::uint32_t>(lEncKeySize)))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error reading encrypted symmetric key.")
            .Flush();
        return false;
    }

    LogInsane(OT_METHOD)(__FUNCTION__)(
        ": encrypted key length actually read: ")(nRead)
        .Flush();

    OT_ASSERT(nRead == static_cast<std::uint32_t>(lEncKeySize));

    // Read network-order "hash check size". (convert to host order)
    //
    std::uint32_t n_hash_check_size = 0;

    if (0 == (nRead = theInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(&n_hash_check_size),
                  static_cast<std::uint32_t>(sizeof(n_hash_check_size))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading n_hash_check_size.")
            .Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Looks like we don't have a hash check yet (will make one)!")
            .Flush();
        has_hash_check_->Off();

        return false;
    }

    OT_ASSERT(nRead == static_cast<std::uint32_t>(sizeof(n_hash_check_size)));

    // convert from network to HOST endian.

    const std::uint32_t lHashCheckSize = ntohl(n_hash_check_size);

    LogInsane(OT_METHOD)(__FUNCTION__)(": hash_check_size value: ")(
        lHashCheckSize)
        .Flush();

    // Then read the Hashcheck itself.
    //
    hash_check_->SetSize(lHashCheckSize);

    if (0 ==
        (nRead = theInput.OTfread(
             static_cast<std::uint8_t*>(const_cast<void*>(hash_check_->data())),
             static_cast<std::uint32_t>(lHashCheckSize)))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading hash check data.")
            .Flush();
        return false;
    }

    LogInsane(OT_METHOD)(__FUNCTION__)(": hash check data actually read: ")(
        nRead)
        .Flush();

    OT_ASSERT(nRead == static_cast<std::uint32_t>(lHashCheckSize));

    has_hash_check_->Set(!hash_check_->empty());

    return true;
}

bool LegacySymmetric::serialize_from(const Lock& lock, const Armored& ascInput)
{
    auto theInput = Data::Factory();

    if (ascInput.Exists() && ascInput.GetData(theInput)) {

        return serialize_from(lock, theInput);
    }

    return false;
}

bool LegacySymmetric::SerializeFrom(Data& theInput)
{
    Lock lock(lock_);

    return serialize_from(lock, theInput);
}

bool LegacySymmetric::SerializeFrom(const Armored& ascInput)
{
    Lock lock(lock_);

    return serialize_from(lock, ascInput);
}

bool LegacySymmetric::SerializeFrom(const String& strInput, bool bEscaped)
{
    Lock lock(lock_);
    auto ascInput = Armored::Factory();

    if (strInput.Exists() && ascInput->LoadFromString(
                                 const_cast<String&>(strInput),
                                 bEscaped,
                                 "-----BEGIN OT ARMORED SYMMETRIC KEY")) {
        return serialize_from(lock, ascInput);
    }

    return false;
}

bool LegacySymmetric::serialize_to(const Lock& lock, Armored& ascOutput) const
{
    auto theOutput = Data::Factory();

    if (serialize_to(lock, theOutput)) {
        ascOutput.SetData(theOutput);

        return true;
    }

    return false;
}

bool LegacySymmetric::serialize_to(const Lock& lock, Data& theOutput) const
{
    OT_ASSERT(verify_lock(lock))

    uint16_t from_bool_is_generated = (m_bIsGenerated ? 1 : 0);
    uint16_t n_is_generated = htons(from_bool_is_generated);
    uint16_t n_key_size_bits = htons(static_cast<uint16_t>(m_nKeySize));

    std::uint32_t n_iteration_count =
        htonl(static_cast<std::uint32_t>(m_uIterationCount));

    std::uint32_t n_salt_size = htonl(salt_->size());
    std::uint32_t n_iv_size = htonl(iv_->size());
    std::uint32_t n_enc_key_size = htonl(encrypted_key_->size());
    std::uint32_t n_hash_check_size = htonl(hash_check_->size());

    LogInsane(OT_METHOD)(__FUNCTION__)(": is_generated: ")(n_is_generated)(
        "   key_size_bits: ")(n_key_size_bits)("   iteration_count: ")(
        n_iteration_count)("\n")("salt_size: ")(n_salt_size)("   iv_size: ")(
        n_iv_size)("   enc_key_size: ")(n_enc_key_size)
        .Flush();

    theOutput.Concatenate(
        reinterpret_cast<void*>(&n_is_generated),
        static_cast<std::uint32_t>(sizeof(n_is_generated)));

    theOutput.Concatenate(
        reinterpret_cast<void*>(&n_key_size_bits),
        static_cast<std::uint32_t>(sizeof(n_key_size_bits)));

    theOutput.Concatenate(
        reinterpret_cast<void*>(&n_iteration_count),
        static_cast<std::uint32_t>(sizeof(n_iteration_count)));

    theOutput.Concatenate(
        reinterpret_cast<void*>(&n_salt_size),
        static_cast<std::uint32_t>(sizeof(n_salt_size)));

    OT_ASSERT(nullptr != salt_->data());
    theOutput.Concatenate(salt_->data(), salt_->size());

    theOutput.Concatenate(
        reinterpret_cast<void*>(&n_iv_size),
        static_cast<std::uint32_t>(sizeof(n_iv_size)));

    OT_ASSERT(nullptr != iv_->data());
    theOutput.Concatenate(iv_->data(), iv_->size());

    theOutput.Concatenate(
        reinterpret_cast<void*>(&n_enc_key_size),
        static_cast<std::uint32_t>(sizeof(n_enc_key_size)));

    OT_ASSERT(nullptr != encrypted_key_->data());
    theOutput.Concatenate(encrypted_key_->data(), encrypted_key_->size());

    theOutput.Concatenate(
        reinterpret_cast<void*>(&n_hash_check_size),
        static_cast<std::uint32_t>(sizeof(n_hash_check_size)));

    OT_ASSERT(nullptr != hash_check_->data());
    theOutput.Concatenate(hash_check_->data(), hash_check_->size());

    return true;
}

bool LegacySymmetric::SerializeTo(Data& theOutput) const
{
    Lock lock(lock_);

    return serialize_to(lock, theOutput);
}

bool LegacySymmetric::SerializeTo(Armored& ascOutput) const
{
    Lock lock(lock_);

    return serialize_to(lock, ascOutput);
}

bool LegacySymmetric::SerializeTo(String& strOutput, bool bEscaped) const
{
    Lock lock(lock_);
    auto ascOutput = Armored::Factory();

    if (serialize_to(lock, ascOutput))

        return ascOutput->WriteArmoredString(
            strOutput, "SYMMETRIC KEY", bEscaped);

    return false;
}

void LegacySymmetric::Release() { Release_SymmetricKey(); }

void LegacySymmetric::Release_SymmetricKey()
{
    m_bIsGenerated = false;
    m_uIterationCount = 0;
    m_nKeySize = 0;
    salt_->Release();
    iv_->Release();
    encrypted_key_->Release();
}

LegacySymmetric::~LegacySymmetric() { Release_SymmetricKey(); }
}  // namespace opentxs::crypto::key::implementation
