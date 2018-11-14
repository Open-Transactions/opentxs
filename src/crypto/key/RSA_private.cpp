// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Native.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/util/stacktrace.h"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/RSA.hpp"
#include "opentxs/OT.hpp"

#include "internal/api/Internal.hpp"
#if OT_CRYPTO_USING_OPENSSL
#include "crypto/library/OpenSSL_BIO.hpp"
#endif

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#include <cstdint>
#include <ostream>

#include "RSA_private.hpp"

// BIO_get_mem_data() macro from OpenSSL uses old style cast
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define OT_METHOD "opentxs::crypto::key::implementation::RSA::"

namespace opentxs::crypto::key::implementation
{
void RSA::d::SetX509(X509* x509)
{
    if (m_pX509 == x509) return;

    if (nullptr != m_pX509) {
        X509_free(m_pX509);
        m_pX509 = nullptr;
    }

    m_pX509 = x509;
}

void RSA::d::SetKeyAsCopyOf(
    EVP_PKEY& theKey,
    bool bIsPrivateKey,
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    backlink->Release();
    OTPasswordData thePWData(
        !pImportPassword ? "Enter your wallet's master passphrase. "
                           "(RSA::SetKeyAsCopyOf)"
                         : "Enter your exported Nym's passphrase.  "
                           "(RSA::SetKeyAsCopyOf)");

    m_pKey = bIsPrivateKey ? RSA::d::CopyPrivateKey(
                                 theKey,
                                 nullptr == pPWData ? &thePWData : pPWData,
                                 pImportPassword)
                           : RSA::d::CopyPublicKey(
                                 theKey,
                                 nullptr == pPWData ? &thePWData : pPWData,
                                 pImportPassword);
    OT_ASSERT_MSG(
        nullptr != m_pKey,
        "RSA::SetKeyAsCopyOf: "
        "ASSERT: nullptr != m_pKey \n");

    backlink->m_bIsPublicKey = !bIsPrivateKey;
    backlink->m_bIsPrivateKey = bIsPrivateKey;
    backlink->m_p_ascKey->Release();
    // By this point, m_p_ascKey definitely exists, and it's empty.

    if (backlink->m_bIsPrivateKey) {
        RSA::d::ArmorPrivateKey(
            *m_pKey,
            backlink->m_p_ascKey,
            backlink->m_timer,
            nullptr == pPWData ? &thePWData : pPWData,
            pImportPassword);
    }
    // NOTE: Timer is already set INSIDE ArmorPrivateKey. No need to set twice.
    //      m_timer.start(); // Note: this isn't the ultimate timer solution.
    // See notes in ReleaseKeyLowLevel.
    else if (backlink->m_bIsPublicKey) {
        RSA::d::ArmorPublicKey(*m_pKey, backlink->m_p_ascKey);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: This key is NEITHER public NOR private!")
            .Flush();
    }
}

EVP_PKEY* RSA::d::GetKeyLowLevel() const { return m_pKey; }

const EVP_PKEY* RSA::d::GetKey(const OTPasswordData* pPWData)
{
    if (backlink->m_timer.getElapsedTimeInSec() > OT_KEY_TIMER)
        backlink->Release();  // This releases the actual loaded key,
                              // but not the ascii-armored, encrypted
                              // version of it.
    // (Thus forcing a reload, and thus forcing the passphrase to be entered
    // again.)

    if (nullptr == m_pKey)
        return InstantiateKey(pPWData);  // this is the ONLY place, currently,
                                         // that this private method is called.

    return m_pKey;
}

EVP_PKEY* RSA::d::InstantiateKey(const OTPasswordData* pPWData)
{
    if (backlink->IsPublic())
        return InstantiatePublicKey(pPWData);  // this is the ONLY place,
                                               // currently, that this private
                                               // method is called.

    else if (backlink->IsPrivate())
        return InstantiatePrivateKey(pPWData);  // this is the ONLY place,
                                                // currently, that this private
                                                // method is called.

    else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Key is neither public nor private!")
            .Flush();

    return nullptr;
}

EVP_PKEY* RSA::d::CopyPublicKey(
    EVP_PKEY& theKey,
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    // Create a new memory buffer on the OpenSSL side
    crypto::implementation::OpenSSL_BIO bmem = BIO_new(BIO_s_mem());
    OT_ASSERT_MSG(
        nullptr != bmem, "RSA::CopyPublicKey: ASSERT: nullptr != bmem");

    EVP_PKEY* pReturnKey = nullptr;

    // write a public key to that buffer, from theKey (parameter.)
    //
    std::int32_t nWriteBio = PEM_write_bio_PUBKEY(bmem, &theKey);

    if (0 == nWriteBio) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Failed writing EVP_PKEY to memory buffer.")
            .Flush();
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(
            ": Success writing EVP_PKEY to memory buffer.")
            .Flush();
        char* pChar = nullptr;

        // After the below call, pChar will point to the memory buffer where the
        // public key supposedly is, and lSize will contain the size of that
        // memory.
        const std::int64_t lSize = BIO_get_mem_data(bmem, &pChar);
        const std::uint32_t nSize = static_cast<uint32_t>(lSize);

        if (nSize > 0) {
            auto theData = Data::Factory();
            // Set the buffer size in our own memory.
            theData->SetSize(nSize);

            void* pv = OTPassword::safe_memcpy(
                (static_cast<char*>(
                    const_cast<void*>(theData->data()))),  // destination
                theData->size(),  // size of destination buffer.
                pChar,            // source
                nSize);           // length of source.

            if (nullptr != pv) {
                // Next, copy theData's contents into a new BIO_mem_buf,
                // so OpenSSL can load the key out of it.
                //
                crypto::implementation::OpenSSL_BIO keyBio = BIO_new_mem_buf(
                    static_cast<char*>(const_cast<void*>(theData->data())),
                    theData->size());
                OT_ASSERT_MSG(
                    nullptr != keyBio,
                    "RSA::"
                    "CopyPublicKey: Assert: nullptr != "
                    "keyBio \n");

                // Next we load up the key from the BIO string into an
                // instantiated key object.
                //
                OTPasswordData thePWData(
                    nullptr == pImportPassword
                        ? "Enter your wallet master passphrase. "
                          "(RSA::CopyPublicKey is calling "
                          "PEM_read_bio_PUBKEY...)"
                        : "Enter the passphrase for your exported Nym.");

                if (nullptr == pImportPassword) {
                    const auto& native =
                        dynamic_cast<const api::internal::Native&>(OT::App());
                    pReturnKey = PEM_read_bio_PUBKEY(
                        keyBio,
                        nullptr,
                        native.GetInternalPasswordCallback(),
                        nullptr == pPWData
                            ? &thePWData
                            : const_cast<OTPasswordData*>(pPWData));
                } else {
                    pReturnKey = PEM_read_bio_PUBKEY(
                        keyBio,
                        nullptr,
                        0,
                        const_cast<OTPassword*>(pImportPassword));
                }

                // We don't need the BIO anymore.
                // Free the BIO and related buffers, filters, etc. (auto with
                // scope).
                //
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error: Failed copying memory from "
                    "BIO into Data.")
                    .Flush();
            }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed copying private key into memory.")
                .Flush();
        }
    }

    return pReturnKey;
}

