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

#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/OTAsymmetricKeyOpenSSL.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHashEngine.hpp"
#include "opentxs/core/crypto/Libsodium.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey_OpenSSLPrivdp.hpp"
#include "opentxs/core/crypto/OpenSSL_BIO.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <opentxs-proto/AsymmetricKey.pb.h>
#include <opentxs-proto/Enums.pb.h>
#include <sodium/crypto_box.h>
#include <stdint.h>
#include <sys/types.h>
#include <ostream>
#include <string>

// BIO_get_mem_ptr() and BIO_get_mem_data() macros from OpenSSL
// use old style cast
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace opentxs
{

OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSL()
    : OTAsymmetricKey(proto::AKEYTYPE_LEGACY, proto::KEYROLE_ERROR)
    , m_p_ascKey(nullptr)
    , dp(new OTAsymmetricKey_OpenSSLPrivdp())
    {

    dp->backlink = this;

    dp->m_pX509 = nullptr;
    dp->m_pKey = nullptr;
}

OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSL(const proto::KeyRole role)
    : OTAsymmetricKey(proto::AKEYTYPE_LEGACY, role)
    , m_p_ascKey(nullptr)
    , dp(new OTAsymmetricKey_OpenSSLPrivdp())
    {

    dp->backlink = this;

    dp->m_pX509 = nullptr;
    dp->m_pKey = nullptr;
}

OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSL(const proto::AsymmetricKey& serializedKey)
    : OTAsymmetricKey(serializedKey)
    , m_p_ascKey(new OTASCIIArmor)
    , dp(new OTAsymmetricKey_OpenSSLPrivdp())
    {

    dp->backlink = this;

    dp->m_pX509 = nullptr;
    dp->m_pKey = nullptr;

    m_keyType = proto::AKEYTYPE_LEGACY;

    Data dataKey(serializedKey.key().c_str(), serializedKey.key().size());
    m_p_ascKey->SetData(dataKey);

    if (proto::KEYMODE_PUBLIC == serializedKey.mode()) {
        SetAsPublic();
    } else if (proto::KEYMODE_PRIVATE == serializedKey.mode()){
        SetAsPrivate();
    }
}

OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSL(const String& publicKey)
    : OTAsymmetricKey()
    , m_p_ascKey(nullptr)
    , dp(new OTAsymmetricKey_OpenSSLPrivdp())
    {

    dp->backlink = this;

    dp->m_pX509 = nullptr;
    dp->m_pKey = nullptr;

    m_keyType = proto::AKEYTYPE_LEGACY;

    SetPublicKey(publicKey);
}

OTAsymmetricKey_OpenSSL::~OTAsymmetricKey_OpenSSL()
{
    // Release the ascii-armored version of the key (safe to store in this
    // form.)
    //
    if (nullptr != m_p_ascKey) delete m_p_ascKey;
    m_p_ascKey = nullptr;

    Release_AsymmetricKey_OpenSSL();

    ReleaseKeyLowLevel_Hook();

    if (nullptr !=
        dp->m_pX509) // Todo: figure out if I should put a copy of this
                     // into ReleaseKeyLowLevel_Hook as we are with
                     // m_pKey.
        X509_free(dp->m_pX509); // FYI: the reason it's not there already is
                                // because the original need was for wiping
                                // m_pKey when a private key timed out.
    dp->m_pX509 = nullptr;      // ReleaseKeyLowLevel is used all over
    // OTAsymmetricKey.cpp for the purpose of wiping that
    // private key. The same need didn't exist with the x509
    // so it was never coded that way. As long as it's
    // cleaned up here in the destructor, seems good enough?
    // YOU MIGHT ASK... Why is m_pKey cleaned up here in the destructor, and
    // ALSO in ReleaseKeyLowLevel_Hook ?
    // The answer is because if we call ReleaseKeyLowLevel_Hook from
    // OTAsymmetricKey's destructor (down that chain)
    // then it will fail at runtime, since it is a pure virtual method. Since we
    // still want the ABILITY to use ReleaseKeyLowLevel_Hook
    // (For cases where the destructor is not being used) and since we still
    // want it to ALSO work when destructing, the
    // easiest/quickest/simplest way is to put the code into
    // OTAsymmetricKey_OpenSSL's destructor directly, as well
    // as OTAsymmetricKey_OpenSSL's override of ReleaseKeyLowLevel_Hook. Then go
    // into OTAsymmetricKey's destructor and
    // make sure the full call path through there doesn't involve any virtual
    // functions.
}

// virtual
bool OTAsymmetricKey_OpenSSL::IsEmpty() const
{
    return (nullptr == m_p_ascKey);
}

// virtual
bool OTAsymmetricKey_OpenSSL::GetPublicKey(String& strKey) const
{
    if (nullptr != m_p_ascKey) {
        strKey.Concatenate(
            "-----BEGIN PUBLIC KEY-----\n" // UN-ESCAPED VERSION
            "%s"
            "-----END PUBLIC KEY-----\n",
            m_p_ascKey->Get());
        return true;
    }
    else
        otErr << "OTAsymmetricKey_OpenSSL::GetPublicKey: Error: no "
                 "public key.\n";

    return false;
}

// virtual
bool OTAsymmetricKey_OpenSSL::SetPublicKey(const String& strKey)
{
    ReleaseKeyLowLevel(); // In case the key is already loaded, we release it
                          // here. (Since it's being replaced, it's now the
                          // wrong key anyway.)
    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;

    if (nullptr == m_p_ascKey) {
        m_p_ascKey = new OTASCIIArmor;
        OT_ASSERT(nullptr != m_p_ascKey);
    }

    // This reads the string into the Armor and removes the bookends. (-----
    // BEGIN ...)
    OTASCIIArmor theArmor;

    if (theArmor.LoadFromString(const_cast<String&>(strKey), false)) {
        m_p_ascKey->Set(theArmor);
        return true;
    }
    else
        otErr << "OTAsymmetricKey_OpenSSL::SetPublicKey: Error: failed loading "
                 "ascii-armored contents from bookended string:\n\n" << strKey
              << "\n\n";

    return false;
}

// virtual
void OTAsymmetricKey_OpenSSL::Release()
{
    Release_AsymmetricKey_OpenSSL(); // My own cleanup is performed here.

    // Next give the base class a chance to do the same...
    ot_super::Release(); // since I've overridden the base class, I call it
                         // now...
}

void OTAsymmetricKey_OpenSSL::Release_AsymmetricKey_OpenSSL()
{
    // Release any dynamically allocated members here. (Normally.)
}

void OTAsymmetricKey_OpenSSL::ReleaseKeyLowLevel_Hook() const
{
    // Release the instantiated OpenSSL key (unsafe to store in this form.)
    //
    if (nullptr != dp->m_pKey) EVP_PKEY_free(dp->m_pKey);
    dp->m_pKey = nullptr;
}

// Load the private key from a .pem formatted cert string
//
bool OTAsymmetricKey_OpenSSL::SetPrivateKey(
    const String& strCert,    // Contains certificate and private key.
    const String* pstrReason, // This reason is what displays on the
                              // passphrase dialog.
    const OTPassword* pImportPassword) // Used when importing an exported
                                       // Nym into a wallet.
{
    Release();

    m_bIsPublicKey = false;
    m_bIsPrivateKey = true;

    if (!strCert.Exists()) {
        otErr << __FUNCTION__ << ": Error: Cert input is nonexistent!\n";
        return false;
    }

    // Read private key
    //
    String strWithBookends;
    otLog3 << __FUNCTION__ << ": FYI, Reading private key from x509 stored in "
                              "bookended string...\n";

    strWithBookends = strCert;

    // Create a new memory buffer on the OpenSSL side.
    //
    //    OpenSSL_BIO bio = BIO_new(BIO_s_mem());
    //  OpenSSL_BIO bio =
    // BIO_new_mem_buf(static_cast<void*>(const_cast<char*>(strWithBookends.Get())),
    // strWithBookends.GetLength() /*+1*/);
    OpenSSL_BIO bio = BIO_new_mem_buf(
        static_cast<void*>(const_cast<char*>(strWithBookends.Get())), -1);
    OT_ASSERT_MSG(nullptr != bio,
                  "OTAsymmetricKey_OpenSSL::"
                  "SetPrivateKey: Assert: nullptr != "
                  "bio \n");

    {
        // TODO security: Need to replace PEM_read_bio_PrivateKey().
        /*
         The old PrivateKey write routines are retained for compatibility.
         New applications should write private keys using the
         PEM_write_bio_PKCS8PrivateKey() or PEM_write_PKCS8PrivateKey()
         routines because they are more secure (they use an iteration count of
         2048 whereas the traditional routines use a
         count of 1) unless compatibility with older versions of OpenSSL is
         important.
         NOTE: The PrivateKey read routines can be used in all applications
         because they handle all formats transparently.
         */
        OTPasswordData thePWData(
            (nullptr == pstrReason)
                ? (nullptr == pImportPassword
                       ? "Enter the master passphrase. "
                         "(SetPrivateKey)"
                       : "Enter the passphrase for this exported nym.")
                : pstrReason->Get());

        EVP_PKEY* pkey = nullptr;

        if (!pImportPassword) // pImportPassword is nullptr? Do it normally then
        {
            pkey = PEM_read_bio_PrivateKey(
                bio, nullptr, OTAsymmetricKey::GetPasswordCallback(),
                &thePWData);
        }
        else // Otherwise, use pImportPassword instead of the normal
               // OTCachedKey system.
        {
            pkey = PEM_read_bio_PrivateKey(
                bio, nullptr, 0,
                const_cast<void*>(reinterpret_cast<const void*>(
                    pImportPassword->getPassword())));
        }

        if (nullptr == pkey) {
            otErr << __FUNCTION__ << ": (pImportPassword size: "
                  << (nullptr == pImportPassword
                          ? 0
                          : pImportPassword->getPasswordSize())
                  << ") Error reading private key from string.\n\n";
            return false;
        }
        else {
            // Note: no need to start m_timer here since SetKeyAsCopyOf already
            // calls ArmorPrivateKey, which does that.
            //
            dp->SetKeyAsCopyOf(*pkey, true, &thePWData,
                               pImportPassword); // bIsPrivateKey=false by
                                                 // default, but true
                                                 // here.
            EVP_PKEY_free(pkey);
            pkey = nullptr;
            otLog3 << __FUNCTION__
                   << ": Successfully loaded private key, FYI.\n";
            return true;
        }
    }
}

bool OTAsymmetricKey_OpenSSL::SetPublicKeyFromPrivateKey(
    const String& strCert, const String* pstrReason,
    const OTPassword* pImportPassword)
{
    Release();

    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;

    bool bReturnValue = false;

    // Read public key
    otLog3 << __FUNCTION__
           << ": Reading public key from x509 stored in bookended string...\n";

    String strWithBookends;

    strWithBookends = strCert;

    // took out the +1 on the length since null terminater only
    // needed in string form, not binary form as OpenSSL treats it.
    //
    OpenSSL_BIO keyBio = BIO_new_mem_buf(
        static_cast<void*>(const_cast<char*>(strWithBookends.Get())), -1);
    //    OpenSSL_BIO keyBio =
    // BIO_new_mem_buf(static_cast<void*>(const_cast<char*>(strWithBookends.Get())),
    // strWithBookends.GetLength() /*+1*/);
    //    OpenSSL_BIO keyBio = BIO_new_mem_buf((void*)strCert.Get(),
    // strCert.GetLength() /*+1*/);
    OT_ASSERT(nullptr != keyBio);

    OTPasswordData thePWData(
        nullptr == pImportPassword ? "Enter your wallet master passphrase. "
                                     "(SetPublicKeyFromPrivateKey)"
                                   :
                                   // pImportPassword exists:
            (nullptr == pstrReason
                 ? "Enter the passphrase for your exported Nym. "
                   "(SetPublicKeyFromPrivateKey)"
                 : pstrReason->Get()));

    X509* x509 = nullptr;

    if (nullptr == pImportPassword)
        x509 = PEM_read_bio_X509(keyBio, nullptr,
                                 OTAsymmetricKey::GetPasswordCallback(),
                                 &thePWData);
    else
        x509 = PEM_read_bio_X509(
            keyBio, nullptr, 0, const_cast<void*>(reinterpret_cast<const void*>(
                                    pImportPassword->getPassword())));

    // TODO security: At some point need to switch to using X509_AUX functions.
    // The current x509 functions will read a trust certificate but discard the
    // trust structure.
    // The X509_AUX functions will process the trust structure.
    // UPDATE: Possibly the trust structure sucks. Need to consult experts. (CA
    // system is a farce.)
    //
    if (nullptr != x509) {
        EVP_PKEY* pkey = X509_get_pubkey(x509);

        if (pkey == nullptr) {
            otErr << __FUNCTION__ << ": Error reading public key from x509.\n";
        }
        else {
            dp->SetKeyAsCopyOf(*pkey, false, // bIsPrivateKey=false. PUBLIC KEY.
                               &thePWData, pImportPassword); // pImportPassword
                                                             // is sometimes
                                                             // nullptr here.

            EVP_PKEY_free(pkey);
            pkey = nullptr;
            otLog3 << __FUNCTION__ << ": Successfully extracted a public key "
                                      "from an x509 certificate.\n";
            bReturnValue = true;
        }
    }
    else {
        otErr << __FUNCTION__ << ": Error reading x509 out of certificate.\n";
    }

    // For now we save the x509, and free it in the destructor, since we may
    // need it in the meantime, to convert the Nym to the new master key and
    // re-save. (Don't want to have to load the x509 AGAIN just to re-save
    // it...)
    //
    if (bReturnValue) {
        dp->SetX509(x509);
    }
    else {
        if (nullptr != x509) X509_free(x509);
        x509 = nullptr;
        dp->SetX509(nullptr);
    }

    return bReturnValue;
}

// Used when importing / exporting Nym to/from the wallet.
//
bool OTAsymmetricKey_OpenSSL::ReEncryptPrivateKey(
    const OTPassword& theExportPassword, bool bImporting) const
{
    OT_ASSERT(m_p_ascKey != nullptr);
    OT_ASSERT(IsPrivate());

    bool bReturnVal = false;

    const EVP_CIPHER* pCipher =
        EVP_des_ede3_cbc(); // todo should this algorithm be hardcoded?

    Data theData; // after base64-decoding the ascii-armored string, the
                    // (encrypted) binary will be stored here.

    // This line base64 decodes the ascii-armored string into binary object
    // theData...
    //
    m_p_ascKey->GetData(theData); // theData now contains binary data, the
                                  // encrypted private key itself, no longer in
                                  // text-armoring.

    if (theData.GetSize() > 0) {
        EVP_PKEY* pClearKey = nullptr;

        // Copy the encrypted binary private key data into an OpenSSL memory
        // BIO...
        //
        OpenSSL_BIO keyBio = BIO_new_mem_buf(
            static_cast<char*>(const_cast<void*>(theData.GetPointer())),
            theData.GetSize()); // theData will zeroMemory upon destruction.
        OT_ASSERT_MSG(nullptr != keyBio,
                      "OTAsymmetricKey_OpenSSL::"
                      "ReEncryptPrivateKey: Assert: nullptr != "
                      "keyBio \n");

        // Here's thePWData we use if we didn't have anything else:
        //
        OTPasswordData thePWData(
            bImporting ? "(Importing) Enter the exported Nym's passphrase."
                       : "(Exporting) Enter your wallet's master passphrase.");

        // If we're importing, that means we're currently stored as an EXPORTED
        // NYM (i.e. with its own
        // password, independent of the wallet.) So we use theExportedPassword.
        //
        if (bImporting) {
            //          otOut << "RE-ENCRYPT PRIVATE KEY -- READING using
            // special EXPORT password: %s\n", theExportPassword.getPassword());
            pClearKey = PEM_read_bio_PrivateKey(
                keyBio, nullptr, 0,
                const_cast<void*>(reinterpret_cast<const void*>(
                    theExportPassword.getPassword())));
        }

        // Else if we're exporting, that means we're currently stored in the
        // wallet (i.e. using the wallet's
        // cached master key.) So we use the normal password callback.
        //
        else {
            //          otOut << "RE-ENCRYPT PRIVATE KEY -- READING using WALLET
            // password.\n";
            pClearKey = PEM_read_bio_PrivateKey(
                keyBio, nullptr, OTAsymmetricKey::GetPasswordCallback(),
                &thePWData);
        }

        if (nullptr != pClearKey) {
            //          otLog4 << "%s: Success reading private key from
            // ASCII-armored data:\n\n%s\n\n",
            //                         __FUNCTION__, m_p_ascKey->Get());
            otLog4
                << __FUNCTION__
                << ": Success reading private key from ASCII-armored data.\n";

            // Okay, we have loaded up the private key, now let's save it back
            // to m_p_ascKey
            // using the new passphrase.
            //
            OpenSSL_BIO bmem = BIO_new(BIO_s_mem());
            OT_ASSERT(nullptr != bmem);

            // write a private key to that buffer, from pClearKey
            //
            int32_t nWriteBio = 0;

            // If we're importing, that means we just loaded up the (previously)
            // exported Nym
            // using theExportedPassphrase, so now we need to save it back again
            // using the
            // normal password callback (for importing it to the wallet.)
            //
            if (bImporting) {
                //              otOut << "RE-ENCRYPT PRIVATE KEY -- WRITING
                // using WALLET password.\n";
                nWriteBio = PEM_write_bio_PrivateKey(
                    bmem, pClearKey, pCipher, nullptr, 0,
                    OTAsymmetricKey::GetPasswordCallback(), &thePWData);
            }

            // Else if we're exporting, that means we just loaded up the Nym
            // from the wallet
            // using the normal password callback, and now we need to save it
            // back again using
            // theExportedPassphrase (for exporting it outside of the wallet.)
            //
            else {
                //              otOut << "RE-ENCRYPT PRIVATE KEY -- WRITING
                // using special EXPORT password: %s  Size: %d\n",
                // theExportPassword.getPassword(),
                //                             theExportPassword.getPasswordSize());
                nWriteBio = PEM_write_bio_PrivateKey(
                    bmem, pClearKey, pCipher, nullptr, 0, 0,
                    const_cast<void*>(reinterpret_cast<const void*>(
                        theExportPassword.getPassword())));
            }

            EVP_PKEY_free(pClearKey);

            if (0 == nWriteBio)
                otErr << __FUNCTION__
                      << ": Failed writing EVP_PKEY to memory buffer.\n";
            else {

                otLog5 << __FUNCTION__
                       << ": Success writing EVP_PKEY to memory buffer.\n";

                Data theNewData;
                char* pChar = nullptr;

                // After the below call, pChar will point to the memory buffer
                // where the private key supposedly is,
                // and lSize will contain the size of that memory.
                //
                int64_t lSize = BIO_get_mem_data(bmem, &pChar);
                uint32_t nSize = static_cast<uint32_t>(lSize);

                if (nSize > 0) {
                    // Set the buffer size in our own memory.
                    theNewData.SetSize(nSize);

                    //                  void * pv =
                    OTPassword::safe_memcpy(
                        (static_cast<char*>(const_cast<void*>(
                            theNewData.GetPointer()))), // destination
                        theNewData.GetSize(), // size of destination buffer.
                        pChar,                // source
                        nSize);               // length of source.
                    // bool bZeroSource=false); // if true, sets the source
                    // buffer to zero after copying is done.

                    // This base64 encodes the private key data, which
                    // is already encrypted to its passphase as well.
                    //
                    m_p_ascKey->SetData(theNewData); // <======== Success!
                    bReturnVal = true;
                }
                else
                    otErr << __FUNCTION__
                          << ": Failed copying private key into memory.\n";
            } // (nWriteBio != 0)

        }
        else
            otErr << __FUNCTION__ << ": Failed loading actual private key from "
                                     "BIO containing ASCII-armored data:\n\n"
                  << m_p_ascKey->Get() << "\n\n";
    }
    else
        otErr << __FUNCTION__
              << ": Failed reading private key from ASCII-armored data.\n\n";
    //      otErr << "%s: Failed reading private key from ASCII-armored
    // data:\n\n%s\n\n",
    //                    __FUNCTION__, m_p_ascKey->Get());

    return bReturnVal;
}

// virtual
bool OTAsymmetricKey_OpenSSL::SaveCertToString(
    String& strOutput, const String* pstrReason,
    const OTPassword* pImportPassword) const
{
    X509* x509 = dp->GetX509();

    if (nullptr == x509) {
        otErr << __FUNCTION__
              << ": Error: Unexpected nullptr x509. (Returning false.)\n";
        return false;
    }

    OpenSSL_BIO bio_out_x509 = BIO_new(BIO_s_mem()); // we now have auto-cleanup

    PEM_write_bio_X509(bio_out_x509, x509);

    bool bSuccess = false;

    uint8_t buffer_x509[8192] = ""; // todo hardcoded
    String strx509;
    int32_t len = 0;

    // todo hardcoded 4080 (see array above.)
    //
    if (0 < (len = BIO_read(bio_out_x509, buffer_x509, 8100))) // returns number
                                                               // of bytes
                                                               // successfully
                                                               // read.
    {
        buffer_x509[len] = '\0';
        strx509.Set(reinterpret_cast<const char*>(buffer_x509));

        EVP_PKEY* pPublicKey = X509_get_pubkey(x509);
        if (nullptr != pPublicKey) {
            OTPasswordData thePWData(
                nullptr == pstrReason
                    ? "OTAsymmetricKey_OpenSSL::SaveCertToString"
                    : pstrReason->Get());

            dp->SetKeyAsCopyOf(*pPublicKey, false, &thePWData, pImportPassword);
            EVP_PKEY_free(pPublicKey);
            pPublicKey = nullptr;
        }

        bSuccess = true;
    }

    if (bSuccess) strOutput = strx509;

    return bSuccess;
}

// virtual
bool OTAsymmetricKey_OpenSSL::GetPrivateKey(
    String& strOutput, const OTAsymmetricKey* pPubkey, const String* pstrReason,
    const OTPassword* pImportPassword) const
{
    const EVP_CIPHER* pCipher =
        EVP_des_ede3_cbc(); // todo security (revisit this mode...)

    if (!IsPrivate()) {
        otErr << __FUNCTION__ << ": Error: !IsPrivate() (This function should "
                                 "only be called on a private key.)\n";
        return false;
    }

    EVP_PKEY* pPrivateKey = dp->GetKeyLowLevel();
    if (nullptr == pPrivateKey) {
        otErr
            << __FUNCTION__
            << ": Error: Unexpected nullptr pPrivateKey. (Returning false.)\n";
        return false;
    }

    OpenSSL_BIO bio_out_pri = BIO_new(BIO_s_mem());
    bio_out_pri.setFreeOnly(); // only BIO_free(), not BIO_free_all();

    OTPasswordData thePWData((nullptr != pstrReason)
                                 ? pstrReason->Get()
                                 : "OTAsymmetricKey_OpenSSL::"
                                   "GetPrivateKey is calling "
                                   "PEM_write_bio_PrivateKey...");

    if (nullptr == pImportPassword)
        PEM_write_bio_PrivateKey(bio_out_pri, pPrivateKey, pCipher, nullptr, 0,
                                 OTAsymmetricKey::GetPasswordCallback(),
                                 &thePWData);
    else
        PEM_write_bio_PrivateKey(
            bio_out_pri, pPrivateKey, pCipher, nullptr, 0, 0,
            const_cast<void*>(
                reinterpret_cast<const void*>(pImportPassword->getPassword())));

    bool privateSuccess = false;
    bool publicSuccess = false;

    int32_t len = 0;
    uint8_t buffer_pri[4096] = ""; // todo hardcoded
    String privateKey, publicKey;

    // todo hardcoded 4080 (see array above.)
    if (0 < (len = BIO_read(bio_out_pri, buffer_pri, 4080))) // returns number
                                                             // of bytes
                                                             // successfully
                                                             // read.
    {
        buffer_pri[len] = '\0';
        privateKey.Set(reinterpret_cast<const char*>(buffer_pri));
        privateSuccess = true;
    }
    else
    {
        otErr << __FUNCTION__ << ": Error : key length is not 1 or more!";
    }

    publicSuccess = dynamic_cast<const OTAsymmetricKey_OpenSSL*>(pPubkey)->SaveCertToString(publicKey, pstrReason, pImportPassword);

    if (publicSuccess)
    {
        strOutput.Format(const_cast<char*>("%s%s"), privateKey.Get(),
                         publicKey.Get());
    }
    return privateSuccess && publicSuccess;
}

CryptoAsymmetric& OTAsymmetricKey_OpenSSL::engine() const

{
    return OT::App().Crypto().RSA();
}

serializedAsymmetricKey OTAsymmetricKey_OpenSSL::Serialize() const

{
    serializedAsymmetricKey serializedKey = ot_super::Serialize();

    Data dataKey;
    OT_ASSERT(m_p_ascKey);
    m_p_ascKey->GetData(dataKey);

    if (IsPrivate()) {
        serializedKey->set_mode(proto::KEYMODE_PRIVATE);
    } else {
        serializedKey->set_mode(proto::KEYMODE_PUBLIC);
    }

    serializedKey->set_key(dataKey.GetPointer(), dataKey.GetSize());

    return serializedKey;
}

bool OTAsymmetricKey_OpenSSL::TransportKey(
    Data& publicKey,
    OTPassword& privateKey) const
{
    OT_ASSERT(nullptr != m_p_ascKey);

    if (!IsPrivate()) { return false; }

    Data key, hash;
    m_p_ascKey->GetData(key);

    OT::App().Crypto().Hash().Digest(
        CryptoEngine::StandardHash,
        key,
        hash);
    OTPassword seed;
    seed.setMemory(hash.GetPointer(), hash.GetSize());
    Ecdsa& engine = static_cast<Libsodium&>(OT::App().Crypto().ED25519());

    return engine.SeedToCurveKey(seed, privateKey, publicKey);
}
} // namespace opentxs

#endif // OT_CRYPTO_SUPPORTED_KEY_RSA
