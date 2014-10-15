/************************************************************
 *
 *  OTCachedKey.cpp
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

#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/OTIdentifier.hpp>
#include <opentxs/core/crypto/OTKeyring.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/crypto/OTSymmetricKey.hpp>

#include <memory>

extern "C" {
#include <openssl/opensslconf.h>
}

#define OT_DEFAULT_PASSWORD "test"

namespace opentxs
{

std::mutex OTCachedKey::s_mutexThreadTimeout;
std::mutex OTCachedKey::s_mutexCachedKeys;
mapOfCachedKeys OTCachedKey::s_mapCachedKeys;

bool OTCachedKey::IsGenerated()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    bool bReturnVal = false;

    if (nullptr != m_pSymmetricKey) {
        bReturnVal = m_pSymmetricKey->IsGenerated();
    }

    return bReturnVal;
}

bool OTCachedKey::HasHashCheck()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    bool bReturnVal = false;

    if (nullptr != m_pSymmetricKey) {
        bReturnVal = m_pSymmetricKey->HasHashCheck();
    }

    return bReturnVal;
}

// if you pass in a master key ID, it will look it up on an existing cached map
// of master keys.
// Otherwise it will use "the" global Master Key (the one used for the Nyms.)
//
// static
std::shared_ptr<OTCachedKey> OTCachedKey::It(OTIdentifier* pIdentifier)
{
    // For now we're only allowing a single global instance, unless you pass in
    // an ID, in which case we keep a map.

    // Default is 0 ("you have to type your PW a million times"), but it's
    // overridden in config file.
    static std::shared_ptr<OTCachedKey> s_theSingleton(new OTCachedKey);

    if (nullptr == pIdentifier)
        return s_theSingleton; // Notice if you pass nullptr (no args) then it
                               // ALWAYS returns a good pointer here.

    // There is a chance of failure if you pass an ID, since maybe it's not
    // already on the map.
    // But at least by this point we know FOR SURE that pIdentifier is NOT
    // nullptr.
    //
    std::lock_guard<std::mutex> lock(OTCachedKey::s_mutexCachedKeys);

    const OTString strIdentifier(*pIdentifier);
    const std::string str_identifier(strIdentifier.Get());

    auto it_keys = s_mapCachedKeys.find(str_identifier);

    if (s_mapCachedKeys.end() != it_keys) // found it!
    {
        std::shared_ptr<OTCachedKey> pShared(it_keys->second);

        if (pShared) {
            return pShared;
        }
        else
            s_mapCachedKeys.erase(it_keys);
    }

    // else: We can't instantiate it, since we don't have the corresponding
    // CachedKey, just its
    // Identifier. We're forced simply to return nullptr in this case.
    //
    // Therefore you should normally pass in the master key (the same one that
    // you want to cache a copy
    // of) using the below version of It(). That version creates the copy, if
    // it's not already there.
    //
    return std::shared_ptr<OTCachedKey>();
}

// if you pass in a master key, it will look it up on an existing cached map of
// master keys,
// based on the ID of the master key passed in. (Where it stores its own cached
// copy of the same
// master key.)
// NOTE: If you use it this way, then you must NEVER use the actual master key
// being cached (such as
// the one stored in a password-protected purse.) Instead, you must always look
// up the cached version,
// and use THAT master key, instead of the actual one in your  OTPurse. The only
// time
// you can use your master key itself is when loading it (such as when OTPurse
// loads
// its internal Master Key.) But thereafter, use the cached version of it for
// all operations
// and for saving.

// Note: parameter no longer const due to need to lock its mutex.
// static
std::shared_ptr<OTCachedKey> OTCachedKey::It(OTCachedKey& theSourceKey)
{
    //    std::lock_guard<std::mutex> lock(*(theSourceKey.GetMutex()));

    // There is no chance of failure since he passed the master key itself,
    // since even if it's not already on the map, we'll just create a copy and
    // put
    // it there ourselves, returning a pointer either way.
    //
    // Except... if theSourceKey isn't generated...
    //
    if (!(const_cast<OTCachedKey&>(theSourceKey))
             .IsGenerated()) // it's only not const due to the mutex inside
    {
        otErr << "OTCachedKey::" << __FUNCTION__
              << ": theSourceKey.IsGenerated() returned false. "
                 "(Returning nullptr.)\n";
        return std::shared_ptr<OTCachedKey>();
    }

    std::lock_guard<std::mutex> lock_keys(OTCachedKey::s_mutexCachedKeys);

    const OTIdentifier theSourceID(theSourceKey);

    const OTString strIdentifier(theSourceID);
    const std::string str_identifier(strIdentifier.Get());

    // Let's see if it's already there on the map...
    //
    auto it_keys = s_mapCachedKeys.find(str_identifier);

    if (s_mapCachedKeys.end() != it_keys) // found it!
    {
        std::shared_ptr<OTCachedKey> pMaster(it_keys->second);

        if (pMaster)
            return pMaster;
        else
            s_mapCachedKeys.erase(it_keys);
    }

    // By this point, pMaster is definitely nullptr. (Not found on the map,
    // needs
    // to be added.)
    //

    // Here we make a copy of the master key and insert it into the map.
    // Then we return a pointer to it.
    //
    OTASCIIArmor ascCachedKey;
    if ((const_cast<OTCachedKey&>(theSourceKey)).SerializeTo(
            ascCachedKey)) // it's only not const due to the mutex inside
    {
        std::shared_ptr<OTCachedKey> pMaster(
            new OTCachedKey); // int32_t nTimeoutSeconds=OT_MASTER_KEY_TIMEOUT;

        pMaster->SetCachedKey(ascCachedKey);

        s_mapCachedKeys.insert(
            std::pair<std::string, std::shared_ptr<OTCachedKey>>(
                str_identifier, pMaster)); // takes ownership here.
        return pMaster;
    }
    // theSourceKey WAS generated, but SerializeTo FAILED? Very strange...
    else
        otErr << __FUNCTION__
              << ": theSourceKey.SerializeTo(ascCachedKey) failed. "
                 "Returning nullptr.\n";

    return std::shared_ptr<OTCachedKey>();
}

// static
void OTCachedKey::Cleanup()
{
    std::lock_guard<std::mutex> lock(OTCachedKey::s_mutexCachedKeys);

    s_mapCachedKeys.clear();

    //    while (!s_mapCachedKeys.empty())
    //    {
    //        OTCachedKey * pTemp = s_mapCachedKeys.begin()->second;
    //        OT_ASSERT(nullptr != pTemp);
    //        s_mapCachedKeys.erase(s_mapCachedKeys.begin());
    //        delete pTemp; pTemp = nullptr;
    //    }
}

OTCachedKey::OTCachedKey(int32_t nTimeoutSeconds)
    : m_pThread(nullptr)
    , m_nTimeoutSeconds(nTimeoutSeconds)
    , m_pMasterPassword(nullptr)
    , // This is created in GetMasterPassword, and destroyed by a timer thread
      // sometime soon after.
    m_bUse_System_Keyring(false)
    , // the config file might turn this on.
    m_pSymmetricKey(nullptr)
    , // OTServer OR OTWallet owns this key, and sets this pointer. It's the
      // encrypted form of s_pMasterPassword.
    m_bPaused(false)
{
}

OTCachedKey::OTCachedKey(const OTASCIIArmor& ascCachedKey)
    : m_pThread(nullptr)
    , m_nTimeoutSeconds(OTCachedKey::It()->GetTimeoutSeconds())
    , m_pMasterPassword(nullptr)
    , // This is created in GetMasterPassword, and destroyed by a timer thread
      // sometime soon after.
    m_bUse_System_Keyring(OTCachedKey::It()->IsUsingSystemKeyring())
    , // this master key instance will decide to use the system keyring based on
      // what the global master key instance is set to do. (So we get the same
      // settings from config file, etc.)
    m_pSymmetricKey(nullptr)
    , // OTServer OR OTWallet owns this key, and sets this pointer. It's the
      // encrypted form of s_pMasterPassword.
    m_bPaused(false)
{
    OT_ASSERT(ascCachedKey.Exists());
    SetCachedKey(ascCachedKey);
}

// We don't lock the mutex here because otherwise we'll freeze ourselves.
//
// UPDATE: Adding lock since we now use recursive mutexes. SHOULD be fine.
// HMM, weird: it's still freezing here. Commenting this one out for now.
//
bool OTCachedKey::isPaused()
{
    //  std::lock_guard<std::mutex> lock(m_Mutex);

    return m_bPaused;
}

// When the master key is on pause, it won't work (Nyms will just use their
// own passwords instead of the master password.) This is important, for
// example,
// if you are loading up a bunch of Old Nyms. You pause before and after each
// one,
// and THEN convert them to the master key.
//
bool OTCachedKey::Pause()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!m_bPaused) {
        m_bPaused = true;
        return true;
    }
    return false;
}

bool OTCachedKey::Unpause()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_bPaused) {
        m_bPaused = false;
        return true;
    }
    return false;
}

// This should ONLY be called from a function that locks the Mutex first!
//
// UPDATE: SHOULD be fine to lock the mutex since we now use recursive mutexes.
// However, I did verify that this function is only called in other functions
// that
// have already locked it, so I'm leaving the call out for now.
//
void OTCachedKey::LowLevelReleaseThread()
{
    // NO NEED TO LOCK THIS ONE -- BUT ONLY CALL IT FROM A LOCKED FUNCTION.
    if (nullptr != m_pThread) {
        std::unique_ptr<std::thread> pThread(m_pThread);
        m_pThread = nullptr;

        if (pThread->joinable()) {
            pThread->detach();
        }
    }
}

OTCachedKey::~OTCachedKey()
{
    std::lock_guard<std::mutex> lock(
        m_Mutex); // I figured this would cause some kind of problem but how
                  // else can I mess with the members unless I lock this?

    LowLevelReleaseThread();

    if (nullptr != m_pMasterPassword) // Only stored temporarily, the purpose of
    // this class is to destoy it after a timer.
    {
        OTPassword* pPassword = m_pMasterPassword;

        m_pMasterPassword = nullptr;

        delete pPassword;
        pPassword = nullptr;
    }

    if (nullptr !=
        m_pSymmetricKey) // Owned / based on a string passed in. Stored
                         // somewhere else (OTServer, OTWallet...)
    {
        OTSymmetricKey* pSymmetricKey = m_pSymmetricKey;

        m_pSymmetricKey = nullptr;

        delete pSymmetricKey;
        pSymmetricKey = nullptr;
    }
}

int32_t OTCachedKey::GetTimeoutSeconds()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    const int32_t nTimeout = m_nTimeoutSeconds;

    return nTimeout;
}

void OTCachedKey::SetTimeoutSeconds(int32_t nTimeoutSeconds) // So we can load
                                                             // from the config
                                                             // file.
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    OT_ASSERT_MSG(nTimeoutSeconds >= (-1), "OTCachedKey::SetTimeoutSeconds: "
                                           "ASSERT: nTimeoutSeconds must be >= "
                                           "(-1)\n");

    m_nTimeoutSeconds = nTimeoutSeconds;
}

// Called by OTServer or OTWallet, or whatever instantiates those.
//
void OTCachedKey::SetCachedKey(const OTASCIIArmor& ascCachedKey)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    OT_ASSERT(ascCachedKey.Exists());

    if (nullptr != m_pSymmetricKey) {
        otErr << "OTCachedKey::SetCachedKey: Warning: This was already set. "
                 "(Re-setting.)\n";

        OTSymmetricKey* pSymmetricKey = m_pSymmetricKey;

        m_pSymmetricKey = nullptr;

        delete pSymmetricKey;
        pSymmetricKey = nullptr;
    }

    m_pSymmetricKey = new OTSymmetricKey;
    OT_ASSERT(nullptr != m_pSymmetricKey);

    // const bool bSerialized =
    m_pSymmetricKey->SerializeFrom(ascCachedKey);
}

// Above version deletes the internal symmetric key if it already exists,
// and then below that, creates it again if it does not exist. Then serializes
// it up from storage via ascCachedKey (input.)
// Whereas below version, if internal symmetric key doesn't exist, simply
// returns false.  Therefore if it's "not generated" and you want to load it
// up from some input, call the above function, SetCachedKey().

// Apparently SerializeFrom (as of this writing) is only used in OTEnvelope.cpp
// whereas SetCachedKey (above) is used in OTWallet and OTServer.
//
bool OTCachedKey::SerializeFrom(const OTASCIIArmor& ascInput)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (nullptr == m_pSymmetricKey) return false;

    return m_pSymmetricKey->SerializeFrom(ascInput);
}

bool OTCachedKey::SerializeTo(OTASCIIArmor& ascOutput)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (nullptr == m_pSymmetricKey) return false;

    return m_pSymmetricKey->SerializeTo(ascOutput);
}

// Note: this calculates its ID based only on m_dataEncryptedKey,
// and does NOT include salt, IV, iteration count, etc when
// generating the hash for the ID.
//
bool OTCachedKey::GetIdentifier(OTIdentifier& theIdentifier) const
{
    std::lock_guard<std::mutex> lock((const_cast<OTCachedKey*>(this))->m_Mutex);

    if ((nullptr == m_pSymmetricKey) || !m_pSymmetricKey->IsGenerated())
        return false;

    m_pSymmetricKey->GetIdentifier(theIdentifier);
    return true;
}

bool OTCachedKey::GetIdentifier(OTString& strIdentifier) const
{
    std::lock_guard<std::mutex> lock((const_cast<OTCachedKey*>(this))->m_Mutex);

    if ((nullptr == m_pSymmetricKey) || !m_pSymmetricKey->IsGenerated())
        return false;

    m_pSymmetricKey->GetIdentifier(strIdentifier);
    return true;
}

/*
 // TOdo: make this so you can pass in a password, or you can pass nullptr
 // and then it will use the GetPasswordCallback() method to collect one
 // from the user.

 OT_OPENSSL_CALLBACK * OTAsymmetricKey::GetPasswordCallback()

 #define OPENSSL_CALLBACK_FUNC(name) extern "C" (name)(char* buf, int32_t size,
 int32_t rwflag, void* userdata)

 */

