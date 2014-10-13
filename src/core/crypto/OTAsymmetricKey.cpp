/************************************************************
 *
 *  OTAsymmetricKey.cpp
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

#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <opentxs/core/crypto/OTCaller.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/OTIdentifier.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/crypto/OTSignatureMetadata.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <cstring>

#if defined(OT_CRYPTO_USING_OPENSSL)
#include <opentxs/core/crypto/OTAsymmetricKeyOpenSSL.hpp>
#endif

namespace opentxs
{

// static
OTAsymmetricKey* OTAsymmetricKey::KeyFactory() // Caller IS responsible to
                                               // delete!
{
    OTAsymmetricKey* pKey = nullptr;
#if defined(OT_CRYPTO_USING_OPENSSL)
    pKey = new OTAsymmetricKey_OpenSSL;
#elif defined(OT_CRYPTO_USING_GPG)
    //  pKey = new OTAsymmetricKey_GPG;
    otErr << __FUNCTION__ << ": Open-Transactions doesn't support GPG (yet), "
                             "so it's impossible to instantiate a key.\n";
#else
    otErr << __FUNCTION__
          << ": Open-Transactions isn't built with any crypto engine, "
             "so it's impossible to instantiate a key.\n";
#endif
    return pKey;
}

// virtual
OTAsymmetricKey* OTAsymmetricKey::ClonePubKey() const // Caller IS responsible
                                                      // to delete!
{
    OTAsymmetricKey* pKey = OTAsymmetricKey::KeyFactory();
    OT_ASSERT(nullptr != pKey);
    OT_ASSERT(nullptr != m_pMetadata);
    OT_ASSERT(nullptr != pKey->m_pMetadata);

    OTASCIIArmor ascTransfer;
    GetPublicKey(ascTransfer); // Get this's public key in ASCII-armored format
    pKey->SetPublicKey(
        ascTransfer); // Decodes that public key from ASCII armor into pKey

    *(pKey->m_pMetadata) = *(m_pMetadata); // Metadata will be attached to
                                           // signatures, if set.

    return pKey;
}

// static
OT_OPENSSL_CALLBACK* OTAsymmetricKey::s_pwCallback = nullptr;

// static void SetPasswordCallback(p_OT_OPENSSL_CALLBACK pCallback);
// static p_OT_OPENSSL_CALLBACK GetPasswordCallback();
// static bool IsPasswordCallbackSet() { (nullptr == s_pwCallback) ? false :
// true;
// }

void OTAsymmetricKey::SetPasswordCallback(OT_OPENSSL_CALLBACK* pCallback)
{
    const char* szFunc = "OTAsymmetricKey::SetPasswordCallback";

    if (nullptr != s_pwCallback)
        otOut << szFunc << ": WARNING: re-setting the password callback (one "
                           "was already there)...\n";
    else
        otWarn << szFunc << ": FYI, setting the password callback to a "
                            "non-nullptr pointer (which is what we want.)\n";

    if (nullptr == pCallback)
        otErr << szFunc
              << ": WARNING, setting the password callback to nullptr! "
                 "(OpenSSL will thus "
                 "be forced to ask for the passphase on the console, "
                 "until this is called "
                 "again and set properly.)\n";

    s_pwCallback = pCallback; // no need to delete function pointer that came
                              // before this function pointer.
}

OT_OPENSSL_CALLBACK* OTAsymmetricKey::GetPasswordCallback()
{
    // cppcheck-suppress variableScope
    const char* szFunc = "OTAsymmetricKey::GetPasswordCallback";

#if defined(OT_TEST_PASSWORD)
    otInfo << szFunc << ": WARNING, OT_TEST_PASSWORD *is* defined. The "
                        "internal 'C'-based password callback was just "
                        "requested by OT (to pass to OpenSSL). So, returning "
                        "the default_pass_cb password callback, which will "
                        "automatically return "
                        "the 'test' password to OpenSSL, if/when it calls that "
                        "callback function.\n";
    return &default_pass_cb;
#else
    if (IsPasswordCallbackSet()) {
        otLog5 << szFunc
               << ": FYI, the internal 'C'-based password callback is now "
                  "being returning to OT, "
                  "which is passing it to OpenSSL "
                  "during a crypto operation. (If OpenSSL invokes it, then we "
                  "should see other logs after this from when it triggers "
                  "whatever password-collection dialog is provided at startup "
                  "by the (probably Java) OTAPI client.)\n";
        return s_pwCallback;
    }
    else {
        //        otInfo << "OTAsymmetricKey::GetPasswordCallback: FYI, the
        // internal 'C'-based password callback was requested by OT (to pass to
        // OpenSSL), "
        //                      "but the callback hasn't been set yet.
        // (Returning nullptr CALLBACK to OpenSSL!! Thus causing it to instead
        // ask
        // for the passphrase on the CONSOLE, "
        //                      "since no Java password dialog was apparently
        // available.)\n");

        //        return static_cast<OT_OPENSSL_CALLBACK *>(nullptr);

        // We have our own "console" password-gathering function, which allows
        // us to use our master key.
        // Since the souped-up cb uses it, I'm just returning that here as a
        // default, for now.
        OTAsymmetricKey::SetPasswordCallback(&souped_up_pass_cb);
        return s_pwCallback;
    }
#endif
}

// static
OTCaller* OTAsymmetricKey::s_pCaller = nullptr;

// Takes ownership. UPDATE: doesn't, since he assumes the Java side
// created it and will therefore delete it when the time comes.
// I keep a pointer, but I don't delete the thing. Let Java do it.
//
bool OTAsymmetricKey::SetPasswordCaller(OTCaller& theCaller)
{
    const char* szFunc = "OTAsymmetricKey::SetPasswordCaller";

    otLog3 << szFunc
           << ": Attempting to set the password caller... "
              "(the Java code has passed us its custom password dialog object "
              "for later use if/when the "
              "OT 'C'-based password callback is triggered by openssl.)\n";

    if (!theCaller.isCallbackSet()) {
        otErr << szFunc << ": ERROR: OTCaller::setCallback() "
                           "MUST be called first, with an OTCallback-extended "
                           "object passed to it,\n"
                           "BEFORE calling this function with that OTCaller. "
                           "(Returning false.)\n";
        return false;
    }

    if (nullptr != s_pCaller) {
        otErr << szFunc
              << ": WARNING: Setting the password caller again, even though "
                 "it was apparently ALREADY set... (Meaning Java has probably "
                 "erroneously called this twice, "
                 "possibly passing the same OTCaller both times.)\n";
        //        delete s_pCaller; // Let Java delete it.
    }

    s_pCaller = &theCaller;

    OTAsymmetricKey::SetPasswordCallback(&souped_up_pass_cb);

    otWarn << szFunc
           << ": FYI, Successfully set the password caller object from "
              "Java, and set the souped_up_pass_cb in C for OpenSSL (which "
              "triggers "
              "that Java object when the time is right.) Returning true.\n";

    return true;
}

OTCaller* OTAsymmetricKey::GetPasswordCaller()
{
    const char* szFunc = "OTAsymmetricKey::GetPasswordCaller";

    otLog4 << szFunc << ": FYI, this was just called by souped_up_pass_cb "
                        "(which must have just been called by OpenSSL.) "
                        "Returning s_pCaller == "
           << ((nullptr == s_pCaller) ? "nullptr" : "VALID POINTER")
           << " (Hopefully NOT nullptr, so the "
              "custom password dialog can be triggered.)\n";

    return s_pCaller;
}

bool OT_API_Set_PasswordCallback(OTCaller& theCaller) // Caller must have
                                                      // Callback attached
                                                      // already.
{
    if (!theCaller.isCallbackSet()) {
        otErr << __FUNCTION__ << ": ERROR:\nOTCaller::setCallback() "
                                 "MUST be called first, with an "
                                 "OTCallback-extended class passed to it,\n"
                                 "before then invoking this function (and "
                                 "passing that OTCaller as a parameter "
                                 "into this function.)\n";
        return false;
    }

    otWarn << __FUNCTION__
           << ": FYI, calling OTAsymmetricKey::SetPasswordCaller(theCaller) "
              "now... (which is where "
              "OT internally sets its pointer to the Java caller object, which "
              "must have been passed in as a "
              "parameter to this function. "
              "This is also where OT either sets its internal 'C'-based "
              "password callback to the souped_up "
              "version which uses that Java caller object, "
              "OR where OT sets its internal callback to nullptr--which causes "
              "OpenSSL to ask for the passphrase "
              "on the CONSOLE instead.)\n";

    const bool bSuccess = OTAsymmetricKey::SetPasswordCaller(theCaller);

    otWarn << __FUNCTION__
           << ": RESULT of call to OTAsymmetricKey::SetPasswordCaller: "
           << (bSuccess ? "SUCCESS" : "FAILURE") << "\n";

    return bSuccess;
}

/*
 extern "C"
 {
 typedef int32_t OT_OPENSSL_CALLBACK(char* buf, int32_t size, int32_t rwflag,
 void* u); // <== Callback type, used for declaring.
 }
*/