// NOTE: OpenSSL will store the EVP_PKEY inside the X509, and when I get it,
// I'm not supposed to destroy the x509 until I destroy the EVP_PKEY FIRST!
// (AND it reference-counts.)
// Since I want ability to destroy the two, independent of each other, I made
// static functions here for copying public and private keys, so I am ALWAYS
// working with MY OWN copy of any given key, and not OpenSSL's
// reference-counted
// one.
//
// Furthermore, BIO_mem_buf doesn't allocate its own memory, but uses the memory
// you pass to it. You CANNOT free that memory until you destroy the BIO.
//
// That's why you see me copying one bio into a payload, before copying it into
// the next bio. Todo security: copy it into an OTPassword here, instead of an
// Data, which is safer, and more appropriate for a private key. Make sure
// OTPassword can accommodate a bit larger size than what it does now.
//
EVP_PKEY* RSA::d::CopyPrivateKey(
    EVP_PKEY& theKey,
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    const EVP_CIPHER* pCipher =
        EVP_des_ede3_cbc();  // todo should this algorithm be hardcoded?

    // Create a new memory buffer on the OpenSSL side
    crypto::implementation::OpenSSL_BIO bmem = BIO_new(BIO_s_mem());
    OT_ASSERT(nullptr != bmem);

    EVP_PKEY* pReturnKey = nullptr;

    // write a private key to that buffer, from theKey
    //
    OTPasswordData thePWDataWrite("RSA::CopyPrivateKey is "
                                  "calling PEM_write_bio_PrivateKey...");

    // todo optimization: might just remove the password callback here, and just
    // write the private key in the clear,
    // and then load it up again, saving the encrypt/decrypt step that otherwise
    // occurs, and then as long as we OpenSSL_cleanse
    // the BIO, then it SHOULD stil be safe, right?
    //
    std::int32_t nWriteBio = false;
    const auto& native = dynamic_cast<const api::internal::Native&>(OT::App());

    if (nullptr == pImportPassword) {
        nWriteBio = PEM_write_bio_PrivateKey(
            bmem,
            &theKey,
            pCipher,
            nullptr,
            0,
            native.GetInternalPasswordCallback(),
            nullptr == pPWData ? &thePWDataWrite
                               : const_cast<OTPasswordData*>(pPWData));
    } else {
        nWriteBio = PEM_write_bio_PrivateKey(
            bmem,
            &theKey,
            pCipher,
            nullptr,
            0,
            0,
            const_cast<void*>(
                reinterpret_cast<const void*>(pImportPassword->getPassword())));
    }

    if (0 == nWriteBio) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed writing EVP_PKEY to memory buffer.")
            .Flush();
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(
            ": Success writing EVP_PKEY to memory buffer.")
            .Flush();
        char* pChar = nullptr;

        // After the below call, pChar will point to the memory buffer where the
        // private key supposedly is,
        // and lSize will contain the size of that memory.
        //
        const std::int64_t lSize = BIO_get_mem_data(bmem, &pChar);
        const std::uint32_t nSize = static_cast<uint32_t>(lSize);

        if (nSize > 0) {
            auto theData = Data::Factory();

            // Set the buffer size in our own memory.
            theData->SetSize(nSize);

            void* pv = OTPassword::safe_memcpy(
                (static_cast<char*>(
                    const_cast<void*>(theData->data()))),  // destination
                theData->size(),  // size of destination buffer.
                pChar,            // source
                nSize);           // length of source.
            // bool bZeroSource=false); // if true, sets the source buffer to
            // zero after copying is done.

            if (nullptr != pv) {

                // Next, copy theData's contents into a new BIO_mem_buf,
                // so OpenSSL can load the key out of it.
                //
                crypto::implementation::OpenSSL_BIO keyBio = BIO_new_mem_buf(
                    static_cast<char*>(const_cast<void*>(theData->data())),
                    theData->size());
                OT_ASSERT_MSG(
                    nullptr != keyBio,
                    "RSA::"
                    "CopyPrivateKey: Assert: nullptr != "
                    "keyBio \n");

                // Next we load up the key from the BIO string into an
                // instantiated key object.
                //
                OTPasswordData thePWData("RSA::"
                                         "CopyPrivateKey is calling "
                                         "PEM_read_bio_PUBKEY...");
                const auto& native =
                    dynamic_cast<const api::internal::Native&>(OT::App());

                if (nullptr == pImportPassword) {
                    pReturnKey = PEM_read_bio_PrivateKey(
                        keyBio,
                        nullptr,
                        native.GetInternalPasswordCallback(),
                        nullptr == pPWData
                            ? &thePWData
                            : const_cast<OTPasswordData*>(pPWData));
                } else {
                    pReturnKey = PEM_read_bio_PrivateKey(
                        keyBio,
                        nullptr,
                        0,
                        const_cast<void*>(reinterpret_cast<const void*>(
                            pImportPassword->getPassword())));
                }
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error: Failed copying memory from "
                    "BIO into Data.")
                    .Flush();
            }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed copying private key into memory.")
                .Flush();
        }
    }

    return pReturnKey;
}