// Caller must delete!
// static
std::shared_ptr<OTCachedKey> OTCachedKey::CreateMasterPassword(
    OTPassword& theOutput, const char* szDisplay, int32_t nTimeoutSeconds)
{
    std::shared_ptr<OTCachedKey> pMaster(new OTCachedKey(nTimeoutSeconds));

    const OTString strDisplay(
        (nullptr == szDisplay)
            ? "Creating a passphrase..."
            : szDisplay); // todo internationalization / hardcoding.

    const bool bGotPassphrase =
        pMaster->GetMasterPassword(pMaster, theOutput, strDisplay.Get(),
                                   true); // bool bVerifyTwice=false by default.
                                          // Really we didn't have to pass true
                                          // here, since it asks twice anyway,
                                          // when first generating the key.

    if (bGotPassphrase) // success!
        return pMaster;

    // If we're still here, that means bGotPassphrase failed.
    //
    //    delete pMaster; pMaster = nullptr;
    return std::shared_ptr<OTCachedKey>();
}

// Called by the password callback function.
// The password callback uses this to get the password for any individual Nym.
// This will also generate the master password, if one does not already exist.
//
bool OTCachedKey::GetMasterPassword(std::shared_ptr<OTCachedKey>& mySharedPtr,
                                    OTPassword& theOutput,
                                    const char* szDisplay, bool bVerifyTwice)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::string str_display(
        nullptr != szDisplay ? szDisplay : "(Display string was blank.)");

    const char* szFunc = "OTCachedKey::GetMasterPassword";

    //  OT_ASSERT(nullptr != m_pSymmetricKey); // (This had better be set
    // already.)
    // // Took this out because calling Generate inside here now.

    //
    if (nullptr != m_pMasterPassword) {
        otInfo << szFunc
               << ": Master password was available. (Returning it now.)\n";

        theOutput = *m_pMasterPassword;
        return true;
    }

    otInfo << szFunc << ": Master password wasn't loaded. Instantiating...\n";

    // If m_pMasterPassword is nullptr, (which below this point it is) then...
    //
    // Either it hasn't been created yet, in which case we need to instantiate
    // it,
    // OR it expired, in which case m_pMasterPassword is nullptr,
    // but m_pThread isn't, and still needs cleaning up before we instantiate
    // another one!
    //
    LowLevelReleaseThread();

    m_pMasterPassword =
        OTCrypto::It()->InstantiateBinarySecret(); // already asserts.

    /*
    How does this work?

    When trying to open a normal nym, the password callback realizes we are
    calling it
    in "NOT master mode", so instead of just collecting the passphrase and
    giving it
    back to OpenSSL, it calls this function first, which returns the master
    password
    (so that IT can be given to OpenSSL instead.)

    If the master wasn't already loaded (common) then we call the callback in
    here ourselves.
    Notice it's recursive! But this time, the callback sees we ARE in master
    mode, so it doesn't
    call this function again (which would be an infinite loop.) Instead, it
    collects the password
    as normal, only instead of passing it back to the caller via the buffer, it
    uses the
    passUserInput by attaching it to thePWData before the call. That way the
    callback function
    can set passUserInput with whatever it retrieved from the user, and then
    back in this function
    again we can get the passUserInput and use it to unlock the MASTER
    passphrase, which we set
    onto theOutput.

    When this function returns true, the callback (0th level of recursion) uses
    theOutput
    as the "passphrase" for all Nyms, passing it to OpenSSL.

    This way, OpenSSL gets a random key instead of a passphrase, and the
    passphrase is just used
    for encrypting that random key whenever its timer has run out.

    */

    bool bReturnVal = false;

    // CALL the callback directly. (To retrieve a passphrase so I can use it in
    // GenerateKey
    // and GetRawKey.)
    //
    // int32_t OT_OPENSSL_CALLBACK (char* buf, int32_t size, int32_t rwflag,
    // void *userdata);
    //
    // For us, it will set passUserInput to the password from the user, and
    // return
    // a simple 1 or 0 (instead of the length.) buf and size can be nullptr / 0,
    // and
    // rwflag should be passed in from somewhere.
    //
    // m_pSymmetricKey is the encrypted form of the master key. Therefore we
    // want to hash
    // it, in order to get the ID for lookups on the keychain.
    //
    OTPassword* pDerivedKey = nullptr;
    std::unique_ptr<OTPassword> theDerivedAngel;

    if (nullptr == m_pSymmetricKey) {
        m_pSymmetricKey = new OTSymmetricKey;
        OT_ASSERT(nullptr != m_pSymmetricKey);
    }

    if (!m_pSymmetricKey->IsGenerated()) // doesn't already exist.
    {
        otWarn << szFunc << ": Master key didn't exist. Need to collect a "
                            "passphrase from the user, "
                            "so we can generate a master key...\n ";

        bVerifyTwice = true; // we force it, in this case.
    }
    else // If the symmetric key itself ALREADY exists (which it usually
           // will...)
    { // then we might have also already stashed the derived key on the system
        // keychain. Let's check there first before asking the user to enter his
        // passphrase...
        //

        const OTIdentifier idCachedKey(
            *m_pSymmetricKey); // Grab the ID of this symmetric key.
        const OTString strCachedKeyHash(
            idCachedKey); // Same thing, in string form.
        //
        // This only happens in here where we KNOW m_pSymmetricKey was already
        // generated.
        //
        //      OTString strCachedKeyHash;
        //      m_pSymmetricKey->GetIdentifier(strCachedKeyHash);

        pDerivedKey =
            OTCrypto::It()->InstantiateBinarySecret(); // pDerivedKey is
                                                       // instantiated here to
                                                       // use as output argument
                                                       // below.

        //
        // *** ATTEMPT to RETRIEVE the *Derived Key* from THE SYSTEM KEYCHAIN
        // ***
        //
        const bool bFoundOnKeyring =
            IsUsingSystemKeyring() &&
            OTKeyring::RetrieveSecret(
                strCachedKeyHash, // HASH OF ENCRYPTED MASTER KEY
                *pDerivedKey,     // (Output) RETRIEVED PASSWORD.
                str_display);     // optional display string.

        if (bFoundOnKeyring) // We found it -- but does it WORK?
        {
            const bool bCachedKey = m_pSymmetricKey->GetRawKeyFromDerivedKey(
                *pDerivedKey, *m_pMasterPassword);

            //
            // Note: What IS the secret? We don't want it to be the user's
            // passphrase that he TYPES.
            // We also don't want it to be the eventual (random) key that
            // unlocks the private keys.
            // Rather, we want it to be the intermediary key, generated from the
            // user's passphrase via
            // a key-derivation algorithm, which is then used to unlock the
            // (random) symmetric key that
            // actually unlocks the private keys.
            // This way the symmetric key itself can be kept locked at ALL
            // times, and instead, we have the
            // derived key on the timer, use it to unlock the symmetric key
            // EVERY TIME we use that, and
            // IMMEDIATELY throw it away afterwards, since we can still open it
            // again (until the timeout) by
            // using the derived key.
            // This is slick because the user doesn't directly enter the derived
            // key, and neither is it
            // directly used for unlocking private keys -- so it's preferable to
            // store in RAM than those things.
            //
            //
            // 1. Make sure the above description is actually what we DO do now.
            // (UPDATE: for keyring, yes. For OT internally, no.)
            // 2. Make sure the derived key, as described above, is also what is
            // stored as the SECRET, here! (UPDATE: Yes!)
            //    (i.e. in other processes such as Mac Keychain or Gnome.)
            // 3. Done. Need to add ability for OTIdentifier to hash
            // OTSymmetricKey, so we can use it for strUser above. DONE.
            //
            // UPDATE: the master key cached inside OT (on a timer) is not the
            // derived key, but the master key itself
            // that's used on the private keys. However, the one we're caching
            // in the system keyring IS the derived key,
            // and not the master key. So for example, if an attacker obtained
            // the derived key from the system keyring,
            //

            if (bCachedKey) // It works!
            {
                otWarn << szFunc << ": Finished calling "
                                    "m_pSymmetricKey->GetRawKeyFromDerivedKey "
                                    "(Success.)\n";
                theOutput = *m_pMasterPassword; // Return it to the caller.
                theDerivedAngel.reset(
                    pDerivedKey);  // Set our own copy to be destroyed later. It
                                   // continues below as "NOT nullptr".
                bReturnVal = true; // Success.
            }
            else                 // It didn't unlock with the one we found.
            {
                otOut << szFunc << ": Unable to unlock master key using "
                                   "derived key found on system keyring.\n";
                delete pDerivedKey;
                pDerivedKey = nullptr; // Below, this function checks
                                       // pDerivedKey for nullptr.
            }
        }
        else // NOT found on keyring.
        {
            if (IsUsingSystemKeyring()) // We WERE using the keying, but
                                        // we DIDN'T find the derived key.
                otWarn << szFunc
                       << ": Unable to find derived key on system keyring.\n";
            // (Otherwise if we WEREN'T using the system keyring, then of course
            // we didn't find any derived key cached there.)
            delete pDerivedKey;
            pDerivedKey =
                nullptr; // Below, this function checks pDerivedKey for nullptr.
        }
    }

    // NOT found on Keyring...
    //
    if (nullptr == pDerivedKey) // Master key was not cached in OT, nor was it
                                // found in the system keychain.
    { // Therefore we HAVE to ask the user for a passphrase and decrypt it
        // ourselves,
        // since we DO have an encrypted version of the key...

        // This time we DEFINITELY force the user input, since we already played
        // our hand.
        // If the master key was still in memory we would have returned already,
        // above.
        // Then we tried to find it on the keyring and we couldn't find it, so
        // now we have
        // to actually ask the user to enter it.
        //

        std::string default_password(OT_DEFAULT_PASSWORD); // default password
        OTPassword passwordDefault;
        passwordDefault.zeroMemory();
        passwordDefault.setPassword(
            default_password.c_str(),
            static_cast<int32_t>(default_password.length()));

        OTPassword passUserInput;
        passUserInput.zeroMemory(); // text mode.
        OTPasswordData thePWData(str_display.c_str(), &passUserInput,
                                 mySharedPtr); // these pointers are only passed
                                               // in the case where it's for a
                                               // master key.
        //      otInfo << "*********Begin OTCachedKey::GetMasterPassword:
        // Calling souped-up password cb...\n * *  * *  * *  * *  * ");

        // It's possible this is the first time this is happening, and the
        // master key
        // hasn't even been generated yet. In which case, we generate it here...
        //
        bool bGenerated = m_pSymmetricKey->IsGenerated();

        if (!bGenerated) // This Symmetric Key hasn't been generated before....
        {

            if (!OTAsymmetricKey::GetPasswordCallback()(
                    nullptr, 0, bVerifyTwice ? 1 : 0,
                    static_cast<void*>(&thePWData))) {
                otErr << __FUNCTION__ << ": Failed to get password from user!";
                return false;
            }

            // If the length of the user supplied password is less than 4
            // characters int64_t, we are going to use the default password!
            bool bUsingDefaultPassword = false;
            {
                if (4 > std::string(passUserInput.getPassword()).length()) {
                    otOut << "\n Password entered was less than 4 characters "
                             "int64_t! This is NOT secure!!\n"
                             "... Assuming password is for testing only... "
                             "setting to default password: "
                          << OT_DEFAULT_PASSWORD << " \n";
                    bUsingDefaultPassword = true;
                }
            }

            //          otOut << "%s: Calling
            // m_pSymmetricKey->GenerateKey()...\n", szFunc);

            bGenerated = m_pSymmetricKey->GenerateKey(
                bUsingDefaultPassword ? passwordDefault : passUserInput,
                &pDerivedKey); // derived key is optional here.
            //
            // Note: since I passed &pDerivedKey in the above call, then **I**
            // am responsible to
            // check it for nullptr, and delete it if there's something there!
            //
            if (nullptr != pDerivedKey)
                theDerivedAngel.reset(pDerivedKey);
            else
                otErr << __FUNCTION__ << ": FYI: Derived key is still nullptr "
                                         "after calling "
                                         "OTSymmetricKey::GenerateKey.\n";

            //          otOut << "%s: Finished calling
            // m_pSymmetricKey->GenerateKey()...\n", szFunc);
        }
        else // m_pSymmetricKey->IsGenerated() == true. (Symmetric Key is
               // already generated.)
        {

            // Generate derived key from passphrase.
            //
            // We generate the derived key here so that
            // GetRawKeyFromPassphrase() call (below)
            // works with it being passed in. (Because the above call to
            // GenerateKey also grabs
            // a copy of the derived key and passes it in below to the same
            // GetRawKeyFromPassphrase.)
            //
            // So WHY are we keeping a copy of the derived key through these
            // calls? Otherwise they
            // would all individually generate it, which is a waste of
            // resources. Also, we want to have
            // our grubby hands on the derived key at the end so we can add it
            // to the system keyring
            // (below), and we'd just end up having to derive it AGAIN in order
            // to do so.
            //
            if (m_pSymmetricKey->HasHashCheck()) {
                pDerivedKey =
                    m_pSymmetricKey->CalculateDerivedKeyFromPassphrase(
                        passwordDefault); // asserts already.

                if (nullptr == pDerivedKey) {
                    otOut << "\n\n" << __FUNCTION__
                          << ": Please enter your password.\n\n";

                    for (;;) // bad passphase (as the calculate key returned
                             // nullptr)
                    {
                        if (!OTAsymmetricKey::GetPasswordCallback()(
                                nullptr, 0, false,
                                static_cast<void*>(&thePWData))) {
                            otErr << "\n\n" << __FUNCTION__
                                  << ": Failed to get password from user!\n\n";
                            return false;
                        }
                        pDerivedKey =
                            m_pSymmetricKey->CalculateDerivedKeyFromPassphrase(
                                passUserInput);            // asserts already.
                        if (nullptr != pDerivedKey) break; // success

                        otOut << "\n\n" << __FUNCTION__
                              << ": Wrong Password, Please Try Again.\n\n";
                    }
                }
            }
            else {
                otOut << "\n Please enter your current password twice, (not a "
                         "new password!!) \n";

                if (!OTAsymmetricKey::GetPasswordCallback()(
                        nullptr, 0, true, static_cast<void*>(&thePWData))) {
                    otErr << __FUNCTION__
                          << ": Failed to get password from user!";
                    return false;
                }

                pDerivedKey =
                    m_pSymmetricKey->CalculateNewDerivedKeyFromPassphrase(
                        passUserInput); // asserts already.
                OT_ASSERT(nullptr != pDerivedKey);
            }
            theDerivedAngel.reset(pDerivedKey);

            otWarn << szFunc << ": FYI, symmetric key was already generated. "
                                "Proceeding to try and use it...\n";

            // bGenerated is true, if we're even in this block in the first
            // place.
            // (No need to set it twice.)
        }

        // Below this point, pDerivedKey could still be null.
        // (And we only clean it up later if we created it.)
        // Also, bGenerated could still be false. (Like if it wasn't
        // generated, then generation itself failed, then it's still false.)
        //
        // Also, even if it was already generated, or if it wasn't but then
        // successfully did,
        //

        if (bGenerated) // If SymmetricKey (*this) is already generated.
        {
            otInfo
                << szFunc
                << ": Calling m_pSymmetricKey->GetRawKeyFromPassphrase()...\n";

            // Once we have the user's password, then we use it to GetKey from
            // the OTSymmetricKey (which
            // is encrypted) and that retrieves the cleartext master password
            // which we set here and also
            // return a copy of.
            //
            // Note: if pDerivedKey was derived above already, which it should
            // have been, then it will
            // be not-nullptr, and will be used here, and will be used
            // subsequently
            // for adding to the system
            // keychain. Otherwise, it will be nullptr, and
            // GetRawKeyFromPassphrase
            // will thus just derive its
            // own copy of the derived key internally. It will still work, but
            // then back up here, it will
            // NOT be added to the system keyring, since it's still nullptr back
            // up
            // here.
            // (FYI.)
            //
            const bool bCachedKey = m_pSymmetricKey->GetRawKeyFromPassphrase(
                passUserInput, *m_pMasterPassword, pDerivedKey);
            if (bCachedKey) {
                otInfo << szFunc << ": Finished calling "
                                    "m_pSymmetricKey->GetRawKeyFromPassphrase "
                                    "(Success.)\n";
                theOutput = *m_pMasterPassword; // Success!

                // Store the derived key to the system keyring.
                //
                if (IsUsingSystemKeyring() && (nullptr != pDerivedKey)) {
                    const std::string str_display(
                        nullptr != szDisplay ? szDisplay
                                             : "(Display string was blank.)");

                    const OTIdentifier idCachedKey(*m_pSymmetricKey);
                    const OTString strCachedKeyHash(
                        idCachedKey); // Same thing, in string form.

                    //                      const bool bStored =
                    OTKeyring::StoreSecret(
                        strCachedKeyHash, // HASH OF ENCRYPTED MASTER KEY
                        *pDerivedKey,     // (Input) Derived Key BEING STORED.
                        str_display);     // optional display string.
                }
                else
                    otWarn << szFunc << ": Strange: Problem with either: "
                                        "IsUsingSystemKeyring"
                                        " ("
                           << (IsUsingSystemKeyring() ? "true" : "false")
                           << ") "
                              "or: (nullptr != pDerivedKey) ("
                           << ((nullptr != pDerivedKey) ? "true" : "false")
                           << ")\n";

                bReturnVal = true;
            }
            else
                otOut
                    << szFunc
                    << ": m_pSymmetricKey->GetRawKeyFromPassphrase() failed.\n";
        } // bGenerated
        else
            otErr << szFunc << ": bGenerated is still false, even after trying "
                               "to generate it, yadda yadda yadda.\n";

    } // nullptr == pDerivedKey

    if (bReturnVal) // Start the thread!
    {
//      otLog4 << "%s: starting up new thread, so we can expire the master key
// from RAM.\n", szFunc);

#if defined(OPENSSL_THREADS)
        // thread support enabled

        otInfo << szFunc << ": Starting thread for Master Key...\n";

        std::shared_ptr<OTCachedKey>* pthreadSharedPtr =
            new std::shared_ptr<OTCachedKey>(mySharedPtr); // TODO: memory leak.

        m_pThread = new std::thread(OTCachedKey::ThreadTimeout,
                                    static_cast<void*>(pthreadSharedPtr));

#else
        // no thread support

        otErr << szFunc
              << ": WARNING: OpenSSL was NOT compiled with thread support. "
                 "(Master Key will not expire.)\n";

#endif

    }
    else if (m_nTimeoutSeconds != (-1)) {
        if (nullptr != m_pMasterPassword) {
            OTPassword* pMasterPassword = m_pMasterPassword;

            m_pMasterPassword = nullptr;

            delete pMasterPassword;
            pMasterPassword = nullptr;
        }
    }
    // Since we have set the cleartext master password, We also have to fire up
    // the thread
    // so it can timeout and be destroyed. In the meantime, it'll be stored in
    // an OTPassword
    // which has these security precautions:
    /*
    1. Zeros memory in a secure and cross-platform way, in its destructor.
    2. OT_Init() uses setrlimit to prevent core dumps.
    3. Uses VirtualLock and mlock to reduce/prevent swapping RAM to hard drive.
    4. (SOON) will use VirtualProtect on Windows (standard API for protected
    memory)
    5. (SOON) and similarly have option in config file for ssh-agent, gpg-agent,
    etc.
    6. Even without those things,the master password is stored in an encrypted
    form after it times out.
    7. While decrypted (while timer is going) it's still got the above security
    mechanisms,
    plus options for standard protected-memory APIs are made available wherever
    possible.
    8. The actual passphrase the user types is not stored in memory, except just
    int64_t enough to
    use it to derive another key, used to unlock the actual key (for a temporary
    period of time.)
    9. Meanwhile the actual key is stored in encrypted form on disk, and the
    derived key isn't stored anywhere.
    10. Ultimately external hardware, and smart cards, are the way to go. But OT
    should still do the best possible.
    */

    return bReturnVal;
}