// If the password callback isn't set, then it uses the default ("test")
// password.
//
extern "C" int32_t default_pass_cb(char* buf, int32_t size, int32_t,
                                   void* userdata)
{
    int32_t len = 0;
    const uint32_t theSize = uint32_t(size);

    // We'd probably do something else if 'rwflag' is 1

    const OTPasswordData* pPWData = nullptr;
    std::string str_userdata;

    if (nullptr != userdata) {
        pPWData = static_cast<OTPasswordData*>(userdata);

        if (nullptr != pPWData) {
            str_userdata = pPWData->GetDisplayString();
        }
    }
    else
        str_userdata = "";

    //    otWarn << "OPENSSL_CALLBACK: (Password callback hasn't been set
    // yet...) Using 'test' pass phrase for \"%s\"\n", (char *)u);

    otWarn << __FUNCTION__ << ": Using DEFAULT TEST PASSWORD: "
                              "'test' (for \"" << str_userdata << "\")\n";

    // get pass phrase, length 'len' into 'tmp'
    //
    //    std::string str_Password;
    //    std::getline (std::cin, str_Password);

    //    const char *tmp_passwd = ""; // For now removing any possibility that
    // "test" password can slip through.
    const char* tmp_passwd = "test";
    //    const char *tmp_passwd = str_Password.c_str();

    len = static_cast<int32_t>(strlen(tmp_passwd));
    //    len = str_Password.size();

    if (len <= 0) {
        otOut << __FUNCTION__ << ": Problem? Returning 0...\n";
        return 0;
    }

    // if too int64_t, truncate
    if (len > size) len = size;

    const uint32_t theLength = static_cast<const uint32_t>(len);

    // void * pv =
    OTPassword::safe_memcpy(buf,        // destination
                            theSize,    // size of destination buffer.
                            tmp_passwd, // source
                            theLength); // length of source.
    // bool bZeroSource=false); // if true, sets the source buffer to zero after
    // copying is done.
    return len;
}

