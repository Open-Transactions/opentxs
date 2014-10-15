/************************************************************
 *
 *  OTAsymmetricKeyOpenSSLPrivdp.cpp
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
#include <opentxs/core/crypto/OTAsymmetricKey_OpenSSLPrivdp.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/OTData.hpp>
#include <opentxs/core/crypto/OpenSSL_BIO.hpp>
#include <opentxs/core/util/stacktrace.h>

// BIO_get_mem_data() macro from OpenSSL uses old style cast
#pragma GCC diagnostic ignored "-Wold-style-cast"

namespace opentxs
{

void OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::SetX509(X509* x509)
{
    if (m_pX509 == x509) return;

    if (nullptr != m_pX509) {
        X509_free(m_pX509);
        m_pX509 = nullptr;
    }

    m_pX509 = x509;
}

void OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::SetKeyAsCopyOf(
    EVP_PKEY& theKey, bool bIsPrivateKey, const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    backlink->Release();
    OTPasswordData thePWData(!pImportPassword
                                 ? "Enter your wallet's master passphrase. "
                                   "(OTAsymmetricKey_OpenSSL::SetKeyAsCopyOf)"
                                 : "Enter your exported Nym's passphrase.  "
                                   "(OTAsymmetricKey_OpenSSL::SetKeyAsCopyOf)");

    m_pKey = bIsPrivateKey
                 ? OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::
                       CopyPrivateKey(theKey,
                                      nullptr == pPWData ? &thePWData : pPWData,
                                      pImportPassword)
                 : OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::
                       CopyPublicKey(theKey,
                                     nullptr == pPWData ? &thePWData : pPWData,
                                     pImportPassword);
    OT_ASSERT_MSG(nullptr != m_pKey, "OTAsymmetricKey_OpenSSL::SetKeyAsCopyOf: "
                                     "ASSERT: nullptr != m_pKey \n");

    backlink->m_bIsPublicKey = !bIsPrivateKey;
    backlink->m_bIsPrivateKey = bIsPrivateKey;

    if (nullptr == backlink->m_p_ascKey) {
        backlink->m_p_ascKey = new OTASCIIArmor;
        OT_ASSERT(nullptr != backlink->m_p_ascKey);
    }
    else {
        backlink->m_p_ascKey->Release();
    }
    // By this point, m_p_ascKey definitely exists, and it's empty.

    if (backlink->m_bIsPrivateKey) {
        OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::ArmorPrivateKey(
            *m_pKey, *backlink->m_p_ascKey, backlink->m_timer,
            nullptr == pPWData ? &thePWData : pPWData, pImportPassword);
    }
    // NOTE: Timer is already set INSIDE ArmorPrivateKey. No need to set twice.
    //      m_timer.start(); // Note: this isn't the ultimate timer solution.
    // See notes in ReleaseKeyLowLevel.
    else if (backlink->m_bIsPublicKey) {
        OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::ArmorPublicKey(
            *m_pKey, *backlink->m_p_ascKey);
    }
    else {
        otErr << __FUNCTION__
              << ": Error: This key is NEITHER public NOR private!\n";
    }
}

EVP_PKEY* OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::
    GetKeyLowLevel() const
{
    return m_pKey;
}

const EVP_PKEY* OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::GetKey(
    const OTPasswordData* pPWData)
{
    OT_ASSERT_MSG(nullptr != backlink->m_p_ascKey,
                  "OTAsymmetricKey_OpenSSL::GetKey: nullptr != m_p_ascKey\n");

    if (nullptr == backlink->m_p_ascKey) {
        otErr << __FUNCTION__
              << ": Unexpected nullptr m_p_ascKey. Printing stack "
                 "trace (and returning nullptr):\n";
        print_stacktrace();
        return nullptr;
    }

    if (backlink->m_timer.getElapsedTimeInSec() > OT_KEY_TIMER)
        backlink->ReleaseKeyLowLevel(); // This releases the actual loaded key,
                                        // but not the ascii-armored, encrypted
                                        // version of it.
    // (Thus forcing a reload, and thus forcing the passphrase to be entered
    // again.)

    if (nullptr == m_pKey)
        return InstantiateKey(pPWData); // this is the ONLY place, currently,
                                        // that this private method is called.

    return m_pKey;
}

EVP_PKEY* OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::
    InstantiateKey(const OTPasswordData* pPWData)
{
    if (backlink->IsPublic())
        return InstantiatePublicKey(pPWData); // this is the ONLY place,
                                              // currently, that this private
                                              // method is called.

    else if (backlink->IsPrivate())
        return InstantiatePrivateKey(pPWData); // this is the ONLY place,
                                               // currently, that this private
                                               // method is called.

    else
        otErr << "OTAsymmetricKey_OpenSSL::InstantiateKey: Error: Key is "
                 "neither public nor private!\n";

    return nullptr;
}

EVP_PKEY* OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::CopyPublicKey(
    EVP_PKEY& theKey, const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    // Create a new memory buffer on the OpenSSL side
    OpenSSL_BIO bmem = BIO_new(BIO_s_mem());
    OT_ASSERT_MSG(
        nullptr != bmem,
        "OTAsymmetricKey_OpenSSL::CopyPublicKey: ASSERT: nullptr != bmem");

    EVP_PKEY* pReturnKey = nullptr;

    // write a public key to that buffer, from theKey (parameter.)
    //
    int32_t nWriteBio = PEM_write_bio_PUBKEY(bmem, &theKey);

    if (0 == nWriteBio) {
        otErr << __FUNCTION__
              << ": Error: Failed writing EVP_PKEY to memory buffer.\n";
    }
    else {
        otLog5 << __FUNCTION__
               << ": Success writing EVP_PKEY to memory buffer.\n";

        char* pChar = nullptr;

        // After the below call, pChar will point to the memory buffer where the
        // public key
        // supposedly is, and lSize will contain the size of that memory.
        //
        const int64_t lSize = BIO_get_mem_data(bmem, &pChar);
        const uint32_t nSize = static_cast<uint32_t>(lSize);

        if (nSize > 0) {
            OTData theData;

            // Set the buffer size in our own memory.
            theData.SetSize(nSize);

            void* pv = OTPassword::safe_memcpy(
                (static_cast<char*>(
                    const_cast<void*>(theData.GetPointer()))), // destination
                theData.GetSize(), // size of destination buffer.
                pChar,             // source
                nSize);            // length of source.

            if (nullptr != pv) {
                // Next, copy theData's contents into a new BIO_mem_buf,
                // so OpenSSL can load the key out of it.
                //
                OpenSSL_BIO keyBio = BIO_new_mem_buf(
                    static_cast<char*>(const_cast<void*>(theData.GetPointer())),
                    theData.GetSize());
                OT_ASSERT_MSG(nullptr != keyBio,
                              "OTAsymmetricKey_OpenSSL::"
                              "CopyPublicKey: Assert: nullptr != "
                              "keyBio \n");

                // Next we load up the key from the BIO string into an
                // instantiated key object.
                //
                OTPasswordData thePWData(
                    nullptr == pImportPassword
                        ? "Enter your wallet master passphrase. "
                          "(OTAsymmetricKey_OpenSSL::CopyPublicKey is calling "
                          "PEM_read_bio_PUBKEY...)"
                        : "Enter the passphrase for your exported Nym.");

                if (nullptr == pImportPassword)
                    pReturnKey = PEM_read_bio_PUBKEY(
                        keyBio, nullptr, OTAsymmetricKey::GetPasswordCallback(),
                        nullptr == pPWData
                            ? &thePWData
                            : const_cast<OTPasswordData*>(pPWData));
                else
                    pReturnKey = PEM_read_bio_PUBKEY(
                        keyBio, nullptr, 0,
                        const_cast<OTPassword*>(pImportPassword));

                // We don't need the BIO anymore.
                // Free the BIO and related buffers, filters, etc. (auto with
                // scope).
                //
            }
            else {
                otErr << __FUNCTION__ << ": Error: Failed copying memory from "
                                         "BIO into OTData.\n";
            }
        }
        else {
            otErr << __FUNCTION__
                  << ": Failed copying private key into memory.\n";
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
// OTData, which is safer, and more appropriate for a private key. Make sure
// OTPassword can accommodate a bit larger size than what it does now.
//
EVP_PKEY* OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::
    CopyPrivateKey(EVP_PKEY& theKey, const OTPasswordData* pPWData,
                   const OTPassword* pImportPassword)
{
    const EVP_CIPHER* pCipher =
        EVP_des_ede3_cbc(); // todo should this algorithm be hardcoded?

    // Create a new memory buffer on the OpenSSL side
    OpenSSL_BIO bmem = BIO_new(BIO_s_mem());
    OT_ASSERT(nullptr != bmem);

    EVP_PKEY* pReturnKey = nullptr;

    // write a private key to that buffer, from theKey
    //
    OTPasswordData thePWDataWrite("OTAsymmetricKey_OpenSSL::CopyPrivateKey is "
                                  "calling PEM_write_bio_PrivateKey...");

    // todo optimization: might just remove the password callback here, and just
    // write the private key in the clear,
    // and then load it up again, saving the encrypt/decrypt step that otherwise
    // occurs, and then as long as we OpenSSL_cleanse
    // the BIO, then it SHOULD stil be safe, right?
    //
    int32_t nWriteBio = false;

    if (nullptr == pImportPassword)
        nWriteBio = PEM_write_bio_PrivateKey(
            bmem, &theKey, pCipher, nullptr, 0,
            OTAsymmetricKey::GetPasswordCallback(),
            nullptr == pPWData ? &thePWDataWrite
                               : const_cast<OTPasswordData*>(pPWData));
    else
        nWriteBio = PEM_write_bio_PrivateKey(
            bmem, &theKey, pCipher, nullptr, 0, 0,
            const_cast<void*>(
                reinterpret_cast<const void*>(pImportPassword->getPassword())));

    if (0 == nWriteBio) {
        otErr << __FUNCTION__
              << ": Failed writing EVP_PKEY to memory buffer.\n";
    }
    else {
        otLog5 << __FUNCTION__
               << ": Success writing EVP_PKEY to memory buffer.\n";

        char* pChar = nullptr;

        // After the below call, pChar will point to the memory buffer where the
        // private key supposedly is,
        // and lSize will contain the size of that memory.
        //
        const int64_t lSize = BIO_get_mem_data(bmem, &pChar);
        const uint32_t nSize = static_cast<uint32_t>(lSize);

        if (nSize > 0) {
            OTData theData;

            // Set the buffer size in our own memory.
            theData.SetSize(nSize);

            void* pv = OTPassword::safe_memcpy(
                (static_cast<char*>(
                    const_cast<void*>(theData.GetPointer()))), // destination
                theData.GetSize(), // size of destination buffer.
                pChar,             // source
                nSize);            // length of source.
            // bool bZeroSource=false); // if true, sets the source buffer to
            // zero after copying is done.

            if (nullptr != pv) {

                // Next, copy theData's contents into a new BIO_mem_buf,
                // so OpenSSL can load the key out of it.
                //
                OpenSSL_BIO keyBio = BIO_new_mem_buf(
                    static_cast<char*>(const_cast<void*>(theData.GetPointer())),
                    theData.GetSize());
                OT_ASSERT_MSG(nullptr != keyBio,
                              "OTAsymmetricKey_OpenSSL::"
                              "CopyPrivateKey: Assert: nullptr != "
                              "keyBio \n");

                // Next we load up the key from the BIO string into an
                // instantiated key object.
                //
                OTPasswordData thePWData("OTAsymmetricKey_OpenSSL::"
                                         "CopyPrivateKey is calling "
                                         "PEM_read_bio_PUBKEY...");

                if (nullptr == pImportPassword)
                    pReturnKey = PEM_read_bio_PrivateKey(
                        keyBio, nullptr, OTAsymmetricKey::GetPasswordCallback(),
                        nullptr == pPWData
                            ? &thePWData
                            : const_cast<OTPasswordData*>(pPWData));
                else
                    pReturnKey = PEM_read_bio_PrivateKey(
                        keyBio, nullptr, 0,
                        const_cast<void*>(reinterpret_cast<const void*>(
                            pImportPassword->getPassword())));

            }
            else
                otErr << __FUNCTION__ << ": Error: Failed copying memory from "
                                         "BIO into OTData.\n";

        }
        else {
            otErr << __FUNCTION__
                  << ": Failed copying private key into memory.\n";
        }
    }

    return pReturnKey;
}

// Take a public key, theKey (input), and create an armored version of
// it into ascKey (output.)
//
// OpenSSL loaded key ===> ASCII-Armored export of same key.
//
bool OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::ArmorPublicKey(
    EVP_PKEY& theKey, OTASCIIArmor& ascKey)
{
    bool bReturnVal = false;

    const char* szFunc = "OTAsymmetricKey_OpenSSL::ArmorPublicKey";

    ascKey.Release();

    // Create a new memory buffer on the OpenSSL side
    OpenSSL_BIO bmem = BIO_new(BIO_s_mem());
    OT_ASSERT_MSG(
        nullptr != bmem,
        "OTAsymmetricKey_OpenSSL::ArmorPublicKey: ASSERT: nullptr != bmem");

    // write a public key to that buffer, from theKey (parameter.)
    //
    int32_t nWriteBio = PEM_write_bio_PUBKEY(bmem, &theKey);

    if (0 == nWriteBio) {
        otErr << szFunc
              << ": Error: Failed writing EVP_PKEY to memory buffer.\n";
    }
    else {
        otLog5 << szFunc << ": Success writing EVP_PKEY to memory buffer.\n";

        OTData theData;
        char* pChar = nullptr;

        // After the below call, pChar will point to the memory buffer where the
        // public key
        // supposedly is, and lSize will contain the size of that memory.
        //
        int64_t lSize = BIO_get_mem_data(bmem, &pChar);
        uint32_t nSize = static_cast<uint32_t>(
            lSize); // todo security, etc. Fix this assumed type conversion.

        if (nSize > 0) {
            // Set the buffer size in our own memory.
            theData.SetSize(nSize);

            //            void * pv =
            OTPassword::safe_memcpy(
                (static_cast<char*>(
                    const_cast<void*>(theData.GetPointer()))), // destination
                theData.GetSize(), // size of destination buffer.
                pChar,             // source
                nSize);            // length of source.
            // bool bZeroSource=false); // if true, sets the source buffer to
            // zero after copying is done.

            // This base64 encodes the public key data
            //
            ascKey.SetData(theData);

            otLog5 << szFunc << ": Success copying public key into memory.\n";
            bReturnVal = true;
        }
        else {
            otErr << szFunc << ": Failed copying public key into memory.\n";
        }
    }

    return bReturnVal;
}

// (Internal) ASCII-Armored key ====> (Internal) Actual loaded OpenSSL key.
//
EVP_PKEY* OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::
    InstantiatePublicKey(const OTPasswordData* pPWData)
{
    OT_ASSERT(m_pKey == nullptr);
    OT_ASSERT(backlink->m_p_ascKey != nullptr);
    OT_ASSERT(backlink->IsPublic());

    const char* szFunc = "OTAsymmetricKey_OpenSSL::InstantiatePublicKey";

    EVP_PKEY* pReturnKey = nullptr;
    OTData theData;

    // This base64 decodes the string m_p_ascKey into the
    // binary payload object "theData"
    //
    backlink->m_p_ascKey->GetData(theData);

    if (theData.GetSize() > 0) {

        // Next, copy theData's contents into a new BIO_mem_buf,
        // so OpenSSL can load the key out of it.
        //
        OpenSSL_BIO keyBio = BIO_new_mem_buf(
            static_cast<char*>(const_cast<void*>(theData.GetPointer())),
            theData.GetSize());
        OT_ASSERT_MSG(nullptr != keyBio,
                      "OTAsymmetricKey_OpenSSL::"
                      "InstantiatePublicKey: Assert: nullptr != "
                      "keyBio \n");

        // Next we load up the key from the BIO string into an instantiated key
        // object.
        //
        OTPasswordData thePWData("OTAsymmetricKey_OpenSSL::"
                                 "InstantiatePublicKey is calling "
                                 "PEM_read_bio_PUBKEY...");

        if (nullptr == pPWData) pPWData = &thePWData;

        pReturnKey = PEM_read_bio_PUBKEY(keyBio, nullptr,
                                         OTAsymmetricKey::GetPasswordCallback(),
                                         const_cast<OTPasswordData*>(pPWData));

        backlink->ReleaseKeyLowLevel(); // Release whatever loaded key I might
                                        // have already had.

        if (nullptr != pReturnKey) {
            m_pKey = pReturnKey;
            otLog4
                << szFunc
                << ": Success reading public key from ASCII-armored data:\n\n"
                << backlink->m_p_ascKey->Get() << "\n\n";
            return m_pKey;
        }
    }

    otErr << szFunc
          << ": Failed reading public key from ASCII-armored data:\n\n"
          << backlink->m_p_ascKey->Get() << "\n\n";
    return nullptr;
}

EVP_PKEY* OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::
    InstantiatePrivateKey(const OTPasswordData* pPWData)
{
    OT_ASSERT(m_pKey == nullptr);
    OT_ASSERT(backlink->m_p_ascKey != nullptr);
    OT_ASSERT(backlink->IsPrivate());

    EVP_PKEY* pReturnKey = nullptr;
    OTData theData; // after base64-decoding the ascii-armored string, the
                    // (encrypted) binary will be stored here.
    // --------------------------------------
    // This line base64 decodes the ascii-armored string into binary object
    // theData...
    //
    backlink->m_p_ascKey->GetData(theData); // theData now contains binary data,
                                            // the encrypted private key itself,
                                            // no longer in text-armoring.
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
    if (theData.GetSize() > 0) {
        OpenSSL_BIO keyBio = BIO_new_mem_buf(
            static_cast<char*>(const_cast<void*>(theData.GetPointer())),
            theData.GetSize()); // theData will zeroMemory upon destruction.
        OT_ASSERT_MSG(nullptr != keyBio,
                      "OTAsymmetricKey_OpenSSL::"
                      "InstantiatePrivateKey: Assert: nullptr != "
                      "keyBio \n");

        // Here's thePWData we use if we didn't have anything else:
        //
        OTPasswordData thePWData("OTAsymmetricKey_OpenSSL::"
                                 "InstantiatePrivateKey is calling "
                                 "PEM_read_bio_PrivateKey...");

        if (nullptr == pPWData) pPWData = &thePWData;

        pReturnKey = PEM_read_bio_PrivateKey(
            keyBio, nullptr, OTAsymmetricKey::GetPasswordCallback(),
            const_cast<OTPasswordData*>(pPWData));

        // Free the BIO and related buffers, filters, etc.
        backlink->ReleaseKeyLowLevel();

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

            backlink->m_timer.start(); // Note: this isn't the ultimate timer
                                       // solution. See notes in
                                       // ReleaseKeyLowLevel.
            otLog4
                << __FUNCTION__
                << ": Success reading private key from ASCII-armored data.\n\n";
            //          otLog4 << __FUNCTION__ << ": Success reading private key
            // from ASCII-armored data:\n\n" << m_p_ascKey->Get() << "\n\n";
            return m_pKey;
        }
    }
    otErr << __FUNCTION__
          << ": Failed reading private key from ASCII-armored data.\n\n";
    //  otErr << __FUNCTION__ << ": Failed reading private key from
    // ASCII-armored data:\n\n" << m_p_ascKey->Get() << "\n\n";
    return nullptr;
}

bool OTAsymmetricKey_OpenSSL::OTAsymmetricKey_OpenSSLPrivdp::ArmorPrivateKey(
    EVP_PKEY& theKey, OTASCIIArmor& ascKey, Timer& theTimer,
    const OTPasswordData* pPWData, const OTPassword* pImportPassword)
{
    bool bReturnVal = false;

    ascKey.Release();

    // Create a new memory buffer on the OpenSSL side
    OpenSSL_BIO bmem = BIO_new(BIO_s_mem());
    OT_ASSERT(nullptr != bmem);

    // write a private key to that buffer, from theKey
    //
    OTPasswordData thePWData("OTAsymmetricKey_OpenSSL::ArmorPrivateKey is "
                             "calling PEM_write_bio_PrivateKey...");

    if (nullptr == pPWData) pPWData = &thePWData;

    int32_t nWriteBio = 0;

    if (nullptr == pImportPassword)
        nWriteBio = PEM_write_bio_PrivateKey(
            bmem, &theKey,
            EVP_des_ede3_cbc(), // todo should this algorithm be hardcoded?
            nullptr, 0, OTAsymmetricKey::GetPasswordCallback(),
            const_cast<OTPasswordData*>(pPWData));
    else
        nWriteBio = PEM_write_bio_PrivateKey(
            bmem, &theKey,
            EVP_des_ede3_cbc(), // todo should this algorithm be hardcoded?
            nullptr, 0, 0, const_cast<void*>(reinterpret_cast<const void*>(
                               pImportPassword->getPassword())));

    if (0 == nWriteBio) {
        otErr << __FUNCTION__
              << ": Failed writing EVP_PKEY to memory buffer.\n";
    }
    else {
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

        theTimer.start(); // Note: this isn't the ultimate timer solution. See
                          // notes in ReleaseKeyLowLevel.

        otLog5 << __FUNCTION__
               << ": Success writing EVP_PKEY to memory buffer.\n";

        OTData theData;
        char* pChar = nullptr;

        // After the below call, pChar will point to the memory buffer where the
        // private key supposedly is,
        // and lSize will contain the size of that memory.
        //
        int64_t lSize = BIO_get_mem_data(bmem, &pChar);
        uint32_t nSize = static_cast<uint32_t>(lSize);

        if (nSize > 0) {
            // Set the buffer size in our own memory.
            theData.SetSize(nSize);

            //            void * pv =
            OTPassword::safe_memcpy(
                (static_cast<char*>(
                    const_cast<void*>(theData.GetPointer()))), // destination
                theData.GetSize(), // size of destination buffer.
                pChar,             // source
                nSize);            // length of source.
            // bool bZeroSource=false); // if true, sets the source buffer to
            // zero after copying is done.

            // This base64 encodes the private key data, which
            // is already encrypted to its passphase as well.
            //
            ascKey.SetData(theData);

            otLog5 << __FUNCTION__
                   << ": Success copying private key into memory.\n";
            bReturnVal = true;
        }
        else {
            otErr << __FUNCTION__
                  << ": Failed copying private key into memory.\n";
        }
    }

    return bReturnVal;
}

} // namespace opentxs