// static
// This is the thread itself.
//
void OTCachedKey::ThreadTimeout(void* pArg)
{
    // TODO: Save a copy of pArg, in the cached key object, and delete it
    // whenever LowLevelRemoveThread
    // is called. Otherwise it's a memory leak.
    //
    std::shared_ptr<OTCachedKey>* pthreadSharedPtr =
        static_cast<std::shared_ptr<OTCachedKey>*>(pArg);
    std::shared_ptr<OTCachedKey> pMyself = *pthreadSharedPtr;

    if (!pMyself) {
        OT_FAIL_MSG("OTCachedKey::ThreadTimeout: Need ptr to master key here, "
                    "that activated this thread.\n");
    }

    //    std::lock_guard<std::mutex> lock(*(pMyself->GetMutex()));

    int32_t nTimeoutSeconds = 0;

    {
        std::lock_guard<std::mutex> lock(OTCachedKey::s_mutexThreadTimeout);

        if (pMyself) {
            nTimeoutSeconds =
                pMyself->GetTimeoutSeconds(); // locks mutex internally.
        }
    }

    if (nTimeoutSeconds > 0) {
        if (pMyself)
            std::this_thread::sleep_for(
                std::chrono::seconds(nTimeoutSeconds)); // <===== ASLEEP!
    }

    {
        std::lock_guard<std::mutex> lock(OTCachedKey::s_mutexThreadTimeout);

        if (pMyself && (nTimeoutSeconds != (-1))) {
            pMyself->DestroyMasterPassword(); // locks mutex internally.
        }
    }
}