//#ifdef _WIN32
//    memcpy_s(buf,
//             theSize,
//             tmp_passwd, // source
//             theLength); // length of source.
//#else
//    memcpy(buf, tmp_passwd, len);
//#endif

#ifndef _PASSWORD_LEN
#define _PASSWORD_LEN 128
#endif

// This is the function that OpenSSL calls when it wants to ask the user for his
// password.
// If we return 0, that's bad, that means the password caller and callback
// failed somehow.
//
extern "C" int32_t souped_up_pass_cb(char* buf, int32_t size, int32_t rwflag,
                                     void* userdata)
{
    //  OT_ASSERT(nullptr != buf); // apparently it CAN be nullptr sometimes.
    OT_ASSERT(nullptr != userdata);
    const OTPasswordData* pPWData = static_cast<OTPasswordData*>(userdata);
    const std::string str_userdata = pPWData->GetDisplayString();

    OTPassword thePassword;
    bool bGotPassword = false;

    // Sometimes it's passed in, otherwise we use the global one.
    std::shared_ptr<OTCachedKey> pCachedKey(pPWData->GetCachedKey());

    if (!pCachedKey) {
        // Global one.
        pCachedKey =
            OTCachedKey::It(); // Used to only use this one (global one) but now
                               // I allow pPWData to contain a pointer to the
                               // exact instance. (To enable multiple
                               // instances...) If that's not found then here we
                               // set it to the global one.
    }
    if (!pCachedKey) OT_FAIL;

    const bool b1 = pPWData->isForNormalNym();
    const bool b3 = !(pCachedKey->isPaused());

    // For example, perhaps we need to collect a password for a symmetric key.
    // In that case, it has nothing to do with any master key, or any
    // public/private
    // keys. It ONLY wants to do a simple password collect.
    //
    const bool bOldSystem = pPWData->isUsingOldSystem();

    //    otLog5 <<
    // "--------------------------------------------------------------------------------\n"
    //                  "TOP OF SOUPED-UP PASS CB:\n pPWData->isForNormalNym():
    // %s \n "
    ////                "!pPWData->isUsingOldSystem(): %s \n "
    //                  "!(pCachedKey->isPaused()): %s \n",
    //                  b1 ? "NORMAL" : "NOT normal",
    ////                b2 ? "NOT using old system" : "USING old system",
    //                  b3 ? "NOT paused" : "PAUSED"
    //                  );

    //
    // It's for one of the normal Nyms.
    // (NOT the master key.)
    // If it was for the master key, we'd just pop up the dialog and get the
    // master passphrase.
    // But since it's for a NORMAL Nym, we have to call
    // OTCachedKey::GetMasterPassword. IT will pop
    // up the dialog if it needs to, by recursively calling this in master mode,
    // and then it'll use
    // the user passphrase from that dialog to derive a key, and use THAT key to
    // unlock the actual
    // "passphrase" (a random value) which is then passed back to OpenSSL to use
    // for the Nyms.
    //
    if (b1 && // Normal Nyms, unlike Master passwords, have to look up the
              // master password first.
        !bOldSystem && b3) // ...Unless they are still using the old system, in
                           // which case they do NOT look up the master
                           // password...
    {

        // Therefore we need to provide the password from an OTSymmetricKey
        // stored here.
        // (the "actual key" in the OTSymmetricKey IS the password that we are
        // passing back!)

        // So either the "actual key" is cached on a timer, from some previous
        // request like
        // this, OR we have to pop up the passphrase dialog, ask for the
        // passphrase for the
        // OTSymmetricKey, and then use it to GET the actual key from that
        // OTSymmetricKey.
        // The OTSymmetricKey should be stored in the OTWallet or OTServer,
        // which sets a pointer
        // to itself inside the OTPasswordData class statically, on
        // initialization.
        // That way, OTPasswordData can use that pointer to get a pointer to the
        // relevant
        // OTSymmetricKey being used as the MASTER key.
        //
        otLog3 << __FUNCTION__ << ": Using GetMasterPassword() call. \n";

        bGotPassword = pCachedKey->GetMasterPassword(
            pCachedKey, thePassword,
            str_userdata.c_str()); // bool bVerifyTwice=false

        // NOTE: shouldn't the above call to GetMasterPassword be passing the
        // rwflag as the final parameter?
        // Just as we see below with the call to GetPasswordFromConsole. Right?
        // Of course, it DOES generate internally,
        // if necessary, and thus it forces an "ask twice" in that situation
        // anyway. (It's that smart.)
        // Actually that's it. The master already asks twice when it's
        // generating.

        //      bool   GetMasterPassword(OTPassword& theOutput,
        //                               const char       * szDisplay=nullptr,
        //                                     bool         bVerifyTwice=false);

        //      otOut << "OPENSSL_CALLBACK (souped_up_pass_cb): Finished calling
        // GetMasterPassword(). Result: %s\n",
        //                     bGotPassword ? "SUCCESS" : "FAILURE");
    }
    else {
        otLog3 << __FUNCTION__ << ": Using OT Password Callback. \n";

        OTCaller* pCaller =
            OTAsymmetricKey::GetPasswordCaller(); // See if the developer
                                                  // registered one via the OT
                                                  // API.

        //      if (nullptr == pCaller)
        //      {
        //          otErr << "OPENSSL_CALLBACK (souped_up_pass_cb): OTCaller is
        // nullptr. Try calling OT_API_Set_PasswordCallback() first.\n";
        //          OT_ASSERT(0); // This will never actually happen, since
        // SetPasswordCaller() and souped_up_pass_cb are activated in same
        // place.
        //      }

        if (nullptr == pCaller) // We'll just grab it from the console then.
        {
            otOut << "Passphrase request for: \"" << str_userdata << "\"\n";

            bGotPassword = OTCrypto::It()->GetPasswordFromConsole(
                thePassword, (1 == rwflag) ? true : false);
        }
        else // Okay, we have a callback, so let's pop up the dialog!
        {

            // The dialog should display this string (so the user knows what he
            // is authorizing.)
            //
            pCaller->SetDisplay(str_userdata.c_str(),
                                static_cast<int32_t>(str_userdata.size()));

            if (1 == rwflag) {
                otLog4 << __FUNCTION__
                       << ": Using OT Password Callback (asks twice) for \""
                       << str_userdata << "\"...\n";
                pCaller->callTwo(); // This is where Java pops up a modal dialog
                                    // and asks for password twice...
            }
            else {
                otLog4 << __FUNCTION__
                       << ": Using OT Password Callback (asks once) for \""
                       << str_userdata << "\"...\n";
                pCaller->callOne(); // This is where Java pops up a modal dialog
                                    // and asks for password once...
            }

            /*
             NOTICE: (For security...)

             We are using an OTPassword object to collect the password from the
             caller.
             (We're not passing strings back and forth.) The OTPassword object
             is where we
             can centralize our efforts to scrub the memory clean as soon as
             we're done with
             the password. It's also designed to be light (no baggage) and to be
             passed around
             easily, with a set-size array for the data.

             Notice I am copying the password directly from the OTPassword
             object into the buffer
             provided to me by OpenSSL. When the OTPassword object goes out of
             scope, then it cleans
             up automatically.
             */

            bGotPassword = pCaller->GetPassword(thePassword);
        }
    }

    if (!bGotPassword) {
        otOut << __FUNCTION__
              << ": Failure: (false == bGotPassword.) (Returning 0.)\n";
        return 0;
    }
    // --------------------------------------
    otInfo << __FUNCTION__ << ": Success!\n";
    /*
     http://openssl.org/docs/crypto/pem.html#
     "The callback must return the number of characters in the passphrase or 0
     if an error occurred."
     */
    int32_t len = thePassword.isPassword() ? thePassword.getPasswordSize()
                                           : thePassword.getMemorySize();

    if (len < 0) {
        otOut << __FUNCTION__ << ": <0 length password was "
                                 "returned from the API password callback. "
                                 "Returning 0.\n";
        return 0;
    }
    // --------------------------------------
    else if (len == 0) {
        const char* szDefault = "test";

        otOut << __FUNCTION__
              << ": 0 length password was "
                 "returned from the API password callback. "
                 "Substituting default password 'test'.\n"; // todo: security:
                                                            // is this safe?
                                                            // Here's what's
                                                            // driving this: We
                                                            // can't return 0
                                                            // length string,
                                                            // but users wanted
                                                            // to be able to
                                                            // "just hit enter"
                                                            // and use an empty
                                                            // passphrase. So
                                                            // for cases where
                                                            // the user has
                                                            // explicitly "hit
                                                            // enter" we will
                                                            // substitute "test"
                                                            // as their
                                                            // passphrase
                                                            // instead. They
                                                            // still have to do
                                                            // this
                                                            // explicitly--it
                                                            // only happens when
                                                            // they use an empty
                                                            // one.

        if (thePassword.isPassword())
            thePassword.setPassword(szDefault,
                                    static_cast<int32_t>(OTString::safe_strlen(
                                        szDefault, _PASSWORD_LEN)));
        else
            thePassword.setMemory(static_cast<const void*>(szDefault),
                                  static_cast<uint32_t>(OTString::safe_strlen(
                                      szDefault, _PASSWORD_LEN)) +
                                      1); // setMemory doesn't assume the null
                                          // terminator like setPassword does.

        len = thePassword.isPassword() ? thePassword.getPasswordSize()
                                       : thePassword.getMemorySize();
    }

    OTPassword* pMasterPW = pPWData->GetMasterPW();

    if (pPWData->isForCachedKey() && (nullptr != pMasterPW)) {
        *pMasterPW = thePassword;
    }
    // --------------------------------------
    else if (nullptr != buf) {
        // if too int64_t, truncate
        if (len > size) len = size;

        const uint32_t theSize = static_cast<uint32_t>(size);
        const uint32_t theLength = static_cast<uint32_t>(len);

        if (thePassword.isPassword()) {
            //          otErr << "%s: BEFORE TEXT PASSWORD: %s  LENGTH: %d\n",
            // __FUNCTION__, thePassword.getPassword(), theLength);

            OTPassword::safe_memcpy(buf,     // destination
                                    theSize, // size of destination buffer.
                                    thePassword.getPassword_uint8(), // source
                                    theLength); // length of source.
            // bool bZeroSource=false); // No need to set this true, since
            // OTPassword (source) already zeros its memory automatically.
            buf[theLength] = '\0'; // null terminator.

            //          int32_t nSize =
            // static_cast<int32_t>(thePassword.getPasswordSize());
            //          otErr << "%s: AFTER TEXT PASSWORD: %s  LENGTH: %d\n",
            // __FUNCTION__, buf, nSize);
        }
        else {
            OTPassword::safe_memcpy(buf,     // destination
                                    theSize, // size of destination buffer.
                                    thePassword.getMemory_uint8(), // source
                                    theLength); // length of source.
            // bool bZeroSource=false); // No need to set this true, since
            // OTPassword (source) already zeros its memory automatically.

            //          int32_t nSize =
            // static_cast<int32_t>(thePassword.getMemorySize());
            //          otErr << "%s: (BINARY PASSWORD)  LENGTH: %d\n",
            // __FUNCTION__, nSize);
        }

    }
    // --------------------------------------
    else // should never happen
    {
        //      OT_FAIL_MSG("This should never happen. (souped_up_pass_cb");
    }
    return len;
}