// Take a public key, theKey (input), and create an armored version of
// it into ascKey (output.)
//
// OpenSSL loaded key ===> ASCII-Armored export of same key.
//
bool RSA::d::ArmorPublicKey(EVP_PKEY& theKey, Armored& ascKey)
{
    bool bReturnVal = false;

    ascKey.Release();

    // Create a new memory buffer on the OpenSSL side
    crypto::implementation::OpenSSL_BIO bmem = BIO_new(BIO_s_mem());
    OT_ASSERT_MSG(
        nullptr != bmem, "RSA::ArmorPublicKey: ASSERT: nullptr != bmem");

    // write a public key to that buffer, from theKey (parameter.)
    //
    std::int32_t nWriteBio = PEM_write_bio_PUBKEY(bmem, &theKey);

    if (0 == nWriteBio) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Failed writing EVP_PKEY to memory buffer.")
            .Flush();
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(
            ": Success writing EVP_PKEY to memory buffer.")
            .Flush();
        auto theData = Data::Factory();
        char* pChar = nullptr;

        // After the below call, pChar will point to the memory buffer where the
        // public key
        // supposedly is, and lSize will contain the size of that memory.
        //
        std::int64_t lSize = BIO_get_mem_data(bmem, &pChar);
        std::uint32_t nSize = static_cast<uint32_t>(
            lSize);  // todo security, etc. Fix this assumed type conversion.

        if (nSize > 0) {
            // Set the buffer size in our own memory.
            theData->SetSize(nSize);

            //            void * pv =
            OTPassword::safe_memcpy(
                (static_cast<char*>(
                    const_cast<void*>(theData->data()))),  // destination
                theData->size(),  // size of destination buffer.
                pChar,            // source
                nSize);           // length of source.
            // bool bZeroSource=false); // if true, sets the source buffer to
            // zero after copying is done.

            // This base64 encodes the public key data
            //
            ascKey.SetData(theData);
            LogInsane(OT_METHOD)(__FUNCTION__)(
                ": Success copying public key into memory.")
                .Flush();
            bReturnVal = true;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed copying public key into memory.")
                .Flush();
        }
    }

    return bReturnVal;
}