// Called by the thread.
// The cleartext version (m_pMasterPassword) is deleted and set nullptr after a
// Timer of X seconds.
// (Timer thread calls this.) The instance that owns each thread will pass its
// instance pointer
// as pArg so we can access the timeout seconds and the mutex, and the password
// we're destroying.
//
void OTCachedKey::DestroyMasterPassword()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    //
    if (m_nTimeoutSeconds != (-1)) {
        // (m_pSymmetricKey stays.
        //  m_pMasterPassword only is destroyed.)
        //
        if (nullptr != m_pMasterPassword) {
            OTPassword* pPassword = m_pMasterPassword;

            m_pMasterPassword = nullptr;

            delete pPassword;
            pPassword = nullptr;
        }
    }
    // (We do NOT call LowLevelReleaseThread(); here, since the thread is
    // what CALLED this function. Instead, we destroy / nullptr the master
    // password,
    // so that next time around we will see it is nullptr and THEN we will know
    // to
    // call LowLevelReleaseThread(); before instantiating a new one.)

    // New: When the master password is destroyed here, we also remove it from
    // the system keychain:
    //
    if (nullptr != m_pSymmetricKey) {
        const std::string str_display;

        const OTIdentifier idCachedKey(*m_pSymmetricKey);
        const OTString strCachedKeyHash(
            idCachedKey); // Same thing, in string form.

        const bool bDeletedSecret =
            IsUsingSystemKeyring() &&
            OTKeyring::DeleteSecret(
                strCachedKeyHash, // HASH OF ENCRYPTED MASTER KEY
                str_display);     // "optional" display string.
        if (bDeletedSecret) {
            otOut << "OTCachedKey::DestroyMasterPassword: FYI, deleted "
                     "the derived key (used for unlocking the master key "
                     "password) "
                     "from system keychain at the same time as we deleted the "
                     "master key "
                     "password from OT internally, due to password timeout.\n";
        }
    }
}