bool OTAsymmetricKey::CalculateID(OTIdentifier& theOutput) const // Only works
                                                                 // for public
                                                                 // keys.
{
    const char* szFunc = "OTAsymmetricKey::CalculateID";

    theOutput.clear();

    if (!IsPublic()) {
        otErr << szFunc << ": Error: !IsPublic() (This function should only be "
                           "called on a public key.)\n";
        return false;
    }

    OTString strPublicKey;
    bool bGotPublicKey = GetPublicKey(strPublicKey);

    if (!bGotPublicKey) {
        otErr << szFunc << ": Error getting public key.\n";
        return false;
    }

    bool bSuccessCalculateDigest = theOutput.CalculateDigest(strPublicKey);

    if (!bSuccessCalculateDigest) {
        theOutput.clear();
        otErr << szFunc << ": Error calculating digest of public key.\n";
        return false;
    }

    return true;
}

// Get the public key in ASCII-armored format with bookends  - ------- BEGIN
// PUBLIC KEY --------
// This version, so far, is escaped-only. Notice the "- " before the rest of the
// bookend starts.
bool OTAsymmetricKey::GetPublicKey(OTString& strKey, bool bEscaped) const
{
    OTASCIIArmor theArmor;

    // TODO: optimization: When optimizing for CPU cycles, and willing to
    // sacrifice a little RAM, we
    // can save this value the first time it's computed, and then as long as the
    // armored version (without
    // bookends) doesn't change, we can save the computed version and pass it
    // back here, instead of re-generating
    // it here each time this is called. This implies a need for the armored
    // version to be able to be flagged
    // as "dirty" when it is changed.

    if (GetPublicKey(theArmor)) {
        if (bEscaped) {
            strKey.Concatenate(
                "- -----BEGIN PUBLIC KEY-----\n" // ESCAPED VERSION
                "%s"
                "- -----END PUBLIC KEY-----\n",
                theArmor.Get());
        }
        else {
            strKey.Concatenate(
                "-----BEGIN PUBLIC KEY-----\n" // UN-ESCAPED VERSION
                "%s"
                "-----END PUBLIC KEY-----\n",
                theArmor.Get());
        }
        return true;
    }
    else
        otErr << "OTAsymmetricKey::GetPublicKey: Error: GetPublicKey(armored) "
                 "returned false. (Returning false.)\n";

    return false;
}