// (Internal) ASCII-Armored key ====> (Internal) Actual loaded OpenSSL key.
//
EVP_PKEY* RSA::d::InstantiatePublicKey(const OTPasswordData* pPWData)
{
    OT_ASSERT(m_pKey == nullptr);
    OT_ASSERT(backlink->IsPublic());

    EVP_PKEY* pReturnKey = nullptr;
    auto theData = Data::Factory();

    // This base64 decodes the string m_p_ascKey into the
    // binary payload object "theData"
    //
    backlink->m_p_ascKey->GetData(theData);

    if (theData->size() > 0) {

        // Next, copy theData's contents into a new BIO_mem_buf,
        // so OpenSSL can load the key out of it.
        //
        crypto::implementation::OpenSSL_BIO keyBio = BIO_new_mem_buf(
            static_cast<char*>(const_cast<void*>(theData->data())),
            theData->size());
        OT_ASSERT_MSG(
            nullptr != keyBio,
            "RSA::"
            "InstantiatePublicKey: Assert: nullptr != "
            "keyBio \n");

        // Next we load up the key from the BIO string into an instantiated key
        // object.
        //
        OTPasswordData thePWData("RSA::"
                                 "InstantiatePublicKey is calling "
                                 "PEM_read_bio_PUBKEY...");

        if (nullptr == pPWData) { pPWData = &thePWData; }

        const auto& native =
            dynamic_cast<const api::internal::Native&>(OT::App());
        pReturnKey = PEM_read_bio_PUBKEY(
            keyBio,
            nullptr,
            native.GetInternalPasswordCallback(),
            const_cast<OTPasswordData*>(pPWData));

        backlink->Release();  // Release whatever loaded key I might
                              // have already had.

        if (nullptr != pReturnKey) {
            m_pKey = pReturnKey;
            LogTrace(OT_METHOD)(__FUNCTION__)(
                ": Success reading public key from ASCII-armored data:\n")(
                backlink->m_p_ascKey->Get())
                .Flush();

            return m_pKey;
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Failed reading public key from ASCII-armored data: ")(
        backlink->m_p_ascKey->Get())(".")
        .Flush();
    return nullptr;
}

EVP_PKEY* RSA::d::InstantiatePrivateKey(const OTPasswordData* pPWData)
{
    OT_ASSERT(m_pKey == nullptr);
    OT_ASSERT(backlink->IsPrivate());

    EVP_PKEY* pReturnKey = nullptr;
    // after base64-decoding the ascii-armored string, the (encrypted) binary
    // will be stored here.
    auto theData = Data::Factory();
    // --------------------------------------
    // This line base64 decodes the ascii-armored string into binary object
    // theData...
    //
    backlink->m_p_ascKey->GetData(theData);  // theData now contains binary
                                             // data, the encrypted private key
                                             // itself, no longer in
                                             // text-armoring.
    //
    // Note, for future optimization: the ASCII-ARMORING could be used for
    // serialization, but the BIO (still encrypted)
    // could be used in RAM for this object. Otherwise you just have to do the
    // extra step of ascii-decoding it first to get
    // the BIO, before being able to instantiate the key itself from that. That
    // final step can't change, but I can remove
    // the step before it, in most cases, by just storing the BIO itself,
    // instead of the ascii-armored string. Or perhaps
    // make them both available...hm.

    // Copy the encrypted binary private key data into an OpenSSL memory BIO...
    //
    if (theData->size() > 0) {
        crypto::implementation::OpenSSL_BIO keyBio = BIO_new_mem_buf(
            static_cast<char*>(const_cast<void*>(theData->data())),
            theData->size());  // theData will zeroMemory upon destruction.
        OT_ASSERT_MSG(
            nullptr != keyBio,
            "RSA::"
            "InstantiatePrivateKey: Assert: nullptr != "
            "keyBio \n");

        // Here's thePWData we use if we didn't have anything else:
        //
        OTPasswordData thePWData("RSA::"
                                 "InstantiatePrivateKey is calling "
                                 "PEM_read_bio_PrivateKey...");

        if (nullptr == pPWData) { pPWData = &thePWData; }

        const auto& native =
            dynamic_cast<const api::internal::Native&>(OT::App());
        pReturnKey = PEM_read_bio_PrivateKey(
            keyBio,
            nullptr,
            native.GetInternalPasswordCallback(),
            const_cast<OTPasswordData*>(pPWData));

        // Free the BIO and related buffers, filters, etc.
        backlink->Release();

        if (nullptr != pReturnKey) {
            m_pKey = pReturnKey;
            // TODO (remove theTimer entirely. OTCachedKey replaces already.)
            // I set this timer because the above required a password. But now
            // that master key is working,
            // the above would flow through even WITHOUT the user typing his
            // passphrase (since master key still
            // not timed out.) Resulting in THIS timer being reset!  Todo: I
            // already shortened this timer to 30
            // seconds, but need to phase it down to 0 and then remove it
            // entirely! Master key takes over now!
            //

            backlink->m_timer.start();  // Note: this isn't the ultimate timer
                                        // solution. See notes in
                                        // ReleaseKeyLowLevel.
            LogTrace(OT_METHOD)(__FUNCTION__)(
                ": Success reading private key from ASCII-armored data.")
                .Flush();

            return m_pKey;
        }
    }
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Failed reading private key from ASCII-armored data.")
        .Flush();
    OT_ASSERT(false);
    //  otErr << __FUNCTION__ << ": Failed reading private key from
    // ASCII-armored data:\n\n" << m_p_ascKey->Get() << "\n\n";
    return nullptr;
}

bool RSA::d::ArmorPrivateKey(
    EVP_PKEY& theKey,
    Armored& ascKey,
    Timer& theTimer,
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    bool bReturnVal = false;

    ascKey.Release();

    // Create a new memory buffer on the OpenSSL side
    crypto::implementation::OpenSSL_BIO bmem = BIO_new(BIO_s_mem());
    OT_ASSERT(nullptr != bmem);

    // write a private key to that buffer, from theKey
    //
    OTPasswordData thePWData("RSA::ArmorPrivateKey is "
                             "calling PEM_write_bio_PrivateKey...");

    if (nullptr == pPWData) pPWData = &thePWData;

    std::int32_t nWriteBio = 0;
    const auto& native = dynamic_cast<const api::internal::Native&>(OT::App());

    if (nullptr == pImportPassword) {
        nWriteBio = PEM_write_bio_PrivateKey(
            bmem,
            &theKey,
            EVP_des_ede3_cbc(),  // todo should this algorithm be hardcoded?
            nullptr,
            0,
            native.GetInternalPasswordCallback(),
            const_cast<OTPasswordData*>(pPWData));
    } else {
        nWriteBio = PEM_write_bio_PrivateKey(
            bmem,
            &theKey,
            EVP_des_ede3_cbc(),  // todo should this algorithm be hardcoded?
            nullptr,
            0,
            0,
            const_cast<void*>(
                reinterpret_cast<const void*>(pImportPassword->getPassword())));
    }

    if (0 == nWriteBio) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed writing EVP_PKEY to memory buffer.")
            .Flush();
    } else {
        // TODO (remove theTimer entirely. OTCachedKey replaces already.)
        // I set this timer because the above required a password. But now that
        // master key is working,
        // the above would flow through even WITHOUT the user typing his
        // passphrase (since master key still
        // not timed out.) Resulting in THIS timer being reset!  Todo: I already
        // shortened this timer to 30
        // seconds, but need to phase it down to 0 and then remove it entirely!
        // Master key takes over now!
        //

        theTimer.start();  // Note: this isn't the ultimate timer solution. See
                           // notes in ReleaseKeyLowLevel.

        LogInsane(OT_METHOD)(__FUNCTION__)(
            ": Success writing EVP_PKEY to memory buffer.")
            .Flush();

        auto theData = Data::Factory();
        char* pChar = nullptr;

        // After the below call, pChar will point to the memory buffer where the
        // private key supposedly is,
        // and lSize will contain the size of that memory.
        //
        std::int64_t lSize = BIO_get_mem_data(bmem, &pChar);
        std::uint32_t nSize = static_cast<uint32_t>(lSize);

        if (nSize > 0) {
            // Set the buffer size in our own memory.
            theData->SetSize(nSize);

            //            void * pv =
            OTPassword::safe_memcpy(
                (static_cast<char*>(
                    const_cast<void*>(theData->data()))),  // destination
                theData->size(),  // size of destination buffer.
                pChar,            // source
                nSize);           // length of source.
            // bool bZeroSource=false); // if true, sets the source buffer to
            // zero after copying is done.

            // This base64 encodes the private key data, which
            // is already encrypted to its passphase as well.
            //
            ascKey.SetData(theData);
            LogInsane(OT_METHOD)(__FUNCTION__)(
                ": Success copying private key into memory.")
                .Flush();
            bReturnVal = true;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed copying private key into memory.")
                .Flush();
        }
    }

    return bReturnVal;
}
}  // namespace opentxs::crypto::key::implementation

#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