// If you actually want to create a new key, and a new passphrase, then use this
// to destroy every last vestige of the old one. (Which will cause a new one to
// be automatically generated the next time OT requests the master key.) NOTE:
// Make SURE you have all your Nyms loaded up and unlocked before you call this.
// Then save them all again so they will be properly stored with the new master
// key.

void OTCachedKey::ResetMasterPassword()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    LowLevelReleaseThread();

    if (nullptr != m_pMasterPassword) {
        OTPassword* pPassword = m_pMasterPassword;

        m_pMasterPassword = nullptr;

        delete pPassword;
        pPassword = nullptr;
    }

    if (nullptr != m_pSymmetricKey) {
        // We also remove it from the system keychain:
        //
        const std::string str_display;

        const OTIdentifier idCachedKey(
            *m_pSymmetricKey); // Symmetric Key ID of the Master key.
        const OTString strCachedKeyHash(
            idCachedKey); // Same thing, in string form.

        const bool bDeletedSecret =
            IsUsingSystemKeyring() &&
            OTKeyring::DeleteSecret(
                strCachedKeyHash, // HASH OF ENCRYPTED MASTER KEY
                str_display);     // "optional" display string.
        if (bDeletedSecret) {
            otOut << "OTCachedKey::ResetMasterPassword: FYI, deleted "
                     "the derived key (used for unlocking the master key "
                     "password) "
                     "from system keychain at the same time as we deleted the "
                     "master key "
                     "itself, presumably due to the passphrase being reset.\n";
        }

        // Now wipe the symmetric key itself (so it can later be
        // re-created as a new key.)
        //
        if (nullptr != m_pSymmetricKey) {
            OTSymmetricKey* pSymmetricKey = m_pSymmetricKey;

            m_pSymmetricKey = nullptr;

            delete pSymmetricKey;
            pSymmetricKey = nullptr;
        }
    }
}

} // namespace opentxs