// Get the public key in ASCII-armored format.
//
bool OTAsymmetricKey::GetPublicKey(OTASCIIArmor& ascKey) const
{
    OT_ASSERT_MSG(IsPublic(),
                  "OTAsymmetricKey::GetPublicKey: ASSERT: IsPublic()\n");

    ascKey.Release();

    if (nullptr == m_p_ascKey) return false;

    ascKey.Set(*m_p_ascKey);

    return true;
}

// High-level.
// This is the version that will handle the bookends ( --------- BEGIN PUBLIC
// KEY -------)
// You can pass it an OTString, and it will then call the lower-level version of
// SetPublicKey
// (the one that takes an OTASCIIArmor object.)
//
bool OTAsymmetricKey::SetPublicKey(const OTString& strKey, bool bEscaped)
{
    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;

    // This reads the string into the Armor and removes the bookends. (-----
    // BEGIN ...)
    OTASCIIArmor theArmor;

    if (theArmor.LoadFromString(const_cast<OTString&>(strKey), bEscaped)) {
        return SetPublicKey(theArmor);
    }
    else
        otErr << "OTAsymmetricKey::SetPublicKey: Error: failed loading "
                 "ascii-armored contents from bookended string:\n\n" << strKey
              << "\n\n";

    return false;
}

// Copies to internal ascii-armored string, and wipes any key if
// one is already loaded.
//
bool OTAsymmetricKey::SetPublicKey(const OTASCIIArmor& ascKey)
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

    m_p_ascKey->Set(ascKey);

    return true;
}

