/************************************************************
 *
 *  OTLowLevelKeyData.cpp
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
#include <opentxs/core/crypto/OTLowLevelKeyData.hpp>
#include <opentxs/core/crypto/OTKeypair.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey_OpenSSLPrivdp.hpp>

namespace opentxs
{

class OTLowLevelKeyData::OTLowLevelKeyDataOpenSSLdp
{
public:
    X509* m_pX509;
    EVP_PKEY* m_pKey; // Instantiated form of key. (For private keys especially,
                      // we don't want it instantiated for any longer than
                      // absolutely necessary.)
};

OTLowLevelKeyData::~OTLowLevelKeyData()
{
    if (m_bCleanup) Cleanup();
    if (nullptr != dp) delete (dp);
}

OTLowLevelKeyData::OTLowLevelKeyData()
    : m_bCleanup(true)
{
    dp = new OTLowLevelKeyDataOpenSSLdp();
    dp->m_pX509 = nullptr;
    dp->m_pKey = nullptr;
}

// Don't force things by explicitly calling this function, unless you are SURE
// there's no one else cleaning up the same objects. Notice the if (m_bCleanup)
// just above in the destructor, for that very reason.
//
void OTLowLevelKeyData::Cleanup()
{
    if (nullptr != dp->m_pKey) EVP_PKEY_free(dp->m_pKey);
    dp->m_pKey = nullptr;
    if (nullptr != dp->m_pX509) X509_free(dp->m_pX509);
    dp->m_pX509 = nullptr;
}

bool OTLowLevelKeyData::MakeNewKeypair(int32_t nBits)
{

    //    OpenSSL_BIO        bio_err    =    nullptr;
    X509* x509 = nullptr;
    EVP_PKEY* pNewKey = nullptr;

    //    CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON); // memory leak detection.
    // Leaving this for now.
    //    bio_err    =    BIO_new_fp(stderr, BIO_NOCLOSE);

    // actually generate the things. // TODO THESE PARAMETERS...(mkcert)
    mkcert(&x509, &pNewKey, nBits, 0, 3650); // 3650=10 years. Todo hardcoded.
    // Note: 512 bit key CRASHES
    // 1024 is apparently a minimum requirement, if not an only requirement.
    // Will need to go over just what sorts of keys are involved here... todo.

    if (nullptr == x509) {
        otErr << __FUNCTION__
              << ": Failed attempting to generate new x509 cert.\n";

        if (nullptr != pNewKey) EVP_PKEY_free(pNewKey);
        pNewKey = nullptr;

        return false;
    }

    if (nullptr == pNewKey) {
        otErr << __FUNCTION__
              << ": Failed attempting to generate new private key.\n";

        if (nullptr != x509) X509_free(x509);
        x509 = nullptr;

        return false;
    }

    // Below this point, x509 and pNewKey will need to be cleaned up properly.

    if (m_bCleanup) Cleanup();

    m_bCleanup = true;
    dp->m_pKey = pNewKey;
    dp->m_pX509 = x509;

    // --------COMMENT THIS OUT FOR PRODUCTION --------  TODO security
    //                  (Debug only.)
    //    RSA_print_fp(stdout, pNewKey->pkey.rsa, 0); // human readable
    //    X509_print_fp(stdout, x509); // human readable

    // --------COMMENT THIS OUT FOR PRODUCTION --------  TODO security
    //                  (Debug only.)
    // write the private key, then the x509, to stdout.

    //    OTPasswordData thePWData2("OTPseudonym::GenerateNym is calling
    // PEM_write_PrivateKey...");
    //
    //    PEM_write_PrivateKey(stdout, pNewKey, EVP_des_ede3_cbc(), nullptr, 0,
    // OTAsymmetricKey::GetPasswordCallback(), &thePWData2);
    //    PEM_write_X509(stdout, x509);

    return true;
}

bool OTLowLevelKeyData::SetOntoKeypair(OTKeypair& theKeypair)
{
    OT_ASSERT(nullptr != dp->m_pKey);
    OT_ASSERT(nullptr != dp->m_pX509);

    OT_ASSERT(nullptr != theKeypair.m_pkeyPublic);
    OT_ASSERT(nullptr != theKeypair.m_pkeyPrivate);

    // Since we are in OpenSSL-specific code, we have to make sure these are
    // OpenSSL-specific keys.
    //
    OTAsymmetricKey_OpenSSL* pPublicKey =
        dynamic_cast<OTAsymmetricKey_OpenSSL*>(theKeypair.m_pkeyPublic);
    OTAsymmetricKey_OpenSSL* pPrivateKey =
        dynamic_cast<OTAsymmetricKey_OpenSSL*>(theKeypair.m_pkeyPrivate);

    if (nullptr == pPublicKey) {
        otErr << __FUNCTION__ << ": dynamic_cast to OTAsymmetricKey_OpenSSL "
                                 "failed. (theKeypair.m_pkeyPublic)\n";
        return false;
    }
    if (nullptr == pPrivateKey) {
        otErr << __FUNCTION__ << ": dynamic_cast to OTAsymmetricKey_OpenSSL "
                                 "failed. (theKeypair.m_pkeyPrivate)\n";
        return false;
    }

    // Now we can call OpenSSL-specific methods on these keys...
    //
    pPublicKey->SetAsPublic();
    //  EVP_PKEY * pEVP_PubKey = X509_get_pubkey(m_pX509);
    //  OT_ASSERT(nullptr != pEVP_PubKey);
    //  pPublicKey-> SetKeyAsCopyOf(*pEVP_PubKey); // bool bIsPrivateKey=false
    // by default.
    pPublicKey->dp->SetKeyAsCopyOf(
        *dp->m_pKey); // bool bIsPrivateKey=false by default.
                      //  EVP_PKEY_free(pEVP_PubKey);
                      //  pEVP_PubKey = nullptr;

    pPublicKey->dp->SetX509(dp->m_pX509); // m_pX509 is now owned by pPublicKey.
                                          // (No need to free it in our own
                                          // destructor anymore.)
    dp->m_pX509 =
        nullptr; // pPublicKey took ownership, so we don't want to ALSO
                 // clean it up, since pPublicKey already will do so.

    pPrivateKey->SetAsPrivate();
    pPrivateKey->dp->SetKeyAsCopyOf(
        *dp->m_pKey, true); // bool bIsPrivateKey=true; (Default is false)
    // Since pPrivateKey only takes a COPY of m_pKey, we are still responsible
    // to clean up m_pKey in our own destructor.
    // (Assuming m_bCleanup is set to true, which is the default.) That's why
    // I'm NOT setting it to nullptr, as I did above
    // with m_pX509.

    EVP_PKEY_free(dp->m_pKey);
    dp->m_pKey = nullptr;

    // Success! At this point, theKeypair's public and private keys have been
    // set.
    // Keep in mind though, they still won't be "quite right" until saved and
    // loaded
    // again, at least according to existing logic. That saving/reloading is
    // currently
    // performed in OTPseudonym::GenerateNym().
    //
    return true;
}

} // namespace opentxs
