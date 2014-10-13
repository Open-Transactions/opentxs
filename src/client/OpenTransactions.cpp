/************************************************************
*
*  OpenTransactions.cpp
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

#include <opentxs/client/OpenTransactions.hpp>
#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTClient.hpp>
#include <opentxs/client/OTServerConnection.hpp>
#include "Helpers.hpp"
#include <opentxs/client/OTWallet.hpp>
#include <opentxs/client/TransportCallback.hpp>

#include <opentxs/ext/InstantiateContract.hpp>
#include <opentxs/ext/OTPayment.hpp>
#include <opentxs/ext/Socket_ZMQ4.hpp>

#include <opentxs/cash/Mint.hpp>
#include <opentxs/cash/Purse.hpp>
#include <opentxs/cash/Token.hpp>

#include <opentxs/basket/Basket.hpp>

#include <opentxs/core/recurring/OTPaymentPlan.hpp>
#include <opentxs/core/script/OTAgent.hpp>
#include <opentxs/core/script/OTBylaw.hpp>
#include <opentxs/core/script/OTParty.hpp>
#include <opentxs/core/script/OTPartyAccount.hpp>
#include <opentxs/core/script/OTSmartContract.hpp>
#include <opentxs/core/trade/OTTrade.hpp>
#include <opentxs/core/trade/OTOffer.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/crypto/OTNymOrSymmetricKey.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/crypto/OTSymmetricKey.hpp>
#include <opentxs/core/OTAssetContract.hpp>
#include <opentxs/core/OTCheque.hpp>
#include <opentxs/core/util/OTDataFolder.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/OTLedger.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTMessage.hpp>
#include <opentxs/core/util/OTPaths.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTIdentifier.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTServerContract.hpp>
#include <opentxs/core/OTStorage.hpp>

#if defined(OT_KEYRING_FLATFILE)
#include <opentxs/core/OTKeyring.hpp>
#endif

#include <cassert>
#include <fstream>
#include <memory>

#include <time.h>

#ifndef WIN32
#include <unistd.h>
#endif

#if defined(OPENTXS_HAVE_NETINET_IN_H)
extern "C" {
#include <netinet/in.h>
}
#endif

#define CLIENT_DEFAULT_LATENCY_SEND_MS 200
#define CLIENT_DEFAULT_LATENCY_SEND_NO_TRIES 7
#define CLIENT_DEFAULT_LATENCY_RECEIVE_MS 200
#define CLIENT_DEFAULT_LATENCY_RECEIVE_NO_TRIES 7
#define CLIENT_DEFAULT_LATENCY_DELAY_AFTER 50
#define CLIENT_DEFAULT_IS_BLOCKING false

#define CLIENT_CONFIG_KEY "client"
#define CLIENT_MASTER_KEY_TIMEOUT_DEFAULT 300
#define CLIENT_WALLET_FILENAME "wallet.xml"
#define CLIENT_USE_SYSTEM_KEYRING false
#define CLIENT_PID_FILENAME "ot.pid"

namespace opentxs
{

namespace
{

// There may be multiple matching receipts... this just returns the first one.
// It's used to verify that any are even there. The pointer is returned only for
// convenience.
//
// pass in the opening number for the cron item that this is a receipt for.
// CALLER RESPONSIBLE TO DELETE.
OTTransaction* GetPaymentReceipt(const mapOfTransactions& transactionsMap,
                                 int64_t lReferenceNum,
                                 OTPayment** ppPaymentOut)
{
    // loop through the transactions that make up this ledger.
    for (auto& it : transactionsMap) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        if (OTTransaction::paymentReceipt !=
            pTransaction->GetType()) // <=======
            continue;

        if (pTransaction->GetReferenceToNum() == lReferenceNum) {
            if (nullptr !=
                ppPaymentOut) // The caller might want a copy of this.
            {
                OTString strPayment;
                pTransaction->GetReferenceString(strPayment);

                if (!strPayment.Exists()) {
                    OTPayment* pPayment = new OTPayment(strPayment);
                    OT_ASSERT(nullptr != pPayment);

                    if (pPayment->IsValid() && pPayment->SetTempValues())
                        *ppPaymentOut =
                            pPayment; // CALLER RESPONSIBLE TO DELETE.
                    else {
                        otErr << __FUNCTION__ << ": Error: Failed loading up "
                                                 "payment instrument from "
                                                 "paymentReceipt.\n";
                        delete pPayment;
                        pPayment = nullptr;
                        *ppPaymentOut = nullptr;
                    }
                }
                else
                    otErr << __FUNCTION__ << ": Error: Unexpected: payment "
                                             "instrument was empty string, on "
                                             "a paymentReceipt.\n";
            }

            return pTransaction;
        }
    }

    return nullptr;
}

bool VerifyBalanceReceipt(OTPseudonym& SERVER_NYM, OTPseudonym& THE_NYM,
                          const OTIdentifier& SERVER_ID,
                          const OTIdentifier& ACCT_ID)
{
    OTIdentifier USER_ID(THE_NYM), SERVER_USER_ID(SERVER_NYM);
    OTString strServerID(SERVER_ID), strReceiptID(ACCT_ID);

    // Load the last successful BALANCE STATEMENT...

    OTTransaction tranOut(SERVER_USER_ID, ACCT_ID, SERVER_ID);

    OTString strFilename;
    strFilename.Format("%s.success", strReceiptID.Get());

    const char* szFolder1name = OTFolders::Receipt().Get(); // receipts
    const char* szFolder2name = strServerID.Get(); // receipts/SERVER_ID
    const char* szFilename =
        strFilename.Get(); // receipts/SERVER_ID/ACCT_ID.success

    if (!OTDB::Exists(szFolder1name, szFolder2name, szFilename)) {
        otWarn << "Receipt file doesn't exist in "
                  "OTTransaction::VerifyBalanceReceipt.\n";
        return false;
    }

    std::string strFileContents(
        OTDB::QueryPlainString(szFolder1name, szFolder2name,
                               szFilename)); // <=== LOADING FROM DATA STORE.

    if (strFileContents.length() < 2) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Error reading file: "
              << szFolder1name << OTLog::PathSeparator() << szFolder2name
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    OTString strTransaction(strFileContents.c_str());

    if (!tranOut.LoadContractFromString(strTransaction)) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Unable to load balance "
                 "statement:\n " << szFolder1name << OTLog::PathSeparator()
              << szFolder2name << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (tranOut.IsAbbreviated()) // should never happen
    {
        int64_t lBoxType = 0;

        if (tranOut.Contains("nymboxRecord"))
            lBoxType = static_cast<int64_t>(OTLedger::nymbox);
        else if (tranOut.Contains("inboxRecord"))
            lBoxType = static_cast<int64_t>(OTLedger::inbox);
        else if (tranOut.Contains("outboxRecord"))
            lBoxType = static_cast<int64_t>(OTLedger::outbox);
        else if (tranOut.Contains("paymentInboxRecord"))
            lBoxType = static_cast<int64_t>(OTLedger::paymentInbox);
        else if (tranOut.Contains("recordBoxRecord"))
            lBoxType = static_cast<int64_t>(OTLedger::recordBox);
        else if (tranOut.Contains("expiredBoxRecord"))
            lBoxType = static_cast<int64_t>(OTLedger::expiredBox);
        else {
            otErr << "OTTransaction::VerifyBalanceReceipt: Error loading from "
                     "abbreviated transaction: "
                     "unknown ledger type. (probably message but who knows.)\n";
            return false;
        }

        pTransaction = LoadBoxReceipt(tranOut, lBoxType);
        if (nullptr == pTransaction) {
            otErr << "OTTransaction::VerifyBalanceReceipt: Error loading from "
                     "abbreviated transaction: "
                     "failed loading box receipt.";
            return false;
        }

        theTransAngel.reset(pTransaction);
    }
    else
        pTransaction = &tranOut;

    if (!pTransaction->VerifySignature(SERVER_NYM)) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Unable to verify "
                 "SERVER_NYM signature on balance statement:\n "
              << szFolder1name << OTLog::PathSeparator() << szFolder2name
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    // At this point, pTransaction is successfully loaded and verified,
    // containing the last balance receipt.

    return pTransaction->VerifyBalanceReceipt(SERVER_NYM, THE_NYM);
}

} // namespace

// static
bool OT_API::bInitOTApp = false;
// static
bool OT_API::bCleanupOTApp = false;

class OT_API::Pid
{
public:
    Pid();
    ~Pid();
    void OpenPid(const OTString& strPidFilePath);
    void ClosePid();
    bool IsPidOpen() const;

private:
    bool m_bIsPidOpen;
    OTString m_strPidFilePath;
};

OT_API::Pid::Pid()
    : m_bIsPidOpen(false)
    , m_strPidFilePath("")
{
}

OT_API::Pid::~Pid()
{
    // nothing for now
}

void OT_API::Pid::OpenPid(const OTString& strPidFilePath)
{
    if (IsPidOpen()) {
        otErr << __FUNCTION__ << ": strPidFilePath is OPEN, MUST CLOSE BEFORE "
                                 "OPENING A NEW ONE!\n";
        OT_FAIL;
    }

    if (!strPidFilePath.Exists()) {
        otErr << __FUNCTION__ << ": strPidFilePath is Empty!\n";
        OT_FAIL;
    }
    if (3 > strPidFilePath.GetLength()) {
        otErr << __FUNCTION__ << ": strPidFilePath is Too Short! ("
              << strPidFilePath << ")\n";
        OT_FAIL;
    }

    otWarn << __FUNCTION__ << ": Using Pid File: " << strPidFilePath << "\n";
    m_strPidFilePath = strPidFilePath;

    {
        // 1. READ A FILE STORING THE PID. (It will already exist, if OT is
        // already running.)
        //
        // We open it for reading first, to see if it already exists. If it
        // does,
        // we read the number. 0 is fine, since we overwrite with 0 on shutdown.
        // But
        // any OTHER number means OT is still running. Or it means it was killed
        // while
        // running and didn't shut down properly, and that you need to delete
        // the pid file
        // by hand before running OT again. (This is all for the purpose of
        // preventing two
        // copies of OT running at the same time and corrupting the data
        // folder.)
        //
        std::ifstream pid_infile(m_strPidFilePath.Get());

        // 2. (IF FILE EXISTS WITH ANY PID INSIDE, THEN DIE.)
        //

        if (pid_infile.is_open()) // it existed already
        {
            uint32_t old_pid = 0;
            pid_infile >> old_pid;
            pid_infile.close();

            // There was a real PID in there.
            if (old_pid != 0) {
                const uint64_t lPID = old_pid;
                otErr
                    << "\n\n\nIS OPEN-TRANSACTIONS ALREADY RUNNING?\n\n"
                       "I found a PID (" << lPID
                    << ") in the data lock file, located at: "
                    << m_strPidFilePath << "\n\n"
                                           "If the OT process with PID " << lPID
                    << " is truly not running "
                       "anymore, "
                       "then just erase that file and restart.\nThis is "
                       "normally "
                       "cleaned "
                       "up during AppCleanup / AppShutdown. (Or should be.)\n";
#ifndef ANDROID
                m_bIsPidOpen = false;
                return;
#endif
            }
            // Otherwise, though the file existed, the PID within was 0.
            // (Meaning the previous instance of OT already set it to 0 as it
            // was shutting down.)
        }
        // Next let's record our PID to the same file, so other copies of OT
        // can't trample on US.

        // 3. GET THE CURRENT (ACTUAL) PROCESS ID.
        //
        uint64_t the_pid = 0;

#ifdef _WIN32
        the_pid = GetCurrentProcessId();
#else
        the_pid = getpid();
#endif

        // 4. OPEN THE FILE IN WRITE MODE, AND SAVE THE PID TO IT.
        //
        std::ofstream pid_outfile(m_strPidFilePath.Get());

        if (pid_outfile.is_open()) {
            pid_outfile << the_pid;
            pid_outfile.close();
            m_bIsPidOpen = true;
        }
        else {
            otErr << "Failed trying to open data locking file (to store "
                     "PID " << the_pid << "): " << m_strPidFilePath << "\n";
            m_bIsPidOpen = false;
        }
    }
}

void OT_API::Pid::ClosePid()
{
    if (!IsPidOpen()) {
        otErr
            << __FUNCTION__
            << ": strPidFilePath is CLOSED, WHY CLOSE A PID IF NONE IS OPEN!\n";
        OT_FAIL;
    }
    if (!m_strPidFilePath.Exists()) {
        otErr << __FUNCTION__ << ": m_strPidFilePath is Empty!\n";
        OT_FAIL;
    }

    // PID -- Set it to 0 in the lock file so the next time we run OT, it knows
    // there isn't
    // another copy already running (otherwise we might wind up with two copies
    // trying to write
    // to the same data folder simultaneously, which could corrupt the data...)
    //

    std::ofstream pid_outfile(m_strPidFilePath.Get());

    if (pid_outfile.is_open()) {
        uint32_t the_pid = 0;
        pid_outfile << the_pid;
        pid_outfile.close();
        m_bIsPidOpen = false;
    }
    else {
        otErr << "Failed trying to open data locking file (to wipe PID "
                 "back to 0): " << m_strPidFilePath << "\n";
        m_bIsPidOpen = true;
    }
}

bool OT_API::Pid::IsPidOpen() const
{
    return m_bIsPidOpen;
}

// Call this once per run of the software. (enforced by a static value)
//
// static
bool OT_API::InitOTApp()
{
    OT_ASSERT(!OT_API::bCleanupOTApp);

    if (!OT_API::bInitOTApp) {
        if (!OTLog::Init("client")) {
            assert(false);
        }

        otOut << "\n\nWelcome to Open Transactions -- version "
              << OTLog::Version() << "\n";

        otWarn << "(transport build: OTMessage -> OTEnvelope -> ZMQ )\n";

// SIGNALS
//
#if defined(OT_SIGNAL_HANDLING)
        //
        OTLog::SetupSignalHandler(); // <===== SIGNALS
                                     //
// This is optional! You can always remove it using the OT_NO_SIGNAL_HANDLING
//  option, and plus, the internals only execute once anyway. (It keeps count.)
#endif
        OTCrypto::It()->Init(); // (OpenSSL gets initialized here.)
        // TODO in the case of Windows, figure err into this return val somehow.
        // (Or log it or something.)
        //

        OT_API::bInitOTApp = true;
        return true;
    }
    else {
        otErr << __FUNCTION__
              << ": ERROR: This function can only be called once.\n";
        return false;
    }
}

// static
bool OT_API::CleanupOTApp()
{
    if (!OT_API::bInitOTApp) {
        otErr << __FUNCTION__ << ": WARNING: Never Successfully called:  "
              << "OT_API::InitCTX()"
              << "\n";
        OT_API::bInitOTApp = true;
        return false;
    }

    if (!OT_API::bCleanupOTApp) {
        OTCachedKey::Cleanup(); // it has a static list of dynamically allocated
                                // master keys that need to be cleaned up, if
                                // the application is shutting down.

        // We clean these up in reverse order from the Init function, which just
        // seems
        // like the best default, in absence of any brighter ideas.
        //
        OTCrypto::It()->Cleanup(); // (OpenSSL gets cleaned up here.)

        return true;
    }
    else {
        otErr << __FUNCTION__
              << ": ERROR: This function can only be called once.\n";
        return false;
    }
}

// The API begins here...
OT_API::OT_API()
    : m_pPid(new Pid())
    , m_bInitialized(false)
    , m_pTransportCallback(nullptr)
    , m_pSocket(new OTSocket_ZMQ_4())
    , m_strDataPath("")
    , m_strWalletFilename("")
    , m_strWalletFilePath("")
    , m_strConfigFilename("")
    , m_strConfigFilePath("")
    , m_pWallet(nullptr)
    , m_pClient(nullptr)

{
    if (!Init()) {
        Cleanup();
    }
}

OT_API::~OT_API()
{
    // DELETE AND SET nullptr
    //
    if (nullptr != m_pSocket) delete m_pSocket;
    m_pSocket = nullptr;

    if (nullptr != m_pTransportCallback) delete m_pTransportCallback;
    m_pTransportCallback = nullptr;
    if (nullptr != m_pWallet) delete m_pWallet;
    m_pWallet = nullptr;
    if (nullptr != m_pClient) delete m_pClient;
    m_pClient = nullptr;

    if (m_bInitialized)
        if (!(Cleanup())) // we only cleanup if we need to
        {
            OT_FAIL;
        }

    // this must be last!
    if (nullptr != m_pPid) delete m_pPid;
}

// Call this once per INSTANCE of OT_API.
//
// Theoretically, someday OTAPI can be the "OT_CTX" and we will be able to
// instantiate
// multiple instances of it within a single application.
//
// So you use OT_API::InitOTAPI to initialize the entire application, and then
// you use
// OT_API::Init() to initialize THIS "OT_CTX" (the OT_API object.)
//
// If not initialized yet, but then this function is successful, it will return
// true.
// If ALREADY initialized, this function still returns true.
// If initialization fails, it will return false, but you can just call it
// again.
//
bool OT_API::Init()
{
    if (true == m_bInitialized) {
        OTString strDataPath;
        bool bGetDataFolderSuccess = OTDataFolder::Get(strDataPath);
        OT_ASSERT_MSG(bGetDataFolderSuccess,
                      "OT_API::Init(): Error! Data path not set!");

        otErr << __FUNCTION__
              << ": OTAPI was already initialized. (Skipping) and using "
                 "path: " << strDataPath << "\n";
        return true;
    }

    if (!OTDataFolder::Init(CLIENT_CONFIG_KEY)) {
        otErr << __FUNCTION__ << ": Unable to Init data folders";
        OT_FAIL;
    }

    static bool bConstruct = false;

    if (!bConstruct) {
        bConstruct = true;
        m_pWallet = new OTWallet;
        m_pClient = new OTClient;
    }

    if (!LoadConfigFile()) {
        otErr << __FUNCTION__ << ": Unable to Load Config File!";
        OT_FAIL;
    }

    // PID -- Make sure we're not running two copies of OT on the same data
    // simultaneously here.
    //
    // we need to get the loacation of where the pid file should be.
    // then we pass it to the OpenPid function.
    OTString strDataPath = "";
    const bool bGetDataFolderSuccess = OTDataFolder::Get(strDataPath);

    {
        bool bExists = false, bIsNew = false;
        if (!OTPaths::ConfirmCreateFolder(strDataPath, bExists, bIsNew)) {
            return false;
        }
    }

    OTString strPIDPath = "";
    OTPaths::AppendFile(strPIDPath, strDataPath, CLIENT_PID_FILENAME);

    if (bGetDataFolderSuccess) m_pPid->OpenPid(strPIDPath);
    if (!m_pPid->IsPidOpen()) {
        m_bInitialized = false;
        return false;
    } // failed loading

    // This way, everywhere else I can use the default storage context (for now)
    // and it will work
    // everywhere I put it. (Because it's now set up...)
    //
    m_bDefaultStore = OTDB::InitDefaultStorage(
        OTDB_DEFAULT_STORAGE,
        OTDB_DEFAULT_PACKER); // We only need to do this once now.

    if (m_bDefaultStore) // success initializing default storage on OTDB.
    {
        otWarn << __FUNCTION__ << ": Success invoking OTDB::InitDefaultStorage";

        if (m_bInitialized)
            otWarn << __FUNCTION__ << ": m_pClient->InitClient() was already "
                                      "initialized. (Skipping.)\n";
        else {
            m_bInitialized = m_pClient->InitClient(*m_pWallet);
            if (m_bInitialized)
                otWarn << __FUNCTION__
                       << ": Success invoking m_pClient->InitClient() \n";
            else
                otErr << __FUNCTION__
                      << ": Failed invoking m_pClient->InitClient()\n";
        }
        return m_bInitialized;
    }
    else
        otErr << __FUNCTION__ << ": Failed invoking OTDB::InitDefaultStorage\n";
    return false;
}

bool OT_API::Cleanup()
{
    if (!m_pPid->IsPidOpen()) {
        return false;
    } // pid isn't open, just return false.

    m_pPid->ClosePid();
    if (m_pPid->IsPidOpen()) {
        OT_FAIL;
    } // failed loading
    return true;
}

// Get
bool OT_API::GetWalletFilename(OTString& strPath) const
{
    if (m_strWalletFilename.Exists()) {
        strPath = m_strWalletFilename;
        return true;
    }
    else {
        strPath.Set("");
        return false;
    }
}

// Set
bool OT_API::SetWalletFilename(const OTString& strPath)
{
    if (strPath.Exists()) {
        m_strWalletFilename = strPath;
        return true;
    }
    else
        return false;
}

bool OT_API::SetTransportCallback(TransportCallback* pTransportCallback)
{
    if (nullptr != pTransportCallback) {
        m_pTransportCallback = pTransportCallback;
        return true;
    }
    else
        return false;
}

TransportCallback* OT_API::GetTransportCallback() const
{
    if (nullptr != m_pTransportCallback) {
        return m_pTransportCallback;
    }
    else {
        OT_FAIL;
    }
}

// Load the configuration file.
//
bool OT_API::LoadConfigFile()
{
    // Setup Config File
    OTString strConfigPath, strConfigFilename;

    if (!OTDataFolder::IsInitialized()) {
        return false;
    }

    // Create Config Object (OTSettings)
    OTString strConfigFilePath = "";
    if (!OTDataFolder::GetConfigFilePath(strConfigFilePath)) {
        OT_FAIL;
    }
    OTSettings* p_Config = nullptr;
    p_Config = new OTSettings(strConfigFilePath);

    // First Load, Create new fresh config file if failed loading.
    if (!p_Config->Load()) {
        otOut << __FUNCTION__
              << ": Note: Unable to Load Config. Creating a new file: "
              << strConfigFilename << "\n";
        if (!p_Config->Reset()) return false;
        if (!p_Config->Save()) return false;
    }

    if (!p_Config->Reset()) return false;

    // Second Load, Throw Assert if Failed loading.
    if (!p_Config->Load()) {
        otErr << __FUNCTION__
              << ": Error: Unable to load config file: " << strConfigFilename
              << " It should exist, as we just saved it!\n";
        OT_FAIL;
    }

    // LOG LEVEL
    {
        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("logging", "log_level", 0, lValue, bIsNewKey);
        OTLog::SetLogLevel(static_cast<int32_t>(lValue));
    }

    // WALLET

    // WALLET FILENAME
    //
    // Clean and Set
    {
        bool bIsNewKey;
        OTString strValue;
        p_Config->CheckSet_str("wallet", "wallet_filename",
                               CLIENT_WALLET_FILENAME, strValue, bIsNewKey);
        OT_API::SetWalletFilename(strValue);
        otWarn << "Using Wallet: " << strValue << "\n";
    }

    // LATENCY
    {
        const char* szComment =
            ";; LATENCY:\n\n"
            ";; For sending and receiving:\n"
            ";; blocking=true (usually not recommended) means OT will hang on "
            "the send/receive\n"
            ";; call, and wait indefinitely until the send or receive has "
            "actually occurred.\n"
            ";; IF BLOCKING IS FALSE (normal, default):\n"
            ";; - no_tries is the number of times OT will try to send or "
            "receive a message.\n"
            ";; - ms is the number of milliseconds it will wait between each "
            "attempt.\n"
            ";; UPDATE: send_ms and receive_ms now DOUBLE after each failed "
            "attempt! (up to 3 tries)\n"
            ";; Meaning that after 3 tries, it's already waited over 21 "
            "seconds trying to get\n"
            ";; the message. \n"
            ";; send_delay_after happens after EVERY SINGLE server "
            "request/reply, which can be\n"
            ";; multiple times per use case. (They can add up quick...)\n";

        bool b_SectionExist;
        p_Config->CheckSetSection("latency", szComment, b_SectionExist);
    }

    {
        if (nullptr == m_pSocket) {
            OT_FAIL;
        }

        const OTSocket::Defaults socketDefaults(
            CLIENT_DEFAULT_LATENCY_SEND_MS,
            CLIENT_DEFAULT_LATENCY_SEND_NO_TRIES,
            CLIENT_DEFAULT_LATENCY_RECEIVE_MS,
            CLIENT_DEFAULT_LATENCY_RECEIVE_NO_TRIES,
            CLIENT_DEFAULT_LATENCY_DELAY_AFTER, CLIENT_DEFAULT_IS_BLOCKING);

        m_pSocket->Init(socketDefaults, p_Config); // setup the socket.
    }

    // SECURITY (beginnings of..)

    // Master Key Timeout
    {
        const char* szComment =
            "; master_key_timeout is how int64_t the master key will be in "
            "memory until a thread wipes it out.\n"
            "; 0   : means you have to type your password EVERY time OT uses a "
            "private key. (Even multiple times in a single function.)\n"
            "; 300 : means you only have to type it once per 5 minutes.\n"
            "; -1  : means you only type it once PER RUN (popular for "
            "servers.)\n";

        bool bIsNewKey;
        int64_t lValue;
        p_Config->CheckSet_long("security", "master_key_timeout",
                                CLIENT_MASTER_KEY_TIMEOUT_DEFAULT, lValue,
                                bIsNewKey, szComment);
        OTCachedKey::It()->SetTimeoutSeconds(static_cast<int32_t>(lValue));
    }

    // Use System Keyring
    {
        bool bValue, bIsNewKey;
        p_Config->CheckSet_bool("security", "use_system_keyring",
                                CLIENT_USE_SYSTEM_KEYRING, bValue, bIsNewKey);
        OTCachedKey::It()->UseSystemKeyring(bValue);
    }

    // Use System Keyring
    {
        bool bValue, bIsNewKey;
        p_Config->CheckSet_bool("security", "use_system_keyring",
                                CLIENT_USE_SYSTEM_KEYRING, bValue, bIsNewKey);
        OTCachedKey::It()->UseSystemKeyring(bValue);

#if defined(OT_KEYRING_FLATFILE)
        // Is there a password folder? (There shouldn't be, but we allow it...)
        //
        if (bValue) {
            bool bIsNewKey2;
            OTString strValue;
            p_Config->CheckSet_str("security", "password_folder",
                                   CLIENT_PASSWORD_FOLDER, strValue,
                                   bIsNewKey2);
            if (strValue.Exists()) {
                OTKeyring::FlatFile_SetPasswordFolder(strValue.Get());
                otOut << " **DANGEROUS!**  Using password folder: " << strValue
                      << "\n";
            }
        }
#endif
    }

    // Done Loading... Lets save any changes...
    if (!p_Config->Save()) {
        otErr << __FUNCTION__ << ": Error! Unable to save updated Config!!!\n";
        OT_FAIL;
    }

    // Finsihed Saving... now lets cleanup!
    if (!p_Config->Reset()) return false;

    if (nullptr != p_Config) delete p_Config;
    p_Config = nullptr;

    return true;
}

bool OT_API::SetWallet(const OTString& strFilename)
{

    if (!m_bInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        OT_FAIL;
    }

    {
        bool bExists = strFilename.Exists();
        if (bExists) {
            otErr << __FUNCTION__ << ": strFilename dose not exist!\n";
            OT_FAIL;
        }
    }

    OT_ASSERT_MSG(strFilename.Exists(),
                  "OT_API::SetWalletFilename: strFilename does not exist.\n");
    OT_ASSERT_MSG((3 < strFilename.GetLength()),
                  "OT_API::SetWalletFilename: strFilename is too short.\n");

    // Set New Wallet Filename
    otOut << __FUNCTION__ << ": Setting Wallet Filename... \n";
    OTString strWalletFilename;
    OT_API::GetWalletFilename(strWalletFilename);

    if (strFilename.Compare(strWalletFilename)) {
        otWarn << __FUNCTION__ << ": Wallet Filename: " << strFilename
               << "  is same as in configuration. (skipping)\n";
        return true;
    }
    else
        strWalletFilename.Set(strWalletFilename);

    // Will save updated config filename.

    // Create Config Object (OTSettings)
    OTString strConfigFilePath;
    OTDataFolder::GetConfigFilePath(strConfigFilePath);
    OTSettings* p_Config(new OTSettings(strConfigFilePath));

    // First Load, Create new fresh config file if failed loading.
    if (!p_Config->Load()) {
        otOut << __FUNCTION__
              << ": Note: Unable to Load Config. Creating a new file: "
              << strConfigFilePath << "\n";
        if (!p_Config->Reset()) return false;
        if (!p_Config->Save()) return false;
    }

    if (!p_Config->Reset()) return false;

    // Second Load, Throw Assert if Failed loading.
    if (!p_Config->Load()) {
        otErr << __FUNCTION__
              << ": Error: Unable to load config file: " << strConfigFilePath
              << " It should exist, as we just saved it!\n";
        OT_FAIL;
    }

    // Set New Wallet Filename
    {
        bool bNewOrUpdated;
        p_Config->Set_str("wallet", "wallet_filename", strWalletFilename,
                          bNewOrUpdated, "; Wallet updated\n");

        OT_API::SetWalletFilename(strWalletFilename);
    }

    // Done Loading... Lets save any changes...
    if (!p_Config->Save()) {
        otErr << __FUNCTION__ << ": Error! Unable to save updated Config!!!\n";
        OT_FAIL;
    }

    // Finsihed Saving... now lets cleanup!
    if (!p_Config->Reset()) return false;

    otOut << __FUNCTION__ << ": Updated Wallet filename: " << strWalletFilename
          << " \n";

    return true;
}

bool OT_API::WalletExists() const
{
    return (nullptr != m_pWallet) ? true : false;
}

bool OT_API::LoadWallet() const
{
    OT_ASSERT_MSG(m_bInitialized,
                  "Not initialized; call OT_API::Init first.\n");
    OT_ASSERT_MSG(
        m_bDefaultStore,
        "Default Storage not Initialized; call OT_API::Init first.\n");

    OTString strWalletFilename;
    bool bGetWalletFilenameSuccess =
        OT_API::GetWalletFilename(strWalletFilename);

    OT_ASSERT_MSG(
        bGetWalletFilenameSuccess,
        "OT_API::GetWalletFilename failed, wallet filename isn't set!");

    // Atempt Load
    otInfo << "m_pWallet->LoadWallet() with: " << strWalletFilename << "\n";
    bool bSuccess = m_pWallet->LoadWallet(strWalletFilename.Get());

    if (bSuccess)
        otInfo << __FUNCTION__
               << ": Success invoking m_pWallet->LoadWallet() with filename: "
               << strWalletFilename << "\n";
    else
        otErr << __FUNCTION__
              << ": Failed invoking m_pWallet->LoadWallet() with filename: "
              << strWalletFilename << "\n";
    return bSuccess;
}

// When the server and client (this API being a client) are built in XmlRpc/HTTP
// mode, then a callback must be provided for passing the messages back and
// forth
// with the server. (Provided below.)
//
// IMPORTANT: If you ever wanted to use a different transport mechanism, it
// would
// be as easy as adding your own version of this callback function, but having
// it
// use your own transport mechanism instead of the xmlrpc in this example.
// Of course, the server would also have to support this transport layer...

// The Callback so OT can give us messages to send using our xmlrpc transport.
// Whenever OT needs to pop a message on over to the server, it calls this so we
// can do the work here.
//
// typedef bool (*OT_CALLBACK_MSG)(OTData & thePayload);
//

bool OT_API::TransportFunction(const OTServerContract& theServerContract,
                               const OTEnvelope& theEnvelope) const
{
    if (!IsInitialized()) {
        otErr << __FUNCTION__ << ": Error: OT_API is not Initialized!\n";
        OT_FAIL;
    }
    if (nullptr == m_pClient) {
        otErr << __FUNCTION__ << ": Error: m_pClient is a nullptr!\n";
        OT_FAIL;
    }
    if (nullptr == m_pClient->m_pConnection) {
        otErr << __FUNCTION__ << ": Error: m_pConnection is a nullptr!\n";
        OT_FAIL;
    }
    OTPseudonym* pNym(m_pClient->m_pConnection->GetNym());
    if (nullptr == pNym) {
        otErr << __FUNCTION__ << ": Error: pNym is a nullptr!\n";
        OT_FAIL;
    }
    if (nullptr == m_pSocket) {
        otErr << __FUNCTION__ << ": Error: m_Socket is a nullptr!\n";
        OT_FAIL;
    }
    if (nullptr == m_pSocket->GetMutex()) {
        otErr << __FUNCTION__ << ": Error: m_Socket is a nullptr!\n";
        OT_FAIL;
    }
    if (!m_pSocket->IsInitialized()) {
        otErr << __FUNCTION__ << ": Error: m_Socket is not Initialized!\n";
        OT_FAIL;
    }
    std::lock_guard<std::mutex> lock(*m_pSocket->GetMutex());
    int32_t nServerPort = 0;
    OTString strServerHostname;

    if (false ==
        theServerContract.GetConnectInfo(strServerHostname, nServerPort)) {
        otErr << __FUNCTION__
              << ": Failed retrieving connection info from server contract.\n";
        return false;
    }
    OTString strConnectPath;
    strConnectPath.Format("tcp://%s:%d", strServerHostname.Get(), nServerPort);

    OTASCIIArmor ascEnvelope(theEnvelope);

    if (ascEnvelope.Exists()) {
        if (!m_pSocket->HasContext())
            if (!m_pSocket->NewContext())
                return false; // unable to make context. btw. should have been
                              // already made.

        bool bSuccessSending =
            m_pSocket->Send(ascEnvelope, strConnectPath); // <========

        if (!bSuccessSending) {
            otErr << __FUNCTION__
                  << ": Failed, even with error correction and retries, "
                     "while trying to send message to server.";
        }
        else {
            OTString strRawServerReply;
            bool bSuccessReceiving =
                m_pSocket->Receive(strRawServerReply); // <========

            if (!bSuccessReceiving || !strRawServerReply.Exists()) {
                otErr << __FUNCTION__
                      << ": Failed trying to receive expected reply "
                         "from server.\n";
            }
            else {
                OTASCIIArmor ascServerReply;
                const bool bLoaded =
                    strRawServerReply.Exists() &&
                    ascServerReply.LoadFromString(strRawServerReply);
                OTString strServerReply;
                bool bRetrievedReply = false;
                if (!bLoaded)
                    otErr << __FUNCTION__
                          << ": Failed trying to load OTASCIIArmor "
                             "object from string:\n\n" << strRawServerReply
                          << "\n\n";

                else if (strRawServerReply.Contains(
                             "ENVELOPE")) // Server sent this encrypted to my
                                          // public key, in an armored envelope.
                {
                    OTEnvelope theServerEnvelope;
                    if (theServerEnvelope.SetAsciiArmoredData(ascServerReply)) {

                        bRetrievedReply =
                            theServerEnvelope.Open(*pNym, strServerReply);
                    }
                    else {
                        otErr << __FUNCTION__
                              << ": Failed: while setting "
                                 "OTASCIIArmor'd string into an "
                                 "OTEnvelope.\n";
                    }
                }
                // NOW ABLE TO HANDLE MESSAGES HERE IN ADDITION TO ENVELOPES!!!!
                // (Sometimes the server HAS to reply with an unencrypted reply,
                // and this is what makes it possible for the client to RECEIVE
                // that reply.)
                //
                // The Server doesn't have to accept both types, but the client
                // does,
                // since technically all clients cannot talk to it without
                // knowing its key first.
                //
                // ===> A CLIENT could POTENTIALLY have sent a message to server
                // when unregistered,
                // leaving server NO WAY to reply! Therefore server HAS to have
                // the OPTION to send
                // an unencrypted message, in that case, and the client HAS to
                // be able to receive it
                // properly!!
                //
                else if (strRawServerReply.Contains("MESSAGE")) // Server sent
                                                                // this NOT
                                                                // encrypted, in
                                                                // an armored
                                                                // message.
                {
                    bRetrievedReply = ascServerReply.GetString(strServerReply);
                }
                else {
                    otErr << __FUNCTION__
                          << ": Error: Unknown reply type received from "
                             "server. (Expected envelope or message.)\n"
                             "\n\n PERHAPS YOU ARE RUNNING AN OLD VERSION "
                             "OF THE SERVER ????? \n\n";
                }
                OTMessage* pServerReply(new OTMessage());
                OT_ASSERT(nullptr != pServerReply);

                if (bRetrievedReply && strServerReply.Exists() &&
                    pServerReply->LoadContractFromString(strServerReply)) {
                    // Now the fully-loaded message object (from the server,
                    // this time) can be processed by the OT library...
                    m_pClient->ProcessServerReply(
                        *pServerReply); // Client takes ownership and will
                                        // handle cleanup.
                }
                else {
                    if (nullptr != pServerReply) delete pServerReply;
                    pServerReply = nullptr; // cleanup

                    otErr << __FUNCTION__
                          << ": Error loading server reply from string:\n\n"
                          << strRawServerReply << "\n\n";
                }
            } // !success receiving.
        }     // else (bSuccessSending)
    }         // if envelope exists.
    return true;
}

int32_t OT_API::GetNymCount() const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr != pWallet) return pWallet->GetNymCount();

    return 0;
}

int32_t OT_API::GetServerCount() const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr != pWallet) return pWallet->GetServerCount();

    return 0;
}

int32_t OT_API::GetAssetTypeCount() const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr != pWallet) return pWallet->GetAssetTypeCount();

    return 0;
}

int32_t OT_API::GetAccountCount() const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr != pWallet) return pWallet->GetAccountCount();

    return 0;
}

bool OT_API::GetNym(int32_t iIndex, OTIdentifier& NYM_ID,
                    OTString& NYM_NAME) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr != pWallet) return pWallet->GetNym(iIndex, NYM_ID, NYM_NAME);

    return false;
}

bool OT_API::GetServer(int32_t iIndex, OTIdentifier& THE_ID,
                       OTString& THE_NAME) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr != pWallet) return pWallet->GetServer(iIndex, THE_ID, THE_NAME);

    return false;
}

bool OT_API::GetAssetType(int32_t iIndex, OTIdentifier& THE_ID,
                          OTString& THE_NAME) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr != pWallet)
        return pWallet->GetAssetType(iIndex, THE_ID, THE_NAME);

    return false;
}

bool OT_API::GetAccount(int32_t iIndex, OTIdentifier& THE_ID,
                        OTString& THE_NAME) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr != pWallet)
        return pWallet->GetAccount(iIndex, THE_ID, THE_NAME);

    return false;
}

OTWallet* OT_API::GetWallet(const char* szFuncName) const
{
    // Any function that calls GetWallet() thus asserts here.
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTWallet* pWallet = m_pWallet; // This is where we "get" the wallet.  :P
    if (nullptr == pWallet)
        otOut << szFunc << ": -- The Wallet is not loaded.\n";
    return pWallet;
}

OTPseudonym* OT_API::GetNym(const OTIdentifier& NYM_ID,
                            const char* szFunc) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }

    OTWallet* pWallet = GetWallet(nullptr != szFunc ? szFunc : __FUNCTION__);
    if (nullptr != pWallet) {
        OTPseudonym* pNym = pWallet->GetNymByID(NYM_ID);
        if ((nullptr == pNym) &&
            (nullptr != szFunc)) // We only log if the caller asked us to.
        {
            const OTString strID(NYM_ID);
            otWarn << __FUNCTION__ << " " << szFunc
                   << ": No Nym found in wallet with ID: " << strID << "\n";
        }
        return pNym;
    }
    return nullptr;
}

OTServerContract* OT_API::GetServer(const OTIdentifier& THE_ID,
                                    const char* szFunc) const
{
    OTWallet* pWallet = GetWallet(nullptr != szFunc ? szFunc : __FUNCTION__);
    if (nullptr != pWallet) {
        OTServerContract* pContract = pWallet->GetServerContract(THE_ID);
        if ((nullptr == pContract) &&
            (nullptr != szFunc)) // We only log if the caller asked us to.
        {
            const OTString strID(THE_ID);
            otWarn << __FUNCTION__ << " " << szFunc
                   << ": No server contract found "
                      "in wallet with ID: " << strID << "\n";
        }
        return pContract;
    }
    return nullptr;
}

OTAssetContract* OT_API::GetAssetType(const OTIdentifier& THE_ID,
                                      const char* szFunc) const
{
    OTWallet* pWallet = GetWallet(nullptr != szFunc ? szFunc : __FUNCTION__);
    if (nullptr != pWallet) {
        OTAssetContract* pContract = pWallet->GetAssetContract(THE_ID);
        if ((nullptr == pContract) &&
            (nullptr != szFunc)) // We only log if the caller asked us to.
        {
            const OTString strID(THE_ID);
            otWarn << __FUNCTION__ << " " << szFunc
                   << ": No asset contract "
                      "found in wallet with ID: " << strID << "\n";
        }
        return pContract;
    }
    return nullptr;
}

OTAccount* OT_API::GetAccount(const OTIdentifier& THE_ID,
                              const char* szFunc) const
{
    OTWallet* pWallet = GetWallet(nullptr != szFunc ? szFunc : __FUNCTION__);
    if (nullptr != pWallet) {
        OTAccount* pAcct = pWallet->GetAccount(THE_ID);
        if ((nullptr == pAcct) &&
            (nullptr != szFunc)) // We only log if the caller asked us to.
        {
            const OTString strID(THE_ID);
            otWarn << __FUNCTION__ << " " << szFunc
                   << ": No account found in "
                      "wallet with ID: " << strID << "\n";
        }
        return pAcct;
    }
    return nullptr;
}

OTPseudonym* OT_API::GetNymByIDPartialMatch(const std::string PARTIAL_ID,
                                            const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTWallet* pWallet = GetWallet(szFunc); // This logs and ASSERTs already.
    if (nullptr != pWallet) return pWallet->GetNymByIDPartialMatch(PARTIAL_ID);

    return nullptr;
}

OTServerContract* OT_API::GetServerContractPartialMatch(
    const std::string PARTIAL_ID, const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTWallet* pWallet = GetWallet(szFunc); // This logs and ASSERTs already.
    if (nullptr != pWallet)
        return pWallet->GetServerContractPartialMatch(PARTIAL_ID);

    return nullptr;
}

OTAssetContract* OT_API::GetAssetContractPartialMatch(
    const std::string PARTIAL_ID, const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTWallet* pWallet = GetWallet(szFunc); // This logs and ASSERTs already.
    if (nullptr != pWallet)
        return pWallet->GetAssetContractPartialMatch(PARTIAL_ID);

    return nullptr;
}

OTAccount* OT_API::GetAccountPartialMatch(const std::string PARTIAL_ID,
                                          const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTWallet* pWallet = GetWallet(szFunc); // This logs and ASSERTs already.
    if (nullptr != pWallet) return pWallet->GetAccountPartialMatch(PARTIAL_ID);

    return nullptr;
}

// returns a new nym (with key pair) and files created. (Or nullptr.)
//
// Adds to wallet. (No need to delete.)
//
OTPseudonym* OT_API::CreateNym(int32_t nKeySize,
                               const std::string str_id_source,
                               const std::string str_alt_location) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    switch (nKeySize) {
    case 1024:
    case 2048:
    case 4096:
    case 8192:
        break;
    default:
        otErr << __FUNCTION__ << ": Failure: nKeySize must be one of: "
                                 "1024, 2048, 4096, 8192. (" << nKeySize
              << " was passed...)\n";
        return nullptr;
    }
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return nullptr;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    OTPseudonym* pNym = new OTPseudonym;
    OT_ASSERT(nullptr != pNym);
    if (false ==
        pNym->GenerateNym(nKeySize, true, str_id_source, str_alt_location)) {
        otErr << __FUNCTION__ << ": Failed trying to generate Nym.\n";
        delete pNym;
        pNym = nullptr;
        return nullptr;
    }
    pWallet->AddNym(
        *pNym); // Add our new nym to the wallet, who "owns" it hereafter.

    // Note: It's already on the master key. To prevent that, we would have had
    // to PAUSE the master key before calling GenerateNym above. So the below
    // call
    // is less about the Nym's encryption, and more about the wallet KNOWING.
    // Because
    // OTWallet::ConvertNymToCachedKey is what adds this nym to the wallet's
    // list of
    // "master key nyms". Until that happens, the wallet has no idea.
    //
    if (!pWallet->ConvertNymToCachedKey(*pNym))
        otErr << __FUNCTION__
              << ": Error: Failed in pWallet->ConvertNymToCachedKey \n";
    pWallet->SaveWallet(); // Since it just changed.
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    return pNym;
}

// The Asset Type's Name is basically just a client-side label.
// This function lets you change it.
//
// Returns success, true or false.
//
bool OT_API::SetAssetType_Name(const OTIdentifier& ASSET_ID,
                               const OTString& STR_NEW_NAME) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return false;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    OTAssetContract* pContract =
        GetAssetType(ASSET_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pContract) return false;
    // By this point, pContract is a good pointer.  (No need to cleanup.)
    // Might want to put some more data validation on the name?
    if (!STR_NEW_NAME.Exists())
        otOut << "OT_API::SetAssetType_Name: Bad: name is empty.\n";
    else {
        pContract->SetName(STR_NEW_NAME);
        return pWallet->SaveWallet(); // Only 'cause the name is actually stored
                                      // here.
    }
    return false;
}

// The Server's Name is basically just a client-side label.
// This function lets you change it.
//
// Returns success, true or false.
//
bool OT_API::SetServer_Name(const OTIdentifier& SERVER_ID,
                            const OTString& STR_NEW_NAME) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return false;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    OTServerContract* pContract =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pContract) return false;
    // By this point, pContract is a good pointer.  (No need to cleanup.)
    // Might want to put some more data validation on the name?
    if (!STR_NEW_NAME.Exists())
        otOut << __FUNCTION__ << ": Bad: name is empty.\n";
    else {
        pContract->SetName(STR_NEW_NAME);
        return pWallet->SaveWallet(); // Only 'cause the name is actually stored
                                      // here.
    }
    return false;
}

bool OT_API::IsNym_RegisteredAtServer(const OTIdentifier& NYM_ID,
                                      const OTIdentifier& SERVER_ID) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }
    OTPseudonym* pNym =
        GetNym(NYM_ID, __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pNym) return false;
    // Below this point, pNym is a good ptr, and will be cleaned up
    // automatically.
    const OTString strServerID(SERVER_ID);
    return pNym->IsRegisteredAtServer(strServerID);
}

/*
 CHANGE MASTER KEY and PASSWORD.

 Normally your passphrase is used to derive a key, which is used to unlock
 a random number (a symmetric key), which is used as the passphrase to open the
 master key, which is used as the passphrase to any given Nym.

 Since all the Nyms are encrypted to the master key, and since we can change the
 passphrase on the master key without changing the master key itself, then we
don't
 have to do anything to update all the Nyms, since that part hasn't changed.

 But we might want a separate "Change Master Key" function that replaces that
key
 itself, in which case we'd HAVE to load up all the Nyms and re-save them.

*** UPDATE: Seems the easiest thing to do is to just change both the key and
passphase
 at the same time here, by loading up all the private nyms, destroying the
master key,
 and then saving all the private Nyms. (With master key never actually being
"paused.")
 This will automatically cause it to generate a new master key during the saving
process.
 (Make sure to save the wallet also.)
 */

/*

 // Done:

 Load up separate copies of all the Nyms.
 (Set them to destruct automatically on exit, no matter what.)

 Change them to temp PW.

 Re-generate master.

 Change them to new master.

 Save nyms, save wallet.

 Reload wallet.

 IF anything fails along the way, we just return. What happens? The temp
 Nyms we converted are destroyed, not saved, and were not the actual wallet
 copies anyway (which are still old.) The master key itself is reverted to
 its former form (I stored a copy of that) and the wallet file itself is
 never overwritten.
 Once we successfully convert everything, we save the Nyms and then re-load
 the wallet (and thus the nyms...) Now the updated Nyms are loaded up in the
 wallet, and the temp ones just destruct when we exit the function. Perfect.

 */

// WARNING: This function, if successful, saves and re-loads the wallet.
// Therefore if you have any pointers to things inside the wallet, such as
// a pointer to a Nym, that pointer will be bad after you call this function.
// So make sure you grab a fresh pointer in such situations, after calling this.
// (Because the one you had before will crash you.)
//
bool OT_API::Wallet_ChangePassphrase() const
{
    bool bInitialized = OTAPI_Wrap::OTAPI()->IsInitialized();
    if (!bInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        OT_FAIL;
    }
    OTWallet* pWallet = OTAPI_Wrap::OTAPI()->GetWallet(
        __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return false;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    OTString strReason("Enter existing wallet master passphrase");

    OTPassword old_passphrase;
    std::shared_ptr<OTCachedKey> ptrMasterKey(OTCachedKey::It());
    const bool bGotOldPassphrase =
        (ptrMasterKey && ptrMasterKey->IsGenerated() &&
         ptrMasterKey->GetMasterPassword(ptrMasterKey, old_passphrase,
                                         strReason.Get()));
    class ot_change_pw
    {
        std::list<OTPseudonym*>* m_plist_nyms; // We'll be responsible in this
                                               // class for cleaning these up.

    public:
        ot_change_pw(std::list<OTPseudonym*>& list_nyms)
            : m_plist_nyms(&list_nyms)
        {
        }
        ~ot_change_pw()
        {
            if (nullptr != m_plist_nyms) {
                while (!m_plist_nyms->empty()) // Here's the cleanup.
                {
                    auto it = m_plist_nyms->begin();
                    OTPseudonym* pNym = *it;
                    OT_ASSERT(nullptr != pNym);

                    delete pNym;
                    pNym = nullptr;
                    m_plist_nyms->erase(it);
                }
            }
        }
    };
    std::list<OTPseudonym*> list_nyms; // Any Nyms on this list will be cleaned
                                       // up automatically in the ot_change_pw
                                       // destructor. Thus
    ot_change_pw the_cleanup(list_nyms); // we cannot add Nyms here that are
                                         // also on the Wallet. We load our own
                                         // copies just for this list.
    bool bAtLeastOneNymHasCredentials = false;
    bool bSuccessLoading =
        true; // defaults to true in case there aren't any Nyms.

    // Loop through all the private Nyms and get them all loaded up into a list.
    //
    const int32_t nNymCount = pWallet->GetNymCount();
    for (int32_t iii = 0; iii < nNymCount; ++iii) {
        OTIdentifier NYM_ID;
        OTString NYM_NAME;

        const bool bGotNym = pWallet->GetNym(iii, NYM_ID, NYM_NAME);
        OT_ASSERT(bGotNym);
        const OTString strNymID(NYM_ID);

        // otherwise it's a public Nym, so we just skip it.
        if (OTPseudonym::DoesCertfileExist(strNymID)) // is there a private key
                                                      // or credential available
                                                      // for this Nym?
        { // In here, we know there's a private key...

            // CALLER responsible to delete!
            OTPseudonym* pNym = LoadPrivateNym(
                NYM_ID, false /*bChecking*/,
                __FUNCTION__); // This also loads credentials, if there are any.

            // We use LoadPrivateNym here instead. (meaning: need to clean them
            // up.)
            // Therefore use a nested class here to handle the cleanup.
            // This way we aren't changing the actual Nyms in the wallet until
            // it's re-loaded.
            // Any failure between now and then, and it's not re-loaded, and
            // none of the Nyms
            // were ever changed!

            if (nullptr == pNym) // Since we KNOW there's a private key, yet it
                                 // failed, therefore the user must have entered
                                 // the wrong password...
            {
                bSuccessLoading = false;
                break;
            }
            // else... (add to list for cleanup, on exit from this function.)
            list_nyms.push_back(pNym); // ONLY private Nyms, and they ALL must
                                       // successfully load and verify.

            if (pNym->GetMasterCredentialCount() > 0)
                bAtLeastOneNymHasCredentials = true;
        }
    }
    if (!bSuccessLoading) {
        otErr << __FUNCTION__
              << ": Error: Failed to load all the private Nyms. Wrong "
                 "passphrase? (Aborting operation.)\n";
        return false;
    }
    // By this point we KNOW we have successfully loaded up ALL the private Nyms
    // for this wallet, and that list_nyms contains a pointer to each one...
    // ENCRYPT ALL CREDENTIALS FROM MASTER KEY INTO A TEMP KEY
    //
    OTPassword theTempPassword; // Used to store a temp password only. Only for
                                // credentialed nyms.
    OTPasswordData thePWData("Enter existing wallet master passphrase");

    // At this point, for Nyms with credentials, we need to ReEncrypt the Nym's
    // credentials,
    // similar to importing or exporting the Nym from the wallet. Except this
    // time, we have
    // to ReEncrypt FROM a wallet-based Nym to a temporary passphrase, then
    // destroy and re-
    // create the wallet's cached master key, then ReEncrypt AGAIN from the
    // temporary passphrase
    // and back to the new master passphrase that was just generated.
    // Makes sense? It's the easiest way to do it based on the existing code we
    // have.
    //
    if (bAtLeastOneNymHasCredentials) // All the Nyms on our list are private,
                                      // by this point. And within this block,
                                      // they have credentials, too.
    {
        theTempPassword.randomizePassword(12); // the new random PW will be 12
                                               // bytes int64_t. (We discard it
                                               // after this function is done.)
        bool bSuccessReEncrypting = true;
        for (auto& it : list_nyms) {
            OTPseudonym* pNym = it;
            OT_ASSERT(nullptr != pNym);
            // We know at least one Nym has credentials. Does this one?
            //
            if (pNym->GetMasterCredentialCount() >
                0) // If this specific Nym has credentials...
            {
                // It won't ask for a passphrase when using the temp password,
                // since it already
                // has the password. So it will only ask for a passphrase when
                // it is NOT using
                // the temp password, e.g. when using the wallet's cached master
                // key. ABOVE (here)
                // that will be the OLD master key, and BELOW (after the master
                // key is re-created)
                // that will be the NEW master key.
                // That's why here, it says: "Enter your EXISTING wallet master
                // passphrase."
                //
                const bool bExported = pNym->ReEncryptPrivateCredentials(
                    false /* (EXPORTING) bImporting=FALSE */, &thePWData,
                    &theTempPassword);
                if (!bExported) {
                    bSuccessReEncrypting = false; // At least this way if
                                                  // there's a failure, it's
                                                  // equally to all the Nyms,
                                                  // and not halfway through.
                }
            }
        }
        if (!bSuccessReEncrypting) // FAILURE
        {
            otErr
                << __FUNCTION__
                << ": Failed trying to re-encrypt Nym's private credentials.\n";
            return false; // Nyms are cleaned up automatically when we return.
        }
    }
    // By this point, if there were credentials on any of the Nyms, they are all
    // now converted
    // (in RAM, not in local storage) to the temp password, and OUT of the
    // wallet's master key
    // (which we're about to destroy and re-create.)
    // Destroy the wallet's cached master key (in Ram, not on disk--yet.)
    //
    OTASCIIArmor ascBackup;
    OTCachedKey::It()->SerializeTo(ascBackup); // Just in case!
    OTCachedKey::It()->ResetMasterPassword();  // Which will force it to be
                                               // re-created next time someone
                                               // tries to use it...

    // NOTE: Below this point we cannot return without setting the master
    // passphrase BACK.
    // GENERATE the wallet's NEW MASTER KEY.
    //
    strReason.Set("Choose a new passphrase: ");

    // This step would be unnecessary if we knew for a fact that at least
    // one Nym exists. But in the off-chance that there ARE NO NYMS in the
    // wallet, we need to have this here, in order to MAKE SURE that the new
    // master key is generated. Otherwise it would never end up actually having
    // to generate the thing. (Since, if there are no Nyms to re-save, it would
    // never need to actually retrieve the master key, which is what triggers it
    // to generate if it's not already there.) So we just force that step here,
    // to make sure it happens, even if there are no Nyms to save below this
    // point.
    //
    OTPassword new_passphrase;
    std::shared_ptr<OTCachedKey> sharedPtr(OTCachedKey::It());
    const bool bRegenerate =
        sharedPtr->GetMasterPassword(sharedPtr, new_passphrase, strReason.Get(),
                                     true); // bVerifyTwice=false by default.
    if (!bRegenerate) // Failure generating new master key.
    {
        otErr << __FUNCTION__ << ": Error: Failed while trying to regenerate "
                                 "master key, in call: "
                                 "OTCachedKey::It()->GetMasterPassword. "
                                 "(Setting it back to the old "
                                 "one.)\n";
        if (!OTCachedKey::It()->SerializeFrom(ascBackup))
            otErr << __FUNCTION__
                  << ": Error: Failed while trying to restore master "
                     "key, in call: "
                     "OTCachedKey::It()->GetMasterPassword. (While "
                     "setting it back to the old one.)\n"
                     "Original value: \n" << ascBackup << "\n";
        // todo security: is
        // it risky to have
        // the key displayed
        // in this log?
        return false;
        //
        // NOTE: Since we loaded our own copies of the Nyms, we haven't changed
        // the copies in the wallet,
        // nor the wallet itself. So we can just return, since our copies of the
        // Nyms will already be cleaned
        // up automatically.
    }
    else // SUCCESS creating new master key, so let's CONVERT AND RE-SAVE ALL
    {      // THE NYMS, so they'll be using it from now on...

        // (Master key would normally be generated here, if we hadn't already
        // forced it above,
        // but we did that to make sure it got re-created in the event there are
        // zero nyms.)

        // Todo: save them to temp files and only copy over if everything
        // else is successful. Same with wallet. Also make backups.
        //
        bool bSuccessResaving =
            true; // in case the list is empty, we assume success here.

        // Let's save all these Nyms under the new master key.
        for (auto& it : list_nyms) {
            OTPseudonym* pNym = it;
            OT_ASSERT(nullptr != pNym);
            bool bSaved = false;

            // CREDENTIALS - Nym has credentials
            if (pNym->GetMasterCredentialCount() > 0) {
                // We had converted the Nyms to a temp key above, so now we need
                // to convert
                // from the temp key to the new wallet key. Then we can save
                // them and re-load
                // the wallet.
                //
                // We're supplying the temp password (from above), and importing
                // the Nyms back
                // from that, back into to the wallet's new master key. So it
                // will only ask
                // for a passphrase for the wallet, since the other passphrase
                // is already provided.
                // Therefore thePWData is relevant to the wallet only.
                //
                bool bSavedCredentials = false;
                const bool bImported = pNym->ReEncryptPrivateCredentials(
                    true /*bImporting*/, // <==== CONVERT FROM TEMP PW TO NEW
                                         // MASTER KEY.
                    &thePWData, &theTempPassword);
                if (bImported) // Success? Okay, let's Save those credentials we
                               // just imported, to local storage.
                {
                    bSavedCredentials = true;
                    OTString strNymID, strCredList, strOutput;
                    OTString::Map mapCredFiles;

                    pNym->GetIdentifier(strNymID);
                    pNym->GetPrivateCredentials(strCredList, &mapCredFiles);
                    OTString strFilename;
                    strFilename.Format("%s.cred", strNymID.Get());
                    OTASCIIArmor ascArmor(strCredList);
                    if (ascArmor.Exists() &&
                        ascArmor.WriteArmoredString(
                            strOutput,
                            "CREDENTIAL LIST") && // bEscaped=false by default.
                        strOutput.Exists()) {
                        if (!OTDB::StorePlainString(
                                strOutput.Get(), OTFolders::Credential().Get(),
                                strFilename.Get())) {
                            otErr << __FUNCTION__
                                  << ": After converting credentials to "
                                     "new master key, failure trying to "
                                     "store private "
                                     "credential list for Nym: " << strNymID
                                  << "\n";
                            bSavedCredentials = false;
                        }
                    }
                    // Here we do the actual credentials.
                    for (auto& itCred : mapCredFiles) {
                        const std::string& str_cred_id = itCred.first;
                        OTString strCredential(itCred.second);
                        strOutput.Release();
                        OTASCIIArmor ascLoopArmor(strCredential);
                        if (ascLoopArmor.Exists() &&
                            ascLoopArmor.WriteArmoredString(
                                strOutput,
                                "CREDENTIAL") && // bEscaped=false by default.
                            strOutput.Exists()) {
                            if (!OTDB::StorePlainString(
                                    strOutput.Get(),
                                    OTFolders::Credential().Get(),
                                    strNymID.Get(), str_cred_id)) {
                                otErr << __FUNCTION__
                                      << ": After converting "
                                         "credentials to new master key, "
                                         "failure trying to store private "
                                         "credential for Nym: " << strNymID
                                      << "\n";
                                bSavedCredentials = false;
                            }
                        }
                    }
                }
                bSaved = bImported && bSavedCredentials;
            }    // If Nym has credentials. (Convert and save them, in the above
                 // block.)
            else // Old-school: the old nyms (from before credentials code)
                 // merely need to have their cert saved (here), which process
                 // will automatically use the current (new) master key to
                 // encrypt the private portion of that cert when saving. So it
                 // gets converted and saved all in this one call. For new-style
                 // Nyms (with credentials) see the above block instead.
                //
                bSaved = pNym->Savex509CertAndPrivateKey(true, &strReason);
            if (!bSaved) bSuccessResaving = false;
        }
        if (!bSuccessResaving) // Failed saving all the Nyms after switching
                               // their credentials over.
        {
            OTASCIIArmor ascBackup2;
            OTCachedKey::It()->SerializeTo(ascBackup2); // Just in case!
            otErr << __FUNCTION__
                  << ": ERROR: Failed re-saving Nym (into new Master "
                     "Key.) It's possible "
                     "some Nyms are already saved on the new key, while "
                     "others are still stuck "
                     "on the old key!! Therefore, asserting now. OLD KEY "
                     "was:\n" << ascBackup << "\n\n NEW KEY is: " << ascBackup2
                  << "\n";
            // Todo: security: keys are exposed
            // here. Is this log safe?
            OT_FAIL_MSG("ASSERT while trying to change wallet's master key and "
                        "passphrase.\n");
        }
        else // SAVE THE WALLET.
        {
            // We've converted all the Nyms, so let's go ahead and convert the
            // extra
            // symmetric keys inside the wallet. (These are what a client app
            // might
            // use to encrypt its local database.)
            //
            if (bGotOldPassphrase) {
                if (!pWallet->ChangePassphrasesOnExtraKeys(old_passphrase,
                                                           new_passphrase))
                    otErr << __FUNCTION__
                          << ": ERROR: "
                             "pWallet->ChangePassphrasesOnExtraKeys "
                             "failed. "
                             "(Continuing, but your extra symmetric keys "
                             "in the wallet "
                             "may be messed up!)\n";
            }
            // By this point, we have successfully converted all the Nyms (our
            // local copies of the wallet's private nyms)
            // to the new master key, AND we have successfully saved those Nyms
            // to local storage. Now, if we just save and
            // re-load the wallet itself, it should load up using the new master
            // key, and it should load up its own copies
            // of those same Nyms again, using that new master key to decrypt
            // them. (Those new copies of those Nyms being
            // the ones that we saved to local storage just above, using our
            // local copies.)
            //
            // With the wallet updated thus, we can simply discard the local
            // copies, which will have outlived their usefulness.
            // They will already be destroyed on exit, automatically.
            //
            bool bLoaded = false;
            const bool bSaved = pWallet->SaveWallet();
            if (bSaved) // Next, re-load it so the Nyms we've changed will be
                        // loaded up in their new forms. (The nyms local to this
                        // function will be destroyed on exit, but they are
                        // separate from the nyms in the wallet, which will
                        // appear in their new forms upon loading, presuming the
                        // Nyms were successfully saved above.)
                //
                bLoaded = pWallet->LoadWallet();
            else
                otErr << __FUNCTION__ << ": Failed saving wallet \n";
            if (!bLoaded) {
                OTASCIIArmor ascBackup2;
                OTCachedKey::It()->SerializeTo(ascBackup2); // Just in case!
                // Note: if we even got this far, that means we already saved
                // the Nyms under the new master Key,
                // to local storage. Therefore we NEED that new key, if it
                // wasn't properly saved in the wallet file
                // just now! (And we need to have made a real backup of the
                // wallet before attempting this...todo.)
                // In the meantime, the best thing we can do is just LOG that
                // key here, and hope the server operator
                // still has the log! Log both keys (new and old.)
                otErr
                    << __FUNCTION__
                    << ": ERROR: Failed saving or re-loading Wallet (with new "
                       "Master Key.) "
                       "Asserting now. OLD KEY was:\n" << ascBackup
                    << "\n\n NEW KEY is: " << ascBackup2 << "\n";
                // Todo: security: keys are exposed here.
                // Is this log safe?
                OT_FAIL_MSG("ASSERT while trying to save and re-load wallet "
                            "with new master key and passphrase.\n");
            }
            else
                otOut << "\nSuccess changing master passphrase for wallet!\n";
            return bLoaded;
        }
    }
    return false;
}

bool OT_API::Wallet_CanRemoveServer(const OTIdentifier& SERVER_ID) const
{
    bool bInitialized = OTAPI_Wrap::OTAPI()->IsInitialized();
    if (!bInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        OT_FAIL;
    }
    if (SERVER_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: SERVER_ID passed in!\n";
        OT_FAIL;
    }
    OTString strName;
    const int32_t nCount = OTAPI_Wrap::OTAPI()->GetAccountCount();

    // Loop through all the accounts.
    for (int32_t i = 0; i < nCount; i++) {
        OTIdentifier accountID;

        OTAPI_Wrap::OTAPI()->GetAccount(i, accountID, strName);
        OTAccount* pAccount =
            OTAPI_Wrap::OTAPI()->GetAccount(accountID, __FUNCTION__);

        OTIdentifier purportedServerID(pAccount->GetPurportedServerID());

        if (SERVER_ID == purportedServerID) {
            OTString strPurportedServerID(purportedServerID),
                strSERVER_ID(SERVER_ID);
            otOut << __FUNCTION__ << ": Unable to remove server contract "
                  << strSERVER_ID << " from wallet, because Account "
                  << strPurportedServerID << " uses it.\n";
            return false;
        }
    }

    const int32_t nNymCount = OTAPI_Wrap::OTAPI()->GetNymCount();

    // Loop through all the Nyms. (One might be registered on that server.)
    //
    for (int32_t i = 0; i < nNymCount; i++) {
        OTIdentifier nymID;
        bool bGetNym = OTAPI_Wrap::OTAPI()->GetNym(i, nymID, strName);

        if (bGetNym)
            if (OTAPI_Wrap::OTAPI()->IsNym_RegisteredAtServer(nymID,
                                                              SERVER_ID)) {
                OTString strNymID(nymID), strSERVER_ID(SERVER_ID);
                otOut << __FUNCTION__ << ": Unable to remove server contract "
                      << strSERVER_ID << " "
                                         "from wallet, because Nym " << strNymID
                      << " is registered "
                         "there. (Delete that first...)\n";
                return false;
            }
    }
    return true;
}

// Can I remove this asset contract from my wallet?
//
// You cannot remove the asset contract from your wallet if there are accounts
// in there using it.
// This function tells you whether you can remove the asset contract or
// not.(Whether there are accounts...)
//
bool OT_API::Wallet_CanRemoveAssetType(const OTIdentifier& ASSET_ID) const
{
    bool bInitialized = OTAPI_Wrap::OTAPI()->IsInitialized();
    if (!bInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        OT_FAIL;
    }

    if (ASSET_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ASSET_ID passed in!\n";
        OT_FAIL;
    }
    OTString strName;
    const int32_t nCount = OTAPI_Wrap::OTAPI()->GetAccountCount();

    // Loop through all the accounts.
    for (int32_t i = 0; i < nCount; i++) {
        OTIdentifier accountID;

        OTAPI_Wrap::OTAPI()->GetAccount(i, accountID, strName);
        OTAccount* pAccount =
            OTAPI_Wrap::OTAPI()->GetAccount(accountID, __FUNCTION__);
        OTIdentifier theTYPE_ID(pAccount->GetAssetTypeID());

        if (ASSET_ID == theTYPE_ID) {
            OTString strASSET_ID(ASSET_ID), strTYPE_ID(theTYPE_ID);

            otOut << __FUNCTION__ << ": Unable to remove asset contract "
                  << strASSET_ID << " from wallet: Account " << strTYPE_ID
                  << " uses it.\n";
            return false;
        }
    }
    return true;
}

// Can I remove this Nym from my wallet?
//
// You cannot remove the Nym from your wallet if there are accounts in there
// using it.
// This function tells you whether you can remove the Nym or not. (Whether there
// are accounts...)
// It also checks to see if the Nym in question is registered at any servers.
//
// returns OT_BOOL
//
bool OT_API::Wallet_CanRemoveNym(const OTIdentifier& NYM_ID) const
{
    bool bInitialized = OTAPI_Wrap::OTAPI()->IsInitialized();
    if (!bInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        OT_FAIL;
    }

    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        OT_FAIL;
    }

    OTPseudonym* pNym = OTAPI_Wrap::OTAPI()->GetNym(NYM_ID, __FUNCTION__);
    if (nullptr == pNym) return false;
    // Make sure the Nym doesn't have any accounts in the wallet.
    // (Client must close those before calling this.)
    //
    const int32_t nCount = OTAPI_Wrap::OTAPI()->GetAccountCount();

    // Loop through all the accounts.
    for (int32_t i = 0; i < nCount; i++) {
        OTIdentifier accountID;
        OTString strName;

        OTAPI_Wrap::OTAPI()->GetAccount(i, accountID, strName);
        OTAccount* pAccount =
            OTAPI_Wrap::OTAPI()->GetAccount(accountID, __FUNCTION__);
        OTIdentifier theNYM_ID(pAccount->GetUserID());

        if (theNYM_ID.empty()) {
            otErr << __FUNCTION__ << ": Bug in OT_API_Wallet_CanRemoveNym / "
                                     "OT_API_GetAccountWallet_NymID\n";
            return false;
        }

        // Looks like the Nym still has some accounts in this wallet.
        if (NYM_ID == theNYM_ID) {
            otOut << __FUNCTION__
                  << ": Nym cannot be removed because there are "
                     "still accounts in the wallet for that Nym.\n";
            return false;
        }
    }

    // Make sure the Nym isn't registered at any servers...
    // (Client must unregister at those servers before calling this function..)
    //
    const int32_t nServerCount = OTAPI_Wrap::OTAPI()->GetServerCount();

    for (int32_t i = 0; i < nServerCount; i++) {
        OTIdentifier theID;
        OTString strName;
        bool bGetServer = OTAPI_Wrap::OTAPI()->GetServer(i, theID, strName);

        if (bGetServer)
            if (!theID.empty()) {
                const OTString strServerID(theID);

                if (pNym->IsRegisteredAtServer(strServerID)) {
                    otOut << __FUNCTION__
                          << ": Nym cannot be removed because there "
                             "are still servers in the wallet that "
                             "the Nym is registered at.\n";
                    return false;
                }
            }
    }

    // TODO:  Make sure Nym doesn't have any cash in any purses...

    return true;
}

// Can I remove this Account from my wallet?
//
// You cannot remove the Account from your wallet if there are transactions
// still open.
// This function tells you whether you can remove the Account or not. (Whether
// there are transactions...)
// Also, balance must be zero to do this.
//
// returns OT_BOOL
//
bool OT_API::Wallet_CanRemoveAccount(const OTIdentifier& ACCOUNT_ID) const
{
    bool bInitialized = OTAPI_Wrap::OTAPI()->IsInitialized();
    if (!bInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        OT_FAIL;
    }

    if (ACCOUNT_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        OT_FAIL;
    }

    const OTString strAccountID(ACCOUNT_ID);

    OTAccount* pAccount =
        OTAPI_Wrap::OTAPI()->GetAccount(ACCOUNT_ID, __FUNCTION__);
    if (nullptr == pAccount) return false;
    // Balance must be zero in order to close an account!
    else if (pAccount->GetBalance() != 0) {
        otOut << __FUNCTION__
              << ": Account balance MUST be zero in order to close "
                 "an asset account: " << strAccountID << ".\n";
        return false;
    }
    bool BOOL_RETURN_VALUE = false;

    const OTIdentifier& theServerID = pAccount->GetPurportedServerID();
    const OTIdentifier& theUserID = pAccount->GetUserID();

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return nullptr if various verification fails.
    std::unique_ptr<OTLedger> pInbox(
        OTAPI_Wrap::OTAPI()->LoadInbox(theServerID, theUserID, ACCOUNT_ID));
    std::unique_ptr<OTLedger> pOutbox(
        OTAPI_Wrap::OTAPI()->LoadOutbox(theServerID, theUserID, ACCOUNT_ID));

    if (nullptr == pInbox) {
        otOut << __FUNCTION__
              << ": Failure calling OT_API::LoadInbox.\n Account ID: "
              << strAccountID << "\n";
    }
    else if (nullptr == pOutbox) {
        otOut << __FUNCTION__
              << ": Failure calling OT_API::LoadOutbox.\n Account ID: "
              << strAccountID << "\n";
    }
    else if ((pInbox->GetTransactionCount() > 0) ||
               (pOutbox->GetTransactionCount() > 0)) {
        otOut << __FUNCTION__
              << ": Failure: You cannot remove an asset account if "
                 "there are inbox/outbox items still waiting to be "
                 "processed.\n";
    }
    else
        BOOL_RETURN_VALUE = true; // SUCCESS!

    return BOOL_RETURN_VALUE;
}

// Remove this server contract from my wallet!
//
// Try to remove the server contract from the wallet.
// This will not work if there are any accounts in the wallet for the same
// server ID.
//
bool OT_API::Wallet_RemoveServer(const OTIdentifier& SERVER_ID) const
{
    bool bInitialized = IsInitialized();
    if (!bInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        OT_FAIL;
    }

    if (SERVER_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ASSET_ID passed in!\n";
        OT_FAIL;
    }

    // Make sure there aren't any dependent accounts..
    if (!Wallet_CanRemoveServer(SERVER_ID)) return false;

    // TODO: the above call proves that there are no accounts laying around
    // for this server ID. (No need to worry about "orphaned accounts.")
    //
    // However, there may still be Nyms registered at the server! Therefore,
    // we need to loop through the Nyms, and make sure none of them has been
    // registered at this server ID. If it has, then we need to message the
    // server
    // to "deregister" the Nym, which is much cleaner.  Otherwise server's only
    // other alternative is to expire Nyms that have gone unused for some
    // specific
    // period of time, presumably those terms are described in the server
    // contract.
    //

    if (nullptr == m_pWallet) {
        otErr << __FUNCTION__ << ":  No wallet found...\n";
        OT_FAIL;
    }

    if (m_pWallet->RemoveServerContract(SERVER_ID)) {
        m_pWallet->SaveWallet();
        otOut << __FUNCTION__ << ": Removed server contract from the wallet: "
              << OTString(SERVER_ID) << "\n";
        return true;
    }
    return false;
}

// Remove this asset contract from my wallet!
//
// Try to remove the asset contract from the wallet.
// This will not work if there are any accounts in the wallet for the same asset
// type ID.
//
bool OT_API::Wallet_RemoveAssetType(const OTIdentifier& ASSET_ID) const
{
    bool bInitialized = IsInitialized();
    if (!bInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        OT_FAIL;
    }

    if (ASSET_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ASSET_ID passed in!\n";
        OT_FAIL;
    }

    // Make sure there aren't any dependent accounts..
    if (!Wallet_CanRemoveAssetType(ASSET_ID)) return false;

    if (nullptr == m_pWallet) {
        otErr << __FUNCTION__ << ": No wallet found...!\n";
        OT_FAIL;
    }

    if (m_pWallet->RemoveAssetContract(ASSET_ID)) {
        m_pWallet->SaveWallet();
        otOut << __FUNCTION__ << ": Removed asset contract from the wallet: "
              << OTString(ASSET_ID) << "\n";
        return true;
    }
    return false;
}

// Remove this Nym from my wallet!
//
// Try to remove the Nym from the wallet.
// This will not work if there are any nyms in the wallet for the same server
// ID.
//
bool OT_API::Wallet_RemoveNym(const OTIdentifier& NYM_ID) const
{
    bool bInitialized = IsInitialized();
    if (!bInitialized) {
        otErr << __FUNCTION__
              << ": Not initialized; call OT_API::Init first.\n";
        OT_FAIL;
    }

    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        OT_FAIL;
    }

    // DONE: The below call proves already that there are no accounts laying
    // around
    // for this Nym. (No need to worry about "orphaned accounts.")
    //
    // DONE (finally):
    // However, the Nym might still be registered at various servers, even
    // without asset accounts.
    // Therefore, we need to iterate through the server contracts, and if the
    // Nym is registered at
    // any of the servers, then "deregister" (before deleting the Nym entirely.)
    // This is much
    // cleaner for the server side, who otherwise has to expired unused nyms
    // based on some rule
    // presumably to be found in the server contract.
    if (!Wallet_CanRemoveNym(NYM_ID)) return false;

    if (nullptr == m_pWallet) {
        otErr << __FUNCTION__ << ": No wallet found...!\n";
        OT_FAIL;
    }

    if (m_pWallet->RemoveNym(NYM_ID)) {
        otOut << __FUNCTION__
              << ": Success erasing Nym from wallet: " << OTString(NYM_ID)
              << "\n";
        m_pWallet->SaveWallet();
        return true;
    }
    else
        otOut << __FUNCTION__
              << ": Failure erasing Nym from wallet: " << OTString(NYM_ID)
              << "\n";

    return false;
}

// OT has the capability to export a Nym (normally stored in several files) as
// an encoded
// object (in base64-encoded form) and then import it again.
//
// Returns bool on success, and strOutput will contain the exported data.
//
bool OT_API::Wallet_ExportNym(const OTIdentifier& NYM_ID,
                              OTString& strOutput) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }
    OTPasswordData thePWDataLoad("Enter wallet master passphrase.");
    OTPasswordData thePWDataSave("Create new passphrase for exported Nym.");
    OTString strReasonToSave(thePWDataSave.GetDisplayString());
    OTPseudonym* pNym =
        GetOrLoadPrivateNym(NYM_ID, false, __FUNCTION__,
                            &thePWDataLoad); // This logs and ASSERTs already.
    if (nullptr == pNym) return false;
    std::string str_nym_name(pNym->GetNymName().Get());
    OTString strID;
    pNym->GetIdentifier(strID);
    std::string str_nym_id(strID.Get());
    // Below this point I can use:
    //
    // pNym, str_nym_name, and str_nym_id.
    //
    // I still need the certfile and the nymfile (both in string form.)
    //
    const bool bHasCredentials = (pNym->GetMasterCredentialCount() > 0);

    OTASCIIArmor ascCredentials, ascCredList;
    OTString strCertfile;
    bool bSavedCert = false;
    if (!bHasCredentials) {
        if (!OTCachedKey::It()->isPaused()) {
            OTCachedKey::It()->Pause();
        }
        bSavedCert = pNym->Savex509CertAndPrivateKeyToString(strCertfile,
                                                             &strReasonToSave);
        if (OTCachedKey::It()->isPaused()) {
            OTCachedKey::It()->Unpause();
        }
    }
    else {
        // We don't have to pause OTCachedKey here like we did above, because
        // this already has built-in mechanisms to go around OTCachedKey.
        //
        const bool bReEncrypted = pNym->ReEncryptPrivateCredentials(
            false /*bImporting*/,
            &thePWDataSave); // Handles OTCachedKey already.
        if (bReEncrypted) {
            // Create a new OTDB::StringMap object.

            // this asserts already, on failure.
            std::unique_ptr<OTDB::Storable> pStorable(
                OTDB::CreateObject(OTDB::STORED_OBJ_STRING_MAP));
            OTDB::StringMap* pMap =
                dynamic_cast<OTDB::StringMap*>(pStorable.get());
            if (nullptr == pMap)
                otErr << __FUNCTION__
                      << ": Error: failed trying to load or create a "
                         "STORED_OBJ_STRING_MAP.\n";
            else // It instantiated.
            {
                OTString strCredList;
                OTString::Map& theMap = pMap->the_map;

                pNym->GetPrivateCredentials(strCredList, &theMap);
                // Serialize the StringMap to a string...

                // Won't bother if there are zero credentials somehow.
                if (strCredList.Exists() && (!theMap.empty())) {
                    std::string str_Encoded = OTDB::EncodeObject(*pMap);
                    const bool bSuccessEncoding = (str_Encoded.size() > 0);
                    if (bSuccessEncoding) {
                        ascCredList.SetString(
                            strCredList); // <========== Success
                        ascCredentials.Set(
                            str_Encoded.c_str()); // Payload contains
                                                  // credentials list, payload2
                                                  // contains actual
                                                  // credentials.
                        bSavedCert = true;
                    }
                }
            }
        } // bReEncrypted.
    }     // bHasCredentials==true
    if (!bSavedCert) {
        otErr << __FUNCTION__
              << ": Failed while saving Nym's private cert, or private "
                 "credentials, to string.\n"
                 "Reason I was doing this: \""
              << thePWDataSave.GetDisplayString() << "\"\n";
        return false;
    }
    OTString strNymfile;
    const bool bSavedNym = pNym->SavePseudonym(strNymfile);

    if (!bSavedNym) {
        otErr << __FUNCTION__
              << ": Failed while calling "
                 "pNym->SavePseudonym(strNymfile) (to string)\n";
        return false;
    }
    // Create an OTDB::StringMap object.
    //
    // Set the name, id, [certfile|credlist credentials], and nymfile onto it.
    // (Our exported
    // Nym appears as an ASCII-armored text to the naked eye, but when loaded up
    // in code it
    // appears as a map of strings: name, id, [certfile|credlist credentials],
    // and nymfile.)

    // this asserts already, on failure.
    std::unique_ptr<OTDB::Storable> pStorable(
        OTDB::CreateObject(OTDB::STORED_OBJ_STRING_MAP));
    OTDB::StringMap* pMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());
    // It exists.
    //
    if (nullptr == pMap) {
        otErr << __FUNCTION__ << ": Error: failed trying to load or create a "
                                 "STORED_OBJ_STRING_MAP.\n";
        return false;
    }
    OTString::Map& theMap = pMap->the_map;
    theMap["id"] = str_nym_id;
    theMap["name"] = str_nym_name;
    theMap["nymfile"] = strNymfile.Get();

    if (strCertfile.Exists()) theMap["certfile"] = strCertfile.Get();

    if (ascCredList.Exists()) theMap["credlist"] = ascCredList.Get();

    if (ascCredentials.Exists()) theMap["credentials"] = ascCredentials.Get();
    // Serialize the StringMap to a string...
    //
    std::string str_Encoded = OTDB::EncodeObject(*pMap);
    bool bReturnVal = (str_Encoded.size() > 0);

    if (bReturnVal) {
        OTASCIIArmor ascTemp;
        ascTemp.Set(str_Encoded.c_str());
        strOutput.Release();
        bReturnVal = ascTemp.WriteArmoredString(
            strOutput, "EXPORTED NYM" // -----BEGIN OT EXPORTED NYM-----
            );                        // (bool bEscaped=false by default.)
    }

    return bReturnVal;
}

// OT has the capability to export a Nym (normally stored in several files) as
// an encoded
// object (in base64-encoded form) and then import it again.
//
// Returns bool on success, and if pNymID is passed in, will set it to the new
// NymID.
// Also on failure, if the Nym was already there with that ID, and if pNymID is
// passed,
// then it will be set to the ID that was already there.
//
bool OT_API::Wallet_ImportNym(const OTString& FILE_CONTENTS,
                              OTIdentifier* pNymID) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return false;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    OTASCIIArmor ascArmor;
    const bool bLoadedArmor = OTASCIIArmor::LoadFromString(
        ascArmor, FILE_CONTENTS); // str_bookend="-----BEGIN" by default
    if (!bLoadedArmor || !ascArmor.Exists()) {
        otErr << __FUNCTION__
              << ": Failure loading string into OTASCIIArmor object:\n\n"
              << FILE_CONTENTS << "\n\n";
        return false;
    }
    std::unique_ptr<OTDB::Storable> pStorable(
        OTDB::DecodeObject(OTDB::STORED_OBJ_STRING_MAP, ascArmor.Get()));
    OTDB::StringMap* pMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());

    if (nullptr == pMap) {
        otOut << __FUNCTION__
              << ": Failed decoding StringMap object while trying "
                 "to import Nym:\n" << FILE_CONTENTS << "\n";
        return false;
    }
    std::map<std::string, std::string>& theMap = pMap->the_map;
    // By this point, there was definitely a StringMap encoded in the
    // FILE_CONTENTS...
    //
    // Decode the FILE_CONTENTS into a StringMap object,
    // and if success, make sure it contains these values:
    //
    // id:       The NymID.
    // name:     The display name from the wallet.
    // certfile: The public / private certfile in openssl format.
    // nymfile:  The contents of the nymfile.
    //
    if (theMap.end() == theMap.find("id")) // todo hardcoding
    {
        // not found.
        otOut << __FUNCTION__
              << ": Unable to find 'id' field while trying to import Nym:\n"
              << FILE_CONTENTS << "\n";
        return false;
    }
    if (theMap.end() == theMap.find("name")) // todo hardcoding
    {
        // not found.
        otOut << __FUNCTION__
              << ": Unable to find 'name' field while trying to import Nym:\n"
              << FILE_CONTENTS << "\n";
        return false;
    }
    if (theMap.end() == theMap.find("nymfile")) // todo hardcoding
    {
        // not found.
        otOut << __FUNCTION__
              << ": Unable to find 'nymfile' field while trying to "
                 "import Nym:\n" << FILE_CONTENTS << "\n";
        return false;
    }
    if ((theMap.end() == theMap.find("certfile")) && // todo hardcoding
        (theMap.end() == theMap.find("credlist"))) // Logic: No certfile AND no
                                                   // credlist? Gotta have one
                                                   // or the other.
    {
        // not found.
        otOut << __FUNCTION__
              << ": Unable to find a 'certfile' nor a 'credlist' "
                 "field while trying to import Nym:\n" << FILE_CONTENTS << "\n";
        return false;
    }
    // Do various verifications on the values to make sure there's no funny
    // business.
    //
    // If Nym with this ID is ALREADY in the wallet, set pNymID and return
    // false.
    const OTIdentifier theNymID(theMap["id"].c_str());
    const OTString strNymName(theMap["name"].c_str());

    if (nullptr != pNymID) pNymID->SetString(theMap["id"].c_str());
    if (theNymID.empty()) {
        otErr << __FUNCTION__
              << ": Error: NYM_ID passed in is empty; returning false";
        return false;
    }
    // MAKE SURE IT'S NOT ALREADY IN THE WALLET.
    //
    std::unique_ptr<OTPseudonym> pNym(GetOrLoadPrivateNym(
        theNymID, true, __FUNCTION__)); // This logs and ASSERTs already.

    if (nullptr != pNym) // already there.
    {
        otOut << __FUNCTION__
              << ": Tried to import a Nym that's already in wallet: "
              << theMap["id"] << "\n";
        pNym.release();
        return false;
    }
    // Create a new Nym object.
    //
    const OTString strNymID(theNymID);
    pNym.reset(new OTPseudonym(theNymID));

    pNym->SetNymName(strNymName);

    // The Nym being imported has its own password. We ask for that here,
    // so we can preserve it in an OTPassword object and pass it around to
    // everyone who needs it.
    //
    // This way OT doesn't automatically ask for it a billion times as it
    // goes through the process of loading and copying these various keys
    // (None of which utilize the wallet's built-in cached master key, since
    // this Nym is being imported and thus is external to the wallet until
    // that process is complete.)
    //
    OTString strDisplay("Enter passphrase for the Nym being imported.");

    // Circumvents the cached key.

    // bAskTwice is true when exporting (since the export passphrase is being
    // created at that time.) But here during importing, we just ask once,
    // since the passphrase is being used, not created.
    std::unique_ptr<OTPassword> pExportPassphrase(
        OTSymmetricKey::GetPassphraseFromUser(&strDisplay, false));

    if (nullptr == pExportPassphrase) {
        otErr << __FUNCTION__ << ": Failed in GetPassphraseFromUser.\n";
        return false;
    }

    // Pause the master key, since this Nym is coming from outside
    // the wallet. We'll let it load with its own key. (No point using
    // the internal wallet master key here, since the Nym is coming from
    // outside, and doesn't use it anyway.)
    // This is true regardless of whether we load via the old system or
    // the new credentials system.
    //
    if (!(OTCachedKey::It()->isPaused())) {
        OTCachedKey::It()->Pause(); // BELOW THIS POINT, CACHED MASTER KEY IS
                                    // DISABLED.
    }
    // Set the credentials or keys on the new Nym object based on the
    // certfile from the StringMap.
    //
    bool bIfNymLoadKeys = false;
    OTString strReasonToLoad(
        "(ImportNym) To import this Nym, what is its passphrase? ");
    OTString strReasonToSave(
        "(ImportNym) What is your wallet's master passphrase? ");

    OTPasswordData thePWDataLoad(strReasonToLoad.Get());
    OTPasswordData thePWDataSave(strReasonToSave.Get());
    auto it_credlist = theMap.find("credlist");
    auto it_credentials = theMap.find("credentials");
    bool bHasCredentials = false;
    // found "credlist"
    //
    if (theMap.end() != it_credlist) {
        OTASCIIArmor ascCredList;
        OTString strCredList;
        if (it_credlist->second.size() > 0) {
            ascCredList.Set(it_credlist->second.c_str());
            ascCredList.GetString(strCredList);
        }
        // cred list exists, and found "credentials"...
        //
        if (strCredList.Exists() &&
            (theMap.end() != it_credentials)) // and found "credentials"
        {
            OTASCIIArmor ascCredentials;
            if (it_credentials->second.size() > 0) {
                ascCredentials.Set(it_credentials->second.c_str());
                std::unique_ptr<OTDB::Storable> pPrivateStorable(
                    OTDB::DecodeObject(OTDB::STORED_OBJ_STRING_MAP,
                                       ascCredentials.Get()));
                OTDB::StringMap* pPrivateMap =
                    dynamic_cast<OTDB::StringMap*>(pPrivateStorable.get());
                if (nullptr == pPrivateMap) {
                    otOut << __FUNCTION__
                          << ": Failed decoding StringMap object.\n";
                    return false;
                }
                else // IF the list saved, then we save the credentials
                       // themselves...
                {
                    OTString::Map& thePrivateMap = pPrivateMap->the_map;
                    if (false ==
                        pNym->LoadFromString(strCredList, &thePrivateMap,
                                             &strReasonToLoad,
                                             pExportPassphrase.get())) {
                        otErr << __FUNCTION__ << ": Failure loading nym "
                              << strNymID << " from credential string.\n";
                        return false;
                    }
                    else // Success
                    {
                        bIfNymLoadKeys = true; // <============
                        bHasCredentials = true;
                    }
                } // success decoding StringMap
            }     // it_credentials.second().size() > 0
        }         // strCredList.Exists() and it_credentials found.
    }             // found "credlist"
    // found "certfile"
    else if (theMap.end() != theMap.find("certfile")) {
        const OTString strCert(theMap["certfile"]);
        bIfNymLoadKeys = pNym->Loadx509CertAndPrivateKeyFromString(
            strCert, &thePWDataLoad, pExportPassphrase.get());
    }
    // Unpause the OTCachedKey (wallet master key.)
    // Now that we've loaded up the "outsider" using its own key,
    // we now resume normal wallet master key operations so that when
    // we save the Nym, it will be saved using the wallet master key.
    // (Henceforth, it has been "imported.")
    //
    if (OTCachedKey::It()->isPaused()) {
        OTCachedKey::It()->Unpause(); // BELOW THIS POINT, CACHED MASTER KEY IS
                                      // BACK IN EFFECT. (Disabled above.)
    }
    //
    if (bIfNymLoadKeys && pNym->VerifyPseudonym()) {
        // Before we go on switching the credentials around, let's make sure
        // this Nym we're
        // importing isn't already in the wallet.

        if (bHasCredentials &&
            !pNym->ReEncryptPrivateCredentials(true, &thePWDataLoad,
                                               pExportPassphrase.get()))
        // Handles OTCachedKey internally, no need for pausing for this call
        {
            otErr
                << __FUNCTION__
                << ": Failed trying to re-encrypt Nym's private credentials.\n";
            return false;
        }
        // load Nymfile from string
        //
        const OTString strNymfile(theMap["nymfile"]);

        bool bConverted = false;
        const bool bLoaded =
            (strNymfile.Exists() && pNym->LoadFromString(strNymfile));
        //      const bool bLoaded    = (strNymfile.Exists() &&
        // pNym->LoadFromString(strNymfile, &thePrivateMap)); // Unnecessary,
        // since pNym has already loaded with this private info, and it will
        // stay loaded even through loading up the nymfile portion, which does
        // not overwrite it. Furthermore, we have since transformed that data,
        // re-encrypting it to a new key, and that's the important change that
        // we're trying to save here! Therefore I don't want to re-introduce
        // this (now old) version of the private info, therefore this is
        // commented out.
        // If success: Add to Wallet including name.
        //
        if (bLoaded) {
            // Insert to wallet's list of Nyms.
            pWallet->AddNym(*(pNym.release()));
            if (!pWallet->ConvertNymToCachedKey(
                    *pNym)) // This also calls SaveX509CertAndPrivateKey, FYI.
                            // (Or saves credentials, too, whichever is
                            // applicable.)
            {
                otErr << __FUNCTION__
                      << ": Failed while calling "
                         "pWallet->ConvertNymToCachedKey(*pNym)\n";
                return false;
            }
            else
                bConverted = true;
        }
        else {
            otErr << __FUNCTION__ << ": Failed loading nymfile from string.\n";
            return false;
        }
        //
        if (bLoaded && bConverted) // bLoaded is probably superfluous here.
        {
            // save the nymfile.
            //
            if (!pNym->SaveSignedNymfile(*pNym)) {
                otErr << __FUNCTION__
                      << ": Error: Failed calling SaveSignedNymfile.\n";
                return false;
            }
            // the conversion process adds values to the wallet, so we must save
            // it after.
            if (!pWallet->SaveWallet()) {
                otErr << __FUNCTION__
                      << ": Error: Failed to save (updated) wallet.\n";
                return false;
            }

            return true; // <========= Success!
        }
    }
    else
        otErr << __FUNCTION__
              << ": Failed loading or verifying keys|credentials|pseudonym.\n";
    return false;
}

// In this case, instead of importing a special "OT Nym all-in-one exported"
// file format,
// we are importing the public/private keys only, from their Cert file contents,
// and then
// creating a blank Nymfile to go along with it. This is for when people wish to
// import
// pre-existing keys to create a new Nym.
//
// Returns bool on success, and if pNymID is passed in, will set it to the new
// NymID.
// Also on failure, if the Nym was already there with that ID, and if pNymID is
// passed,
// then it will be set to the ID that was already there.
//

bool OT_API::Wallet_ImportCert(const OTString& DISPLAY_NAME,
                               const OTString& FILE_CONTENTS,
                               OTIdentifier* pNymID) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return false;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)

    // Do various verifications on the values to make sure there's no funny
    // business.
    //
    // If Nym with this ID is ALREADY in the wallet, set pNymID and return
    // false.
    // Create a new Nym object.
    //
    std::unique_ptr<OTPseudonym> pNym(new OTPseudonym);

    if (DISPLAY_NAME.Exists()) pNym->SetNymName(DISPLAY_NAME);
    // Pause the master key, since this Nym is coming from outside
    // the wallet.
    //
    if (!(OTCachedKey::It()->isPaused())) {
        OTCachedKey::It()->Pause();
    }
    // Set the public and private keys on the new Nym object based on the
    // certfile from the StringMap.
    //
    OTPasswordData thePWData("To import this Cert, what is its passphrase? ");
    const bool bIfNymLoadKeys =
        pNym->Loadx509CertAndPrivateKeyFromString(FILE_CONTENTS, &thePWData);
    // Unpause the master key. (This may go above the add to wallet, or it may
    // stay here, with the "convert to master key" below.)
    //
    if (OTCachedKey::It()->isPaused()) {
        OTCachedKey::It()->Unpause();
    }
    if (bIfNymLoadKeys && pNym->SetIdentifierByPubkey()) {
        if (nullptr != pNymID) *pNymID = pNym->GetConstID();
        OTPseudonym* pTempNym =
            GetOrLoadPrivateNym(pNym->GetConstID(), false,
                                __FUNCTION__); // This logs and ASSERTs already.

        if (nullptr != pTempNym) // already there.
        {
            const OTString strNymID(pNym->GetConstID());
            otOut << __FUNCTION__
                  << ": Tried to import the Cert for a Nym that's "
                     "already in wallet: " << strNymID << "\n";
            return false;
        }
        // If success: Add to Wallet including name.
        //
        pWallet->AddNym(*(pNym.release())); // Insert to wallet's list of Nyms.

        const bool bConverted = pWallet->ConvertNymToCachedKey(*pNym);

        if (!bConverted) {
            otErr << __FUNCTION__ << ": Failed while calling "
                                     "pWallet->ConvertNymToCachedKey(*pNym)\n";
        }
        else {
            pNym->SaveSignedNymfile(*pNym);
            pWallet->SaveWallet(); // the conversion process adds values to the
                                   // wallet, so we must save it after.
            return true;
        }
    }
    else
        otErr << __FUNCTION__
              << ": Failed loading or verifying key from cert string.\n";
    return false;
}

bool OT_API::Wallet_ExportCert(const OTIdentifier& NYM_ID,
                               OTString& strOutput) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }
    OTPasswordData thePWDataLoad(
        "Need Wallet Master passphrase to export any Cert.");
    OTPasswordData thePWDataSave("Create new passphrase for exported Cert.");
    OTString strReasonToSave(thePWDataSave.GetDisplayString());
    OTPseudonym* pNym =
        GetOrLoadPrivateNym(NYM_ID, false, __FUNCTION__,
                            &thePWDataLoad); // This logs and ASSERTs already.
    if (nullptr == pNym) return false;
    // Pause the master key before exporting, since we want to save this Nym
    // WITHOUT the master key, which it will no longer have, outside of this
    // wallet.
    //
    if (!(OTCachedKey::It()->isPaused())) {
        OTCachedKey::It()->Pause();
    }
    OTString strCertfile;
    const bool bSavedCert =
        pNym->Savex509CertAndPrivateKeyToString(strCertfile, &strReasonToSave);
    // Unpause the master key.
    //
    if (OTCachedKey::It()->isPaused()) {
        OTCachedKey::It()->Unpause();
    }
    if (!bSavedCert) {
        otErr << __FUNCTION__
              << ": Failed while calling "
                 "pNym->Savex509CertAndPrivateKeyToString(strCertfile, "
                 "\"" << thePWDataSave.GetDisplayString() << "\")\n";
        return false;
    }
    if (strCertfile.Exists()) {
        strOutput.Concatenate("%s", strCertfile.Get());
        return true;
    }

    return false;
}

// bool  NumList::Peek(int64_t & lPeek) const;
// bool  NumList::Pop();

bool OT_API::NumList_Add(OTNumList& theList,
                         const OTNumList& theNewNumbers) const
{
    OTNumList tempNewList(theList);

    const bool bSuccess = tempNewList.Add(theNewNumbers);

    if (bSuccess) {
        theList.Release();
        theList.Add(tempNewList);
        return true;
    }
    return false;
}

bool OT_API::NumList_Remove(OTNumList& theList,
                            const OTNumList& theOldNumbers) const
{
    OTNumList tempNewList(theList), tempOldList(theOldNumbers);

    while (tempOldList.Count() > 0) {
        int64_t lPeek = 0;

        if (!tempOldList.Peek(lPeek) || !tempOldList.Pop()) OT_FAIL;

        if (!tempNewList.Remove(lPeek)) return false;
    }

    theList.Release();
    theList.Add(tempNewList);
    return true;
}

// Verifies the presence of theQueryNumbers on theList (as a subset)
//
bool OT_API::NumList_VerifyQuery(const OTNumList& theList,
                                 const OTNumList& theQueryNumbers) const
{
    OTNumList theTempQuery(theQueryNumbers);

    while (theTempQuery.Count() > 0) {
        int64_t lPeek = 0;

        if (!theTempQuery.Peek(lPeek) || !theTempQuery.Pop()) OT_FAIL;

        if (!theList.Verify(lPeek)) return false;
    }

    return true;
}

// Verifies the COUNT and CONTENT (but not the order) matches EXACTLY.
//
bool OT_API::NumList_VerifyAll(const OTNumList& theList,
                               const OTNumList& theQueryNumbers) const
{
    return theList.Verify(theQueryNumbers);
}

int32_t OT_API::NumList_Count(const OTNumList& theList) const
{
    return theList.Count();
}

/** TIME (in seconds, as int64_t)

 This will return the current time in seconds, as a int64_t int32_t.

 Todo:  consider making this available on the server side as well,
 so the smart contracts can see what time it is.
 */
time64_t OT_API::GetTime() const
{
    return OTTimeGetCurrentTime();
}

/** OT-encode a plaintext string.

 const char * OT_API_Encode(const char * szPlaintext);

 This will pack, compress, and base64-encode a plain string.
 Returns the base64-encoded string, or nullptr.

 Internally:
 OTString        strPlain(szPlaintext);
 OTASCIIArmor    ascEncoded(thePlaintext);    // ascEncoded now contains the
 OT-encoded string.
 return            ascEncoded.Get();            // We return it.
 */
bool OT_API::Encode(const OTString& strPlaintext, OTString& strOutput,
                    bool bLineBreaks) const
{
    OTASCIIArmor ascArmor;
    bool bSuccess = ascArmor.SetString(strPlaintext, bLineBreaks); // encodes.

    if (bSuccess) {
        strOutput.Release();
        bSuccess = ascArmor.WriteArmoredString(
            strOutput, "ENCODED TEXT" // -----BEGIN OT ENCODED TEXT-----
            );                        // (bool bEscaped=false by default.)
    }
    return bSuccess;
}

/** Decode an OT-encoded string (back to plaintext.)

 const char * OT_API_Decode(const char * szEncoded);

 This will base64-decode, uncompress, and unpack an OT-encoded string.
 Returns the plaintext string, or nullptr.

 Internally:
 OTASCIIArmor    ascEncoded(szEncoded);
 OTString        strPlain(ascEncoded);    // strPlain now contains the decoded
 plaintext string.
 return            strPlain.Get();            // We return it.
 */
bool OT_API::Decode(const OTString& strEncoded, OTString& strOutput,
                    bool bLineBreaks) const
{
    OTASCIIArmor ascArmor;
    const bool bLoadedArmor = OTASCIIArmor::LoadFromString(
        ascArmor, strEncoded); // str_bookend="-----BEGIN" by default
    if (!bLoadedArmor || !ascArmor.Exists()) {
        otErr << __FUNCTION__
              << ": Failure loading string into OTASCIIArmor object:\n\n"
              << strEncoded << "\n\n";
        return false;
    }
    strOutput.Release();
    const bool bSuccess = ascArmor.GetString(strOutput, bLineBreaks);
    return bSuccess;
}

/** OT-ENCRYPT a plaintext string.

 const char * OT_API_Encrypt(const char * RECIPIENT_NYM_ID, const char *
 szPlaintext);

 This will encode, ENCRYPT, and encode a plain string.
 Returns the base64-encoded ciphertext, or nullptr.

 Internally the C++ code is:
 OTString        strPlain(szPlaintext);
 OTEnvelope        theEnvelope;
 if (theEnvelope.Seal(RECIPIENT_NYM, strPlain)) {    // Now it's encrypted (in
 binary form, inside the envelope), to the recipient's nym.
    OTASCIIArmor    ascCiphertext(theEnvelope);        // ascCiphertext now
 contains the base64-encoded ciphertext (as a string.)
    return ascCiphertext.Get();
 }
 */
bool OT_API::Encrypt(const OTIdentifier& theRecipientNymID,
                     const OTString& strPlaintext, OTString& strOutput) const
{
    OTPasswordData thePWData(OT_PW_DISPLAY);
    OTPseudonym* pRecipientNym =
        GetOrLoadNym(theRecipientNymID, false, __FUNCTION__,
                     &thePWData); // This logs and ASSERTs already.
    if (nullptr == pRecipientNym) return false;
    OTEnvelope theEnvelope;
    bool bSuccess = theEnvelope.Seal(*pRecipientNym, strPlaintext);

    if (bSuccess) {
        OTASCIIArmor ascCiphertext(theEnvelope);
        strOutput.Release();

        bSuccess = ascCiphertext.WriteArmoredString(
            strOutput, "ENCRYPTED TEXT" // -----BEGIN OT ENCRYPTED TEXT-----
            );                          // (bool bEscaped=false by default.)
    }
    return bSuccess;
}

/** OT-DECRYPT an OT-encrypted string back to plaintext.

 const char * OT_API_Decrypt(const char * RECIPIENT_NYM_ID, const char *
 szCiphertext);

 Decrypts the base64-encoded ciphertext back into a normal string plaintext.
 Returns the plaintext string, or nullptr.

 Internally the C++ code is:
 OTEnvelope        theEnvelope;                    // Here is the envelope
 object. (The ciphertext IS the data for an OTEnvelope.)
 OTASCIIArmor    ascCiphertext(szCiphertext);    // The base64-encoded
 ciphertext passed in. Next we'll try to attach it to envelope object...
 if (theEnvelope.SetAsciiArmoredData(ascCiphertext)) {    // ...so that we can
 open it using the appropriate Nym, into a plain string object:
    OTString    strServerReply;                    // This will contain the
 output when we're done.
    const bool    bOpened =                        // Now we try to decrypt:
    theEnvelope.Open(RECIPIENT_NYM, strServerReply);
    if (bOpened) {
        return strServerReply.Get();
    }
 }
 */
bool OT_API::Decrypt(const OTIdentifier& theRecipientNymID,
                     const OTString& strCiphertext, OTString& strOutput) const
{
    OTPseudonym* pRecipientNym =
        GetOrLoadPrivateNym(theRecipientNymID, false,
                            __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pRecipientNym) return false;
    OTEnvelope theEnvelope;
    OTASCIIArmor ascCiphertext;
    const bool bLoadedArmor = OTASCIIArmor::LoadFromString(
        ascCiphertext, strCiphertext); // str_bookend="-----BEGIN" by default
    if (!bLoadedArmor || !ascCiphertext.Exists()) {
        otErr << __FUNCTION__
              << ": Failure loading string into OTASCIIArmor object:\n\n"
              << strCiphertext << "\n\n";
        return false;
    }
    if (theEnvelope.SetAsciiArmoredData(ascCiphertext)) {
        strOutput.Release();
        return theEnvelope.Open(*pRecipientNym, strOutput);
    }
    return false;
}

/** OT-Sign a piece of flat text. (With no discernible bookends around it.)
 strContractType contains, for example, if you are trying to sign a ledger
 (which does not have any existing signatures on it) then you would pass
 LEDGER for strContractType, resulting in -----BEGIN OT SIGNED LEDGER-----
 */
bool OT_API::FlatSign(const OTIdentifier& theSignerNymID,
                      const OTString& strInput, const OTString& strContractType,
                      OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        theSignerNymID, false, __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    if (!strInput.Exists()) {
        otOut << __FUNCTION__
              << ": Empty contract passed in. (Returning false.)\n";
        return false;
    }
    if (!strContractType.Exists()) {
        otOut << __FUNCTION__
              << ": Empty contract type passed in. (Returning false.)\n";
        return false;
    }
    OTString strTemp(strInput);
    return OTContract::SignFlatText(strTemp, strContractType, *pNym, strOutput);
}

/** OT-Sign a CONTRACT.  (First signature)

 const char * OT_API_SignContract(const char * SIGNER_NYM_ID, const char *
 THE_CONTRACT);

 Tries to instantiate the contract object, based on the string passed in.
 Releases all signatures, and then signs the contract.
 Returns the signed contract, or nullptr if failure.

 NOTE: The actual OT functionality (Use Cases) NEVER requires you to sign via
 this function. Why not? because, anytime a signature is needed on something,
 the relevant OT API call will require you to pass in the Nym, and the API
 already
 signs internally wherever it deems appropriate. Thus, this function is only for
 advanced uses, for OT-Scripts, server operators, etc.
 */

bool OT_API::SignContract(const OTIdentifier& theSignerNymID,
                          const OTString& strContract,
                          OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        theSignerNymID, false, __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    if (!strContract.Exists()) {
        otOut << __FUNCTION__
              << ": Empty contract passed in. (Returning false.)\n";
        return false;
    }
    //
    std::unique_ptr<OTContract> pContract(
        OTTransactionType::TransactionFactory(strContract));

    if (nullptr == pContract)
        pContract.reset(OTScriptable::InstantiateScriptable(strContract));

    if (nullptr == pContract)
        pContract.reset(::InstantiateContract(strContract));

    if (nullptr == pContract) {
        otOut << __FUNCTION__ << ": I tried my best. "
                                 "Unable to instantiate contract passed in:\n\n"
              << strContract << "\n\n";
        return false;
    }

    pContract->ReleaseSignatures();
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

/** OT-Sign a CONTRACT.  (Add a signature)

 const char * OT_API_AddSignature(const char * SIGNER_NYM_ID, const char *
 THE_CONTRACT);

 Tries to instantiate the contract object, based on the string passed in.
 Signs the contract, without releasing any signatures that are already there.
 Returns the signed contract, or nullptr if failure.

 NOTE: The actual OT functionality (Use Cases) NEVER requires you to sign via
 this function. Why not? because, anytime a signature is needed on something,
 the relevant OT API call will require you to pass in the Nym, and the API
 already
 signs internally wherever it deems appropriate. Thus, this function is only for
 advanced uses, for OT-Scripts, server operators, etc.
 */
bool OT_API::AddSignature(const OTIdentifier& theSignerNymID,
                          const OTString& strContract,
                          OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        theSignerNymID, false, __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    if (!strContract.Exists()) {
        otOut << __FUNCTION__
              << ": Empty contract passed in. (Returning false.)\n";
        return false;
    }
    //
    std::unique_ptr<OTContract> pContract(
        OTTransactionType::TransactionFactory(strContract));

    if (nullptr == pContract)
        pContract.reset(OTScriptable::InstantiateScriptable(strContract));

    if (nullptr == pContract)
        pContract.reset(::InstantiateContract(strContract));

    if (nullptr == pContract) {
        otOut << __FUNCTION__
              << ": I tried my best. Unable to instantiate contract "
                 "passed in:\n\n" << strContract << "\n\n";
        return false;
    }

    //    pContract->ReleaseSignatures();        // Other than this line, this
    // function is identical to
    // OT_API::SignContract(). (This one adds signatures without removing
    // existing ones.)
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

/** OT-Verify the signature on a CONTRACT.
 Returns true/false (success/fail)

 NOTICE that ppContract can be set, EVEN WHEN THIS RETURNS FALSE!
 Therefore, if you are low-level enough to actually be passing ppContract
 in here, then you had better check it, after the call, whether it returns
 success or fail! Either way, that pointer could be set, in which case YOU
 are responsible to clean it up! Check that pointer for nullptr!
 */
bool OT_API::VerifySignature(const OTString& strContract,
                             const OTIdentifier& theSignerNymID,
                             OTContract** ppContract) const // If you use
                                                            // this optional
                                                            // parameter,
                                                            // then YOU are
                                                            // responsible
                                                            // to clean it
                                                            // up.
{
    OTPasswordData thePWData(OT_PW_DISPLAY);
    OTPseudonym* pNym =
        GetOrLoadNym(theSignerNymID, false, __FUNCTION__,
                     &thePWData); // This logs and ASSERTs already.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    if (!strContract.Exists()) {
        otOut << "OT_API::VerifySignature: Empty contract passed in. "
                 "(Returning false.)\n";
        return false;
    }
    //
    OTContract* pContract = nullptr;
    std::unique_ptr<OTContract> theAngel;

    if (nullptr == pContract)
        pContract = OTTransactionType::TransactionFactory(strContract);

    if (nullptr == pContract)
        pContract = OTScriptable::InstantiateScriptable(strContract);

    if (nullptr == pContract) pContract = ::InstantiateContract(strContract);

    if (nullptr == pContract) {
        otOut << "OT_API::VerifySignature: I tried my best. Unable to "
                 "instantiate contract passed in:\n\n" << strContract << "\n\n";
        return false;
    }
    // Since we created it ourselves using a factory, we can do this:

    // The caller wants to take the contract for himself, without us cleaning
    // it up in here.
    //
    if (nullptr != ppContract) *ppContract = pContract;
    // Else, we will clean it up ourselves...
    else
        theAngel.reset(pContract);
    //    if (!pContract->VerifyContractID())
    ////    if (!pContract->VerifyContract())    // This calls
    /// VerifyContractID(), then GetContractPublicNym(), then VerifySignature()
    ///(with that Nym)
    //    {                                            // Therefore it's only
    // useful for server contracts and asset contracts. Here we can VerifyID and
    // Signature,
    //                                                // and that's good enough
    // for here and most other places, generically speaking.
    //        otOut << "OT_API::VerifySignature: Unable to verify
    // contract ID for contract passed in. NOTE: If you are experiencing "
    //                       "a problem here, CONTACT FELLOW TRAVELER and let
    // him know WHAT KIND OF CONTRACT, and what symptoms you are seeing, "
    //                       "versus what you were expecting to see. Contract
    // contents:\n\n" << strContract << "\n\n";
    //        return false;
    //    }

    if (!pContract->VerifySignature(*pNym)) {
        OTString strSignerNymID(theSignerNymID);
        otOut << "OT_API::VerifySignature: For Nym (" << strSignerNymID
              << "), unable to "
                 "verify signature on contract passed in:\n\n" << strContract
              << "\n\n";
        return false;
    }

    return true;
}

// Verify and Retrieve XML Contents.
//
// Pass in a contract and a user ID, and this function will:
// -- Load the contract up and verify it.
// -- Verify the user's signature on it.
// -- Remove the PGP-style bookends (the signatures, etc)
// -- Output: the XML contents of the contract in string form.
// -- Returns: bool. (success/fail)
//
bool OT_API::VerifyAndRetrieveXMLContents(const OTString& strContract,
                                          const OTIdentifier& theSignerNymID,
                                          OTString& strOutput)
{
    OTContract* pContract = nullptr;
    const bool bSuccess =
        VerifySignature(strContract, theSignerNymID, &pContract);
    //
    // If pContract is not nullptr after the above call, then this Cleanup
    // object
    // will clean it up after we leave the scope of this block.
    //
    std::unique_ptr<OTContract> theAngel(pContract);
    strOutput.Release();

    if (nullptr !=
        pContract) // pContract will always exist, if we were successful.
        return (bSuccess && pContract->SaveContractRaw(strOutput));

    return bSuccess; // In practice this will only happen on failure. (Could
                     // have put "return false".)
}

/// === Verify Account Receipt ===
/// Returns bool. Verifies any asset account (intermediary files) against its
/// own last signed receipt.
/// Obviously this will fail for any new account that hasn't done any
/// transactions yet (and thus has no receipts.)
///
///
bool OT_API::VerifyAccountReceipt(const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& USER_ID,
                                  const OTIdentifier& ACCOUNT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return false;
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTPseudonym* pServerNym =
        const_cast<OTPseudonym*>(pServer->GetContractPublicNym());
    if (nullptr == pServerNym) {
        otErr << "OT_API::VerifyAccountReceipt: should never happen. "
                 "pServerNym is nullptr.\n";
        return false;
    }
    return VerifyBalanceReceipt(*pServerNym, *pNym, SERVER_ID, ACCOUNT_ID);
}

bool OT_API::Create_SmartContract(
    const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                       // signing at this point is only to cause
                                       // a save.)
    time64_t VALID_FROM,               // Default (0 or nullptr) == NOW
    time64_t VALID_TO, // Default (0 or nullptr) == no expiry / cancel anytime
    OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTSmartContract* pContract = new OTSmartContract();
    OT_ASSERT_MSG(nullptr != pContract,
                  "OT_API::Create_SmartContract: ASSERT "
                  "while trying to instantiate blank smart "
                  "contract.\n");
    if (!pContract->SetDateRange(VALID_FROM, VALID_TO)) {
        otOut << "OT_API::Create_SmartContract: Failed trying to set date "
                 "range.\n";
        return false;
    }
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_AddParty(
    const OTString& THE_CONTRACT, // The contract, about to have the bylaw added
                                  // to it.
    const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                       // signing at this point is only to cause
                                       // a save.)
    const OTString& PARTY_NAME, // The Party's NAME as referenced in the smart
                                // contract. (And the scripts...)
    const OTString& AGENT_NAME, // An AGENT will be added by default for this
                                // party. Need Agent NAME.
    OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTScriptable> pContract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == pContract) {
        otOut << __FUNCTION__ << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get()),
        str_agent_name(AGENT_NAME.Get());

    OTParty* pParty = pContract->GetParty(str_party_name);

    if (nullptr != pParty) {
        otOut << __FUNCTION__ << ": Failure: Party already exists. \n";
        return false;
    }

    pParty = new OTParty(str_party_name.c_str(), true /*bIsOwnerNym*/,
                         nullptr /*OwnerID not set until confirm*/,
                         str_agent_name.c_str(),
                         true); // bCreateAgent=false by default.
    OT_ASSERT(nullptr != pParty);

    if (!pContract->AddParty(*pParty)) // takes ownership.
    {
        otOut << __FUNCTION__
              << ": Failed while trying to add party: " << PARTY_NAME << " \n";
        delete pParty;
        pParty = nullptr;
        return false;
    }
    // Success!
    //
    pContract->ReleaseSignatures();
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_AddAccount(
    const OTString& THE_CONTRACT,      // The contract, about to have the clause
                                       // added to it.
    const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                       // signing at this point is only to cause
                                       // a save.)
    const OTString& PARTY_NAME, // The Party's NAME as referenced in the smart
                                // contract. (And the scripts...)
    const OTString& ACCT_NAME,  // The Account's name as referenced in the smart
                                // contract
    const OTString& ASSET_TYPE_ID, // Asset Type ID for the Account.
    OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTScriptable> pContract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == pContract) {
        otOut << "OT_API::SmartContract_AddAccount: Error loading "
                 "smart contract:\n\n" << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    OTParty* pParty = pContract->GetParty(str_party_name);

    if (nullptr == pParty) {
        otOut << "OT_API::SmartContract_AddAccount: Failure: Party "
                 "doesn't exist. \n";
        return false;
    }
    const std::string str_name(ACCT_NAME.Get()),
        str_asset_id(ASSET_TYPE_ID.Get());

    if (nullptr != pParty->GetAccount(str_name)) {
        otOut << "OT_API::SmartContract_AddAccount: Failed adding: "
                 "account is already there with that name (" << str_name
              << ") on "
                 "party: " << str_party_name << " \n";
        return false;
    }
    const OTString strAgentName, strAcctName(str_name.c_str()), strAcctID,
        strAssetTypeID(str_asset_id.c_str());

    if (false ==
        pParty->AddAccount(strAgentName, strAcctName, strAcctID, strAssetTypeID,
                           0)) {
        otOut << "OT_API::SmartContract_AddAccount: Failed trying to "
                 "add account (" << str_name << ") to party: " << str_party_name
              << " \n";
        return false;
    }
    // Success!
    //
    pContract->ReleaseSignatures();
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

int32_t OT_API::SmartContract_CountNumsNeeded(
    const OTString& THE_CONTRACT, // The contract, about to have the bylaw added
                                  // to it.
    const OTString& AGENT_NAME) const // An AGENT will be added by default for
                                      // this
                                      // party. Need Agent NAME.
{
    int32_t nReturnValue = 0;
    const std::string str_agent_name(AGENT_NAME.Get());
    std::unique_ptr<OTScriptable> pContract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));

    if (nullptr == pContract) {
        otOut << "OT_API::SmartContract_CountNumsNeeded: Error loading "
                 "smart contract. \n";
        return nReturnValue;
    }
    // -- nReturnValue starts as 0.
    // -- If agent is authorizing agent for a party, nReturnValue++. (Opening
    // number.)
    // -- If agent is authorized agent for any of party's accts, nReturnValue++
    // for each. (Closing numbers.)
    //
    // (Then return the count.)

    nReturnValue = pContract->GetCountTransNumsNeededForAgent(str_agent_name);
    return nReturnValue;
}

bool OT_API::SmartContract_ConfirmAccount(const OTString& THE_CONTRACT,
                                          const OTIdentifier& SIGNER_NYM_ID,
                                          const OTString& PARTY_NAME,
                                          const OTString& ACCT_NAME,
                                          const OTString& AGENT_NAME,
                                          const OTString& ACCT_ID,
                                          OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    const OTIdentifier theAcctID(ACCT_ID);
    OTAccount* pAccount = GetAccount(theAcctID, __FUNCTION__);
    if (nullptr == pAccount) return false;
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == pScriptable) {
        otOut << __FUNCTION__ << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    OTSmartContract* pContract =
        dynamic_cast<OTSmartContract*>(pScriptable.get());
    if (nullptr == pContract) {
        otOut << __FUNCTION__
              << ": Failure casting to Smart Contract. "
                 "Are you SURE it's a smart contract? Contents:\n"
              << THE_CONTRACT << "\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());
    OTParty* pParty = pContract->GetParty(str_party_name);
    if (nullptr == pParty) {
        otOut << __FUNCTION__
              << ": Failure: Party doesn't exist: " << PARTY_NAME << " \n";
        return false;
    }
    // Make sure there's not already an account here with the same ID (Server
    // disallows.)
    //
    OTPartyAccount* pDupeAcct = pParty->GetAccountByID(theAcctID);
    if (nullptr != pDupeAcct) // It's already there.
    {
        otOut << __FUNCTION__ << ": Failed, since a duplicate account ID ("
              << ACCT_ID << ") was already found on this contract. (Server "
                            "disallows, sorry.)\n";
        return false;
    }
    // Find the account template based on its name, to affix the acct ID to.
    //
    const std::string str_name(ACCT_NAME.Get());

    OTPartyAccount* pPartyAcct = pParty->GetAccount(str_name);
    if (nullptr ==
        pPartyAcct) // It's not already there. (Though it should be...)
    {
        otOut << __FUNCTION__
              << ": Failed: No account found on contract with name: "
              << str_name << " \n";
        return false;
    }
    // the actual asset type ID

    const OTIdentifier theExpectedAssetTypeID(
        pPartyAcct->GetAssetTypeID()); // The expected asset type ID, converting
                                       // from a string.
    const OTIdentifier& theActualAssetTypeID =
        pAccount->GetAssetTypeID(); // the actual asset type ID, already an
                                    // identifier, from the actual account.

    if (theExpectedAssetTypeID != theActualAssetTypeID) {
        const OTString strAssetTypeID(theActualAssetTypeID);
        otOut << __FUNCTION__
              << ": Failed, since the asset type ID of the account ("
              << strAssetTypeID << ") does not match what was expected ("
              << pPartyAcct->GetAssetTypeID()
              << ") according to this contract.\n";
        return false;
    }
    // I'm leaving this here for now, since a party can only be a Nym for now
    // anyway (until I code entities.)
    // Therefore this account COULD ONLY be owned by that Nym anyway, and thus
    // will pass this test.
    // All the above functions aren't that stringent because they are about
    // designing the smart contract, not
    // executing it. But in THIS case, we are actually confirming the thing, and
    // adding our actual account #s
    // to it, signing it, etc, so I might as well save the person the hassle of
    // being rejected later because
    // he accidentally set it up with the wrong Nym.
    //
    if (!pAccount->VerifyOwner(*pNym)) {
        const OTString strNymID(SIGNER_NYM_ID);
        otOut << __FUNCTION__ << ": Failed, since this nym (" << strNymID
              << ") isn't the owner of this account (" << str_name << ").\n";
        return false;
    }
    // When the first ACCOUNT is confirmed, then at that moment, we know which
    // server
    // this smart contract is intended to execute on.
    //
    // If this is the first account being confirmed, then we will set the server
    // ID
    // for the smart contract based on the server ID of this account. Otherwise
    // if this
    // is not the first account being confirmed, then we will compare the server
    // ID
    // that's already on the smart contract, to the server ID for this account,
    // and make
    // sure they match. (Otherwise we will reject the confirmation.)
    //
    // Once the contract is activated, the server will verify all the parties
    // and accounts
    // anyway. So might as well save ourselves the hassle, if this doesn't match
    // up now.
    //
    if (pContract->SetServerIDIfEmpty(pAccount->GetPurportedServerID())) {
        // todo security: possibly want to verify here that this really is the
        // FIRST
        // account being confirmed in this smart contract, or at least the first
        // party.
        // Right now we're just using the server ID being empty as an easy way
        // to find
        // out, but technically a party could slip in a "signed version" without
        // setting
        // the server ID, and it might slip by here (though it would eventually
        // fail some
        // verification.) In the int64_t term we'll do a more thorough check
        // here, though.
    }
    else if (pContract->GetServerID() != pAccount->GetPurportedServerID()) {
        const OTString strServer1(pContract->GetServerID()),
            strServer2(pAccount->GetPurportedServerID());
        otOut << __FUNCTION__
              << ": Failure: The smart contract has a different "
                 "server ID on it already (" << strServer1
              << ") than the one "
                 "that goes with this account (server " << strServer2
              << ", for account " << ACCT_ID << ")\n";
        return false;
    }
    // BY THIS POINT, we know that the account is actually owned by the Nym,
    // and we know that it's got the proper asset type ID that was expected
    // according to the smart contract. We also know that the smart contract
    // has the same server ID as the account being confirmed.
    //
    pPartyAcct->SetAcctID(ACCT_ID.Get());
    pPartyAcct->SetAgentName(AGENT_NAME.Get());
    pContract->ReleaseSignatures();
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_ConfirmParty(
    const OTString& THE_CONTRACT, // The smart contract, about to be changed by
                                  // this function.
    const OTString& PARTY_NAME,   // Should already be on the contract. This way
                                  // we can find it.
    const OTIdentifier& NYM_ID,   // Nym ID for the party, the actual owner,
    OTString& strOutput) const // ===> AS WELL AS for the default AGENT of that
                               // party.
                               // (For now, until I code entities)
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTScriptable> pContract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));

    if (nullptr == pContract) {
        otOut << __FUNCTION__ << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    OTParty* pParty = pContract->GetParty(str_party_name);

    if (nullptr == pParty) {
        otOut << __FUNCTION__ << ": Failure: Party (" << str_party_name
              << ") doesn't exist, so how can you confirm it?\n";
        return false;
    }
    OTParty* pNewParty = new OTParty(
        pParty->GetPartyName(),
        *pNym, // party keeps an internal pointer to pNym from here on.
        pParty->GetAuthorizingAgentName()); // Party name and agent name must
                                            // match, in order to replace /
                                            // activate this party.
    OT_ASSERT(nullptr != pNewParty);
    if (!pParty->CopyAcctsToConfirmingParty(*pNewParty)) {
        otOut << __FUNCTION__
              << ": Failed while trying to copy accounts, while "
                 "confirming party: " << PARTY_NAME << " \n";
        delete pNewParty;
        pNewParty = nullptr;
        return false;
    }
    if (!pContract->ConfirmParty(*pNewParty)) // takes ownership.
                                              // (Deletes the
                                              // theoretical version of
                                              // the party, replaced by
                                              // our actual version
                                              // pNewParty.)
    {
        otOut << __FUNCTION__
              << ": Failed while trying to confirm party: " << PARTY_NAME
              << " \n";
        delete pNewParty;
        pNewParty = nullptr;
        return false;
    }
    // ConfirmParty(), unlike most others, actually signs and saves the contract
    // already.
    // Therefore, all that's needed here is to grab it in string form...
    // (No need to sign again, it's just a waste of resource..)
    //    pContract->ReleaseSignatures();
    //    pContract->SignContract(*pNym);
    //    pContract->SaveContract();

    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to confirm the party, we
    // have to track it until it's activated or until we cancel it.)
    //
    const OTString strInstrument(*pContract);
    OTMessage* pMessage = new OTMessage;
    OT_ASSERT(nullptr != pMessage);

    const OTString strNymID(NYM_ID);

    pMessage->m_strCommand = "outpaymentsMessage";
    pMessage->m_strNymID = strNymID;
    //  pMessage->m_strServerID        = strServerID;
    pMessage->m_ascPayload.SetString(strInstrument);

    pMessage->SignContract(*pNym);
    pMessage->SaveContract();

    pNym->AddOutpayments(*pMessage); // Now the Nym is responsible to delete it.
                                     // It's in his "outpayments".
    OTPseudonym* pSignerNym = pNym;
    pNym->SaveSignedNymfile(*pSignerNym);
    return true;
}

bool OT_API::SmartContract_AddBylaw(
    const OTString& THE_CONTRACT, // The contract, about to have the bylaw added
                                  // to it.
    const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                       // signing at this point is only to cause
                                       // a save.)
    const OTString& BYLAW_NAME, // The Bylaw's NAME as referenced in the smart
                                // contract. (And the scripts...)
    OTString& strOutput) const
{
    const char* BYLAW_LANGUAGE = "chai"; // todo hardcoding.
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTScriptable> pContract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == pContract) {
        otOut << "OT_API::SmartContract_AddBylaw: Error loading smart "
                 "contract:\n\n" << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get()),
        str_language(BYLAW_LANGUAGE);

    OTBylaw* pBylaw = pContract->GetBylaw(str_bylaw_name);

    if (nullptr != pBylaw) {
        otOut << "OT_API::SmartContract_AddBylaw: Failure: Bylaw "
                 "already exists: " << BYLAW_NAME << " \n";
        return false;
    }
    pBylaw = new OTBylaw(str_bylaw_name.c_str(), str_language.c_str());
    OT_ASSERT(nullptr != pBylaw);

    if (!pContract->AddBylaw(*pBylaw)) // takes ownership.
    {
        otOut << "OT_API::SmartContract_AddBylaw: Failed while trying "
                 "to add bylaw: " << BYLAW_NAME << " \n";
        delete pBylaw;
        pBylaw = nullptr;
        return false;
    }
    // Success!
    //
    pContract->ReleaseSignatures();
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_AddHook(
    const OTString& THE_CONTRACT,      // The contract, about to have the clause
                                       // added to it.
    const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                       // signing at this point is only to cause
                                       // a save.)
    const OTString& BYLAW_NAME,  // Should already be on the contract. (This way
                                 // we can find it.)
    const OTString& HOOK_NAME,   // The Hook's name as referenced in the smart
                                 // contract. (And the scripts...)
    const OTString& CLAUSE_NAME, // The actual clause that will be triggered by
                                 // the hook. (You can call this multiple times,
                                 // and have multiple clauses trigger on the
                                 // same hook.)
    OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTScriptable> pContract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == pContract) {
        otOut << "OT_API::SmartContract_AddHook: Error loading smart "
                 "contract:\n\n" << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = pContract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otOut << "OT_API::SmartContract_AddHook: Failure: Bylaw "
                 "doesn't exist: " << str_bylaw_name << " \n";
        return false;
    }
    const std::string str_name(HOOK_NAME.Get()), str_clause(CLAUSE_NAME.Get());

    if (!pBylaw->AddHook(str_name, str_clause)) {
        otOut << "OT_API::SmartContract_AddHook: Failed trying to add "
                 "hook (" << str_name << ", clause " << str_clause
              << ") to bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    // Success!
    //
    pContract->ReleaseSignatures();
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_AddCallback(
    const OTString& THE_CONTRACT,      // The contract, about to have the clause
                                       // added to it.
    const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                       // signing at this point is only to cause
                                       // a save.)
    const OTString& BYLAW_NAME, // Should already be on the contract. (This way
                                // we can find it.)
    const OTString& CALLBACK_NAME, // The Callback's name as referenced in the
                                   // smart contract. (And the scripts...)
    const OTString& CLAUSE_NAME, // The actual clause that will be triggered by
                                 // the callback. (Must exist.)
    OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTScriptable> pContract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == pContract) {
        otOut << "OT_API::SmartContract_AddCallback: Error loading "
                 "smart contract:\n\n" << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = pContract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otOut << "OT_API::SmartContract_AddCallback: Failure: Bylaw "
                 "doesn't exist: " << str_bylaw_name << " \n";
        return false;
    }
    const std::string str_name(CALLBACK_NAME.Get()),
        str_clause(CLAUSE_NAME.Get());

    if (nullptr != pBylaw->GetCallback(str_name)) {
        otOut << "OT_API::SmartContract_AddCallback: Failure: "
                 "Callback (" << str_name
              << ") already exists on bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    if (!pBylaw->AddCallback(str_name.c_str(), str_clause.c_str())) {
        otOut << "OT_API::SmartContract_AddCallback: Failed trying to "
                 "add callback (" << str_name << ", clause " << str_clause
              << ") to bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    // Success!
    //
    pContract->ReleaseSignatures();
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_AddClause(
    const OTString& THE_CONTRACT,      // The contract, about to have the clause
                                       // added to it.
    const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                       // signing at this point is only to cause
                                       // a save.)
    const OTString& BYLAW_NAME,  // Should already be on the contract. (This way
                                 // we can find it.)
    const OTString& CLAUSE_NAME, // The Clause's name as referenced in the smart
                                 // contract. (And the scripts...)
    const OTString& SOURCE_CODE, // The actual source code for the clause.
    OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTScriptable> pContract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == pContract) {
        otOut << "OT_API::SmartContract_AddClause: Error loading "
                 "smart contract:\n\n" << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = pContract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otOut << "OT_API::SmartContract_AddClause: Failure: Bylaw "
                 "doesn't exist: " << str_bylaw_name
              << " \n Input contract: \n\n" << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_name(CLAUSE_NAME.Get()), str_code(SOURCE_CODE.Get());

    if (nullptr != pBylaw->GetClause(str_name)) {
        otOut << "OT_API::SmartContract_AddClause: Failed adding: "
                 "clause is already there with that name (" << str_name
              << ") on "
                 "bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    if (!pBylaw->AddClause(str_name.c_str(), str_code.c_str())) {
        otOut << "OT_API::SmartContract_AddClause: Failed trying to "
                 "add clause (" << str_name << ") to bylaw: " << str_bylaw_name
              << " \n";
        return false;
    }
    // Success!
    //
    pContract->ReleaseSignatures();
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_AddVariable(
    const OTString& THE_CONTRACT,      // The contract, about to have the clause
                                       // added to it.
    const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                       // signing at this point is only to cause
                                       // a save.)
    const OTString& BYLAW_NAME, // Should already be on the contract. (This way
                                // we can find it.)
    const OTString& VAR_NAME, // The Variable's name as referenced in the smart
                              // contract. (And the scripts...)
    const OTString& VAR_ACCESS, // "constant", "persistent", or "important".
    const OTString& VAR_TYPE,   // "string", "int64_t", or "bool"
    const OTString& VAR_VALUE,  // Contains a string. If type is int64_t, atol()
    // will be used to convert value to a int64_t. If
    // type is bool, the strings "true" or "false"
    // are expected here in order to convert to a
    // bool.
    OTString& strOutput) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTScriptable> pContract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == pContract) {
        otOut << "OT_API::SmartContract_AddVariable: Error loading "
                 "smart contract:\n\n" << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = pContract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otOut << "OT_API::SmartContract_AddVariable: Failure: Bylaw "
                 "doesn't exist: " << str_bylaw_name << " \n";
        return false;
    }
    const std::string str_name(VAR_NAME.Get()), str_access(VAR_ACCESS.Get()),
        str_type(VAR_TYPE.Get()), str_value(VAR_VALUE.Get());

    if (nullptr != pBylaw->GetVariable(str_name)) {
        otOut << "OT_API::SmartContract_AddVariable: Failure: "
                 "Variable (" << str_name
              << ") already exists on bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    OTVariable::OTVariable_Access theAccess = OTVariable::Var_Error_Access;

    if (str_access.compare("constant") == 0)
        theAccess = OTVariable::Var_Constant;
    else if (str_access.compare("persistent") == 0)
        theAccess = OTVariable::Var_Persistent;
    else if (str_access.compare("important") == 0)
        theAccess = OTVariable::Var_Important;
    OTVariable::OTVariable_Type theType = OTVariable::Var_Error_Type;

    if (str_type.compare("bool") == 0)
        theType = OTVariable::Var_Bool;
    else if (str_type.compare("integer") == 0)
        theType = OTVariable::Var_Integer;
    else if (str_type.compare("string") == 0)
        theType = OTVariable::Var_String;
    if ((OTVariable::Var_Error_Type == theType) ||
        (OTVariable::Var_Error_Access == theAccess)) {
        otOut << "OT_API::SmartContract_AddVariable: Failed due to bad "
                 "variable type or bad access type. \n";
        return false;
    }
    bool bAdded = false;

    switch (theType) {
    case OTVariable::Var_Bool: {
        const bool bValue = (str_value.compare("true") == 0);
        bAdded = pBylaw->AddVariable(str_name, bValue, theAccess);
    } break;
    case OTVariable::Var_Integer: {
        const int32_t nValue = atoi(str_value.c_str());
        bAdded = pBylaw->AddVariable(str_name, nValue, theAccess);
    } break;
    case OTVariable::Var_String:
        bAdded = pBylaw->AddVariable(str_name, str_value, theAccess);
        break;
    default:
        // SHOULD NEVER HAPPEN (We already return above, if the variable type
        // isn't correct.)
        OT_FAIL_MSG("Should never happen. You aren't seeing this.");
        break;
    }

    if (!bAdded) {
        otOut << "OT_API::SmartContract_AddVariable: Failed trying to "
                 "add variable (" << str_name
              << ") to bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    // Success!
    //
    pContract->ReleaseSignatures();
    pContract->SignContract(*pNym);
    pContract->SaveContract();
    strOutput.Release();
    pContract->SaveContractRaw(strOutput);
    return true;
}

// The Nym's Name is basically just a client-side label.
// This function lets you change it.
//
// Returns success, true or false.
//
bool OT_API::SetNym_Name(const OTIdentifier& NYM_ID,
                         const OTIdentifier& SIGNER_NYM_ID,
                         const OTString& NYM_NEW_NAME) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return false;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    // -----------------------------------------------------}
    OTPseudonym* pNym = GetNym(NYM_ID, __FUNCTION__);
    OTPseudonym* pSignerNym =
        GetOrLoadPrivateNym(SIGNER_NYM_ID, false, __FUNCTION__);
    if ((nullptr == pNym) || (nullptr == pSignerNym)) return false;
    // By this point, pNym and pSignerNym are good pointers.  (No need to
    // cleanup.)
    // -----------------------------------------------------}
    // Might want to put some more data validation on the name?
    if (!NYM_NEW_NAME.Exists())
        otOut << "OT_API::SetNym_Name: Empty name (bad).\n";
    else {
        OTString strOldName(pNym->GetNymName()); // just in case.
        pNym->SetNymName(NYM_NEW_NAME);
        if (pNym->SaveSignedNymfile(*pSignerNym)) {
            bool bSaveWallet = pWallet->SaveWallet(); // Only cause the nym's
                                                      // name is stored here,
                                                      // too.
            if (!bSaveWallet)
                otErr << __FUNCTION__
                      << ": Failed while trying to save wallet.\n";
            return bSaveWallet;
        }
        else
            pNym->SetNymName(
                strOldName); // Set it back to the old name if failure.
    }
    return false;
}

// The Asset Account's Name is basically just a client-side label.
// This function lets you change it.
//
// Returns success, true or false.
//
bool OT_API::SetAccount_Name(const OTIdentifier& ACCT_ID,
                             const OTIdentifier& SIGNER_NYM_ID,
                             const OTString& ACCT_NEW_NAME) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return false;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    OTPseudonym* pSignerNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pSignerNym) return false;
    OTAccount* pAccount =
        GetAccount(ACCT_ID, __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pAccount) return false;
    if (!ACCT_NEW_NAME.Exists()) // Any other validation to do on the name?
    {
        otOut << "OT_API::SetAccount_Name: New name is empty (bad).\n";
    }
    else {
        otInfo << "Saving updated account file to disk...\n";
        pAccount->SetName(ACCT_NEW_NAME);
        pAccount->ReleaseSignatures();
        if (pAccount->SignContract(*pSignerNym) && pAccount->SaveContract() &&
            pAccount->SaveAccount())
            return pWallet->SaveWallet(); // Only cause the account's name is
                                          // stored here, too.
        else
            otErr
                << "OT_API::SetAccount_Name: Failed doing this:  if "
                   "(pAccount->SignContract(*pSignerNym) "
                   "&& pAccount->SaveContract() && pAccount->SaveAccount())\n";
    }
    return false;
}

/// CALLER is responsible to delete this Nym!
/// (Low level.)
OTPseudonym* OT_API::LoadPublicNym(const OTIdentifier& NYM_ID,
                                   const char* szFuncName) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }

    //    const char * szFunc = (nullptr != szFuncName) ? szFuncName :
    // __FUNCTION__;
    // Grab the name, if there is one.
    // That way if we have to reload it, we'll be able to preserve the name.
    OTString strName;
    const OTString strNymID(NYM_ID);
    OTPseudonym* pNym =
        GetNym(NYM_ID, szFuncName); // This already logs and ASSERTs
    strName = (nullptr != pNym) ? pNym->GetNymName().Get() : strNymID.Get();
    // now strName contains either "" or the Nym's name from wallet.
    return OTPseudonym::LoadPublicNym(NYM_ID, &strName, szFuncName);
}

/// CALLER is responsible to delete the Nym that's returned here!
/// (Low level.)
OTPseudonym* OT_API::LoadPrivateNym(const OTIdentifier& NYM_ID, bool bChecking,
                                    const char* szFuncName,
                                    const OTPasswordData* pPWData,
                                    const OTPassword* pImportPassword) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }
    // Grab the name, if there is one.
    // That way if we have to reload it, we'll be able to preserve the name.
    OTString strName;
    const OTString strNymID(NYM_ID);
    // If the Nym is already in the wallet, we grab the name from the wallet, so
    // we can
    // set the same name onto that Nym again when he's re-loaded.
    //
    OTPseudonym* pNym =
        GetNym(NYM_ID, szFuncName); // This already logs and ASSERTs
    strName = (nullptr != pNym) ? pNym->GetNymName().Get() : strNymID.Get();
    // now strName contains either "" or the Nym's name from wallet.
    OTPasswordData thePWData(OT_PW_DISPLAY);
    if (nullptr == pPWData) pPWData = &thePWData;
    return OTPseudonym::LoadPrivateNym(NYM_ID, bChecking, &strName, szFuncName,
                                       pPWData,
                                       pImportPassword); // CALLER must delete!
}

/*
 OT_API_Msg_HarvestTransactionNumbers

 This function will load up the cron item (which is either a market offer, a
 payment plan,
 or a SMART CONTRACT.)  UPDATE: this function operates on messages, not cron
 items.

 Then it will try to harvest all of the closing transaction numbers for NYM_ID
 that are
 available to be harvested from THE_CRON_ITEM. (There might be zero #s available
 for that
 Nym, which is still a success and will return true. False means error.)

 YOU MIGHT ASK:

 WHY WOULD I WANT to harvest ONLY the closing numbers for the Nym, and not the
 OPENING
 numbers as well? The answer is because for this Nym, the opening number might
 already
 be burned. For example, if Nym just tried to activate a smart contract, and the
 activation
 FAILED, then maybe the opening number is already gone, even though his closing
 numbers, on the
 other hand, are still valid for retrieval. (I have to double check this.)

 HOWEVER, what if the MESSAGE failed, before it even TRIED the transaction? In
 which case,
 the opening number is still good also, and should be retrieved.

 Remember, I have to keep signing for my transaction numbers until they are
 finally closed out.
 They will appear on EVERY balance agreement and transaction statement from here
 until the end
 of time, whenever I finally close out those numbers. If some of them are still
 good on a failed
 transaction, then I want to retrieve them so I can use them, and eventually
 close them out.

 ==> Whereas, what if I am the PARTY to a smart contract, but I am not the
 actual ACTIVATOR / ORIGINATOR
 (who activated the smart contract on the server).  Therefore I never sent any
 transaction to the
 server, and I never burned my opening number. It's probably still a good #. If
 my wallet is not a piece
 of shit, then I should have a stored copy of any contract that I signed. If it
 turns out in the future
 that that contract wasn't activated, then I can retrieve not only my closing
 numbers, but my OPENING
 number as well! IN THAT CASE, I would call OT_API_HarvestAllNumbers() instead
 of OT_API_HarvestClosingNumbers().


 UPDATE: The above logic is now handled automatically in
 OT_API_HarvestTransactionNumbers.
 Therefore OT_API_HarvestClosingNumbers and OT_API_HarvestAllNumbers have been
 removed.

 */

// true == no errors. false == errors.
//
bool OT_API::Msg_HarvestTransactionNumbers(
    const OTMessage& theMsg, const OTIdentifier& USER_ID,
    bool bHarvestingForRetry,          // false until positively asserted.
    bool bReplyWasSuccess,             // false until positively asserted.
    bool bReplyWasFailure,             // false until positively asserted.
    bool bTransactionWasSuccess,       // false until positively asserted.
    bool bTransactionWasFailure) const // false until positively asserted.
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    return theMsg.HarvestTransactionNumbers(
        *pNym, bHarvestingForRetry, bReplyWasSuccess, bReplyWasFailure,
        bTransactionWasSuccess, bTransactionWasFailure);
}

/*

 ------ TODO: Smart Contracts -----------

 TODO:  Whenever a party confirms a smart contract (sending it on to the next
 party) then a copy of
 the smart contract should go into that party's paymentOutbox. Same thing if the
 party is the last
 one in the chain, and has activated it on to the server. A copy sits in the
 paymentOutbox until
 that smart contract is either successfully activated, or FAILS to activate.

 If a smart contract activates, static OTAgreement::DropServerNoticeToNymbox
 already sends an
 'acknowledgment' notice to all parties.

 TODO: If a smart contract fails to activate, it should ALSO send a notice
 ('rejection') to
 all parties.

 TODO: When a party receives a rejection notice in his Nymbox for a certain
 smart contract,
 he looks up that same smart contract in his paymentOutbox, HARVESTS THE CLOSING
 NUMBERS, and
 then moves the notice from his paymentOutbox to his recordBox.

 Until this is added, then clients will go out of sync on rejected smart
 contracts. (Not the kind
 of out-of-sync where they can't do any transactions, but rather, the kind where
 they have certain
 numbers signed out forever but then never use them on anything because their
 client thinks those
 numbers were already used on a smart contract somewhere, and without the above
 code they would
 never have clawed back those numbers.)
 */

bool OT_API::HarvestClosingNumbers(const OTIdentifier&,
                                   const OTIdentifier& NYM_ID,
                                   const OTString& THE_CRON_ITEM) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTCronItem> pCronItem(
        OTCronItem::NewCronItem(THE_CRON_ITEM));
    if (nullptr == pCronItem) {
        otOut << __FUNCTION__
              << ": Error loading the cron item (a cron item is a "
                 "smart contract, or some other recurring transaction such "
                 "as a market offer, or a payment plan.) Contents:\n\n"
              << THE_CRON_ITEM << "\n\n";
        return false;
    }
    pCronItem->HarvestClosingNumbers(*pNym); // <==== the Nym is actually
                                             // harvesting the numbers from the
                                             // Cron Item, and not the other way
                                             // around.
    return true;
}

// NOTE: usually you will want to call OT_API_HarvestClosingNumbers (above),
// since the Opening number is usually
// burned up from some failed transaction or whatever. You don't want to harvest
// the opening number usually,
// just the closing numbers. (If the opening number is burned up, yet you still
// harvest it, then your OT wallet
// could end up using that number again on some other transaction, which will
// obviously then fail since the number
// isn't good anymore. In fact much of OT's design is based on
// minimizing/eliminating any such sync issues.)
// This function is only for those cases where you are sure that the opening
// transaction # hasn't been burned yet,
// such as when the message failed and the transaction wasn't attempted (because
// it never got that far) or such as
// when the contract simply never got signed or activated by one of the other
// parties, and so you want to claw ALL your
// #'s back, and since in that case your opening number is still good, you would
// use the below function to get it back.
//
bool OT_API::HarvestAllNumbers(const OTIdentifier&, const OTIdentifier& NYM_ID,
                               const OTString& THE_CRON_ITEM) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        NYM_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTCronItem> pCronItem(
        OTCronItem::NewCronItem(THE_CRON_ITEM));
    if (nullptr == pCronItem) {
        otOut << __FUNCTION__
              << ": Error loading the cron item (a cron item is a "
                 "smart contract, or some other recurring transaction such "
                 "as a market offer, or a payment plan.) "
                 "Contents:\n\n" << THE_CRON_ITEM << "\n\n";
        return false;
    }
    pCronItem->HarvestOpeningNumber(*pNym);  // <==== the Nym is actually
                                             // harvesting the numbers from the
                                             // Cron Item, and not the other way
                                             // around.
    pCronItem->HarvestClosingNumbers(*pNym); // <==== the Nym is actually
                                             // harvesting the numbers from the
                                             // Cron Item, and not the other way
                                             // around.
    return true;
}

/// This function only tries to load as a public Nym.
/// No need to cleanup, since it adds the Nym to the wallet.
///
OTPseudonym* OT_API::GetOrLoadPublicNym(const OTIdentifier& NYM_ID,
                                        const char* szFuncName) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }

    OTWallet* pWallet = GetWallet(szFuncName); // This logs and ASSERTs already.
    if (nullptr == pWallet) return nullptr;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    //
    // This already logs copiously, including szFuncName...
    //
    return pWallet->GetOrLoadPublicNym(NYM_ID, szFuncName);
}

/// This function only tries to load as a private Nym.
/// No need to cleanup, since it adds the Nym to the wallet.
///
/// It is smart enough to Get the Nym from the wallet, and if it
/// sees that it's only a public nym (no private key) then it
/// reloads it as a private nym at that time.
///
OTPseudonym* OT_API::GetOrLoadPrivateNym(
    const OTIdentifier& NYM_ID, bool bChecking, const char* szFuncName,
    const OTPasswordData* pPWData, const OTPassword* pImportPassword) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }

    OTWallet* pWallet = GetWallet(szFuncName); // This logs and ASSERTs already.
    if (nullptr == pWallet) return nullptr;
    if (NYM_ID.empty()) return nullptr;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    //
    // This already logs copiously, including szFuncName...
    //
    OTPasswordData thePWData(OT_PW_DISPLAY);
    return pWallet->GetOrLoadPrivateNym(
        NYM_ID, bChecking, szFuncName,
        nullptr == pPWData ? &thePWData : pPWData, pImportPassword);
}

/// This function tries to load as public Nym first, then if it fails,
/// it tries the private one next. (So as to avoid unnecessarily asking
/// users for their passphrase.) Be sure to use GetOrLoadPublicNym() or
/// GetOrLoadPrivateNym() if you want to force it one way or the other.
///
/// No need to cleanup the Nym returned here, since it's added to the wallet and
/// the wallet takes ownership.
///
OTPseudonym* OT_API::GetOrLoadNym(const OTIdentifier& NYM_ID, bool bChecking,
                                  const char* szFuncName,
                                  const OTPasswordData* pPWData) const
{
    if (NYM_ID.empty()) {
        otErr << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }

    OTWallet* pWallet = GetWallet(szFuncName); // This logs and ASSERTs already.
    if (nullptr == pWallet) return nullptr;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    //
    // This already logs copiously, including szFuncName...
    //

    OTPasswordData thePWData(OT_PW_DISPLAY);

    return pWallet->GetOrLoadNym(NYM_ID, bChecking, szFuncName,
                                 nullptr == pPWData ? &thePWData : pPWData);
}

/** Tries to get the account from the wallet.
 Otherwise loads it from local storage.
 */
OTAccount* OT_API::GetOrLoadAccount(const OTPseudonym& theNym,
                                    const OTIdentifier& ACCT_ID,
                                    const OTIdentifier& SERVER_ID,
                                    const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTWallet* pWallet = GetWallet(szFunc); // This logs and ASSERTs already.
    if (nullptr == pWallet) return nullptr;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    return pWallet->GetOrLoadAccount(theNym, ACCT_ID, SERVER_ID,
                                     szFunc); // This logs plenty.
}

/** Tries to get the account from the wallet.
 Otherwise loads it from local storage.
 */
OTAccount* OT_API::GetOrLoadAccount(const OTIdentifier& NYM_ID,
                                    const OTIdentifier& ACCT_ID,
                                    const OTIdentifier& SERVER_ID,
                                    const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTPseudonym* pNym = GetOrLoadPrivateNym(NYM_ID, false, szFunc);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    return GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID,
                            szFunc); // This logs plenty.
}

// WRITE CHEQUE
//
// Returns an OTCheque pointer, or nullptr.
// (Caller responsible to delete.)
//
OTCheque* OT_API::WriteCheque(
    const OTIdentifier& SERVER_ID, const int64_t& CHEQUE_AMOUNT,
    const time64_t& VALID_FROM, const time64_t& VALID_TO,
    const OTIdentifier& SENDER_ACCT_ID, const OTIdentifier& SENDER_USER_ID,
    const OTString& CHEQUE_MEMO, const OTIdentifier* pRECIPIENT_USER_ID) const
{
    OTPseudonym* pNym =
        GetOrLoadPrivateNym(SENDER_USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, SENDER_ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return nullptr;
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    // To write a cheque, we need to burn one of our transaction numbers.
    // (Presumably the wallet
    // is also storing a couple of these, since they are needed to perform any
    // transaction.)
    //
    // I don't have to contact the server to write a cheque -- as long as I
    // already have a transaction
    // number I can use to write it with. (Otherwise I'd have to ask the server
    // to send me one first.)
    //
    OTString strServerID(SERVER_ID);
    int64_t lTransactionNumber =
        0; // Notice I use the server ID on the ACCOUNT.

    if (false ==
        pNym->GetNextTransactionNum(*pNym, strServerID, lTransactionNumber)) {
        otOut << __FUNCTION__
              << ": User attempted to write a cheque, but had no "
                 "transaction numbers.\n";
        return nullptr;
    }
    // At this point, I know that lTransactionNumber contains one I can use.
    OTCheque* pCheque =
        new OTCheque(pAccount->GetRealServerID(), pAccount->GetAssetTypeID());
    OT_ASSERT_MSG(
        nullptr != pCheque,
        "OT_API::WriteCheque: Error allocating memory in the OT API.");
    // At this point, I know that pCheque is a good pointer that I either
    // have to delete, or return to the caller.
    bool bIssueCheque = pCheque->IssueCheque(
        CHEQUE_AMOUNT, lTransactionNumber, VALID_FROM, VALID_TO, SENDER_ACCT_ID,
        SENDER_USER_ID, CHEQUE_MEMO, pRECIPIENT_USER_ID);
    if (!bIssueCheque) {
        otErr << __FUNCTION__ << ": Failure calling OTCheque::IssueCheque().\n";
        delete pCheque;
        pCheque = nullptr;
        // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE NUMBERS.
        pNym->AddTransactionNum(*pNym, strServerID, lTransactionNumber,
                                true); // bSave=true
        return nullptr;
    }
    pCheque->SignContract(*pNym);
    pCheque->SaveContract();
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to write the cheque,
    // we have to track it until it's deposited or until we cancel
    // it.)
    //
    const OTString strInstrument(*pCheque);
    OTMessage* pMessage = new OTMessage;
    OT_ASSERT(nullptr != pMessage);

    const OTString strNymID(SENDER_USER_ID);

    pMessage->m_strCommand = "outpaymentsMessage";
    pMessage->m_strNymID = strNymID;

    if (nullptr != pRECIPIENT_USER_ID) {
        const OTString strNymID2(*pRECIPIENT_USER_ID);
        pMessage->m_strNymID2 = strNymID2;
    }
    pMessage->m_strServerID = strServerID;
    pMessage->m_ascPayload.SetString(strInstrument);

    pMessage->SignContract(*pNym);
    pMessage->SaveContract();

    pNym->AddOutpayments(*pMessage); // Now the Nym is responsible to delete it.
                                     // It's in his "outpayments".
    OTPseudonym* pSignerNym = pNym;
    pNym->SaveSignedNymfile(*pSignerNym);
    return pCheque;
}

// PROPOSE PAYMENT PLAN  (MERCHANT calls this function)
//
// Returns an OTPaymentPlan pointer, or nullptr.
// (Caller responsible to delete.)
//
// The process (finally) is:
//
// 1) Payment plan is written, and signed, by the recipient. (Merchant.)
// 2) He sends it to the sender, who signs it and submits it. (Payer /
// Customer.)
// 3) The server loads the recipient nym to verify the transaction
//    number. The sender also had to burn a transaction number (to
//    submit it) so now, both have verified trns#s in this way.
//
//
// Payment Plan Delay, and Payment Plan Period, both default to 30 days (if you
// pass 0.)
// Payment Plan Length, and Payment Plan Max Payments, both default to 0, which
// means
// no maximum length and no maximum number of payments.
//
// WARNING: the OTPaymentPlan object being returned, contains 2 transaction
// numbers of the Recipient.
// If you delete that object without actually activating it, make sure you
// retrieve those transaction
// numbers first, and add them BACK to the Recipient's Nym!!
//
// Furthermore, recipient should keep a COPY of this proposal after making it,
// so that he can retrieve the transaction numbers from it, for the same reason.
//
OTPaymentPlan* OT_API::ProposePaymentPlan(
    const OTIdentifier& SERVER_ID,
    const time64_t& VALID_FROM, // Default (0) == NOW (It will set it to the
                                // current time in seconds since Jan 1970)
    const time64_t& VALID_TO,   // Default (0) == no expiry / cancel anytime.
                                // Otherwise this is a LENGTH and is ADDED to
                                // VALID_FROM
    const OTIdentifier& SENDER_ACCT_ID, const OTIdentifier& SENDER_USER_ID,
    const OTString& PLAN_CONSIDERATION, // Like a memo.
    const OTIdentifier& RECIPIENT_ACCT_ID,
    const OTIdentifier& RECIPIENT_USER_ID,
    // ----------------------------------------  // If it's above zero, the
    // initial
    const int64_t& INITIAL_PAYMENT_AMOUNT, // amount will be processed after
    const time64_t& INITIAL_PAYMENT_DELAY, // delay (seconds from now.)
    // ----------------------------------------  // AND SEPARATELY FROM THIS...
    const int64_t& PAYMENT_PLAN_AMOUNT,  // The regular amount charged,
    const time64_t& PAYMENT_PLAN_DELAY,  // which begins occuring after delay
    const time64_t& PAYMENT_PLAN_PERIOD, // (seconds from now) and happens
    // ----------------------------------------// every period, ad infinitum,
    // until
    time64_t PAYMENT_PLAN_LENGTH,     // after the length (in seconds)
    int32_t PAYMENT_PLAN_MAX_PAYMENTS // expires, or after the maximum
    ) const                           // number of payments. These last
{                                     // two arguments are optional.
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        RECIPIENT_USER_ID, false, __FUNCTION__); // This logs, ASSERTs, etc.
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, RECIPIENT_ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return nullptr;
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    OTPaymentPlan* pPlan =
        new OTPaymentPlan(SERVER_ID, pAccount->GetAssetTypeID(), SENDER_ACCT_ID,
                          SENDER_USER_ID, RECIPIENT_ACCT_ID, RECIPIENT_USER_ID);
    OT_ASSERT_MSG(nullptr != pPlan,
                  "OT_API::ProposePaymentPlan: Error allocating "
                  "memory in the OT API for new "
                  "OTPaymentPlan.\n");
    // At this point, I know that pPlan is a good pointer that I either
    // have to delete, or return to the caller. CLEANUP WARNING!
    bool bSuccessSetProposal =
        pPlan->SetProposal(*pNym, PLAN_CONSIDERATION, VALID_FROM, VALID_TO);
    // WARNING!!!! SetProposal() burns TWO transaction numbers for RECIPIENT.
    // (*pNym)
    // BELOW THIS POINT, if you have an error, then you must retrieve those
    // numbers from
    // the plan, and set them BACK on pNym before you return!!!
    const OTString strServerID(SERVER_ID);

    if (!bSuccessSetProposal) {
        otOut << __FUNCTION__ << ": Failed trying to set the proposal.\n";
        pPlan->HarvestOpeningNumber(*pNym);
        pPlan->HarvestClosingNumbers(*pNym);
        delete pPlan;
        pPlan = nullptr;
        return nullptr;
    }
    bool bSuccessSetInitialPayment = true; // the default, in case user chooses
                                           // not to even have this payment.
    bool bSuccessSetPaymentPlan =
        true; // the default, in case user chooses not to have a payment plan
    if ((INITIAL_PAYMENT_AMOUNT > 0) &&
        (INITIAL_PAYMENT_DELAY >= OT_TIME_ZERO)) {
        // The Initial payment delay is measured in seconds, starting from the
        // "Creation Date".
        bSuccessSetInitialPayment = pPlan->SetInitialPayment(
            INITIAL_PAYMENT_AMOUNT, INITIAL_PAYMENT_DELAY);
    }
    if (!bSuccessSetInitialPayment) {
        otOut << __FUNCTION__
              << ": Failed trying to set the initial payment.\n";
        pPlan->HarvestOpeningNumber(*pNym);
        pPlan->HarvestClosingNumbers(*pNym);
        delete pPlan;
        pPlan = nullptr;
        return nullptr;
    }
    //
    //                  " 6 minutes    ==      360 Seconds\n"
    //                  "10 minutes    ==      600 Seconds\n"
    //                  "1 hour        ==     3600 Seconds\n"
    //                  "1 day        ==    86400 Seconds\n"
    //                  "30 days        ==  2592000 Seconds\n"
    //                  "3 months        ==  7776000 Seconds\n"
    //                  "6 months        == 15552000 Seconds\n\n"
    //
    if (PAYMENT_PLAN_AMOUNT > 0) // If there are regular payments.
    {
        // The payment plan delay is measured in seconds, starting from the
        // "Creation Date".
        time64_t PAYMENT_DELAY =
            OT_TIME_MONTH_IN_SECONDS; // Defaults to 30 days, measured in
                                      // seconds (if you pass 0.)

        if (PAYMENT_PLAN_DELAY > OT_TIME_ZERO)
            PAYMENT_DELAY = PAYMENT_PLAN_DELAY;
        time64_t PAYMENT_PERIOD =
            OT_TIME_MONTH_IN_SECONDS; // Defaults to 30 days, measured in
                                      // seconds (if you pass 0.)

        if (PAYMENT_PLAN_PERIOD > OT_TIME_ZERO)
            PAYMENT_PERIOD = PAYMENT_PLAN_PERIOD;
        time64_t PLAN_LENGTH =
            OT_TIME_ZERO; // Defaults to 0 seconds (for no max length).

        if (PAYMENT_PLAN_LENGTH > OT_TIME_ZERO)
            PLAN_LENGTH = PAYMENT_PLAN_LENGTH;
        int32_t nMaxPayments =
            0; // Defaults to 0 maximum payments (for no maximum).

        if (PAYMENT_PLAN_MAX_PAYMENTS > 0)
            nMaxPayments = PAYMENT_PLAN_MAX_PAYMENTS;
        bSuccessSetPaymentPlan =
            pPlan->SetPaymentPlan(PAYMENT_PLAN_AMOUNT, PAYMENT_DELAY,
                                  PAYMENT_PERIOD, PLAN_LENGTH, nMaxPayments);
    }
    if (!bSuccessSetPaymentPlan) {
        otOut << __FUNCTION__ << ": Failed trying to set the payment plan.\n";
        pPlan->HarvestOpeningNumber(*pNym);
        pPlan->HarvestClosingNumbers(*pNym);
        delete pPlan;
        pPlan = nullptr;
        return nullptr;
    }
    pPlan->SignContract(*pNym); // Here we have saved the MERCHANT's VERSION.
    pPlan->SaveContract(); // A copy of this will be attached to the CUSTOMER's
                           // version as well.
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to propose the plan, we
    // have to track it until it's deposited or until we cancel it.)
    //
    const OTString strInstrument(*pPlan);
    OTMessage* pMessage = new OTMessage;
    OT_ASSERT(nullptr != pMessage);

    const OTString strNymID(RECIPIENT_USER_ID), strNymID2(SENDER_USER_ID);

    pMessage->m_strCommand = "outpaymentsMessage";
    pMessage->m_strNymID = strNymID;
    pMessage->m_strNymID2 = strNymID2;
    pMessage->m_strServerID = strServerID;
    pMessage->m_ascPayload.SetString(strInstrument);

    pMessage->SignContract(*pNym);
    pMessage->SaveContract();

    pNym->AddOutpayments(*pMessage); // Now the Nym is responsible to delete it.
                                     // It's in his "outpayments".
    OTPseudonym* pSignerNym = pNym;
    pNym->SaveSignedNymfile(*pSignerNym);
    return pPlan;
}

// CONFIRM PAYMENT PLAN  (CUSTOMER)
//
// Returns an OTPaymentPlan pointer, or nullptr.
// (Caller responsible to delete.)
//
// The process (currently) is:
//
// 1) Payment plan is written, and signed, by the recipient. (Merchant.)
// 2) He sends it to the sender, who signs it and submits it. (Payer /
// Customer.)
// 3) The server loads the recipient nym to verify the transaction
//    number. The sender also had to burn a transaction number (to
//    submit it) so now, both have verified trns#s in this way.
//
//
bool OT_API::ConfirmPaymentPlan(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& SENDER_USER_ID,
                                const OTIdentifier& SENDER_ACCT_ID,
                                const OTIdentifier& RECIPIENT_USER_ID,
                                OTPaymentPlan& thePlan) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SENDER_USER_ID, false, __FUNCTION__); // This logs, ASSERTs, etc.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, SENDER_ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return false;
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    std::unique_ptr<OTPseudonym> pMerchantNym(
        LoadPublicNym(RECIPIENT_USER_ID, __FUNCTION__));

    //  if (nullptr == pMerchantNym) // We don't have this Nym in our storage
    // already.
    //    {
    //        const OTString strRecipNymID(RECIPIENT_USER_ID);
    //        otOut << __FUNCTION__ << ": There's no (Merchant) Nym with the
    // recipient's NymID in local storage: " << strRecipNymID << "\n";
    //        return false;
    //    }
    // pMerchantNym is also good, and has an angel. (No need to cleanup.)
    //
    // The "Creation Date" of the agreement is re-set here.
    //
    bool bConfirmed =
        thePlan.Confirm(*pNym, pMerchantNym.get(), &RECIPIENT_USER_ID);
    //
    // WARNING:  The call to "Confirm()" uses TWO transaction numbers from pNym!
    // If you don't end up actually USING this payment plan, then you need to
    // retrieve
    // those numbers and Add them back to pNym again.
    // A nice client will store a copy of any payment plans until they are
    // closed out, or canceled,
    // or whatever.
    // ANY FAILURES BELOW THIS POINT need to be smart enough to retrieve those
    // numbers before returning.
    //
    const OTString strServerID(SERVER_ID);

    if (!bConfirmed) {
        otOut << __FUNCTION__ << ": Failed trying to confirm the agreement.\n";
        thePlan.HarvestOpeningNumber(*pNym);
        thePlan.HarvestClosingNumbers(*pNym);
        return false;
    }
    thePlan.SignContract(*pNym); // Here we have saved the CUSTOMER's version,
    thePlan.SaveContract(); // which contains a copy of the merchant's version.
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to confirm the plan, we
    // have to track it until it's deposited or until we cancel it.)
    //
    const OTString strInstrument(thePlan);
    OTMessage* pMessage = new OTMessage;
    OT_ASSERT(nullptr != pMessage);

    const OTString strNymID(SENDER_USER_ID), strNymID2(RECIPIENT_USER_ID);

    pMessage->m_strCommand = "outpaymentsMessage";
    pMessage->m_strNymID = strNymID;
    pMessage->m_strNymID2 = strNymID2;
    pMessage->m_strServerID = strServerID;
    pMessage->m_ascPayload.SetString(strInstrument);

    pMessage->SignContract(*pNym);
    pMessage->SaveContract();

    pNym->AddOutpayments(*pMessage); // Now the Nym is responsible to delete it.
                                     // It's in his "outpayments".
    OTPseudonym* pSignerNym = pNym;
    pNym->SaveSignedNymfile(*pSignerNym);
    return true;
}

// LOAD PURSE
//
// Returns an Purse pointer, or nullptr.
//
// (Caller responsible to delete.)
//
Purse* OT_API::LoadPurse(const OTIdentifier& SERVER_ID,
                         const OTIdentifier& ASSET_ID,
                         const OTIdentifier& USER_ID,
                         const OTString* pstrDisplay) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    const OTString strReason((nullptr == pstrDisplay)
                                 ? "Loading purse from local storage."
                                 : pstrDisplay->Get());
    OTPasswordData thePWData(strReason);
    const OTString strServerID(SERVER_ID);
    const OTString strUserID(USER_ID);
    const OTString strAssetTypeID(ASSET_ID);
    OTPseudonym* pNym =
        GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__,
                            &thePWData); // These copiously log, and ASSERT.
    if (nullptr == pNym) return nullptr;
    Purse* pPurse = new Purse(SERVER_ID, ASSET_ID, USER_ID);
    OT_ASSERT_MSG(nullptr != pPurse,
                  "Error allocating memory in the OT API."); // responsible to
                                                             // delete or return
                                                             // pPurse below
                                                             // this point.

    if (pPurse->LoadPurse(strServerID.Get(), strUserID.Get(),
                          strAssetTypeID.Get())) {
        if (pPurse->VerifySignature(*pNym) &&
            (SERVER_ID == pPurse->GetServerID()) &&
            (ASSET_ID == pPurse->GetAssetID())) {
            return pPurse;
        }
        else
            otOut << __FUNCTION__ << ": Failed verifying purse.\n";
    }
    else
        otInfo << __FUNCTION__ << ": Failed loading purse.\n";
    delete pPurse;
    pPurse = nullptr;

    return nullptr;
}

bool OT_API::SavePurse(const OTIdentifier& SERVER_ID,
                       const OTIdentifier& ASSET_ID,
                       const OTIdentifier& USER_ID, Purse& THE_PURSE) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    if (THE_PURSE.IsPasswordProtected()) {
        otOut << __FUNCTION__
              << ": Failure: This purse is password-protected (exported) "
                 "and cannot be saved inside the wallet without first "
                 "re-importing it.\n";
    }
    else if ((THE_PURSE.GetServerID() != SERVER_ID) ||
               (THE_PURSE.GetAssetID() != ASSET_ID)) {
        otOut << __FUNCTION__ << ": Error: Wrong server or asset ID passed in, "
                                 "considering the purse that was passed.\n";
    }
    else {
        const OTString strServerID(SERVER_ID);
        const OTString strAssetTypeID(ASSET_ID);
        const OTString strUserID(USER_ID);
        if (THE_PURSE.SavePurse(strServerID.Get(), strUserID.Get(),
                                strAssetTypeID.Get()))
            return true;
    }
    const OTString strPurse(THE_PURSE);
    otOut << __FUNCTION__ << ": Failed saving purse:\n\n" << strPurse << "\n\n";
    return false;
}

// Creates a purse owned by a specific Nym. (OWNER_ID.)
//
// Caller is responsible to delete!
//
Purse* OT_API::CreatePurse(const OTIdentifier& SERVER_ID,
                           const OTIdentifier& ASSET_ID,
                           const OTIdentifier& OWNER_ID) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    Purse* pPurse = new Purse(SERVER_ID, ASSET_ID, OWNER_ID);
    OT_ASSERT_MSG(nullptr != pPurse,
                  "Error allocating memory in the OT API."); // responsible to
                                                             // delete or return
                                                             // pPurse below
                                                             // this point.

    return pPurse;
}

// This is the same as CreatePurse, except it creates a password-protected
// purse,
// instead of a Nym-encrypted purse.
//
// Caller is responsible to delete!
//
Purse* OT_API::CreatePurse_Passphrase(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& ASSET_ID) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    Purse* pPurse = new Purse(SERVER_ID, ASSET_ID);
    OT_ASSERT_MSG(nullptr != pPurse,
                  "Error allocating memory in the OT API."); // responsible to
                                                             // delete or return
                                                             // pPurse below
                                                             // this point.
    if (pPurse->GenerateInternalKey())
        return pPurse;
    else
        otErr << "OT_API::CreatePurse_Passphrase: "
                 "pPurse->GenerateInternalKey() returned false. (Failure.)\n";
    delete pPurse;
    pPurse = nullptr;
    return nullptr;
}

// Caller responsible to delete!
//
// This method was added because otherwise its code would be
// duplicated across many of the Purse functions.
//
OTNym_or_SymmetricKey* OT_API::LoadPurseAndOwnerFromString(
    const OTIdentifier& theServerID, const OTIdentifier& theAssetTypeID,
    const OTString& strPurse, Purse& thePurse, // output
    OTPassword& thePassword, // Only used in the case of password-protected
                             // purses. Passed in so it won't go out of scope
                             // when pOwner is set to point to it.
    bool bForEncrypting,     // true==encrypting,false==decrypting.
                             // Only relevant if there's an owner.
    const OTIdentifier* pOWNER_ID,      // This can be nullptr, **IF** purse
                                        // is password-protected. (It's
                                        // just ignored in that case.)
                                        // Otherwise MUST contain the NymID
                                        // for the Purse owner.
    const OTString* pstrDisplay1,       // for purses owned by the wallet
                                        // already
    const OTString* pstrDisplay2) const // for password-protected purses
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    const bool bDoesOwnerIDExist =
        (nullptr !=
         pOWNER_ID); // If not true, purse MUST be password-protected.
    OTPasswordData thePWData1(
        (nullptr == pstrDisplay1)
            ? "Enter the master passphrase for your wallet. "
              "(LoadPurseAndOwnerFromString)"
            : pstrDisplay1->Get()); // for purses already owned by the wallet
    OTPasswordData thePWData2(
        (nullptr == pstrDisplay2)
            ? "Enter the passphrase for this purse. "
              "(LoadPurseAndOwnerFromString)"
            : pstrDisplay2->Get()); // for password-protected purses.
    OTPseudonym* pOwnerNym =
        nullptr; // In the case where there is an owner, this will point to it.
    if (bDoesOwnerIDExist) {
        pOwnerNym =
            bForEncrypting
                ? GetOrLoadNym(*pOWNER_ID, false, __FUNCTION__, &thePWData1)
                : GetOrLoadPrivateNym(
                      *pOWNER_ID, false, __FUNCTION__,
                      &thePWData1); // These copiously log, and ASSERT.
        if (nullptr == pOwnerNym) return nullptr;
    }
    // By this point, pOwnerNym may be a good pointer, and on the wallet. (No
    // need to cleanup.)
    // Or, it may also be nullptr, in the case that the purse is
    // password-protected.
    // bDoesOwnerIDExist is an easy way to tell, either way.
    OTNym_or_SymmetricKey* pOwner = nullptr;
    if (strPurse.Exists() && thePurse.LoadContractFromString(strPurse)) {
        const bool bNymIDIncludedInPurse = thePurse.IsNymIDIncluded();
        OTIdentifier idPurseNym;

        if (thePurse.GetServerID() != theServerID)
            otErr << __FUNCTION__ << ": Failed: ServerID doesn't match.\n";
        else if (thePurse.GetAssetID() != theAssetTypeID)
            otErr << __FUNCTION__ << ": Failed: AssetTypeID doesn't match.\n";
        else if (bNymIDIncludedInPurse && !thePurse.GetNymID(idPurseNym))
            otErr << __FUNCTION__
                  << ": Failed trying to get the NymID from the "
                     "purse (though one WAS apparently present.)\n";
        else if (bNymIDIncludedInPurse && !bDoesOwnerIDExist) {
            const OTString strPurseNymID(idPurseNym);
            otErr << __FUNCTION__
                  << ": Error: The purse is owned by a NymID, but no "
                     "NymID was passed into "
                     "this function.\nNym who owns the purse: " << strPurseNymID
                  << "\n\n";
        }
        else if (bNymIDIncludedInPurse &&
                   !(pOwnerNym->GetConstID() == idPurseNym)) {
            const OTString strPurseNymID(idPurseNym),
                strNymActuallyPassed(pOwnerNym->GetConstID());
            otErr << __FUNCTION__
                  << ": Error: the API call mentions Nym A, but the "
                     "purse is actually owned by Nym B.\nNym A: "
                  << strNymActuallyPassed << "\nNym B: " << strPurseNymID
                  << "\n\n";
        }
        else if (!bDoesOwnerIDExist && !thePurse.IsPasswordProtected())
            otErr << __FUNCTION__
                  << ": Failed: The USER_ID was nullptr, which is only allowed "
                     "for a password-protected purse. (And this purse is NOT "
                     "password-protected.) Please provide a USER_ID.\n";
        // By this point, we know the purse loaded up properly, and the server
        // and asset IDs match
        // what we expected. We also know that if the purse included a NymID, it
        // matches the USER_ID
        // that was passed in. We also know that if a User ID was NOT passed in,
        // that the purse WAS
        // definitely a password-protected purse.
        //
        else if (thePurse.IsPasswordProtected()) // Purse is encrypted based on
                                                 // its own built-in symmetric
                                                 // key.
        {
            OTSymmetricKey* pSymmetricKey = thePurse.GetInternalKey();
            OT_ASSERT(nullptr != pSymmetricKey);
            const bool bGotPassphrase = thePurse.GetPassphrase(
                thePassword, thePWData2.GetDisplayString());

            if (!bGotPassphrase)
                otOut << __FUNCTION__
                      << ": Authentication failed, or otherwise failed "
                         "retrieving secret from user.\n";
            // Below this point, we know thePassword is good, and we know
            // pSymmetricKey is good.
            // Therefore...
            //
            else {
                pOwner = new OTNym_or_SymmetricKey(*pSymmetricKey, thePassword,
                                                   pstrDisplay2); // Can't put
                                                                  // &strReason
                                                                  // here. (It
                                                                  // goes out of
                                                                  // scope.)
                OT_ASSERT(nullptr != pOwner);
            }
        }
        else if (bDoesOwnerIDExist) // Purse is encrypted based on Nym.
        {
            pOwner = new OTNym_or_SymmetricKey(
                *pOwnerNym, pstrDisplay1); // Can't put &strReason here. (It
                                           // goes out of scope.)
            OT_ASSERT(nullptr != pOwner);
        }
        else
            otErr << __FUNCTION__
                  << ": Failed: The purse is not password-protected, "
                     "but rather, is locked by a Nym. "
                     "However, no USER_ID was passed in! Please supply a "
                     "USER_ID in order "
                     "to open this purse.\n";
        // (By this point, pOwner is all set up and ready to go.)
    }
    else
        otOut << __FUNCTION__ << ": Failure loading purse from string:\n"
              << strPurse << "\n";
    return pOwner;
}

// NOTE: This function is identical to the one above it, except
// that one has to verify that the Nym IDs match, whereas this one
// takes the NymID from the purse, if one is there, and makes it
// authoritative. If no NymID is listed on the purse (but it's NOT
// password protected, i.e. there IS a Nym, it's just not listed)
// then pOWNER_ID is the one it will try.
//
// Caller must delete.
//
OTNym_or_SymmetricKey* OT_API::LoadPurseAndOwnerForMerge(
    const OTString& strPurse, Purse& thePurse, // output
    OTPassword& thePassword, // Only used in the case of password-protected
                             // purses. Passed in so it won't go out of scope
                             // when pOwner is set to point to it.
    bool bCanBePublic,       // true==private nym isn't mandatory.
                             // false==private nym IS mandatory.
                             // (Only relevant if there's an owner.)
    const OTIdentifier* pOWNER_ID, // This can be nullptr, **IF** purse
                                   // is password-protected. (It's
                                   // just ignored in that case.)
                                   // Otherwise if it's Nym-protected,
                                   // the purse will have a NymID on
                                   // it already. If not (it's
                                   // optional), then pOWNER_ID is the
                                   // ID it will try next, before
                                   // failing.
    const OTString* pstrDisplay) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    OTPasswordData thePWData((nullptr == pstrDisplay) ? OT_PW_DISPLAY
                                                      : pstrDisplay->Get());
    OTNym_or_SymmetricKey* pOwner = nullptr;
    if (strPurse.Exists() && thePurse.LoadContractFromString(strPurse)) {
        OTIdentifier idPurseNym;

        if (thePurse.IsNymIDIncluded() && !thePurse.GetNymID(idPurseNym))
            otErr << __FUNCTION__
                  << ": Failed trying to get the NymID from the "
                     "purse (though one WAS apparently present.)\n";
        // Purse is encrypted based on Nym.
        //
        // If the purse includes a NymID, then we'll use it. (If we can't use
        // that specific one, then we have failed.)
        else if (thePurse.IsNymIDIncluded() ||
                 // Else if the purse does NOT include a NymID (but also is NOT
                 // password protected, meaning a Nym still exists, but just
                 // isn't named) then we'll use pOWNER_ID passed in. If that's
                 // nullptr, or we fail to find the Nym with it, then we have
                 // failed.
                 (!thePurse.IsNymIDIncluded() &&
                  !thePurse.IsPasswordProtected()) // && (nullptr != pOWNER_ID))
                                                   // //
                                                   // checked inside the block.
                 ) {
            const OTIdentifier* pActualOwnerID =
                thePurse.IsNymIDIncluded() ? &idPurseNym : pOWNER_ID;

            if (nullptr == pActualOwnerID) {
                otErr << __FUNCTION__
                      << ": Failed: The purse is encrypted to a "
                         "specific Nym (not a passphrase) "
                         "but that Nym is not specified in the purse, nor "
                         "was it passed into this "
                         "function. (Failure. Unable to access purse.)\n";
                return nullptr;
            }
            OTPseudonym* pOwnerNym =
                bCanBePublic
                    ? GetOrLoadNym(*pActualOwnerID, false, __FUNCTION__,
                                   &thePWData)
                    : GetOrLoadPrivateNym(
                          *pActualOwnerID, false, __FUNCTION__,
                          &thePWData); // These copiously log, and ASSERT.
            if (nullptr == pOwnerNym) {
                const OTString strAttemptedID(*pActualOwnerID);
                otErr << __FUNCTION__
                      << ": Failed: The purse is encrypted to a specific NymID "
                         "(not a passphrase) which was either specified inside "
                         "the purse, "
                         "or wasn't specified so we guessed the user ID. "
                         "Either way, we "
                         "then failed loading that Nym: " << strAttemptedID
                      << "\n"
                         "(Failure. Unable to access purse.)\n";
                return nullptr;
            }
            // We found the Nym. If it was the Nym listed in the purse, then
            // it's almost certainly the right one.
            // Else if it was the Nym we guessed, it could be right and it could
            // be wrong. We won't find out in that
            // case, until we actually try to use it for decrypting tokens on
            // the purse (and then we'll fail at that
            // time, if it's the wrong Nym.)
            //
            pOwner = new OTNym_or_SymmetricKey(
                *pOwnerNym, pstrDisplay); // Can't put &strReason here. (It goes
                                          // out of scope.)
            OT_ASSERT(nullptr != pOwner);
        }
        // Else if purse IS password protected, then use its internal key.
        else if (thePurse.IsPasswordProtected()) // Purse is encrypted based on
                                                 // its own built-in symmetric
                                                 // key.
        {
            OTSymmetricKey* pSymmetricKey = thePurse.GetInternalKey();
            OT_ASSERT(nullptr != pSymmetricKey);
            const bool bGotPassphrase = thePurse.GetPassphrase(
                thePassword, thePWData.GetDisplayString());

            if (!bGotPassphrase)
                otOut << __FUNCTION__
                      << ": Authentication failed, or otherwise failed "
                         "retrieving secret from user.\n";
            // Below this point, we know thePassword is good, and we know
            // pSymmetricKey is good.
            // Therefore...
            //
            else {
                pOwner = new OTNym_or_SymmetricKey(*pSymmetricKey, thePassword,
                                                   pstrDisplay); // Can't put
                                                                 // &strReason
                // here. (It goes
                // out of scope.)
                OT_ASSERT(nullptr != pOwner);
            }
        }
        else
            otErr << __FUNCTION__ << ": Failed: Somehow this purse is not "
                                     "password-protected, nor "
                                     "is it Nym-protected. (This error should "
                                     "never actually happen.)\n";
        // (By this point, pOwner is all set up and ready to go.)
    }
    else
        otOut << __FUNCTION__ << ": Failure loading purse from string:\n"
              << strPurse << "\n";
    return pOwner;
}

/// Returns the TOKEN on top of the stock (LEAVING it on top of the stack,
/// but giving you a decrypted copy of it.)
///
/// USER_ID can be nullptr, **if** the purse is password-protected.
/// (It's just ignored in that case.) Otherwise MUST contain the NymID for
/// the Purse owner (necessary to decrypt the token.)
///
/// CALLER must delete!!
///
/// returns nullptr if failure.
///
Token* OT_API::Purse_Peek(const OTIdentifier& SERVER_ID,
                          const OTIdentifier& ASSET_TYPE_ID,
                          const OTString& THE_PURSE,
                          const OTIdentifier* pOWNER_ID, // This can be
                                                         // nullptr,
                                                         // **IF** purse
                          // is password-protected. (It's
                          // just ignored in that case.)
                          // Otherwise MUST contain the NymID
                          // for the Purse owner (necessary
                          // to decrypt the token.)
                          const OTString* pstrDisplay) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    const OTString strReason1(
        (nullptr == pstrDisplay)
            ? "Enter your master passphrase for your wallet. (Purse_Peek)"
            : pstrDisplay->Get());
    const OTString strReason2(
        (nullptr == pstrDisplay)
            ? "Enter the passphrase for this purse. (Purse_Peek)"
            : pstrDisplay->Get());
    //  OTPasswordData thePWData(strReason);
    Purse thePurse(SERVER_ID, ASSET_TYPE_ID);
    OTPassword thePassword; // Only used in the case of password-protected
                            // purses.
    // What's going on here?
    // A purse can be encrypted by a private key (controlled by a Nym) or by a
    // symmetric
    // key (embedded inside the purse along with a corresponding master key.)
    // The below
    // function is what actually loads up thePurse from string (THE_PURSE) and
    // this call
    // also returns pOwner, which is a pointer to a special wrapper class (which
    // you must
    // delete, when you're done with it) which contains a pointer EITHER to the
    // owner Nym
    // for that purse, OR to the "owner" symmetric key for that purse.
    //
    // This way, any subsequent purse operations can use pOwner, regardless of
    // whether there
    // is actually a Nym inside, or a symmetric key. (None of the purse
    // operations will care,
    // since they can use pOwner either way.)
    //
    std::unique_ptr<OTNym_or_SymmetricKey> pOwner(LoadPurseAndOwnerFromString(
        SERVER_ID, ASSET_TYPE_ID, THE_PURSE, thePurse, thePassword,
        false, // bForEncrypting=true by default. (Peek needs to decrypt.)
        pOWNER_ID, &strReason1, &strReason2));
    if (nullptr == pOwner)
        return nullptr; // This already logs, no need for more logs.
    Token* pToken = nullptr;

    if (thePurse.IsEmpty())
        otOut << __FUNCTION__ << ": Failed attempt to peek; purse is empty.\n";
    else {
        pToken = thePurse.Peek(*pOwner);

        if (nullptr == pToken)
            otOut << __FUNCTION__
                  << ": Failed peeking a token from a "
                     "purse that supposedly had tokens on it...\n";
    }
    return pToken;
}

/// Returns the PURSE after popping a single token off of it.
///
/// NOTE: Caller must delete!
/// NOTE: Caller must sign/save in order to effect the change.
///
/// OWNER_ID can be nullptr, **if** the purse is password-protected.
/// (It's just ignored in that case.) Otherwise MUST contain the NymID for
/// the Purse owner (necessary to decrypt the token.)
///
/// The reason you don't see a signer being passed here (to save the purse
/// again, after popping) is because OTAPI.cpp Purse_Pop does the saving.
/// (That's the function that calls this one.) So IT has a signer ID passed
/// in, in addition to the owner ID--but we don't need that here.)
///
/// returns nullptr if failure.
///
Purse* OT_API::Purse_Pop(const OTIdentifier& SERVER_ID,
                         const OTIdentifier& ASSET_TYPE_ID,
                         const OTString& THE_PURSE,
                         const OTIdentifier* pOWNER_ID, // This can be
                                                        // nullptr,
                                                        // **IF** purse
                         // is password-protected. (It's
                         // just ignored in that case.)
                         // Otherwise MUST contain the NymID
                         // for the Purse owner (necessary
                         // to decrypt the token.)
                         const OTString* pstrDisplay) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    const OTString strReason1(
        (nullptr == pstrDisplay)
            ? "Enter your master passphrase for your wallet. (Purse_Pop)"
            : pstrDisplay->Get());
    const OTString strReason2(
        (nullptr == pstrDisplay)
            ? "Enter the passphrase for this purse. (Purse_Pop)"
            : pstrDisplay->Get());
    //  OTPasswordData thePWData(strReason);
    std::unique_ptr<Purse> pPurse(new Purse(SERVER_ID, ASSET_TYPE_ID));

    OTPassword thePassword; // Only used in the case of password-protected
                            // purses.
    // What's going on here?
    // A purse can be encrypted by a private key (controlled by a Nym) or by a
    // symmetric
    // key (embedded inside the purse along with a corresponding master key.)
    // The below
    // function is what actually loads up pPurse from string (THE_PURSE) and
    // this call
    // also returns pOwner, which is a pointer to a special wrapper class (which
    // you must
    // delete, when you're done with it) which contains a pointer EITHER to the
    // owner Nym
    // for that purse, OR to the "owner" symmetric key for that purse.
    //
    // This way, any subsequent purse operations can use pOwner, regardless of
    // whether there
    // is actually a Nym inside, or a symmetric key. (None of the purse
    // operations will care,
    // since they can use pOwner either way.)
    //
    std::unique_ptr<OTNym_or_SymmetricKey> pOwner(LoadPurseAndOwnerFromString(
        SERVER_ID, ASSET_TYPE_ID, THE_PURSE, *pPurse, thePassword,
        false, // bForEncrypting=true by default, but Pop needs to decrypt.
        pOWNER_ID, &strReason1, &strReason2));
    if (nullptr == pOwner)
        return nullptr; // This already logs, no need for more logs.
    Purse* pReturnPurse = nullptr;

    if (pPurse->IsEmpty())
        otOut << __FUNCTION__ << ": Failed attempt to pop; purse is empty.\n";
    else {
        std::unique_ptr<Token> pToken(pPurse->Pop(*pOwner));

        if (nullptr == pToken)
            otOut << __FUNCTION__
                  << ": Failed popping a token from a "
                     "purse that supposedly had tokens on it...\n";
        else {
            pReturnPurse = pPurse.release();
        }
    }
    return pReturnPurse;

    // NOTE: Caller must release/sign/save pReturnPurse, once this returns, in
    // order to effect the change.
}

// Caller must delete.
Purse* OT_API::Purse_Empty(const OTIdentifier& SERVER_ID,
                           const OTIdentifier& ASSET_TYPE_ID,
                           const OTString& THE_PURSE,
                           const OTString* pstrDisplay) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    const OTString strReason((nullptr == pstrDisplay)
                                 ? "Making an empty copy of a cash purse."
                                 : pstrDisplay->Get());
    //  OTPasswordData thePWData(strReason);
    Purse* pPurse = Purse::PurseFactory(THE_PURSE, SERVER_ID, ASSET_TYPE_ID);

    if (nullptr == pPurse) {
        otOut << __FUNCTION__
              << ": Error: THE_PURSE is an empty string. Please pass a "
                 "real purse when calling this function.\n";
        return nullptr;
    }
    pPurse->ReleaseTokens();
    // NOTE: Caller must release/sign/save pReturnPurse, once this returns, in
    // order to effect the change.
    return pPurse;
}

/// Returns the PURSE after pushing a single token onto it.
///
/// NOTE: Caller must delete!
/// NOTE: Caller must sign/save in order to effect the change.
///
/// USER_ID can be nullptr, **if** the purse is password-protected.
/// (It's just ignored in that case.) Otherwise MUST contain the NymID for
/// the Purse owner (necessary to encrypt the token.)
///
/// returns nullptr if failure.
///
Purse* OT_API::Purse_Push(const OTIdentifier& SERVER_ID,
                          const OTIdentifier& ASSET_TYPE_ID,
                          const OTString& THE_PURSE, const OTString& THE_TOKEN,
                          const OTIdentifier* pOWNER_ID, // This can be nullptr,
                                                         // **IF** purse
                          // is password-protected. (It's
                          // just ignored in that case.)
                          // Otherwise MUST contain the NymID
                          // for the Purse owner (necessary
                          // to encrypt the token.)
                          const OTString* pstrDisplay) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    const OTString strReason1(
        (nullptr == pstrDisplay)
            ? "Enter your master passphrase for your wallet. (Purse_Push)"
            : pstrDisplay->Get());
    const OTString strReason2(
        (nullptr == pstrDisplay)
            ? "Enter the passphrase for this purse. (Purse_Push)"
            : pstrDisplay->Get());
    //  OTPasswordData thePWData(strReason);
    if (!THE_PURSE.Exists()) {
        otOut << __FUNCTION__ << ": Purse does not exist.\n";
        return nullptr;
    }
    else if (!THE_TOKEN.Exists()) {
        otOut << __FUNCTION__ << ": Token does not exist.\n";
        return nullptr;
    }
    OTString strToken(THE_TOKEN);
    std::unique_ptr<Token> pToken(
        Token::TokenFactory(strToken, SERVER_ID, ASSET_TYPE_ID));

    if (nullptr == pToken) // TokenFactory instantiates AND loads from string.
    {
        otOut << __FUNCTION__
              << ": Unable to instantiate or load token from string:\n\n"
              << THE_TOKEN << "\n\n";
        return nullptr;
    }
    std::unique_ptr<Purse> pPurse(new Purse(SERVER_ID, ASSET_TYPE_ID));
    OTPassword thePassword; // Only used in the case of password-protected
                            // purses.
    // What's going on here?
    // A purse can be encrypted by a private key (controlled by a Nym) or by a
    // symmetric
    // key (embedded inside the purse along with a corresponding master key.)
    // The below
    // function is what actually loads up pPurse from string (THE_PURSE) and
    // this call
    // also returns pOwner, which is a pointer to a special wrapper class (which
    // you must
    // delete, when you're done with it) which contains a pointer EITHER to the
    // owner Nym
    // for that purse, OR to the "owner" symmetric key for that purse.
    //
    // This way, any subsequent purse operations can use pOwner, regardless of
    // whether there
    // is actually a Nym inside, or a symmetric key. (None of the purse
    // operations will care,
    // since they can use pOwner either way.)
    //
    std::unique_ptr<OTNym_or_SymmetricKey> pOwner(LoadPurseAndOwnerFromString(
        SERVER_ID, ASSET_TYPE_ID, THE_PURSE, *pPurse, thePassword,
        true, // bForEncrypting=true by default.
        pOWNER_ID, &strReason1, &strReason2));
    if (nullptr == pOwner)
        return nullptr; // This already logs, no need for more logs.
    Purse* pReturnPurse = nullptr;

    const bool bPushed = pPurse->Push(*pOwner, *pToken);

    if (!bPushed)
        otOut << __FUNCTION__ << ": Failed pushing a token onto a purse.\n";
    else {
        pReturnPurse = pPurse.release();
    }
    return pReturnPurse;

    // NOTE: Caller must release/sign/save pReturnPurse, once this returns, in
    // order to effect the change.
}

bool OT_API::Wallet_ImportPurse(
    const OTIdentifier& SERVER_ID, const OTIdentifier& ASSET_TYPE_ID,
    const OTIdentifier& SIGNER_ID, // We must know the SIGNER_ID in order to
                                   // know which "old purse" to load and merge
                                   // into. The New Purse may have a different
                                   // one, but its ownership will be re-assigned
                                   // in that case, as part of the merging
                                   // process, to SIGNER_ID. Otherwise the New
                                   // Purse might be symmetrically encrypted
                                   // (instead of using a Nym) in which case
                                   // again, its ownership will be re-assigned
                                   // from that key, to SIGNER_ID, as part of
                                   // the merging process.
    const OTString& THE_PURSE, const OTString* pstrDisplay)
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    OTString strPurseReason((nullptr == pstrDisplay)
                                ? "Enter passphrase for purse being imported."
                                : pstrDisplay->Get());
    OTPasswordData thePWDataWallet(
        (nullptr == pstrDisplay) ? OT_PW_DISPLAY : pstrDisplay->Get());
    OTPassword thePassword; // Only used in the case of password-protected
                            // purses.
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        SIGNER_ID, false, __FUNCTION__,
        &thePWDataWallet); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<Purse> pNewPurse(new Purse(SERVER_ID, ASSET_TYPE_ID));
    // What's going on here?
    // A purse can be encrypted by a private key (controlled by a Nym) or by a
    // symmetric
    // key (embedded inside the purse along with a corresponding master key.)
    // The below
    // function is what actually loads up pPurse from string (THE_PURSE) and
    // this call
    // also returns pOwner, which is a pointer to a special wrapper class (which
    // you must
    // delete, when you're done with it) which contains a pointer EITHER to the
    // owner Nym
    // for that purse, OR to the "owner" symmetric key for that purse.
    //
    // This way, any subsequent purse operations can use pOwner, regardless of
    // whether there
    // is actually a Nym inside, or a symmetric key. (None of the purse
    // operations will care,
    // since they can use pOwner either way.)
    //
    std::unique_ptr<OTNym_or_SymmetricKey> pNewOwner(LoadPurseAndOwnerForMerge(
        THE_PURSE, *pNewPurse, thePassword,
        false, // bCanBePublic=false by default. (Private Nym must be loaded, if
               // a nym is the owner.)
        &SIGNER_ID, // This can be nullptr, **IF** purse is password-protected.
                    // (It's just ignored in that case.) Otherwise if it's
                    // Nym-protected, the purse will have a NymID on it already,
                    // which is what LoadPurseAndOwnerForMerge will try first.
                    // If not (it's optional), then SIGNER_ID is the ID it will
                    // try next, before failing altogether.
        &strPurseReason));
    if (nullptr == pNewOwner)
        return false; // This already logs, no need for more logs.
    std::unique_ptr<Purse> pOldPurse(
        LoadPurse(SERVER_ID, ASSET_TYPE_ID, SIGNER_ID));

    if (nullptr == pOldPurse) // apparently there's not already a purse of this
                              // type, let's create it.
    {
        pOldPurse.reset(new Purse(SERVER_ID, ASSET_TYPE_ID, SIGNER_ID));
    }
    else if (!pOldPurse->VerifySignature(*pNym)) {
        otErr
            << __FUNCTION__
            << ": Failed to verify signature on old purse. (Very strange...)\n";
        return false;
    }
    // By this point, the old purse has either been loaded, or created.
    // Let's make sure the server and asset ID match between the purses,
    // since they are now actually loaded up.
    //
    if (pOldPurse->GetServerID() != pNewPurse->GetServerID()) {
        otOut << __FUNCTION__
              << ": Failure: ServerIDs don't match between these two purses.\n";
        return false;
    }
    else if (pOldPurse->GetAssetID() != pNewPurse->GetAssetID()) {
        otOut << __FUNCTION__
              << ": Failure: AssetIDs don't match between these two purses.\n";
        return false;
    }
    //
    // By this point, I have two owners, and two purses.
    //
    // NOTE: if I want to pass in a custom display string here, pNym could be
    // replaced with
    // pOldOwner (an OTNym_or_SymmetricKey instance) which can contain that
    // string.
    // (I'm referring to the string that contains the "reason" why the
    // passphrase needs to
    // be entered, so it can be shown to the user on the passphrase dialog.)
    //
    if (pOldPurse->Merge(*pNym,       // signer
                         *pNym,       // old owner (must be private, if a nym.)
                         *pNewOwner,  // new owner (must be private, if a nym.)
                         *pNewPurse)) // new purse (being merged into old.)
    {
        pOldPurse->ReleaseSignatures();
        pOldPurse->SignContract(*pNym);
        pOldPurse->SaveContract();
        return SavePurse(SERVER_ID, ASSET_TYPE_ID, SIGNER_ID, *pOldPurse);
    }
    else // Failed merge.
    {
        OTString strNymID1, strNymID2;
        pNym->GetIdentifier(strNymID1);
        pNewOwner->GetIdentifier(strNymID2);
        otOut << __FUNCTION__ << ": (OldNymID: " << strNymID1
              << ".) (New Owner ID: " << strNymID2 << ".) Failure merging new "
                                                      "purse:\n\n" << THE_PURSE
              << "\n\n";
    }
    return false;
}

// ALLOW the caller to pass a symmetric key here, instead of either Nym ID.
// We'll load it up and use it instead of a Nym. Update: make that a purse.
// These tokens already belong to specific purses, so just pass the purse here
//
// Done: Also, add a key cache with a timeout (similar to Master Key) where we
// can stash
// any already-loaded symmetric keys, and a thread wipes them out later. That
// way
// even if we have to load the key each time this func is called, we still avoid
// the
// user having to enter the passphrase more than once per timeout period.
//
// Done also: allow a "Signer ID" to be passed in here, since either owner could
// potentially
// now be a symmetric key.
//
// Caller must delete!
//
Token* OT_API::Token_ChangeOwner(
    const OTIdentifier& SERVER_ID, const OTIdentifier& ASSET_TYPE_ID,
    const OTString& THE_TOKEN, const OTIdentifier& SIGNER_NYM_ID,
    const OTString& OLD_OWNER, // Pass a NymID here, or a purse.
    const OTString& NEW_OWNER, // Pass a NymID here, or a purse.
    const OTString* pstrDisplay) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    OTString strWalletReason(
        (nullptr == pstrDisplay)
            ? "Enter your wallet's master passphrase. (Token_ChangeOwner.)"
            : pstrDisplay->Get());
    OTString strPurseReason(
        (nullptr == pstrDisplay)
            ? "Enter the passphrase for this purse. (Token_ChangeOwner.)"
            : pstrDisplay->Get());
    OTPasswordData thePWDataWallet(strWalletReason);
    OTPseudonym* pSignerNym = GetOrLoadPrivateNym(
        SIGNER_NYM_ID, false, __FUNCTION__,
        &thePWDataWallet); // These copiously log, and ASSERT.
    if (nullptr == pSignerNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    // ALL THE COMPLEXITY YOU SEE BELOW is mainly just about handling OLD_OWNER
    // and NEW_OWNER each as either a NymID or as a Purse (containing a
    // symmetric key and
    // a corresponding master key.)
    OTIdentifier oldOwnerNymID,
        newOwnerNymID; // if either owner is a Nym, the ID goes here.
    std::unique_ptr<Purse> theOldPurseAngel;
    OTPassword theOldPassword; // Only used in the case of password-protected
                               // purses.
    OTNym_or_SymmetricKey* pOldOwner = nullptr;
    std::unique_ptr<OTNym_or_SymmetricKey> theOldOwnerAngel;
    std::unique_ptr<Purse> theNewPurseAngel;
    OTPassword theNewPassword; // Only used in the case of password-protected
                               // purses.
    OTNym_or_SymmetricKey* pNewOwner = nullptr;
    std::unique_ptr<OTNym_or_SymmetricKey> theNewOwnerAngel;
    const bool bOldOwnerIsPurse = OLD_OWNER.Contains("PURSE");
    const bool bNewOwnerIsPurse = NEW_OWNER.Contains("PURSE");
    if (!bOldOwnerIsPurse) // The old owner is a NYM (public/private keys.)
    {
        oldOwnerNymID.SetString(OLD_OWNER);
        OTPseudonym* pOldNym = GetOrLoadPrivateNym(
            oldOwnerNymID, false, __FUNCTION__,
            &thePWDataWallet); // These copiously log, and ASSERT.
                               //      if (nullptr == pOldNym)    pOldNym =
        // GetOrLoadPublicNym(oldOwnerNymID, __FUNCTION__); // must be private,
        // in order to decrypt the old token.
        if (nullptr == pOldNym) return nullptr;
        pOldOwner = new OTNym_or_SymmetricKey(*pOldNym, &strWalletReason);
        OT_ASSERT(nullptr != pOldOwner);
        theOldOwnerAngel.reset(pOldOwner);
    }
    else // The old owner is a PURSE (Symmetric/master keys, internal to that
           // purse.)
    {
        // if the old owner is a Purse (symmetric+master key), the entire
        // purse is loaded.
        Purse* pOldPurse = new Purse(SERVER_ID, ASSET_TYPE_ID);
        OT_ASSERT(nullptr != pOldPurse);
        theOldPurseAngel.reset(pOldPurse);
        pOldOwner = LoadPurseAndOwnerForMerge(
            OLD_OWNER, *pOldPurse, theOldPassword,
            false, // bCanBePublic=false by default. In this case, it definitely
                   // must be private.
            &SIGNER_NYM_ID, // This can be nullptr, **IF** purse is
                            // password-protected. (It's just ignored in that
                            // case.) Otherwise if it's Nym-protected, the purse
                            // will have a NymID on it already, which is what
                            // LoadPurseAndOwnerForMerge will try first. If not
                            // (it's optional), then SIGNER_NYM_ID is the ID it
                            // will try next, before failing altogether.
                            // ADDITIONAL NOTE: We don't expect the purse to
                            // ever be Nym-based since in this function, we pass
                            // a purse in order to pass the symmetric and master
                            // keys. Otherwise if this token's owner was already
                            // a Nym, then we would have passed a NymID in here,
                            // instead of a purse, in the first place.
            &strPurseReason);
        theOldOwnerAngel.reset(pOldOwner);
        if (nullptr == pOldOwner)
            return nullptr; // This already logs, no need for more logs.
    }
    if (!bNewOwnerIsPurse) // The new owner is a NYM
    {
        newOwnerNymID.SetString(NEW_OWNER);
        OTPseudonym* pNewNym =
            GetOrLoadNym(newOwnerNymID, false, __FUNCTION__,
                         &thePWDataWallet); // These copiously log, and ASSERT.
        if (nullptr == pNewNym) return nullptr;
        pNewOwner = new OTNym_or_SymmetricKey(*pNewNym, &strWalletReason);
        OT_ASSERT(nullptr != pNewOwner);
        theNewOwnerAngel.reset(pNewOwner);
    }
    else // The new owner is a PURSE
    {
        // if the new owner is a Purse (symmetric+master key), the entire purse
        // is loaded.
        Purse* pNewPurse = new Purse(SERVER_ID, ASSET_TYPE_ID);
        OT_ASSERT(nullptr != pNewPurse);
        theNewPurseAngel.reset(pNewPurse);
        pNewOwner = LoadPurseAndOwnerForMerge(
            NEW_OWNER, *pNewPurse, theNewPassword,
            true, // bCanBePublic=false by default, but set TRUE here, since you
                  // SHOULD be able to re-assign ownership of a token to someone
                  // else, without having to load their PRIVATE key (which you
                  // don't have.) Sort of irrelevant here actually, since this
                  // block is for purses only...
            &SIGNER_NYM_ID, // This can be nullptr, **IF** purse is
                            // password-protected. (It's just ignored in that
                            // case.) Otherwise if it's Nym-protected, the purse
                            // will have a NymID on it already, which is what
                            // LoadPurseAndOwnerForMerge will try first. If not
                            // (it's optional), then SIGNER_NYM_ID is the ID it
                            // will try next, before failing altogether.
                            // ADDITIONAL NOTE: We don't expect the purse to
                            // ever be Nym-based since in this function, we pass
                            // a purse in order to pass the symmetric and master
                            // keys. Otherwise if this token's owner was already
                            // a Nym, then we would have passed a NymID in here,
                            // instead of a purse, in the first place.
            &strPurseReason);
        theNewOwnerAngel.reset(pNewOwner);
        if (nullptr == pNewOwner)
            return nullptr; // This already logs, no need for more logs.
    }
    //
    // (By this point, pOldOwner and pNewOwner should both be good to go.)
    //
    std::unique_ptr<Token> pToken(
        Token::TokenFactory(THE_TOKEN, SERVER_ID, ASSET_TYPE_ID));
    OT_ASSERT(nullptr != pToken);
    if (false ==
        pToken->ReassignOwnership(*pOldOwner,  // must be private, if a Nym.
                                  *pNewOwner)) // can be public, if a Nym.
    {
        otErr << __FUNCTION__ << ": Error re-assigning ownership of token.\n";
    }
    else {
        otLog3 << __FUNCTION__
               << ": Success re-assigning ownership of token.\n";

        pToken->ReleaseSignatures();
        pToken->SignContract(*pSignerNym);
        pToken->SaveContract();

        return pToken.release();
    }

    return nullptr;
}

// LOAD Mint
//
// Returns an OTMint pointer, or nullptr.
// (Caller responsible to delete.)
//
Mint* OT_API::LoadMint(const OTIdentifier& SERVER_ID,
                       const OTIdentifier& ASSET_ID) const
{
    const OTString strServerID(SERVER_ID);
    const OTString strAssetTypeID(ASSET_ID);
    OTServerContract* pServerContract = GetServer(SERVER_ID, __FUNCTION__);
    if (nullptr == pServerContract) return nullptr;
    const OTPseudonym* pServerNym = pServerContract->GetContractPublicNym();
    if (nullptr == pServerNym) {
        otErr << __FUNCTION__
              << ": Failed trying to get contract public Nym for ServerID: "
              << strServerID << " \n";
        return nullptr;
    }
    Mint* pMint = Mint::MintFactory(strServerID, strAssetTypeID);
    OT_ASSERT_MSG(nullptr != pMint,
                  "OT_API::LoadMint: Error allocating memory in the OT API");
    // responsible to delete or return pMint below this point.
    if (!pMint->LoadMint() || !pMint->VerifyMint(*pServerNym)) {
        otOut << __FUNCTION__
              << ": Unable to load or verify Mintfile : " << OTFolders::Mint()
              << OTLog::PathSeparator() << strServerID << OTLog::PathSeparator()
              << strAssetTypeID << "\n";
        delete pMint;
        pMint = nullptr;
        return nullptr;
    }
    return pMint;
}

// LOAD SERVER CONTRACT (from local storage)
//
// Caller is responsible to delete.
//
OTServerContract* OT_API::LoadServerContract(
    const OTIdentifier& SERVER_ID) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    OTString strServerID(SERVER_ID);

    OTString strFoldername = OTFolders::Contract().Get();
    OTString strFilename = strServerID.Get();
    if (!OTDB::Exists(strFoldername.Get(), strFilename.Get())) {
        otErr << "OT_API::LoadServerContract: File does not exist: "
              << strFoldername.Get() << OTLog::PathSeparator() << strFilename
              << "\n";
        return nullptr;
    }
    OTServerContract* pContract = new OTServerContract(
        strServerID, strFoldername, strFilename, strServerID);
    OT_ASSERT_MSG(nullptr != pContract,
                  "Error allocating memory for Server "
                  "Contract in OT_API::LoadServerContract\n");

    if (pContract->LoadContract() && pContract->VerifyContract())
        return pContract;
    else
        otOut << "OT_API::LoadServerContract: Unable to load or "
                 "verify server contract. (Maybe it's just not there, "
                 "and needs to be downloaded.) Server ID: " << strServerID
              << "\n";
    delete pContract;
    pContract = nullptr;

    return nullptr;
}

// LOAD ASSET CONTRACT (from local storage)
//
// Caller is responsible to delete.
//
OTAssetContract* OT_API::LoadAssetContract(const OTIdentifier& ASSET_ID) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    OTString strAssetTypeID(ASSET_ID);

    OTString strFoldername = OTFolders::Contract().Get();
    OTString strFilename = strAssetTypeID.Get();
    if (!OTDB::Exists(strFoldername.Get(), strFilename.Get())) {
        otErr << "OT_API::LoadAssetContract: File does not exist: "
              << strFoldername.Get() << OTLog::PathSeparator() << strFilename
              << "\n";
        return nullptr;
    }
    OTAssetContract* pContract = new OTAssetContract(
        strAssetTypeID, strFoldername, strFilename, strAssetTypeID);
    OT_ASSERT_MSG(nullptr != pContract,
                  "Error allocating memory for Asset "
                  "Contract in OT_API::LoadAssetContract\n");

    if (pContract->LoadContract() && pContract->VerifyContract())
        return pContract;
    else
        otOut << "OT_API::LoadAssetContract: Unable to load or verify "
                 "asset contract (Maybe it's just not there, and "
                 "needs to be downloaded.) Asset ID: " << strAssetTypeID
              << "\n";
    delete pContract;
    pContract = nullptr;

    return nullptr;
}

// LOAD ASSET ACCOUNT
//
// Caller is NOT responsible to delete -- I add it to the wallet!
//
// We don't care if this asset account is already loaded in the wallet.
// Presumably, the user has just downloaded the latest copy of the account
// from the server, and the one in the wallet is old, so now this function
// is being called to load the new one from storage and update the wallet.
//
// See also: OT_API::GetOrLoadAccount()
//
OTAccount* OT_API::LoadAssetAccount(const OTIdentifier& SERVER_ID,
                                    const OTIdentifier& USER_ID,
                                    const OTIdentifier& ACCOUNT_ID) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return nullptr;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    return pWallet->LoadAccount(*pNym, ACCOUNT_ID, SERVER_ID, __FUNCTION__);
}

// LOAD NYMBOX
//
// Caller IS responsible to delete
//
OTLedger* OT_API::LoadNymbox(const OTIdentifier& SERVER_ID,
                             const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger =
        OTLedger::GenerateLedger(USER_ID, USER_ID, SERVER_ID, OTLedger::nymbox);
    OT_ASSERT_MSG(nullptr != pLedger,
                  "OT_API::LoadNymbox: Error allocating memory in the OT API.");
    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    if (pLedger->LoadNymbox() && pLedger->VerifyAccount(*pNym))
        return pLedger;
    else {
        OTString strUserID(USER_ID);
        otOut << "OT_API::LoadNymbox: Unable to load or verify nymbox: "
              << strUserID << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

// LOAD NYMBOX NO VERIFY
// (VerifyAccount, for ledgers, loads all the Box Receipts. You may not want
// this.
// For example, you may be loading this ledger precisely so you can iterate
// through
// its receipts and download them from the server, so they will all load up on a
// subsequent verify.)
//
// Caller IS responsible to delete
//
OTLedger* OT_API::LoadNymboxNoVerify(const OTIdentifier& SERVER_ID,
                                     const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger =
        OTLedger::GenerateLedger(USER_ID, USER_ID, SERVER_ID, OTLedger::nymbox);
    OT_ASSERT_MSG(
        nullptr != pLedger,
        "OT_API::LoadNymboxNoVerify: Error allocating memory in the OT API.");
    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    if (pLedger->LoadNymbox()) // The Verify would go here.
        return pLedger;
    else {
        OTString strUserID(USER_ID);
        otOut << "OT_API::LoadNymboxNoVerify: Unable to load nymbox: "
              << strUserID << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

// LOAD INBOX
//
// Caller IS responsible to delete
//
OTLedger* OT_API::LoadInbox(const OTIdentifier& SERVER_ID,
                            const OTIdentifier& USER_ID,
                            const OTIdentifier& ACCOUNT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, ACCOUNT_ID, SERVER_ID,
                                                 OTLedger::inbox);
    OT_ASSERT_MSG(nullptr != pLedger,
                  "OT_API::LoadInbox: Error allocating memory in the OT API.");

    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    if (pLedger->LoadInbox() && pLedger->VerifyAccount(*pNym))
        return pLedger;
    else {
        OTString strUserID(USER_ID), strAcctID(ACCOUNT_ID);
        otWarn << "OT_API::LoadInbox: Unable to load or verify inbox: "
               << strAcctID << "\n For user: " << strUserID << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

// LOAD INBOX NO VERIFY
//
// (VerifyAccount, for ledgers, loads all the Box Receipts. You may not want
// this.
// For example, you may be loading this ledger precisely so you can iterate
// through
// its receipts and download them from the server, so they will all load up on a
// subsequent verify.)
//
// Caller IS responsible to delete
//
OTLedger* OT_API::LoadInboxNoVerify(const OTIdentifier& SERVER_ID,
                                    const OTIdentifier& USER_ID,
                                    const OTIdentifier& ACCOUNT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, ACCOUNT_ID, SERVER_ID,
                                                 OTLedger::inbox);
    OT_ASSERT_MSG(
        nullptr != pLedger,
        "OT_API::LoadInboxNoVerify: Error allocating memory in the OT API.");

    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    if (pLedger->LoadInbox()) // The Verify would go here.
        return pLedger;
    else {
        OTString strUserID(USER_ID), strAcctID(ACCOUNT_ID);
        otWarn << "OT_API::LoadInboxNoVerify: Unable to load inbox: "
               << strAcctID << "\n For user: " << strUserID << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

// LOAD OUTBOX
//
// Caller IS responsible to delete
//
OTLedger* OT_API::LoadOutbox(const OTIdentifier& SERVER_ID,
                             const OTIdentifier& USER_ID,
                             const OTIdentifier& ACCOUNT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, ACCOUNT_ID, SERVER_ID,
                                                 OTLedger::outbox);
    OT_ASSERT_MSG(nullptr != pLedger,
                  "OT_API::LoadOutbox: Error allocating memory in the OT API.");

    // Beyond this point, I know that pLedger is loaded and will need to be
    // deleted or returned.

    if (pLedger->LoadOutbox() && pLedger->VerifyAccount(*pNym))
        return pLedger;
    else {
        OTString strUserID(USER_ID), strAcctID(ACCOUNT_ID);

        otWarn << "OT_API::LoadOutbox: Unable to load or verify "
                  "outbox: " << strAcctID << "\n For user: " << strUserID
               << "\n";

        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

// LOAD OUTBOX NO VERIFY
//
// (VerifyAccount, for ledgers, loads all the Box Receipts. You may not want
// this.
// For example, you may be loading this ledger precisely so you can iterate
// through
// its receipts and download them from the server, so they will all load up on a
// subsequent verify.)
//
// Caller IS responsible to delete
//
OTLedger* OT_API::LoadOutboxNoVerify(const OTIdentifier& SERVER_ID,
                                     const OTIdentifier& USER_ID,
                                     const OTIdentifier& ACCOUNT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, ACCOUNT_ID, SERVER_ID,
                                                 OTLedger::outbox);
    OT_ASSERT_MSG(
        nullptr != pLedger,
        "OT_API::LoadOutboxNoVerify: Error allocating memory in the OT API.");

    // Beyond this point, I know that pLedger is loaded and will need to be
    // deleted or returned.

    if (pLedger->LoadOutbox()) // The Verify would go here.
        return pLedger;
    else {
        OTString strUserID(USER_ID), strAcctID(ACCOUNT_ID);
        otWarn << "OT_API::LoadOutboxNoVerify: Unable to load outbox: "
               << strAcctID << "\n For user: " << strUserID << "\n";

        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

OTLedger* OT_API::LoadPaymentInbox(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, USER_ID, SERVER_ID,
                                                 OTLedger::paymentInbox);
    OT_ASSERT_MSG(
        nullptr != pLedger,
        "OT_API::LoadPaymentInbox: Error allocating memory in the OT API.");
    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    if (pLedger->LoadPaymentInbox() && pLedger->VerifyAccount(*pNym))
        return pLedger;
    else {
        OTString strUserID(USER_ID), strAcctID(USER_ID);
        otWarn << __FUNCTION__ << ": Unable to load or verify: " << strUserID
               << " / " << strAcctID << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

OTLedger* OT_API::LoadPaymentInboxNoVerify(const OTIdentifier& SERVER_ID,
                                           const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, USER_ID, SERVER_ID,
                                                 OTLedger::paymentInbox);
    OT_ASSERT_MSG(nullptr != pLedger, "OT_API::LoadPaymentInboxNoVerify: Error "
                                      "allocating memory in the OT API.");
    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    if (pLedger->LoadPaymentInbox()) // The Verify would have gone here.
        return pLedger;
    else {
        OTString strUserID(USER_ID), strAcctID(USER_ID);
        otWarn << __FUNCTION__ << ": Unable to load or verify: " << strUserID
               << " / " << strAcctID << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

// Caller IS responsible to delete
//
OTLedger* OT_API::LoadRecordBox(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTIdentifier& ACCOUNT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;

    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, ACCOUNT_ID, SERVER_ID,
                                                 OTLedger::recordBox);
    OT_ASSERT_MSG(
        nullptr != pLedger,
        "OT_API::LoadRecordBox: Error allocating memory in the OT API.");
    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    const bool bLoaded = pLedger->LoadRecordBox();

    bool bVerified = false;

    if (bLoaded) bVerified = pLedger->VerifyAccount(*pNym);

    if (bLoaded && bVerified)
        return pLedger;
    else {
        OTString strUserID(USER_ID), strAcctID(ACCOUNT_ID);
        otWarn << __FUNCTION__ << ": Unable to load or verify: " << strUserID
               << " / " << strAcctID << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

OTLedger* OT_API::LoadRecordBoxNoVerify(const OTIdentifier& SERVER_ID,
                                        const OTIdentifier& USER_ID,
                                        const OTIdentifier& ACCOUNT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, ACCOUNT_ID, SERVER_ID,
                                                 OTLedger::recordBox);
    OT_ASSERT_MSG(nullptr != pLedger, "OT_API::LoadRecordBoxNoVerify: Error "
                                      "allocating memory in the OT API.");
    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    if (pLedger->LoadRecordBox()) // The Verify would have gone here.
        return pLedger;
    else {
        OTString strUserID(USER_ID), strAcctID(ACCOUNT_ID);
        otWarn << __FUNCTION__ << ": Unable to load or verify: " << strUserID
               << " / " << strAcctID << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

// Caller IS responsible to delete.

OTLedger* OT_API::LoadExpiredBox(const OTIdentifier& SERVER_ID,
                                 const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;

    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, USER_ID, SERVER_ID,
                                                 OTLedger::expiredBox);
    OT_ASSERT_MSG(
        nullptr != pLedger,
        "OT_API::LoadExpiredBox: Error allocating memory in the OT API.");
    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    const bool bLoaded = pLedger->LoadExpiredBox();

    bool bVerified = false;

    if (bLoaded) bVerified = pLedger->VerifyAccount(*pNym);

    if (bLoaded && bVerified)
        return pLedger;
    else {
        OTString strUserID(USER_ID);
        otWarn << __FUNCTION__ << ": Unable to load or verify: " << strUserID
               << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

OTLedger* OT_API::LoadExpiredBoxNoVerify(const OTIdentifier& SERVER_ID,
                                         const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pLedger = OTLedger::GenerateLedger(USER_ID, USER_ID, SERVER_ID,
                                                 OTLedger::expiredBox);
    OT_ASSERT_MSG(nullptr != pLedger, "OT_API::LoadExpiredBoxNoVerify: Error "
                                      "allocating memory in the OT API.");
    // Beyond this point, I know that pLedger will need to be deleted or
    // returned.
    if (pLedger->LoadExpiredBox()) // The Verify would have gone here.
        return pLedger;
    else {
        OTString strUserID(USER_ID);
        otWarn << __FUNCTION__ << ": Unable to load or verify: " << strUserID
               << "\n";
        delete pLedger;
        pLedger = nullptr;
    }
    return nullptr;
}

bool OT_API::ClearExpired(const OTIdentifier& SERVER_ID,
                          const OTIdentifier& USER_ID, const int32_t nIndex,
                          bool bClearAll) const // if true, nIndex is
                                                // ignored.
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTLedger> pExpiredBox(LoadExpiredBox(SERVER_ID, USER_ID));
    if (nullptr == pExpiredBox) {
        pExpiredBox.reset(OTLedger::GenerateLedger(USER_ID, USER_ID, SERVER_ID,
                                                   OTLedger::expiredBox, true));

        if (nullptr == pExpiredBox) {
            otErr << __FUNCTION__
                  << ": Unable to load or create expired box (and thus "
                     "unable to do anything with it.)\n";
            return false;
        }
    }
    if (bClearAll) {
        pExpiredBox->ReleaseTransactions();
        pExpiredBox->ReleaseSignatures();
        pExpiredBox->SignContract(*pNym);
        pExpiredBox->SaveContract();
        pExpiredBox->SaveExpiredBox();
        return true;
    }
    // Okay, it's not "clear all" but "clear at index" ...
    //
    const int32_t nTransCount = pExpiredBox->GetTransactionCount();

    if ((nIndex < 0) || (nIndex >= nTransCount)) {
        otErr << __FUNCTION__
              << ": Index out of bounds (highest allowed index for this "
                 "expired box is " << nTransCount - 1 << ".)\n";
        return false;
    }
    OTTransaction* pTransaction = pExpiredBox->GetTransactionByIndex(nIndex);
    bool bRemoved = false;

    if (nullptr != pTransaction) {
        const int64_t lTransactionNum = pTransaction->GetTransactionNum();

        if (!pExpiredBox->DeleteBoxReceipt(lTransactionNum)) {
            otErr << __FUNCTION__
                  << ": Failed trying to delete the box receipt for a "
                     "transaction being removed from a expired box: "
                  << lTransactionNum << "\n";
        }
        bRemoved = pExpiredBox->RemoveTransaction(lTransactionNum);
    }
    if (bRemoved) {
        pExpiredBox->ReleaseSignatures();
        pExpiredBox->SignContract(*pNym);
        pExpiredBox->SaveContract();
        pExpiredBox->SaveExpiredBox();
        return true;
    }
    else {
        const int32_t nTemp = static_cast<int32_t>(nIndex);
        otOut << __FUNCTION__
              << ": Failed trying to clear an expired record from "
                 "the expired box at index: " << nTemp << "\n";
    }
    return false;
}

// From OTAPI.cpp:
//
// int32_t                OT_API_GetNym_OutpaymentsCount(const char * NYM_ID);
//
// const char *    OT_API_GetNym_OutpaymentsContentsByIndex(const char * NYM_ID,
// int32_t nIndex); /// returns the message itself
//
// const char *    OT_API_GetNym_OutpaymentsRecipientIDByIndex(const char *
// NYM_ID, int32_t nIndex); /// returns the NymID of the recipient.
// const char *    OT_API_GetNym_OutpaymentsServerIDByIndex(const char * NYM_ID,
// int32_t nIndex); /// returns the ServerID where the message came from.
//
// int32_t                OT_API_Nym_RemoveOutpaymentsByIndex(const char *
// NYM_ID, int32_t nIndex); /// actually returns OT_BOOL, (1 or 0.)
// int32_t                OT_API_Nym_VerifyOutpaymentsByIndex(const char *
// NYM_ID, int32_t nIndex); /// actually returns OT_BOOL. OT_TRUE if signature
// verifies. (Sender Nym MUST be in my wallet for this to work.)

// If you write a cheque, a copy should go in outpayments box.
// If you SEND a cheque, a copy should go in your outpayments box. (Perhaps just
// replacing the first one.)
// Once that cheque is CASHED, the copy should be removed from the outpayments.
// (Copied to record box.)
//
// If the cheque is discarded by sender, it can be moved in the same fashion.
// But the critical difference
// is, the recipient MIGHT still have a copy of it, and thus MIGHT still run it
// through, unless you take
// that discarded number and use it on another server transaction or cancel it
// somehow. Normally you'd just
// harvest the number for some other transaction (which the "discard cheque"
// function actually does) but if
// I think about it, that cheque can still come through, meanwhile I'm thinking
// the transaction # is available
// still to be attached to some other cheque (at least as far as my wallet can
// tell, if I've harvested the number
// by this point.) My wallet will think the server is trying to screw me
// somehow, since it apparently hasn't
// even USED that number yet on a subsequent transaction, yet here's the server
// claiming it's already used and
// funds were deducted!
// Therefore we MUST (todo) add the functionality to cancel a transaction
// number!
// TODO: Need a server message for canceling a transaction.
// IF there are no receipts in your inbox or outbox regarding a transaction #,
// then you can cancel it. (Hmm re-think: Is that possible? What if I have a
// transaction # out there on a smart contract somewhere -- that someone ELSE
// activated? Then how would I find it? Perhaps server-side Nym needs to track
// that...)
//
// Really, if a cheque receipt comes through, and we accept it out of the inbox,
// then the "SUCCESS"
// reply for that processInbox is where we need to remove the corresponding
// outpayments entry and move
// it to the record box.
//
// Similarly, if a cheque is canceled (transaction # is canceled) then we should
// receive a "SUCCESS" reply
// (to that cancel transaction) and again, remove the corresponding outpayments
// entry and move it to the
// record box.
//
//
/*
 - In my Payments Inbox, there could be a cheque or invoice. Either way, when I
 deposit the cheque or
 pay the invoice, the chequeReceipt goes back to the signer's asset account's
 inbox.
 - When he accepts the chequeReceipt (during a processInbox) and WHEN HE GETS
 THE "SUCCESS" REPLY to that
 processInbox, is when the chequeReceipt should be moved from his inbox to his
 record box. It MUST be
 done then, inside OT, because the next time he downloads the inbox from the
 server, that chequeReceipt
 won't be in there anymore! It'll be too late to pass it on to the records.
 - Whereas I, being the recipient of his cheque, had it in my **payments
 inbox,** and thus upon receipt
 of a successful server-reply to my deposit transaction, need to move it from my
 payments inbox to my
 record box. (The record box will eventually be a callback so that client
 software can take over that
 functionality, which is outside the scope of OT. The actual CALL to store in
 the record box, however
 should occur inside OT.)
 - For now, I'm using the below API call, so it's available inside the scripts.
 This is "good enough"
 for now, just to get the payments inbox/outbox working for the scripts. But in
 the int64_t term, I'll need
 to add the hooks directly into OT as described just above. (It'll be necessary
 in order to get the record
 box working.)
 - Since I'm only worried about Payments Inbox for now, and since I'll be
 calling the below function
 directly from inside the scripts, how will this work? Incoming cheque or
 invoice will be in the payments
 inbox, and will need to be moved to recordBox (below call) when the script
 receives a success reply to
 the depositing/paying of that cheque/invoice.
 - Whereas outoing cheque/invoice is in the Outpayments box, (fundamentally more
 similar to the outmail
 box than to the payments inbox.) If the cheque/invoice is cashed/paid by the
 endorsee, **I** will receive
 the chequeReceipt, in MY asset account inbox, and when I accept it during a
 processInbox transaction,
 the SUCCESS REPLY from the server for that processInbox is where I should
 actually process that chequeReceipt
 and, if it appears in the outpayments box, move it at that time to the record
 box. The problem is, I can NOT
 do this much inside the script. To do this part, I thus HAVE to go into OT
 itself as I just described.
 - Fuck!
 - Therefore I might as well comment this out, since this simply isn't going to
 work.



 - Updated plan:
   1. DONE: Inside OT, when processing successful server reply to processInbox
 request, if a chequeReceipt
      was processed out successfully, and if that cheque is found inside the
 outpayments, then
      move it at that time to the record box.
   2. DONE: Inside OT, when processing successful server reply to depositCheque
 request, if that cheque is
      found inside the Payments Inbox, move it to the record box.
   3. As for cash:
        If I SENT cash, it will be in my outpayments box. But that's wrong.
 Because I can
      never see if the guy cashed it or not. Therefore it should go straight to
 the record box, when
      sent. AND it needs to be encrypted to MY key, not his -- so need to
 generate BOTH versions, when
      exporting the purse to him in the FIRST PLACE. Then my version goes
 straight into my record box and
      I can delete it at my leisure. (If he comes running the next day saying "I
 lost it!!" I can still
      recover it. But once he deposits it, then the cash will be no good and I
 might as well archive it
      or destroy it, or whatever I choose to do with my personal records.)
        If I RECEIVED cash, it will be in my payments inbox, and then when I
 deposit it, and when I process
      the SUCCESSFUL server REPLY to my depositCash request, it should be moved
 to my record Box.
   4. How about vouchers? If I deposit a voucher, then the "original sender"
 should get some sort of
      notice. This means attaching his ID to the voucher--which should be
 optional--and then dropping an
      "FYI" notice to him when it gets deposited. It can't be a normal
 chequeReceipt because that's used
      to verify the balance agreement against a balance change, whereas a
 "voucher receipt" wouldn't represent
      a balance change at all, since the balance was already changed when you
 originally bought the voucher.
      Instead it would probably be sent to your Nymbox but it COULD NOT BE
 PROVEN that it was, since OT currently
      can't prove NOTICE!! Nevertheless, in the meantime, OT Server should still
 drop a notice in the Nymbox
      of the original sender which is basically a "voucher receipt" (containing
 the voucher but interpreted
      as a receipt and not as a payment instrument.)
        How about this===> when such a receipt is received, instead of moving it
 to the payments inbox like
      we would with invoices/cheques/purses we'll just move it straight to the
 record box instead.

 All of the above needs to happen inside OT, since there are many places where
 it's the only appropriate
 place to take the necessary action. (Script cannot.)

 */

// So far I haven't needed this yet, since sent and received payments already
// handle moving
// payments-inbox receipts to the record box, and moving outpayments instruments
// to the
// record box (built into OT.) But finally a case occured: the instrument is
// expired, so OT
// will never move it, since OT can never deposit it, in the case of incoming
// payments, and
// in the case of outgoing payments, clearly the recipient never deposited it,
// or I would have
// gotten a receipt by now and it would have already been cleared out of my
// outpayments box.
// Since neither of those cases will ever happen, for an expired instrument,
// then how in the heck
// do I get that damned instrument out of my outpayments / payments inbox, and
// moved over to
// the record box so the client software can deal with it? Answer: this API
// call: OT_API::RecordPayment
//
// ONE MORE THING: Let's say I sent a cheque and it expired. The recipient never
// cashed it.
// At this point, I am SAFE to harvest the transaction number(s) back off of the
// cheque. After
// all, it was never cashed, right? And since it's now expired, it never WILL be
// cashed, right?
// Therefore I NEED to harvest the transaction number back from the expired
// instrument, so I can
// use it again in the future and eventually get it closed out.
//
// UPDATE: This should now also work for smart contracts and payment plans.
//
bool OT_API::RecordPayment(
    const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
    bool bIsInbox,  // true == payments inbox. false == outpayments box.
    int32_t nIndex, // removes payment instrument (from payments inbox or
                    // outpayments box) and moves to record box.
    bool bSaveCopy) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTLedger* pRecordBox = nullptr;
    OTLedger* pExpiredBox = nullptr;
    OTLedger* pActualBox =
        nullptr; // This points to either pRecordBox or pExpiredBox.
    std::unique_ptr<OTLedger> theRecordBoxAngel;
    std::unique_ptr<OTLedger> theExpiredBoxAngel;
    if (bSaveCopy) {
        pRecordBox = LoadRecordBox(SERVER_ID, USER_ID, USER_ID);
        pExpiredBox = LoadExpiredBox(SERVER_ID, USER_ID);
        theRecordBoxAngel.reset(pRecordBox);
        theExpiredBoxAngel.reset(pExpiredBox);
        if (nullptr == pRecordBox) {
            pRecordBox = OTLedger::GenerateLedger(USER_ID, USER_ID, SERVER_ID,
                                                  OTLedger::recordBox, true);
            if (nullptr == pRecordBox) {
                otErr << __FUNCTION__
                      << ": Unable to load or create record box "
                         "(and thus unable to do anything with it.)\n";
                return false;
            }
            theRecordBoxAngel.reset(pRecordBox);
        }
        if (nullptr == pExpiredBox) {
            pExpiredBox = OTLedger::GenerateLedger(USER_ID, USER_ID, SERVER_ID,
                                                   OTLedger::expiredBox, true);
            if (nullptr == pExpiredBox) {
                otErr << __FUNCTION__
                      << ": Unable to load or create expired box"
                         "(and thus unable to do anything with it.)\n";
                return false;
            }
            theExpiredBoxAngel.reset(pExpiredBox);
        }
    }
    pActualBox = pRecordBox;
    OTLedger* pPaymentInbox = nullptr;
    std::unique_ptr<OTLedger> thePaymentBoxAngel;

    bool bIsExpired = false;

    // first block:
    OTTransaction* pTransaction = nullptr;
    std::unique_ptr<OTTransaction> theTransactionAngel;

    // second block:
    std::unique_ptr<OTMessage> theMessageAngel;

    bool bRemoved = false, bNeedToSaveTheNym = false;

    if (bIsInbox) {
        pPaymentInbox = LoadPaymentInbox(SERVER_ID, USER_ID);
        thePaymentBoxAngel.reset(pPaymentInbox);
        if (nullptr == pPaymentInbox) {
            otErr << __FUNCTION__
                  << ": Unable to load payment inbox "
                     "(and thus unable to do anything with it.)\n";
            return false;
        }
        if ((nIndex < 0) || (nIndex >= pPaymentInbox->GetTransactionCount())) {
            otErr << __FUNCTION__
                  << ": Unable to find transaction in payment inbox "
                     "based on index " << nIndex << ". (Out of bounds.)\n";
            return false;
        }
        pTransaction = pPaymentInbox->GetTransactionByIndex(nIndex);

        if (nullptr == pTransaction) {
            otErr << __FUNCTION__
                  << ": Unable to find transaction in payment inbox "
                     "based on index " << nIndex << ".\n";
            return false;
        }
        std::unique_ptr<OTPayment> pPayment(
            GetInstrument(*pNym, nIndex, *pPaymentInbox));

        pPayment->IsExpired(bIsExpired);

        if (bIsExpired) pActualBox = pExpiredBox;
        // Remove it from the payments inbox...
        //
        const int64_t lTransactionNum = pTransaction->GetTransactionNum();

        if (!pPaymentInbox->DeleteBoxReceipt(lTransactionNum)) {
            otErr << __FUNCTION__
                  << ": Failed trying to delete the box receipt for a "
                     "transaction being removed from the payment inbox: "
                  << lTransactionNum << "\n";
        }
        bRemoved = pPaymentInbox->RemoveTransaction(
            lTransactionNum, false); // bDeleteIt=true by default. We pass false
                                     // since we are moving it to another box.
                                     // Note that we still need to save
                                     // pPaymentInbox somewhere below, assuming
                                     // it's all successful.
        theTransactionAngel.reset(
            pTransaction); // If below we put pTransaction onto the Record Box,
                           // then we have to set this to nullptr.

        // NOTE: pTransaction is still good, below this point, and will be
        // cleaned up automatically
        // whenever we exit this function.

        // Anything else?
        // Note: no need to harvest transaction number for incoming payments.
        // But for outgoing (see below) then harvesting becomes an issue.

    }
    else // Outpayments box (which is not stored in an OTLedger like payments
           // inbox, but rather, is stored similarly to outmail.)
    {
        //
        if ((nIndex < 0) || (nIndex >= pNym->GetOutpaymentsCount())) {
            otErr << __FUNCTION__
                  << ": Unable to find payment in outpayment box based "
                     "on index " << nIndex << ". (Out of bounds.)\n";
            return false;
        }
        OTMessage* pMessage = pNym->GetOutpaymentsByIndex(nIndex);

        if (nullptr == pMessage) {
            otErr << __FUNCTION__
                  << ": Unable to find payment message in outpayment "
                     "box based on index " << nIndex << ".\n";
            return false;
        }
        OTString strInstrument;
        if (!pMessage->m_ascPayload.GetString(strInstrument)) {
            otErr << __FUNCTION__
                  << ": Unable to find payment instrument in outpayment "
                     "message at index " << nIndex << ".\n";
            return false;
        }
        OTPayment thePayment(strInstrument);

        if (thePayment.IsValid() && thePayment.SetTempValues()) {
            // EXPIRED?
            //
            thePayment.IsExpired(bIsExpired);

            if (bIsExpired) pActualBox = pExpiredBox;
            // Anything but a purse?
            //
            int64_t lPaymentOpeningNum = 0;
            int64_t lPaymentTransNum = 0;
            if (thePayment.GetOpeningNum(lPaymentOpeningNum,
                                         USER_ID)) // cheques, invoices,
                                                   // vouchers, smart contracts,
                                                   // payment plans.
            {
                // We we-grab the transaction number at this time. That way if
                // it's a transaction num that
                // belongs to some other Nym (and is different than our own
                // opening number) then we will
                // get the different number here.
                //
                if (!thePayment.GetTransactionNum(lPaymentTransNum)) {
                    otErr << __FUNCTION__
                          << ": Should never happen! "
                             "Failed to get transaction num from payment "
                             "RIGHT AFTER succeeded getting opening "
                             "num.\n";
                }
                // However, if it IS a different number, in the case of smart
                // contracts and payment plans,
                // we then change it BACK to the opening number again. Read the
                // next comment for details why.
                //
                bool bIsRecurring = false;

                if ((OTPayment::PAYMENT_PLAN == thePayment.GetType()) ||
                    (OTPayment::SMART_CONTRACT == thePayment.GetType())) {
                    bIsRecurring = true;
                    lPaymentTransNum = lPaymentOpeningNum;
                    // We do this because the ACTUAL transaction number on a
                    // smart contract or payment plan
                    // might be different that THIS Nym's opening number (it
                    // might be some other Nym's opening
                    // number.) But even if that's the case, we still want to
                    // harvest THIS Nym's opening number,
                    // not the other Nym's opening number. So for these
                    // instruments, we set the "transaction number"
                    // to be THIS Nym's opening number. (Versus just saying, "Oh
                    // the trans number is for some other
                    // Nym, so just ignore this" which would cause us to not
                    // harvest the numbers for THIS Nym that
                    // we probably SHOULD be harvesting.
                }

                // See what account the payment instrument is drawn from.
                // Is it mine?
                // If so, load up the inbox and see if there are any related
                // receipts inside.
                // If so, do NOT harvest the transaction numbers from the
                // instrument.
                // Otherwise, harvest them. (The instrument hasn't been redeemed
                // yet.)
                // Also, use the transaction number on the instrument to see if
                // it's signed out to me.
                //
                // Hmm: If the instrument is definitely expired, and there's
                // definitely nothing in the asset
                // accountinbox, then I can DEFINITELY harvest it back.
                //
                // But if the instrument is definitely NOT expired, and the
                // transaction # IS signed out to ME,
                // then I can't just harvest the numbers, since the original
                // recipient could still come through
                // and deposit that cheque. So in this case, I would HAVE to
                // cancel the transaction, and then
                // such cancellation would automatically harvest while
                // processing the successful server reply.
                //
                // Therefore make sure not to move the instrument here, unless
                // it's definitely expired.
                // Whereas if it's not expired, then the API must cancel it with
                // the server, and can't simply
                // come in here and move/harvest it. So this function can only
                // be for expired transactions or
                // those where the transaction number is no longer issued. (And
                // in cases where it's expired but
                // STILL issued, then it definitely DOES need to harvest.)
                //

                bool bShouldHarvestPayment = false;
                bool bNeedToLoadAssetAcctInbox = false;
                OTIdentifier theSenderUserID, theSenderAcctID;

                bool bPaymentSenderIsNym = false;
                bool bFromAcctIsAvailable = false;
                if (thePayment.IsVoucher()) {
                    bPaymentSenderIsNym =
                        (thePayment.GetRemitterUserID(theSenderUserID) &&
                         pNym->CompareID(theSenderUserID));
                    bFromAcctIsAvailable =
                        thePayment.GetRemitterAcctID(theSenderAcctID);
                }
                else {
                    bPaymentSenderIsNym =
                        (thePayment.GetSenderUserID(theSenderUserID) &&
                         pNym->CompareID(theSenderUserID));
                    bFromAcctIsAvailable =
                        thePayment.GetSenderAcctID(theSenderAcctID);
                }
                if (bPaymentSenderIsNym || // true for cheques as well as
                                           // vouchers. (We grab the remitter
                                           // above, for vouchers.)
                    bIsRecurring) // false for cheques/vouchers; true for
                                  // payment plans and smart contracts.
                {
                    // NOTE: With bPaymentSenderIsNym, we know pNym owns the
                    // transaction number on the cheque.
                    // NOTE: with bIsRecurring, we know pNym is one of the
                    // parties of the smart contract.
                    //       (Since we found an opening number for pNym on it.)

                    const OTString strServerID(SERVER_ID);

                    // If the transaction # isn't signed out to me, then there's
                    // no need to check the inbox
                    // for any receipts, since those would have to have been
                    // already closed out, in order for
                    // the number not to be signed out to me anymore.
                    //
                    // Therefore let's check that first, before bothering to
                    // load the inbox.
                    //
                    if (pNym->VerifyTentativeNum(
                            strServerID, lPaymentTransNum)) // If I'm in the
                                                            // middle of trying
                                                            // to sign it out...
                    {
                        otErr << __FUNCTION__
                              << ": Error: Why on earth is this "
                                 "transaction number (" << lPaymentTransNum
                              << ") on an outgoing "
                                 "payment instrument, if it's still on my "
                                 "'Tentative' list? If I haven't even "
                                 "signed out that number, how did I send "
                                 "an instrument to someone else with that "
                                 "number on it?\n";
                        return false;
                    }
                    bool bIsIssued =
                        pNym->VerifyIssuedNum(strServerID, lPaymentTransNum);

                    // If pNym is the sender AND the payment instrument IS
                    // expired.
                    //
                    if (bIsExpired) {
                        if (bIsIssued) // ...and if this number is still signed
                                       // out to pNym...
                        {
                            // If the instrument is definitely expired, and its
                            // number is still issued to pNym, and
                            // there's definitely no related chequeReceipts in
                            // the asset acocunt inbox, then I
                            // can DEFINITELY harvest it back. After all, it
                            // hasn't been used, and since it's
                            // expired, now it CAN'T be used. So might as well
                            // harvest the number back, since we've
                            // established that it's still signed out.
                            //
                            // You might ask, but what if there IS a
                            // chequeReceipt in the asset account inbox? How
                            // does that change things? The answer is, because
                            // the transaction # might still be signed
                            // out to me, even in a case where the payment
                            // instrument is expired, but a chequeReceipt
                            // still IS present! How so? Simple: I sent him the
                            // cheque, he cashed it. It's in my outpayments
                            // still, because his chequeReceipt is in my inbox
                            // still, and hasn't been processed out.
                            // Meanwhile I wait a few weeks, and then the
                            // instrument, meanwhile, expires. It's already
                            // been processed, so it doesn't matter that it's
                            // expired. But nonetheless, according to the
                            // dates affixed to it, it IS expired, and the
                            // receipt IS present. So this is clearly a
                            // realistic and legitimate case for our logic to
                            // take into account. When this happens, we
                            // should NOT harvest the transaction # back when
                            // recording the payment, because that number
                            // has been used already!
                            //
                            // That, in a nutshell, is why we have to load the
                            // inbox and see if that receipt's there,
                            // to MAKE SURE whether the instrument was
                            // negotiated already, before it expired, even
                            // though
                            // I haven't accepted the receipt and closed out the
                            // transaction # yet, because it impacts
                            // our decision of whether or not to harvest back
                            // the number.
                            //
                            // The below two statements are interpreted based on
                            // this logic (see comment for each.)
                            //
                            bShouldHarvestPayment = true; // If NO
                            // chequeReceipt/paymentReceipt/finalReceipt
                            // in inbox, definitely SHOULD harvest
                            // back the trans # (since the cheque's
                            // expired and could never otherwise be
                            // closed out)...
                            bNeedToLoadAssetAcctInbox = true; // ...But if
                            // chequeReceipt/paymentReceipt/finalReceipt
                            // IS in inbox, definitely should NOT
                            // harvest back the # (since it's already
                            // been used.)
                            //
                            // =====> Therefore bNeedToLoadAssetAcctInbox is a
                            // caveat, which OVERRIDES bShouldHarvestPayment.
                            // <=====
                        }
                        else // pNym is sender, payment instrument IS expired,
                               // and most importantly: the transaction # is no
                               // longer signed out
                        {      // to pNym. Normally the closing of the # (by
                               // accepting whatever its related receipt was)
                               // should
                               // have already
                            // removed the outpayment, so we are cleared here to
                            // go ahead and remove it.
                            //
                            bShouldHarvestPayment =
                                false; // The # isn't signed out anymore, so we
                                       // don't want to harvest it (which would
                                       // cause the wallet to try and use it
                                       // again -- but we don't want that, since
                                       // we can't be using numnbers that aren't
                                       // even signed out to us!)
                            bNeedToLoadAssetAcctInbox =
                                false; // No need to check the inbox since the #
                                       // isn't even signed out anymore. Even if
                                       // some related receipt was in the inbox
                                       // (for some non-cheque instrument, say)
                                       // we'd still never need to harvest it
                                       // back for re-use, since it's not even
                                       // signed out to us anymore, and we can
                                       // only use numbers that are signed out
                                       // to us.
                        }
                    }    // if bIsExpired.
                    else // Not expired. pNym is the sender but the payment
                         // instrument is NOT expired.
                    {
                        // Remember that the transaction number is still signed
                        // out to me until I accept that
                        // chequeReceipt. So whether the receipt is there or
                        // not, the # will still be signed
                        // out to me. But if there's no receipt yet in my inbox,
                        // that means the cheque hasn't
                        // been cashed yet. And THAT means I still have time to
                        // cancel it. I can't just discard
                        // the payment instrument, since its trans# would still
                        // need to be harvested if it's not
                        // being cancelled (so as to get it closed out on some
                        // other instrument, presumably), but
                        // I can't just harvest a number when there's some
                        // instrument still floating around out
                        // there, with that same number already on it! I HAVE to
                        // cancel it.
                        //
                        // If I discard it without harvesting, and if the
                        // recipient never cashes it, then that
                        // transaction # will end up signed out to me FOREVER
                        // (bad thing.) So I would have to
                        // cancel it first, in order to discard it. The server's
                        // success reply to my cancel
                        // would be the proper time to discard the old
                        // outpayment. Until I see that, how do I know
                        // if I won't need it in the future, for harvesting
                        // back?
                        //
                        if (bIsIssued) // If this number is still signed out to
                                       // pNym...
                        {
                            // If the instrument is definitely NOT expired, and
                            // the transaction # definitely IS issued to ME,
                            // then I can't just harvest the numbers, since the
                            // original recipient could still come through
                            // and deposit that cheque.  So in this case, I
                            // would HAVE to cancel the transaction first, and
                            // then
                            // such cancellation could sign a new transaction
                            // statement with that # removed from my list of
                            // "signed out" numbers. Only then am I safe from
                            // the cheque being cashed by the recipient,
                            // and only then could I even think about harvesting
                            // the number back--that itself being unnecessary,
                            // since the transaction # would then be
                            // cancelled/closed and thus would eliminate any
                            // need of
                            // harvesting it (since now I will never use it.)
                            //
                            // Also, if I DON'T cancel it, then I don't want to
                            // remove it from the outpayments box, because
                            // it will be removed automatically whenever the
                            // cheque is eventually cashed, and until/unless
                            // that
                            // happens, I need to keep it around for potential
                            // harvesting or cancellation. Therefore I can't
                            // discard the instrument either--I need to keep it
                            // in the outpayments box for now, in case it's
                            // needed later.
                            //
                            otOut << __FUNCTION__
                                  << ": This outpayment isn't expired "
                                     "yet, and the transaction number "
                                     "(" << lPaymentTransNum
                                  << ") is still signed out. "
                                     "(Skipping moving it to record box "
                                     "-- it will be moved automatically "
                                     "once you cancel the transaction "
                                     "or the recipient deposits it.)\n";
                            return false;
                        }
                        else // The payment is NOT expired yet, but most
                               // importantly, its transaction # is NOT signed
                               // out to pNym anymore.
                        {      // Normally the closing of the # (by accepting
                               // whatever its related receipt was) should have
                               // already
                            // removed the outpayment by now, so we are cleared
                            // here to go ahead and remove it.
                            //
                            bShouldHarvestPayment =
                                false; // The # isn't signed out anymore, so we
                                       // don't want to harvest it (which would
                                       // cause the wallet to try and use it
                                       // again.)
                            bNeedToLoadAssetAcctInbox =
                                false; // No need to check the inbox since the #
                                       // isn't even signed out anymore and
                                       // we're certainly not interested in
                                       // harvesting it if it's not even signed
                                       // out to us.
                        }
                    } // !bIsExpired
                }     // sender is pNym

                // TODO: Add OPTIONAL field to Purse: "Remitter".
                // This way the sender has the OPTION to attach his ID "for the
                // record" even though
                // a cash transaction HAS NO "SENDER."
                //
                // This would make it convenient for a purse to, for example,
                // create a second copy
                // of the cash, encrypted to the remitter's public key. This is
                // important, since the
                // if the remitter has the cash in his outpayment's box, he will
                // want a way to recover
                // it if his friend returns and says, "I lost that USB key! Do
                // you still have that cash?!?"
                //
                // In fact we may want to use the field for that purpose,
                // WHETHER OR NOT the final sent
                // instrument actually includes the remitter's ID.

                // If the SenderUserID on this instrument isn't Nym's ID (as in
                // the case of vouchers),
                // or isn't even there (as in the case of cash) then why is it
                // in Nym's payment outbox?
                // Well, maybe the recipient of your voucher, lost it. You still
                // need to be able to
                // get another copy for him, or get it refunded (if you are
                // listed as the remitter...)
                // Therefore you keep it in your outpayments box "just in case."
                // In the case of cash,
                // the same could be true.
                //
                // In a way it doesn't matter, since eventually those
                // instruments will expire and then
                // they will be swept into the record box with everything else
                // (probably by this function.)
                //
                // But what if the instruments never expire? Say a voucher with
                // a very very int64_t expiration
                // date? It's still going to sit there, stuck in your
                // outpayments box, even though the
                // recipient have have cashed it int64_t, int64_t ago! The only
                // way to get rid of it is to have
                // the server send you a notice when it's cashed, which is only
                // possible if your ID is
                // listed as the remitter. (Otherwise the server wouldn't know
                // who to send the notice to.)
                //
                // Further, there's no PROVING whether the server sent you
                // notice for anything -- whether
                // you are listed as the remitter or not, the server could
                // choose to LIE and just NOT TELL
                // YOU that the voucher was cashed. How would you know the
                // difference? Thus "Notice" is
                // an important problem, peripheral to OT. Balances are safe
                // from any change without a
                // proper signed and authorized receipt--that is a powerful
                // strength of OT--but notice
                // cannot be proven.
                //
                // If the remitter has no way to prove that the recipient
                // actually deposited the cheque,
                // (even though most servers will of course provide this, they
                // cannot PROVE that they
                // provided it) then what good is the instrument to the
                // remitter? What good is such a
                // cashier's cheque? Well, the voucher is still in my
                // outpayments, so I can see the transaction
                // number on it, and then I should be able to query the server
                // and see which transaction
                // numbers are signed out to it. In which case a simple server
                // message (available to all
                // users) will return the numbers signed out to the server and
                // thus I can see whether or
                // not the number on the voucher is still valid. If it's not,
                // the server reply to that
                // message would be the appropriate place to move the outpayment
                // to the record box.
                //
                // What if we don't want to have these transaction numbers
                // signed out to the server at all?
                // Maybe we use numbers signed out to the users instead. Then
                // when the voucher is cashed,
                // instead of checking the server's list of issued transaction
                // numbers to see if the voucher
                // is valid, we would be checking the remitter's list instead.
                // And thus whenever the voucher
                // is cashed, we would have to drop a voucherReceipt in the
                // REMITTER's inbox (and nymbox)
                // so he would know to discard the transaction number.
                //
                // This would require adding the voucherReceipt, and would
                // restrict the use of vouchers to
                // those people who have an asset account (though that's
                // currently the only people who can
                // use them now anyway, since vouchers are withdrawn from
                // accounts) but it would eliminate
                // the need for the server nym to store all the transaction
                // numbers for all the open vouchers,
                // and it would also eliminate the problem of "no notice" for
                // the remitters of vouchers.
                // They'd be guaranteed, in fact, to get notice, since the
                // voucherReceipt is what removes
                // the transaction number from the user's list of signed-out
                // transaction numbers (and thus
                // prevents anyone from spending the voucher twice, which the
                // server wants to avoid at all
                // costs.) Thus if the server wants to avoid having a voucher
                // spent twice, then it needs to
                // get that transaction number off of my list of numbers, and it
                // can't do that without putting
                // a receipt in my inbox to justify the removal of the number.
                // Otherwise my receipt verifications
                // will start failing and I'll be unable to do any new
                // transactions, and then the server will
                // have to explain why it removed a transaction number from my
                // list, even though it still was
                // on my list during the last transaction statement, and even
                // though there's no new receipt in
                // my inbox to justify removing it.
                //
                // Conclusion, DONE: vouchers WITH a remitter acct, should store
                // the remitter's user AND acct IDs,
                // and should use a transaction # that's signed out to the
                // remitter (instead of the server) and
                // should drop a voucherReceipt in the remitter's asset account
                // inbox when they are cashed.
                // These vouchers are guaranteed to provide notice to the
                // remitter.
                //
                // Whereas vouchers WITHOUT a remitter acct should store the
                // remitter's user ID (or not), but should
                // NOT store the remitter's acct ID, and should use a
                // transaction # that's signed out to the server,
                // and should drop a notice in the Nymbox of the remitter IF his
                // user ID is available, but it should
                // be understood that such notice is a favor the server is
                // doing, and not something that's PROVABLE
                // as in the case of the vouchers in the above paragraph.
                //
                // This is similar to smart contracts, which can only be
                // activated by a party who has an
                // asset account, so he has somewhere to receive the
                // finalReceipt for that contract. (And thus
                // close out the transcation number he used to open it...)
                //
                // Perhaps we'll just offer both types of vouchers, and just let
                // users choose which they are willing
                // to pay for, and which trade-off is most palatable to them
                // (having to have an asset account to
                // get notice, or instead verifying the voucher's spent status
                // based on some publicly-available
                // listing of the transaction #'s currently signed out to the
                // server.)
                //
                // In the case of having transaction #'s signed out to the
                // server, perhaps the server's internal storage
                // of these should be paired each with the NymID of the owner
                // nym for that number, just so no one
                // would ever have any incentive to try and use one of those
                // numbers on some instrument somehow, and
                // also so that the server wouldn't necessarily have to post the
                // entire list of numbers, but just
                // rather give you the ones that are relevant to you (although
                // the entire list may have to be posted
                // in some public way in any case, for notice reasons.)
                //
                // By this point, you may be wondering, but what does all this
                // have to do with the function
                // we're in now? Well... in the below block, with a voucher,
                // pNym would NOT be the sender.
                // The voucher would still be drawn off a server account. But
                // nevertheless, pNym might still be
                // the owner of the transaction # for that voucher (assuming I
                // change OT around to use the above
                // system, which I will have to do if I want provable notice for
                // vouchers.) And if pNym is the
                // owner of that number, then he will want the option later of
                // refunding it or re-issuing it,
                // and he will have to possibly load up his inbox to make sure
                // there's no voucherReceipt in it,
                // etc. So for now, the vouchers will be handled by the below
                // code, but in the future, they might
                // be moved to the above code.
                //
                //
                else // pNym is not the sender.
                {
                    // pNym isn't even the "sender" (although he is) but since
                    // it's cash or voucher,
                    // he's not waiting on any transaction number to close out
                    // or be harvested.
                    // (In the case of cash, in fact, we might as well just put
                    // it straight in the records
                    // and bypass the outpayments box entirely.) But this
                    // function can sweep it into there
                    // all in due time anyway, once it expires, so it seems
                    // harmless to leave it there before
                    // then. Plus, that way we always know which tokens are
                    // still potentially exchangeable,
                    // for cases where the recipient never cashed them.
                    //
                    if (bIsExpired) {
                        // pNym is NOT the sender AND the payment instrument IS
                        // expired.
                        // Therefore, say in the case of sent vouchers and sent
                        // cash, there
                        // may have been some legitimate reason for keeping them
                        // in outpayments
                        // up until this point, but now that the instrument is
                        // expired, might as
                        // well get it out of the outpayments box and move it to
                        // the record box.
                        // Let the client software do its own historical
                        // archiving.
                        //
                        bShouldHarvestPayment =
                            false; // The # isn't signed out to pNym, so we
                                   // don't want to harvest it (which would
                                   // cause the wallet to try and use it even
                                   // though it's signed out to someone else --
                                   // bad.)
                        bNeedToLoadAssetAcctInbox =
                            false; // No need to check the inbox since the #
                                   // isn't even signed out to pNym and we're
                                   // certainly not interested in harvesting it
                                   // if it's not even signed out to us. (Thus
                                   // no reason to check the inbox.)
                    }
                    else // pNym is NOT the sender and the payment instrument
                           // is NOT expired.
                    {
                        // For example, for a sent voucher that has not expired
                        // yet.
                        // Those we'll keep here for now, until some server
                        // notice is
                        // received, or the instrument expires. What if we need
                        // to re-issue
                        // the cheque to the recipient who lost it? Or what if
                        // we want to
                        // cancel it before he tries to cash it? Since it's not
                        // expired yet,
                        // it's wise to keep a copy in the outpayments box for
                        // now.
                        //
                        otOut << __FUNCTION__
                              << ": This outpayment isn't expired yet.\n";
                        return false;
                    }
                }
                //
                bool bFoundReceiptInInbox = false;
                OTSmartContract* pSmartContract = nullptr;
                OTPaymentPlan* pPlan = nullptr;
                //
                // In certain cases (logic above) it is determined that we have
                // to load the
                // asset account inbox and make sure there aren't any
                // chequeReceipts there,
                // before we go ahead and harvest any transaction numbers.
                //
                if (bNeedToLoadAssetAcctInbox &&
                    (bFromAcctIsAvailable || bIsRecurring)) {
                    bool bIsSmartContract = false;
                    if (bIsRecurring) {
                        std::unique_ptr<OTTrackable> pTrackable(
                            thePayment.Instantiate());
                        if (nullptr == pTrackable) {
                            OTString strPaymentContents;
                            thePayment.GetPaymentContents(strPaymentContents);
                            otErr << __FUNCTION__
                                  << ": Failed instantiating OTPayment "
                                     "containing:\n" << strPaymentContents
                                  << "\n";
                            return false;
                        }
                        pSmartContract =
                            dynamic_cast<OTSmartContract*>(pTrackable.get());
                        pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable.get());
                        if (nullptr != pSmartContract) {
                            bIsSmartContract = true; // In this case we have to
                                                     // loop through all the
                                                     // accounts on the smart
                                                     // contract...
                        }
                        else if (nullptr != pPlan) {
                            // Payment plan is a funny case.
                            // The merchant (RECIPIENT aka payee) creates the
                            // payment plan, and then he SENDS
                            // it to the customer (SENDER aka payer). From
                            // there, the customer ACTIVATES it on
                            // the server. BOTH could potentially have it in
                            // their outpayments box. In one case.
                            // the Nym is the "sender" and in another case, he's
                            // the "recipient."
                            // (So we need to figure out which, and set the
                            // account accordingly.)
                            //
                            if (USER_ID == pPlan->GetRecipientUserID())
                                theSenderAcctID = pPlan->GetRecipientAcctID();
                            else if (USER_ID == pPlan->GetSenderUserID())
                                theSenderAcctID = pPlan->GetSenderAcctID();
                            else
                                otErr << __FUNCTION__
                                      << ": ERROR: Should never happen: "
                                         "USER_ID didn't match this "
                                         "payment plan for sender OR "
                                         "recipient. "
                                         "(Expected one or the other.)\n";
                        }
                    }
                    if (bIsSmartContract) // In this case we have to loop
                                          // through all the accounts on the
                                          // smart contract... We have to
                    { // check the inbox on each, to make sure there aren't any
                        // related paymentReceipts or final receipts.
                        const int32_t nPartyCount =
                            pSmartContract->GetPartyCount();

                        for (int32_t nCurrentParty = 0;
                             nCurrentParty < nPartyCount; ++nCurrentParty) {
                            OTParty* pParty =
                                pSmartContract->GetPartyByIndex(nCurrentParty);
                            OT_ASSERT(nullptr != pParty);
                            if (nullptr != pParty) {
                                const int32_t nAcctCount =
                                    pParty->GetAccountCount();

                                for (int32_t nCurrentAcct = 0;
                                     nCurrentAcct < nAcctCount;
                                     ++nCurrentAcct) {
                                    OTPartyAccount* pPartyAcct =
                                        pParty->GetAccountByIndex(nCurrentAcct);
                                    OT_ASSERT(nullptr != pPartyAcct);
                                    if (nullptr != pPartyAcct) {
                                        OTAgent* pAgent =
                                            pPartyAcct->GetAuthorizedAgent();

                                        // If pNym is a signer for pPartyAcct,
                                        // then we need to check pPartyAcct's
                                        // inbox
                                        // to make sure there aren't any
                                        // paymentReceipts or finalReceipts
                                        // lingering in there...
                                        //
                                        if (pAgent->IsValidSigner(*pNym)) {
                                            const OTString& strAcctID =
                                                pPartyAcct->GetAcctID();
                                            const OTIdentifier theAcctID(
                                                strAcctID);

                                            OTLedger theSenderInbox(
                                                USER_ID, theAcctID, SERVER_ID);

                                            const bool
                                                bSuccessLoadingSenderInbox =
                                                    (theSenderInbox
                                                         .LoadInbox() &&
                                                     theSenderInbox
                                                         .VerifyAccount(*pNym));
                                            if (bSuccessLoadingSenderInbox) {
                                                // Loop through the inbox and
                                                // see if there are any receipts
                                                // for lPaymentTransNum inside.
                                                //
                                                if (GetPaymentReceipt(
                                                        theSenderInbox
                                                            .GetTransactionMap(),
                                                        lPaymentTransNum,
                                                        nullptr) ||
                                                    theSenderInbox
                                                        .GetFinalReceipt(
                                                            lPaymentTransNum)) {
                                                    bFoundReceiptInInbox = true;
                                                    break;
                                                }
                                                // else we didn't find a receipt
                                                // in the asset account inbox,
                                                // which means we are safe to
                                                // harvest.
                                            }
                                            // else unable to load inbox. Maybe
                                            // it's empty, never been used
                                            // before. i.e. it doesn't even
                                            // exist.
                                        } // pNym is valid signer for agent
                                    }     // nullptr != pPartyAccount
                                }         // loop party accounts.

                                if (bFoundReceiptInInbox) break;
                            }
                        } // loop parties
                    }     // smart contract
                    else  // not a smart contract. (It's a payment plan or a
                          // cheque, most likely.)
                    {
                        OTLedger theSenderInbox(USER_ID, theSenderAcctID,
                                                SERVER_ID);

                        const bool bSuccessLoadingSenderInbox =
                            (theSenderInbox.LoadInbox() &&
                             theSenderInbox.VerifyAccount(*pNym));
                        if (bSuccessLoadingSenderInbox) {
                            // Loop through the inbox and see if there are any
                            // receipts for lPaymentTransNum inside.
                            // Technically this would have to be a
                            // chequeReceipt, or possibly a voucherReceipt if I
                            // add
                            // that (see giant comment above.)
                            //
                            // There are other instrument types but only a
                            // cheque, at this point, would be in my outpayments
                            // box AND could have a receipt in my asset account
                            // inbox. So let's see if there's a chequeReceipt
                            // in there that corresponds to lPaymentTransNum...
                            //
                            OTTransaction* pChequeReceipt =
                                theSenderInbox.GetChequeReceipt(
                                    lPaymentTransNum); // cheque

                            if (nullptr != pChequeReceipt) {
                                bFoundReceiptInInbox = true;
                            }
                            // No cheque receipt? Ok let's see if there's a
                            // paymentReceipt or finalReceipt (for a payment
                            // plan...)
                            else if (GetPaymentReceipt(
                                         theSenderInbox.GetTransactionMap(),
                                         lPaymentTransNum, nullptr) ||
                                     theSenderInbox.GetFinalReceipt(
                                         lPaymentTransNum)) // payment plan.
                            {
                                bFoundReceiptInInbox = true;
                            }
                            // else we didn't find a receipt in the asset
                            // account inbox, which means we are safe to
                            // harvest.
                        }
                        // else unable to load inbox. Maybe it's empty, never
                        // been used before. i.e. it doesn't even exist.
                    } // not a smart contract
                } // if (bNeedToLoadAssetAcctInbox && (bFromAcctIsAvailable ||
                  // bIsRecurring))
                // If we should harvest the transaction numbers,
                // AND if we don't need to double-check that against the asset
                // inbox to make sure the receipt's not there,
                // (or if we do, that it was a successful double-check and the
                // receipt indeed is not there.)
                //
                if (bShouldHarvestPayment &&
                    ((!bNeedToLoadAssetAcctInbox) ||
                     (bNeedToLoadAssetAcctInbox && !bFoundReceiptInInbox))) {
                    // Harvest the transaction number(s).
                    //
                    if (nullptr != pSmartContract) {
                        pSmartContract->HarvestOpeningNumber(*pNym);
                        pSmartContract->HarvestClosingNumbers(*pNym);
                    }
                    else if (nullptr != pPlan) {
                        pPlan->HarvestOpeningNumber(*pNym);
                        pPlan->HarvestClosingNumbers(*pNym);
                    }
                    else {
                        pNym->ClawbackTransactionNumber(
                            SERVER_ID, lPaymentTransNum, false); // bSave=false
                    }

                    bNeedToSaveTheNym = true;

                    // Note, food for thought: IF the receipt had popped into
                    // your asset inbox on the server
                    // side, since the last time you downloaded your inbox, then
                    // you could be making the wrong
                    // decision here, and harvesting a number that's already
                    // spent. (You just didn't know it yet.)
                    //
                    // What happens in that case, when I download the inbox
                    // again? A new receipt is there, and a
                    // transaction # is used, which I thought was still
                    // available for use? (Since I harvested it?)
                    // The appearance of the receipt in the inbox, as long as
                    // properly formed and signed by me,
                    // should be enough information for the client side to
                    // adjust its records, because if it
                    // doesn't anticipate this possibility, then it will be
                    // forced to resync entirely, which I want
                    // to avoid in all cases period.
                    //
                    // In this block, we clawed back the number because if
                    // there's no chequeReceipt in the inbox,
                    // then that means the cheque has never been used, since if
                    // I had closed the chequeReceipt out
                    // already, then the transaction number on that cheque would
                    // already have been closed out at
                    // that time.
                    // We only clawback for expired instruments, where the
                    // transaction # is still outstanding
                    // and where no receipt is present in the asset acct inbox.
                    // If the instrument is not expired,
                    // then you must cancel it properly. And if the cheque
                    // receipt is in the inbox, then you must
                    // close it properly.
                }
            } // if (thePayment.GetTransactionNum(lPaymentTransNum))
            else if (bSaveCopy && (nullptr != pActualBox) &&
                     thePayment.IsPurse()) {
                OT_ASSERT(nullptr != pActualBox);

                // A purse has no transaction number on itself, and if it's in
                // the outpayment box,
                // it has no transaction number from its ledger, either! It's
                // numberless. So what
                // we do for now is, we use the expiration timestamp for the
                // purse to create its
                // "transaction number" (we also add a billion to it, and then
                // increment it until
                // we are sure it's unused in the box already) for the new
                // receipt we're inserting
                // for this purse into the record box. (Otherwise we'd have to
                // skip this part and
                // we wouldn't save a record of the purse at all.)
                //
                // ALSO NOTE that people shouldn't really be discarding the
                // purse anyway from the outpayment
                // box, since it will ALREADY disappear once it expires.
                // (Presumably you'd want the option to
                // recover it, at any time prior to that point...) But we still
                // must consider that people sent
                // cash and they just want it erased, so...
                //
                time64_t tValidTo = OT_TIME_ZERO;

                if (thePayment.GetValidTo(tValidTo)) {
                    lPaymentTransNum = OTTimeGetSecondsFromTime(tValidTo) +
                                       1000000000; // todo hardcoded. (But
                                                   // should be harmless since
                                                   // this is record box.)

                    // Since we're using a made-up transaction number here,
                    // let's
                    // make sure it's not already being used. If it is, we'll
                    // increment it until nothing is found.
                    //
                    while (nullptr !=
                           pActualBox->GetTransaction(lPaymentTransNum))
                        ++lPaymentTransNum;
                }
                else
                    otErr << __FUNCTION__
                          << ": Failed trying to get 'valid to' from purse.\n";
            }
            // Create the notice to put in the Record Box.
            //
            if (bSaveCopy && (nullptr != pActualBox) &&
                (lPaymentTransNum > 0)) {
                OT_ASSERT(nullptr != pActualBox);

                OTTransaction* pNewTransaction =
                    OTTransaction::GenerateTransaction(
                        *pActualBox, OTTransaction::notice, lPaymentTransNum);

                if (nullptr != pNewTransaction) // The above has an OT_ASSERT
                // within, but I just like to check
                // my pointers.
                {
                    pNewTransaction->SetReferenceToNum(
                        lPaymentTransNum); // referencing myself here. We'll see
                                           // how it works out.
                    pNewTransaction->SetReferenceString(
                        strInstrument); // the cheque, invoice, etc that used to
                                        // be in the outpayments box.

                    pNewTransaction->SignContract(*pNym);
                    pNewTransaction->SaveContract();
                    pTransaction = pNewTransaction;

                    theTransactionAngel.reset(pTransaction);
                }
                else // should never happen
                {
                    const OTString strUserID(USER_ID);
                    otErr << __FUNCTION__
                          << ": Failed while trying to generate "
                             "transaction in order to "
                             "add a new transaction (for a payment "
                             "instrument from the outpayments box) "
                             "to Record Box: " << strUserID << "\n";
                }
            }
        } // if (thePayment.IsValid() && thePayment.SetTempValues())
        //
        // Now we actually remove the message from the outpayments...
        //
        bRemoved = pNym->RemoveOutpaymentsByIndex(
            nIndex, false); // bDeleteIt=true by default
        theMessageAngel.reset(
            pMessage); // Since we chose to keep pMessage alive after removing
                       // it from the outpayments, we set the angel here to make
                       // sure it gets cleaned up later whenever we return out
                       // of this godforsaken function.

        // Anything else?
    } // outpayments box.
    //
    // Okay by this point, whether the payment was in the payments inbox, or
    // whether it was in the outpayments box, either way, it has now been
    // removed
    // from that box. (Otherwise we would have returned already by this point.)
    //
    // It's still safer to explicitly check bRemoved, just in case.
    //
    if (bRemoved) {
        if (bSaveCopy && (nullptr != pActualBox) && (nullptr != pTransaction)) {
            OT_ASSERT(nullptr != pActualBox);

            const bool bAdded = pActualBox->AddTransaction(*pTransaction);

            if (!bAdded) {
                otErr << __FUNCTION__ << ": Unable to add transaction "
                      << pTransaction->GetTransactionNum()
                      << " to record box (after "
                         "tentatively removing from payment "
                      << (bIsInbox ? "inbox" : "outbox")
                      << ", an action that is now canceled.)\n";
                return false;
            }
            else {
                // If successfully added to the record box, then no need
                // anymore to clean it up ourselves.
                theTransactionAngel.release();
            }

            pActualBox->ReleaseSignatures();
            pActualBox->SignContract(*pNym);
            pActualBox->SaveContract();
            if (bIsExpired)
                pActualBox->SaveExpiredBox(); // todo log failure.
            else
                pActualBox->SaveRecordBox(); // todo log failure.

            // Any inbox/nymbox/outbox ledger will only itself contain
            // abbreviated versions of the receipts, including their hashes.
            //
            // The rest is stored separately, in the box receipt, which is
            // created
            // whenever a receipt is added to a box, and deleted after a receipt
            // is removed from a box.
            //
            pTransaction->SaveBoxReceipt(*pActualBox); // todo: log failure
        }
        if (bIsInbox) {
            pPaymentInbox->ReleaseSignatures();
            pPaymentInbox->SignContract(*pNym);
            pPaymentInbox->SaveContract();
            const bool bSavedInbox =
                pPaymentInbox->SavePaymentInbox(); // todo: log failure
            if (!bSavedInbox)
                otOut << "/n" << __FUNCTION__
                      << ": Unable to Save PaymentInbox./n";
        }
        else // outbox
        {
            // Outpayments are currently stored in the Nymfile.
            //
            bNeedToSaveTheNym = true;
        }
        if (bNeedToSaveTheNym) {
            pNym->SaveSignedNymfile(*pNym);
        }
    }
    else {
        otErr << __FUNCTION__ << ": Unable to remove from payment "
              << (bIsInbox ? "inbox" : "outbox") << " based on index " << nIndex
              << ".\n";
        return false;
    }
    //
    return true;
}

bool OT_API::ClearRecord(
    const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
    const OTIdentifier& ACCOUNT_ID,       // USER_ID can be passed here as well.
    int32_t nIndex, bool bClearAll) const // if true, nIndex is ignored.
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    std::unique_ptr<OTLedger> pRecordBox(
        LoadRecordBox(SERVER_ID, USER_ID, ACCOUNT_ID));
    if (nullptr == pRecordBox) {
        pRecordBox.reset(OTLedger::GenerateLedger(
            USER_ID, ACCOUNT_ID, SERVER_ID, OTLedger::recordBox, true));

        if (nullptr == pRecordBox) {
            otErr << __FUNCTION__
                  << ": Unable to load or create record box "
                     "(and thus unable to do anything with it.)\n";
            return false;
        }
    }
    if (bClearAll) {
        pRecordBox->ReleaseTransactions();
        pRecordBox->ReleaseSignatures();
        pRecordBox->SignContract(*pNym);
        pRecordBox->SaveContract();
        pRecordBox->SaveRecordBox();
        return true;
    }
    // Okay, it's not "clear all" but "clear at index" ...
    //
    const int32_t nTransCount = pRecordBox->GetTransactionCount();

    if ((nIndex < 0) || (nIndex >= nTransCount)) {
        otOut << __FUNCTION__
              << ": Index out of bounds (highest allowed index for "
                 "this record box is " << nTransCount - 1 << ".)\n";
        return false;
    }
    OTTransaction* pTransaction = pRecordBox->GetTransactionByIndex(nIndex);
    bool bRemoved = false;

    if (nullptr != pTransaction) {
        const int64_t lTransactionNum = pTransaction->GetTransactionNum();

        if (!pRecordBox->DeleteBoxReceipt(lTransactionNum)) {
            otErr << __FUNCTION__
                  << ": Failed trying to delete the box receipt for a "
                     "transaction being removed from a record box: "
                  << lTransactionNum << "\n";
        }
        bRemoved = pRecordBox->RemoveTransaction(lTransactionNum);
    }
    if (bRemoved) {
        pRecordBox->ReleaseSignatures();
        pRecordBox->SignContract(*pNym);
        pRecordBox->SaveContract();
        pRecordBox->SaveRecordBox();
        return true;
    }
    else {
        const int32_t nTemp = nIndex;
        otOut << __FUNCTION__
              << ": Failed trying to clear a record from the record "
                 "box at index: " << nTemp << "\n";
    }
    return false;
}

// This function assumes you have already downloaded the latest copy of your
// Nymbox,
// and that you have already retrieved theMessageNym from the @createUserAccount
// message, with both objects being loaded and passed as arguments here, ready
// to go.
//
bool OT_API::ResyncNymWithServer(OTPseudonym& theNym, const OTLedger& theNymbox,
                                 const OTPseudonym& theMessageNym) const
{
    OT_ASSERT_MSG(m_bInitialized, "Not initialized; call OT_API::Init first.");
    if (OTLedger::nymbox != theNymbox.GetType()) {
        otErr << "OT_API::ResyncNymWithServer: Error: Expected a Nymbox, "
                 "but you passed in a " << theNymbox.GetTypeString() << ".\n";
        return false;
    }
    if (!theNym.CompareID(theNymbox.GetUserID())) {
        const OTString id1(theNym.GetConstID()), id2(theNymbox.GetUserID());
        otErr << "OT_API::ResyncNymWithServer: Error: NymID of Nym (" << id1
              << ") "
                 "doesn't match NymID on (supposedly) his own Nymbox "
                 "(" << id2 << ").\n";
        return false;
    }
    //    if (!theNym.CompareID(theMessageNym))
    //    {
    //        const OTString id1(theNym.GetConstID()),
    // id2(theMessageNym.GetConstID());
    //        otErr << "OT_API::ResyncNymWithServer: Error: NymID of Nym
    // (" << id1 << ") doesn't match NymID on (supposedly) his server-side
    // doppelganger
    // (" << id2 << ").\n";
    //        return false;
    //    }

    return theNym.ResyncWithServer(theNymbox, theMessageNym);
}

// INCOMING SERVER REPLIES

// YOU are responsible to delete the OTMessage object, once
// you receive the pointer that comes back from this function.
// (It also might return nullptr, if there are none there.)
//
OTMessage* OT_API::PopMessageBuffer(const int64_t& lRequestNumber,
                                    const OTIdentifier& SERVER_ID,
                                    const OTIdentifier& USER_ID) const
{
    OT_ASSERT_MSG((m_bInitialized && (m_pClient != nullptr)),
                  "Not initialized; call OT_API::Init first.");
    OT_ASSERT_MSG(lRequestNumber > 0,
                  "OT_API::PopMessageBuffer: lRequestNumber is less than 1.");

    const OTString strServerID(SERVER_ID), strNymID(USER_ID);

    return m_pClient->GetMessageBuffer().Pop(lRequestNumber, strServerID,
                                             strNymID); // deletes
}

void OT_API::FlushMessageBuffer()
{
    OT_ASSERT_MSG(m_bInitialized && (m_pClient != nullptr),
                  "Not initialized; call OT_API::Init first.");

    m_pClient->GetMessageBuffer().Clear();
}

// OUTOING MESSSAGES

// NOTE: Currently it just stores ALL sent messages, if they were sent (as far
// as it
// can tell.) But the idea for the future is to have messages marked as
// "important"
// and for "only those" to actually be saved here. (Those important msgs are the
// ones
// that we need to track for purposes of harvesting transaction numbers.)

// Do NOT delete the message that is returned here.
// Use the "Remove" call if you want to remove it.
//

OTMessage* OT_API::GetSentMessage(const int64_t& lRequestNumber,
                                  const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& USER_ID) const
{
    OT_ASSERT_MSG((m_bInitialized && (m_pClient != nullptr)),
                  "Not initialized; call OT_API::Init first.");
    OT_ASSERT_MSG(lRequestNumber > 0,
                  "OT_API::GetSentMessage: lRequestNumber is less than 1.");

    const OTString strServerID(SERVER_ID), strNymID(USER_ID);

    return m_pClient->GetMessageOutbuffer().GetSentMessage(
        lRequestNumber, strServerID, strNymID); // doesn't delete.
}

bool OT_API::RemoveSentMessage(const int64_t& lRequestNumber,
                               const OTIdentifier& SERVER_ID,
                               const OTIdentifier& USER_ID) const
{
    OT_ASSERT_MSG(m_bInitialized && (m_pClient != nullptr),
                  "Not initialized; call OT_API::Init first.");
    OT_ASSERT_MSG(lRequestNumber > 0,
                  "OT_API::RemoveSentMessage: lRequestNumber is less than 1.");

    const OTString strServerID(SERVER_ID), strNymID(USER_ID);

    return m_pClient->GetMessageOutbuffer().RemoveSentMessage(
        lRequestNumber, strServerID, strNymID); // deletes.
}

//  Basically, the sent messages queue must store
// messages (by request number) until we know for SURE whether we have a
// success, a failure,
// or a lost/rejected message. That is, until we DOWNLOAD the Nymbox, and thus
// know for SURE
// that a response to a given message is there...or not. Why do we care? For
// making this
// choice:
//
// Messages that DO have a reply are therefore already "in the system" and will
// be handled
// normally--they can be ignored and flushed from the "sent messages" queue.
// Whereas messages
// that do NOT have a reply in the Nymbox (yet are still in the "sent messages"
// queue) can be
// assumed safely to have been rejected at "message level" (before any
// transaction could
// have processed) and the reply must have been dropped on the network, OR the
// server never
// even received the message in the first place. EITHER WAY the trans #s can be
// harvested
// accordingly and then removed from the sent buffer. In a perfect world (read:
// iteration 2)
// these sent messages will be serialized somehow along with the Nym, and not
// just stored in
// RAM like this version does.
//
// Therefore this function will be called only after @getNymbox (in OTClient),
// where each
// replyNotice in the Nymbox will be removed from the sent messages
// individually, and then
// the rest of the sent messages can be flushed
//
//
// OT_API::FlushSentMessages
//
// Make sure to call this directly after a successful @getNymbox.
// (And ONLY at that time.)
//
// This empties the buffer of sent messages.
// (Harvesting any transaction numbers that are still there.)
//
// NOTE: You normally ONLY call this immediately after receiving
// a successful @getNymbox. It's only then that you can see which
// messages a server actually received or not -- which transactions
// it processed (success or fail) vs which transactions did NOT
// process (and thus did NOT leave any success/fail receipt in the
// nymbox.)
//
// I COULD have just flushed myself IN the @getNymbox code (where
// the reply is processed.) But then the developer using the OT API
// would never have the opportunity to see whether a message was
// replied to, and harvest it for himself (say, just before attempting
// a re-try, which I plan to do in the high-level Java API, which is
// why I'm coding it this way.)
//
// This way, he can do that if he wishes, THEN call this function,
// and harvesting will still occur properly, and he will also thus have
// his chance to check for his own replies to harvest before then.
// This all depends on the developer using the API being smart enough
// to call this function after a successful @getNymbox!
//
void OT_API::FlushSentMessages(bool bHarvestingForRetry,
                               const OTIdentifier& SERVER_ID,
                               const OTIdentifier& USER_ID,
                               const OTLedger& THE_NYMBOX) const
{
    OT_ASSERT_MSG(m_bInitialized && (m_pClient != nullptr),
                  "Not initialized; call OT_API::Init first.");
    OTPseudonym* pNym =
        GetNym(USER_ID, __FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pNym) return;
    // Below this point, pNym is a good ptr, and will be cleaned up
    // automatically.
    const OTString strServerID(SERVER_ID), strNymID(USER_ID);
    if ((THE_NYMBOX.GetUserID() != USER_ID) ||
        (THE_NYMBOX.GetPurportedServerID() != SERVER_ID)) {
        const OTString strLedger(THE_NYMBOX);
        otErr << __FUNCTION__ << ": Failure, Bad input data: UserID ("
              << strNymID << ") or ServerID "
                             "(" << strServerID
              << ") failed to match Nymbox:\n\n" << strLedger << "\n\n";
        return;
    }
    // Iterate through the nymbox receipts.
    // If ANY of them is a REPLY NOTICE, then see if the SENT MESSAGE for that
    // SAME
    // request number is in the sent queue.
    // If it IS, REMOVE IT from the sent queue. (Since Nymbox clearly already
    // contains
    // a reply to it, no need to queue it anymore.)
    // At the end of this loop, then FLUSH the sent queue. We KNOW only
    // important
    // (nymbox related) messages should be queued there, and once we see the
    // latest Nymbox,
    // then we KNOW which ones to remove before flushing.
    //
    for (auto& it : THE_NYMBOX.GetTransactionMap()) {
        const OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        if (OTTransaction::replyNotice == pTransaction->GetType()) {
            OTMessage* pMessage =
                m_pClient->GetMessageOutbuffer().GetSentMessage(*pTransaction);

            if (nullptr != pMessage) // It WAS there in my sent buffer!
            {
                // Since it IS in my Nymbox already as a replyNotice,
                // therefore I'm safe to erase it from my sent queue.
                //
                m_pClient->GetMessageOutbuffer().RemoveSentMessage(
                    *pTransaction);
            }
            // else do nothing, must have already removed it.
        }
    }
    // Now that we've REMOVED the sent messages that already have a reply
    // in the Nymbox (and thus CLEARLY do not need harvesting...)
    //
    // Clear all "sent message" cached messages for this server and NymID.
    // (And harvest their transaction numbers back, since we know for a fact
    // the server hasn't replied to them (the reply would be in the Nymbox,
    // which
    // we just got.)
    //
    // UPDATE: We will have to rely on the Developer using the OT API to call
    // OT_API_FlushSentMessages IMMEDIATELY after calling getNymbox and
    // receiving
    // a successful reply. Why? Because that's the only way to give him the
    // chance
    // to see if certain replies are there or not (before they get removed.)
    // That way
    // he can do his own harvesting, do a re-try, etc and then finally when he
    // is done
    // with that, do the flush.
    //
    m_pClient->GetMessageOutbuffer().Clear(
        &strServerID, &strNymID, pNym,
        &bHarvestingForRetry); // FYI: This HARVESTS any sent messages that need
                               // harvesting, before flushing them all.
}

bool OT_API::HaveAlreadySeenReply(const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& USER_ID,
                                  const int64_t& lRequestNumber) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)

    // "Client verifies it has already seen a server reply."
    //  bool OTPseudonym:::VerifyAcknowledgedNum(const OTString & strServerID,
    // const int64_t & lRequestNum);
    //
    const OTString strServerID(SERVER_ID);
    return pNym->VerifyAcknowledgedNum(strServerID, lRequestNumber);
}

// IS BASKET CURRENCY ?
//
// Tells you whether or not a given asset type is actually a basket currency.
//
bool OT_API::IsBasketCurrency(
    const OTIdentifier& BASKET_ASSET_TYPE_ID) const // returns true or false.
{
    // There is an OT_ASSERT_MSG in here for memory failure,
    // but it still might return nullptr if various verification fails.
    OTAssetContract* pContract =
        GetAssetType(BASKET_ASSET_TYPE_ID, __FUNCTION__);
    if (nullptr == pContract) return false;
    // No need to cleanup pContract.
    // Next load the Basket object out of that contract.
    Basket theBasket;

    // todo perhaps verify the basket here, even though I already verified the
    // asset contract itself...
    // Can't never be too sure...
    //
    if (pContract->GetBasketInfo().Exists() &&
        theBasket.LoadContractFromString(pContract->GetBasketInfo())) {
        if (theBasket.Count() > 0) return true;
    }

    return false;
}

// Get Basket Count (of member currency types.)
//
// Returns the number of asset types that make up this basket.
// (Or zero.)
//
int32_t OT_API::GetBasketMemberCount(
    const OTIdentifier& BASKET_ASSET_TYPE_ID) const
{
    // There is an OT_ASSERT_MSG in here for memory failure,
    // but it still might return nullptr if various verification fails.
    OTAssetContract* pContract =
        GetAssetType(BASKET_ASSET_TYPE_ID, __FUNCTION__);
    if (nullptr == pContract) return 0;
    // No need to cleanup pContract.
    // Next load the Basket object out of that contract.
    Basket theBasket;

    // todo perhaps verify the basket here, even though I already verified the
    // asset contract itself...
    // Can't never be too sure...
    //
    if (pContract->GetBasketInfo().Exists() &&
        theBasket.LoadContractFromString(pContract->GetBasketInfo()))
        return theBasket.Count();

    return 0;
}

// Get Basket Member Asset Type
//
// Returns one of the asset types that make up this basket,
// by index, and true.
// (Or false.)
//
bool OT_API::GetBasketMemberType(const OTIdentifier& BASKET_ASSET_TYPE_ID,
                                 int32_t nIndex,
                                 OTIdentifier& theOutputMemberType) const
{
    // There is an OT_ASSERT_MSG in here for memory failure,
    // but it still might return nullptr if various verification fails.
    OTAssetContract* pContract =
        GetAssetType(BASKET_ASSET_TYPE_ID, __FUNCTION__);
    if (nullptr == pContract) return false;
    // No need to cleanup pContract.
    // Next load the Basket object out of that contract.
    Basket theBasket;

    // todo perhaps verify the basket here, even though I already verified the
    // asset contract itself...
    // Can't never be too sure.
    if (pContract->GetBasketInfo().GetLength() &&
        theBasket.LoadContractFromString(pContract->GetBasketInfo())) {
        if ((nIndex >= theBasket.Count()) || (nIndex < 0)) {
            otErr << "OT_API::GetBasketMemberType: Index out of bounds: "
                  << nIndex << "\n";
            return false;
        }

        BasketItem* pItem = theBasket.At(nIndex);
        OT_ASSERT_MSG(nullptr != pItem,
                      "Bad index in OT_API::GetBasketMemberType");

        theOutputMemberType = pItem->SUB_CONTRACT_ID;

        return true;
    }
    return false;
}

// Get Basket Member Minimum Transfer Amount
//
// Returns the minimum transfer amount for one of the asset types that
// makes up this basket, by index.
// (Or 0.)
//
int64_t OT_API::GetBasketMemberMinimumTransferAmount(
    const OTIdentifier& BASKET_ASSET_TYPE_ID, int32_t nIndex) const
{
    // There is an OT_ASSERT_MSG in here for memory failure,
    // but it still might return nullptr if various verification fails.
    OTAssetContract* pContract =
        GetAssetType(BASKET_ASSET_TYPE_ID, __FUNCTION__);
    if (nullptr == pContract) return 0;
    // No need to cleanup pContract.
    // Next load the Basket object out of that contract.
    Basket theBasket;

    // todo perhaps verify the basket here, even though I already verified the
    // asset contract itself...
    // Can't never be too sure.
    if (pContract->GetBasketInfo().GetLength() &&
        theBasket.LoadContractFromString(pContract->GetBasketInfo())) {
        if ((nIndex >= theBasket.Count()) || (nIndex < 0)) {
            otErr << "OT_API::GetBasketMemberMinimumTransferAmount: Index "
                     "out of bounds: " << nIndex << "\n";
            return 0;
        }

        BasketItem* pItem = theBasket.At(nIndex);

        OT_ASSERT_MSG(
            nullptr != pItem,
            "Bad index in OT_API::GetBasketMemberMinimumTransferAmount.");

        return pItem->lMinimumTransferAmount;
        ;
    }
    else
        otErr << "OT_API::GetBasketMemberMinimumTransferAmount: Failed "
                 "loading basket info from basket asset contract.\n";

    return 0;
}

// Get Basket Minimum Transfer Amount
//
// Returns the minimum transfer amount for the basket.
// (Or 0.)
//
int64_t OT_API::GetBasketMinimumTransferAmount(
    const OTIdentifier& BASKET_ASSET_TYPE_ID) const
{
    // There is an OT_ASSERT_MSG in here for memory failure,
    // but it still might return nullptr if various verification fails.
    OTAssetContract* pContract =
        GetAssetType(BASKET_ASSET_TYPE_ID, __FUNCTION__);
    if (nullptr == pContract) return 0;
    // No need to cleanup pContract.
    // Next load the Basket object out of that contract.
    Basket theBasket;

    // todo perhaps verify the basket here, even though I already verified the
    // asset contract itself...
    // Can't never be too sure.
    if (pContract->GetBasketInfo().GetLength() &&
        theBasket.LoadContractFromString(pContract->GetBasketInfo()))
        return theBasket.GetMinimumTransfer();

    return 0;
}

// GENERATE BASKET CREATION REQUEST
//
// Creates a new request (for generating a new Basket type).
// (Each currency in this request will be added with
// subsequent calls to OT_API::GenerateBasketItem()).
//
// (Caller is responsible to delete.)
//
Basket* OT_API::GenerateBasketCreation(
    const OTIdentifier& USER_ID,
    int64_t MINIMUM_TRANSFER) const // Must be above zero. If <= 0, defaults to
                                    // 10.
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    int64_t lMinimumTransferAmount = 10;

    if (MINIMUM_TRANSFER > 0) lMinimumTransferAmount = MINIMUM_TRANSFER;
    Basket* pBasket = new Basket(0, lMinimumTransferAmount);
    OT_ASSERT_MSG(nullptr != pBasket, "OT_API::GenerateBasketCreation: Error "
                                      "allocating memory in the OT API");

    pBasket->SignContract(*pNym);
    pBasket->SaveContract();

    return pBasket;
}

// ADD BASKET CREATION ITEM
//
// Used for creating a request to generate a new basket currency.
bool OT_API::AddBasketCreationItem(const OTIdentifier& USER_ID, // for
                                                                // signature.
                                   Basket& theBasket,
                                   const OTIdentifier& ASSET_TYPE_ID,
                                   int64_t MINIMUM_TRANSFER) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    // There is an OT_ASSERT_MSG in here for memory failure,
    // but it still might return nullptr if various verification fails.
    OTAssetContract* pContract = GetAssetType(ASSET_TYPE_ID, __FUNCTION__);
    if (nullptr == pContract) return false;
    // No need to cleanup pContract.

    theBasket.AddSubContract(ASSET_TYPE_ID, MINIMUM_TRANSFER);

    theBasket.IncrementSubCount();

    theBasket.ReleaseSignatures();
    theBasket.SignContract(*pNym);
    theBasket.SaveContract();

    return true;
}

// ISSUE BASKET CREATION REQUEST (to server.)
//
int32_t OT_API::issueBasket(const OTIdentifier& SERVER_ID,
                            const OTIdentifier& USER_ID,
                            const OTString& BASKET_INFO) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer = GetServer(SERVER_ID, __FUNCTION__);
    if (nullptr == pServer) return (-1);
    // AT SOME POINT, BASKET_INFO has been populated with the relevant data.
    // (see test client for example.)
    OTString strServerID(SERVER_ID), strNymID(USER_ID);

    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) Set up member variables
    theMessage.m_strCommand = "issueBasket";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_ascPayload.SetString(BASKET_INFO);

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

// GENERATE BASKET EXCHANGE REQUEST
//
// Creates a new request (for exchanging funds in/out of a Basket).
// (Each currency in this request will be added with
// subsequent calls to OT_API::GenerateBasketItem()).
//
// (Caller is responsible to delete.)
//
Basket* OT_API::GenerateBasketExchange(const OTIdentifier& SERVER_ID,
                                       const OTIdentifier& USER_ID,
                                       const OTIdentifier& BASKET_ASSET_TYPE_ID,
                                       const OTIdentifier& BASKET_ASSET_ACCT_ID,
                                       int32_t TRANSFER_MULTIPLE) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return nullptr;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTAssetContract* pContract =
        GetAssetType(BASKET_ASSET_TYPE_ID, __FUNCTION__);
    if (nullptr == pContract) return nullptr;
    // By this point, pContract is a good pointer, and is on the wallet. (No
    // need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, BASKET_ASSET_ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return nullptr;
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    if (BASKET_ASSET_TYPE_ID != pAccount->GetAssetTypeID()) {
        const OTString strAcctID(BASKET_ASSET_ACCT_ID),
            strAcctTypeID(BASKET_ASSET_TYPE_ID);
        otOut << "OT_API::GenerateBasketExchange: Wrong asset type ID "
                 "on account " << strAcctID << " (expected type to be "
              << strAcctTypeID << ")\n";
        return nullptr;
    }
    // By this point, I know that everything checks out. Signature and Account
    // ID.
    // pAccount is good, and no need to clean it up.
    OTString strServerID(SERVER_ID);

    int32_t nTransferMultiple = 1;

    if (TRANSFER_MULTIPLE > 0) nTransferMultiple = TRANSFER_MULTIPLE;

    // Next load the Basket object out of that contract.
    Basket theBasket;
    Basket* pRequestBasket = nullptr;

    // todo perhaps verify the basket here, even though I already verified the
    // asset contract itself...
    // Can't never be too sure.
    if (pContract->GetBasketInfo().Exists() &&
        theBasket.LoadContractFromString(pContract->GetBasketInfo())) {
        // We need a transaction number just to send this thing. Plus, we need a
        // number for
        // each sub-account to the basket, as well as the basket's main account.
        // That is: 1 + theBasket.Count() + 1
        //
        if (pNym->GetTransactionNumCount(SERVER_ID) < (2 + theBasket.Count())) {
            otOut << "OT_API::GenerateBasketExchange: you don't have "
                     "enough transaction numbers to perform the "
                     "exchange.\n";
        }
        else {
            pRequestBasket =
                new Basket(theBasket.Count(), theBasket.GetMinimumTransfer());
            OT_ASSERT_MSG(nullptr != pRequestBasket,
                          "OT_API::GenerateBasketExchange: Error allocating "
                          "memory in the OT API");

            pRequestBasket->SetTransferMultiple(
                nTransferMultiple); // This stays in this function.

            // Make sure the server knows where to put my new basket currency
            // funds,
            // once the exchange is done.
            pRequestBasket->SetRequestAccountID(
                BASKET_ASSET_ACCT_ID); // This stays too

            // Export the Basket object into a string, add it as
            // a payload on my request, and send to server.
            pRequestBasket->SignContract(*pNym);
            pRequestBasket->SaveContract();
        } // *pNym apparently has enough transaction numbers to exchange the
          // basket.
    }
    else {
        otOut << "OT_API::GenerateBasketExchange: Error loading "
                 "basket info from asset contract. "
                 "Are you SURE this is a basket currency?\n";
        return nullptr;
    }
    return pRequestBasket;
}

// ADD BASKET EXCHANGE ITEM
//
bool OT_API::AddBasketExchangeItem(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   Basket& theBasket,
                                   const OTIdentifier& ASSET_TYPE_ID,
                                   const OTIdentifier& ASSET_ACCT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTAssetContract* pContract = GetAssetType(ASSET_TYPE_ID, __FUNCTION__);
    if (nullptr == pContract) return false;
    // By this point, pContract is a good pointer, and is on the wallet. (No
    // need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ASSET_ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return false;
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    if (pNym->GetTransactionNumCount(SERVER_ID) < 1) {
        otOut << "OT_API::AddBasketExchangeItem: you need at least one "
                 "transaction number to add this exchange item.\n";
        return false;
    }
    if (ASSET_TYPE_ID != pAccount->GetAssetTypeID()) {
        const OTString strAssetTypeID(ASSET_TYPE_ID), strAcctID(ASSET_ACCT_ID);
        otOut << "OT_API::AddBasketExchangeItem: Wrong asset type ID "
                 "on account " << strAcctID << " (expected to find asset type "
              << strAssetTypeID << ")\n";
        return false;
    }
    // By this point, I know that everything checks out. Signature and Account
    // ID.
    // pAccount is good, and no need to clean it up.
    const OTString strServerID(SERVER_ID);

    int64_t lSubClosingTransactionNo = 0; // For the basketReceipt (closing
                                          // transaction num) for the sub
                                          // account.

    if (pNym->GetNextTransactionNum(*pNym, strServerID,
                                    lSubClosingTransactionNo)) // this saves
    {
        theBasket.AddRequestSubContract(ASSET_TYPE_ID, ASSET_ACCT_ID,
                                        lSubClosingTransactionNo);

        theBasket.ReleaseSignatures();
        theBasket.SignContract(*pNym);
        theBasket.SaveContract();

        return true;
    }
    else
        otErr << "OT_API::AddBasketExchangeItem: Failed getting next "
                 "transaction number. \n";

    return false;
}

// TODO!  Either stop getting the next transaction numbers above, and do it all
// in a big
// loop in the next function below, OR, provide a way to harvest those numbers
// back again
// in the event of error (ugh.)
//

// TODO: It was a mistake to only drop Nymbox notices for successful messages.
// If the client misses a failure message, the client will then fail to claw
// back
// his transaction numbers from that failure. For transactions, at least,
// failure
// notices should also be dropped.

// BUT perhaps we can avoid many failure notices, by nipping things in the bud.
// For
// example, if the NymboxHash is wrong, we can reject transactions before even
// the
// opening number is burned (similar to rejecting messages based on bad request
// #.)
// This "message level" rejection allows the client to claw-back or re-try,
// without
// the pain of having burned SOME numbers while others are still good (and then
// having to reconcile that mess to get back into sync.)

// Some notes on TRANSACTION NUMBERS:
/*
 1. You must BURN a number even to ATTEMPT a transaction.

 2. However, remember that OT has messages, and those messages contain
 transactions.
    A message might fail due to out-of-sync request number, meaning it was cut
 off
    before even having a chance to call the NotarizeTransactions code.

 3. If the message failed, that means the transaction has not yet even been
 tried, so
    the number on it is still good. (You can re-send the message.)

 4. But if the message succeeded, then the transaction itself requires 2 steps:
    the balance agreement, and the transaction itself.

 5. If the balance agreement or transaction fails, then the primary transaction
 # has been burned.
    BUT there may be OTHER transaction numbers that are STILL good. The closing
 numbers, etc.

 6. The OTHER numbers are only burned if the transaction is a SUCCESS.


 Therefore:
 -- If the message fails, we can re-sync and re-try. All trans #s are still
 good.
 -- If the message succeeds, but balance agreement fails, or the transaction
 fails,
    then the primary transaction # is burned, but any additional numbers are
 still good.
 -- If the transaction itself succeeds, then ALL numbers are burned.

 OT Client, if the MESSAGE was a failure, MUST re-send it, or claw back the
 numbers, since
 they are all still good, even the primary #. (This is only if an EXPLICIT
 message failure
 is received. A null reply could mean it was SUCCESSFUL but we just didn't SEE
 that success
 because we never got the reply! In that case, we do NOT want to re-try, nor do
 we want to
 claw the numbers back.)

 Thus OT client, as long as the MESSAGE was a success, can operate based on the
 assumption
 that the primary transaction number IS burned, whether the transaction itself
 was successful
 or not.

 OT client also assumes the other numbers are burned as well, UNLESS IT RECEIVES
 A FAILURE REPLY.
 If an explicit transaction failure is received, then OT client can CLAW BACK
 all of the transaction
 numbers EXCEPT the primary one.

 OT MUST SEE THE FAILURE MESSAGES -- AT LEAST FOR TRANSACTIONS!
 What if:

 What if I try a TRANSACTION, and fail to receive the server reply?
 Potentialities:

 1. Message was a failure. (No numbers are burned.)
 2. Message was success but balance agreement was a failure.
    (The opening # was burned, but the others are still good.)
 3. Message and balance agreement were both success, but transaction itself
 failed.
    (The opening # was burned, but the others are still good.)
 4. Message, balance agreement, and transaction were all a success.
    (ALL transaction #s on the message are burned.)


 Currently:
 1. If I don't receive this failure message, then I resend, which would
 basically work since the numbers are all still good.
    BUT IN THE CASE OF EXCHANGE BASKET, IT FUCKING GRABS A NEW NUMBER EACH CALL!
 So when OTAPI_Func retries, it grabs a new one each time.
    (Instead of just using the ones that are still good.)

 2. If I miss this success reply, or treat the null as a failure, then I will
 end up re-trying a transaction
    that has no prayer of working, since its opening # is already burned. I'll
 also fail to claw back my numbers, which are mostly still good.

 3. If I miss this success reply, or treat the null as a failure, then I will
 end up re-trying a transaction
    that has no prayer of working, since its opening # is already burned. I'll
 also fail to claw back my numbers, which are mostly still good.

 4. If I miss this success reply, the numbers are all burned on both sides, but
 I don't KNOW that, and though I correctly believe the numbers
    are all burned, I also end up CONTINUING to re-try the message, which fails
 every time, since it was already a success and the numbers are
    all burned already.
 ----------------------------------

 Therefore, the new solutions...



 --------------------------------------------------




 */

// EXCHANGE (into or out of) BASKET (request to server.)
//
int32_t OT_API::exchangeBasket(
    const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
    const OTIdentifier& BASKET_ASSET_ID, const OTString& BASKET_INFO,
    bool bExchangeInOrOut // exchanging in == true, out == false.
    ) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAssetContract* pContract = GetAssetType(BASKET_ASSET_ID, __FUNCTION__);
    if (nullptr == pContract) return (-1);
    // By this point, pContract is a good pointer, and is on the wallet. (No
    // need to cleanup.)
    const OTString strServerID(SERVER_ID), strUserID(USER_ID);

    // Next load the Basket object out of that contract, and load the
    // RequestBasket object that was passed in.
    Basket theBasket, theRequestBasket;

    if (pContract->GetBasketInfo().GetLength() &&
        theBasket.LoadContractFromString(pContract->GetBasketInfo()) &&
        BASKET_INFO.GetLength() &&
        theRequestBasket.LoadContractFromString(BASKET_INFO)) {
        const OTIdentifier& BASKET_ASSET_ACCT_ID(
            theRequestBasket.GetRequestAccountID());
        OTAccount* pAccount = GetOrLoadAccount(*pNym, BASKET_ASSET_ACCT_ID,
                                               SERVER_ID, __FUNCTION__);
        if (nullptr == pAccount) return (-1);

        // By this point, pAccount is a good pointer, and is on the wallet. (No
        // need to cleanup.)
        // We need a transaction number just to send this thing. Plus, we need a
        // number for
        // each sub-account to the basket, as well as the basket's main account.
        // That is: 1 + theBasket.Count() + 1
        // Total of 2, since theBasket.Count() worth of numbers were already
        // added in the
        // calls to OT_API::AddBasketExchangeItem.

        if (pNym->GetTransactionNumCount(SERVER_ID) < 2) {
            otOut << "OT_API::exchangeBasket: you don't have enough "
                     "transaction numbers to perform the exchange.\n";
        }
        else {
            int64_t lStoredTransactionNumber = 0;
            bool bGotTransNum = pNym->GetNextTransactionNum(
                *pNym, strServerID, lStoredTransactionNumber); // this saves

            if (bGotTransNum) {
                // LOAD the INBOX for the MAIN ACCOUNT
                //
                std::unique_ptr<OTLedger> pInbox(pAccount->LoadInbox(*pNym));
                std::unique_ptr<OTLedger> pOutbox(pAccount->LoadOutbox(*pNym));

                if (nullptr == pInbox) {
                    otOut << "OT_API::exchangeBasket: Failed loading inbox!\n";

                    // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF
                    // AVAILABLE NUMBERS.
                    pNym->AddTransactionNum(*pNym, strServerID,
                                            lStoredTransactionNumber,
                                            true); // bSave=true
                }
                else if (nullptr == pOutbox) {
                    otOut << "OT_API::exchangeBasket: Failed loading outbox!\n";

                    // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF
                    // AVAILABLE NUMBERS.
                    pNym->AddTransactionNum(*pNym, strServerID,
                                            lStoredTransactionNumber,
                                            true); // bSave=true
                }
                // Set up the Request Basket! ------------------------------
                else {
                    // Create a transaction
                    OTTransaction* pTransaction =
                        OTTransaction::GenerateTransaction(
                            USER_ID, BASKET_ASSET_ACCT_ID, SERVER_ID,
                            OTTransaction::exchangeBasket,
                            lStoredTransactionNumber);

                    // set up the transaction item (each transaction may have
                    // multiple items...)
                    OTItem* pItem = OTItem::CreateItemFromTransaction(
                        *pTransaction, OTItem::exchangeBasket);

                    // This pItem is where the Basket Info will be stored. (So
                    // it ends up on receipts...)
                    pTransaction->AddItem(*pItem); // the Transaction's
                                                   // destructor will cleanup
                                                   // the item. It "owns" it
                                                   // now.

                    // NOTE: I'm not checking this call for success...
                    // But, I DID check the count beforehand, and I know there
                    // are enough numbers.
                    //
                    int64_t lClosingTransactionNo =
                        0; // for Main Basket Acct on the Request Basket.
                    OT_ASSERT(pNym->GetNextTransactionNum(
                        *pNym, strServerID,
                        lClosingTransactionNo)); // this saves

                    // This goes in the final API call.
                    theRequestBasket.SetClosingNum(
                        lClosingTransactionNo); // For the basketReceipt
                                                // (Closing Transaction Num) for
                                                // main account.

                    // This goes in the final API call.
                    theRequestBasket.SetExchangingIn(bExchangeInOrOut);

                    theRequestBasket.ReleaseSignatures();
                    theRequestBasket.SignContract(*pNym);
                    theRequestBasket.SaveContract();

                    // Export the Basket object into a string, add it as
                    // a payload on my request, and send to server.
                    OTString strBasketInfo;
                    theRequestBasket.SaveContractRaw(strBasketInfo);

                    pItem->SetAttachment(strBasketInfo);

                    // sign the item. save it.
                    //
                    pItem->SignContract(*pNym);
                    pItem->SaveContract();

                    // BALANCE AGREEMENT!
                    //
                    // pBalanceItem is signed and saved within this call. No
                    // need to do that again.
                    OTItem* pBalanceItem = pInbox->GenerateBalanceStatement(
                        0, // Change in balance is 0. (The accounts will all be
                           // changed,
                        *pTransaction, *pNym, *pAccount,
                        *pOutbox); // but basketReceipts will be used to account
                                   // for it.)

                    if (nullptr !=
                        pBalanceItem) // will never be nullptr. Will assert
                                      // above before it gets here.
                        pTransaction->AddItem(*pBalanceItem); // Better not be
                    // nullptr... message
                    // will fail...
                    // But better
                    // check anyway.

                    // sign the transaction
                    pTransaction->SignContract(*pNym);
                    pTransaction->SaveContract();

                    // set up the ledger
                    OTLedger theLedger(USER_ID, BASKET_ASSET_ACCT_ID,
                                       SERVER_ID);
                    theLedger.GenerateLedger(
                        BASKET_ASSET_ACCT_ID, SERVER_ID,
                        OTLedger::message); // bGenerateLedger defaults to
                                            // false, which is correct.
                    theLedger.AddTransaction(*pTransaction);

                    // sign the ledger
                    theLedger.SignContract(*pNym);
                    theLedger.SaveContract();

                    // extract the ledger in ascii-armored form
                    OTString strLedger(theLedger);
                    OTASCIIArmor ascLedger; // I can't pass strLedger into this
                                            // constructor because I want to
                                            // encode it

                    // Encoding...
                    ascLedger.SetString(strLedger);

                    OTMessage theMessage;

                    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
                    int64_t lRequestNumber = 0;
                    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
                    theMessage.m_strRequestNum.Format(
                        "%ld", lRequestNumber); // Always have to send this.
                    pNym->IncrementRequestNum(
                        *pNym, strServerID); // since I used it for a server
                                             // request, I have to increment it

                    // (1) Set up member variables
                    theMessage.m_strCommand = "notarizeTransactions";
                    theMessage.m_strNymID = strUserID;
                    theMessage.m_strServerID = strServerID;
                    theMessage.SetAcknowledgments(
                        *pNym); // Must be called AFTER theMessage.m_strServerID
                                // is already set. (It uses it.)

                    BASKET_ASSET_ACCT_ID.GetString(theMessage.m_strAcctID);
                    theMessage.m_ascPayload = ascLedger;

                    OTIdentifier NYMBOX_HASH;
                    const std::string str_server(strServerID.Get());
                    const bool bNymboxHash =
                        pNym->GetNymboxHash(str_server, NYMBOX_HASH);
                    NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

                    if (!bNymboxHash)
                        otErr << "Failed getting NymboxHash from Nym for "
                                 "server: " << str_server << "\n";

                    // (2) Sign the Message
                    theMessage.SignContract(*pNym);

                    // (3) Save the Message (with signatures and all, back to
                    // its internal member m_strRawFile.)
                    theMessage.SaveContract();

                    // (Send it)
                    m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                                      m_pTransportCallback);
                    m_pClient->ProcessMessageOut(theMessage);

                    return m_pClient->CalcReturnVal(lRequestNumber);
                } // Inbox loaded.
            }     // successfully got first transaction number.
        }
    }

    return (-1);
}

int32_t OT_API::getTransactionNumber(const OTIdentifier& SERVER_ID,
                                     const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)

    const int32_t nCount = pNym->GetTransactionNumCount(SERVER_ID);
    const int32_t nMaxCount = 50; // todo no hardcoding. (max transaction nums
                                  // allowed out at a single time.)

    if (nCount > nMaxCount) {
        otOut << "OT_API::getTransactionNumber: Failure: That Nym already has "
                 "more than " << nMaxCount
              << " transaction numbers signed out. (Use those first.)\n";
        return 0; // Java code needs to know that no msg went out, BUT it's
                  // because we already have TOO
        // MANY numbers (i.e. "a good reason.") This means "process your Nymbox
        // and grab your intermediary
        // files, and try again!" So, we return 0 to indicate this.
        //
        //      return (-1);
    }
    OTMessage theMessage;

    int32_t nReturnValue = m_pClient->ProcessUserCommand(
        OTClient::getTransactionNum, theMessage, *pNym, *pServer,
        nullptr); // nullptr pAccount on this command.
    if (0 < nReturnValue) {
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return nReturnValue;
    }
    else
        otErr << "OT_API::getTransactionNumber: Error processing "
                 "getTransactionNumber command. Return value: " << nReturnValue
              << "\n";

    return (-1);
}

int32_t OT_API::notarizeWithdrawal(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   const OTIdentifier& ACCT_ID,
                                   const int64_t& AMOUNT) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return (-1);
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    OTIdentifier CONTRACT_ID;
    OTString strContractID, strServerID(SERVER_ID);
    CONTRACT_ID = pAccount->GetAssetTypeID();
    CONTRACT_ID.GetString(strContractID);
    if (!OTDB::Exists(OTFolders::Mint().Get(), strServerID.Get(),
                      strContractID.Get())) {
        otErr << "OT_API::notarizeWithdrawal: File does not exist: "
              << OTFolders::Mint() << OTLog::PathSeparator() << strServerID
              << OTLog::PathSeparator() << strContractID << "\n";
        return -1;
    }
    std::unique_ptr<Mint> pMint(Mint::MintFactory(strServerID, strContractID));
    OT_ASSERT(nullptr != pMint);
    OTMessage theMessage;

    const int64_t lTotalAmount = AMOUNT;
    int64_t lAmount = lTotalAmount;

    OTString strNymID(USER_ID), strFromAcct(ACCT_ID);

    int64_t lStoredTransactionNumber = 0;
    bool bGotTransNum = false;
    std::unique_ptr<OTLedger> pInbox(pAccount->LoadInbox(*pNym));
    std::unique_ptr<OTLedger> pOutbox(pAccount->LoadOutbox(*pNym));

    if (nullptr == pInbox) {
        otOut << __FUNCTION__ << ": Error: Failed loading inbox!\n";
        return (-1);
    }
    if (nullptr == pOutbox) {
        otOut << __FUNCTION__ << ": Error: Failed loading outbox!\n";
        return (-1);
    }

    bGotTransNum = pNym->GetNextTransactionNum(*pNym, strServerID,
                                               lStoredTransactionNumber);
    if (!bGotTransNum) {
        otOut << __FUNCTION__ << ": Next Transaction Number Available: Suggest "
                                 "requesting the server for a new one.\n";
        return (-1);
    }

    // Create a transaction
    OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
        USER_ID, ACCT_ID, SERVER_ID, OTTransaction::withdrawal,
        lStoredTransactionNumber);

    // set up the transaction item (each transaction may have multiple items...)
    OTItem* pItem =
        OTItem::CreateItemFromTransaction(*pTransaction, OTItem::withdrawal);
    pItem->SetAmount(lTotalAmount);

    OTString strNote("Gimme cash!"); // TODO: Note is unnecessary for cash
                                     // withdrawal. Research uses / risks.
    pItem->SetNote(strNote);
    const OTPseudonym* pServerNym = pServer->GetContractPublicNym();

    const OTIdentifier SERVER_USER_ID(*pServerNym);
    if ((nullptr != pServerNym) && pMint->LoadMint() &&
        pMint->VerifyMint(const_cast<OTPseudonym&>(*pServerNym))) {
        int64_t lRequestNumber = 0;
        Purse* pPurse = new Purse(SERVER_ID, CONTRACT_ID, SERVER_USER_ID);
        Purse* pPurseMyCopy = new Purse(SERVER_ID, CONTRACT_ID, USER_ID);

        // Create all the necessary tokens for the withdrawal amount.
        // Push copies of each token into a purse to be sent to the server,
        // as well as a purse to be kept for unblinding when we receive the
        // server response.  (Coin private unblinding keys are not sent to
        // the server, obviously.)
        int64_t lTokenAmount = 0;
        while ((lTokenAmount = pMint->GetLargestDenomination(lAmount)) > 0) {
            lAmount -= lTokenAmount;

            // Create the relevant token request with same server/asset ID as
            // the purse.
            // the purse does NOT own the token at this point. The token's
            // constructor
            // just uses it to copy some IDs, since they must match.
            //
            std::unique_ptr<Token> pToken(
                Token::InstantiateAndGenerateTokenRequest(
                    *pPurse, *pNym, *pMint, lTokenAmount));
            OT_ASSERT(nullptr != pToken);

            // Sign it and save it.
            pToken->SignContract(*pNym);
            pToken->SaveContract();

            // Now the proto-token is generated, let's add it to a purse
            // By pushing *pToken into pPurse with *pServerNym, I encrypt it to
            // pServerNym.
            // So now only the server Nym can decrypt that token and pop it out
            // of that purse.
            pPurse->Push(*pServerNym, *pToken);

            // I'm saving my own copy of all this, encrypted to my nym
            // instead of the server's, so I can get to my private coin data.
            // The server's copy of pToken is already Pushed, so I can re-use
            // the variable now for my own purse.
            pToken->ReleaseSignatures();
            pToken->SetSavePrivateKeys(); // This time it will save the private
                                          // keys when I sign it
            pToken->SignContract(*pNym);
            pToken->SaveContract();

            pPurseMyCopy->Push(*pNym, *pToken); // Now my copy of the purse has
                                                // a version of the token,
        }
        // Save the purse into a string...
        OTString strPurse;
        pPurse->SignContract(*pNym);
        pPurse->SaveContract();
        pPurse->SaveContractRaw(strPurse);

        // Add the purse string as the attachment on the transaction item.
        pItem->SetAttachment(
            strPurse); // The purse is contained in the reference string.
        pPurseMyCopy->SignContract(
            *pNym); // encrypted to me instead of the server, and including
        pPurseMyCopy->SaveContract(); // the private keys for unblinding the
                                      // server response.
        // This thing is neat and tidy. The wallet can just save it as an
        // ascii-armored string as a
        // purse field inside the wallet file.  It doesn't do that for now
        // (TODO) but it easily could.
        // Add the purse to the wallet
        // (We will need it to look up the private coin info for unblinding the
        // token,
        //  when the response comes from the server.)
        pWallet->AddPendingWithdrawal(*pPurseMyCopy);

        delete pPurse;
        pPurse = nullptr;       // We're done with this one.
        pPurseMyCopy = nullptr; // The wallet owns my copy now and will handle
                                // cleaning it up.

        // sign the item
        pItem->SignContract(*pNym);
        pItem->SaveContract();

        pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                       // cleanup the item. It "owns" it now.
        // BALANCE AGREEMENT

        // pBalanceItem is signed and saved within this call. No need to do that
        // again.
        OTItem* pBalanceItem = pInbox->GenerateBalanceStatement(
            lTotalAmount * (-1), *pTransaction, *pNym, *pAccount, *pOutbox);

        if (nullptr != pBalanceItem) // will never be nullptr. Will assert above
                                     // before it gets here.
            pTransaction->AddItem(*pBalanceItem); // Better not be nullptr...
                                                  // message will fail... But
                                                  // better check anyway.

        // sign the transaction
        pTransaction->SignContract(*pNym);
        pTransaction->SaveContract();

        // set up the ledger
        OTLedger theLedger(USER_ID, ACCT_ID, SERVER_ID);
        theLedger.GenerateLedger(ACCT_ID, SERVER_ID,
                                 OTLedger::message); // bGenerateLedger defaults
                                                     // to false, which is
                                                     // correct.
        theLedger.AddTransaction(*pTransaction);

        // sign the ledger
        theLedger.SignContract(*pNym);
        theLedger.SaveContract();

        // extract the ledger in ascii-armored form
        OTString strLedger(theLedger);
        OTASCIIArmor ascLedger; // I can't pass strLedger into this constructor
                                // because I want to encode it

        // Encoding...
        ascLedger.SetString(strLedger);
        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%ld", lRequestNumber); // Always have to send this.
        pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                       // server request, I have
                                                       // to increment it

        // (1) Set up member variables
        theMessage.m_strCommand = "notarizeTransactions";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strServerID = strServerID;
        theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                              // theMessage.m_strServerID is
                                              // already set. (It uses it.)

        theMessage.m_strAcctID = strFromAcct;
        theMessage.m_ascPayload = ascLedger;

        OTIdentifier NYMBOX_HASH;
        const std::string str_server(strServerID.Get());
        const bool bNymboxHash = pNym->GetNymboxHash(str_server, NYMBOX_HASH);
        NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

        if (!bNymboxHash)
            otErr << "Failed getting NymboxHash from Nym for server: "
                  << str_server << "\n";

        // (2) Sign the Message
        theMessage.SignContract(*pNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        // (Send it)
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return m_pClient->CalcReturnVal(lRequestNumber);
    }
    else {
        // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE NUMBERS.
        pNym->AddTransactionNum(*pNym, strServerID, lStoredTransactionNumber,
                                true); // bSave=true
    }

    return (-1);
}

int32_t OT_API::notarizeDeposit(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTIdentifier& ACCT_ID,
                                const OTString& THE_PURSE) const
{
    OTString strPurseReason(
        "Depositing a cash purse. Enter passphrase for the purse.");
    OTPasswordData thePWDataWallet(
        "Depositing a cash purse. Enter master passphrase for wallet.");
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__,
        &thePWDataWallet); // These copiously log, and ASSERT.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    OTIdentifier CONTRACT_ID;
    OTString strContractID;

    CONTRACT_ID = pAccount->GetAssetTypeID();
    CONTRACT_ID.GetString(strContractID);
    OTMessage theMessage;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strFromAcct(ACCT_ID);

    const OTPseudonym* pServerNym = pServer->GetContractPublicNym();
    const OTIdentifier SERVER_USER_ID(*pServerNym);
    Purse thePurse(SERVER_ID, CONTRACT_ID, SERVER_USER_ID);

    int64_t lStoredTransactionNumber = 0;
    bool bGotTransNum = false;
    std::unique_ptr<OTLedger> pInbox(pAccount->LoadInbox(*pNym));
    std::unique_ptr<OTLedger> pOutbox(pAccount->LoadOutbox(*pNym));

    if (nullptr == pInbox) {
        otOut << __FUNCTION__ << ": Error: Failed loading inbox!\n";
        return (-1);
    }
    if (nullptr == pOutbox) {
        otOut << __FUNCTION__ << ": Error: Failed loading outbox!\n";
        return (-1);
    }

    bGotTransNum = pNym->GetNextTransactionNum(*pNym, strServerID,
                                               lStoredTransactionNumber);
    if (!bGotTransNum) {
        otOut << __FUNCTION__ << ": Next Transaction Number Available: Suggest "
                                 "requesting the server for a new one.\n";
        return (-1);
    }

    if (nullptr == pServerNym) OT_FAIL;

    bool bSuccess = false;

    // Create a transaction
    OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
        USER_ID, ACCT_ID, SERVER_ID, OTTransaction::deposit,
        lStoredTransactionNumber);
    // set up the transaction item (each transaction may have multiple items...)
    OTItem* pItem =
        OTItem::CreateItemFromTransaction(*pTransaction, OTItem::deposit);

    // What's going on here?
    // A purse can be encrypted by a private key (controlled by a Nym) or by a
    // symmetric
    // key (embedded inside the purse along with a corresponding master key.)
    // The below
    // function is what actually loads up pPurse from string (THE_PURSE) and
    // this call
    // also returns pOwner, which is a pointer to a special wrapper class (which
    // you must
    // delete, when you're done with it) which contains a pointer EITHER to the
    // owner Nym
    // for that purse, OR to the "owner" symmetric key for that purse.
    //
    // This way, any subsequent purse operations can use pOwner, regardless of
    // whether there
    // is actually a Nym inside, or a symmetric key. (None of the purse
    // operations will care,
    // since they can use pOwner either way.)
    //
    OTPassword thePassword;
    Purse theSourcePurse(thePurse);

    std::unique_ptr<OTNym_or_SymmetricKey> pPurseOwner(
        LoadPurseAndOwnerForMerge(
            THE_PURSE, theSourcePurse, thePassword,
            false,    // MUST be private, if a nym.
            &USER_ID, // This can be nullptr, *IF* purse is password-protected.
                      // (It's just ignored in that case.) Otherwise if it's
                      // Nym-protected, the purse will have a NymID on it
                      // already, which is what LoadPurseAndOwnerForMerge will
                      // try first. If not (it's optional), then USER_ID is the
                      // ID it will try next, before failing altogether.
            &strPurseReason));
    if (nullptr != pPurseOwner) {
        OTNym_or_SymmetricKey theServerNymAsOwner(*pServerNym);

        while (!theSourcePurse.IsEmpty()) {
            Token* pToken = theSourcePurse.Pop(*pPurseOwner);

            if (nullptr != pToken) {
                // TODO need 2-recipient envelopes. My request to the server is
                // encrypted to the server's nym,
                // but it should be encrypted to my Nym also, so both have
                // access to decrypt it.

                // Now the token is ready, let's add it to a purse
                // By pushing theToken into thePurse with *pServerNym, I encrypt
                // it to pServerNym.
                // So now only the server Nym can decrypt that token and pop it
                // out of that purse.
                if (false ==
                    pToken->ReassignOwnership(
                        *pPurseOwner,         // must be private, if a nym.
                        theServerNymAsOwner)) // can be public, if a nym.
                {
                    otErr << "OT_API::notarizeDeposit: Error re-assigning "
                             "ownership of token (to server.)\n";
                    delete pToken;
                    pToken = nullptr;
                    bSuccess = false;
                    break;
                }
                else {
                    otLog3 << "OT_API::notarizeDeposit: Success "
                              "re-assigning ownership of token (to "
                              "server.)\n";

                    bSuccess = true;

                    pToken->ReleaseSignatures();
                    pToken->SignContract(*pNym);
                    pToken->SaveContract();

                    thePurse.Push(theServerNymAsOwner, *pToken);

                    int64_t lTemp = pItem->GetAmount();
                    pItem->SetAmount(lTemp += pToken->GetDenomination());
                }
                delete pToken;
                pToken = nullptr;
            }
            else {
                otErr << "Error loading token from purse.\n";
                break;
            }
        } // while
    }
    if (bSuccess) {
        int64_t lRequestNumber = 0;
        // Save the purse into a string...
        OTString strPurse;
        thePurse.SignContract(*pNym);
        thePurse.SaveContract();
        thePurse.SaveContractRaw(strPurse);

        // Add the purse string as the attachment on the transaction item.
        pItem->SetAttachment(
            strPurse); // The purse is contained in the reference string.

        // sign the item
        pItem->SignContract(*pNym);
        pItem->SaveContract();

        // the Transaction "owns" the item now and will handle cleaning it up.
        pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                       // cleanup the item. It "owns" it now.

        // BALANCE AGREEMENT
        //
        // pBalanceItem is signed and saved within this call. No need to do that
        // again.
        OTItem* pBalanceItem = pInbox->GenerateBalanceStatement(
            pItem->GetAmount(), *pTransaction, *pNym, *pAccount, *pOutbox);

        if (nullptr != pBalanceItem) // will never be nullptr. Will assert above
                                     // before it gets here.
            pTransaction->AddItem(*pBalanceItem); // Better not be nullptr...
                                                  // message will fail... But
                                                  // better check anyway.

        // sign the transaction
        pTransaction->SignContract(*pNym);
        pTransaction->SaveContract();

        // set up the ledger
        OTLedger theLedger(USER_ID, ACCT_ID, SERVER_ID);
        theLedger.GenerateLedger(ACCT_ID, SERVER_ID,
                                 OTLedger::message); // bGenerateLedger defaults
                                                     // to false, which is
                                                     // correct.
        theLedger.AddTransaction(*pTransaction); // now the ledger "owns" and
                                                 // will handle cleaning up the
                                                 // transaction.

        // sign the ledger
        theLedger.SignContract(*pNym);
        theLedger.SaveContract();

        // extract the ledger in ascii-armored form... encoding...
        OTString strLedger(theLedger);
        OTASCIIArmor ascLedger(strLedger);

        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%ld", lRequestNumber); // Always have to send this.
        pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                       // server request, I have
                                                       // to increment it

        // (1) Set up member variables
        theMessage.m_strCommand = "notarizeTransactions";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strServerID = strServerID;
        theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                              // theMessage.m_strServerID is
                                              // already set. (It uses it.)

        theMessage.m_strAcctID = strFromAcct;
        theMessage.m_ascPayload = ascLedger;

        OTIdentifier NYMBOX_HASH;
        const std::string str_server(strServerID.Get());
        const bool bNymboxHash = pNym->GetNymboxHash(str_server, NYMBOX_HASH);
        NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

        if (!bNymboxHash)
            otErr << "Failed getting NymboxHash from Nym for server: "
                  << str_server << "\n";

        // (2) Sign the Message
        theMessage.SignContract(*pNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        // (Send it)
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return m_pClient->CalcReturnVal(lRequestNumber);

    } // bSuccess
    else {
        delete pItem;
        pItem = nullptr;
        delete pTransaction;
        pTransaction = nullptr;

        // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE NUMBERS.
        pNym->AddTransactionNum(*pNym, strServerID, lStoredTransactionNumber,
                                true); // bSave=true
    }

    return (-1);
}

// Let's pretend you are paying a dollar dividend for Pepsi shares...
// Therefore ACCT_ID needs to be a dollar account, and SHARES_ASSET_ID needs
// to be the Pepsi asset type ID. (NOT the dollar asset type ID...)
//
int32_t OT_API::payDividend(
    const OTIdentifier& SERVER_ID,
    const OTIdentifier& ISSUER_USER_ID,        // must be issuer of
                                               // SHARES_ASSET_TYPE_ID
    const OTIdentifier& DIVIDEND_FROM_ACCT_ID, // if dollars paid for pepsi
                                               // shares,
    // then this is the issuer's dollars
    // account.
    const OTIdentifier& SHARES_ASSET_TYPE_ID, // if dollars paid for pepsi
                                              // shares,
    // then this is the pepsi shares asset
    // type id.
    const OTString& DIVIDEND_MEMO, // a message attached to the payout request.
    const int64_t& AMOUNT_PER_SHARE) const // number of dollars to be paid out
                                           // PER
// SHARE (multiplied by total number of
// shares issued.)
{
    OTPseudonym* pNym =
        GetOrLoadPrivateNym(ISSUER_USER_ID, false,
                            __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pDividendSourceAccount =
        GetOrLoadAccount(*pNym, DIVIDEND_FROM_ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pDividendSourceAccount) return (-1);
    // By this point, pDividendSourceAccount is a good pointer, and is on the
    // wallet. (No need to cleanup.)
    OTAssetContract* pSharesContract = GetAssetType(
        SHARES_ASSET_TYPE_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pSharesContract) return (-1);
    // By this point, pSharesContract is a good pointer.  (No need to cleanup.)
    OTWallet* pWallet =
        GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    if (nullptr == pWallet) return (-1);
    OTAccount* pSharesIssuerAcct =
        pWallet->GetIssuerAccount(SHARES_ASSET_TYPE_ID);

    if (nullptr == pSharesIssuerAcct) {
        otErr << __FUNCTION__
              << ": Failure: Unable to find issuer account for the "
                 "shares asset type. Are you sure you're the issuer?\n";
        return (-1);
    }
    const OTIdentifier SHARES_ISSUER_ACCT_ID(*pSharesIssuerAcct);
    if (!pDividendSourceAccount->VerifyOwner(*pNym)) {
        otErr << __FUNCTION__
              << ": Failure: Nym doesn't verify as owner of the source "
                 "account for the dividend payout.\n";
        return (-1);
    }
    if (!pSharesIssuerAcct->VerifyOwner(*pNym)) {
        otErr << __FUNCTION__
              << ": Failure: Nym doesn't verify as owner of issuer "
                 "account for the shares (the shares we're paying the dividend "
                 "on...)\n";
        return (-1);
    }
    OT_ASSERT_MSG(pSharesIssuerAcct->GetBalance() <= 0,
                  "Assert (strange): issuer account should never have a "
                  "higher-than-zero balance.\n");

    if (0 == pSharesIssuerAcct->GetBalance()) {
        otErr << __FUNCTION__
              << ": Failure: There are no shares issued for that asset type. "
                 "(Therefore you cannot pay any dividend...)\n";
        return (-1);
    }
    const int64_t lAmountPerShare = AMOUNT_PER_SHARE;

    if (lAmountPerShare <= 0) {
        otErr << __FUNCTION__
              << ": Failure: The amount per share must be larger than zero.\n";
        return (-1);
    }
    // If there are 100,000 Pepsi shares, then the Pepsi issuer acct will have
    // -100,000 balance.
    // Therefore we multiply by -1, resulting in 100,000. Then we multiple that
    // by the amount to be
    // paid per share, let's say $5, resulting in a lTotalCostOfDividend of
    // $500,000 that must be
    // available in the dollar account (in order to successfully pay this
    // dividend.)
    //
    const int64_t lTotalCostOfDividend =
        ((-1) * pSharesIssuerAcct->GetBalance()) * lAmountPerShare;
    // Let's make sure we have enough money in the dividend source account, to
    // pay the total cost..
    //
    if (pDividendSourceAccount->GetBalance() < lTotalCostOfDividend) {
        otErr << __FUNCTION__ << ": Failure: There's not enough ("
              << pDividendSourceAccount->GetBalance()
              << ") in the source "
                 "account, to cover the total cost of the dividend ("
              << lTotalCostOfDividend << ".)\n";
        return (-1);
    }
    OTMessage theMessage;

    OTString strServerID(SERVER_ID), strNymID(ISSUER_USER_ID),
        strFromAcct(DIVIDEND_FROM_ACCT_ID);

    int64_t lStoredTransactionNumber = 0;
    bool bGotTransNum = pNym->GetNextTransactionNum(*pNym, strServerID,
                                                    lStoredTransactionNumber);

    if (bGotTransNum) {
        // Expiration (ignored by server -- it sets its own for its vouchers.)
        const time64_t VALID_FROM =
            OTTimeGetCurrentTime(); // This time is set to TODAY NOW
        const time64_t VALID_TO = OTTimeAddTimeInterval(
            VALID_FROM, OTTimeGetSecondsFromTime(
                            OT_TIME_SIX_MONTHS_IN_SECONDS)); // 6 months.
        // The server only uses the amount and asset type from this cheque when
        // it
        // constructs the actual voucher (to the dividend payee.) And remember
        // there
        // might be a hundred shareholders, so the server would create a hundred
        // vouchers in that case.
        //
        OTCheque theRequestVoucher(
            SERVER_ID, SHARES_ASSET_TYPE_ID); // <====== Server needs this
                                              // (SHARES_ASSET_TYPE_ID)

        bool bIssueCheque = theRequestVoucher.IssueCheque(
            lAmountPerShare, // <====== Server needs this (lAmountPerShare.)
            lStoredTransactionNumber, // server actually ignores this and
                                      // supplies its own transaction number for
                                      // any vouchers.
            VALID_FROM, VALID_TO, SHARES_ISSUER_ACCT_ID, ISSUER_USER_ID,
            DIVIDEND_MEMO);

        /*
         NOTE: The above cheque isn't actually USED for anything, except as a
         vehicle to send additional
         data to the server. For example, the server will need to know the asset
         type ID for the shares.
         It gets that information from this voucher's asset type ID. It will
         also need to know the amount-
         per-share, which is also on this voucher, as its amount. The voucher
         code already does a similar
         thing, and this code already copied the voucher code since they were so
         similar, so we're just
         using the same mechanism here. It's consistent.
         On the server side, we'll also need to know the issuer account ID for
         the shares asset type, so
         we set that as the "from" account on the request voucher (again, just
         as a way of transmitting it.)
         */
        std::unique_ptr<OTLedger> pInbox(
            pDividendSourceAccount->LoadInbox(*pNym));
        std::unique_ptr<OTLedger> pOutbox(
            pDividendSourceAccount->LoadOutbox(*pNym));

        if (nullptr == pInbox) {
            otOut << __FUNCTION__ << ": Failed loading inbox for acct "
                  << strFromAcct << "\n";

            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            pNym->AddTransactionNum(*pNym, strServerID,
                                    lStoredTransactionNumber,
                                    true); // bSave=true
        }
        else if (nullptr == pOutbox) {
            otOut << __FUNCTION__ << ": Failed loading outbox for acct "
                  << strFromAcct << "\n";

            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            pNym->AddTransactionNum(*pNym, strServerID,
                                    lStoredTransactionNumber,
                                    true); // bSave=true
        }
        else if (!bIssueCheque) {
            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            pNym->AddTransactionNum(*pNym, strServerID,
                                    lStoredTransactionNumber,
                                    true); // bSave=true
        }
        else {
            // Create a transaction
            OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
                ISSUER_USER_ID, DIVIDEND_FROM_ACCT_ID, SERVER_ID,
                OTTransaction::payDividend, lStoredTransactionNumber);
            // set up the transaction item (each transaction may have multiple
            // items...)
            OTItem* pItem = OTItem::CreateItemFromTransaction(
                *pTransaction, OTItem::payDividend);
            pItem->SetAmount(lTotalCostOfDividend); // <=== Notice, while the
                                                    // CHEQUE is for
                                                    // lAmountPerShare, the
                                                    // item's AMOUNT is set to
                                                    // lTotalCostOfDividend.
            OTString strNote("Pay Dividend: "); // The server just needs both of
                                                // those, so that's how we send
                                                // them (Similar to the voucher
                                                // code.)
            pItem->SetNote(strNote);

            // Add the voucher request string as the attachment on the
            // transaction item.
            theRequestVoucher.SignContract(*pNym);
            theRequestVoucher.SaveContract();
            OTString strVoucher(theRequestVoucher);
            pItem->SetAttachment(strVoucher); // The voucher request is
                                              // contained in the reference
                                              // string.

            // sign the item
            pItem->SignContract(*pNym);
            pItem->SaveContract();

            pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                           // cleanup the item. It "owns" it
                                           // now.
            // BALANCE AGREEMENT
            //
            // The item is signed and saved within this call as well. No need to
            // do that again.
            OTItem* pBalanceItem = pInbox->GenerateBalanceStatement(
                lTotalCostOfDividend * (-1), *pTransaction, *pNym,
                *pDividendSourceAccount, *pOutbox);

            // Notice the balance agreement is made for the "total cost of the
            // dividend", which we calculated as the issuer's
            // account balance, times -1, times the amount per share. So for
            // 100,000 shares of Pepsi, at a dividend payout of
            // $2 per share, then $200,000 must be removed from my dollar
            // account, in order to cover it. Therefore I sign a
            // balance agreement for $200,000. The server removes it all at
            // once, and then iterates through a loop, sending
            // vouchers to people. If any fail, or there is any left over, then
            // vouchers are sent back to pNym again, containing
            // the difference.
            // todo failsafe: We can't just loop, int64_t-term, and send a
            // voucher at the end. What if it crashes halfway through
            // the loop? It seems that the dividend payout still needs to be
            // "REGISTERED" somewhere until successfully completed.
            // (And therefore, that this concept must be repeated throughout OT
            // for other transactions, not just this example.)
            // This is already done with Cron, but just thinking about how to
            // best do it for "single action" transactions.

            if (nullptr != pBalanceItem)
                pTransaction->AddItem(
                    *pBalanceItem); // Better not be nullptr...
                                    // message will fail...
                                    // But better check
                                    // anyway.
            // sign the transaction
            pTransaction->SignContract(*pNym);
            pTransaction->SaveContract();

            // set up the ledger
            OTLedger theLedger(ISSUER_USER_ID, DIVIDEND_FROM_ACCT_ID,
                               SERVER_ID);
            theLedger.GenerateLedger(DIVIDEND_FROM_ACCT_ID, SERVER_ID,
                                     OTLedger::message); // bGenerateLedger
                                                         // defaults to false,
                                                         // which is correct.
            theLedger.AddTransaction(*pTransaction);

            // sign the ledger
            theLedger.SignContract(*pNym);
            theLedger.SaveContract();

            // extract the ledger in ascii-armored form
            OTString strLedger(theLedger);
            OTASCIIArmor ascLedger(strLedger);

            int64_t lRequestNumber = 0;

            // (0) Set up the REQUEST NUMBER and then INCREMENT IT
            pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
            theMessage.m_strRequestNum.Format(
                "%ld", lRequestNumber); // Always have to send this.
            pNym->IncrementRequestNum(*pNym, strServerID); // since I used it
                                                           // for a server
                                                           // request, I have to
                                                           // increment it

            // (1) Set up member variables
            theMessage.m_strCommand = "notarizeTransactions";
            theMessage.m_strNymID = strNymID;
            theMessage.m_strServerID = strServerID;
            theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                                  // theMessage.m_strServerID is
                                                  // already set. (It uses it.)

            theMessage.m_strAcctID = strFromAcct;
            theMessage.m_ascPayload = ascLedger;

            OTIdentifier NYMBOX_HASH;
            const std::string str_server(strServerID.Get());
            const bool bNymboxHash =
                pNym->GetNymboxHash(str_server, NYMBOX_HASH);
            NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

            if (!bNymboxHash)
                otErr << __FUNCTION__
                      << ": Failed getting NymboxHash from Nym for server: "
                      << str_server << "\n";

            // (2) Sign the Message
            theMessage.SignContract(*pNym);

            // (3) Save the Message (with signatures and all, back to its
            // internal member m_strRawFile.)
            theMessage.SaveContract();

            // (Send it)
            m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                              m_pTransportCallback);
            m_pClient->ProcessMessageOut(theMessage);

            return m_pClient->CalcReturnVal(lRequestNumber);
        }
    }
    else
        otOut << __FUNCTION__
              << ": No Transaction Numbers were available. "
                 "Suggest requesting the server for a new one.\n";

    return (-1);
}

int32_t OT_API::withdrawVoucher(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTIdentifier& ACCT_ID,
                                const OTIdentifier& RECIPIENT_USER_ID,
                                const OTString& CHEQUE_MEMO,
                                const int64_t& AMOUNT) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    OTIdentifier CONTRACT_ID;
    OTString strContractID;

    CONTRACT_ID = pAccount->GetAssetTypeID();
    CONTRACT_ID.GetString(strContractID);
    OTMessage theMessage;

    const int64_t lAmount = AMOUNT;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strFromAcct(ACCT_ID);

    int64_t lWithdrawTransNum = 0, lVoucherTransNum = 0;

    bool bGotTransNum1 =
        pNym->GetNextTransactionNum(*pNym, strServerID, lWithdrawTransNum);
    bool bGotTransNum2 =
        pNym->GetNextTransactionNum(*pNym, strServerID, lVoucherTransNum);

    if (!bGotTransNum1 || !bGotTransNum2) {
        otOut << __FUNCTION__
              << ": Not enough Transaction Numbers were available. "
                 "(Suggest requesting the server for more.)\n";

        if (bGotTransNum1)
            pNym->AddTransactionNum(*pNym, strServerID, lWithdrawTransNum,
                                    true); // bSave=true
        if (bGotTransNum2)
            pNym->AddTransactionNum(*pNym, strServerID, lVoucherTransNum,
                                    true); // bSave=true

        return (-1);
    }
    const OTString strChequeMemo(CHEQUE_MEMO);
    const OTString strRecipientUserID(RECIPIENT_USER_ID);
    // Expiration (ignored by server -- it sets its own for its vouchers.)
    const time64_t VALID_FROM =
        OTTimeGetCurrentTime(); // This time is set to TODAY NOW
    const time64_t VALID_TO = OTTimeAddTimeInterval(
        VALID_FROM,
        OTTimeGetSecondsFromTime(OT_TIME_SIX_MONTHS_IN_SECONDS)); // 6 months.
    // The server only uses the memo, amount, and recipient from this cheque
    // when it
    // constructs the actual voucher.
    OTCheque theRequestVoucher(SERVER_ID, CONTRACT_ID);
    bool bIssueCheque = theRequestVoucher.IssueCheque(
        lAmount, lVoucherTransNum, VALID_FROM, VALID_TO, ACCT_ID, USER_ID,
        strChequeMemo,
        (strRecipientUserID.GetLength() > 2) ? &(RECIPIENT_USER_ID) : nullptr);
    std::unique_ptr<OTLedger> pInbox(pAccount->LoadInbox(*pNym));
    std::unique_ptr<OTLedger> pOutbox(pAccount->LoadOutbox(*pNym));

    if (nullptr == pInbox) {
        otOut << "OT_API::" << __FUNCTION__
              << ": Failed loading inbox for acct " << strFromAcct << "\n";

        // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE NUMBERS.
        pNym->AddTransactionNum(*pNym, strServerID, lWithdrawTransNum,
                                true); // bSave=true
        pNym->AddTransactionNum(*pNym, strServerID, lVoucherTransNum,
                                true); // bSave=true
    }
    else if (nullptr == pOutbox) {
        otOut << "OT_API::" << __FUNCTION__
              << ": Failed loading outbox for acct " << strFromAcct << "\n";

        // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE NUMBERS.
        pNym->AddTransactionNum(*pNym, strServerID, lWithdrawTransNum,
                                true); // bSave=true
        pNym->AddTransactionNum(*pNym, strServerID, lVoucherTransNum,
                                true); // bSave=true
    }
    else if (!bIssueCheque) {
        // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE NUMBERS.
        pNym->AddTransactionNum(*pNym, strServerID, lWithdrawTransNum,
                                true); // bSave=true
        pNym->AddTransactionNum(*pNym, strServerID, lVoucherTransNum,
                                true); // bSave=true
    }
    else {
        // Create a transaction
        OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
            USER_ID, ACCT_ID, SERVER_ID, OTTransaction::withdrawal,
            lWithdrawTransNum);
        // set up the transaction item (each transaction may have multiple
        // items...)
        OTItem* pItem = OTItem::CreateItemFromTransaction(
            *pTransaction, OTItem::withdrawVoucher);
        pItem->SetAmount(lAmount);
        OTString strNote(" ");
        pItem->SetNote(strNote);

        // Add the voucher request string as the attachment on the transaction
        // item.
        theRequestVoucher.SignContract(*pNym);
        theRequestVoucher.SaveContract();
        OTString strVoucher(theRequestVoucher);
        pItem->SetAttachment(strVoucher); // The voucher request is contained in
                                          // the reference string.

        // sign the item
        pItem->SignContract(*pNym);
        pItem->SaveContract();

        pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                       // cleanup the item. It "owns" it now.
        // BALANCE AGREEMENT
        //
        // The item is signed and saved within this call as well. No need to do
        // that again.
        OTItem* pBalanceItem = pInbox->GenerateBalanceStatement(
            lAmount * (-1), *pTransaction, *pNym, *pAccount, *pOutbox);

        if (nullptr != pBalanceItem)
            pTransaction->AddItem(*pBalanceItem); // Better not be nullptr...
                                                  // message will fail... But
                                                  // better check anyway.
        // sign the transaction
        pTransaction->SignContract(*pNym);
        pTransaction->SaveContract();

        // set up the ledger
        OTLedger theLedger(USER_ID, ACCT_ID, SERVER_ID);
        theLedger.GenerateLedger(ACCT_ID, SERVER_ID,
                                 OTLedger::message); // bGenerateLedger defaults
                                                     // to false, which is
                                                     // correct.
        theLedger.AddTransaction(*pTransaction);

        // sign the ledger
        theLedger.SignContract(*pNym);
        theLedger.SaveContract();

        // extract the ledger in ascii-armored form
        OTString strLedger(theLedger);
        OTASCIIArmor ascLedger(strLedger);

        int64_t lRequestNumber = 0;

        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%ld", lRequestNumber); // Always have to send this.
        pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                       // server request, I have
                                                       // to increment it

        // (1) Set up member variables
        theMessage.m_strCommand = "notarizeTransactions";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strServerID = strServerID;
        theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                              // theMessage.m_strServerID is
                                              // already set. (It uses it.)

        theMessage.m_strAcctID = strFromAcct;
        theMessage.m_ascPayload = ascLedger;

        OTIdentifier NYMBOX_HASH;
        const std::string str_server(strServerID.Get());
        const bool bNymboxHash = pNym->GetNymboxHash(str_server, NYMBOX_HASH);
        NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

        if (!bNymboxHash)
            otErr << __FUNCTION__
                  << ": Failed getting NymboxHash from Nym for server: "
                  << str_server << "\n";

        // (2) Sign the Message
        theMessage.SignContract(*pNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        // (Send it)
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return m_pClient->CalcReturnVal(lRequestNumber);
    }

    return (-1);
}

//
// DISCARD CHEQUE (recover the transaction number for re-use, so the cheque
// can be discarded.)
//
// NOTE: this function is only for cheques that haven't been sent to anyone.
// Once a cheque has been sent to someone, more is necessary to cancel the
// cheque -- you must recover the transaction number on the server side,
// not just on the client side. (So if someone tries to deposit it, the cheque
// is no good since the transaction number is already closed out.)
//
// Hmm -- but the cheque goes into the outbox as soon as it's written. Plus,
// just because I didn't send the cheque through OT, doesn't mean I didn't send
// him a copy in the email. So I really need to close out the number either
// way. I think the easiest way to do this is to alter the NotarizeDepositCheque
// (on server side) so that if you deposit a cheque back into the same account
// it's drawn on, that it cancels the cheque and closes out the transaction
// number. So "cheque cancellation" will just be handled by the deposit
// function.
//
// Therefore this "discard cheque" function will probably only be used
// internally
// by the high-level API, for certain special harvesting cases where the cheque
// hasn't possibly been sent or used anywhere when it's discarded.
//
// Voucher update: now that the remitter's transaction number is used on
// vouchers,
// you would think that we would have to retrofit this function to support
// vouchers
// as well. However, that's not the case. The reason is because vouchers must be
// withdrawn at the server, which means creating a voucher automatically implies
// that the server has already marked the transaction number as "in use."
// BUT ACTUALLY I'M WRONG ABOUT THAT! The server doesn't mark it as "in use"
// until
// it's DEPOSITED -- and then marks it as "closed" once the voucherReceipt is
// processed. That might seem strange -- the server issues a voucher, drawn on
// its own account, without marking its transaction number as "in use" ? Reason
// is,
// the instrument still cannot actually be used without depositing it, at which
// time
// the transaction number WILL be marked as "in use." Until then, you could
// recover
// the number and use it somewhere else instead. But why the hell would you do
// that?
// Because once you do that, you can no longer use it with the voucher, which
// means
// you can no longer recover any of the money that you sent to the server, when
// you
// purchased that voucher in the first place. Therefore you would NEVER want to
// just
// "discard" a voucher like you might with a cheque -- that voucher cost you
// money!
// Basically you would DEFINITELY want to REFUND that voucher and get that money
// BACK,
// and not merely re-use the number on it. Therefore vouchers will never just be
// "discarded" but rather, stored in the outpayment box and REFUNDED if
// necessary.
// (Therefore we won't be retrofitting this function for vouchers.)
//
bool OT_API::DiscardCheque(const OTIdentifier& SERVER_ID,
                           const OTIdentifier& USER_ID,
                           const OTIdentifier& ACCT_ID,
                           const OTString& THE_CHEQUE) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return false;
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return false;
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return false;
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    OTIdentifier CONTRACT_ID;
    OTString strContractID;

    CONTRACT_ID = pAccount->GetAssetTypeID();
    CONTRACT_ID.GetString(strContractID);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)  pServer and pAccount are also good.
    //
    const OTString strServerID(SERVER_ID), strNymID(USER_ID);

    OTCheque theCheque(SERVER_ID, CONTRACT_ID);

    if (!theCheque.LoadContractFromString(THE_CHEQUE)) {
        otOut << __FUNCTION__ << ": Unable to load cheque from string. Sorry. "
                                 "Cheque contents:\n\n" << THE_CHEQUE << "\n\n";
        return false;
    }
    else if ((theCheque.GetServerID() == SERVER_ID) &&
               (theCheque.GetAssetID() == CONTRACT_ID) &&
               (theCheque.GetSenderUserID() == USER_ID) &&
               (theCheque.GetSenderAcctID() == ACCT_ID)) {
        if (pNym->VerifyIssuedNum(
                strServerID, theCheque.GetTransactionNum())) // we only "add it
                                                             // back" if it was
                                                             // really there in
                                                             // the first place.
        {
            pNym->AddTransactionNum(*pNym, strServerID,
                                    theCheque.GetTransactionNum(),
                                    true); // bSave=true
            return true;
        }
        else // No point adding it back as available to use, if pNym doesn't
               // even have it signed out!
        {
            otOut << __FUNCTION__
                  << ": Failed attempt to claw back a transaction number that "
                     "wasn't signed out to pNym in the first place. "
                     "Cheque contents:\n\n" << THE_CHEQUE << "\n\n";
            return false;
        }
    } // bSuccess
    else
        otOut << __FUNCTION__
              << ": Unable to verify cheque's server ID, asset type "
                 "ID, user ID, or acct ID. Sorry. "
                 "Cheque contents:\n\n" << THE_CHEQUE << "\n\n";
    return false;
}

// DEPOSIT CHEQUE
//
// NOTE: If THE_CHEQUE is drawn on the account denoted by ACCT_ID (meaning
// it's being deposited back into the same account that originally wrote
// the cheque) this means the original cheque writer is CANCELLING the
// cheque, to prevent the recipient from depositing it.

int32_t OT_API::depositCheque(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& USER_ID,
                              const OTIdentifier& ACCT_ID,
                              const OTString& THE_CHEQUE) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // These copiously log, and ASSERT.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    OTIdentifier CONTRACT_ID;
    OTString strContractID;
    CONTRACT_ID = pAccount->GetAssetTypeID();
    CONTRACT_ID.GetString(strContractID);
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strDepositAcct(ACCT_ID);

    OTCheque theCheque(SERVER_ID, CONTRACT_ID);

    int64_t lStoredTransactionNumber = 0;
    bool bGotTransNum = pNym->GetNextTransactionNum(*pNym, strServerID,
                                                    lStoredTransactionNumber);

    if (!bGotTransNum)
        otOut << __FUNCTION__ << ": No transaction numbers were available. "
                                 "Try requesting the server for a new one.\n";
    else if (!theCheque.LoadContractFromString(THE_CHEQUE)) {
        otOut << __FUNCTION__
              << ": Unable to load cheque from string. Sorry. Contents:\n\n"
              << THE_CHEQUE << "\n\n";
        // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE NUMBERS.
        pNym->AddTransactionNum(*pNym, strServerID, lStoredTransactionNumber,
                                true); // bSave=true
    }
    else if (theCheque.GetServerID() != SERVER_ID) {
        const OTString strChequeServerID(theCheque.GetServerID());
        otOut << __FUNCTION__ << ": ServerID on cheque (" << strChequeServerID
              << ") doesn't "
                 "match serverID where it's being deposited to (" << strServerID
              << ").";
        // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE NUMBERS.
        pNym->AddTransactionNum(*pNym, strServerID, lStoredTransactionNumber,
                                true); // bSave=true
    }
    else {
        std::unique_ptr<OTLedger> pInbox(pAccount->LoadInbox(*pNym));

        if (nullptr == pInbox) {
            otOut << __FUNCTION__ << ": Failed loading inbox for acct "
                  << strDepositAcct << "\n";
            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            pNym->AddTransactionNum(*pNym, strServerID,
                                    lStoredTransactionNumber,
                                    true); // bSave=true
            return -1;
        }
        // If bCancellingCheque==true, we're actually cancelling the cheque by
        // "depositing"
        // it back into the same account it's drawn on.
        //
        bool bCancellingCheque = false;

        if (theCheque.HasRemitter())
            bCancellingCheque = ((theCheque.GetRemitterAcctID() == ACCT_ID) &&
                                 (theCheque.GetRemitterUserID() == USER_ID));
        else {
            bCancellingCheque = ((theCheque.GetSenderAcctID() == ACCT_ID) &&
                                 (theCheque.GetSenderUserID() == USER_ID));
            if (bCancellingCheque)
                bCancellingCheque = theCheque.VerifySignature(*pNym);
        }
        if (bCancellingCheque) // By this point he's definitely TRYING to cancel
                               // the cheque.
        {
            bCancellingCheque = pNym->VerifyIssuedNum(
                strServerID, theCheque.GetTransactionNum());

            // If we TRIED to cancel the cheque (being in this block...) yet the
            // signature fails
            // to verify, or the transaction number isn't even issued, then our
            // attempt to cancel
            // the cheque is going to fail.
            if (!bCancellingCheque) {
                // This is the "tried and failed" block.
                //
                otOut << __FUNCTION__
                      << ": Cannot cancel this cheque. Either the "
                         "signature fails to verify,\n"
                         "or the transaction number is already closed "
                         "out. (Failure.) Cheque contents:\n\n" << THE_CHEQUE
                      << "\n\n";

                // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
                // NUMBERS.
                pNym->AddTransactionNum(*pNym, strServerID,
                                        lStoredTransactionNumber,
                                        true); // bSave=true
                return (-1);
            }
            // Else we succeeded in verifying signature and issued num.
            // Let's just make sure there isn't a chequeReceipt or
            // voucherReceipt
            // already sitting in the inbox, for this same cheque.
            //
            OTTransaction* pChequeReceipt =
                pInbox->GetChequeReceipt(theCheque.GetTransactionNum());

            if (nullptr != pChequeReceipt) // Hmm looks like there's ALREADY a
                                           // chequeReceipt in the inbox (so we
                                           // can't cancel it.)
            {
                otOut << __FUNCTION__
                      << ": Cannot cancel this cheque. There is "
                         "already a "
                      << (theCheque.HasRemitter() ? "voucherReceipt"
                                                  : "chequeReceipt")
                      << " for it in the inbox. "
                         "(Failure.) Cheque contents:\n\n" << THE_CHEQUE
                      << "\n\n";
                // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
                // NUMBERS.
                pNym->AddTransactionNum(*pNym, strServerID,
                                        lStoredTransactionNumber,
                                        true); // bSave=true
                return (-1);
            }
        }
        // By this point, we're either NOT cancelling the cheque, or if we are,
        // we've already
        // verified the signature and transaction number on the cheque. (AND
        // we've already
        // verified that there aren't any chequeReceipts for this cheque, in the
        // inbox.)
        //
        if (bCancellingCheque && !theCheque.HasRemitter()) {
            theCheque.CancelCheque();      // Sets the amount to zero.
            theCheque.ReleaseSignatures(); // Usually when you deposit a cheque,
                                           // it's signed by someone else.
            theCheque.SignContract(*pNym); // But if we are CANCELING a cheque,
                                           // that means we wrote that cheque
            theCheque.SaveContract();      // originally, so we are the original
                                           // signer.
        }                                  // cancelling cheque
        // Create a transaction
        std::unique_ptr<OTTransaction> pTransaction(
            OTTransaction::GenerateTransaction(USER_ID, ACCT_ID, SERVER_ID,
                                               OTTransaction::deposit,
                                               lStoredTransactionNumber));

        // set up the transaction item (each transaction may have multiple
        // items...)
        OTItem* pItem = OTItem::CreateItemFromTransaction(
            *pTransaction, OTItem::depositCheque);

        OTString strNote(bCancellingCheque
                             ? "Cancel this cheque, please!"
                             : "Deposit this cheque, please!"); // todo
        pItem->SetNote(strNote);

        OTString strCheque(theCheque); // <===== THE CHEQUE

        // Add the cheque string as the attachment on the transaction item.
        pItem->SetAttachment(
            strCheque); // The cheque is contained in the reference string.

        // sign the item
        pItem->SignContract(*pNym);
        pItem->SaveContract();

        // the Transaction "owns" the item now and will handle cleaning it up.
        pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                       // cleanup the item. It "owns" it now.
        std::unique_ptr<OTLedger> pOutbox(pAccount->LoadOutbox(*pNym));

        if (nullptr == pOutbox) {
            otOut << "OT_API::depositCheque: Failed loading outbox for acct "
                  << strDepositAcct << "\n";
            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            pNym->AddTransactionNum(*pNym, strServerID,
                                    lStoredTransactionNumber,
                                    true); // bSave=true
        }
        else {
            // BALANCE AGREEMENT
            // pBalanceItem is signed and saved within this call. No need to do
            // that twice.
            //
            OTItem* pBalanceItem = pInbox->GenerateBalanceStatement(
                theCheque.GetAmount(), *pTransaction, *pNym, *pAccount,
                *pOutbox);

            if (nullptr !=
                pBalanceItem) // will never be nullptr. Will assert above
                              // before it gets here.
                pTransaction->AddItem(
                    *pBalanceItem); // Better not be nullptr...
                                    // message will fail...
                                    // But better check
                                    // anyway.
            // else log
            // sign the transaction
            pTransaction->SignContract(*pNym);
            pTransaction->SaveContract();

            // set up the ledger
            OTLedger theLedger(USER_ID, ACCT_ID, SERVER_ID);
            theLedger.GenerateLedger(ACCT_ID, SERVER_ID,
                                     OTLedger::message); // bGenerateLedger
                                                         // defaults to false,
                                                         // which is correct.
            theLedger.AddTransaction(*pTransaction); // now the ledger "owns"
                                                     // and will handle cleaning
                                                     // up the transaction.
            pTransaction.release();

            // sign the ledger
            theLedger.SignContract(*pNym);
            theLedger.SaveContract();

            // extract the ledger in ascii-armored form... encoding...
            OTString strLedger(theLedger);
            OTASCIIArmor ascLedger(strLedger);

            // (0) Set up the REQUEST NUMBER and then INCREMENT IT
            pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
            theMessage.m_strRequestNum.Format(
                "%ld", lRequestNumber); // Always have to send this.
            pNym->IncrementRequestNum(*pNym, strServerID); // since I used it
                                                           // for a server
                                                           // request, I have to
                                                           // increment it

            // (1) Set up member variables
            theMessage.m_strCommand = "notarizeTransactions";
            theMessage.m_strNymID = strNymID;
            theMessage.m_strServerID = strServerID;
            theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                                  // theMessage.m_strServerID is
                                                  // already set. (It uses it.)

            theMessage.m_strAcctID = strDepositAcct;
            theMessage.m_ascPayload = ascLedger;

            OTIdentifier NYMBOX_HASH;
            const std::string str_server(strServerID.Get());
            const bool bNymboxHash =
                pNym->GetNymboxHash(str_server, NYMBOX_HASH);
            NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

            if (!bNymboxHash)
                otErr << "Failed getting NymboxHash from Nym for server: "
                      << str_server << "\n";

            // (2) Sign the Message
            theMessage.SignContract(*pNym);

            // (3) Save the Message (with signatures and all, back to its
            // internal member m_strRawFile.)
            theMessage.SaveContract();

            // (Send it)
            m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                              m_pTransportCallback);
            m_pClient->ProcessMessageOut(theMessage);

            return m_pClient->CalcReturnVal(lRequestNumber);
        }
    } // bSuccess

    return -1;
}

// DEPOSIT PAYMENT PLAN
//
// The Recipient Creates the Payment Plan using ProposePaymentPlan.
// Then the sender Confirms it using ConfirmPaymentPlan.
// Both of the above steps involve attaching transaction numbers to
// the payment plan and signing copies of it.
// This function here is the final step, where the payment plan
// contract is now being deposited by the customer (who is also
// the sender), in a message to the server.
//
int32_t OT_API::depositPaymentPlan(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   const OTString& THE_PAYMENT_PLAN) const
{
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer.  (No need to cleanup.)
    OTPaymentPlan thePlan;
    OTMessage theMessage;

    const OTString strServerID(SERVER_ID), strNymID(USER_ID);

    if (thePlan.LoadContractFromString(THE_PAYMENT_PLAN) &&
        thePlan.VerifySignature(*pNym)) {
        int64_t lRequestNumber = 0;
        const bool bCancelling = (thePlan.GetRecipientUserID() == USER_ID);

        if (bCancelling) {
            if (thePlan.IsCanceled()) {
                otErr << __FUNCTION__
                      << ": Error: attempted to cancel (pre-emptively, "
                         "before activation) a payment plan "
                         "that was already set as canceled.\n";
                return (-1);
            }
            else if (!thePlan.CancelBeforeActivation(
                           *pNym)) // releases signatures, signs, saves.
            {
                otErr << __FUNCTION__
                      << ": Error: attempted to cancel (pre-emptively, "
                         "before activation) a payment plan, "
                         "but the attempt somehow failed.\n";
                return (-1);
            }
        }

        // The logic for DEPOSITOR_ACCT_ID comes about because normally the
        // sender is the one
        // who activates the payment plan. BUT the recipient might ALSO activate
        // it as a way
        // of CANCELLING it. So we check to see if the recipient user is the
        // same as USER_ID.
        // If so, then we use the RECIPIENT account here (it's being cancelled.)
        // Otherwise,
        // we use the sender account ID here as normal (it's being activated.)
        //
        const OTIdentifier DEPOSITOR_ACCT_ID(bCancelling
                                                 ? thePlan.GetRecipientAcctID()
                                                 : thePlan.GetSenderAcctID());
        const OTString strDepositAcct(DEPOSITOR_ACCT_ID);
        OTAccount* pAccount =
            GetOrLoadAccount(*pNym, DEPOSITOR_ACCT_ID, SERVER_ID, __FUNCTION__);
        if (nullptr == pAccount) return (-1);
        // By this point, pAccount is a good pointer and in the wallet.
        // (No need to cleanup.) I also know it has the right Server ID
        // and that the Nym owns it, and has signed it, etc.
        // If the depositor is the recipient, then it's not properly confirmed
        // yet, and he's
        // actually cancelling it here. The transaction number on the plan will
        // still be set to 0.
        // Therefore he needs to use a fresh transaction number to perform this
        // action.
        // Otherwise, if the depositor is the sender (as should normally be)
        // then the plan is
        // actually being activated, and has already been properly confirmed,
        // and will thus
        // already have its own transaction number set on it.
        //
        const int64_t lTransactionNum = thePlan.GetOpeningNumber(USER_ID);
        // Create a transaction
        OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
            USER_ID, DEPOSITOR_ACCT_ID, SERVER_ID, OTTransaction::paymentPlan,
            lTransactionNum);
        // set up the transaction item (each transaction may have multiple
        // items...)
        OTItem* pItem = OTItem::CreateItemFromTransaction(*pTransaction,
                                                          OTItem::paymentPlan);

        OTString strPlan(thePlan);

        // Add the payment plan string as the attachment on the transaction
        // item.
        pItem->SetAttachment(
            strPlan); // The payment plan is contained in the reference string.

        // sign the item
        pItem->SignContract(*pNym);
        pItem->SaveContract();

        // the Transaction "owns" the item now and will handle cleaning it up.
        pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                       // cleanup the item. It "owns" it now.

        // TRANSACTION AGREEMENT

        // pBalanceItem is signed and saved within this call. No need to do that
        // again.
        OTItem* pStatementItem =
            pNym->GenerateTransactionStatement(*pTransaction);

        if (nullptr !=
            pStatementItem) // will never be nullptr. Will assert above
                            // before it gets here.
            pTransaction->AddItem(*pStatementItem); // Better not be nullptr...
                                                    // message will fail... But
                                                    // better check anyway.
        // sign the transaction
        pTransaction->SignContract(*pNym);
        pTransaction->SaveContract();

        // set up the ledger
        OTLedger theLedger(USER_ID, DEPOSITOR_ACCT_ID, SERVER_ID);
        theLedger.GenerateLedger(DEPOSITOR_ACCT_ID, SERVER_ID,
                                 OTLedger::message); // bGenerateLedger defaults
                                                     // to false, which is
                                                     // correct.
        theLedger.AddTransaction(*pTransaction); // now the ledger "owns" and
                                                 // will handle cleaning up the
                                                 // transaction.

        // sign the ledger
        theLedger.SignContract(*pNym);
        theLedger.SaveContract();

        // extract the ledger in ascii-armored form... encoding...
        OTString strLedger(theLedger);
        OTASCIIArmor ascLedger(strLedger);

        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%ld", lRequestNumber); // Always have to send this.
        pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                       // server request, I have
                                                       // to increment it

        // (1) Set up member variables
        theMessage.m_strCommand = "notarizeTransactions";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strServerID = strServerID;
        theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                              // theMessage.m_strServerID is
                                              // already set. (It uses it.)

        theMessage.m_strAcctID = strDepositAcct;
        theMessage.m_ascPayload = ascLedger;

        OTIdentifier NYMBOX_HASH;
        const std::string str_server(strServerID.Get());
        const bool bNymboxHash = pNym->GetNymboxHash(str_server, NYMBOX_HASH);
        NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

        if (!bNymboxHash)
            otErr << "Failed getting NymboxHash from Nym for server: "
                  << str_server << "\n";

        // (2) Sign the Message
        theMessage.SignContract(*pNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        // (Send it)
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return m_pClient->CalcReturnVal(lRequestNumber);
    } // thePlan.LoadContractFromString()
    else {
        otOut << "Unable to load payment plan from string, or verify it. "
                 "Sorry.\n";
    }

    return (-1);
}

// If a smart contract is already running on a specific server, and the Nym in
// question (USER_ID)
// is an authorized agent for that smart contract, then he can trigger clauses.
// All he needs is
// the transaction ID for the smart contract, and the name of the clause.
//
int32_t OT_API::triggerClause(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& USER_ID,
                              const int64_t& lTransactionNum,
                              const OTString& strClauseName,
                              const OTString* pStrParam) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "triggerClause";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_lTransactionNum = lTransactionNum;
    theMessage.m_strNymID2 = strClauseName;

    // Optional string parameter. Available as "param_string"
    // inside the script.
    //
    if ((nullptr != pStrParam) && (pStrParam->Exists()))
        theMessage.m_ascPayload.SetString(*pStrParam); // <===

    OTIdentifier NYMBOX_HASH;
    const std::string str_server(strServerID.Get());
    const bool bNymboxHash = pNym->GetNymboxHash(str_server, NYMBOX_HASH);
    NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

    if (!bNymboxHash)
        otErr << "Failed getting NymboxHash from Nym for server: " << str_server
              << "\n";

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

int32_t OT_API::activateSmartContract(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& USER_ID,
                                      const OTString& THE_SMART_CONTRACT) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTSmartContract theContract(SERVER_ID);
    OTMessage theMessage;
    const OTString strServerID(SERVER_ID), strNymID(USER_ID);

    if (theContract.LoadContractFromString(THE_SMART_CONTRACT)) {
        int64_t lRequestNumber = 0;
        OTAgent* pAgent = nullptr;
        OTParty* pParty =
            theContract.FindPartyBasedOnNymAsAuthAgent(*pNym, &pAgent);
        if (nullptr == pParty) {
            otOut << __FUNCTION__
                  << ": Failure: USER_ID *IS* a valid Nym, but that "
                     "Nym is not the authorizing agent for any "
                     "of the parties on this contract. Try calling "
                     "ConfirmParty() first.\n";
            return (-1);
        }
        OT_ASSERT(nullptr != pAgent);
        //
        // BELOW THIS POINT, pAgent and pParty are both valid, and no need to
        // clean them up.
        // Note: Usually, payment plan or market offer will load up the Nym and
        // accounts,
        // and verify ownership, etc.
        // But in this case, the Nym who actually activates the smart contract
        // is merely
        // the authorizing agent for a single party, where there may be a dozen
        // parties listed
        // on the actual contract.
        //
        // It would be unreasonable to expect a party to have EVERY nym and
        // EVERY account
        // to the entire contract. As long as the Nym is legitimately the
        // authorizing agent
        // for one of the parties, then we allow him to send the activation
        // message to the server.
        //
        // (Let the server sort out the trouble of loading all the nyms, loading
        // all the accounts,
        // verifying all the asset type IDs, verifying all the agent names,
        // making sure there
        // are no stashes already created, ETC ETC.)
        //
        // ONE THING I should verify, is that all of the parties are confirmed.
        // I mean, it's not
        // even worth the bother to send the damn thing until then, right? And
        // the Server ID.
        //

        if (false ==
            theContract.AllPartiesHaveSupposedlyConfirmed()) // all parties are
                                                             // NOT confirmed.
        {
            //            otOut << __FUNCTION__ << ": Failed. EACH PARTY to this
            // smart contract needs to CONFIRM IT FIRST, before one of them "
            //                           "then activates it at the server. (But
            // THIS smart contract has NOT yet been confirmed, at least, "
            //                           "not by all of its parties.)\n";
            //            return -1;

            // UPDATE: This is now how we will trigger the "cancel smart
            // contract" functionality, which can
            // only be performed BEFORE the contract has been activated.

            otOut << "Not all parties to smart contract are "
                     "confirmed. Treating this as a request for "
                     "cancelation...\n";

            if (theContract.IsCanceled()) {
                otErr << __FUNCTION__
                      << ": Error: attempted to cancel (pre-emptively, "
                         "before activation) a smart contract "
                         "that was already set as canceled.\n";
                return (-1);
            }
            else if (!theContract.CancelBeforeActivation(
                           *pNym)) // sets as canceler, releases signatures,
                                   // signs, saves.
            {
                otErr << __FUNCTION__
                      << ": Error: attempted to cancel (pre-emptively, "
                         "before activation) a smart contract, "
                         "but the attempt somehow failed.\n";
                return (-1);
            }
        }
        if (SERVER_ID != theContract.GetServerID()) {
            otOut << __FUNCTION__
                  << ": Failed. The server ID passed in doesn't "
                     "match the one on the contract itself.\n";
            return -1;
        }
        if (!(pParty->GetAccountCount() > 0)) {
            otOut << __FUNCTION__
                  << ": Failed. The activating Nym must not only be "
                     "the authorizing agent for one of the parties, "
                     "but must also be the authorized agent for one of "
                     "that party's asset accounts. That party must have "
                     "at least one asset account "
                     "for this reason. (See code comment below this "
                     "message, in the code.)\n";
            return -1;
        }
        //
        // REQUIREMENT:  The ACTIVATOR aka the Originator Nym (the party who
        // activates the smart
        // contract on the server) must have at least one asset account as part
        // of the smart contract,
        // for which the authorizing agent for that party is also the authorized
        // agent for that account.
        // This is in order to make sure that the smart contracts will work with
        // the existing infrastructure,
        // so that functions like "GetOpeningNumber()" and "GetClosingNumber()"
        // will continue to work the way
        // they are expected to, and so that there is always at least ONE
        // closing transaction number supplied
        // for the smart contract (at least ONE finalReceipt will actually have
        // to be signed for once it is
        // deactivated) and so we can know who the owner of that account, and
        // what acct # it will be.
        //
        // This is also a requirement because the existing infrastructure uses
        // the transaction system for
        // activating cron items. It was simply not anticipated in the past that
        // cron items, based on valid
        // transactions, wouldn't also therefore be associated with at least one
        // asset account. In fact,
        // the original difference between transactions (vs normal messages) was
        // that transactions dealt
        // with asset accounts, whereas normal messages did not. (Such as,
        // "checkUser" or "getRequest".)
        //
        // A big piece of this is the BALANCE AGREEMENT. Obviously it wasn't
        // anticipated that "balance
        // agreements" would ever be needed for transactions, yet asset accounts
        // wouldn't be. So the below
        // TRANSACTION STATEMENT, for example, expects an asset account, as does
        // the verification code
        // associated with it. Even transaction statements were originally made
        // for situations where the balance
        // wasn't actually changing, (and so a balance agreement wasn't needed),
        // but where a signature on
        // the transaction numbers was still necessary, since one had to be
        // burned in order to prove that
        // the instrument was valid.
        //
        // Unfortunately, transaction statements still expected an asset account
        // to at least EXIST, since
        // they involved market offers (which do not immediately change the
        // balance) or payment plans
        // (which also do not immediately change the balance.) Yet the account's
        // ID was still at least
        // present in the receipt, and a transaction # was still used.
        //
        // In the case of smart contracts, I will probably someday lift this
        // restriction (that the activating
        // nym must also be the authorized agent on one of the asset accounts on
        // the smart contract). But I
        // will have to go over all the plumbing first and deal with that. In
        // the meantime:
        //
        // --- THE ACTIVATING NYM *MUST* ALSO BE THE AUTHORIZED AGENT
        //     FOR AT LEAST ONE ASSET ACCOUNT, FOR THAT PARTY. ---
        //

        const std::string str_agent_name(pAgent->GetName().Get());
        OTPartyAccount* pAcct = pParty->GetAccountByAgent(str_agent_name);

        if (nullptr == pAcct) {
            otOut << __FUNCTION__
                  << ": Failed. The activating Nym must not only be "
                     "the authorizing agent for one of the parties, "
                     "but must also be the authorized agent for one of "
                     "that party's asset accounts. That party must have "
                     "at least one asset account "
                     "for this reason. (See code comment above this "
                     "message, in the code.)\n";
            return -1;
        }
        const OTString& strAcctID = pAcct->GetAcctID();

        if (!strAcctID.Exists()) {
            otOut << __FUNCTION__
                  << ": Failed. The Account ID is blank for asset acct ("
                  << pAcct->GetName() << ") for party ("
                  << pParty->GetPartyName()
                  << "). Did you confirm this account, before trying to "
                     "activate this contract?\n";
            return -1;
        }
        OTIdentifier theAcctID(strAcctID);
        const int64_t lOpeningTransNo = pParty->GetOpeningTransNo();
        const int64_t lClosingTransNo = pAcct->GetClosingTransNo();

        if ((lOpeningTransNo <= 0) || (lClosingTransNo <= 0)) {
            otOut << __FUNCTION__ << ": Failed. Opening Transaction # ("
                  << lOpeningTransNo << ") or "
                                        "Closing # (" << lClosingTransNo
                  << ") were invalid "
                     "for asset acct (" << pAcct->GetName() << ") for party ("
                  << pParty->GetPartyName()
                  << "). Did you "
                     "confirm this account and party, before trying to "
                     "activate this contract?\n";
            return -1;
        }
        if (!pNym->VerifyIssuedNum(strServerID, lOpeningTransNo)) {
            otOut << __FUNCTION__ << ": Failed. Opening Transaction # ("
                  << lOpeningTransNo << ") wasn't "
                                        "valid/issued to this Nym, "
                                        "for asset acct (" << pAcct->GetName()
                  << ") for party (" << pParty->GetPartyName() << ") on server "
                                                                  "("
                  << strServerID
                  << "). Did you confirm this account and party, "
                     "before trying to activate this contract?\n";
            return -1;
        }
        if (!pNym->VerifyIssuedNum(strServerID, lClosingTransNo)) {
            otOut << __FUNCTION__ << ": Failed. Closing Transaction # ("
                  << lClosingTransNo << ") wasn't "
                                        "issued to this Nym, "
                                        "for asset acct (" << pAcct->GetName()
                  << ") for party (" << pParty->GetPartyName()
                  << "). Did you "
                     "confirm this account and party, "
                     "before trying to activate this contract?\n";
            return -1;
        }
        // These numbers (the opening and closing transaction #s) were already
        // available on
        // the smart contract -- specifically an opening number is stored for
        // each party,
        // taken from the authorizing agent for that party, and a closing number
        // is stored for
        // each asset account, taken from the authorized agent for that asset
        // account.
        //
        // Now we set the appropriate opening/closing number onto the smart
        // contract, in the way
        // of the existing CronItem system, so that it will work properly within
        // that infrastructure.
        // (This is what makes the activating Nym slightly different / more than
        // the other authorizing
        // agent nyms for each party to the contract. Not only does it have
        // opening/closing numbers
        // on its party like all the others, but its numbers are also those used
        // for the cron item itself.)
        //
        theContract.PrepareToActivate(lOpeningTransNo, lClosingTransNo, USER_ID,
                                      theAcctID);
        // This call changes the contract slightly, so it must be re-signed (in
        // order to save changes.)
        theContract.ReleaseSignatures();

        if (!pAgent->SignContract(theContract)) // RE-SIGN HERE.
        {
            otErr << __FUNCTION__
                  << ": Failed re-signing contract, after calling "
                     "PrepareToActivate().\n";
            return -1;
        }

        theContract.SaveContract();
        const OTString strContract(theContract); // Grab a string version of the
                                                 // latest signed contract.
        // Create a transaction
        //
        OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
            USER_ID, theAcctID, SERVER_ID, OTTransaction::smartContract,
            theContract.GetTransactionNum());

        // set up the transaction item (each transaction may have multiple
        // items...)
        OTItem* pItem = OTItem::CreateItemFromTransaction(
            *pTransaction, OTItem::smartContract);

        // Add the smart contract string as the attachment on the transaction
        // item.
        pItem->SetAttachment(strContract);

        // sign the item
        pItem->SignContract(*pNym);
        pItem->SaveContract();

        // the Transaction "owns" the item now and will handle cleaning it up.
        pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                       // cleanup the item. It "owns" it now.
        // TRANSACTION AGREEMENT

        // pStatementItem is signed and saved within this call. No need to do
        // that again.
        OTItem* pStatementItem =
            pNym->GenerateTransactionStatement(*pTransaction);

        if (nullptr !=
            pStatementItem) // Will never be nullptr. Will assert above
                            // before it gets here.
            pTransaction->AddItem(*pStatementItem); // Better not be nullptr...
                                                    // message will fail... But
                                                    // better check anyway.
        // sign the transaction
        pTransaction->SignContract(*pNym);
        pTransaction->SaveContract();
        // set up the ledger
        OTLedger theLedger(USER_ID, theAcctID, SERVER_ID);

        theLedger.GenerateLedger(theAcctID, SERVER_ID,
                                 OTLedger::message); // bGenerateLedger defaults
                                                     // to false, which is
                                                     // correct.
        theLedger.AddTransaction(*pTransaction); // now the ledger "owns" and
                                                 // will handle cleaning up the
                                                 // transaction.
        // sign the ledger
        theLedger.SignContract(*pNym);
        theLedger.SaveContract();
        // extract the ledger in ascii-armored form... encoding...
        OTString strLedger(theLedger);
        OTASCIIArmor ascLedger(strLedger);
        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%ld", lRequestNumber); // Always have to send this.
        pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                       // server request, I have
                                                       // to increment it
        // (1) Set up member variables
        theMessage.m_strCommand = "notarizeTransactions";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strServerID = strServerID;
        theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                              // theMessage.m_strServerID is
                                              // already set. (It uses it.)
        theAcctID.GetString(theMessage.m_strAcctID);
        theMessage.m_ascPayload = ascLedger;
        OTIdentifier NYMBOX_HASH;
        const std::string str_server(strServerID.Get());
        const bool bNymboxHash = pNym->GetNymboxHash(str_server, NYMBOX_HASH);
        NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);
        if (!bNymboxHash)
            otErr << __FUNCTION__
                  << ": Failed getting NymboxHash from Nym for server: "
                  << str_server << "\n";
        // (2) Sign the Message
        theMessage.SignContract(*pNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        // (Send it)
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);
        return m_pClient->CalcReturnVal(lRequestNumber);

    } // theContract.LoadContractFromString()
    else
        otOut << __FUNCTION__
              << ": Unable to load smart contract from string:\n\n"
              << THE_SMART_CONTRACT << "\n\n";

    return (-1);
}

// Done: make a new transaction (and item) type for smart contract. (for a cron
// item.)

// Done: Add an API call that allows the coder to harvest transaction numbers
// from Cron Items.
// This will be used on the client side, I believe, and not server side (not in
// smart contracts.)
//

// Done: Make a copy of CancelCronItem(), and use it to make an EXECUTE CLAUSE
// message
// for Nyms to trigger existing smart contracts!!!!!!!!!
// WAIT: Except it DOESN'T have to be a transaction! Because as long as the Nym
// is a valid
// party, and the action occurs, it will ALREADY drop receipts into the
// appropriate boxes!
// But wait... How do we know that the Nym REALLY triggered that clause, if
// there's not
// a transaction # burned? It doesn't matter: the purpose of transaction #s is
// to make sure
// that CHANGES in BALANCE are signed off on, by leaving the # open, and putting
// a receipt
// in the inbox. But if Nym really is a party to this smart contract (which will
// be verified
// in any case), then he DOES have transaction #s open already, and if any
// balances change,
// receipts will be dropped into the appropriate inboxes already containing
// those transaction
// #s. (As well as containing his original signed copy of the cron item, AND as
// well as
// containing the latest message, with his signature and request number on it,
// which triggered
// the clause to be called.)  Furthermore, if the clause is triggered multiple
// times, then
// there must be multiple receipts, and each one should feature a NEW instance
// of the triggering,
// WITH a valid signature on it, and WITH a NEW REQUEST NUMBER ON IT. Without
// this, none of
// the balances could change anyway.
// Therefore I conclude that the "trigger a clause" message does NOT have to be
// a transaction,
// but only needs to be a message  :-)
//
// Done: the "trigger clause" message!

// DONE: Change this into a TRANSACTION!
// (So there can be a transaction statement, since canceling a cron
// item removes a transaction number from your issued list.)
///-------------------------------------------------------
/// CANCEL A SPECIFIC OFFER (THAT SAME NYM PLACED PREVIOUSLY ON SAME SERVER.)
/// By transaction number as key.
///
int32_t OT_API::cancelCronItem(const OTIdentifier& SERVER_ID,
                               const OTIdentifier& USER_ID,
                               const OTIdentifier& ASSET_ACCT_ID,
                               const int64_t& lTransactionNum) const // so the
                                                                     // server
// can lookup the
// offer in Cron.
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    const OTString strServerID(SERVER_ID), strNymID(USER_ID);

    if (pNym->GetTransactionNumCount(strServerID) < 1) {
        otOut << "OT_API::cancelCronItem: At least 1 Transaction Number is "
                 "necessary to cancel any cron item. "
                 "Try requesting the server for more numbers (you are low.)\n";
        return (-1);
    }
    int64_t lStoredTransactionNumber = 0;
    bool bGotTransNum = pNym->GetNextTransactionNum(
        *pNym, strServerID, lStoredTransactionNumber, true); // bSave=false

    if (!bGotTransNum)
        otErr << "OT_API::cancelCronItem: Supposedly there was a "
                 "transaction number available, but the call\n"
                 "still failed.\n";
    else {
        int64_t lRequestNumber = 0;

        OTString str_ASSET_ACCT_ID(ASSET_ACCT_ID);

        // Create a transaction
        OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
            USER_ID, ASSET_ACCT_ID, SERVER_ID, OTTransaction::cancelCronItem,
            lStoredTransactionNumber);
        // set up the transaction item (each transaction may have multiple
        // items...)
        OTItem* pItem = OTItem::CreateItemFromTransaction(
            *pTransaction, OTItem::cancelCronItem);
        OT_ASSERT_MSG(nullptr != pItem,
                      "Error allocating memory in the OT API");

        pItem->SetReferenceToNum(lTransactionNum); // This transaction is being
                                                   // sent in order to cancel
                                                   // another transaction.
        pTransaction->SetReferenceToNum(lTransactionNum); // This is where we
                                                          // clearly show which
                                                          // one is actually
                                                          // being cancelled.

        // sign the item
        pItem->SignContract(*pNym);
        pItem->SaveContract();

        // the Transaction "owns" the item now and will handle cleaning it up.
        pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                       // cleanup the item. It "owns" it now.
        // TRANSACTION AGREEMENT

        // pBalanceItem is signed and saved within this call. No need to do that
        // again.
        OTItem* pStatementItem =
            pNym->GenerateTransactionStatement(*pTransaction);

        if (nullptr !=
            pStatementItem) // will never be nullptr. Will assert above
                            // before it gets here.
            pTransaction->AddItem(*pStatementItem); // Better not be nullptr...
                                                    // message will fail... But
                                                    // better check anyway.
        // sign the transaction
        pTransaction->SignContract(*pNym);
        pTransaction->SaveContract();

        // set up the ledger
        OTLedger theLedger(USER_ID, ASSET_ACCT_ID, SERVER_ID);
        theLedger.GenerateLedger(ASSET_ACCT_ID, SERVER_ID,
                                 OTLedger::message); // bGenerateLedger defaults
                                                     // to false, which is
                                                     // correct.
        theLedger.AddTransaction(*pTransaction); // now the ledger "owns" and
                                                 // will handle cleaning up the
                                                 // transaction.

        // sign the ledger
        theLedger.SignContract(*pNym);
        theLedger.SaveContract();

        // extract the ledger in ascii-armored form... encoding...
        OTString strLedger(theLedger);
        OTASCIIArmor ascLedger(strLedger);

        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%ld", lRequestNumber); // Always have to send this.
        pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                       // server request, I have
                                                       // to increment it

        // (1) Set up member variables
        theMessage.m_strCommand = "notarizeTransactions";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strServerID = strServerID;
        theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                              // theMessage.m_strServerID is
                                              // already set. (It uses it.)

        theMessage.m_strAcctID = str_ASSET_ACCT_ID;
        theMessage.m_ascPayload = ascLedger;

        OTIdentifier NYMBOX_HASH;
        const std::string str_server(strServerID.Get());
        const bool bNymboxHash = pNym->GetNymboxHash(str_server, NYMBOX_HASH);
        NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

        if (!bNymboxHash)
            otErr << "Failed getting NymboxHash from Nym for server: "
                  << str_server << "\n";

        // (2) Sign the Message
        theMessage.SignContract(*pNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        // (Send it)
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return m_pClient->CalcReturnVal(lRequestNumber);
    } // got transaction number.

    return (-1);
}

// ISSUE MARKET OFFER
//
//
int32_t OT_API::issueMarketOffer(
    const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
    const OTIdentifier& ASSET_ACCT_ID, const OTIdentifier& CURRENCY_ACCT_ID,
    const int64_t& MARKET_SCALE,      // Defaults to minimum of 1. Market
                                      // granularity.
    const int64_t& MINIMUM_INCREMENT, // This will be multiplied by the Scale.
                                      // Min 1.
    const int64_t& TOTAL_ASSETS_ON_OFFER, // Total assets available for sale or
                                          // purchase. Will be multiplied by
                                          // minimum increment.
    const int64_t& PRICE_LIMIT,           // Per Minimum Increment...
    bool bBuyingOrSelling,                //  BUYING == false, SELLING == true.
    time64_t tLifespanInSeconds,          // 86400 == 1 day.
    char STOP_SIGN,                       // For stop orders, set to '<' or '>'
    int64_t ACTIVATION_PRICE) const       // For stop orders, this is
                                          // threshhold price.
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet. (No need to
    // cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAssetAccount =
        GetOrLoadAccount(*pNym, ASSET_ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAssetAccount) return (-1);
    // By this point, pAssetAccount is a good pointer.  (No need to cleanup.)
    OTAccount* pCurrencyAccount =
        GetOrLoadAccount(*pNym, CURRENCY_ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pCurrencyAccount) return (-1);
    // By this point, pCurrencyAccount is a good pointer.  (No need to cleanup.)
    const OTIdentifier& ASSET_TYPE_ID = pAssetAccount->GetAssetTypeID();
    const OTIdentifier& CURRENCY_TYPE_ID = pCurrencyAccount->GetAssetTypeID();
    if (ASSET_TYPE_ID == CURRENCY_TYPE_ID) {
        otOut << __FUNCTION__
              << ": The asset account and currency account cannot "
                 "have the same asset type ID. (You can't, for "
                 "example, trade dollars against other dollars. Why "
                 "bother trading them in the first place?)\n";
        return (-1);
    }
    OTMessage theMessage;
    const OTString strServerID(SERVER_ID), strNymID(USER_ID);
    if (pNym->GetTransactionNumCount(strServerID) < 3) {
        otOut << __FUNCTION__
              << ": At least 3 Transaction Numbers are necessary to "
                 "issue a market offer. "
                 "Try requesting the server for more (you are low.)\n";
        return (-1);
    }
    int64_t lStoredTransactionNumber = 0, lAssetAcctClosingNo = 0,
            lCurrencyAcctClosingNo = 0;
    bool bGotTransNum = pNym->GetNextTransactionNum(
        *pNym, strServerID, lStoredTransactionNumber, false); // bSave=false
    bool bGotAssetClosingNum = pNym->GetNextTransactionNum(
        *pNym, strServerID, lAssetAcctClosingNo,
        false); // bSave=false -- (true by default, FYI.)
    bool bGotCurrencyClosingNum = pNym->GetNextTransactionNum(
        *pNym, strServerID, lCurrencyAcctClosingNo, true); // bSave=true
    if (!bGotTransNum || !bGotAssetClosingNum || !bGotCurrencyClosingNum) {
        otErr << __FUNCTION__
              << ": Supposedly there were 3 transaction numbers "
                 "available, but the call(s)\n"
                 "still failed. (Re-adding back to Nym, and failing out "
                 "of this function.)\n";
        if (bGotTransNum)
            pNym->AddTransactionNum(*pNym, strServerID,
                                    lStoredTransactionNumber,
                                    false); // bSave=true
        if (bGotAssetClosingNum)
            pNym->AddTransactionNum(*pNym, strServerID, lAssetAcctClosingNo,
                                    false); // bSave=true
        if (bGotCurrencyClosingNum)
            pNym->AddTransactionNum(*pNym, strServerID, lCurrencyAcctClosingNo,
                                    false); // bSave=true
        if (bGotTransNum || bGotAssetClosingNum || bGotCurrencyClosingNum)
            pNym->SaveSignedNymfile(*pNym);
    }
    else {
        const time64_t VALID_FROM = GetTime(); // defaults to RIGHT NOW
                                               // aka OT_API_GetTime() if
                                               // set to 0 anyway.
        const time64_t VALID_TO = OTTimeAddTimeInterval(
            VALID_FROM, // defaults to 24 hours (a "Day Order") aka
                        // OT_API_GetTime() + 86,400
            OTTimeGetSecondsFromTime(OT_TIME_ZERO == tLifespanInSeconds
                                         ? OT_TIME_DAY_IN_SECONDS
                                         : tLifespanInSeconds));
        int64_t lTotalAssetsOnOffer = 1, lMinimumIncrement = 1,
                lPriceLimit = 0, // your price limit, per scale of assets.
            lMarketScale = 1, lActivationPrice = 0;
        if (TOTAL_ASSETS_ON_OFFER > 0)
            lTotalAssetsOnOffer =
                TOTAL_ASSETS_ON_OFFER; // otherwise, defaults to 1.
        if (MARKET_SCALE > 0)
            lMarketScale = MARKET_SCALE; // otherwise, defaults to 1.
        if (MINIMUM_INCREMENT > 0)
            lMinimumIncrement = MINIMUM_INCREMENT; // otherwise, defaults to 1.
        if (PRICE_LIMIT > 0)
            lPriceLimit = PRICE_LIMIT; // otherwise, defaults to 0. (0 Being a
                                       // market order.)
        char cStopSign = 0;

        if ((ACTIVATION_PRICE > 0) &&
            (('<' == STOP_SIGN) || ('>' == STOP_SIGN))) {
            cStopSign = STOP_SIGN;
            lActivationPrice = ACTIVATION_PRICE;
        }
        lMinimumIncrement *= lMarketScale; // minimum increment is PER SCALE.
        //        lTotalAssetsOnOffer *= lMinimumIncrement;  // This was a bug.
        // (Left as a warning.)

        OTString strOfferType("market order");

        if (lPriceLimit > 0) strOfferType = "limit order";

        if (0 != cStopSign) {
            if (lPriceLimit > 0)
                strOfferType.Format("stop limit order, at threshhold: %c%ld",
                                    cStopSign, lActivationPrice);
            else
                strOfferType.Format("stop order, at threshhold: %c%ld",
                                    cStopSign, lActivationPrice);
        }

        OTString strPrice("");

        if (lPriceLimit > 0) strPrice.Format("Price: %ld\n", lPriceLimit);

        otOut << "Placing market offer " << lStoredTransactionNumber
              << ", type: " << (bBuyingOrSelling ? "selling" : "buying") << ", "
              << strOfferType << "\n" << strPrice
              << "Assets for sale/purchase: " << lTotalAssetsOnOffer
              << "\n"
                 "In minimum increments of: " << lMinimumIncrement
              << "\n"
                 "At market of scale: " << lMarketScale
              << "\n"
                 "Valid From: " << OTTimeGetSecondsFromTime(VALID_FROM)
              << "  To: " << OTTimeGetSecondsFromTime(VALID_TO) << "\n";

        OTOffer theOffer(SERVER_ID, ASSET_TYPE_ID, CURRENCY_TYPE_ID,
                         lMarketScale);
        OTTrade theTrade(SERVER_ID, ASSET_TYPE_ID, ASSET_ACCT_ID, USER_ID,
                         CURRENCY_TYPE_ID, CURRENCY_ACCT_ID);
        // MAKE OFFER...
        //
        bool bCreateOffer = theOffer.MakeOffer(
            bBuyingOrSelling,    // True == SELLING, False == BUYING
            lPriceLimit,         // Per Minimum Increment...
            lTotalAssetsOnOffer, // Total assets available for sale or purchase.
            lMinimumIncrement,   // The minimum increment that must be bought or
                                 // sold for each transaction
            lStoredTransactionNumber, // Transaction number matches on
                                      // transaction, item, offer, and trade.
            VALID_FROM, // defaults to RIGHT NOW aka OT_API_GetTime()
            VALID_TO);  // defaults to 24 hours (a "Day Order") aka
                        // OT_API_GetTime() + 86,400
        // ISSUE TRADE.
        bool bIssueTrade = false;

        if (bCreateOffer) {
            bCreateOffer = theOffer.SignContract(*pNym);
            if (bCreateOffer) {
                bCreateOffer = theOffer.SaveContract();
                if (bCreateOffer) {
                    bIssueTrade = theTrade.IssueTrade(
                        theOffer, cStopSign,
                        lActivationPrice); // <==== ISSUE TRADE <=====
                    // This is new: the closing transaction number is now used
                    // for CLOSING recurring
                    // cron items, like market offers and payment plans. It's
                    // also useful for baskets.
                    // Since this is a market offer, it needs a closing number
                    // (for later cancellation
                    // or expiration.)
                    // lAssetAcctClosingNo=0, lCurrencyAcctClosingNo=0;
                    //
                    theTrade.AddClosingTransactionNo(lAssetAcctClosingNo);
                    theTrade.AddClosingTransactionNo(lCurrencyAcctClosingNo);

                    if (bIssueTrade) {
                        bIssueTrade = theTrade.SignContract(*pNym);
                        if (bIssueTrade) bIssueTrade = theTrade.SaveContract();
                    } // if ( bIssueTrade )
                }
            }
        } // if ( bCreateOffer )
        if (bCreateOffer && bIssueTrade) {
            int64_t lRequestNumber = 0;
            OTString str_ASSET_ACCT_ID(ASSET_ACCT_ID);

            // Create a transaction
            OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
                USER_ID, ASSET_ACCT_ID, SERVER_ID, OTTransaction::marketOffer,
                lStoredTransactionNumber);

            // set up the transaction item (each transaction may have multiple
            // items...)
            OTItem* pItem = OTItem::CreateItemFromTransaction(
                *pTransaction, OTItem::marketOffer,
                const_cast<OTIdentifier*>((&CURRENCY_ACCT_ID)));
            // the "To" account (normally used for a TRANSFER transaction) is
            // used here
            // storing the Currency Acct ID. The Server will expect the Trade
            // object bundled
            // within this item to have an Asset Acct ID and "Currency" Acct ID
            // that match
            // those on this Item. Otherwise it will reject the offer.
            //
            OT_ASSERT_MSG(nullptr != pItem, "OT_API::issueMarketOffer: Error "
                                            "allocating memory in the OT API");

            OTString strTrade;
            theTrade.SaveContractRaw(strTrade);

            // Add the trade string as the attachment on the transaction item.
            pItem->SetAttachment(strTrade); // The trade is contained in the
                                            // attachment string. (The offer is
                                            // within the trade.)

            // sign the item
            pItem->SignContract(*pNym);
            pItem->SaveContract();

            // the Transaction "owns" the item now and will handle cleaning it
            // up.
            pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                           // cleanup the item. It "owns" it
                                           // now.
            // TRANSACTION AGREEMENT

            // pBalanceItem is signed and saved within this call. No need to do
            // that again.
            OTItem* pStatementItem =
                pNym->GenerateTransactionStatement(*pTransaction);

            if (nullptr !=
                pStatementItem) // will never be nullptr. Will assert above
                                // before it gets here.
                pTransaction->AddItem(
                    *pStatementItem); // Better not be nullptr...
                                      // message will fail...
                                      // But better check
                                      // anyway.
            // sign the transaction
            pTransaction->SignContract(*pNym);
            pTransaction->SaveContract();

            // set up the ledger
            OTLedger theLedger(USER_ID, ASSET_ACCT_ID, SERVER_ID);
            theLedger.GenerateLedger(ASSET_ACCT_ID, SERVER_ID,
                                     OTLedger::message); // bGenerateLedger
                                                         // defaults to false,
                                                         // which is correct.
            theLedger.AddTransaction(*pTransaction); // now the ledger "owns"
                                                     // and will handle cleaning
                                                     // up the transaction.

            // sign the ledger
            theLedger.SignContract(*pNym);
            theLedger.SaveContract();

            // extract the ledger in ascii-armored form... encoding...
            OTString strLedger(theLedger);
            OTASCIIArmor ascLedger(strLedger);

            // (0) Set up the REQUEST NUMBER and then INCREMENT IT
            pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
            theMessage.m_strRequestNum.Format(
                "%ld", lRequestNumber); // Always have to send this.
            pNym->IncrementRequestNum(*pNym, strServerID); // since I used it
                                                           // for a server
                                                           // request, I have to
                                                           // increment it

            // (1) Set up member variables
            theMessage.m_strCommand = "notarizeTransactions";
            theMessage.m_strNymID = strNymID;
            theMessage.m_strServerID = strServerID;
            theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                                  // theMessage.m_strServerID is
                                                  // already set. (It uses it.)

            theMessage.m_strAcctID = str_ASSET_ACCT_ID;
            theMessage.m_ascPayload = ascLedger;

            OTIdentifier NYMBOX_HASH;
            const std::string str_server(strServerID.Get());
            const bool bNymboxHash =
                pNym->GetNymboxHash(str_server, NYMBOX_HASH);
            NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

            if (!bNymboxHash)
                otErr << __FUNCTION__ << ": Failed getting NymboxHash from Nym "
                                         "for server: " << str_server << "\n";

            // (2) Sign the Message
            theMessage.SignContract(*pNym);

            // (3) Save the Message (with signatures and all, back to its
            // internal member m_strRawFile.)
            theMessage.SaveContract();

            // (Send it)
            m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                              m_pTransportCallback);
            m_pClient->ProcessMessageOut(theMessage);

            return m_pClient->CalcReturnVal(lRequestNumber);

        } // if (bCreateOffer && bIssueTrade)
        else {
            otOut << __FUNCTION__
                  << ": Unable to create offer or issue trade. Sorry.\n";

            // IF FAILED, add the transaction number (and closing number)
            // BACK to the list of available numbers.
            //
            pNym->AddTransactionNum(*pNym, strServerID,
                                    lStoredTransactionNumber,
                                    false); // bSave defaults to true
            pNym->AddTransactionNum(*pNym, strServerID, lAssetAcctClosingNo,
                                    false);
            pNym->AddTransactionNum(
                *pNym, strServerID, lCurrencyAcctClosingNo,
                true); // bSave=true (No sense saving thrice in a row.)
        }
    } // got transaction number.

    return (-1);
}

/// GET MARKET LIST
///
/// Connect to a specific server, as a specific Nym, and request the list of
/// markets.
/// (Flush the buffer before calling this. Then after you make this call, wait
/// 50 ms
/// and then pop the buffer and check the server reply for success. From there
/// you can
/// either read the reply data directly out of the reply message, or you can
/// load it from
/// storage (OT will probably auto-store the reply to storage, for your
/// convenience.)
///
int32_t OT_API::getMarketList(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    OTString strServerID(SERVER_ID);
    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    int64_t lRequestNumber = 0;
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    OTString strNymID(USER_ID);

    // (1) Set up member variables
    theMessage.m_strCommand = "getMarketList";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

/// GET ALL THE OFFERS ON A SPECIFIC MARKET
///
/// A specific Nym is requesting the Server to send a list of the offers on a
/// specific
/// Market ID-- the bid/ask, and prices/amounts, basically--(up to lDepth or
/// server Max)
///
int32_t OT_API::getMarketOffers(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTIdentifier& MARKET_ID,
                                const int64_t& lDepth) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    OTString strServerID(SERVER_ID), strMarketID(MARKET_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    int64_t lRequestNumber = 0;
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    OTString strNymID(USER_ID);
    // (1) Set up member variables
    theMessage.m_strCommand = "getMarketOffers";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strNymID2 = strMarketID;
    theMessage.m_lDepth = lDepth;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

///-------------------------------------------------------
/// GET RECENT TRADES FOR A SPECIFIC MARKET ID
///
/// Most likely, ticker data will be made available through a separate ZMQ
/// instance,
/// which will use the publisher/subscriber model to distribute ticker data.
/// From there,
/// those privileged subscribers can distribute it via RSS, store it for future
/// analysis,
/// display charts, etc.
///
/// (So this function is not here to usurp that purpose.)
///
int32_t OT_API::getMarketRecentTrades(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& USER_ID,
                                      const OTIdentifier& MARKET_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    OTString strServerID(SERVER_ID), strMarketID(MARKET_ID);
    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    int64_t lRequestNumber = 0;
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    OTString strNymID(USER_ID);
    // (1) Set up member variables
    theMessage.m_strCommand = "getMarketRecentTrades";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strNymID2 = strMarketID;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

///-------------------------------------------------------
/// GET ALL THE ACTIVE (in Cron) MARKET OFFERS FOR A SPECIFIC NYM.
/// (ON A SPECIFIC SERVER, OBVIOUSLY.) Remember to use Flush/Call/Wait/Pop
/// to check the server reply for success or fail.
/// Hmm for size reasons, this really will have to return a list of transaction
/// #s,
/// and then I request them one-by-one after that...
///
int32_t OT_API::getNym_MarketOffers(const OTIdentifier& SERVER_ID,
                                    const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    OTString strServerID(SERVER_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    int64_t lRequestNumber = 0;
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    OTString strNymID(USER_ID);

    // (1) Set up member variables
    theMessage.m_strCommand = "getNym_MarketOffers";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

// ===============================================================

int32_t OT_API::notarizeTransfer(const OTIdentifier& SERVER_ID,
                                 const OTIdentifier& USER_ID,
                                 const OTIdentifier& ACCT_FROM,
                                 const OTIdentifier& ACCT_TO,
                                 const int64_t& AMOUNT,
                                 const OTString& NOTE) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_FROM, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    const int64_t lAmount = AMOUNT;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strFromAcct(ACCT_FROM),
        strToAcct(ACCT_TO);

    int64_t lStoredTransactionNumber = 0;
    bool bGotTransNum = pNym->GetNextTransactionNum(*pNym, strServerID,
                                                    lStoredTransactionNumber);

    if (bGotTransNum) {
        // Create a transaction
        OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
            USER_ID, ACCT_FROM, SERVER_ID, OTTransaction::transfer,
            lStoredTransactionNumber);

        // set up the transaction item (each transaction may have multiple
        // items...)
        OTItem* pItem = OTItem::CreateItemFromTransaction(
            *pTransaction, OTItem::transfer, &ACCT_TO);
        pItem->SetAmount(lAmount);

        // The user can include a note here for the recipient.
        if (NOTE.Exists() && NOTE.GetLength() > 2) {
            pItem->SetNote(NOTE);
        }

        // sign the item
        pItem->SignContract(*pNym);
        pItem->SaveContract();

        pTransaction->AddItem(*pItem); // the Transaction's destructor will
                                       // cleanup the item. It "owns" it now.
        std::unique_ptr<OTLedger> pInbox(pAccount->LoadInbox(*pNym));
        std::unique_ptr<OTLedger> pOutbox(pAccount->LoadOutbox(*pNym));

        if (nullptr == pInbox) {
            otOut << "OT_API::notarizeTransfer: Failed loading inbox for acct: "
                  << strFromAcct << "\n";
            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            pNym->AddTransactionNum(*pNym, strServerID,
                                    lStoredTransactionNumber,
                                    true); // bSave=true
        }
        else if (nullptr == pOutbox) {
            otOut << "OT_API::notarizeTransfer: Failed loading outbox "
                     "for acct: " << strFromAcct << "\n";
            // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE
            // NUMBERS.
            pNym->AddTransactionNum(*pNym, strServerID,
                                    lStoredTransactionNumber,
                                    true); // bSave=true
        }
        else {
            int64_t lRequestNumber = 0;
            // Need to setup a dummy outbox transaction (to mimic the one that
            // will be on the server side when this pending transaction is
            // actually put into the real outbox.)
            // When the server adds its own, and then compares the two, they
            // should both show the same pending transaction, in order for this
            // balance agreement to be valid..
            // Otherwise the server would have to refuse it for being inaccurate
            // (server can't sign something inaccurate!) So I throw a dummy on
            // there before generating balance statement.
            //
            OTTransaction* pOutboxTransaction = OTTransaction::GenerateTransaction(
                *pOutbox, OTTransaction::pending,
                1 /*todo pick some number that everyone agrees doesn't matter, like 1. The referring-to is the important
                                                                                       number in this case, and perhaps server should update this value too before signing and returning.*/); // todo use a constant instead of '1'
            OT_ASSERT(nullptr != pOutboxTransaction); // for now.

            OTString strItem(*pItem);
            pOutboxTransaction->SetReferenceString(
                strItem); // So the GenerateBalanceStatement function below can
                          // get the other info off this item (like amount, etc)
            pOutboxTransaction->SetReferenceToNum(pItem->GetTransactionNum());

            //            pOutboxTransaction->SignContract(*pNym);    //
            // Unnecessary to sign/save, since this is just a dummy data for
            // verification purposes, and isn't being
            //            pOutboxTransaction->SaveContract();            //
            // serialized anywhere. (I download the actual outbox from server,
            // and verify against last signed receipt.)

            pOutbox->AddTransaction(*pOutboxTransaction); // no need to cleanup
                                                          // pOutboxTransaction
                                                          // since pOutbox will
                                                          // handle it now.
            // BALANCE AGREEMENT

            // pBalanceItem is signed and saved within this call. No need to do
            // that twice.
            //
            OTItem* pBalanceItem = pInbox->GenerateBalanceStatement(
                lAmount * (-1), *pTransaction, *pNym, *pAccount, *pOutbox);

            if (nullptr !=
                pBalanceItem) // will never be nullptr. Will assert above
                              // before it gets here.
                pTransaction->AddItem(
                    *pBalanceItem); // Better not be nullptr...
                                    // message will fail...
                                    // But better check
                                    // anyway.
            // sign the transaction
            pTransaction->SignContract(*pNym);
            pTransaction->SaveContract();

            // set up the ledger
            OTLedger theLedger(USER_ID, ACCT_FROM, SERVER_ID);
            theLedger.GenerateLedger(ACCT_FROM, SERVER_ID,
                                     OTLedger::message); // bGenerateLedger
                                                         // defaults to false,
                                                         // which is correct.
            theLedger.AddTransaction(*pTransaction);

            // sign the ledger
            theLedger.SignContract(*pNym);
            theLedger.SaveContract();

            // extract the ledger in ascii-armored form
            OTString strLedger(theLedger);
            OTASCIIArmor ascLedger;

            // Encoding...
            ascLedger.SetString(strLedger);
            // (0) Set up the REQUEST NUMBER and then INCREMENT IT
            pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
            theMessage.m_strRequestNum.Format(
                "%ld", lRequestNumber); // Always have to send this.
            pNym->IncrementRequestNum(*pNym, strServerID); // since I used it
                                                           // for a server
                                                           // request, I have to
                                                           // increment it

            // (1) Set up member variables
            theMessage.m_strCommand = "notarizeTransactions";
            theMessage.m_strNymID = strNymID;
            theMessage.m_strServerID = strServerID;
            theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                                  // theMessage.m_strServerID is
                                                  // already set. (It uses it.)

            theMessage.m_strAcctID = strFromAcct;
            theMessage.m_ascPayload = ascLedger;

            OTIdentifier NYMBOX_HASH;
            const std::string str_server(strServerID.Get());
            const bool bNymboxHash =
                pNym->GetNymboxHash(str_server, NYMBOX_HASH);
            NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

            if (!bNymboxHash)
                otErr << "Failed getting NymboxHash from Nym for server: "
                      << str_server << "\n";

            // (2) Sign the Message
            theMessage.SignContract(*pNym);

            // (3) Save the Message (with signatures and all, back to its
            // internal member m_strRawFile.)
            theMessage.SaveContract();

            // (Send it)
            m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                              m_pTransportCallback);
            m_pClient->ProcessMessageOut(theMessage);

            return m_pClient->CalcReturnVal(lRequestNumber);
        }
    }
    else
        otOut << "No transaction numbers were available. Suggest "
                 "requesting the server for one.\n";

    // IF FAILED, ADD TRANSACTION NUMBER BACK TO LIST OF AVAILABLE NUMBERS.
    //        pNym->AddTransactionNum(*pNym, strServerID,
    // lStoredTransactionNumber, true); // bSave=true
    // Duh! No need to re-add a transaction num when the error is that there
    // weren't any transaction numbers...

    return -1;
}

int32_t OT_API::getNymbox(const OTIdentifier& SERVER_ID,
                          const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "getNymbox";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

// NOTE: Deprecated. Replaced by getAccountFiles.
int32_t OT_API::getInbox(const OTIdentifier& SERVER_ID,
                         const OTIdentifier& USER_ID,
                         const OTIdentifier& ACCT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strAcctID(ACCT_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "getInbox";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAcctID = strAcctID;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

// NOTE: Deprecated. Replaced by getAccountFiles.
int32_t OT_API::getOutbox(const OTIdentifier& SERVER_ID,
                          const OTIdentifier& USER_ID,
                          const OTIdentifier& ACCT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strAcctID(ACCT_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "getOutbox";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAcctID = strAcctID;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

// Returns:
// -1 if error.
//  0 if Nymbox is empty.
//  1 or more: Count of items in Nymbox before processing.
//  UPDATE: This now returns the request number of the message sent, if success.
//
int32_t OT_API::processNymbox(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    const OTString strNymID(USER_ID);
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    bool bSuccess = false;
    int32_t nReceiptCount = (-1);
    int32_t nRequestNum = (-1);
    bool bIsEmpty = true;

    {
        OTPseudonym& theNym = *pNym;
        OTServerContract& theServer = *pServer;

        // Load up the appropriate Nymbox...
        OTLedger theNymbox(USER_ID, USER_ID, SERVER_ID);

        bool bLoadedNymbox = theNymbox.LoadNymbox();
        bool bVerifiedNymbox =
            bLoadedNymbox ? theNymbox.VerifyAccount(theNym) : false;

        if (!bLoadedNymbox)
            otOut << "OT_API::processNymbox: Failed loading Nymbox: "
                  << strNymID << " \n";
        else if (!bVerifiedNymbox)
            otOut << "OT_API::processNymbox: Failed verifying Nymbox: "
                  << strNymID << " \n";
        else {
            nReceiptCount = theNymbox.GetTransactionCount();
            bIsEmpty = (nReceiptCount < 1);

            if (!bIsEmpty)
                bSuccess = m_pClient->AcceptEntireNymbox(
                    theNymbox, SERVER_ID, theServer, theNym, theMessage);

            if (!bSuccess) {
                if (bIsEmpty) {
                    otWarn << "OT_API::processNymbox: Nymbox (" << strNymID
                           << ") is "
                              "empty (so, skipping processNymbox.)\n";
                    nRequestNum = 0;
                    nReceiptCount = 0; // redundant.
                }
                else {
                    otOut << "OT_API::processNymbox: Failed trying to "
                             "accept the entire Nymbox. (And no, it's "
                             "not empty.)\n";
                    nReceiptCount = (-1);
                }
            }
            else // Success!
            {
                OTIdentifier NYMBOX_HASH;
                const OTString strServerID(SERVER_ID);
                const std::string str_server(strServerID.Get());
                const bool bNymboxHash =
                    theNym.GetNymboxHash(str_server, NYMBOX_HASH);
                NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

                if (!bNymboxHash)
                    otErr << "Failed getting NymboxHash from Nym for server: "
                          << str_server << "\n";

                // (2) Sign the Message
                theMessage.SignContract(theNym);

                // (3) Save the Message (with signatures and all, back to its
                // internal member m_strRawFile.)
                theMessage.SaveContract();
            }
        }
    }

    if (bSuccess) {
        // Instead of the receipt count, in the case of success sending, we
        // return the
        // actual request number for the message that was sent.
        // (Otherwise 0 means Nymbox was empty, and -1 means there was an
        // error.)
        //
        nRequestNum = atoi(theMessage.m_strRequestNum.Get());

        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return nRequestNum;
    }
    // if successful, ..., else if not successful--and wasn't empty--then error.
    else if (!bIsEmpty)
        otErr << "Error performing processNymbox command in "
                 "OT_API::processNymbox\n";

    return nReceiptCount;
}

int32_t OT_API::processInbox(const OTIdentifier& SERVER_ID,
                             const OTIdentifier& USER_ID,
                             const OTIdentifier& ACCT_ID,
                             const OTString& ACCT_LEDGER) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strAcctID(ACCT_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "processInbox";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAcctID = strAcctID;

    // Presumably ACCT_LEDGER was already set up before this function was
    // called...
    // See test client for example of it being done.
    theMessage.m_ascPayload.SetString(ACCT_LEDGER);

    OTIdentifier NYMBOX_HASH;
    const std::string str_server(strServerID.Get());
    const bool bNymboxHash = pNym->GetNymboxHash(str_server, NYMBOX_HASH);
    NYMBOX_HASH.GetString(theMessage.m_strNymboxHash);

    if (!bNymboxHash)
        otErr << "Failed getting NymboxHash from Nym for server: " << str_server
              << "\n";

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

int32_t OT_API::issueAssetType(const OTIdentifier& SERVER_ID,
                               const OTIdentifier& USER_ID,
                               const OTString& THE_CONTRACT) const
{
    OTWallet* pWallet = GetWallet(__FUNCTION__);
    if (nullptr == pWallet) return (-1);
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    // otErr << "OT_API::issueAssetType: About to trim this contract: **BEGIN:"
    //  << THE_CONTRACT << "***END\n\n";

    std::string str_Trim(THE_CONTRACT.Get());
    std::string str_Trim2 = OTString::trim(str_Trim);
    OTString strTrimContract(str_Trim2.c_str());
    OTAssetContract theAssetContract;

    if (!theAssetContract.LoadContractFromString(strTrimContract)) {
        otOut << __FUNCTION__
              << ": Failed trying to load asset contract from string:\n\n"
              << strTrimContract << "\n\n";
    }
    else {
        OTIdentifier newID;
        theAssetContract.CalculateContractID(newID);
        theAssetContract.SetIdentifier(newID); // probably unnecessary
        OTMessage theMessage;
        int64_t lRequestNumber = 0;

        OTString strServerID(SERVER_ID), strNymID(USER_ID);

        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%ld", lRequestNumber); // Always have to send this.
        pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                       // server request, I have
                                                       // to increment it

        // (1) set up member variables
        theMessage.m_strCommand = "issueAssetType";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strServerID = strServerID;
        theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                              // theMessage.m_strServerID is
                                              // already set. (It uses it.)

        newID.GetString(theMessage.m_strAssetID);
        OTString strAssetContract(theAssetContract);
        theMessage.m_ascPayload.SetString(strAssetContract);

        // (2) Sign the Message
        theMessage.SignContract(*pNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();
        // Save the contract to local storage and add to wallet.
        //
        OTString strFilename; // In this case the filename isn't actually used,
                              // since SaveToContractFolder will
        // handle setting up the filename and overwrite it anyway. But I still
        // prefer to set it
        // up correctly, rather than pass a blank. I'm just funny like that.
        strFilename = theMessage.m_strAssetID.Get();

        OTString strFoldername(OTFolders::Contract().Get());

        OTAssetContract* pContract =
            new OTAssetContract(theMessage.m_strAssetID, strFoldername,
                                strFilename, theMessage.m_strAssetID);
        OT_ASSERT(nullptr != pContract);

        // Check the server signature on the contract here. (Perhaps the message
        // is good enough?
        // After all, the message IS signed by the server and contains the
        // Account.
        //        if (pContract->LoadContract() && pContract->VerifyContract())
        if (!pContract->LoadContractFromString(strTrimContract)) {
            otOut << __FUNCTION__
                  << ": Failed(2) trying to load asset contract "
                     "from string:\n\n" << strTrimContract << "\n\n";
        }
        else if (!pContract->VerifyContract()) {
            otOut << __FUNCTION__ << ": Failed verifying asset contract:\n\n"
                  << strTrimContract << "\n\n";
        }
        else {
            // Next make sure the wallet has this contract on its list...
            pWallet->AddAssetContract(
                *pContract); // this saves both the contract and the wallet.
            pContract =
                nullptr; // Success. The wallet "owns" it now, no need to
                         // clean it up.
        }
        // cleanup
        if (pContract) {
            delete pContract;
            pContract = nullptr;
        }

        // (Send it)
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return m_pClient->CalcReturnVal(lRequestNumber);
    }

    return -1;
}

int32_t OT_API::getContract(const OTIdentifier& SERVER_ID,
                            const OTIdentifier& USER_ID,
                            const OTIdentifier& ASSET_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID),
        strAssetTypeID(ASSET_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "getContract";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAssetID = strAssetTypeID;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

int32_t OT_API::getMint(const OTIdentifier& SERVER_ID,
                        const OTIdentifier& USER_ID,
                        const OTIdentifier& ASSET_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAssetContract* pAssetContract =
        GetAssetType(ASSET_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pAssetContract) return (-1);
    // By this point, pAssetContract is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID),
        strAssetTypeID(ASSET_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "getMint";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAssetID = strAssetTypeID;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

// Sends a list of asset IDs to the server, which replies with a list of the
// actual receipts
// with the issuer's signature on them, from when the currency was issued.
//
// So you can tell for sure whether or not they are actually issued on that
// server.
//
// Map input: key is asset type ID, and value is blank (server reply puts
// issuer's receipts
// in that spot.)
//
int32_t OT_API::queryAssetTypes(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTASCIIArmor& ENCODED_MAP) const
{
    /*
     // Java code will create a StringMap object:

        String strEncodedObj(""); // output will go here.
        StringMap stringMap = null;    // we are about to create this object

        Storable storable =
     otapi.CreateObject(StoredObjectType.STORED_OBJ_STRING_MAP);

        if (storable != null)
        {
            stringMap = StringMap.ot_dynamic_cast(storable);
            if (stringMap != null)
            {
                // ADD ALL THE ASSET IDs HERE (To the string map, so you
                // can ask the server about them...)
                //
                for_each(the asset IDs you want to query the server about)
                {
                    stringMap.SetValue(ASSET_TYPE_ID, "exists");
                }

                strEncodedObj = otapi.EncodeObject(stringMap);
            }
        }

        if (null == strEncodedObj)
            Error;
     ----------------------------------------------------------------------

        Then send the server message:

     var theRequest := OTAPI_Func(ot_Msg.QUERY_ASSET_TYPES, SERVER_ID, NYM_ID,
     strEncodedObj);
     var    strResponse = theRequest.SendRequest(theRequest,
     "QUERY_ASSET_TYPES");

     String strReplyMap = null;

     // When the server reply comes back, get the payload from it:
     //
     if (strResponse != null)
        strReplyMap = OT_API_Message_GetPayload(strReply);
     ----------------------------------------------------------------------

        //    Pass the payload (the StringMap from the server's reply) to
     otapi.DecodeObject:
        //
        if (strReplyMap != null)
        {
            StringMap stringMap = null;
            Storable storable =
     otapi.DecodeObject(StoredObjectType.STORED_OBJ_STRING_MAP, strReplyMap);
            if (storable != null)
            {
                stringMap = StringMap.ot_dynamic_cast(storable);
                if (stringMap != null)
                {
                    // Loop through string map. For each asset ID key, the value
     will
                    // say either "true" or "false".
                    //
                    for_each(stringMap)
                    {
                        strValue = stringMap.GetValue(ASSET_TYPE_ID);

                        if (strValue.compare("true") == 0)
                        {
                            // ... do something here ...
                        }
                    }
                }
            }
        }

     */
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "queryAssetTypes";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_ascPayload = ENCODED_MAP;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

int32_t OT_API::createAssetAccount(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   const OTIdentifier& ASSET_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAssetContract* pAssetContract =
        GetAssetType(ASSET_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pAssetContract) return (-1);
    // By this point, pAssetContract is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID),
        strAssetTypeID(ASSET_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "createAccount";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAssetID = strAssetTypeID;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

int32_t OT_API::deleteAssetAccount(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   const OTIdentifier& ACCOUNT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCOUNT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strAcctID(ACCOUNT_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "deleteAssetAccount";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAcctID = strAcctID;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

bool OT_API::DoesBoxReceiptExist(
    const OTIdentifier& SERVER_ID,
    const OTIdentifier& USER_ID, // Unused here for now, but still convention.
    const OTIdentifier& ACCOUNT_ID, // If for Nymbox (vs inbox/outbox) then pass
                                    // USER_ID in this field also.
    int32_t nBoxType,               // 0/nymbox, 1/inbox, 2/outbox
    const int64_t& lTransactionNum) const
{
    // static
    return VerifyBoxReceiptExists(
        SERVER_ID, USER_ID, // Unused here for now, but still convention.
        ACCOUNT_ID, // If for Nymbox (vs inbox/outbox) then pass USER_ID in this
                    // field also.
        nBoxType,   // 0/nymbox, 1/inbox, 2/outbox
        lTransactionNum);
}

int32_t OT_API::getBoxReceipt(
    const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
    const OTIdentifier& ACCOUNT_ID, // If for Nymbox (vs inbox/outbox) then pass
                                    // USER_ID in this field also.
    int32_t nBoxType,               // 0/nymbox, 1/inbox, 2/outbox
    const int64_t& lTransactionNum) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(USER_ID, false, __FUNCTION__);
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    if (USER_ID != ACCOUNT_ID) // inbox/outbox (if it were nymbox, the USER_ID
                               // and ACCOUNT_ID would match)
    {
        OTAccount* pAccount =
            GetOrLoadAccount(*pNym, ACCOUNT_ID, SERVER_ID, __FUNCTION__);
        if (nullptr == pAccount) return (-1);
    }
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    // Note: pAccount could be nullptr also, but this function doesn't actually
    // use
    // it anyway.
    // I just force it to load in order to keep things clean, since this is an
    // API call. (I'm
    // real strict on the API user, making him keep his nose clean.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    const OTString strServerID(SERVER_ID), strNymID(USER_ID),
        strAcctID(ACCOUNT_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "getBoxReceipt";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAcctID = strAcctID;
    theMessage.m_lDepth = static_cast<int64_t>(nBoxType);
    theMessage.m_lTransactionNum = lTransactionNum;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

// NOTE: Deprecated. Replaced by getAccountFiles.
int32_t OT_API::getAccount(const OTIdentifier& SERVER_ID,
                           const OTIdentifier& USER_ID,
                           const OTIdentifier& ACCT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strAcctID(ACCT_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "getAccount";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAcctID = strAcctID;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

int32_t OT_API::getAccountFiles(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTIdentifier& ACCT_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTAccount* pAccount =
        GetOrLoadAccount(*pNym, ACCT_ID, SERVER_ID, __FUNCTION__);
    if (nullptr == pAccount) return (-1);
    // By this point, pAccount is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID), strAcctID(ACCT_ID);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "getAccountFiles";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_strAcctID = strAcctID;

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

int32_t OT_API::getRequest(const OTIdentifier& SERVER_ID,
                           const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    int32_t nReturnValue = m_pClient->ProcessUserCommand(
        OTClient::getRequest, theMessage, *pNym, *pServer,
        nullptr); // nullptr pAccount on this command.
    if (0 < nReturnValue) {
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return nReturnValue;
    }
    else
        otErr << "Error processing getRequest command in OT_API::getRequest\n";

    return (-1);
}

int32_t OT_API::usageCredits(const OTIdentifier& SERVER_ID,
                             const OTIdentifier& USER_ID,
                             const OTIdentifier& USER_ID_CHECK,
                             int64_t lAdjustment) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID),
        strNymID2(USER_ID_CHECK);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "usageCredits";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strNymID2 = strNymID2;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    theMessage.m_lDepth = lAdjustment; // Default is "no adjustment"
                                       // (@usageCredits returns current balance
                                       // regardless.)

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

int32_t OT_API::checkUser(const OTIdentifier& SERVER_ID,
                          const OTIdentifier& USER_ID,
                          const OTIdentifier& USER_ID_CHECK) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID),
        strNymID2(USER_ID_CHECK);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "checkUser";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strNymID2 = strNymID2;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    // (2) Sign the Message
    theMessage.SignContract(*pNym);

    // (3) Save the Message (with signatures and all, back to its internal
    // member m_strRawFile.)
    theMessage.SaveContract();

    // (Send it)
    m_pClient->SetFocusToServerAndNym(*pServer, *pNym, m_pTransportCallback);
    m_pClient->ProcessMessageOut(theMessage);

    return m_pClient->CalcReturnVal(lRequestNumber);
}

int32_t OT_API::sendUserMessage(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTIdentifier& USER_ID_RECIPIENT,
                                const OTString& RECIPIENT_PUBKEY, // unescaped
                                                                  // and
                                                                  // bookended.
                                const OTString& THE_MESSAGE) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID),
        strNymID2(USER_ID_RECIPIENT);

    // (0) Set up the REQUEST NUMBER and then INCREMENT IT
    pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
    theMessage.m_strRequestNum.Format(
        "%ld", lRequestNumber);                    // Always have to send this.
    pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                   // server request, I have to
                                                   // increment it

    // (1) set up member variables
    theMessage.m_strCommand = "sendUserMessage";
    theMessage.m_strNymID = strNymID;
    theMessage.m_strNymID2 = strNymID2;
    theMessage.m_strServerID = strServerID;
    theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                          // theMessage.m_strServerID is already
                                          // set. (It uses it.)

    OTEnvelope theEnvelope;
    std::unique_ptr<OTAsymmetricKey> pPubkey(OTAsymmetricKey::KeyFactory());
    OT_ASSERT(nullptr != pPubkey);

    int32_t nReturnValue = -1;

    if (!pPubkey->SetPublicKey(RECIPIENT_PUBKEY)) {
        otOut << "OT_API::sendUserMessage: Failed setting public key.\n";
    }
    else if (THE_MESSAGE.Exists() &&
               theEnvelope.Seal(*pPubkey, THE_MESSAGE) &&
               theEnvelope.GetAsciiArmoredData(theMessage.m_ascPayload)) {
        // (2) Sign the Message
        theMessage.SignContract(*pNym);

        // (3) Save the Message (with signatures and all, back to its internal
        // member m_strRawFile.)
        theMessage.SaveContract();

        // (Send it)
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        // store a copy in the outmail.
        // (not encrypted, since the Nymfile will be encrypted anyway.
        //
        OTMessage* pMessage = new OTMessage;

        OT_ASSERT(nullptr != pMessage);

        pMessage->m_strCommand = "outmailMessage";
        pMessage->m_strNymID = strNymID;
        pMessage->m_strNymID2 = strNymID2;
        pMessage->m_strServerID = strServerID;
        pMessage->m_strRequestNum.Format("%ld", lRequestNumber);

        pMessage->m_ascPayload.SetString(THE_MESSAGE);

        pMessage->SignContract(*pNym);
        pMessage->SaveContract();

        pNym->AddOutmail(*pMessage); // Now the Nym is responsible to delete it.
                                     // It's in his "outmail".
        OTPseudonym* pSignerNym = pNym;
        pNym->SaveSignedNymfile(*pSignerNym); // commented out temp for testing.

        nReturnValue = m_pClient->CalcReturnVal(lRequestNumber);
    }
    else
        otOut << "OT_API::sendUserMessage: Failed sealing envelope.\n";

    return nReturnValue;
}

// UPDATE: Sometimes you want to send something to yourself, meaning just put a
// copy in your
// outpayments box, without sending anything to anyone. (Specifically, after a
// withdrawVoucher
// is performed, you want to save a copy in your outbox since the transaction
// number on it is
// signed out to you.) So I'm updating this function so that if USER_ID and
// USER_ID_RECIPIENT
// are the same, it puts a copy in your outpayment box, without sending anything
// at all.
//
int32_t OT_API::sendUserInstrument(
    const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
    const OTIdentifier& USER_ID_RECIPIENT, const OTString& RECIPIENT_PUBKEY,
    const OTPayment& THE_INSTRUMENT,
    const OTPayment* pINSTRUMENT_FOR_SENDER) const // This is only used for cash
                                                   // purses. It's a copy of the
                                                   // purse in THE_INSTRUMENT,
                                                   // except all the tokens are
                                                   // already encrypted to the
// sender's public key, instead
// of the recipient's public
// key (as THE_INSTRUMENT is.)
// This is what we put in the
// sender's outpayments, so he
// can retrieve those tokens if
// he needs to.
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;
    int32_t nReturnValue = -1;
    int64_t lRequestNumber = 0;

    OTString strServerID(SERVER_ID), strNymID(USER_ID),
        strNymID2(USER_ID_RECIPIENT);
    OTString strInstrument, strInstrumentForSender;
    const bool bGotPaymentContents =
        THE_INSTRUMENT.GetPaymentContents(strInstrument);
    const bool bGotSenderPmntCnts =
        (nullptr == pINSTRUMENT_FOR_SENDER)
            ? false
            : pINSTRUMENT_FOR_SENDER->GetPaymentContents(
                  strInstrumentForSender);
    // PREPARE TO SAVE A COPY IN THE OUTPAYMENT BOX
    //
    if (!THE_INSTRUMENT.IsPurse()) {
        // store a copy in the outpayments.
        // (not encrypted, since the Nymfile will be encrypted anyway.)
        //
        // UPDATE: We are now storing a copy in outpayments when the
        // cheque is WRITTEN. But for other instruments (like cash, or
        // vouchers) we cannot store them in outpayments until they are
        // SENT. ...Meaning we must record at this point regardless.
        // Should we have an exception here for cheques?
        //
        // Solution: Let's just make sure there's not already one there...
        //
        int64_t lTempTransNum = 0;
        bool bGotTransNum =
            THE_INSTRUMENT.GetOpeningNum(lTempTransNum, USER_ID);
        int32_t lOutpaymentsIndex =
            bGotTransNum ? GetOutpaymentsIndexByTransNum(*pNym, lTempTransNum)
                         : (-1);

        if (lOutpaymentsIndex > (-1)) // found something that matches...
        {
            // Remove it from Outpayments box. We're adding an updated version
            // of this same instrument here anyway. We can erase the old one.
            //
            if (!pNym->RemoveOutpaymentsByIndex(
                    lOutpaymentsIndex)) // <==== REMOVED! (So the one added
                                        // below isn't a duplicate.)
            {
                otErr << __FUNCTION__
                      << ": Error calling RemoveOutpaymentsByIndex for Nym: "
                      << strNymID << "\n";
            }
            // Save Nym to local storage, since an outpayment was erased.
            // Note: we're saving below anyway. Might as well not save twice.
            //
            //          else if (!pNym->SaveSignedNymfile(*pNym))
            //              otErr << __FUNCTION__ << ": Error saving Nym: "
            //              << strNymID << "\n";
        }
        else
            otOut << __FUNCTION__
                  << ": FYI, didn't remove an older copy of the "
                     "instrument from the payments outbox, "
                     "since I couldn't find it in there.\n";
    }
    // OUTPAYMENT COPY:
    std::unique_ptr<OTMessage> pMessage(new OTMessage);

    pMessage->m_strCommand = "outpaymentsMessage";
    pMessage->m_strNymID = strNymID;
    pMessage->m_strNymID2 = strNymID2;
    pMessage->m_strServerID = strServerID;
    pMessage->m_ascPayload.SetString(bGotSenderPmntCnts ? strInstrumentForSender
                                                        : strInstrument);
    // If they're the same, we only save a copy in the outbox.
    // (We only SEND if they are different.)
    //
    if (USER_ID != USER_ID_RECIPIENT) {
        // (0) Set up the REQUEST NUMBER and then INCREMENT IT
        pNym->GetCurrentRequestNum(strServerID, lRequestNumber);
        theMessage.m_strRequestNum.Format(
            "%ld", lRequestNumber); // Always have to send this.
        pNym->IncrementRequestNum(*pNym, strServerID); // since I used it for a
                                                       // server request, I have
                                                       // to increment it

        // (1) set up member variables
        theMessage.m_strCommand = "sendUserInstrument";
        theMessage.m_strNymID = strNymID;
        theMessage.m_strNymID2 = strNymID2;
        theMessage.m_strServerID = strServerID;
        theMessage.SetAcknowledgments(*pNym); // Must be called AFTER
                                              // theMessage.m_strServerID is
                                              // already set. (It uses it.)

        OTEnvelope theEnvelope;
        std::unique_ptr<OTAsymmetricKey> pPubkey(OTAsymmetricKey::KeyFactory());
        if (!pPubkey->SetPublicKey(RECIPIENT_PUBKEY)) {
            otOut << __FUNCTION__
                  << ": Failed setting public key from string ===>"
                  << RECIPIENT_PUBKEY << "<===\n";
        }
        else if (bGotPaymentContents &&
                   theEnvelope.Seal(*pPubkey, strInstrument) &&
                   theEnvelope.GetAsciiArmoredData(theMessage.m_ascPayload)) {
            // (2) Sign the Message
            theMessage.SignContract(*pNym);

            // (3) Save the Message (with signatures and all, back to its
            // internal member m_strRawFile.)
            theMessage.SaveContract();
            // Back to the outpayments message...
            // (We may want it saved in the outpayment box, before the reply
            // from the
            // above message comes in. Actually that might be wrong, since we
            // care more
            // about the receipt from the recipient depositing the instrument,
            // then we do
            // about the reply for the sendInstrument itself. Anyway, better
            // safe than sorry...)
            //
            pMessage->m_strRequestNum.Format("%ld", lRequestNumber);

            pMessage->SignContract(*pNym);
            pMessage->SaveContract();

            pNym->AddOutpayments(*(pMessage.release()));
            OTPseudonym* pSignerNym = pNym;
            pNym->SaveSignedNymfile(*pSignerNym); // <==== SAVED.
            // (Send it)
            m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                              m_pTransportCallback);
            m_pClient->ProcessMessageOut(theMessage);
            nReturnValue = m_pClient->CalcReturnVal(lRequestNumber);
        }
        else
            otOut << __FUNCTION__ << ": Failed sealing envelope.\n";
    }
    else // (USER_ID == USER_ID_RECIPIENT)
    {
        // You may be wondering why this code seems to repeat?
        // The answer is because above, it needs to happen BEFORE
        // the message is sent, since the processing of the server
        // reply may expect the outpayments copy to already exist.
        //
        // (NOTE: That may actually not be true. That is more true for
        // when the receipt comes in, from the recipient depositing
        // the cheque, versus the server reply to the sendInstrument
        // itself. Anyway...)
        //
        // Whereas here, it needs to happen in the case where the
        // message is NOT sent. (USER as RECIPIENT means "just put
        // a copy in my outpayments box.")
        //
        // But if it DOES happen BEFORE, then we want to properly
        // add the request number to it, even though we apparently
        // don't use the request number on outpayments messages.
        // Whereas here, we don't have a request number, so it just
        // gets set to 0.
        //
        pMessage->m_strRequestNum.Format(
            "%ld", lRequestNumber); // Will be 0 in this case.

        pMessage->SignContract(*pNym);
        pMessage->SaveContract();

        pNym->AddOutpayments(*(pMessage.release()));
        OTPseudonym* pSignerNym = pNym;
        pNym->SaveSignedNymfile(*pSignerNym); // <==== SAVED.
        nReturnValue = 0; // 0 means, nothing was sent, but no error occurred.
    }
    return nReturnValue;
}

// PROBLEM: How can I put anything in an "out" box (ledger) when I can't
// generate a
// transaction number on the client side?  Normally I download the outbox and
// it contains transactions that the SERVER put there--not me!

// THEREFORE!!! The paymentOutbox NEEDS to be like OUTMAIL! It's just a message
// on a Nym. (No transaction numbers.)
// The instrument I sent will be a payload on a message, and that message will
// be copied into the paymentOutbox...

// The messages definitely go OUT as messages, as do instruments, and those same
// messages end up
// "in reference to" on transactions stuffed in my nymbox and then those
// transactions have their
// message copied OUT, and added as a message to my OUTMAIL.
// Therefore Payment Outbox needs to be similar. It will contain the message
// originally sent -- no more.

// whereas payment inbox comes from Nymbox (just copy it over.)
// Similarly, recordbox comes from either payment inbox or asset account inbox.

// So tomorrow:  REMOVE the payment Outbox since it is BULLSHIT and replace with
// "outmail" like functionality for that box.
//
// Update: DONE.

int32_t OT_API::createUserAccount(const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    int32_t nReturnValue = m_pClient->ProcessUserCommand(
        OTClient::createUserAccount, theMessage, *pNym, *pServer,
        nullptr); // nullptr pAccount on this command.
    if (0 < nReturnValue) {
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return nReturnValue;
    }
    else
        otErr << "OT_API::createUserAccount: Error in "
                 "m_pClient->ProcessUserCommand() \n";

    return -1;
}

int32_t OT_API::deleteUserAccount(const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    int32_t nReturnValue = m_pClient->ProcessUserCommand(
        OTClient::deleteUserAccount, theMessage, *pNym, *pServer,
        nullptr); // nullptr pAccount on this command.
    if (0 < nReturnValue) {
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return nReturnValue;
    }
    else
        otErr << "Error processing deleteUserAccount command in "
                 "OT_API::deleteUserAccount\n";

    return -1;
}

int32_t OT_API::checkServerID(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& USER_ID) const
{
    OTPseudonym* pNym = GetOrLoadPrivateNym(
        USER_ID, false, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pNym) return (-1);
    // By this point, pNym is a good pointer, and is on the wallet.
    //  (No need to cleanup.)
    OTServerContract* pServer =
        GetServer(SERVER_ID, __FUNCTION__); // This ASSERTs and logs already.
    if (nullptr == pServer) return (-1);
    // By this point, pServer is a good pointer.  (No need to cleanup.)
    OTMessage theMessage;

    int32_t nReturnValue = m_pClient->ProcessUserCommand(
        OTClient::checkServerID, theMessage, *pNym, *pServer,
        nullptr); // nullptr pAccount on this command.
    if (0 < nReturnValue) {
        m_pClient->SetFocusToServerAndNym(*pServer, *pNym,
                                          m_pTransportCallback);
        m_pClient->ProcessMessageOut(theMessage);

        return nReturnValue;
    }
    else
        otErr << "Error processing checkServerID command in "
                 "OT_API::checkServerID\n";

    return -1;
}

void OT_API::AddServerContract(const OTServerContract& pContract) const
{
    m_pWallet->AddServerContract(pContract);
}

void OT_API::AddAssetContract(const OTAssetContract& theContract) const
{
    m_pWallet->AddAssetContract(theContract);
}

} // namespace opentxs