// Get the private key in ASCII-armored format with bookends  - ------- BEGIN
// ENCRYPTED PRIVATE KEY --------
// This version, so far, is escaped-only. Notice the "- " before the rest of the
// bookend starts.
//
bool OTAsymmetricKey::GetPrivateKey(OTString& strKey, bool bEscaped) const
{
    OTASCIIArmor theArmor;

    if (GetPrivateKey(theArmor)) {
        if (bEscaped) {
            strKey.Concatenate("- -----BEGIN ENCRYPTED PRIVATE KEY-----\n"
                               "%s"
                               "- -----END ENCRYPTED PRIVATE KEY-----\n",
                               theArmor.Get());
        }
        else {
            strKey.Concatenate("-----BEGIN ENCRYPTED PRIVATE KEY-----\n"
                               "%s"
                               "-----END ENCRYPTED PRIVATE KEY-----\n",
                               theArmor.Get());
        }
        return true;
    }
    else
        otErr << "OTAsymmetricKey::GetPrivateKey: Error: "
                 "GetPrivateKey(armored) returned false. (Returning false.)\n";

    return false;
}

// Get the private key in ASCII-armored format
bool OTAsymmetricKey::GetPrivateKey(OTASCIIArmor& ascKey) const // (ascKey is
                                                                // the output.)
{
    OT_ASSERT(IsPrivate());

    ascKey.Release();

    if (nullptr == m_p_ascKey) return false;

    ascKey.Set(*m_p_ascKey);

    return true;
}

// Decodes a private key from ASCII armor into an actual key pointer
// and sets that as the keypointer on this object.
// This is the version that will handle the bookends ( --------- BEGIN ENCRYPTED
// PRIVATE KEY -------)
bool OTAsymmetricKey::SetPrivateKey(const OTString& strKey, bool bEscaped)
{
    m_bIsPublicKey = false;
    m_bIsPrivateKey = true;

    // This reads the string into the Armor and removes the bookends. (-----
    // BEGIN ...)
    //
    OTASCIIArmor theArmor;
    const char* szPrivateKeyStarts = "-----BEGIN ENCRYPTED PRIVATE KEY-----";
    if (theArmor.LoadFromString(const_cast<OTString&>(strKey), bEscaped,
                                szPrivateKeyStarts)) // This last param causes
                                                     // OTASCIIArmor to only
                                                     // "start loading" when it
                                                     // reaches the private key.
    {
        return SetPrivateKey(theArmor);
    }

    return false;
}

// Copies to the internal ascii-armored storage. Wipes the internal
// private key, if one is loaded.
bool OTAsymmetricKey::SetPrivateKey(const OTASCIIArmor& ascKey)
{
    ReleaseKeyLowLevel();

    m_bIsPublicKey = false;
    m_bIsPrivateKey = true; // PRIVATE KEY

    if (nullptr == m_p_ascKey) {
        m_p_ascKey = new OTASCIIArmor;
        OT_ASSERT(nullptr != m_p_ascKey);
    }

    m_p_ascKey->Set(ascKey);

    return true;
}

OTAsymmetricKey::OTAsymmetricKey()
    : m_p_ascKey(nullptr)
    , m_bIsPublicKey(false)
    , m_bIsPrivateKey(false)
    , m_pMetadata(new OTSignatureMetadata)
{
    //    if (nullptr == m_p_ascKey)
    //    {
    //        m_p_ascKey = new OTASCIIArmor;
    //        OT_ASSERT(nullptr != m_p_ascKey);
    //    }
}

OTAsymmetricKey::~OTAsymmetricKey()
{
    //    Release_AsymmetricKey(); // ******

    m_timer.clear(); // Since ReleaseKeyLowLevel is no longer called here (via
                     // Release_AsymmetricKey) then
    // m_timer.clear() was no longer getting called, so I added it here to
    // rectify that. See below NOTE for
    // more details.
    //
    // NOTE: It's very unusual that the above is commented out, since my normal
    // convention is to
    // call this function both in the destructor, as well as in the Release()
    // method for any given
    // class.
    // Here's why it's commented out, in this case. Because all it does is call
    // ReleaseKeyLowLevel,
    // which calls ReleaseKeyLowLevel_Hook, which is pure virtual and is what
    // allows the
    // OTAsymmetricKey_OpenSSL class to clean up its OpenSSL-specific private
    // members.
    // We CANNOT call a pure virtual method from a destructor (which is where we
    // currently are)
    // and so we cannot call Release_AsymmetricKey without removing that pure
    // virtual call. The
    // problem is, ReleaseKeyLowLevel USES that pure virtual call all over this
    // file (i.e. in many
    // other places BESIDES the destructor) and so we cannot just remove the
    // pure virtual call it
    // uses which is, in fact, the entire purpose it's being called in the first
    // place. So what I
    // did was, I changed OTAsymmetricKey_OpenSSL to directly clean up its
    // OpenSSL-specific key data,
    // and it just ALSO has the hook override doing the same thing. This way
    // Release_AsymmetricKey
    // can continue to be used by ReleaseKeyLowLevel all over this file, as it
    // was designed, using
    // the hook and the override, yet also this destructor will continue to work
    // perfectly... because
    // Release_AsymmetricKey() is commented out above, we won't have any runtime
    // errors from trying to
    // run a pure virtual method from a destructor. And because
    // OTAsymmetricKey_OpenSSL now cleans itself
    // up in its own destructor automatically, we have no need whatsoever to
    // call a virtual function here
    // to clean it up. So it's commented out.
    // Makes sense? Of course we didn't have any virtuality before
    // OTAsymmetricKey_OpenSSL was added,
    // since OTAsymmetricKey previously had no subclasses at all. But that has
    // changed recently, so that
    // it is now an abstract interface, so that someday a GPG implementation, or
    // NaCl implementation can
    // someday be added.

    // Release the ascii-armored version of the key (safe to store in this
    // form.)
    //
    if (nullptr != m_p_ascKey) delete m_p_ascKey;
    m_p_ascKey = nullptr;

    if (nullptr != m_pMetadata) delete m_pMetadata;
    m_pMetadata = nullptr;
}

void OTAsymmetricKey::Release_AsymmetricKey()
{

    // Release the ascii-armored version of the key (safe to store in this
    // form.)
    //
    // Moving this to the destructor. Shouldn't be going nullptr here IMO.
    //    if (nullptr != m_p_ascKey)
    //        delete m_p_ascKey;
    //    m_p_ascKey = nullptr;

    // Release the instantiated OpenSSL key (unsafe to store in this form.)
    //
    ReleaseKeyLowLevel();

    //    m_bIsPrivateKey = false;  // Every time this Releases, I don't want to
    // lose what kind of key it was. (Once we know, we know.)
}

void OTAsymmetricKey::ReleaseKeyLowLevel()
{
    ReleaseKeyLowLevel_Hook();

    m_timer.clear();

    //    m_bIsPrivateKey = false;  // Every time this Releases, I don't want to
    // lose what kind of key it was. (Once we know, we know.)
}

// High-level, used only by programmatic user, not by this class internally.
void OTAsymmetricKey::ReleaseKey()
{
    // Todo: someday put a timer inside this class, so it doesn't REALLY release
    // the
    // key until the Timer has expired.
    // NOTE: marking this as "todo" because I won't necessarily want it done
    // this way. This
    // solution would keep the actual private key loaded in OpenSSL, meaning I'd
    // have to take
    // over the OpenSSL memory management to make it into a safe solution
    // (though I probably
    // have to do that anyway.)
    // A better solution might be to have a random session key used for
    // protecting a hashed
    // version of the passphrase, and then ALWAYS destroying the key as fast as
    // possible,
    // instead using the hashed passphrase within the bounds of a timer, with
    // the hashed passphrase
    // being decrypted JUST before use and then destroyed, just as the private
    // key is destroyed.
    // I might also wish such a thing was stored not just in OT protected
    // memory, but in a standard
    // *API* for protected memory, such as ssh-agent or the Mac Keychain.
    // However, that can easily
    // be just an option to be added later, meaning I can go ahead and code my
    // hashed password solution
    // in the meantime. But will that be coded here at the OTAsymmetricKey
    // level? Or at the Nym level,
    // or at the static Nym level, or the app level? Hmm...
    //
    // Security:
    // UPDATE: Since the above solution is coming at some point anyway, I'm
    // going ahead and adding a
    // Timer version that works at this level (OTAsymmetricKey.)  The reason is
    // because it will be quick
    // and easy, and will give me the functionality I need for now, until I code
    // all the stuff above.
    // Just keep in mind: This is good enough for now, but it WILL result in the
    // private key staying
    // loaded in memory until the timer runs out, meaning if an attacker has
    // access to the RAM on the
    // local machine, if I haven't replaced the OpenSSL memory management, then
    // that is a security issue.
    //
    // TODO (remove theTimer entirely. OTCachedKey replaces already.)
    // I set this timer because the above required a password. But now that
    // master key is working,
    // the above would flow through even WITHOUT the user typing his passphrase
    // (since master key still
    // not timed out.) Resulting in THIS timer being reset!  Todo: I already
    // shortened this timer to 30
    // seconds, but need to phase it down to 0 and then remove it entirely!
    // Master key takes over now!
    //

    //  if (m_timer.getElapsedTimeInSec() > OT_KEY_TIMER)
    ReleaseKeyLowLevel();

    // Programmatic user (developer) may call ReleaseKey, but then we don't
    // actually release it until it's
    // been at least X minutes.
}

void OTAsymmetricKey::Release()
{
    Release_AsymmetricKey(); // My own cleanup is done here.

    // Next give the base class a chance to do the same...
    //    ot_super::Release(); // THERE IS NO base class in this case. But
    // normally this would be here for most classes.
}

// Load the private key from a .pem file
bool OTAsymmetricKey::LoadPrivateKey(
    const OTString& strFoldername, const OTString& strFilename,
    const OTString* pstrReason,
    const OTPassword* pImportPassword) // This reason
                                       // is what
                                       // displays on
// the passphrase dialog.
{
    Release();

    m_bIsPublicKey = false;
    m_bIsPrivateKey = true;

    const char* szFoldername = strFoldername.Get();
    const char* szFilename = strFilename.Get();

    OT_ASSERT(strFoldername.Exists());
    OT_ASSERT(strFilename.Exists());

    if (!OTDB::Exists(szFoldername, szFilename)) {
        otOut << "OTAsymmetricKey::LoadPrivateKey: Unable to find private key "
                 "file: " << szFoldername << OTLog::PathSeparator()
              << szFilename << "\n";
        return false;
    }

    // The private key is stored in an encrypted form, which is how it stays
    // until the
    // password callback is passed into some OpenSSL call that attempts to use
    // it.
    //
    std::string strFileContents(OTDB::QueryPlainString(
        szFoldername, szFilename)); // <=== LOADING "AS-IS" FROM DATA STORE.

    if (strFileContents.length() < 2) {
        otErr << "OTAsymmetricKey::LoadPrivateKey: Error reading file: "
              << szFoldername << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    const OTString strCert(strFileContents),
        strReason(nullptr == pstrReason ? "OTAsymmetricKey::LoadPrivateKey"
                                        : *pstrReason);

    return LoadPrivateKeyFromCertString(strCert, false, &strReason,
                                        pImportPassword); // bEscaped=false;
    // "escaped" means pre-pended with "- " as in:   - -----BEGIN CER....
}

// Load the public key from a .pem file
bool OTAsymmetricKey::LoadPublicKey(const OTString& strFoldername,
                                    const OTString& strFilename)
{
    // Already done in SetPublicKey()
    //    m_bIsPublicKey    = true;
    //    m_bIsPrivateKey    = false;

    Release();

    // This doesn't use assert on the arguments, but theArmor.LoadFromFile DOES.

    OTASCIIArmor theArmor;

    if (theArmor.LoadFromFile(strFoldername, strFilename)) {
        if (SetPublicKey(theArmor)) {
            otLog4 << "Success setting public key from OTASCIIArmor in "
                      "OTAsymmetricKey::LoadPublicKey.\n";
            return true;
        }
        else {
            otErr << "Unable to convert from OTASCIIArmor to public key in "
                     "OTAsymmetricKey::LoadPublicKey: " << strFilename << "\n";
            return false;
        }
    }
    else {
        otErr
            << "Unable to read pubkey file in OTAsymmetricKey::LoadPublicKey: "
            << strFilename << "\n";
        return false;
    }
}

// Load the public key from a x509 stored in a .pem file
bool OTAsymmetricKey::LoadPublicKeyFromCertFile(
    const OTString& strFoldername, const OTString& strFilename,
    const OTString* pstrReason, const OTPassword* pImportPassword)
{
    Release();

    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;

    const char* szFoldername = strFoldername.Get();
    const char* szFilename = strFilename.Get();

    OT_ASSERT(strFoldername.Exists());
    OT_ASSERT(strFilename.Exists());

    if (!OTDB::Exists(szFoldername, szFilename)) {
        otErr << __FUNCTION__ << ": File does not exist: " << szFoldername
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    //
    std::string strFileContents(OTDB::QueryPlainString(
        szFoldername, szFilename)); // <=== LOADING FROM DATA STORE.

    if (strFileContents.length() < 2) {
        otErr << __FUNCTION__ << ": Error reading file: " << szFoldername
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    const OTString strCert(strFileContents);

    return LoadPublicKeyFromCertString(
        strCert, false, pstrReason,
        pImportPassword); // bEscaped=false; "escaped" means pre-pended with "-
                          // " as in:   - -----BEGIN CER....
}

} // namespace opentxs
