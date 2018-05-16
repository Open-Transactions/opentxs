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

#include "opentxs/stdafx.hpp"

#include "opentxs/client/OT_API.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Activity.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Settings.hpp"
#if OT_CASH
#include "opentxs/cash/Mint.hpp"
#include "opentxs/cash/Purse.hpp"
#include "opentxs/cash/Token.hpp"
#endif  // OT_CASH
#include "opentxs/client/Helpers.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OTClient.hpp"
#include "opentxs/client/OTMessageOutbuffer.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/core/crypto/Bip32.hpp"
#endif
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/core/crypto/Bip39.hpp"
#endif
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#if defined(OT_KEYRING_FLATFILE)
#include "opentxs/core/crypto/OTKeyring.hpp"
#endif
#include "opentxs/core/crypto/OTNymOrSymmetricKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSymmetricKey.hpp"
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTAgent.hpp"
#include "opentxs/core/script/OTBylaw.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTPartyAccount.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/ext/InstantiateContract.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/Proto.hpp"

#include <signal.h>
#include <stdlib.h>
#include <cassert>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#ifndef WIN32
#include <unistd.h>
#endif

#if defined(OPENTXS_HAVE_NETINET_IN_H)
extern "C" {
#include <netinet/in.h>
}
#endif

// On certain platforms we can actually check to
// see if the PID is running, and if not, we can
// proceed even if the PID is in the file. (To
// spare the user from having to delete the pid
// file by hand.)
//
#ifdef PREDEF_PLATFORM_UNIX

#if defined(ANDROID)
// blank
#elif defined(TARGET_OS_MAC)

#if TARGET_OS_MAC
#include <sys/wait.h>

#define OT_CHECK_PID 1
#endif

#else
#include <sys/wait.h>

#define OT_CHECK_PID 1
#endif

#endif

#define CLIENT_MASTER_KEY_TIMEOUT_DEFAULT 300
#define CLIENT_WALLET_FILENAME "wallet.xml"
#define CLIENT_USE_SYSTEM_KEYRING false
#define CLIENT_PID_FILENAME "ot.pid"
// -------------------------------------------------------
#define OT_METHOD "opentxs::OT_API::"
// -------------------------------------------------------

// The #defines for the latency values can be found in ServerConnection.cpp.

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
OTTransaction* GetPaymentReceipt(
    const mapOfTransactions& transactionsMap,
    std::int64_t lReferenceNum,
    OTPayment** ppPaymentOut)
{
    // loop through the transactions that make up this ledger.
    for (auto& it : transactionsMap) {
        OTTransaction* transaction = it.second;
        OT_ASSERT(nullptr != transaction);

        if (OTTransaction::paymentReceipt !=
            transaction->GetType())  // <=======
            continue;

        if (transaction->GetReferenceToNum() == lReferenceNum) {
            if (nullptr !=
                ppPaymentOut)  // The caller might want a copy of this.
            {
                String strPayment;
                transaction->GetReferenceString(strPayment);

                if (strPayment.Exists()) {
                    OTPayment* pPayment = new OTPayment(strPayment);
                    OT_ASSERT(nullptr != pPayment);

                    if (pPayment->IsValid() && pPayment->SetTempValues())
                        *ppPaymentOut =
                            pPayment;  // CALLER RESPONSIBLE TO DELETE.
                    else {
                        otErr << OT_METHOD << __FUNCTION__
                              << ": Error: Failed loading up "
                                 "payment instrument from "
                                 "paymentReceipt.\n";
                        delete pPayment;
                        pPayment = nullptr;
                        *ppPaymentOut = nullptr;
                    }
                } else
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Error: Unexpected: payment "
                             "instrument was empty string, on "
                             "a paymentReceipt.\n";
            }

            return transaction;
        }
    }

    return nullptr;
}

bool VerifyBalanceReceipt(
    const ServerContext& context,
    const Identifier& NOTARY_ID,
    const Identifier& accountID)
{
    const auto& THE_NYM = *context.Nym();
    const auto& SERVER_NYM = context.RemoteNym();
    auto NYM_ID = Identifier::Factory(THE_NYM),
         NOTARY_NYM_ID = Identifier::Factory(SERVER_NYM);
    String strNotaryID(NOTARY_ID), strReceiptID(accountID);

    // Load the last successful BALANCE STATEMENT...

    OTTransaction tranOut(NOTARY_NYM_ID, accountID, NOTARY_ID);

    String strFilename;
    strFilename.Format("%s.success", strReceiptID.Get());

    const char* szFolder1name = OTFolders::Receipt().Get();  // receipts
    const char* szFolder2name = strNotaryID.Get();  // receipts/NOTARY_ID
    const char* szFilename =
        strFilename.Get();  // receipts/NOTARY_ID/accountID.success

    if (!OTDB::Exists(szFolder1name, szFolder2name, szFilename)) {
        otWarn << "Receipt file doesn't exist in "
                  "OTTransaction::VerifyBalanceReceipt.\n";
        return false;
    }

    std::string strFileContents(OTDB::QueryPlainString(
        szFolder1name,
        szFolder2name,
        szFilename));  // <=== LOADING FROM DATA STORE.

    if (strFileContents.length() < 2) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Error reading file: "
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    String strTransaction(strFileContents.c_str());

    if (!tranOut.LoadContractFromString(strTransaction)) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Unable to load balance "
                 "statement:\n "
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    OTTransaction* transaction = nullptr;
    std::unique_ptr<OTTransaction> theTransAngel;

    if (tranOut.IsAbbreviated())  // should never happen
    {
        std::int64_t lBoxType = 0;

        if (tranOut.Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(Ledger::nymbox);
        else if (tranOut.Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(Ledger::inbox);
        else if (tranOut.Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(Ledger::outbox);
        else if (tranOut.Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(Ledger::paymentInbox);
        else if (tranOut.Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(Ledger::recordBox);
        else if (tranOut.Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(Ledger::expiredBox);
        else {
            otErr << "OTTransaction::VerifyBalanceReceipt: Error loading from "
                     "abbreviated transaction: "
                     "unknown ledger type. (probably message but who knows.)\n";
            return false;
        }

        transaction = LoadBoxReceipt(tranOut, lBoxType);
        if (nullptr == transaction) {
            otErr << "OTTransaction::VerifyBalanceReceipt: Error loading from "
                     "abbreviated transaction: "
                     "failed loading box receipt.";
            return false;
        }

        theTransAngel.reset(transaction);
    } else
        transaction = &tranOut;

    if (!transaction->VerifySignature(SERVER_NYM)) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Unable to verify "
                 "SERVER_NYM signature on balance statement:\n "
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    // At this point, transaction is successfully loaded and verified,
    // containing the last balance receipt.

    return transaction->VerifyBalanceReceipt(context);
}

}  // namespace

class OT_API::Pid
{
public:
    Pid();
    ~Pid();
    void OpenPid(const String& strPidFilePath);
    void ClosePid();
    bool IsPidOpen() const;
    static bool PIDAutorecoverImpossible(std::uint32_t pid);

private:
    bool m_bIsPidOpen;
    String m_strPidFilePath;
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

// static
bool OT_API::Pid::PIDAutorecoverImpossible(std::uint32_t pid)
{
#ifdef OT_CHECK_PID
    while (waitpid(-1, 0, WNOHANG) > 0) {
        // Wait for defunct....
    }

    if (0 != kill(pid, 0)) return false;  // Process exists
#endif
    return true;
}

void OT_API::Pid::OpenPid(const String& strPidFilePath)
{
    if (IsPidOpen()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": strPidFilePath is OPEN, MUST CLOSE BEFORE "
                 "OPENING A NEW ONE!\n";
        OT_FAIL;
    }

    if (!strPidFilePath.Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": strPidFilePath is Empty!\n";
        OT_FAIL;
    }
    if (3 > strPidFilePath.GetLength()) {
        otErr << OT_METHOD << __FUNCTION__ << ": strPidFilePath is Too Short! ("
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

        if (pid_infile.is_open())  // it existed already
        {
            std::uint32_t old_pid = 0;
            pid_infile >> old_pid;
            pid_infile.close();

            // There was a real PID in there.
            if ((old_pid != 0) && PIDAutorecoverImpossible(old_pid)) {
#if !(defined(ANDROID) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE))
                const std::uint64_t lPID = old_pid;
                otErr
                    << "\n\n\nIS OPEN-TRANSACTIONS ALREADY RUNNING?\n\n"
                       "I found a PID ("
                    << lPID << ") in the data lock file, located at: "
                    << m_strPidFilePath
                    << "\n\n"
                       "If the OT process with PID "
                    << lPID
                    << " is truly not running "
                       "anymore, "
                       "then just erase that file and restart.\nThis is "
                       "normally "
                       "cleaned "
                       "up during AppCleanup / AppShutdown. (Or should be.)\n";

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
        std::uint64_t the_pid = 0;

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
        } else {
            otErr << "Failed trying to open data locking file (to store "
                     "PID "
                  << the_pid << "): " << m_strPidFilePath << "\n";
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
        otErr << OT_METHOD << __FUNCTION__ << ": m_strPidFilePath is Empty!\n";
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
        std::uint32_t the_pid = 0;
        pid_outfile << the_pid;
        pid_outfile.close();
        m_bIsPidOpen = false;
    } else {
        otErr << "Failed trying to open data locking file (to wipe PID "
                 "back to 0): "
              << m_strPidFilePath << "\n";
        m_bIsPidOpen = true;
    }
}

bool OT_API::Pid::IsPidOpen() const { return m_bIsPidOpen; }

OT_API::OT_API(
    const api::Activity& activity,
    const api::Settings& config,
    const api::ContactManager& contacts,
    const api::Crypto& crypto,
    const api::Identity& identity,
    const api::storage::Storage& storage,
    const api::client::Wallet& wallet,
    const api::client::Workflow& workflow,
    const api::network::ZMQ& zmq,
    const ContextLockCallback& lockCallback)
    : activity_(activity)
    , config_(config)
    , contacts_(contacts)
    , crypto_(crypto)
    , identity_(identity)
    , storage_(storage)
    , wallet_(wallet)
    , workflow_(workflow)
    , zeromq_(zmq)
    , m_strDataPath("")
    , m_strWalletFilename("")
    , m_strWalletFilePath("")
    , m_strConfigFilename("")
    , m_strConfigFilePath("")
    , m_pWallet(nullptr)
    , m_pClient(nullptr)
    , lock_callback_(lockCallback)
{
    pid_.reset(new Pid);

    if (!Init()) {
        Cleanup();
        OT_FAIL;
    }
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
    if (!LoadConfigFile()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to Load Config File!";

        return false;
    }

    // PID -- Make sure we're not running two copies of OT on the same data
    // simultaneously here.
    //
    // we need to get the loacation of where the pid file should be.
    // then we pass it to the OpenPid function.
    String strDataPath = "";
    const bool bGetDataFolderSuccess = OTDataFolder::Get(strDataPath);

    {
        bool bExists = false, bIsNew = false;
        if (!OTPaths::ConfirmCreateFolder(strDataPath, bExists, bIsNew)) {
            return false;
        }
    }

    String strPIDPath = "";
    OTPaths::AppendFile(strPIDPath, strDataPath, CLIENT_PID_FILENAME);

    if (bGetDataFolderSuccess) pid_->OpenPid(strPIDPath);

    if (!pid_->IsPidOpen()) {
        return false;
    }  // failed loading

    // This way, everywhere else I can use the default storage context (for now)
    // and it will work everywhere I put it. (Because it's now set up...)
    //
    m_bDefaultStore = OTDB::InitDefaultStorage(
        OTDB_DEFAULT_STORAGE,
        OTDB_DEFAULT_PACKER);  // We only need to do this once now.

    if (m_bDefaultStore) {
        otWarn << __FUNCTION__ << ": Success invoking OTDB::InitDefaultStorage";

        m_pWallet = new OTWallet(crypto_, storage_);
        m_pClient.reset(
            new OTClient(*m_pWallet, activity_, contacts_, wallet_, workflow_));

        OT_ASSERT(m_pClient);

        return true;
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed invoking OTDB::InitDefaultStorage\n";
    }

    return false;
}

bool OT_API::Cleanup()
{
    if (!pid_->IsPidOpen()) {

        return false;
    }  // pid isn't open, just return false.

    pid_->ClosePid();

    if (pid_->IsPidOpen()) {
        OT_FAIL;
    }  // failed closing

    return true;
}

// Get
bool OT_API::GetWalletFilename(String& strPath) const
{
    if (m_strWalletFilename.Exists()) {
        strPath = m_strWalletFilename;
        return true;
    } else {
        strPath.Set("");
        return false;
    }
}

// Set
bool OT_API::SetWalletFilename(const String& strPath) const
{
    if (strPath.Exists()) {
        m_strWalletFilename = strPath;
        return true;
    } else
        return false;
}

// Load the configuration file.
//
bool OT_API::LoadConfigFile()
{
    Lock lock(lock_);

    // LOG LEVEL
    {
        bool bIsNewKey = false;
        std::int64_t lValue = 0;
        config_.CheckSet_long("logging", "log_level", 0, lValue, bIsNewKey);
        Log::SetLogLevel(static_cast<std::int32_t>(lValue));
    }

    // WALLET

    // WALLET FILENAME
    //
    // Clean and Set
    {
        bool bIsNewKey = false;
        String strValue;
        config_.CheckSet_str(
            "wallet",
            "wallet_filename",
            CLIENT_WALLET_FILENAME,
            strValue,
            bIsNewKey);
        OT_API::SetWalletFilename(strValue);
        otWarn << "Using Wallet: " << strValue << "\n";
    }

    // LATENCY
    {
        const char* szComment =
            ";; LATENCY:\n\n"
            ";; - linger is the number of milliseconds OT will try to send any "
            "outgoing messages queued in a socket that's being closed.\n"
            ";; - send_timeout is the number of milliseconds OT will wait "
            "while sending a message, before it gives up.\n"
            ";; - recv_timeout is the number of milliseconds OT will wait "
            "while receiving a reply, before it gives up.\n";

        bool b_SectionExist = false;
        config_.CheckSetSection("latency", szComment, b_SectionExist);
    }

    // SECURITY (beginnings of..)

    // Master Key Timeout
    {
        const char* szComment =
            "; master_key_timeout is how long the master key will be in "
            "memory until a thread wipes it out.\n"
            "; 0   : This means you have to type your password EVERY time OT "
            "uses a "
            "private key. (Even multiple times in a single function.)\n"
            "; 300 : This means you only have to type it once per 5 minutes.\n"
            "; -1  : This means you only type it once PER RUN (popular for "
            "servers.)\n";

        bool bIsNewKey = false;
        std::int64_t lValue = 0;
        config_.CheckSet_long(
            "security",
            "master_key_timeout",
            CLIENT_MASTER_KEY_TIMEOUT_DEFAULT,
            lValue,
            bIsNewKey,
            szComment);
        crypto_.SetTimeout(std::chrono::seconds(lValue));
    }

    // Use System Keyring
    {
        bool bValue = false, bIsNewKey = false;
        config_.CheckSet_bool(
            "security",
            "use_system_keyring",
            CLIENT_USE_SYSTEM_KEYRING,
            bValue,
            bIsNewKey);
        crypto_.SetSystemKeyring(bValue);

#if defined(OT_KEYRING_FLATFILE)
        // Is there a password folder? (There shouldn't be, but we allow it...)
        //
        if (bValue) {
            bool bIsNewKey2 = false;
            String strValue;
            config_.CheckSet_str(
                "security", "password_folder", "", strValue, bIsNewKey2);
            if (strValue.Exists()) {
                OTKeyring::FlatFile_SetPasswordFolder(strValue.Get());
                otErr << " **DANGEROUS!**  Using password folder: " << strValue
                      << "\n";
            }
        }
#endif
    }

    // Done Loading... Lets save any changes...
    if (!config_.Save()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error! Unable to save updated Config!!!\n";
        OT_FAIL;
    }

    return true;
}

bool OT_API::SetWallet(const String& strFilename) const
{
    Lock lock(lock_);

    OT_NEW_ASSERT_MSG(strFilename.Exists(), "strFilename does not exist.");
    OT_NEW_ASSERT_MSG(
        (3 < strFilename.GetLength()), "strFilename is too short.");

    // Set New Wallet Filename
    otErr << OT_METHOD << __FUNCTION__ << ": Setting Wallet Filename... \n";
    String strWalletFilename;
    OT_API::GetWalletFilename(strWalletFilename);

    if (strFilename.Compare(strWalletFilename)) {
        otWarn << OT_METHOD << __FUNCTION__
               << ": Wallet Filename: " << strFilename
               << "  is same as in configuration. (skipping)\n";
        return true;
    } else
        strWalletFilename.Set(strWalletFilename);

    // Set New Wallet Filename
    {
        bool bNewOrUpdated;
        config_.Set_str(
            "wallet",
            "wallet_filename",
            strWalletFilename,
            bNewOrUpdated,
            "; Wallet updated\n");

        OT_API::SetWalletFilename(strWalletFilename);
    }

    // Done Loading... Lets save any changes...
    if (!config_.Save()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error! Unable to save updated Config!!!\n";
        OT_FAIL;
    }

    otErr << OT_METHOD << __FUNCTION__
          << ": Updated Wallet filename: " << strWalletFilename << " \n";

    return true;
}

bool OT_API::WalletExists() const
{
    Lock lock(lock_);

    return (nullptr != m_pWallet) ? true : false;
}

bool OT_API::LoadWallet() const
{
    Lock lock(lock_);

    OT_ASSERT_MSG(
        m_bDefaultStore,
        "Default Storage not Initialized; call OT_API::Init first.\n");

    String strWalletFilename;
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
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed invoking m_pWallet->LoadWallet() with filename: "
              << strWalletFilename << "\n";
    return bSuccess;
}

std::int32_t OT_API::GetNymCount() const { return wallet_.LocalNymCount(); }

std::set<OTIdentifier> OT_API::LocalNymList() const
{
    return wallet_.LocalNyms();
}

std::set<AccountInfo> OT_API::Accounts() const
{
    auto wallet = GetWallet(__FUNCTION__);

    OT_ASSERT(nullptr != wallet)

    return wallet->AccountList();
}

std::int32_t OT_API::GetAccountCount() const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__);  // This logs and ASSERTs already.
    if (nullptr != pWallet) return pWallet->GetAccountCount();

    return 0;
}

bool OT_API::GetNym(std::int32_t iIndex, Identifier& NYM_ID, String& NYM_NAME)
    const
{
    if (wallet_.NymNameByIndex(iIndex, NYM_NAME)) {
        NYM_ID = Identifier::Factory(NYM_NAME);

        return true;
    }

    return false;
}

bool OT_API::GetAccount(
    std::int32_t iIndex,
    Identifier& THE_ID,
    String& THE_NAME) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__);  // This logs and ASSERTs already.

    if (nullptr != pWallet)
        return pWallet->GetAccount(iIndex, THE_ID, THE_NAME);

    return false;
}

OTWallet* OT_API::GetWallet(const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTWallet* pWallet = m_pWallet;  // This is where we "get" the wallet.  :P

    if (nullptr == pWallet)
        otErr << szFunc << ": -- The Wallet is not loaded.\n";

    return pWallet;
}

// Wallet owns this pointer. Do not delete
const BasketContract* OT_API::GetBasketContract(
    const Identifier& THE_ID,
    const char* szFunc) const
{
    auto contract = wallet_.UnitDefinition(THE_ID);
    if (!contract) {
        if (nullptr != szFunc) {  // We only log if the caller asked us to.
            const String strID(THE_ID);
            otWarn << __FUNCTION__ << " " << szFunc
                   << ": No asset contract "
                      "found in wallet with ID: "
                   << strID << "\n";
        }
    } else {
        auto currency = dynamic_cast<const BasketContract*>(contract.get());
        if (nullptr != currency) {
            return currency;
        }
    }

    return nullptr;
}

std::shared_ptr<Account> OT_API::GetAccount(
    const Identifier& THE_ID,
    const char* szFunc) const
{
    OTWallet* pWallet = GetWallet(nullptr != szFunc ? szFunc : __FUNCTION__);

    if (nullptr != pWallet) {
        auto account = pWallet->GetAccount(THE_ID);

        if ((nullptr == account) &&
            (nullptr != szFunc))  // We only log if the caller asked us to.
        {
            const String strID(THE_ID);
            otWarn << __FUNCTION__ << " " << szFunc
                   << ": No account found in "
                      "wallet with ID: "
                   << strID << "\n";
        }

        return account;
    }

    return nullptr;
}

std::shared_ptr<Account> OT_API::GetAccountPartialMatch(
    const std::string PARTIAL_ID,
    const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTWallet* pWallet = GetWallet(szFunc);  // This logs and ASSERTs already.

    if (nullptr != pWallet) return pWallet->GetAccountPartialMatch(PARTIAL_ID);

    return nullptr;
}

bool OT_API::IsNym_RegisteredAtServer(
    const Identifier& NYM_ID,
    const Identifier& NOTARY_ID) const
{
    if (NYM_ID.IsEmpty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }

    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (context) {

        return (0 != context->Request());
    }

    return false;
}

// --------------------------------------------------------------------
/**

 Since all the Nyms are encrypted to the master key, and since we can change the
 passphrase on the master key without changing the master key itself, then we
 don't have to do anything to update all the Nyms, since that part hasn't
 changed.

 (Make sure to save the wallet also.)
 */
bool OT_API::Wallet_ChangePassphrase() const
{
    Lock lock(lock_);

    OTWallet* pWallet = GetWallet(__FUNCTION__);

    if (nullptr == pWallet) {

        return false;
    }

    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    auto key = crypto_.mutable_DefaultKey();
    auto& cachedKey = key.It();

    if (!cachedKey.IsGenerated()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Wallet master cached key doesn't exist. "
                 "Try creating a new Nym first.\n";
        return false;
    }

    OTASCIIArmor ascBackup;
    cachedKey.SerializeTo(ascBackup);  // Just in case!

    const bool bSuccess = cachedKey.ChangeUserPassphrase();

    if (!bSuccess) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to change the user master passphrase.\n";
        return false;
    }

    const bool bSaved = pWallet->SaveWallet();

    if (!bSaved) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed saving wallet (reverting.)\n";

        if (cachedKey.SerializeFrom(ascBackup)) {
            pWallet->SaveWallet();
        }

        return false;
    } else {
        otErr << "\nSuccess changing master passphrase for wallet!\n";
    }

    return true;
}

std::string OT_API::Wallet_GetPhrase() const
{
#if OT_CRYPTO_WITH_BIP32
    OTWallet* pWallet = GetWallet(__FUNCTION__);

    if (nullptr == pWallet) {
        return "";
    };
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    auto& cachedKey = crypto_.DefaultKey();

    if (!cachedKey.IsGenerated()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Wallet master cached key doesn't exist. "
                 "Try creating a new Nym first.\n";
        return "";
    }

    return pWallet->GetPhrase();
#else
    return "";
#endif
}

std::string OT_API::Wallet_GetSeed() const
{
#if OT_CRYPTO_WITH_BIP32
    OTWallet* pWallet = GetWallet(__FUNCTION__);

    if (nullptr == pWallet) {
        return "";
    }

    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    auto& cachedKey = crypto_.DefaultKey();

    if (!cachedKey.IsGenerated()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Wallet master cached key doesn't exist. "
                 "Try creating a new Nym first.\n";
        return "";
    }

    return pWallet->GetSeed();
#else
    return "";
#endif
}

std::string OT_API::Wallet_GetWords() const
{
#if OT_CRYPTO_WITH_BIP39
    OTWallet* pWallet = GetWallet(__FUNCTION__);

    if (nullptr == pWallet) {
        return "";
    };
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    auto& cachedKey = crypto_.DefaultKey();

    if (!cachedKey.IsGenerated()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Wallet master cached key doesn't exist. "
                 "Try creating a new Nym first.\n";
        return "";
    }

    return pWallet->GetWords();
#else
    return "";
#endif
}

std::string OT_API::Wallet_ImportSeed(
    __attribute__((unused)) const OTPassword& words,
    __attribute__((unused)) const OTPassword& passphrase) const
{
    Lock lock(lock_);

    std::string output;
#if OT_CRYPTO_WITH_BIP39
    OTWallet* pWallet =
        GetWallet(__FUNCTION__);  // This logs and ASSERTs already.

    if (nullptr == pWallet) {
        return "";
    };

    output = pWallet->ImportSeed(words, passphrase);
    pWallet->SaveWallet();
#endif

    return output;
}

bool OT_API::Wallet_CanRemoveServer(const Identifier& NOTARY_ID) const
{
    Lock lock(lock_);

    if (NOTARY_ID.IsEmpty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null: NOTARY_ID passed in!\n";
        OT_FAIL;
    }
    String strName;
    const std::int32_t nCount = GetAccountCount();

    // Loop through all the accounts.
    for (std::int32_t i = 0; i < nCount; i++) {
        auto accountID = Identifier::Factory();
        GetAccount(i, accountID, strName);
        auto account = GetAccount(accountID, __FUNCTION__);
        auto purportedNotaryID =
            Identifier::Factory(account->GetPurportedNotaryID());

        if (NOTARY_ID == purportedNotaryID) {
            String strPurportedNotaryID(purportedNotaryID),
                strNOTARY_ID(NOTARY_ID);
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to remove server contract " << strNOTARY_ID
                  << " from wallet, because Account " << strPurportedNotaryID
                  << " uses it.\n";
            return false;
        }
    }

    // Loop through all the Nyms. (One might be registered on that server.)
    //
    std::set<OTIdentifier> nymIDs = wallet_.LocalNyms();
    for (auto& nymID : nymIDs) {
        if (IsNym_RegisteredAtServer(nymID, NOTARY_ID)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to remove server contract " << NOTARY_ID.str()
                  << " "
                     "from wallet, because Nym "
                  << nymID->str()
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
bool OT_API::Wallet_CanRemoveAssetType(
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    Lock lock(lock_);

    if (INSTRUMENT_DEFINITION_ID.IsEmpty()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Null: INSTRUMENT_DEFINITION_ID passed in!\n";
        OT_FAIL;
    }
    String strName;
    const std::int32_t nCount = GetAccountCount();

    // Loop through all the accounts.
    for (std::int32_t i = 0; i < nCount; i++) {
        auto accountID = Identifier::Factory();
        GetAccount(i, accountID, strName);
        auto account = GetAccount(accountID, __FUNCTION__);
        auto theTYPE_ID =
            Identifier::Factory(account->GetInstrumentDefinitionID());

        if (INSTRUMENT_DEFINITION_ID == theTYPE_ID) {
            String strINSTRUMENT_DEFINITION_ID(INSTRUMENT_DEFINITION_ID),
                strTYPE_ID(theTYPE_ID);

            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to remove asset contract "
                  << strINSTRUMENT_DEFINITION_ID << " from wallet: Account "
                  << strTYPE_ID << " uses it.\n";
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
bool OT_API::Wallet_CanRemoveNym(const Identifier& NYM_ID) const
{
    Lock lock(lock_);

    if (NYM_ID.IsEmpty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null: NYM_ID passed in!\n";
        OT_FAIL;
    }

    auto nym = wallet_.Nym(NYM_ID);
    if (false == bool(nym)) return false;
    // Make sure the Nym doesn't have any accounts in the wallet.
    // (Client must close those before calling this.)
    //
    const std::int32_t nCount = GetAccountCount();

    // Loop through all the accounts.
    for (std::int32_t i = 0; i < nCount; i++) {
        auto accountID = Identifier::Factory();
        String strName;
        GetAccount(i, accountID, strName);
        auto account = GetAccount(accountID, __FUNCTION__);
        auto theNYM_ID = Identifier::Factory(account->GetNymID());

        if (theNYM_ID->IsEmpty()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Bug in OT_API_Wallet_CanRemoveNym / "
                     "OT_API_GetAccountWallet_NymID\n";
            return false;
        }

        // Looks like the Nym still has some accounts in this wallet.
        if (NYM_ID == theNYM_ID) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Nym cannot be removed because there are "
                     "still accounts in the wallet for that Nym.\n";
            return false;
        }
    }

    // Make sure the Nym isn't registered at any servers...
    // (Client must unregister at those servers before calling this function..)
    //
    for (auto& server : wallet_.ServerList()) {
        auto context =
            wallet_.ServerContext(nym->ID(), Identifier::Factory(server.first));

        if (context) {
            if (0 != context->Request()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Nym cannot be removed because there"
                         " are still servers in the wallet that "
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
bool OT_API::Wallet_CanRemoveAccount(const Identifier& ACCOUNT_ID) const
{
    Lock lock(lock_);

    if (ACCOUNT_ID.IsEmpty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Null: ACCOUNT_ID passed in!\n";
        OT_FAIL;
    }

    const String strAccountID(ACCOUNT_ID);
    auto account = GetAccount(ACCOUNT_ID, __FUNCTION__);

    if (false == bool(account)) return false;

    // Balance must be zero in order to close an account!
    else if (account->GetBalance() != 0) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Account balance MUST be zero in order to close "
                 "an asset account: "
              << strAccountID << ".\n";
        return false;
    } else if (account->IsIssuer()) {
        otErr
            << __FUNCTION__
            << ": One does not simply delete an issuer account: "
            << strAccountID
            << ". "
               "Yes, the account is empty, but the unit type is still issued "
               "onto "
               "the notary. It must be un-issued, before it can be destroyed.";
        return false;
    }

    bool BOOL_RETURN_VALUE = false;

    const Identifier& theNotaryID = account->GetPurportedNotaryID();
    const Identifier& theNymID = account->GetNymID();

    // There is an OT_ASSERT in here for memory failure,
    // but it still might return nullptr if various verification fails.
    std::unique_ptr<Ledger> inbox(LoadInbox(theNotaryID, theNymID, ACCOUNT_ID));
    std::unique_ptr<Ledger> outbox(
        LoadOutbox(theNotaryID, theNymID, ACCOUNT_ID));

    if (!inbox) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure calling OT_API::LoadInbox.\n Account ID: "
              << strAccountID << "\n";
    } else if (!outbox) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure calling OT_API::LoadOutbox.\n Account ID: "
              << strAccountID << "\n";
    } else if (
        (inbox->GetTransactionCount() > 0) ||
        (outbox->GetTransactionCount() > 0)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: You cannot remove an asset account if "
                 "there are inbox/outbox items still waiting to be "
                 "processed.\n";
    } else
        BOOL_RETURN_VALUE = true;  // SUCCESS!

    return BOOL_RETURN_VALUE;
}

// OT has the capability to export a Nym (normally stored in several files) as
// an encoded
// object (in base64-encoded form) and then import it again.
//
// Returns bool on success, and strOutput will contain the exported data.
bool OT_API::Wallet_ExportNym(const Identifier& NYM_ID, String& strOutput) const
{
    Lock lock(lock_);

    if (NYM_ID.IsEmpty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": NYM_ID is empty!";
        OT_FAIL;
    }

    OTPasswordData thePWDataSave("Create new passphrase for exported Nym.");
    String strReasonToSave(thePWDataSave.GetDisplayString());
    auto nym = wallet_.Nym(NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    std::string str_nym_name(nym->Alias());
    String strID;
    nym->GetIdentifier(strID);
    std::string str_nym_id(strID.Get());
    // Below this point I can use:
    //
    // nymfile, str_nym_name, and str_nym_id.
    //
    // I still need the certfile and the nymfile (both in string form.)
    //

    OT_ASSERT(nym->GetMasterCredentialCount() > 0);

    OTASCIIArmor ascCredentials, ascCredList;
    String strCertfile;
    bool bSavedCert = false;

    // We don't have to pause OTCachedKey here, because
    // this already has built-in mechanisms to go around OTCachedKey.
    //
    const bool bReEncrypted = nym->ReEncryptPrivateCredentials(
        false /*bImporting*/,
        &thePWDataSave);  // Handles OTCachedKey already.
    if (bReEncrypted) {
        // Create a new OTDB::StringMap object.

        // this asserts already, on failure.
        std::unique_ptr<OTDB::Storable> pStorable(
            OTDB::CreateObject(OTDB::STORED_OBJ_STRING_MAP));
        OTDB::StringMap* pMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());
        if (nullptr == pMap)
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error: failed trying to load or create a "
                     "STORED_OBJ_STRING_MAP.\n";
        else  // It instantiated.
        {
            String strCredList;
            String::Map& theMap = pMap->the_map;

            nym->GetPrivateCredentials(strCredList, &theMap);
            // Serialize the StringMap to a string...

            // Won't bother if there are zero credentials somehow.
            if (strCredList.Exists() && (!theMap.empty())) {
                std::string str_Encoded = OTDB::EncodeObject(*pMap);
                const bool bSuccessEncoding = (str_Encoded.size() > 0);
                if (bSuccessEncoding) {
                    ascCredList.SetString(strCredList);  // <========== Success
                    ascCredentials.Set(
                        str_Encoded.c_str());  // Payload contains
                                               // credentials list, payload2
                                               // contains actual
                                               // credentials.
                    bSavedCert = true;
                }
            }
        }
    }  // bReEncrypted.

    if (!bSavedCert) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed while saving Nym's private cert, or private "
                 "credentials, to string.\n"
                 "Reason I was doing this: \""
              << thePWDataSave.GetDisplayString() << "\"\n";
        return false;
    }
    String strNymfile;
    const bool bSavedNym = nym->SavePseudonym(strNymfile);

    if (!bSavedNym) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed while calling "
                 "nymfile->SavePseudonym(strNymfile) (to string)\n";
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
        otErr << OT_METHOD << __FUNCTION__
              << ": Error: failed trying to load or create a "
                 "STORED_OBJ_STRING_MAP.\n";
        return false;
    }
    String::Map& theMap = pMap->the_map;
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
            strOutput, "EXPORTED NYM"  // -----BEGIN OT EXPORTED NYM-----
        );                             // (bool bEscaped=false by default.)
    }

    return bReturnVal;
}

// OT has the capability to export a Nym (normally stored in several files) as
// an encoded
// object (in base64-encoded form) and then import it again.
//
// Returns bool on success, and if nymfileID is passed in, will set it to the
// new
// NymID.
// Also on failure, if the Nym was already there with that ID, and if nymfileID
// is
// passed,
// then it will be set to the ID that was already there.
//
bool OT_API::Wallet_ImportNym(
    const String& FILE_CONTENTS,
    Identifier* nymfileID) const
{
    Lock lock(lock_);

    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    OTASCIIArmor ascArmor;
    const bool bLoadedArmor = OTASCIIArmor::LoadFromString(
        ascArmor, FILE_CONTENTS);  // str_bookend="-----BEGIN" by default
    if (!bLoadedArmor || !ascArmor.Exists()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure loading string into OTASCIIArmor object:\n\n"
              << FILE_CONTENTS << "\n\n";
        return false;
    }
    std::unique_ptr<OTDB::Storable> pStorable(
        OTDB::DecodeObject(OTDB::STORED_OBJ_STRING_MAP, ascArmor.Get()));
    OTDB::StringMap* pMap = dynamic_cast<OTDB::StringMap*>(pStorable.get());

    if (nullptr == pMap) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed decoding StringMap object while trying "
                 "to import Nym:\n"
              << FILE_CONTENTS << "\n";
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
    if (theMap.end() == theMap.find("id"))  // todo hardcoding
    {
        // not found.
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to find 'id' field while trying to import Nym:\n"
              << FILE_CONTENTS << "\n";
        return false;
    }
    if (theMap.end() == theMap.find("name"))  // todo hardcoding
    {
        // not found.
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to find 'name' field while trying to import Nym:\n"
              << FILE_CONTENTS << "\n";
        return false;
    }
    if (theMap.end() == theMap.find("nymfile"))  // todo hardcoding
    {
        // not found.
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to find 'nymfile' field while trying to "
                 "import Nym:\n"
              << FILE_CONTENTS << "\n";
        return false;
    }
    if ((theMap.end() == theMap.find("certfile")) &&  // todo hardcoding
        (theMap.end() == theMap.find("credlist")))  // Logic: No certfile AND no
                                                    // credlist? Gotta have one
                                                    // or the other.
    {
        // not found.
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to find a 'certfile' nor a 'credlist' "
                 "field while trying to import Nym:\n"
              << FILE_CONTENTS << "\n";
        return false;
    }
    // Do various verifications on the values to make sure there's no funny
    // business.
    //
    // If Nym with this ID is ALREADY in the wallet, set nymfileID and return
    // false.
    const auto theNymID = Identifier::Factory(theMap["id"]);
    const String strNymName(theMap["name"].c_str());

    if (nullptr != nymfileID) nymfileID->SetString(theMap["id"]);
    if (theNymID->IsEmpty()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error: NYM_ID passed in is empty; returning false";
        return false;
    }
    // MAKE SURE IT'S NOT ALREADY IN THE WALLET.
    //
    bool exists = wallet_.IsLocalNym(theMap["id"]);

    if (exists) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Tried to import a Nym that's already in wallet: "
              << theMap["id"] << "\n";
        return false;
    }
    // Create a new Nym object.
    //
    const String strNymID(theNymID);
    auto nym = std::make_unique<Nym>(theNymID);

    OT_ASSERT(nym);

    nym->SetAlias(strNymName.Get());

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
    String strDisplay("Enter passphrase for the Nym being imported.");

    // Circumvents the cached key.

    // bAskTwice is true when exporting (since the export passphrase is being
    // created at that time.) But here during importing, we just ask once,
    // since the passphrase is being used, not created.
    std::unique_ptr<OTPassword> pExportPassphrase(
        OTSymmetricKey::GetPassphraseFromUser(&strDisplay, false));

    if (nullptr == pExportPassphrase) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed in GetPassphraseFromUser.\n";
        return false;
    }

    // Pause the master key, since this Nym is coming from outside
    // the wallet. We'll let it load with its own key. (No point using
    // the internal wallet master key here, since the Nym is coming from
    // outside, and doesn't use it anyway.)
    // This is true regardless of whether we load via the old system or
    // the new credentials system.
    auto key = crypto_.mutable_DefaultKey();
    auto& cachedKey = key.It();

    if (!(cachedKey.isPaused())) {
        cachedKey.Pause();  // BELOW THIS POINT, CACHED MASTER KEY IS
                            // DISABLED.
    }
    // Set the credentials or keys on the new Nym object based on the
    // certfile from the StringMap.
    //
    bool bIfNymLoadKeys = false;
    String strReasonToLoad(
        "(ImportNym) To import this Nym, what is its passphrase? ");
    String strReasonToSave(
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
        String strCredList;
        if (it_credlist->second.size() > 0) {
            ascCredList.Set(it_credlist->second.c_str());
            ascCredList.GetString(strCredList);
        }
        // cred list exists, and found "credentials"...
        //
        if (strCredList.Exists() &&
            (theMap.end() != it_credentials))  // and found "credentials"
        {
            OTASCIIArmor ascCredentials;
            if (it_credentials->second.size() > 0) {
                ascCredentials.Set(it_credentials->second.c_str());
                std::unique_ptr<OTDB::Storable> pPrivateStorable(
                    OTDB::DecodeObject(
                        OTDB::STORED_OBJ_STRING_MAP, ascCredentials.Get()));
                OTDB::StringMap* pPrivateMap =
                    dynamic_cast<OTDB::StringMap*>(pPrivateStorable.get());
                if (nullptr == pPrivateMap) {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Failed decoding StringMap object.\n";
                    return false;
                } else  // IF the list saved, then we save the credentials
                        // themselves...
                {
                    String::Map& thePrivateMap = pPrivateMap->the_map;
                    bool unused = false;

                    if (false == nym->LoadNymFromString(
                                     strCredList,
                                     unused,
                                     &thePrivateMap,
                                     &strReasonToLoad,
                                     pExportPassphrase.get())) {
                        otErr << OT_METHOD << __FUNCTION__
                              << ": Failure loading nym " << strNymID
                              << " from credential string.\n";
                        return false;
                    } else  // Success
                    {
                        bIfNymLoadKeys = true;  // <============
                        bHasCredentials = true;
                    }
                }  // success decoding StringMap
            }      // it_credentials.second().size() > 0
        }          // strCredList.Exists() and it_credentials found.
    }              // found "credlist"
    // Unpause the OTCachedKey (wallet master key.)
    // Now that we've loaded up the "outsider" using its own key,
    // we now resume normal wallet master key operations so that when
    // we save the Nym, it will be saved using the wallet master key.
    // (Henceforth, it has been "imported.")

    if (cachedKey.isPaused()) {
        cachedKey.Unpause();  // BELOW THIS POINT, CACHED MASTER KEY IS
                              // BACK IN EFFECT. (Disabled above.)
    }

    if (bIfNymLoadKeys && nym->VerifyPseudonym()) {
        // Before we go on switching the credentials around, let's make sure
        // this Nym we're
        // importing isn't already in the wallet.

        if (bHasCredentials &&
            !nym->ReEncryptPrivateCredentials(
                true, &thePWDataLoad, pExportPassphrase.get()))
        // Handles OTCachedKey internally, no need for pausing for this call
        {
            otErr
                << __FUNCTION__
                << ": Failed trying to re-encrypt Nym's private credentials.\n";
            return false;
        }
        // load Nymfile from string
        //
        const String strNymfile(theMap["nymfile"]);

        bool bConverted = false;
        bool unused = false;
        const bool bLoaded =
            (strNymfile.Exists() && nym->LoadNymFromString(strNymfile, unused));
        //      const bool bLoaded    = (strNymfile.Exists() &&
        // nymfile->LoadFromString(strNymfile, &thePrivateMap)); // Unnecessary,
        // since nymfile has already loaded with this private info, and it will
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
            auto pNym = wallet_.Nym(nym->ID());
            if (false == bool(pNym)) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed while saving "
                         "the nym to the wallet.";
                return false;
            } else
                bConverted = true;
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed loading nymfile from string.\n";
            return false;
        }
        //
        if (bLoaded && bConverted)  // bLoaded is probably superfluous here.
        {
            OT_ASSERT(nullptr != nym);
            // save the nymfile.
            //
            if (!nym->SaveSignedNymfile(*nym)) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Error: Failed calling SaveSignedNymfile.\n";
                return false;
            }

            return true;  // <========= Success!
        }
    } else
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed loading or verifying keys|credentials|pseudonym.\n";
    return false;
}

bool OT_API::NumList_Add(NumList& theList, const NumList& theNewNumbers) const
{
    NumList tempNewList(theList);

    const bool bSuccess = tempNewList.Add(theNewNumbers);

    if (bSuccess) {
        theList.Release();
        theList.Add(tempNewList);
        return true;
    }
    return false;
}

bool OT_API::NumList_Remove(NumList& theList, const NumList& theOldNumbers)
    const
{
    NumList tempNewList(theList), tempOldList(theOldNumbers);

    while (tempOldList.Count() > 0) {
        std::int64_t lPeek = 0;

        if (!tempOldList.Peek(lPeek) || !tempOldList.Pop()) OT_FAIL;

        if (!tempNewList.Remove(lPeek)) return false;
    }

    theList.Release();
    theList.Add(tempNewList);
    return true;
}

// Verifies the presence of theQueryNumbers on theList (as a subset)
//
bool OT_API::NumList_VerifyQuery(
    const NumList& theList,
    const NumList& theQueryNumbers) const
{
    NumList theTempQuery(theQueryNumbers);

    while (theTempQuery.Count() > 0) {
        std::int64_t lPeek = 0;

        if (!theTempQuery.Peek(lPeek) || !theTempQuery.Pop()) OT_FAIL;

        if (!theList.Verify(lPeek)) return false;
    }

    return true;
}

// Verifies the COUNT and CONTENT (but not the order) matches EXACTLY.
//
bool OT_API::NumList_VerifyAll(
    const NumList& theList,
    const NumList& theQueryNumbers) const
{
    return theList.Verify(theQueryNumbers);
}

std::int32_t OT_API::NumList_Count(const NumList& theList) const
{
    return theList.Count();
}

/** TIME (in seconds, as std::int64_t)

 This will return the current time in seconds, as a std::int64_t std::int32_t.

 Todo:  consider making this available on the server side as well,
 so the smart contracts can see what time it is.
 */
time64_t OT_API::GetTime() const { return OTTimeGetCurrentTime(); }

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
bool OT_API::Encode(
    const String& strPlaintext,
    String& strOutput,
    bool bLineBreaks) const
{
    OTASCIIArmor ascArmor;
    bool bSuccess = ascArmor.SetString(strPlaintext, bLineBreaks);  // encodes.

    if (bSuccess) {
        strOutput.Release();
        bSuccess = ascArmor.WriteArmoredString(
            strOutput, "ENCODED TEXT"  // -----BEGIN OT ENCODED TEXT-----
        );                             // (bool bEscaped=false by default.)
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
bool OT_API::Decode(
    const String& strEncoded,
    String& strOutput,
    bool bLineBreaks) const
{
    OTASCIIArmor ascArmor;
    const bool bLoadedArmor = OTASCIIArmor::LoadFromString(
        ascArmor, strEncoded);  // str_bookend="-----BEGIN" by default
    if (!bLoadedArmor || !ascArmor.Exists()) {
        otErr << OT_METHOD << __FUNCTION__
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
bool OT_API::Encrypt(
    const Identifier& theRecipientNymID,
    const String& strPlaintext,
    String& strOutput) const
{
    auto pRecipientNym = wallet_.Nym(theRecipientNymID);
    if (false == bool(pRecipientNym)) return false;
    OTEnvelope theEnvelope;
    bool bSuccess = theEnvelope.Seal(*pRecipientNym, strPlaintext);

    if (bSuccess) {
        OTASCIIArmor ascCiphertext(theEnvelope);
        strOutput.Release();

        bSuccess = ascCiphertext.WriteArmoredString(
            strOutput, "ENCRYPTED TEXT"  // -----BEGIN OT ENCRYPTED TEXT-----
        );                               // (bool bEscaped=false by default.)
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
bool OT_API::Decrypt(
    const Identifier& theRecipientNymID,
    const String& strCiphertext,
    String& strOutput) const
{
    auto pRecipientNym = wallet_.Nym(theRecipientNymID);

    if (false == bool(pRecipientNym)) {
        return false;
    }

    OTEnvelope theEnvelope;
    OTASCIIArmor ascCiphertext;
    const bool bLoadedArmor = OTASCIIArmor::LoadFromString(
        ascCiphertext, strCiphertext);  // str_bookend="-----BEGIN" by default
    if (!bLoadedArmor || !ascCiphertext.Exists()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure loading string into OTASCIIArmor object:\n\n"
              << strCiphertext << "\n\n";
        return false;
    }
    if (theEnvelope.SetCiphertext(ascCiphertext)) {
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
bool OT_API::FlatSign(
    const Identifier& theSignerNymID,
    const String& strInput,
    const String& strContractType,
    String& strOutput) const
{
    auto nym = wallet_.Nym(theSignerNymID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    if (!strInput.Exists()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Empty contract passed in. (Returning false.)\n";
        return false;
    }
    if (!strContractType.Exists()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Empty contract type passed in. (Returning false.)\n";
        return false;
    }
    String strTemp(strInput);
    return Contract::SignFlatText(strTemp, strContractType, *nym, strOutput);
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
bool OT_API::SignContract(
    const Identifier& theSignerNymID,
    const String& strContract,
    String& strOutput) const
{
    auto nym = wallet_.Nym(theSignerNymID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    if (!strContract.Exists()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Empty contract passed in. (Returning false.)\n";
        return false;
    }
    //
    std::unique_ptr<Contract> contract(
        OTTransactionType::TransactionFactory(strContract));

    if (nullptr == contract)
        contract.reset(OTScriptable::InstantiateScriptable(strContract));

    if (nullptr == contract) contract.reset(::InstantiateContract(strContract));

    if (nullptr == contract) {
        otErr << OT_METHOD << __FUNCTION__
              << ": I tried my best. "
                 "Unable to instantiate contract passed in:\n\n"
              << strContract << "\n\n";
        return false;
    }

    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
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
bool OT_API::AddSignature(
    const Identifier& theSignerNymID,
    const String& strContract,
    String& strOutput) const
{
    auto nym = wallet_.Nym(theSignerNymID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    if (!strContract.Exists()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Empty contract passed in. (Returning false.)\n";
        return false;
    }
    //
    std::unique_ptr<Contract> contract(
        OTTransactionType::TransactionFactory(strContract));

    if (nullptr == contract)
        contract.reset(OTScriptable::InstantiateScriptable(strContract));

    if (nullptr == contract) contract.reset(::InstantiateContract(strContract));

    if (nullptr == contract) {
        otErr << OT_METHOD << __FUNCTION__
              << ": I tried my best. Unable to instantiate contract "
                 "passed in:\n\n"
              << strContract << "\n\n";
        return false;
    }

    //    contract->ReleaseSignatures();        // Other than this line, this
    // function is identical to
    // OT_API::SignContract(). (This one adds signatures without removing
    // existing ones.)
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

/** OT-Verify the signature on a CONTRACT.
 Returns true/false (success/fail)

 NOTICE that pcontract can be set, EVEN WHEN THIS RETURNS FALSE!
 Therefore, if you are low-level enough to actually be passing pcontract
 in here, then you had better check it, after the call, whether it returns
 success or fail! Either way, that pointer could be set, in which case YOU
 are responsible to clean it up! Check that pointer for nullptr!
 */
bool OT_API::VerifySignature(
    const String& strContract,
    const Identifier& theSignerNymID,
    Contract** pcontract) const  // If you use
                                 // this optional
                                 // parameter,
                                 // then YOU are
                                 // responsible
                                 // to clean it
                                 // up.
{
    OTPasswordData thePWData(OT_PW_DISPLAY);
    auto nym = wallet_.Nym(theSignerNymID);
    if (false == bool(nym)) return false;
    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    if (!strContract.Exists()) {
        otErr << "OT_API::VerifySignature: Empty contract passed in. "
                 "(Returning false.)\n";
        return false;
    }
    //
    Contract* contract = nullptr;
    std::unique_ptr<Contract> theAngel;

    if (nullptr == contract)
        contract = OTTransactionType::TransactionFactory(strContract);

    if (nullptr == contract)
        contract = OTScriptable::InstantiateScriptable(strContract);

    if (nullptr == contract) contract = ::InstantiateContract(strContract);

    if (nullptr == contract) {
        otErr << "OT_API::VerifySignature: I tried my best. Unable to "
                 "instantiate contract passed in:\n\n"
              << strContract << "\n\n";
        return false;
    }
    // Since we created it ourselves using a factory, we can do this:

    // The caller wants to take the contract for himself, without us cleaning
    // it up in here.
    //
    if (nullptr != pcontract) *pcontract = contract;
    // Else, we will clean it up ourselves...
    else
        theAngel.reset(contract);
    //    if (!contract->VerifyContractID())
    ////    if (!contract->VerifyContract())    // This calls
    /// VerifyContractID(), then PublicNym(), then VerifySignature()
    ///(with that Nym)
    //    {                                            // Therefore it's only
    // useful for server contracts and asset contracts. Here we can VerifyID and
    // Signature,
    //                                                // and that's good enough
    // for here and most other places, generically speaking.
    //        otErr << "OT_API::VerifySignature: Unable to verify
    // contract ID for contract passed in. NOTE: If you are experiencing "
    //                       "a problem here, CONTACT FELLOW TRAVELER and let
    // him know WHAT KIND OF CONTRACT, and what symptoms you are seeing, "
    //                       "versus what you were expecting to see. Contract
    // contents:\n\n" << strContract << "\n\n";
    //        return false;
    //    }

    if (!contract->VerifySignature(*nym)) {
        String strSignerNymID(theSignerNymID);
        otErr << "OT_API::VerifySignature: For Nym (" << strSignerNymID
              << "), unable to "
                 "verify signature on contract passed in:\n\n"
              << strContract << "\n\n";
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
bool OT_API::VerifyAndRetrieveXMLContents(
    const String& strContract,
    const Identifier& theSignerNymID,
    String& strOutput) const
{
    Contract* contract = nullptr;
    const bool bSuccess =
        VerifySignature(strContract, theSignerNymID, &contract);
    //
    // If contract is not nullptr after the above call, then this Cleanup
    // object
    // will clean it up after we leave the scope of this block.
    //
    std::unique_ptr<Contract> theAngel(contract);
    strOutput.Release();

    if (nullptr !=
        contract)  // contract will always exist, if we were successful.
        return (bSuccess && contract->SaveContractRaw(strOutput));

    return bSuccess;  // In practice this will only happen on failure. (Could
                      // have put "return false".)
}

/// === Verify Account Receipt ===
/// Returns bool. Verifies any asset account (intermediary files) against its
/// own last signed receipt.
/// Obviously this will fail for any new account that hasn't done any
/// transactions yet (and thus has no receipts.)
///
///
bool OT_API::VerifyAccountReceipt(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID) const
{
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return false;
    }

    return VerifyBalanceReceipt(*context, NOTARY_ID, ACCOUNT_ID);
}

bool OT_API::Create_SmartContract(
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    time64_t VALID_FROM,              // Default (0 or nullptr) == NOW
    time64_t VALID_TO,  // Default (0 or nullptr) == no expiry / cancel anytime
    bool SPECIFY_ASSETS,   // This means asset type IDs must be provided for
                           // every named account.
    bool SPECIFY_PARTIES,  // This means Nym IDs must be provided for every
                           // party.
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    OTSmartContract* contract = new OTSmartContract;
    OT_ASSERT_MSG(
        nullptr != contract,
        "OT_API::Create_SmartContract: ASSERT "
        "while trying to instantiate blank smart "
        "contract.\n");
    if (!contract->SetDateRange(VALID_FROM, VALID_TO)) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failed trying to set date "
                 "range.\n";
        return false;
    }

    contract->specifyParties(SPECIFY_PARTIES);
    contract->specifyAssetTypes(SPECIFY_ASSETS);

    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_SetDates(
    const String& THE_CONTRACT,       // The contract, about to have the dates
                                      // changed on it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    time64_t VALID_FROM,              // Default (0 or nullptr) == NOW
    time64_t VALID_TO,  // Default (0 or nullptr) == no expiry / cancel anytime.
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)

    std::unique_ptr<OTCronItem> contract(OTCronItem::NewCronItem(THE_CONTRACT));
    if (!contract) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    if (!contract->SetDateRange(VALID_FROM, VALID_TO)) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failed trying to set date "
                 "range.\n";
        return false;
    }
    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_AddParty(
    const String& THE_CONTRACT,  // The contract, about to have the party added
                                 // to it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& PARTY_NYM_ID,  // Optional. Some smart contracts require the
                                 // party's Nym to be specified in advance.
    const String& PARTY_NAME,    // The Party's NAME as referenced in the smart
                                 // contract. (And the scripts...)
    const String& AGENT_NAME,    // An AGENT will be added by default for this
                                 // party. Need Agent NAME.
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (!contract) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get()),
        str_agent_name(AGENT_NAME.Get());

    OTParty* party = contract->GetParty(str_party_name);

    if (nullptr != party) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: Party already exists. \n";
        return false;
    }

    // New param PARTY_NYM_ID must either be present, or empty, based on
    // arePartiesSpecified().
    //
    const char* szPartyNymID = nullptr;

    if (contract->arePartiesSpecified()) {
        if (!PARTY_NYM_ID.Exists()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failure: Party Nym ID is empty, even though this "
                     "contract is configured to require a Party's NymID to "
                     "appear on the contract. \n";
            return false;
        }

        szPartyNymID = PARTY_NYM_ID.Get();
    } else {
        if (PARTY_NYM_ID.Exists()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failure: Party Nym ID was provided but erroneously so, "
                     "since "
                     "this contract is NOT configured to require a Party's "
                     "NymID when first adding the party.\n";
            return false;
        }
    }

    party = new OTParty(
        str_party_name.c_str(),
        true /*bIsOwnerNym*/,
        szPartyNymID,
        str_agent_name.c_str(),
        true);  // bCreateAgent=false by default.
    OT_ASSERT(nullptr != party);

    if (!contract->AddParty(*party))  // takes ownership.
    {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed while trying to add party: " << PARTY_NAME << " \n";
        delete party;
        party = nullptr;
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_RemoveParty(
    const String& THE_CONTRACT,       // The contract, about to have the party
                                      // removed
                                      // from it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& PARTY_NAME,  // The Party's NAME as referenced in the smart
                               // contract. (And the scripts...)
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) return false;
    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (!contract) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    if (contract->RemoveParty(str_party_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

bool OT_API::SmartContract_AddAccount(
    const String& THE_CONTRACT,       // The contract, about to have the account
                                      // added to it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& PARTY_NAME,  // The Party's NAME as referenced in the smart
                               // contract. (And the scripts...)
    const String& ACCT_NAME,   // The Account's name as referenced in the smart
                               // contract
    const String& INSTRUMENT_DEFINITION_ID,  // Instrument Definition ID for the
                                             // Account.
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::SmartContract_AddAccount: Error loading "
                 "smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    OTParty* party = contract->GetParty(str_party_name);

    if (nullptr == party) {
        otErr << "OT_API::SmartContract_AddAccount: Failure: Party "
                 "doesn't exist. \n";
        return false;
    }
    const std::string str_name(ACCT_NAME.Get()),
        str_instrument_definition_id(INSTRUMENT_DEFINITION_ID.Get());

    if (nullptr != party->GetAccount(str_name)) {
        otErr << "OT_API::SmartContract_AddAccount: Failed adding: "
                 "account is already there with that name ("
              << str_name
              << ") on "
                 "party: "
              << str_party_name << " \n";
        return false;
    }
    // -----------------------------------
    // Need to explicitly check contract->areAssetTypesSpecified() and then
    // mandate that the instrument definition ID must either be present, or not,
    // based on that.

    const char* szAssetTypeID = nullptr;

    if (contract->areAssetTypesSpecified()) {
        if (str_instrument_definition_id.empty()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failure: Asset Type ID is empty, even though this "
                     "contract is configured to require the Asset Types to "
                     "appear on the contract. \n";
            return false;
        }

        szAssetTypeID = str_instrument_definition_id.c_str();
    } else {
        if (!str_instrument_definition_id.empty()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failure: Asset Type ID was provided but erroneously "
                     "so, since "
                     "this contract is NOT configured to require the Asset "
                     "Types when first adding the account.\n";
            return false;
        }
    }

    const String strAgentName, strAcctName(str_name.c_str()), strAcctID;
    String strInstrumentDefinitionID;

    if (nullptr != szAssetTypeID) strInstrumentDefinitionID.Set(szAssetTypeID);

    if (false == party->AddAccount(
                     strAgentName,
                     strAcctName,
                     strAcctID,
                     strInstrumentDefinitionID,
                     0)) {
        otErr << "OT_API::SmartContract_AddAccount: Failed trying to "
                 "add account ("
              << str_name << ") to party: " << str_party_name << " \n";
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_RemoveAccount(
    const String& THE_CONTRACT,       // The contract, about to have the account
                                      // removed from it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& PARTY_NAME,  // The Party's NAME as referenced in the smart
                               // contract. (And the scripts...)
    const String& ACCT_NAME,   // The Account's name as referenced in the smart
                               // contract
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::SmartContract_RemoveAccount: Error loading "
                 "smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    OTParty* party = contract->GetParty(str_party_name);

    if (nullptr == party) {
        otErr << "OT_API::SmartContract_RemoveAccount: Failure: Party "
                 "doesn't exist. \n";
        return false;
    }
    const std::string str_name(ACCT_NAME.Get());

    if (party->RemoveAccount(str_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

std::int32_t OT_API::SmartContract_CountNumsNeeded(
    const String& THE_CONTRACT,  // The contract, about to have the bylaw added
                                 // to it.
    const String& AGENT_NAME) const  // An AGENT will be added by default for
                                     // this
                                     // party. Need Agent NAME.
{
    std::int32_t nReturnValue = 0;
    const std::string str_agent_name(AGENT_NAME.Get());
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));

    if (nullptr == contract) {
        otErr << "OT_API::SmartContract_CountNumsNeeded: Error loading "
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

    nReturnValue = contract->GetCountTransNumsNeededForAgent(str_agent_name);
    return nReturnValue;
}

bool OT_API::SmartContract_ConfirmAccount(
    const String& THE_CONTRACT,
    const Identifier& SIGNER_NYM_ID,
    const String& PARTY_NAME,
    const String& ACCT_NAME,
    const String& AGENT_NAME,
    const String& ACCT_ID,
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    const auto accountID = Identifier::Factory(ACCT_ID);
    auto account = GetAccount(accountID, __FUNCTION__);

    if (false == bool(account)) return false;

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    std::unique_ptr<OTScriptable> pScriptable(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == pScriptable) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    OTSmartContract* contract =
        dynamic_cast<OTSmartContract*>(pScriptable.get());
    if (nullptr == contract) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure casting to Smart Contract. "
                 "Are you SURE it's a smart contract? Contents:\n"
              << THE_CONTRACT << "\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());
    OTParty* party = contract->GetParty(str_party_name);
    if (nullptr == party) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: Party doesn't exist: " << PARTY_NAME << " \n";
        return false;
    }
    // Make sure there's not already an account here with the same ID (Server
    // disallows.)
    //
    OTPartyAccount* pDupeAcct = party->GetAccountByID(accountID);
    if (nullptr != pDupeAcct)  // It's already there.
    {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed, since a duplicate account ID (" << ACCT_ID
              << ") was already found on this contract. (Server "
                 "disallows, sorry.)\n";
        return false;
    }
    // Find the account template based on its name, to affix the acct ID to.
    //
    const std::string str_name(ACCT_NAME.Get());

    OTPartyAccount* partyAcct = party->GetAccount(str_name);
    if (nullptr ==
        partyAcct)  // It's not already there. (Though it should be...)
    {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed: No account found on contract with name: "
              << str_name << " \n";
        return false;
    }

    // the actual instrument definition ID

    const auto theExpectedInstrumentDefinitionID = Identifier::Factory(
        partyAcct->GetInstrumentDefinitionID());  // The expected instrument
                                                  // definition ID,
                                                  // converting
                                                  // from a string.
    const Identifier& theActualInstrumentDefinitionID =
        account->GetInstrumentDefinitionID();  // the actual instrument
                                               // definition
    // ID, already an identifier, from
    // the actual account.

    if (contract->areAssetTypesSpecified() &&
        (theExpectedInstrumentDefinitionID !=
         theActualInstrumentDefinitionID)) {
        const String strInstrumentDefinitionID(theActualInstrumentDefinitionID);
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed, since the instrument definition ID of the account ("
              << strInstrumentDefinitionID
              << ") does not match what was expected ("
              << partyAcct->GetInstrumentDefinitionID()
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
    if (!account->VerifyOwner(*nym)) {
        const String strNymID(SIGNER_NYM_ID);
        otErr << OT_METHOD << __FUNCTION__ << ": Failed, since this nym ("
              << strNymID << ") isn't the owner of this account (" << str_name
              << ").\n";
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
    if (contract->SetNotaryIDIfEmpty(account->GetPurportedNotaryID())) {
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
        // verification.) In the std::int64_t term we'll do a more thorough
        // check
        // here, though.
    } else if (contract->GetNotaryID() != account->GetPurportedNotaryID()) {
        const String strServer1(contract->GetNotaryID()),
            strServer2(account->GetPurportedNotaryID());
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: The smart contract has a different "
                 "server ID on it already ("
              << strServer1
              << ") than the one "
                 "that goes with this account (server "
              << strServer2 << ", for account " << ACCT_ID << ")\n";
        return false;
    }
    // BY THIS POINT, we know that the account is actually owned by the Nym,
    // and we know that it's got the proper instrument definition ID that was
    // expected
    // according to the smart contract. We also know that the smart contract
    // has the same server ID as the account being confirmed.
    //
    partyAcct->SetAcctID(ACCT_ID.Get());
    partyAcct->SetAgentName(AGENT_NAME.Get());
    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::Smart_ArePartiesSpecified(const String& THE_CONTRACT) const
{
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));

    if (nullptr == contract) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }

    return contract->arePartiesSpecified();
}

bool OT_API::Smart_AreAssetTypesSpecified(const String& THE_CONTRACT) const
{
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));

    if (nullptr == contract) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }

    return contract->areAssetTypesSpecified();
}

bool OT_API::SmartContract_ConfirmParty(
    const String& THE_CONTRACT,  // The smart contract, about to be changed by
                                 // this function.
    const String& PARTY_NAME,    // Should already be on the contract. This way
                                 // we can find it.
    const Identifier& NYM_ID,    // Nym ID for the party, the actual owner,
    const Identifier& NOTARY_ID,
    String& strOutput) const  // ===> AS WELL AS for the default AGENT of that
                              // party.
                              // (For now, until I code entities)
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.mutable_ServerContext(NYM_ID, NOTARY_ID);
    auto nymfile = context.It().mutable_Nymfile(__FUNCTION__);
    auto nym = context.It().Nym();

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)

    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));

    if (nullptr == contract) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    OTParty* party = contract->GetParty(str_party_name);

    if (nullptr == party) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failure: Party ("
              << str_party_name
              << ") doesn't exist, so how can you confirm it?\n";
        return false;
    }

    if (contract->arePartiesSpecified()) {
        bool bSuccessID = false;
        const std::string partyNymID = party->GetNymID(&bSuccessID);

        if (bSuccessID && !partyNymID.empty()) {
            String strPartyNymID(partyNymID);
            auto idParty = Identifier::Factory(strPartyNymID);

            if (idParty != NYM_ID) {
                otErr << OT_METHOD << __FUNCTION__ << ": Failure: Party ("
                      << str_party_name
                      << ") has an expected NymID that doesn't match the "
                         "actual NymID.\n";
                return false;
            }
        }
    }

    OTParty* pNewParty = new OTParty(
        party->GetPartyName(),
        *nym,  // party keeps an internal pointer to nym from here on.
        party->GetAuthorizingAgentName());  // Party name and agent name must
                                            // match, in order to replace /
                                            // activate this party.
    OT_ASSERT(nullptr != pNewParty);
    if (!party->CopyAcctsToConfirmingParty(*pNewParty)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed while trying to copy accounts, while "
                 "confirming party: "
              << PARTY_NAME << " \n";
        delete pNewParty;
        pNewParty = nullptr;
        return false;
    }

    if (!contract->ConfirmParty(*pNewParty, context.It())) {
        otErr << OT_METHOD << __FUNCTION__
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
    //    contract->ReleaseSignatures();
    //    contract->SignContract(*nymfile);
    //    contract->SaveContract();

    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to confirm the party, we
    // have to track it until it's activated or until we cancel it.)
    //
    const String strInstrument(*contract);
    Message* pMessage = new Message;
    OT_ASSERT(nullptr != pMessage);

    const String strNymID(NYM_ID);

    pMessage->m_strCommand = "outpaymentsMessage";
    pMessage->m_strNymID = strNymID;
    //  pMessage->m_strNotaryID        = strNotaryID;
    pMessage->m_ascPayload.SetString(strInstrument);

    auto pNym = wallet_.Nym(nymfile.It().GetConstID());
    OT_ASSERT(nullptr != pNym)

    pMessage->SignContract(*pNym);
    pMessage->SaveContract();

    nymfile.It().AddOutpayments(
        *pMessage);  // Now the Nym is responsible to delete it.
                     // It's in his "outpayments".

    return true;
}

bool OT_API::SmartContract_AddBylaw(
    const String& THE_CONTRACT,  // The contract, about to have the bylaw added
                                 // to it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the smart
                               // contract. (And the scripts...)
    String& strOutput) const
{
    const char* BYLAW_LANGUAGE = "chai";  // todo hardcoding.
    auto nym = wallet_.Nym(SIGNER_NYM_ID);
    if (false == bool(nym)) return false;
    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::SmartContract_AddBylaw: Error loading smart "
                 "contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get()),
        str_language(BYLAW_LANGUAGE);

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr != pBylaw) {
        otErr << "OT_API::SmartContract_AddBylaw: Failure: Bylaw "
                 "already exists: "
              << BYLAW_NAME << " \n";
        return false;
    }
    pBylaw = new OTBylaw(str_bylaw_name.c_str(), str_language.c_str());
    OT_ASSERT(nullptr != pBylaw);

    if (!contract->AddBylaw(*pBylaw))  // takes ownership.
    {
        otErr << "OT_API::SmartContract_AddBylaw: Failed while trying "
                 "to add bylaw: "
              << BYLAW_NAME << " \n";
        delete pBylaw;
        pBylaw = nullptr;
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_RemoveBylaw(
    const String& THE_CONTRACT,       // The contract, about to have the bylaw
                                      // removed
                                      // from it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the smart
                               // contract. (And the scripts...)
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::SmartContract_RemoveBylaw: Error loading smart "
                 "contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    if (contract->RemoveBylaw(str_bylaw_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

bool OT_API::SmartContract_AddHook(
    const String& THE_CONTRACT,       // The contract, about to have the hook
                                      // added to it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& HOOK_NAME,    // The Hook's name as referenced in the smart
                                // contract. (And the scripts...)
    const String& CLAUSE_NAME,  // The actual clause that will be triggered by
                                // the hook. (You can call this multiple times,
                                // and have multiple clauses trigger on the
                                // same hook.)
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::SmartContract_AddHook: Error loading smart "
                 "contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otErr << "OT_API::SmartContract_AddHook: Failure: Bylaw "
                 "doesn't exist: "
              << str_bylaw_name << " \n";
        return false;
    }
    const std::string str_name(HOOK_NAME.Get()), str_clause(CLAUSE_NAME.Get());

    if (!pBylaw->AddHook(str_name, str_clause)) {
        otErr << "OT_API::SmartContract_AddHook: Failed trying to add "
                 "hook ("
              << str_name << ", clause " << str_clause
              << ") to bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_RemoveHook(
    const String& THE_CONTRACT,       // The contract, about to have the hook
                                      // removed from it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& HOOK_NAME,    // The Hook's name as referenced in the smart
                                // contract. (And the scripts...)
    const String& CLAUSE_NAME,  // The actual clause that will be triggered by
                                // the hook. (You can call this multiple times,
                                // and have multiple clauses trigger on the
                                // same hook.)
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::SmartContract_RemoveHook: Error loading smart "
                 "contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otErr << "OT_API::SmartContract_RemoveHook: Failure: Bylaw "
                 "doesn't exist: "
              << str_bylaw_name << " \n";
        return false;
    }

    const std::string str_name(HOOK_NAME.Get()), str_clause(CLAUSE_NAME.Get());

    if (pBylaw->RemoveHook(str_name, str_clause)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

bool OT_API::SmartContract_AddCallback(
    const String& THE_CONTRACT,  // The contract, about to have the callback
                                 // added to it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,  // Should already be on the contract. (This way
                               // we can find it.)
    const String& CALLBACK_NAME,  // The Callback's name as referenced in the
                                  // smart contract. (And the scripts...)
    const String& CLAUSE_NAME,    // The actual clause that will be triggered by
                                  // the callback. (Must exist.)
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::SmartContract_AddCallback: Error loading "
                 "smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otErr << "OT_API::SmartContract_AddCallback: Failure: Bylaw "
                 "doesn't exist: "
              << str_bylaw_name << " \n";
        return false;
    }
    const std::string str_name(CALLBACK_NAME.Get()),
        str_clause(CLAUSE_NAME.Get());

    if (nullptr != pBylaw->GetCallback(str_name)) {
        otErr << "OT_API::SmartContract_AddCallback: Failure: "
                 "Callback ("
              << str_name << ") already exists on bylaw: " << str_bylaw_name
              << " \n";
        return false;
    }
    if (!pBylaw->AddCallback(str_name.c_str(), str_clause.c_str())) {
        otErr << "OT_API::SmartContract_AddCallback: Failed trying to "
                 "add callback ("
              << str_name << ", clause " << str_clause
              << ") to bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_RemoveCallback(
    const String& THE_CONTRACT,  // The contract, about to have the callback
                                 // removed from it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,  // Should already be on the contract. (This way
                               // we can find it.)
    const String& CALLBACK_NAME,  // The Callback's name as referenced in the
                                  // smart contract. (And the scripts...)
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::SmartContract_RemoveCallback: Error loading "
                 "smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otErr << "OT_API::SmartContract_RemoveCallback: Failure: Bylaw "
                 "doesn't exist: "
              << str_bylaw_name << " \n";
        return false;
    }
    const std::string str_name(CALLBACK_NAME.Get());

    if (pBylaw->RemoveCallback(str_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

bool OT_API::SmartContract_AddClause(
    const String& THE_CONTRACT,       // The contract, about to have the clause
                                      // added to it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& CLAUSE_NAME,  // The Clause's name as referenced in the smart
                                // contract. (And the scripts...)
    const String& SOURCE_CODE,  // The actual source code for the clause.
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Error loading "
                 "smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failure: Bylaw "
                 "doesn't exist: "
              << str_bylaw_name << " \n Input contract: \n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_name(CLAUSE_NAME.Get()), str_code(SOURCE_CODE.Get());

    if (nullptr != pBylaw->GetClause(str_name)) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failed adding: "
                 "clause is already there with that name ("
              << str_name
              << ") on "
                 "bylaw: "
              << str_bylaw_name << " \n";
        return false;
    }
    if (!pBylaw->AddClause(str_name.c_str(), str_code.c_str())) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failed trying to "
                 "add clause ("
              << str_name << ") to bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_UpdateClause(
    const String& THE_CONTRACT,       // The contract, about to have the clause
                                      // updated on it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& CLAUSE_NAME,  // The Clause's name as referenced in the smart
                                // contract. (And the scripts...)
    const String& SOURCE_CODE,  // The actual source code for the clause.
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Error loading "
                 "smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failure: Bylaw "
                 "doesn't exist: "
              << str_bylaw_name << " \n Input contract: \n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_name(CLAUSE_NAME.Get()), str_code(SOURCE_CODE.Get());

    if (pBylaw->UpdateClause(str_name, str_code)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

bool OT_API::SmartContract_RemoveClause(
    const String& THE_CONTRACT,       // The contract, about to have the clause
                                      // removed from it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& CLAUSE_NAME,  // The Clause's name as referenced in the smart
                                // contract. (And the scripts...)
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Error loading "
                 "smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failure: Bylaw "
                 "doesn't exist: "
              << str_bylaw_name << " \n Input contract: \n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_name(CLAUSE_NAME.Get());

    if (pBylaw->RemoveClause(str_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

bool OT_API::SmartContract_AddVariable(
    const String& THE_CONTRACT,  // The contract, about to have the variable
                                 // added to it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,  // Should already be on the contract. (This way
                               // we can find it.)
    const String& VAR_NAME,    // The Variable's name as referenced in the smart
                               // contract. (And the scripts...)
    const String& VAR_ACCESS,  // "constant", "persistent", or "important".
    const String& VAR_TYPE,    // "string", "std::int64_t", or "bool"
    const String& VAR_VALUE,   // Contains a string. If type is :std::int64_t,
                               // atol()
    // will be used to convert value to a std::int64_t. If
    // type is bool, the strings "true" or "false"
    // are expected here in order to convert to a
    // bool.
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Error loading "
                 "smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failure: Bylaw "
                 "doesn't exist: "
              << str_bylaw_name << " \n";
        return false;
    }
    const std::string str_name(VAR_NAME.Get()), str_access(VAR_ACCESS.Get()),
        str_type(VAR_TYPE.Get()), str_value(VAR_VALUE.Get());

    if (nullptr != pBylaw->GetVariable(str_name)) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failure: "
                 "Variable ("
              << str_name << ") already exists on bylaw: " << str_bylaw_name
              << " \n";
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
        otErr << "OT_API::" << __FUNCTION__
              << ": Failed due to bad "
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
            const std::int32_t nValue = atoi(str_value.c_str());
            bAdded = pBylaw->AddVariable(str_name, nValue, theAccess);
        } break;
        case OTVariable::Var_String:
            bAdded = pBylaw->AddVariable(str_name, str_value, theAccess);
            break;
        default:
            // SHOULD NEVER HAPPEN (We already return above, if the variable
            // type
            // isn't correct.)
            OT_FAIL_MSG("Should never happen. You aren't seeing this.");
            break;
    }

    if (!bAdded) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failed trying to "
                 "add variable ("
              << str_name << ") to bylaw: " << str_bylaw_name << " \n";
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

bool OT_API::SmartContract_RemoveVariable(
    const String& THE_CONTRACT,  // The contract, about to have the variable
                                 // added to it.
    const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                      // signing at this point is only to cause
                                      // a save.)
    const String& BYLAW_NAME,  // Should already be on the contract. (This way
                               // we can find it.)
    const String& VAR_NAME,    // The Variable's name as referenced in the smart
                               // contract. (And the scripts...)
    String& strOutput) const
{
    auto nym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) {
        return false;
    }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<OTScriptable> contract(
        OTScriptable::InstantiateScriptable(THE_CONTRACT));
    if (nullptr == contract) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Error loading "
                 "smart contract:\n\n"
              << THE_CONTRACT << "\n\n";
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        otErr << "OT_API::" << __FUNCTION__
              << ": Failure: Bylaw "
                 "doesn't exist: "
              << str_bylaw_name << " \n";
        return false;
    }
    const std::string str_name(VAR_NAME.Get());

    if (pBylaw->RemoveVariable(str_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

// The Nym's Name is basically just a client-side label.
// This function lets you change it.
//
// Returns success, true or false.
bool OT_API::SetNym_Alias(
    const Identifier& targetNymID,
    const Identifier&,
    const String& name) const
{
    return wallet_.SetNymAlias(targetNymID, name.Get());
}

bool OT_API::Rename_Nym(
    const Identifier& nymID,
    const std::string& name,
    const proto::ContactItemType type,
    const bool primary) const
{
    if (name.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Empty name (bad)."
              << std::endl;

        return false;
    }

    auto nymdata = wallet_.mutable_Nym(nymID);

    proto::ContactItemType realType = proto::CITEMTYPE_ERROR;

    if (proto::CITEMTYPE_ERROR == type) {
        const auto existingType = nymdata.Claims().Type();

        if (proto::CITEMTYPE_ERROR == existingType) {
            return false;
        }

        realType = existingType;
    } else {
        realType = type;
    }

    const bool renamed = nymdata.SetScope(realType, name, primary);

    if (!renamed) {
        return false;
    }

    nymdata.SetAlias(name);

    return true;
}

// The Asset Account's Name is basically just a client-side label.
// This function lets you change it.
//
// Returns success, true or false.
bool OT_API::SetAccount_Name(
    const Identifier& accountID,
    const Identifier& SIGNER_NYM_ID,
    const String& ACCT_NEW_NAME) const
{
    Lock lock(lock_);
    OTWallet* pWallet =
        GetWallet(__FUNCTION__);  // This logs and ASSERTs already.

    if (nullptr == pWallet) {

        return false;
    }

    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    auto pSignerNym = wallet_.Nym(SIGNER_NYM_ID);

    if (false == bool(pSignerNym)) {
        return false;
    }

    auto account =
        GetAccount(accountID, __FUNCTION__);  // This logs and ASSERTs already.

    if (false == bool(account)) return false;

    if (!ACCT_NEW_NAME.Exists())  // Any other validation to do on the name?
    {
        otErr << OT_METHOD << __FUNCTION__
              << ": FYI, new name is empty. "
                 "(Proceeding anyway)\n";
    }
    account->SetName(ACCT_NEW_NAME);
    account->ReleaseSignatures();
    otInfo << "Saving updated account file to disk...\n";
    if (account->SignContract(*pSignerNym) && account->SaveContract() &&
        account->SaveAccount())
        return pWallet->SaveWallet();  // Only because the account's name is
                                       // stored here, too.
    else
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed doing this:  "
                 "if (account->SignContract(*pSignerNym) "
                 "&& account->SaveContract() && account->SaveAccount())\n";
    return false;
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
    const Message& theMsg,
    const Identifier& NYM_ID,
    bool bHarvestingForRetry,           // false until positively asserted.
    bool bReplyWasSuccess,              // false until positively asserted.
    bool bReplyWasFailure,              // false until positively asserted.
    bool bTransactionWasSuccess,        // false until positively asserted.
    bool bTransactionWasFailure) const  // false until positively asserted.
{
    rLock lock(lock_callback_({NYM_ID.str(), theMsg.m_strNotaryID.Get()}));
    auto context = wallet_.mutable_ServerContext(
        NYM_ID, Identifier::Factory(theMsg.m_strNotaryID));

    return theMsg.HarvestTransactionNumbers(
        context.It(),
        bHarvestingForRetry,
        bReplyWasSuccess,
        bReplyWasFailure,
        bTransactionWasSuccess,
        bTransactionWasFailure);
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

bool OT_API::HarvestClosingNumbers(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const String& THE_CRON_ITEM) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.mutable_ServerContext(NYM_ID, NOTARY_ID);
    std::unique_ptr<OTCronItem> pCronItem(
        OTCronItem::NewCronItem(THE_CRON_ITEM));

    if (nullptr == pCronItem) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading the cron item (a cron item is a "
                 "smart contract, or some other recurring transaction such "
                 "as a market offer, or a payment plan.) Contents:\n\n"
              << THE_CRON_ITEM << "\n\n";

        return false;
    }

    // the Nym is actually harvesting the numbers from the Cron Item, and not
    // the other way around.
    pCronItem->HarvestClosingNumbers(context.It());

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
bool OT_API::HarvestAllNumbers(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const String& THE_CRON_ITEM) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.mutable_ServerContext(NYM_ID, NOTARY_ID);
    std::unique_ptr<OTCronItem> pCronItem(
        OTCronItem::NewCronItem(THE_CRON_ITEM));

    if (nullptr == pCronItem) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error loading the cron item (a cron item is a "
                 "smart contract, or some other recurring transaction such "
                 "as a market offer, or a payment plan.) "
                 "Contents:\n\n"
              << THE_CRON_ITEM << "\n\n";
        return false;
    }

    pCronItem->HarvestOpeningNumber(context.It());  // <==== the Nym is actually
    // harvesting the numbers from the
    // Cron Item, and not the other way
    // around.
    pCronItem->HarvestClosingNumbers(
        context.It());  // <==== the Nym is actually
                        // harvesting the numbers from the
    // Cron Item, and not the other way
    // around.
    return true;
}

// static
std::string OT_API::NymIDFromPaymentCode(__attribute__((unused))
                                         const std::string& paymentCode)
{
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    auto code = PaymentCode::Factory(paymentCode);

    if (code->VerifyInternally()) {
        return code->ID()->str();
    } else {
        return "";
    }
#else
    return "";
#endif
}

bool OT_API::AddClaim(
    NymData& toNym,
    const proto::ContactSectionName& section,
    const proto::ContactItemType& type,
    const std::string& value,
    const bool primary,
    const bool active,
    const std::uint64_t start,
    const std::uint64_t end,
    const std::uint32_t) const
{
    std::set<std::uint32_t> attribute;

    if (active || primary) {
        attribute.insert(proto::CITEMATTR_ACTIVE);
    }

    if (primary) {
        attribute.insert(proto::CITEMATTR_PRIMARY);
    }

    const Claim claim{"", section, type, value, start, end, attribute};
    toNym.AddClaim(claim);

    return true;
}

/** Tries to get the account from the wallet.
 Otherwise loads it from local storage.
 */
std::shared_ptr<Account> OT_API::GetOrLoadAccount(
    const Nym& theNym,
    const Identifier& accountID,
    const Identifier& NOTARY_ID,
    const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    OTWallet* pWallet = GetWallet(szFunc);  // This logs and ASSERTs already.

    if (nullptr == pWallet) {

        return {};
    }

    return pWallet->GetOrLoadAccount(
        theNym,
        accountID,
        NOTARY_ID,
        szFunc);  // This logs plenty.
}

/** Tries to get the account from the wallet.
 Otherwise loads it from local storage.
 */
std::shared_ptr<Account> OT_API::GetOrLoadAccount(
    const Identifier& NYM_ID,
    const Identifier& accountID,
    const Identifier& NOTARY_ID,
    const char* szFuncName) const
{
    const char* szFunc = (nullptr != szFuncName) ? szFuncName : __FUNCTION__;
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return {};
    }

    auto nym = context->Nym();

    return GetOrLoadAccount(
        *nym,
        accountID,
        NOTARY_ID,
        szFunc);  // This logs plenty.
}

// WRITE CHEQUE
//
// Returns an OTCheque pointer, or nullptr.
// (Caller responsible to delete.)
Cheque* OT_API::WriteCheque(
    const Identifier& NOTARY_ID,
    const std::int64_t& CHEQUE_AMOUNT,
    const time64_t& VALID_FROM,
    const time64_t& VALID_TO,
    const Identifier& SENDER_accountID,
    const Identifier& SENDER_NYM_ID,
    const String& CHEQUE_MEMO,
    const Identifier& pRECIPIENT_NYM_ID) const
{
    rLock lock(lock_callback_({SENDER_NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.mutable_ServerContext(SENDER_NYM_ID, NOTARY_ID);
    auto nym = context.It().Nym();
    auto account =
        GetOrLoadAccount(*nym, SENDER_accountID, NOTARY_ID, __FUNCTION__);

    if (false == bool(account)) {

        return nullptr;
    }

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)

    // To write a cheque, we need to burn one of our transaction numbers.
    // (Presumably the wallet is also storing a couple of these, since they are
    // needed to perform any transaction.)
    //
    // I don't have to contact the server to write a cheque -- as long as I
    // already have a transaction number I can use to write it with. (Otherwise
    // I'd have to ask the server to send me one first.)
    String strNotaryID(NOTARY_ID);
    const auto number =
        context.It().NextTransactionNumber(MessageType::notarizeTransaction);

    if (false == number.Valid()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": User attempted to write a cheque, but had no "
                 "transaction numbers.\n";

        return nullptr;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << number << std::endl;

    // At this point, I know that number contains one I can use.
    Cheque* pCheque = new Cheque(
        account->GetRealNotaryID(), account->GetInstrumentDefinitionID());
    OT_ASSERT_MSG(
        nullptr != pCheque,
        "OT_API::WriteCheque: Error allocating memory in the OT API.");
    // At this point, I know that pCheque is a good pointer that I either
    // have to delete, or return to the caller.
    bool bIssueCheque = pCheque->IssueCheque(
        CHEQUE_AMOUNT,
        number,
        VALID_FROM,
        VALID_TO,
        SENDER_accountID,
        SENDER_NYM_ID,
        CHEQUE_MEMO,
        pRECIPIENT_NYM_ID);

    if (!bIssueCheque) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure calling OTCheque::IssueCheque().\n";
        delete pCheque;
        pCheque = nullptr;

        return nullptr;
    }

    pCheque->SignContract(*nym);
    pCheque->SaveContract();
    auto workflow = workflow_.WriteCheque(*pCheque);

    if (workflow->empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to create workflow."
              << std::endl;

        return nullptr;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Started workflow "
              << workflow->str() << std::endl;
    }

    // Above this line, the transaction number will be recovered automatically
    number.SetSuccess(true);

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
OTPaymentPlan* OT_API::ProposePaymentPlan(
    const Identifier& NOTARY_ID,
    const time64_t& VALID_FROM,  // Default (0) == NOW (It will set it to the
                                 // current time in seconds since Jan 1970)
    const time64_t& VALID_TO,    // Default (0) == no expiry / cancel anytime.
                                 // Otherwise this is a LENGTH and is ADDED to
                                 // VALID_FROM
    const Identifier* pSENDER_accountID,
    const Identifier& SENDER_NYM_ID,
    const String& PLAN_CONSIDERATION,  // Like a memo.
    const Identifier& RECIPIENT_accountID,
    const Identifier& RECIPIENT_NYM_ID,
    // ----------------------------------------  // If it's above zero, the
    // initial
    const std::int64_t& INITIAL_PAYMENT_AMOUNT,  // amount will be processed
                                                 // after
    const time64_t& INITIAL_PAYMENT_DELAY,       // delay (seconds from now.)
    // ----------------------------------------  // AND SEPARATELY FROM THIS...
    const std::int64_t& PAYMENT_PLAN_AMOUNT,  // The regular amount charged,
    const time64_t& PAYMENT_PLAN_DELAY,   // which begins occuring after delay
    const time64_t& PAYMENT_PLAN_PERIOD,  // (seconds from now) and happens
    // ----------------------------------------// every period, ad infinitum,
    time64_t PAYMENT_PLAN_LENGTH,  // until after the length (in seconds)
    std::int32_t PAYMENT_PLAN_MAX_PAYMENTS  // expires, or after the maximum
    ) const                                 // number of payments. These last
{                                           // two arguments are optional.
    auto context = wallet_.mutable_ServerContext(RECIPIENT_NYM_ID, NOTARY_ID);
    auto nymfile = context.It().mutable_Nymfile(__FUNCTION__);

    auto nym = context.It().Nym();

    auto account =
        GetOrLoadAccount(*nym, RECIPIENT_accountID, NOTARY_ID, __FUNCTION__);

    if (false == bool(account)) {

        return nullptr;
    }

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)

    OTPaymentPlan* pPlan = nullptr;

    // We don't always know the sender's account ID at the time of the proposal.
    // (The sender is the payer aka customer, who selects his account at the
    // time of
    // confirmation, which is after the merchant has created the proposal (here)
    // and
    // sent it to him.)
    if (nullptr == pSENDER_accountID) {
        pPlan =
            new OTPaymentPlan(NOTARY_ID, account->GetInstrumentDefinitionID());

        OT_ASSERT_MSG(
            nullptr != pPlan,
            "OT_API::ProposePaymentPlan: 1 Error allocating "
            "memory in the OT API for new "
            "OTPaymentPlan.\n");

        pPlan->setCustomerNymId(SENDER_NYM_ID);
        pPlan->SetRecipientNymID(RECIPIENT_NYM_ID);
        pPlan->SetRecipientAcctID(RECIPIENT_accountID);
    } else {
        pPlan = new OTPaymentPlan(
            NOTARY_ID,
            account->GetInstrumentDefinitionID(),
            *pSENDER_accountID,
            SENDER_NYM_ID,
            RECIPIENT_accountID,
            RECIPIENT_NYM_ID);

        OT_ASSERT_MSG(
            nullptr != pPlan,
            "OT_API::ProposePaymentPlan: 2 Error allocating "
            "memory in the OT API for new "
            "OTPaymentPlan.\n");
    }
    // At this point, I know that pPlan is a good pointer that I either
    // have to delete, or return to the caller. CLEANUP WARNING!
    bool bSuccessSetProposal = pPlan->SetProposal(
        context.It(), *account, PLAN_CONSIDERATION, VALID_FROM, VALID_TO);
    // WARNING!!!! SetProposal() burns TWO transaction numbers for RECIPIENT.
    // (*nymfile)
    // BELOW THIS POINT, if you have an error, then you must retrieve those
    // numbers from
    // the plan, and set them BACK on nymfile before you return!!!
    const String strNotaryID(NOTARY_ID);

    if (!bSuccessSetProposal) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to set the proposal.\n";
        pPlan->HarvestOpeningNumber(context.It());
        pPlan->HarvestClosingNumbers(context.It());
        delete pPlan;
        pPlan = nullptr;
        return nullptr;
    }
    // the default, in case user chooses not to even have this payment.
    bool bSuccessSetInitialPayment = true;
    // the default, in case user chooses not to have a payment plan.
    bool bSuccessSetPaymentPlan = true;
    if ((INITIAL_PAYMENT_AMOUNT > 0) &&
        (INITIAL_PAYMENT_DELAY >= OT_TIME_ZERO)) {
        // The Initial payment delay is measured in seconds, starting from the
        // "Creation Date".
        bSuccessSetInitialPayment = pPlan->SetInitialPayment(
            INITIAL_PAYMENT_AMOUNT, INITIAL_PAYMENT_DELAY);
    }
    if (!bSuccessSetInitialPayment) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to set the initial payment.\n";
        pPlan->HarvestOpeningNumber(context.It());
        pPlan->HarvestClosingNumbers(context.It());
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
    if (PAYMENT_PLAN_AMOUNT > 0)  // If there are regular payments.
    {
        // The payment plan delay is measured in seconds, starting from the
        // "Creation Date".
        time64_t PAYMENT_DELAY =
            OT_TIME_MONTH_IN_SECONDS;  // Defaults to 30 days, measured in
                                       // seconds (if you pass 0.)

        if (PAYMENT_PLAN_DELAY > OT_TIME_ZERO)
            PAYMENT_DELAY = PAYMENT_PLAN_DELAY;
        // Defaults to 30 days, measured in seconds (if you pass 0.)
        time64_t PAYMENT_PERIOD = OT_TIME_MONTH_IN_SECONDS;

        if (PAYMENT_PLAN_PERIOD > OT_TIME_ZERO)
            PAYMENT_PERIOD = PAYMENT_PLAN_PERIOD;
        // Defaults to 0 seconds (for no max length).
        time64_t PLAN_LENGTH = OT_TIME_ZERO;

        if (PAYMENT_PLAN_LENGTH > OT_TIME_ZERO)
            PLAN_LENGTH = PAYMENT_PLAN_LENGTH;
        std::int32_t nMaxPayments =
            0;  // Defaults to 0 maximum payments (for no maximum).

        if (PAYMENT_PLAN_MAX_PAYMENTS > 0)
            nMaxPayments = PAYMENT_PLAN_MAX_PAYMENTS;
        bSuccessSetPaymentPlan = pPlan->SetPaymentPlan(
            PAYMENT_PLAN_AMOUNT,
            PAYMENT_DELAY,
            PAYMENT_PERIOD,
            PLAN_LENGTH,
            nMaxPayments);
    }
    if (!bSuccessSetPaymentPlan) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to set the payment plan.\n";
        pPlan->HarvestOpeningNumber(context.It());
        pPlan->HarvestClosingNumbers(context.It());
        delete pPlan;
        pPlan = nullptr;
        return nullptr;
    }
    pPlan->SignContract(*nym);  // Here we have saved the MERCHANT's VERSION.
    pPlan->SaveContract();  // A copy of this will be attached to the CUSTOMER's
                            // version as well.
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to propose the plan, we
    // have to track it until it's deposited or until we cancel it.)
    //
    const String strInstrument(*pPlan);
    Message* pMessage = new Message;
    OT_ASSERT(nullptr != pMessage);

    const String strNymID(RECIPIENT_NYM_ID), strNymID2(SENDER_NYM_ID);

    pMessage->m_strCommand = "outpaymentsMessage";
    pMessage->m_strNymID = strNymID;
    pMessage->m_strNymID2 = strNymID2;
    pMessage->m_strNotaryID = strNotaryID;
    pMessage->m_ascPayload.SetString(strInstrument);

    pMessage->SignContract(*nym);
    pMessage->SaveContract();

    nymfile.It().AddOutpayments(
        *pMessage);  // Now the Nym is responsible to delete it.
                     // It's in his "outpayments".
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
bool OT_API::ConfirmPaymentPlan(
    const Identifier& NOTARY_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& SENDER_accountID,
    const Identifier& RECIPIENT_NYM_ID,
    OTPaymentPlan& thePlan) const
{
    rLock lock(lock_callback_({SENDER_NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.mutable_ServerContext(SENDER_NYM_ID, NOTARY_ID);
    auto nymfile = context.It().mutable_Nymfile(__FUNCTION__);

    auto nym = context.It().Nym();

    auto account =
        GetOrLoadAccount(*nym, SENDER_accountID, NOTARY_ID, __FUNCTION__);

    if (false == bool(account)) {

        return false;
    }

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    auto pMerchantNym = wallet_.Nym(RECIPIENT_NYM_ID);

    if (!pMerchantNym)  // We don't have this Nym in our storage already.
    {
        const String strRecinymfileID(RECIPIENT_NYM_ID);
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: First you need to download the missing "
                 "(Merchant) Nym's credentials: "
              << strRecinymfileID << "\n";
        return false;
    }
    // pMerchantNym is also good, and has an angel. (No need to cleanup.)
    // --------------------------------------------------------
    // The "Creation Date" of the agreement is re-set here.
    //
    bool bConfirmed = thePlan.Confirm(
        context.It(), *account, pMerchantNym.get(), &RECIPIENT_NYM_ID);
    //
    // WARNING:  The call to "Confirm()" uses TWO transaction numbers from
    // nymfile!
    // If you don't end up actually USING this payment plan, then you need to
    // retrieve
    // those numbers and Add them back to nymfile again.
    // A nice client will store a copy of any payment plans until they are
    // closed out, or canceled,
    // or whatever.
    // ANY FAILURES BELOW THIS POINT need to be smart enough to retrieve those
    // numbers before returning.
    //
    const String strNotaryID(NOTARY_ID);

    if (!bConfirmed) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to confirm the agreement.\n";
        thePlan.HarvestOpeningNumber(context.It());
        thePlan.HarvestClosingNumbers(context.It());
        return false;
    }
    thePlan.SignContract(*nym);  // Here we have saved the CUSTOMER's version,
    thePlan.SaveContract();  // which contains a copy of the merchant's version.
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to confirm the plan, we
    // have to track it until it's deposited or until we cancel it.)
    //
    const String strInstrument(thePlan);
    Message* pMessage = new Message;
    OT_ASSERT(nullptr != pMessage);

    const String strNymID(SENDER_NYM_ID), strNymID2(RECIPIENT_NYM_ID);

    pMessage->m_strCommand = "outpaymentsMessage";
    pMessage->m_strNymID = strNymID;
    pMessage->m_strNymID2 = strNymID2;
    pMessage->m_strNotaryID = strNotaryID;
    pMessage->m_ascPayload.SetString(strInstrument);

    pMessage->SignContract(*nym);
    pMessage->SaveContract();

    nymfile.It().AddOutpayments(
        *pMessage);  // Now the Nym is responsible to delete it.
                     // It's in his "outpayments".
    return true;
}

#if OT_CASH
// LOAD PURSE
//
// Returns an Purse pointer, or nullptr.
//
// (Caller responsible to delete.)
Purse* OT_API::LoadPurse(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& NYM_ID,
    const String* pstrDisplay) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    const String strReason(
        (nullptr == pstrDisplay) ? "Loading purse from local storage."
                                 : pstrDisplay->Get());
    OTPasswordData thePWData(strReason);
    const String strNotaryID(NOTARY_ID);
    const String strNymID(NYM_ID);
    const String strInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return nullptr;
    }

    auto nym = context->Nym();
    Purse* pPurse = new Purse(NOTARY_ID, INSTRUMENT_DEFINITION_ID, NYM_ID);
    OT_ASSERT_MSG(
        nullptr != pPurse,
        "Error allocating memory in the OT API.");  // responsible to
                                                    // delete or return
                                                    // pPurse below
                                                    // this point.

    if (pPurse->LoadPurse(
            strNotaryID.Get(),
            strNymID.Get(),
            strInstrumentDefinitionID.Get())) {
        if (pPurse->VerifySignature(*nym) &&
            (NOTARY_ID == pPurse->GetNotaryID()) &&
            (INSTRUMENT_DEFINITION_ID == pPurse->GetInstrumentDefinitionID())) {
            return pPurse;
        } else
            otErr << OT_METHOD << __FUNCTION__ << ": Failed verifying purse.\n";
    } else
        otInfo << __FUNCTION__ << ": Failed loading purse.\n";
    delete pPurse;
    pPurse = nullptr;

    return nullptr;
}

bool OT_API::SavePurse(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& NYM_ID,
    Purse& THE_PURSE) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    if (THE_PURSE.IsPasswordProtected()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: This purse is password-protected (exported) "
                 "and cannot be saved inside the wallet without first "
                 "re-importing it.\n";
    } else if (
        (THE_PURSE.GetNotaryID() != NOTARY_ID) ||
        (THE_PURSE.GetInstrumentDefinitionID() != INSTRUMENT_DEFINITION_ID)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error: Wrong server or instrument definition id passed in, "
                 "considering the purse that was passed.\n";
    } else {
        const String strNotaryID(NOTARY_ID);
        const String strInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
        const String strNymID(NYM_ID);
        if (THE_PURSE.SavePurse(
                strNotaryID.Get(),
                strNymID.Get(),
                strInstrumentDefinitionID.Get()))
            return true;
    }
    const String strPurse(THE_PURSE);
    otErr << OT_METHOD << __FUNCTION__ << ": Failed saving purse:\n\n"
          << strPurse << "\n\n";
    return false;
}

// Creates a purse owned by a specific Nym. (OWNER_ID.)
//
// Caller is responsible to delete!
//
Purse* OT_API::CreatePurse(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& OWNER_ID) const
{
    rLock lock(lock_callback_({OWNER_ID.str(), NOTARY_ID.str()}));
    Purse* pPurse = new Purse(NOTARY_ID, INSTRUMENT_DEFINITION_ID, OWNER_ID);
    OT_ASSERT_MSG(
        nullptr != pPurse,
        "Error allocating memory in the OT API.");  // responsible to
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
Purse* OT_API::CreatePurse_Passphrase(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    Purse* pPurse = new Purse(NOTARY_ID, INSTRUMENT_DEFINITION_ID);
    OT_ASSERT_MSG(
        nullptr != pPurse,
        "Error allocating memory in the OT API.");  // responsible to
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
    const Identifier& theNotaryID,
    const Identifier& theInstrumentDefinitionID,
    const String& strPurse,
    Purse& thePurse,          // output
    OTPassword& thePassword,  // Only used in the case of password-protected
                              // purses. Passed in so it won't go out of scope
                              // when pOwner is set to point to it.
    bool bForEncrypting,      // true==encrypting,false==decrypting.
                              // Only relevant if there's an owner.
    const Identifier* pOWNER_ID,       // This can be nullptr, **IF** purse
                                       // is password-protected. (It's
                                       // just ignored in that case.)
                                       // Otherwise MUST contain the NymID
                                       // for the Purse owner.
    const String* pstrDisplay1,        // for purses owned by the wallet
                                       // already
    const String* pstrDisplay2) const  // for password-protected purses
{
    const bool bDoesOwnerIDExist =
        (nullptr !=
         pOWNER_ID);  // If not true, purse MUST be password-protected.
    OTPasswordData thePWData1(
        (nullptr == pstrDisplay1)
            ? "Enter the master passphrase for your wallet. "
              "(LoadPurseAndOwnerFromString)"
            : pstrDisplay1->Get());  // for purses already owned by the wallet
    OTPasswordData thePWData2(
        (nullptr == pstrDisplay2)
            ? "Enter the passphrase for this purse. "
              "(LoadPurseAndOwnerFromString)"
            : pstrDisplay2->Get());  // for password-protected purses.
    const Nym* pOwnerNym =
        nullptr;  // In the case where there is an owner, this will point to it.
    if (bDoesOwnerIDExist) {
        pOwnerNym = wallet_.Nym(*pOWNER_ID).get();

        if (nullptr == pOwnerNym) {
            return nullptr;
        }
    }
    // By this point, pOwnerNym may be a good pointer, and on the wallet. (No
    // need to cleanup.)
    // Or, it may also be nullptr, in the case that the purse is
    // password-protected.
    // bDoesOwnerIDExist is an easy way to tell, either way.
    OTNym_or_SymmetricKey* pOwner = nullptr;
    if (strPurse.Exists() && thePurse.LoadContractFromString(strPurse)) {
        const bool bNymIDIncludedInPurse = thePurse.IsNymIDIncluded();
        auto idPurseNym = Identifier::Factory();

        if (thePurse.GetNotaryID() != theNotaryID)
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed: NotaryID doesn't match.\n";
        else if (
            thePurse.GetInstrumentDefinitionID() != theInstrumentDefinitionID)
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed: InstrumentDefinitionID doesn't match.\n";
        else if (bNymIDIncludedInPurse && !thePurse.GetNymID(idPurseNym))
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed trying to get the NymID from the "
                     "purse (though one WAS apparently present.)\n";
        else if (bNymIDIncludedInPurse && !bDoesOwnerIDExist) {
            const String strPurseNymID(idPurseNym);
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error: The purse is owned by a NymID, but no "
                     "NymID was passed into "
                     "this function.\nNym who owns the purse: "
                  << strPurseNymID << "\n\n";
        } else if (
            bNymIDIncludedInPurse && !(pOwnerNym->GetConstID() == idPurseNym)) {
            const String strPurseNymID(idPurseNym),
                strNymActuallyPassed(pOwnerNym->GetConstID());
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error: the API call mentions Nym A, but the "
                     "purse is actually owned by Nym B.\nNym A: "
                  << strNymActuallyPassed << "\nNym B: " << strPurseNymID
                  << "\n\n";
        } else if (!bDoesOwnerIDExist && !thePurse.IsPasswordProtected())
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed: The NYM_ID was nullptr, which is only allowed "
                     "for a password-protected purse. (And this purse is NOT "
                     "password-protected.) Please provide a NYM_ID.\n";
        // By this point, we know the purse loaded up properly, and the server
        // and instrument definition ids match
        // what we expected. We also know that if the purse included a NymID, it
        // matches the NYM_ID
        // that was passed in. We also know that if a Nym ID was NOT passed in,
        // that the purse WAS
        // definitely a password-protected purse.
        //
        else if (thePurse.IsPasswordProtected())  // Purse is encrypted based on
                                                  // its own built-in symmetric
                                                  // key.
        {
            OTSymmetricKey* pSymmetricKey = thePurse.GetInternalKey();
            OT_ASSERT(nullptr != pSymmetricKey);
            const bool bGotPassphrase = thePurse.GetPassphrase(
                thePassword, thePWData2.GetDisplayString());

            if (!bGotPassphrase)
                otErr << OT_METHOD << __FUNCTION__
                      << ": Authentication failed, or otherwise failed "
                         "retrieving secret from user.\n";
            // Below this point, we know thePassword is good, and we know
            // pSymmetricKey is good.
            // Therefore...
            //
            else {
                pOwner = new OTNym_or_SymmetricKey(
                    *pSymmetricKey,
                    thePassword,
                    pstrDisplay2);  // Can't put
                                    // &strReason
                                    // here. (It
                                    // goes out of
                                    // scope.)
                OT_ASSERT(nullptr != pOwner);
            }
        } else if (bDoesOwnerIDExist)  // Purse is encrypted based on Nym.
        {
            pOwner = new OTNym_or_SymmetricKey(
                *pOwnerNym, pstrDisplay1);  // Can't put &strReason here. (It
                                            // goes out of scope.)
            OT_ASSERT(nullptr != pOwner);
        } else
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed: The purse is not password-protected, "
                     "but rather, is locked by a Nym. "
                     "However, no NYM_ID was passed in! Please supply a "
                     "NYM_ID in order "
                     "to open this purse.\n";
        // (By this point, pOwner is all set up and ready to go.)
    } else
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure loading purse from string:\n"
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
    const String& strPurse,
    Purse& thePurse,          // output
    OTPassword& thePassword,  // Only used in the case of password-protected
                              // purses. Passed in so it won't go out of scope
                              // when pOwner is set to point to it.
    bool bCanBePublic,        // true==private nym isn't mandatory.
                              // false==private nym IS mandatory.
                              // (Only relevant if there's an owner.)
    const Identifier* pOWNER_ID,  // This can be nullptr, **IF** purse
                                  // is password-protected. (It's
                                  // just ignored in that case.)
                                  // Otherwise if it's Nym-protected,
                                  // the purse will have a NymID on
                                  // it already. If not (it's
                                  // optional), then pOWNER_ID is the
                                  // ID it will try next, before
                                  // failing.
    const String* pstrDisplay) const
{
    OTPasswordData thePWData(
        (nullptr == pstrDisplay) ? OT_PW_DISPLAY : pstrDisplay->Get());
    OTNym_or_SymmetricKey* pOwner = nullptr;

    if (strPurse.Exists() && thePurse.LoadContractFromString(strPurse)) {
        auto idPurseNym = Identifier::Factory();

        if (thePurse.IsNymIDIncluded() && !thePurse.GetNymID(idPurseNym))
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed trying to get the NymID from the "
                     "purse (though one WAS apparently present.)\n";
        // Purse is encrypted based on Nym.
        //
        // If the purse includes a NymID, then we'll use it. (If we can't use
        // that specific one, then we have failed.)
        else if (
            thePurse.IsNymIDIncluded() ||
            // Else if the purse does NOT include a NymID (but also is NOT
            // password protected, meaning a Nym still exists, but just
            // isn't named) then we'll use pOWNER_ID passed in. If that's
            // nullptr, or we fail to find the Nym with it, then we have
            // failed.
            (!thePurse.IsNymIDIncluded() &&
             !thePurse.IsPasswordProtected())  // && (nullptr != pOWNER_ID))
                                               //
                                               // checked inside the block.
        ) {
            const Identifier& pActualOwnerID =
                thePurse.IsNymIDIncluded() ? idPurseNym.get() : *pOWNER_ID;

            if (pActualOwnerID.empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed: The purse is encrypted to a "
                         "specific Nym (not a passphrase) "
                         "but that Nym is not specified in the purse, nor "
                         "was it passed into this "
                         "function. (Failure. Unable to access purse.)\n";
                return nullptr;
            }
            auto pOwnerNym = wallet_.Nym(pActualOwnerID);
            if (false == bool(pOwnerNym)) {
                const String strAttemptedID(pActualOwnerID);
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed: The purse is encrypted to a specific NymID "
                         "(not a passphrase) which was either specified inside "
                         "the purse, "
                         "or wasn't specified so we guessed the user ID. "
                         "Either way, we "
                         "then failed loading that Nym: "
                      << strAttemptedID
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
                *pOwnerNym,
                pstrDisplay);  // Can't put &strReason here. (It goes
                               // out of scope.)
            OT_ASSERT(nullptr != pOwner);
        }
        // Else if purse IS password protected, then use its internal key.
        else if (thePurse.IsPasswordProtected())  // Purse is encrypted based on
                                                  // its own built-in symmetric
                                                  // key.
        {
            OTSymmetricKey* pSymmetricKey = thePurse.GetInternalKey();
            OT_ASSERT(nullptr != pSymmetricKey);
            const bool bGotPassphrase = thePurse.GetPassphrase(
                thePassword, thePWData.GetDisplayString());

            if (!bGotPassphrase)
                otErr << OT_METHOD << __FUNCTION__
                      << ": Authentication failed, or otherwise failed "
                         "retrieving secret from user.\n";
            // Below this point, we know thePassword is good, and we know
            // pSymmetricKey is good.
            // Therefore...
            //
            else {
                pOwner = new OTNym_or_SymmetricKey(
                    *pSymmetricKey,
                    thePassword,
                    pstrDisplay);  // Can't put
                                   // &strReason
                // here. (It goes
                // out of scope.)
                OT_ASSERT(nullptr != pOwner);
            }
        } else
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed: Somehow this purse is not "
                     "password-protected, nor "
                     "is it Nym-protected. (This error should "
                     "never actually happen.)\n";
        // (By this point, pOwner is all set up and ready to go.)
    } else
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure loading purse from string:\n"
              << strPurse << "\n";
    return pOwner;
}

/// Returns the TOKEN on top of the stock (LEAVING it on top of the stack,
/// but giving you a decrypted copy of it.)
///
/// NYM_ID can be nullptr, **if** the purse is password-protected.
/// (It's just ignored in that case.) Otherwise MUST contain the NymID for
/// the Purse owner (necessary to decrypt the token.)
///
/// CALLER must delete!!
///
/// returns nullptr if failure.
///
Token* OT_API::Purse_Peek(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const String& THE_PURSE,
    const Identifier* pOWNER_ID,  // This can be
                                  // nullptr,
                                  // **IF** purse
    // is password-protected. (It's
    // just ignored in that case.)
    // Otherwise MUST contain the NymID
    // for the Purse owner (necessary
    // to decrypt the token.)
    const String* pstrDisplay) const
{
    const String strReason1(
        (nullptr == pstrDisplay)
            ? "Enter your master passphrase for your wallet. (Purse_Peek)"
            : pstrDisplay->Get());
    const String strReason2(
        (nullptr == pstrDisplay)
            ? "Enter the passphrase for this purse. (Purse_Peek)"
            : pstrDisplay->Get());
    //  OTPasswordData thePWData(strReason);
    Purse thePurse(NOTARY_ID, INSTRUMENT_DEFINITION_ID);
    OTPassword thePassword;  // Only used in the case of password-protected
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
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        THE_PURSE,
        thePurse,
        thePassword,
        false,  // bForEncrypting=true by default. (Peek needs to decrypt.)
        pOWNER_ID,
        &strReason1,
        &strReason2));
    if (nullptr == pOwner)
        return nullptr;  // This already logs, no need for more logs.
    Token* token = nullptr;

    if (thePurse.IsEmpty())
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed attempt to peek; purse is empty.\n";
    else {
        token = thePurse.Peek(*pOwner);

        if (nullptr == token)
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed peeking a token from a "
                     "purse that supposedly had tokens on it...\n";
    }
    return token;
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
Purse* OT_API::Purse_Pop(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const String& THE_PURSE,
    const Identifier* pOWNER_ID,  // This can be
                                  // nullptr,
                                  // **IF** purse
    // is password-protected. (It's
    // just ignored in that case.)
    // Otherwise MUST contain the NymID
    // for the Purse owner (necessary
    // to decrypt the token.)
    const String* pstrDisplay) const
{
    const String strReason1(
        (nullptr == pstrDisplay)
            ? "Enter your master passphrase for your wallet. (Purse_Pop)"
            : pstrDisplay->Get());
    const String strReason2(
        (nullptr == pstrDisplay)
            ? "Enter the passphrase for this purse. (Purse_Pop)"
            : pstrDisplay->Get());
    //  OTPasswordData thePWData(strReason);
    std::unique_ptr<Purse> pPurse(
        new Purse(NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    OTPassword thePassword;  // Only used in the case of password-protected
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
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        THE_PURSE,
        *pPurse,
        thePassword,
        false,  // bForEncrypting=true by default, but Pop needs to decrypt.
        pOWNER_ID,
        &strReason1,
        &strReason2));
    if (nullptr == pOwner)
        return nullptr;  // This already logs, no need for more logs.
    Purse* pReturnPurse = nullptr;

    if (pPurse->IsEmpty())
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed attempt to pop; purse is empty.\n";
    else {
        std::unique_ptr<Token> token(pPurse->Pop(*pOwner));

        if (nullptr == token)
            otErr << OT_METHOD << __FUNCTION__
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
Purse* OT_API::Purse_Empty(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const String& THE_PURSE,
    const String* pstrDisplay) const
{
    const String strReason(
        (nullptr == pstrDisplay) ? "Making an empty copy of a cash purse."
                                 : pstrDisplay->Get());
    //  OTPasswordData thePWData(strReason);
    Purse* pPurse =
        Purse::PurseFactory(THE_PURSE, NOTARY_ID, INSTRUMENT_DEFINITION_ID);

    if (nullptr == pPurse) {
        otErr << OT_METHOD << __FUNCTION__
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
/// NYM_ID can be nullptr, **if** the purse is password-protected.
/// (It's just ignored in that case.) Otherwise MUST contain the NymID for
/// the Purse owner (necessary to encrypt the token.)
///
/// returns nullptr if failure.
///
Purse* OT_API::Purse_Push(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const String& THE_PURSE,
    const String& THE_TOKEN,
    const Identifier* pOWNER_ID,  // This can be nullptr,
                                  // **IF** purse
    // is password-protected. (It's
    // just ignored in that case.)
    // Otherwise MUST contain the NymID
    // for the Purse owner (necessary
    // to encrypt the token.)
    const String* pstrDisplay) const
{
    const String strReason1(
        (nullptr == pstrDisplay)
            ? "Enter your master passphrase for your wallet. (Purse_Push)"
            : pstrDisplay->Get());
    const String strReason2(
        (nullptr == pstrDisplay)
            ? "Enter the passphrase for this purse. (Purse_Push)"
            : pstrDisplay->Get());
    //  OTPasswordData thePWData(strReason);
    if (!THE_PURSE.Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Purse does not exist.\n";
        return nullptr;
    } else if (!THE_TOKEN.Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Token does not exist.\n";
        return nullptr;
    }
    String strToken(THE_TOKEN);
    std::unique_ptr<Token> token(
        Token::TokenFactory(strToken, NOTARY_ID, INSTRUMENT_DEFINITION_ID));

    if (nullptr == token)  // TokenFactory instantiates AND loads from string.
    {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to instantiate or load token from string:\n\n"
              << THE_TOKEN << "\n\n";
        return nullptr;
    }
    std::unique_ptr<Purse> pPurse(
        new Purse(NOTARY_ID, INSTRUMENT_DEFINITION_ID));
    OTPassword thePassword;  // Only used in the case of password-protected
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
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        THE_PURSE,
        *pPurse,
        thePassword,
        true,  // bForEncrypting=true by default.
        pOWNER_ID,
        &strReason1,
        &strReason2));
    if (nullptr == pOwner)
        return nullptr;  // This already logs, no need for more logs.
    Purse* pReturnPurse = nullptr;

    const bool bPushed = pPurse->Push(*pOwner, *token);

    if (!bPushed)
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed pushing a token onto a purse.\n";
    else {
        pReturnPurse = pPurse.release();
    }
    return pReturnPurse;

    // NOTE: Caller must release/sign/save pReturnPurse, once this returns, in
    // order to effect the change.
}

bool OT_API::Wallet_ImportPurse(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& SIGNER_ID,  // We must know the SIGNER_ID in order to
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
    const String& THE_PURSE,
    const String* pstrDisplay) const
{
    Lock lock(lock_);
    String reason(
        (nullptr == pstrDisplay) ? "Enter passphrase for purse being imported."
                                 : pstrDisplay->Get());
    OTPasswordData cashPasswordReason(
        (nullptr == pstrDisplay) ? OT_PW_DISPLAY : pstrDisplay->Get());
    OTPassword thePassword;  // Only used in the case of password-protected
                             // purses.
    auto context = wallet_.ServerContext(SIGNER_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << SIGNER_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return false;
    }

    auto nym = context->Nym();

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    std::unique_ptr<Purse> pNewPurse(
        new Purse(NOTARY_ID, INSTRUMENT_DEFINITION_ID));
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
        THE_PURSE,
        *pNewPurse,
        thePassword,
        false,       // bCanBePublic=false by default. (Private Nym must be
                     // loaded, if
                     // a nym is the owner.)
        &SIGNER_ID,  // This can be nullptr, **IF** purse is
                     // password-protected.
                     // (It's just ignored in that case.) Otherwise if it's
        // Nym-protected, the purse will have a NymID on it already,
        // which is what LoadPurseAndOwnerForMerge will try first.
        // If not (it's optional), then SIGNER_ID is the ID it will
        // try next, before failing altogether.
        &reason));
    if (nullptr == pNewOwner)
        return false;  // This already logs, no need for more logs.
    std::unique_ptr<Purse> pOldPurse(
        LoadPurse(NOTARY_ID, INSTRUMENT_DEFINITION_ID, SIGNER_ID));

    if (nullptr == pOldPurse)  // apparently there's not already a purse of this
                               // type, let's create it.
    {
        pOldPurse.reset(
            new Purse(NOTARY_ID, INSTRUMENT_DEFINITION_ID, SIGNER_ID));
    } else if (!pOldPurse->VerifySignature(*nym)) {
        otErr
            << __FUNCTION__
            << ": Failed to verify signature on old purse. (Very strange...)\n";
        return false;
    }
    // By this point, the old purse has either been loaded, or created.
    // Let's make sure the server and instrument definition id match between the
    // purses,
    // since they are now actually loaded up.
    //
    if (pOldPurse->GetNotaryID() != pNewPurse->GetNotaryID()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: NotaryIDs don't match between these two purses.\n";
        return false;
    } else if (
        pOldPurse->GetInstrumentDefinitionID() !=
        pNewPurse->GetInstrumentDefinitionID()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: InstrumentDefinitionIDs don't "
                 "match between these two purses.\n";
        return false;
    }
    //
    // By this point, I have two owners, and two purses.
    //
    // NOTE: if I want to pass in a custom display string here, nymfile could be
    // replaced with
    // pOldOwner (an OTNym_or_SymmetricKey instance) which can contain that
    // string.
    // (I'm referring to the string that contains the "reason" why the
    // passphrase needs to
    // be entered, so it can be shown to the user on the passphrase dialog.)
    //
    if (pOldPurse->Merge(
            *nym,         // signer
            *nym,         // old owner (must be private, if a nym.)
            *pNewOwner,   // new owner (must be private, if a nym.)
            *pNewPurse))  // new purse (being merged into old.)
    {
        pOldPurse->ReleaseSignatures();
        pOldPurse->SignContract(*nym);
        pOldPurse->SaveContract();
        return SavePurse(
            NOTARY_ID, INSTRUMENT_DEFINITION_ID, SIGNER_ID, *pOldPurse);
    } else  // Failed merge.
    {
        String strNymID1, strNymID2;
        nym->GetIdentifier(strNymID1);
        pNewOwner->GetIdentifier(strNymID2);
        otErr << OT_METHOD << __FUNCTION__ << ": (OldNymID: " << strNymID1
              << ".) (New Owner ID: " << strNymID2
              << ".) Failure merging new "
                 "purse:\n\n"
              << THE_PURSE << "\n\n";
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
Token* OT_API::Token_ChangeOwner(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const String& THE_TOKEN,
    const Identifier& SIGNER_NYM_ID,
    const String& OLD_OWNER,  // Pass a NymID here, or a purse.
    const String& NEW_OWNER,  // Pass a NymID here, or a purse.
    const String* pstrDisplay) const
{
    String strWalletReason(
        (nullptr == pstrDisplay)
            ? "Enter your wallet's master passphrase. (Token_ChangeOwner.)"
            : pstrDisplay->Get());
    String reason(
        (nullptr == pstrDisplay)
            ? "Enter the passphrase for this purse. (Token_ChangeOwner.)"
            : pstrDisplay->Get());
    OTPasswordData cashPasswordReason(strWalletReason);
    auto context = wallet_.ServerContext(SIGNER_NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << SIGNER_NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return nullptr;
    }

    auto pSignerNym = context->Nymfile(cashPasswordReason);

    if (false == bool(pSignerNym)) {
        return nullptr;
    }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    // ALL THE COMPLEXITY YOU SEE BELOW is mainly just about handling OLD_OWNER
    // and NEW_OWNER each as either a NymID or as a Purse (containing a
    // symmetric key and
    // a corresponding master key.)
    auto oldOwnerNymID = Identifier::Factory(),
         newOwnerNymID = Identifier::Factory();  // if either owner is a Nym,
                                                 // the ID goes here.
    std::unique_ptr<Purse> theOldPurseAngel;
    OTPassword theOldPassword;  // Only used in the case of password-protected
                                // purses.
    OTNym_or_SymmetricKey* pOldOwner = nullptr;
    std::unique_ptr<OTNym_or_SymmetricKey> theOldOwnerAngel;
    std::unique_ptr<Purse> theNewPurseAngel;
    OTPassword theNewPassword;  // Only used in the case of password-protected
                                // purses.
    OTNym_or_SymmetricKey* pNewOwner = nullptr;
    std::unique_ptr<OTNym_or_SymmetricKey> theNewOwnerAngel;
    const bool bOldOwnerIsPurse = OLD_OWNER.Contains("PURSE");
    const bool bNewOwnerIsPurse = NEW_OWNER.Contains("PURSE");
    if (!bOldOwnerIsPurse)  // The old owner is a NYM (public/private keys.)
    {
        oldOwnerNymID->SetString(OLD_OWNER);
        auto pOldNym = wallet_.Nym(oldOwnerNymID);

        if (nullptr == pOldNym) {
            return nullptr;
        }

        pOldOwner = new OTNym_or_SymmetricKey(*pOldNym, &strWalletReason);
        OT_ASSERT(nullptr != pOldOwner);
        theOldOwnerAngel.reset(pOldOwner);
    } else  // The old owner is a PURSE (Symmetric/master keys, internal to that
            // purse.)
    {
        // if the old owner is a Purse (symmetric+master key), the entire
        // purse is loaded.
        Purse* pOldPurse = new Purse(NOTARY_ID, INSTRUMENT_DEFINITION_ID);
        OT_ASSERT(nullptr != pOldPurse);
        theOldPurseAngel.reset(pOldPurse);
        pOldOwner = LoadPurseAndOwnerForMerge(
            OLD_OWNER,
            *pOldPurse,
            theOldPassword,
            false,           // bCanBePublic=false by default. In this case, it
                             // definitely
                             // must be private.
            &SIGNER_NYM_ID,  // This can be nullptr, **IF** purse is
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
            &reason);
        theOldOwnerAngel.reset(pOldOwner);
        if (nullptr == pOldOwner)
            return nullptr;  // This already logs, no need for more logs.
    }
    if (!bNewOwnerIsPurse)  // The new owner is a NYM
    {
        newOwnerNymID->SetString(NEW_OWNER);
        auto pNewNym = wallet_.Nym(newOwnerNymID);
        if (nullptr == pNewNym) return nullptr;
        pNewOwner = new OTNym_or_SymmetricKey(*pNewNym, &strWalletReason);
        OT_ASSERT(nullptr != pNewOwner);
        theNewOwnerAngel.reset(pNewOwner);
    } else  // The new owner is a PURSE
    {
        // if the new owner is a Purse (symmetric+master key), the entire purse
        // is loaded.
        Purse* pNewPurse = new Purse(NOTARY_ID, INSTRUMENT_DEFINITION_ID);
        OT_ASSERT(nullptr != pNewPurse);
        theNewPurseAngel.reset(pNewPurse);
        pNewOwner = LoadPurseAndOwnerForMerge(
            NEW_OWNER,
            *pNewPurse,
            theNewPassword,
            true,  // bCanBePublic=false by default, but set TRUE here, since
                   // you
            // SHOULD be able to re-assign ownership of a token to someone
            // else, without having to load their PRIVATE key (which you
            // don't have.) Sort of irrelevant here actually, since this
            // block is for purses only...
            &SIGNER_NYM_ID,  // This can be nullptr, **IF** purse is
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
            &reason);
        theNewOwnerAngel.reset(pNewOwner);
        if (nullptr == pNewOwner)
            return nullptr;  // This already logs, no need for more logs.
    }
    //
    // (By this point, pOldOwner and pNewOwner should both be good to go.)
    //
    std::unique_ptr<Token> token(
        Token::TokenFactory(THE_TOKEN, NOTARY_ID, INSTRUMENT_DEFINITION_ID));
    OT_ASSERT(nullptr != token);
    if (false == token->ReassignOwnership(
                     *pOldOwner,   // must be private, if a Nym.
                     *pNewOwner))  // can be public, if a Nym.
    {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error re-assigning ownership of token.\n";
    } else {
        otLog3 << __FUNCTION__
               << ": Success re-assigning ownership of token.\n";

        token->ReleaseSignatures();

        auto pNym = wallet_.Nym(pSignerNym->GetConstID());
        OT_ASSERT(nullptr != pNym);

        token->SignContract(*pNym);
        token->SaveContract();

        return token.release();
    }

    return nullptr;
}

// LOAD Mint
//
// Returns an OTMint pointer, or nullptr.
// (Caller responsible to delete.)
//
Mint* OT_API::LoadMint(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    const String strNotaryID(NOTARY_ID);
    const String strInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
    auto pServer = wallet_.Server(NOTARY_ID);

    if (!pServer) {
        return nullptr;
    }

    auto pServerNym = pServer->Nym();

    if (!pServerNym) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to get contract public Nym for NotaryID: "
              << strNotaryID << " \n";
        return nullptr;
    }
    Mint* mint = Mint::MintFactory(strNotaryID, strInstrumentDefinitionID);
    OT_ASSERT_MSG(
        nullptr != mint,
        "OT_API::LoadMint: Error allocating memory in the OT API");
    // responsible to delete or return mint below this point.
    if (!mint->LoadMint() || !mint->VerifyMint(*pServerNym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load or verify Mintfile : " << OTFolders::Mint()
              << Log::PathSeparator() << strNotaryID << Log::PathSeparator()
              << strInstrumentDefinitionID << "\n";
        delete mint;
        mint = nullptr;
        return nullptr;
    }
    return mint;
}
#endif  // OT_CASH

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
std::shared_ptr<Account> OT_API::LoadAssetAccount(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID) const
{
    OTWallet* pWallet =
        GetWallet(__FUNCTION__);  // This logs and ASSERTs already.
    if (nullptr == pWallet) return nullptr;
    // By this point, pWallet is a good pointer.  (No need to cleanup.)
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return nullptr;
    }

    auto nym = context->Nym();

    return pWallet->LoadAccount(*nym, ACCOUNT_ID, NOTARY_ID, __FUNCTION__);
}

// LOAD NYMBOX
//
// Caller IS responsible to delete
Ledger* OT_API::LoadNymbox(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return nullptr;
    }

    auto nym = context->Nym();
    std::unique_ptr<Ledger> pLedger{
        Ledger::GenerateLedger(NYM_ID, NYM_ID, NOTARY_ID, Ledger::nymbox)};

    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API.");

    if (pLedger->LoadNymbox() && pLedger->VerifyAccount(*nym)) {

        return pLedger.release();
    }

    return nullptr;
}

// LOAD NYMBOX NO VERIFY
// (VerifyAccount, for ledgers, loads all the Box Receipts. You may not want
// this.
// For example, you may be loading this ledger precisely so you can iterate
// through its receipts and download them from the server, so they will all
// load up on a subsequent verify.)
//
// Caller IS responsible to delete
Ledger* OT_API::LoadNymboxNoVerify(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    if (!wallet_.IsLocalNym(NYM_ID.str())) {
        return nullptr;
    }
    // By this point, nymfile is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{
        Ledger::GenerateLedger(NYM_ID, NYM_ID, NOTARY_ID, Ledger::nymbox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API.");
    // ---------------------------------------------
    if (pLedger->LoadNymbox())  // The Verify would go here.
        return pLedger.release();
    else {
        String strNymID(NYM_ID);
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load nymbox: " << strNymID << "\n";
    }
    return nullptr;
}

// LOAD INBOX
//
// Caller IS responsible to delete
Ledger* OT_API::LoadInbox(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return nullptr;
    }

    auto nym = context->Nym();

    // By this point, nym is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{
        Ledger::GenerateLedger(NYM_ID, ACCOUNT_ID, NOTARY_ID, Ledger::inbox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API.");

    if (pLedger->LoadInbox() && pLedger->VerifyAccount(*nym))
        return pLedger.release();
    else {
        String strNymID(NYM_ID), strAcctID(ACCOUNT_ID);
        otWarn << OT_METHOD << __FUNCTION__
               << ": Unable to load or verify inbox: " << strAcctID
               << "\n For user: " << strNymID << "\n";
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
Ledger* OT_API::LoadInboxNoVerify(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    if (!wallet_.IsLocalNym(NYM_ID.str())) {
        return nullptr;
    }
    // By this point, nymfile is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{
        Ledger::GenerateLedger(NYM_ID, ACCOUNT_ID, NOTARY_ID, Ledger::inbox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API");

    if (pLedger->LoadInbox())  // The Verify would go here.
        return pLedger.release();
    else {
        String strNymID(NYM_ID), strAcctID(ACCOUNT_ID);
        otWarn << OT_METHOD << __FUNCTION__
               << ": Unable to load inbox: " << strAcctID
               << "\n For user: " << strNymID << "\n";
    }
    return nullptr;
}

// LOAD OUTBOX
//
// Caller IS responsible to delete
Ledger* OT_API::LoadOutbox(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return nullptr;
    }

    auto nym = context->Nym();

    // By this point, nym is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{
        Ledger::GenerateLedger(NYM_ID, ACCOUNT_ID, NOTARY_ID, Ledger::outbox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API");

    if (pLedger->LoadOutbox() && pLedger->VerifyAccount(*nym))
        return pLedger.release();
    else {
        String strNymID(NYM_ID), strAcctID(ACCOUNT_ID);
        otWarn << OT_METHOD << __FUNCTION__
               << ": Unable to load  or verify outbox: " << strAcctID
               << "\n For user: " << strNymID << "\n";
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
Ledger* OT_API::LoadOutboxNoVerify(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    if (!wallet_.IsLocalNym(NYM_ID.str())) {
        return nullptr;
    }
    // By this point, nymfile is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{
        Ledger::GenerateLedger(NYM_ID, ACCOUNT_ID, NOTARY_ID, Ledger::outbox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API");

    if (pLedger->LoadOutbox())  // The Verify would go here.
        return pLedger.release();
    else {
        String strNymID(NYM_ID), strAcctID(ACCOUNT_ID);
        otWarn << OT_METHOD << __FUNCTION__
               << ": Unable to load  or verify outbox: " << strAcctID
               << "\n For user: " << strNymID << "\n";
    }
    return nullptr;
}

// Caller is responsible to delete!
Ledger* OT_API::LoadPaymentInbox(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return nullptr;
    }

    auto nym = context->Nym();

    std::unique_ptr<Ledger> pLedger{Ledger::GenerateLedger(
        NYM_ID, NYM_ID, NOTARY_ID, Ledger::paymentInbox)};

    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API");

    if (pLedger->LoadPaymentInbox() && pLedger->VerifyAccount(*nym)) {
        return pLedger.release();
    } else {
        String strNymID(NYM_ID), strAcctID(NYM_ID);
        otWarn << OT_METHOD << __FUNCTION__
               << ": Unable to load or verify for Nym/Acct: " << strNymID
               << " / " << strAcctID << "\n";
    }
    return nullptr;
}

// Caller is responsible to delete!
Ledger* OT_API::LoadPaymentInboxNoVerify(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    if (!wallet_.IsLocalNym(NYM_ID.str())) {
        return nullptr;
    }
    // By this point, nymfile is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{Ledger::GenerateLedger(
        NYM_ID, NYM_ID, NOTARY_ID, Ledger::paymentInbox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API");

    if (pLedger->LoadPaymentInbox())  // The Verify would have gone here.
        return pLedger.release();
    else {
        String strNymID(NYM_ID), strAcctID(NYM_ID);
        otWarn << OT_METHOD << __FUNCTION__
               << ": Unable to load or verify for Nym/Acct: " << strNymID
               << " / " << strAcctID << "\n";
    }
    return nullptr;
}

// Caller IS responsible to delete
Ledger* OT_API::LoadRecordBox(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return nullptr;
    }

    auto nym = context->Nym();

    // By this point, nym is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{Ledger::GenerateLedger(
        NYM_ID, ACCOUNT_ID, NOTARY_ID, Ledger::recordBox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API");

    const bool bLoaded = pLedger->LoadRecordBox();
    bool bVerified = false;
    if (bLoaded) bVerified = pLedger->VerifyAccount(*nym);

    if (bLoaded && bVerified)
        return pLedger.release();
    else {
        String strNymID(NYM_ID), strAcctID(ACCOUNT_ID);
        otWarn << __FUNCTION__
               << ": Unable to load or verify for Nym/Acct: " << strNymID
               << " / " << strAcctID << "\n";
    }
    return nullptr;
}

// Caller is responsible to delete!
Ledger* OT_API::LoadRecordBoxNoVerify(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    if (!wallet_.IsLocalNym(NYM_ID.str())) {
        return nullptr;
    }
    // By this point, nymfile is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{Ledger::GenerateLedger(
        NYM_ID, ACCOUNT_ID, NOTARY_ID, Ledger::recordBox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API");

    if (pLedger->LoadRecordBox())  // Verify would go here.
        return pLedger.release();
    else {
        String strNymID(NYM_ID), strAcctID(ACCOUNT_ID);
        otWarn << __FUNCTION__
               << ": Unable to load or verify for Nym/Acct: " << strNymID
               << " / " << strAcctID << "\n";
    }
    return nullptr;
}

// Caller IS responsible to delete.
Ledger* OT_API::LoadExpiredBox(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return nullptr;
    }

    auto nym = context->Nym();

    // By this point, nym is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{
        Ledger::GenerateLedger(NYM_ID, NYM_ID, NOTARY_ID, Ledger::expiredBox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API");

    const bool bLoaded = pLedger->LoadExpiredBox();
    bool bVerified = false;
    if (bLoaded) bVerified = pLedger->VerifyAccount(*nym);

    if (bLoaded && bVerified)
        return pLedger.release();
    else {
        String strNymID(NYM_ID), strNotaryID(NOTARY_ID);
        otWarn << OT_METHOD << __FUNCTION__
               << ": Unable to load or verify for Nym/Notary: " << strNymID
               << " / " << strNotaryID << "\n";
    }
    return nullptr;
}

Ledger* OT_API::LoadExpiredBoxNoVerify(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    if (!wallet_.IsLocalNym(NYM_ID.str())) {
        return nullptr;
    }
    // By this point, nymfile is a good pointer, and is on the wallet.
    // (No need to cleanup later.)
    // ---------------------------------------------
    std::unique_ptr<Ledger> pLedger{
        Ledger::GenerateLedger(NYM_ID, NYM_ID, NOTARY_ID, Ledger::expiredBox)};
    OT_NEW_ASSERT_MSG(pLedger, "Error allocating memory in the OT API");

    if (pLedger->LoadExpiredBox())  // The Verify would have gone here.
        return pLedger.release();
    else {
        String strNymID(NYM_ID), strNotaryID(NOTARY_ID);
        otWarn << OT_METHOD << __FUNCTION__
               << ": Unable to load or verify for Nym/Notary: " << strNymID
               << " / " << strNotaryID << "\n";
    }
    return nullptr;
}

bool OT_API::ClearExpired(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const std::int32_t nIndex,
    bool bClearAll) const  // if true, nIndex is
                           // ignored.
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return false;
    }

    auto nym = context->Nym();
    std::unique_ptr<Ledger> pExpiredBox(LoadExpiredBox(NOTARY_ID, NYM_ID));

    if (false == bool(pExpiredBox)) {
        pExpiredBox.reset(Ledger::GenerateLedger(
            NYM_ID, NYM_ID, NOTARY_ID, Ledger::expiredBox, true));

        if (false == bool(pExpiredBox)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to load or create expired box (and thus "
                     "unable to do anything with it.)\n";
            return false;
        }
    }
    if (bClearAll) {
        pExpiredBox->ReleaseTransactions();
        pExpiredBox->ReleaseSignatures();
        pExpiredBox->SignContract(*nym);
        pExpiredBox->SaveContract();
        pExpiredBox->SaveExpiredBox();
        return true;
    }
    // Okay, it's not "clear all" but "clear at index" ...
    //
    const std::int32_t nTransCount = pExpiredBox->GetTransactionCount();

    if ((nIndex < 0) || (nIndex >= nTransCount)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Index out of bounds (highest allowed index for this "
                 "expired box is "
              << nTransCount - 1 << ".)\n";
        return false;
    }
    OTTransaction* transaction = pExpiredBox->GetTransactionByIndex(nIndex);
    bool bRemoved = false;

    if (nullptr != transaction) {
        const std::int64_t lTransactionNum = transaction->GetTransactionNum();

        if (!pExpiredBox->DeleteBoxReceipt(lTransactionNum)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed trying to delete the box receipt for a "
                     "transaction being removed from a expired box: "
                  << lTransactionNum << "\n";
        }
        bRemoved = pExpiredBox->RemoveTransaction(lTransactionNum);
    }
    if (bRemoved) {
        pExpiredBox->ReleaseSignatures();
        pExpiredBox->SignContract(*nym);
        pExpiredBox->SaveContract();
        pExpiredBox->SaveExpiredBox();
        return true;
    } else {
        const std::int32_t nTemp = static_cast<std::int32_t>(nIndex);
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to clear an expired record from "
                 "the expired box at index: "
              << nTemp << "\n";
    }
    return false;
}

// From OTAPI.cpp:
//
// std::int32_t         OT_API_GetNym_OutpaymentsCount(const char * NYM_ID);
//
// const char *    OT_API_GetNym_OutpaymentsContentsByIndex(const char * NYM_ID,
// std::int32_t nIndex); /// returns the message itself
//
// const char *    OT_API_GetNym_OutpaymentsRecipientIDByIndex(const char *
// NYM_ID, std::int32_t nIndex); /// returns the NymID of the recipient.
// const char *    OT_API_GetNym_OutpaymentsNotaryIDByIndex(const char * NYM_ID,
// std::int32_t nIndex); /// returns the NotaryID where the message came from.
//
// std::int32_t OT_API_Nym_RemoveOutpaymentsByIndex(const char * NYM_ID,
// std::int32_t
// nIndex); /// actually returns OT_BOOL, (1 or 0.)
// std::int32_t OT_API_Nym_VerifyOutpaymentsByIndex(const char * NYM_ID,
// std::int32_t
// nIndex); /// actually returns OT_BOOL. OT_TRUE if signature verifies. (Sender
// Nym MUST be in my wallet for this to work.)

// If you write a cheque, a copy should go in outpayments box.
// If you SEND a cheque, a copy should go in your outpayments box. (Perhaps just
// replacing the first one.)
// Once that cheque is CASHED, the copy should be removed from the outpayments.
// (Copied to record box.)
//
// If the cheque is discarded by sender, it can be moved in the same
// fashion. But the critical difference is, the recipient MIGHT still
// have a copy of it, and thus MIGHT still run it through, unless you
// take that discarded number and use it on another server transaction
// or cancel it somehow. Normally you'd just harvest the number for some
// other transaction (which the "discard cheque" function actually does)
// but if I think about it, that cheque can still come through,
// meanwhile I'm thinking the transaction # is available still to be
// attached to some other cheque (at least as far as my wallet can tell,
// if I've harvested the number by this point.) My wallet will think the
// server is trying to screw me somehow, since it apparently hasn't even
// USED that number yet on a subsequent transaction, yet here's the
// server claiming it's already used and funds were deducted! Therefore
// we MUST (todo) add the functionality to cancel a transaction number!
// TODO: Need a server message for canceling a transaction. IF there are
// no receipts in your inbox or outbox regarding a transaction #, then
// you can cancel it. (Hmm re-think: Is that possible? What if I have a
// transaction # out there on a smart contract somewhere -- that someone
// ELSE activated? Then how would I find it? Perhaps server-side Nym
// needs to track that...)
//
// Really, if a cheque receipt comes through, and we accept it out of
// the inbox, then the "SUCCESS" reply for that processInbox is where we
// need to remove the corresponding outpayments entry and move it to the
// record box.
//
// Similarly, if a cheque is canceled (transaction # is canceled) then
// we should receive a "SUCCESS" reply (to that cancel transaction) and
// again, remove the corresponding outpayments entry and move it to the
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
   of a successful server-reply to my deposit transaction, need to move it from
 my payments inbox to my
   record box. (The record box will eventually be a callback so that client
 software can take over that
   functionality, which is outside the scope of OT. The actual CALL to store in
 the record box, however
   should occur inside OT.)
 - For now, I'm using the below API call, so it's available inside the scripts.
 This is "good enough"
   for now, just to get the payments inbox/outbox working for the scripts. But
 in the std::int64_t term, I'll need
   to add the hooks directly into OT as described just above. (It'll be
 necessary in order to get the record
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

// So far I haven't needed this yet, since sent and received payments
// already handle moving payments-inbox receipts to the record box, and
// moving outpayments instruments to the record box (built into OT.) But
// finally a case occured: the instrument is expired, so OT will never
// move it, since OT can never deposit it, in the case of incoming
// payments, and in the case of outgoing payments, clearly the recipient
// never deposited it, or I would have gotten a receipt by now and it
// would have already been cleared out of my outpayments box. Since
// neither of those cases will ever happen, for an expired instrument,
// then how in the heck do I get that damned instrument out of my
// outpayments / payments inbox, and moved over to the record box so the
// client software can deal with it? Answer: this API call:
// OT_API::RecordPayment
//
// ONE MORE THING: Let's say I sent a cheque and it expired. The
// recipient never cashed it. At this point, I am SAFE to harvest the
// transaction number(s) back off of the cheque. After all, it was never
// cashed, right? And since it's now expired, it never WILL be cashed,
// right? Therefore I NEED to harvest the transaction number back from
// the expired instrument, so I can use it again in the future and
// eventually get it closed out.
//
// UPDATE: This should now also work for smart contracts and payment plans.
bool OT_API::RecordPayment(
    const Identifier& TRANSPORT_NOTARY_ID,  // Transport Notary for inPayments
                                            // box. (May differ from notary ID
                                            // on the payment itself)
    const Identifier& NYM_ID,
    bool bIsInbox,        // true == payments inbox. false == outpayments box.
    std::int32_t nIndex,  // removes payment instrument (from payments inbox or
                          // outpayments box) and moves to record box.
    bool bSaveCopy) const
{
    // ----------------------------------------
    rLock transport_lock(
        lock_callback_({NYM_ID.str(), TRANSPORT_NOTARY_ID.str()}));
    auto transport_context =
        wallet_.mutable_ServerContext(NYM_ID, TRANSPORT_NOTARY_ID);
    auto transport_nymfile =
        transport_context.It().mutable_Nymfile(__FUNCTION__);
    auto transport_nym = transport_context.It().Nym();

    // Sometimes the payment and transport notaries are the same
    // notary. Sometimes they are different, we handle both cases.
    ConstNym payment_nym;  // So far, still null.
    // ----------------------------------------
    std::unique_ptr<rLock> payment_lock;

    // All this to solve a problem of scope...
    class recordpayment_cleanup1
    {
        Editor<opentxs::ServerContext> payment_context_;

    public:
        Editor<opentxs::ServerContext>& paymentContext()
        {
            return payment_context_;
        }

        recordpayment_cleanup1(Editor<opentxs::ServerContext>&& payment_context)
            : payment_context_(std::move(payment_context))
        {
        }
    };
    class recordpayment_cleanup2
    {
        Editor<class NymFile> payment_nymfile_;

    public:
        Editor<class NymFile>& paymentNymfile() { return payment_nymfile_; }

        recordpayment_cleanup2(Editor<class NymFile>&& payment_nymfile)
            : payment_nymfile_(std::move(payment_nymfile))
        {
        }
    };
    std::unique_ptr<recordpayment_cleanup1> pCleanup1;
    std::unique_ptr<recordpayment_cleanup2> pCleanup2;

    Editor<opentxs::ServerContext>* payment_context{nullptr};
    Editor<class NymFile>* payment_nymfile{nullptr};
    // ----------------------------------------
    Ledger* pRecordBox{nullptr};
    Ledger* pExpiredBox{nullptr};
    Ledger* pActualBox{nullptr};  // Points to either pRecordBox or pExpiredBox

    std::unique_ptr<Ledger> theRecordBoxAngel;
    std::unique_ptr<Ledger> theExpiredBoxAngel;

    if (bSaveCopy) {
        pRecordBox = LoadRecordBox(TRANSPORT_NOTARY_ID, NYM_ID, NYM_ID);
        pExpiredBox = LoadExpiredBox(TRANSPORT_NOTARY_ID, NYM_ID);
        theRecordBoxAngel.reset(pRecordBox);
        theExpiredBoxAngel.reset(pExpiredBox);

        if (nullptr == pRecordBox) {
            pRecordBox = Ledger::GenerateLedger(
                NYM_ID, NYM_ID, TRANSPORT_NOTARY_ID, Ledger::recordBox, true);
            if (nullptr == pRecordBox) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to load or create record box "
                         "(and thus unable to do anything with it.)\n";
                return false;
            }
            theRecordBoxAngel.reset(pRecordBox);
        }
        if (nullptr == pExpiredBox) {
            pExpiredBox = Ledger::GenerateLedger(
                NYM_ID, NYM_ID, TRANSPORT_NOTARY_ID, Ledger::expiredBox, true);
            if (nullptr == pExpiredBox) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to load or create expired box"
                         "(and thus unable to do anything with it.)\n";
                return false;
            }
            theExpiredBoxAngel.reset(pExpiredBox);
        }
    }
    pActualBox = pRecordBox;
    Ledger* pPaymentInbox{nullptr};
    std::unique_ptr<Ledger> thePaymentBoxAngel;

    bool bIsExpired{false};

    // first block:
    OTTransaction* transaction{nullptr};
    std::unique_ptr<OTTransaction> theTransactionAngel;

    // second block:
    std::unique_ptr<Message> theMessageAngel;

    bool bRemoved{false};

    std::unique_ptr<OTPayment> pPayment;

    if (bIsInbox) {
        pPaymentInbox = LoadPaymentInbox(TRANSPORT_NOTARY_ID, NYM_ID);
        thePaymentBoxAngel.reset(pPaymentInbox);

        if (nullptr == pPaymentInbox) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to load payment inbox "
                     "(and thus unable to do anything with it.)\n";
            return false;
        }
        if ((nIndex < 0) || (nIndex >= pPaymentInbox->GetTransactionCount())) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to find transaction in payment inbox "
                     "based on index "
                  << nIndex << ". (Out of bounds.)\n";
            return false;
        }
        transaction = pPaymentInbox->GetTransactionByIndex(nIndex);

        if (nullptr == transaction) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to find transaction in payment inbox "
                     "based on index "
                  << nIndex << ".\n";
            return false;
        }
        pPayment.reset(
            GetInstrumentByIndex(*transport_nym, nIndex, *pPaymentInbox));
        // ------------------------------------------
        // Payment Notary (versus the Transport Notary).
        //
        OTIdentifier paymentNotaryId = Identifier::Factory();

        if (pPayment->GetNotaryID(paymentNotaryId)) {
            if (paymentNotaryId != TRANSPORT_NOTARY_ID) {
                payment_lock.reset(new rLock(
                    lock_callback_({NYM_ID.str(), paymentNotaryId->str()})));

                pCleanup1.reset(new recordpayment_cleanup1(
                    wallet_.mutable_ServerContext(NYM_ID, paymentNotaryId)));
                payment_context = &(pCleanup1->paymentContext());

                pCleanup2.reset(new recordpayment_cleanup2(
                    payment_context->It().mutable_Nymfile(__FUNCTION__)));
                payment_nymfile = &(pCleanup2->paymentNymfile());

                payment_nym = payment_context->It().Nym();
            } else {
                payment_context = &transport_context;
                payment_nymfile = &transport_nymfile;
                payment_nym = transport_nym;
            }
        }
        // ------------------------------------------
        pPayment->IsExpired(bIsExpired);

        if (bIsExpired) pActualBox = pExpiredBox;

        // Remove it from the payments inbox...
        //
        const std::int64_t lTransactionNum = transaction->GetTransactionNum();

        if (!pPaymentInbox->DeleteBoxReceipt(lTransactionNum)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed trying to delete the box receipt for a "
                     "transaction being removed from the payment inbox: "
                  << lTransactionNum << "\n";
        }
        bRemoved = pPaymentInbox->RemoveTransaction(
            lTransactionNum,
            false);  // bDeleteIt=true by default. We pass false
                     // since we are moving it to another box.
                     // Note that we still need to save
                     // pPaymentInbox somewhere below, assuming
                     // it's all successful.
        theTransactionAngel.reset(
            transaction);  // If below we put transaction onto the Record Box,
                           // then we have to set this to nullptr.

        // NOTE: transaction is still good, below this point, and will be
        // cleaned up automatically whenever we exit this function.

        // Anything else?
        // Note: no need to harvest transaction number for incoming payments.
        // But for outgoing (see below) then harvesting becomes an issue.
    } else  // Outpayments box (which is not stored in an OTLedger like payments
            // inbox, but rather, is stored similarly to outmail.)
    {
        if ((nIndex < 0) ||
            (nIndex >= transport_nymfile.It().GetOutpaymentsCount())) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to find payment in outpayment box based "
                     "on index "
                  << nIndex << ". (Out of bounds.)\n";
            return false;
        }
        Message* pMessage =
            transport_nymfile.It().GetOutpaymentsByIndex(nIndex);

        if (nullptr == pMessage) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to find payment message in outpayment "
                     "box based on index "
                  << nIndex << ".\n";
            return false;
        }

        String strInstrument;
        if (!pMessage->m_ascPayload.GetString(strInstrument)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to find payment instrument in outpayment "
                     "message at index "
                  << nIndex << ".\n";
            return false;
        }
        OTPayment thePayment(strInstrument);
        originType theOriginType = originType::not_applicable;

        if (thePayment.IsValid() && thePayment.SetTempValues()) {
            // EXPIRED?
            //
            thePayment.IsExpired(bIsExpired);
            if (bIsExpired) pActualBox = pExpiredBox;
            // ------------------------------------------
            // Payment Notary (versus the Transport Notary).
            //
            OTIdentifier paymentNotaryId = Identifier::Factory();
            if (thePayment.GetNotaryID(paymentNotaryId)) {
                // If you write a cheque drawn on server ABC, and then you send
                // it to me on my server DEF, then I will open my payments inbox
                // on server DEF, which is the Transport Notary, and inside I
                // will find a cheque from you, drawn on server ABC, which is
                // the Payment Notary.
                // So one is the notary where I prefer to receive messages, and
                // the other is the notary where your cheque is drawn on. They
                // are not necessarily the same, and usually different in fact.
                // So we distinguish between the two notaries depending on what
                // we need to do.
                //
                // In this case, if the notary IDs do not match, then I need to
                // lock ANOTHER context here -- the one where the payment is
                // processed, versus the notary where the payment instrument
                // was received. (Which is what we locked at the top of this
                // function).
                //
                if (paymentNotaryId != TRANSPORT_NOTARY_ID) {
                    payment_lock.reset(new rLock(lock_callback_(
                        {NYM_ID.str(), paymentNotaryId->str()})));

                    pCleanup1.reset(new recordpayment_cleanup1(
                        wallet_.mutable_ServerContext(
                            NYM_ID, paymentNotaryId)));
                    payment_context = &(pCleanup1->paymentContext());

                    pCleanup2.reset(new recordpayment_cleanup2(
                        payment_context->It().mutable_Nymfile(__FUNCTION__)));
                    payment_nymfile = &(pCleanup2->paymentNymfile());

                    payment_nym = payment_context->It().Nym();
                } else {
                    payment_context = &transport_context;
                    payment_nymfile = &transport_nymfile;
                    payment_nym = transport_nym;
                }
            }
            // ------------------------------------------
            // Anything but a purse?
            //
            std::int64_t lPaymentOpeningNum = 0;
            std::int64_t lPaymentTransNum = 0;
            if (thePayment.GetOpeningNum(
                    lPaymentOpeningNum,
                    NYM_ID))  // cheques, invoices, vouchers, smart contracts,
                              // payment plans.
            {
                // We we-grab the transaction number at this time. That way if
                // it's a transaction num that
                // belongs to some other Nym (and is different than our own
                // opening number) then we will
                // get the different number here.
                //
                if (!thePayment.GetTransactionNum(lPaymentTransNum)) {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Should never happen! "
                             "Failed to get transaction num from payment "
                             "RIGHT AFTER succeeded getting opening num.\n";
                }
                // However, if it IS a different number, in the case of smart
                // contracts and payment plans, we then change it BACK to the
                // opening number again. Read the next comment for details why.
                //
                bool bIsRecurring = false;

                if ((OTPayment::PAYMENT_PLAN == thePayment.GetType()) ||
                    (OTPayment::SMART_CONTRACT == thePayment.GetType())) {
                    bIsRecurring = true;

                    if (OTPayment::PAYMENT_PLAN == thePayment.GetType()) {
                        theOriginType = originType::origin_payment_plan;
                    } else if (
                        OTPayment::SMART_CONTRACT == thePayment.GetType()) {
                        theOriginType = originType::origin_smart_contract;
                    }

                    lPaymentTransNum = lPaymentOpeningNum;
                    // We do this because the ACTUAL transaction number on a
                    // smart contract or payment plan might be different that
                    // THIS Nym's opening number (it might be some other Nym's
                    // opening number.) But even if that's the case, we still
                    // want to harvest THIS Nym's opening number, not the other
                    // Nym's opening number. So for these instruments, we set
                    // the "transaction number" to be THIS Nym's opening number.
                    // (Versus just saying, "Oh the trans number is for some
                    // other Nym, so just ignore this" which would cause us to
                    // not harvest the numbers for THIS Nym that we probably
                    // SHOULD be harvesting.
                }

                // See what account the payment instrument is drawn from. Is it
                // mine? If so, load up the inbox and see if there are any
                // related receipts inside. If so, do NOT harvest the
                // transaction numbers from the instrument. Otherwise, harvest
                // them. (The instrument hasn't been redeemed yet.) Also, use
                // the transaction number on the instrument to see if it's
                // signed out to me.
                //
                // Hmm: If the instrument is definitely expired, and there's
                // definitely nothing in the asset accountinbox, then I can
                // DEFINITELY harvest it back.
                //
                // But if the instrument is definitely NOT expired, and the
                // transaction # IS signed out to ME, then I can't just harvest
                // the numbers, since the original recipient could still come
                // through and deposit that cheque. So in this case, I would
                // HAVE to cancel the transaction, and then such cancellation
                // would automatically harvest while processing the successful
                // server reply.
                //
                // Therefore make sure not to move the instrument here, unless
                // it's definitely expired. Whereas if it's not expired, then
                // the API must cancel it with the server, and can't simply come
                // in here and move/harvest it. So this function can only be for
                // expired transactions or those where the transaction number is
                // no longer issued. (And in cases where it's expired but STILL
                // issued, then it definitely DOES need to harvest.)

                bool bShouldHarvestPayment = false;
                bool bNeedToLoadAssetAcctInbox = false;
                auto theSenderNymID = Identifier::Factory(),
                     theSenderAcctID = Identifier::Factory();

                bool bPaymentSenderIsNym = false;
                bool bFromAcctIsAvailable = false;
                if (thePayment.IsVoucher()) {
                    bPaymentSenderIsNym =
                        (thePayment.GetRemitterNymID(theSenderNymID) &&
                         payment_nymfile->It().CompareID(theSenderNymID));
                    bFromAcctIsAvailable =
                        thePayment.GetRemitterAcctID(theSenderAcctID);
                } else {
                    bPaymentSenderIsNym =
                        (thePayment.GetSenderNymID(theSenderNymID) &&
                         payment_nymfile->It().CompareID(theSenderNymID));
                    bFromAcctIsAvailable =
                        thePayment.GetSenderAcctID(theSenderAcctID);
                }
                if (bPaymentSenderIsNym ||  // true for cheques as well as
                                            // vouchers. (We grab the remitter
                                            // above, for vouchers.)
                    bIsRecurring)  // false for cheques/vouchers; true for
                                   // payment plans and smart contracts.
                {
                    // NOTE: With bPaymentSenderIsNym, we know nymfile owns the
                    // transaction number on the cheque.
                    // NOTE: with bIsRecurring, we know nymfile is one of the
                    // parties of the smart contract.
                    // (Since we found an opening number for nymfile on it.)

                    // If the transaction # isn't signed out to me, then there's
                    // no need to check the inbox for any receipts, since those
                    // would have to have been already closed out, in order for
                    // the number not to be signed out to me anymore.
                    //
                    // Therefore let's check that first, before bothering to
                    // load the inbox.
                    //

                    // If I'm in the middle of trying to sign it out...
                    if (payment_context->It().VerifyTentativeNumber(
                            lPaymentTransNum)) {
                        otErr << OT_METHOD << __FUNCTION__
                              << ": Error: Why on earth is this "
                                 "transaction number ("
                              << lPaymentTransNum
                              << ") on an outgoing payment instrument, if it's "
                                 "still on my "
                                 "'Tentative' list? If I haven't even signed "
                                 "out that number, how did I send "
                                 "an instrument to someone else with that "
                                 "number on it?\n";
                        return false;
                    }
                    const bool bIsIssued =
                        payment_context->It().VerifyIssuedNumber(
                            lPaymentTransNum);

                    // If nymfile is the sender AND the payment instrument IS
                    // expired.
                    if (bIsExpired) {
                        if (bIsIssued)  // ...and if this number is still signed
                                        // out to nymfile...
                        {
                            // If the instrument is definitely expired, and its
                            // number is still issued to nymfile, and there's
                            // definitely no related chequeReceipts in the asset
                            // acocunt inbox, then I can DEFINITELY harvest it
                            // back. After all, it hasn't been used, and since
                            // it's expired, now it CAN'T be used. So might as
                            // well harvest the number back, since we've
                            // established that it's still signed out.
                            //
                            // You might ask, but what if there IS a
                            // chequeReceipt in the asset account inbox? How
                            // does that change things? The answer is, because
                            // the transaction # might still be signed out to
                            // me, even in a case where the payment instrument
                            // is expired, but a chequeReceipt still IS present!
                            // How so? Simple: I sent him the cheque, he cashed
                            // it. It's in my outpayments still, because his
                            // chequeReceipt is in my inbox still, and hasn't
                            // been processed out. Meanwhile I wait a few weeks,
                            // and then the instrument, meanwhile, expires. It's
                            // already been processed, so it doesn't matter that
                            // it's expired. But nonetheless, according to the
                            // dates affixed to it, it IS expired, and the
                            // receipt IS present. So this is clearly a
                            // realistic and legitimate case for our logic to
                            // take into account. When this happens, we should
                            // NOT harvest the transaction # back when recording
                            // the payment, because that number has been used
                            // already!
                            //
                            // That, in a nutshell, is why we have to load the
                            // inbox and see if that receipt's there, to MAKE
                            // SURE whether the instrument was negotiated
                            // already, before it expired, even though I haven't
                            // accepted the receipt and closed out the
                            // transaction # yet, because it impacts our
                            // decision of whether or not to harvest back the
                            // number.
                            //
                            // The below two statements are interpreted based on
                            // this logic (see comment for each.)
                            //
                            bShouldHarvestPayment = true;  // If NO
                            // chequeReceipt/paymentReceipt/finalReceipt
                            // in inbox, definitely SHOULD harvest
                            // back the trans # (since the cheque's expired and
                            // could never otherwise be closed out)...
                            bNeedToLoadAssetAcctInbox = true;  // ...But if
                            // chequeReceipt/paymentReceipt/finalReceipt IS in
                            // inbox, definitely should NOT harvest back the #
                            // (since it's already been used.)
                            //
                            // =====> Therefore bNeedToLoadAssetAcctInbox is a
                            // caveat, which OVERRIDES bShouldHarvestPayment.
                            // <=====
                        } else  // nymfile is sender, payment instrument IS
                                // expired, and most importantly: the
                                // transaction # is no longer signed out
                        {       // to nymfile. Normally the closing of the # (by
                            // accepting whatever its related receipt was)
                            // should have already removed the outpayment, so we
                            // are cleared here to go ahead and remove it.
                            //
                            bShouldHarvestPayment =
                                false;  // The # isn't signed out anymore, so we
                                        // don't want to harvest it (which would
                            // cause the wallet to try and use it again -- but
                            // we don't want that, since we can't be using
                            // numbers that aren't even signed out to us!)
                            bNeedToLoadAssetAcctInbox =
                                false;  // No need to check the inbox since the
                                        // # isn't even signed out anymore. Even
                                        // if
                            // some related receipt was in the inbox (for some
                            // non-cheque instrument, say) we'd still never need
                            // to harvest it back for re-use, since it's not
                            // even signed out to us anymore, and we can only
                            // use numbers that are signed out to us.
                        }
                    }     // if bIsExpired.
                    else  // Not expired. nymfile is the sender but the payment
                          // instrument is NOT expired.
                    {
                        // Remember that the transaction number is still signed
                        // out to me until I accept that chequeReceipt. So
                        // whether the receipt is there or not, the # will still
                        // be signed out to me. But if there's no receipt yet in
                        // my inbox, that means the cheque hasn't been cashed
                        // yet. And THAT means I still have time to cancel it. I
                        // can't just discard the payment instrument, since its
                        // trans# would still need to be harvested if it's not
                        // being cancelled (so as to get it closed out on some
                        // other instrument, presumably), but I can't just
                        // harvest a number when there's some instrument still
                        // floating around out there, with that same number
                        // already on it! I HAVE to cancel it.
                        //
                        // If I discard it without harvesting, and if the
                        // recipient never cashes it, then that transaction #
                        // will end up signed out to me FOREVER (bad thing.) So
                        // I would have to cancel it first, in order to discard
                        // it. The server's success reply to my cancel would be
                        // the proper time to discard the old outpayment. Until
                        // I see that, how do I know if I won't need it in the
                        // future, for harvesting back?
                        //
                        if (bIsIssued)  // If this number is still signed out to
                                        // nymfile...
                        {
                            // If the instrument is definitely NOT expired, and
                            // the transaction # definitely IS issued to ME,
                            // then I can't just harvest the numbers, since the
                            // original recipient could still come through and
                            // deposit that cheque.  So in this case, I would
                            // HAVE to cancel the transaction first, and then
                            // such cancellation could sign a new transaction
                            // statement with that # removed from my list of
                            // "signed out" numbers. Only then am I safe from
                            // the cheque being cashed by the recipient, and
                            // only then could I even think about harvesting the
                            // number back--that itself being unnecessary, since
                            // the transaction
                            // # would then be cancelled/closed and thus would
                            // eliminate any need of harvesting it (since now I
                            // will never use it.)
                            //
                            // Also, if I DON'T cancel it, then I don't want to
                            // remove it from the outpayments box, because it
                            // will be removed automatically whenever the cheque
                            // is eventually cashed, and until/unless that
                            // happens, I need to keep it around for potential
                            // harvesting or cancellation. Therefore I can't
                            // discard the instrument either--I need to keep it
                            // in the outpayments box for now, in case it's
                            // needed later.
                            //
                            otErr << OT_METHOD << __FUNCTION__
                                  << ": This outpayment isn't expired yet, and "
                                     "the transaction number "
                                     "("
                                  << lPaymentTransNum
                                  << ") is still signed out. "
                                     "(Skipping moving it to record box -- it "
                                     "will be moved automatically "
                                     "once you cancel the transaction or the "
                                     "recipient deposits it.)\n";
                            return false;
                        } else  // The payment is NOT expired yet, but most
                                // importantly, its transaction # is NOT signed
                                // out to nymfile anymore.
                        {       // Normally the closing of the # (by accepting
                            // whatever its related receipt was) should have
                            // already removed the outpayment by now, so we are
                            // cleared here to go ahead and remove it.
                            //
                            bShouldHarvestPayment =
                                false;  // The # isn't signed out anymore, so we
                                        // don't want to harvest it (which would
                            // cause the wallet to try and use it again.)
                            bNeedToLoadAssetAcctInbox =
                                false;  // No need to check the inbox since the
                                        // # isn't even signed out anymore and
                            // we're certainly not interested in harvesting it
                            // if it's not even signed
                            // out to us.
                        }
                    }  // !bIsExpired
                }      // sender is nymfile

                // TODO: Add OPTIONAL field to Purse: "Remitter". This way the
                // sender has the OPTION to attach his ID "for the record" even
                // though a cash transaction HAS NO "SENDER."
                //
                // This would make it convenient for a purse to, for example,
                // create a second copy of the cash, encrypted to the remitter's
                // public key. This is important, since the if the remitter has
                // the cash in his outpayment's box, he will want a way to
                // recover it if his friend returns and says, "I lost that USB
                // key! Do you still have that cash?!?"
                //
                // In fact we may want to use the field for that purpose,
                // WHETHER OR NOT the final sent instrument actually includes
                // the remitter's ID.
                //
                // If the SenderNymID on this instrument isn't Nym's ID (as in
                // the case of vouchers), or isn't even there (as in the case of
                // cash) then why is it in Nym's payment outbox? Well, maybe the
                // recipient of your voucher, lost it. You still need to be able
                // to get another copy for him, or get it refunded (if you are
                // listed as the remitter...) Therefore you keep it in your
                // outpayments box "just in case." In the case of cash, the same
                // could be true.
                //
                // In a way it doesn't matter, since eventually those
                // instruments will expire and then they will be swept into the
                // record box with everything else (probably by this function.)
                //
                // But what if the instruments never expire? Say a voucher with
                // a very very std::int64_t expiration date? It's still going to
                // sit there, stuck in your outpayments box, even though the
                // recipient have have cashed it std::int64_t, std::int64_t ago!
                // The only way to get rid of it is to have the server send you
                // a notice when it's cashed, which is only possible if your ID
                // is listed as the remitter. (Otherwise the server wouldn't
                // know who to send the notice to.)
                //
                // Further, there's no PROVING whether the server sent you
                // notice for anything -- whether you are listed as the remitter
                // or not, the server could choose to LIE and just NOT TELL YOU
                // that the voucher was cashed. How would you know the
                // difference? Thus "Notice" is an important problem, peripheral
                // to OT. Balances are safe from any change without a proper
                // signed and authorized receipt--that is a powerful strength of
                // OT--but notice cannot be proven.
                //
                // If the remitter has no way to prove that the recipient
                // actually deposited the cheque, (even though most servers will
                // of course provide this, they cannot PROVE that they provided
                // it) then what good is the instrument to the remitter? What
                // good is such a cashier's cheque? Well, the voucher is still
                // in my outpayments, so I can see the transaction number on it,
                // and then I should be able to query the server and see which
                // transaction numbers are signed out to it. In which case a
                // simple server message (available to all users) will return
                // the numbers signed out to the server and thus I can see
                // whether or not the number on the voucher is still valid. If
                // it's not, the server reply to that message would be the
                // appropriate place to move the outpayment to the record box.
                //
                // What if we don't want to have these transaction numbers
                // signed out to the server at all? Maybe we use numbers signed
                // out to the users instead. Then when the voucher is cashed,
                // instead of checking the server's list of issued transaction
                // numbers to see if the voucher is valid, we would be checking
                // the remitter's list instead. And thus whenever the voucher is
                // cashed, we would have to drop a voucherReceipt in the
                // REMITTER's inbox (and nymbox) so he would know to discard the
                // transaction number.
                //
                // This would require adding the voucherReceipt, and would
                // restrict the use of vouchers to those people who have an
                // asset account (though that's currently the only people who
                // can use them now anyway, since vouchers are withdrawn from
                // accounts) but it would eliminate the need for the server nym
                // to store all the transaction numbers for all the open
                // vouchers, and it would also eliminate the problem of "no
                // notice" for the remitters of vouchers. They'd be guaranteed,
                // in fact, to get notice, since the voucherReceipt is what
                // removes the transaction number from the user's list of
                // signed-out transaction numbers (and thus prevents anyone from
                // spending the voucher twice, which the server wants to avoid
                // at all costs.) Thus if the server wants to avoid having a
                // voucher spent twice, then it needs to get that transaction
                // number off of my list of numbers, and it can't do that
                // without putting a receipt in my inbox to justify the removal
                // of the number. Otherwise my receipt verifications will start
                // failing and I'll be unable to do any new transactions, and
                // then the server will have to explain why it removed a
                // transaction number from my list, even though it still was on
                // my list during the last transaction statement, and even
                // though there's no new receipt in my inbox to justify removing
                // it.
                //
                // Conclusion, DONE: vouchers WITH a remitter acct, should store
                // the remitter's user AND acct IDs, and should use a
                // transaction # that's signed out to the remitter (instead of
                // the server) and should drop a voucherReceipt in the
                // remitter's asset account inbox when they are cashed. These
                // vouchers are guaranteed to provide notice to the remitter.
                //
                // Whereas vouchers WITHOUT a remitter acct should store the
                // remitter's user ID (or not), but should NOT store the
                // remitter's acct ID, and should use a transaction # that's
                // signed out to the server, and should drop a notice in the
                // Nymbox of the remitter IF his user ID is available, but it
                // should be understood that such notice is a favor the server
                // is doing, and not something that's PROVABLE as in the case of
                // the vouchers in the above paragraph.
                //
                // This is similar to smart contracts, which can only be
                // activated by a party who has an asset account, so he has
                // somewhere to receive the finalReceipt for that contract. (And
                // thus close out the transcation number he used to open it...)
                //
                // Perhaps we'll just offer both types of vouchers, and just let
                // users choose which they are willing to pay for, and which
                // trade-off is most palatable to them (having to have an asset
                // account to get notice, or instead verifying the voucher's
                // spent status based on some publicly-available listing of the
                // transaction #'s currently signed out to the server.)
                //
                // In the case of having transaction #'s signed out to the
                // server, perhaps the server's internal storage of these should
                // be paired each with the NymID of the owner nym for that
                // number, just so no one would ever have any incentive to try
                // and use one of those numbers on some instrument somehow, and
                // also so that the server wouldn't necessarily have to post the
                // entire list of numbers, but just rather give you the ones
                // that are relevant to you (although the entire list may have
                // to be posted in some public way in any case, for notice
                // reasons.)
                //
                // By this point, you may be wondering, but what does all this
                // have to do with the function we're in now? Well... in the
                // below block, with a voucher, nymfile would NOT be the sender.
                // The voucher would still be drawn off a server account. But
                // nevertheless, nymfile might still be the owner of the
                // transaction # for that voucher (assuming I change OT around
                // to use the above system, which I will have to do if I want
                // provable notice for vouchers.) And if nymfile is the owner of
                // that number, then he will want the option later of refunding
                // it or re-issuing it, and he will have to possibly load up his
                // inbox to make sure there's no voucherReceipt in it, etc. So
                // for now, the vouchers will be handled by the below code, but
                // in the future, they might be moved to the above code.
                //
                else  // nymfile is not the sender.
                {
                    // nymfile isn't even the "sender" (although he is) but
                    // since it's cash or voucher, he's not waiting on any
                    // transaction number to close out or be harvested. (In the
                    // case of cash, in fact, we might as well just put it
                    // straight in the records and bypass the outpayments box
                    // entirely.) But this function can sweep it into there all
                    // in due time anyway, once it expires, so it seems harmless
                    // to leave it there before then. Plus, that way we always
                    // know which tokens are still potentially exchangeable, for
                    // cases where the recipient never cashed them.
                    //
                    if (bIsExpired) {
                        // nymfile is NOT the sender AND the payment instrument
                        // IS expired. Therefore, say in the case of sent
                        // vouchers and sent cash, there may have been some
                        // legitimate reason for keeping them in outpayments up
                        // until this point, but now that the instrument is
                        // expired, might as well get it out of the outpayments
                        // box and move it to the record box. Let the client
                        // software do its own historical archiving.
                        //
                        bShouldHarvestPayment =
                            false;  // The # isn't signed out to nymfile, so we
                                    // don't want to harvest it (which would
                        // cause the wallet to try and use it even though it's
                        // signed out to someone else -- bad.)
                        bNeedToLoadAssetAcctInbox =
                            false;  // No need to check the inbox since the #
                        // isn't even signed out to nymfile and we're
                        // certainly not interested in harvesting it if it's not
                        // even signed out to us.
                        // (Thus no reason to check the inbox.)
                    } else  // nymfile is NOT the sender and the payment
                            // instrument is NOT expired.
                    {
                        // For example, for a sent voucher that has not
                        // expired yet. Those we'll keep here for now, until
                        // some server notice is received, or the instrument
                        // expires. What if we need to re-issue the cheque to
                        // the recipient who lost it? Or what if we want to
                        // cancel it before he tries to cash it? Since it's
                        // not expired yet, it's wise to keep a copy in the
                        // outpayments box for now.
                        //
                        otErr << OT_METHOD << __FUNCTION__
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
                // to load the asset account inbox and make sure there aren't
                // any chequeReceipts there, before we go ahead and harvest any
                // transaction numbers.
                //
                if (bNeedToLoadAssetAcctInbox &&
                    (bFromAcctIsAvailable || bIsRecurring)) {
                    bool bIsSmartContract = false;
                    if (bIsRecurring) {
                        std::unique_ptr<OTTrackable> pTrackable(
                            thePayment.Instantiate());

                        if (!pTrackable) {
                            String strPaymentContents;
                            thePayment.GetPaymentContents(strPaymentContents);
                            otErr << OT_METHOD << __FUNCTION__
                                  << ": Failed instantiating "
                                     "OTPayment containing:\n"
                                  << strPaymentContents << "\n";
                            return false;
                        }
                        pSmartContract =
                            dynamic_cast<OTSmartContract*>(pTrackable.get());
                        pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable.get());

                        if (nullptr != pSmartContract) {
                            bIsSmartContract = true;  // In this case we have to
                                                      // loop through all the
                            // accounts on the smart contract...
                        } else if (nullptr != pPlan) {
                            // Payment plan is a funny case.
                            // The merchant (RECIPIENT aka payee) creates the
                            // payment plan, and then he SENDS it to the
                            // customer (SENDER aka payer). From there, the
                            // customer ACTIVATES it on the server. BOTH could
                            // potentially have it in their outpayments box. In
                            // one case. the Nym is the "sender" and in another
                            // case, he's the "recipient." (So we need to figure
                            // out which, and set the account accordingly.)
                            //
                            if (NYM_ID == pPlan->GetRecipientNymID())
                                theSenderAcctID = pPlan->GetRecipientAcctID();
                            else if (NYM_ID == pPlan->GetSenderNymID())
                                theSenderAcctID = pPlan->GetSenderAcctID();
                            else
                                otErr << OT_METHOD << __FUNCTION__
                                      << ": ERROR: Should never happen: NYM_ID "
                                         "didn't match this payment plan for "
                                         "sender OR recipient. "
                                         "(Expected one or the other.)\n";
                        }
                    }
                    if (bIsSmartContract)  // In this case we have to loop
                                           // through all the accounts on the
                                           // smart contract... We have to
                    {  // check the inbox on each, to make sure there aren't any
                        // related paymentReceipts or final receipts.
                        const std::int32_t nPartyCount =
                            pSmartContract->GetPartyCount();

                        for (std::int32_t nCurrentParty = 0;
                             nCurrentParty < nPartyCount;
                             ++nCurrentParty) {
                            OTParty* party =
                                pSmartContract->GetPartyByIndex(nCurrentParty);
                            OT_ASSERT(nullptr != party);

                            if (nullptr != party) {
                                const std::int32_t nAcctCount =
                                    party->GetAccountCount();

                                for (std::int32_t nCurrentAcct = 0;
                                     nCurrentAcct < nAcctCount;
                                     ++nCurrentAcct) {
                                    OTPartyAccount* partyAcct =
                                        party->GetAccountByIndex(nCurrentAcct);
                                    OT_ASSERT(nullptr != partyAcct);

                                    if (nullptr != partyAcct) {
                                        OTAgent* agent =
                                            partyAcct->GetAuthorizedAgent();

                                        // If nymfile is a signer for partyAcct,
                                        // then we need to check partyAcct's
                                        // inbox to make sure there aren't any
                                        // paymentReceipts or finalReceipts
                                        // lingering in there...
                                        //
                                        if (agent->IsValidSigner(
                                                *payment_nym)) {
                                            const String& strAcctID =
                                                partyAcct->GetAcctID();
                                            const auto accountID =
                                                Identifier::Factory(strAcctID);

                                            Ledger theSenderInbox(
                                                NYM_ID,
                                                accountID,
                                                paymentNotaryId);

                                            const bool
                                                bSuccessLoadingSenderInbox =
                                                    (theSenderInbox
                                                         .LoadInbox() &&
                                                     theSenderInbox
                                                         .VerifyAccount(
                                                             *payment_nym));

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
                                        }  // nymfile is valid signer for agent
                                    }      // nullptr != partyAccount
                                }          // loop party accounts.

                                if (bFoundReceiptInInbox) break;
                            }
                        }  // loop parties
                    }      // smart contract
                    else   // not a smart contract. (It's a payment plan or a
                           // cheque, most likely.)
                    {
                        Ledger theSenderInbox(
                            NYM_ID, theSenderAcctID, paymentNotaryId);

                        const bool bSuccessLoadingSenderInbox =
                            (theSenderInbox.LoadInbox() &&
                             theSenderInbox.VerifyAccount(*payment_nym));

                        if (bSuccessLoadingSenderInbox) {
                            // Loop through the inbox and see if there are any
                            // receipts for lPaymentTransNum inside.
                            // Technically this would have to be a
                            // chequeReceipt, or possibly a voucherReceipt if
                            // I add that (see giant comment above.)
                            //
                            // There are other instrument types but only a
                            // cheque, at this point, would be in my
                            // outpayments box AND could have a receipt in my
                            // asset account inbox. So let's see if there's a
                            // chequeReceipt in there that corresponds to
                            // lPaymentTransNum...
                            //
                            OTTransaction* pChequeReceipt =
                                theSenderInbox.GetChequeReceipt(
                                    lPaymentTransNum);  // cheque

                            if (nullptr != pChequeReceipt) {
                                bFoundReceiptInInbox = true;
                            }

                            // No cheque receipt? Ok let's see if there's a
                            // paymentReceipt or finalReceipt (for a payment
                            // plan...)
                            else if (
                                GetPaymentReceipt(
                                    theSenderInbox.GetTransactionMap(),
                                    lPaymentTransNum,
                                    nullptr) ||
                                theSenderInbox.GetFinalReceipt(
                                    lPaymentTransNum))  // payment plan.
                            {
                                bFoundReceiptInInbox = true;
                            }
                            // else we didn't find a receipt in the asset
                            // account inbox, which means we are safe to
                            // harvest.
                        }
                        // else unable to load inbox. Maybe it's empty, never
                        // been used before. i.e. it doesn't even exist.
                    }  // not a smart contract
                }  // if (bNeedToLoadAssetAcctInbox && (bFromAcctIsAvailable ||
                   // bIsRecurring))

                // If we should harvest the transaction numbers, AND if we don't
                // need to double-check that against the asset inbox to make
                // sure the receipt's not there, (or if we do, that it was a
                // successful double-check and the receipt indeed is not there.)
                //
                if (bShouldHarvestPayment &&
                    ((!bNeedToLoadAssetAcctInbox) ||
                     (bNeedToLoadAssetAcctInbox && !bFoundReceiptInInbox))) {
                    // Harvest the transaction number(s).
                    //
                    if (nullptr != pSmartContract) {
                        pSmartContract->HarvestOpeningNumber(
                            payment_context->It());
                        pSmartContract->HarvestClosingNumbers(
                            payment_context->It());
                    } else if (nullptr != pPlan) {
                        pPlan->HarvestOpeningNumber(payment_context->It());
                        pPlan->HarvestClosingNumbers(payment_context->It());
                    } else {
                        payment_context->It().RecoverAvailableNumber(
                            lPaymentTransNum);
                    }

                    // Note, food for thought: IF the receipt had popped into
                    // your asset inbox on the server side, since the last
                    // time you downloaded your inbox, then you could be
                    // making the wrong decision here, and harvesting a number
                    // that's already spent. (You just didn't know it yet.)
                    //
                    // What happens in that case, when I download the inbox
                    // again? A new receipt is there, and a transaction # is
                    // used, which I thought was still available for use?
                    // (Since I harvested it?) The appearance of the receipt
                    // in the inbox, as long as properly formed and signed by
                    // me, should be enough information for the client side to
                    // adjust its records, because if it doesn't anticipate
                    // this possibility, then it will be forced to resync
                    // entirely, which I want to avoid in all cases period.
                    //
                    // In this block, we clawed back the number because if
                    // there's no chequeReceipt in the inbox, then that means
                    // the cheque has never been used, since if I had closed
                    // the chequeReceipt out already, then the transaction
                    // number on that cheque would already have been closed
                    // out at that time. We only clawback for expired
                    // instruments, where the transaction # is still
                    // outstanding and where no receipt is present in the
                    // asset acct inbox. If the instrument is not expired,
                    // then you must cancel it properly. And if the cheque
                    // receipt is in the inbox, then you must close it
                    // properly.
                }
            }  // if (thePayment.GetTransactionNum(lPaymentTransNum))
            else if (
                bSaveCopy && (nullptr != pActualBox) && thePayment.IsPurse()) {
                OT_ASSERT(nullptr != pActualBox);

                // A purse has no transaction number on itself, and if it's in
                // the outpayment box, it has no transaction number from its
                // ledger, either! It's numberless. So what we do for now is,
                // we use the expiration timestamp for the purse to create its
                // "transaction number" (we also add a billion to it, and then
                // increment it until we are sure it's unused in the box
                // already) for the new receipt we're inserting for this purse
                // into the record box. (Otherwise we'd have to skip this part
                // and we wouldn't save a record of the purse at all.)
                //
                // ALSO NOTE that people shouldn't really be discarding the
                // purse anyway from the outpayment box, since it will ALREADY
                // disappear once it expires. (Presumably you'd want the
                // option to recover it, at any time prior to that point...)
                // But we still must consider that people sent cash and they
                // just want it erased, so...
                //
                time64_t tValidTo = OT_TIME_ZERO;

                if (thePayment.GetValidTo(tValidTo)) {
                    lPaymentTransNum = OTTimeGetSecondsFromTime(tValidTo) +
                                       1000000000;  // todo hardcoded. (But
                                                    // should be harmless since
                                                    // this is record box.)

                    // Since we're using a made-up transaction number here,
                    // let's make sure it's not already being used. If it is,
                    // we'll increment it until nothing is found.
                    //
                    while (nullptr !=
                           pActualBox->GetTransaction(lPaymentTransNum))
                        ++lPaymentTransNum;
                } else
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Failed trying to get 'valid to' from purse.\n";
            }
            // Create the notice to put in the Record Box.
            //
            if (bSaveCopy && (nullptr != pActualBox) &&
                (lPaymentTransNum > 0)) {
                OT_ASSERT(nullptr != pActualBox);

                OTTransaction* pNewTransaction =
                    OTTransaction::GenerateTransaction(
                        *pActualBox,
                        OTTransaction::notice,
                        theOriginType,
                        lPaymentTransNum);

                if (nullptr != pNewTransaction)  // The above has an OT_ASSERT
                                                 // within, but I just like to
                                                 // check my pointers.
                {
                    pNewTransaction->SetReferenceToNum(
                        lPaymentTransNum);  // referencing myself here. We'll
                                            // see how it works out.
                    pNewTransaction->SetReferenceString(
                        strInstrument);  // the cheque, invoice, etc that used
                                         // to be in the outpayments box.

                    pNewTransaction->SignContract(*transport_nym);
                    pNewTransaction->SaveContract();
                    transaction = pNewTransaction;

                    theTransactionAngel.reset(transaction);
                } else  // should never happen
                {
                    const String strNymID(NYM_ID);
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Failed while trying to generate transaction in "
                             "order to "
                             "add a new transaction (for a payment instrument "
                             "from the outpayments box) "
                             "to Record Box: "
                          << strNymID << "\n";
                }
            }
        }  // if (thePayment.IsValid() && thePayment.SetTempValues())
        //
        // Now we actually remove the message from the outpayments...
        //
        bRemoved = transport_nymfile.It().RemoveOutpaymentsByIndex(
            nIndex, false);  // bDeleteIt=true by default
        theMessageAngel.reset(
            pMessage);  // Since we chose to keep pMessage alive after removing
        // it from the outpayments, we set the angel here to make
        // sure it gets cleaned up later whenever we return out
        // of this godforsaken function.
        // Anything else?
    }  // outpayments box.
    //
    // Okay by this point, whether the payment was in the payments inbox, or
    // whether it was in the outpayments box, either way, it has now been
    // removed from that box. (Otherwise we would have returned already by this
    // point.)
    //
    // It's still safer to explicitly check bRemoved, just in case.
    //
    if (bRemoved) {
        if (bSaveCopy && (nullptr != pActualBox) && (nullptr != transaction)) {
            OT_ASSERT(nullptr != pActualBox);

            const bool bAdded = pActualBox->AddTransaction(*transaction);

            if (!bAdded) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to add transaction "
                      << transaction->GetTransactionNum()
                      << " to record box (after tentatively removing from "
                         "payment "
                      << (bIsInbox ? "inbox" : "outbox")
                      << ", an action that is now canceled.)\n";
                return false;
            } else {
                // If successfully added to the record box, then no need
                // anymore to clean it up ourselves.
                theTransactionAngel.release();
            }

            pActualBox->ReleaseSignatures();
            pActualBox->SignContract(*transport_nym);
            pActualBox->SaveContract();
            if (bIsExpired)
                pActualBox->SaveExpiredBox();  // todo log failure.
            else
                pActualBox->SaveRecordBox();  // todo log failure.

            // Any inbox/nymbox/outbox ledger will only itself contain
            // abbreviated versions of the receipts, including their hashes.
            //
            // The rest is stored separately, in the box receipt, which is
            // created whenever a receipt is added to a box, and deleted after a
            // receipt is removed from a box.
            //
            transaction->SaveBoxReceipt(*pActualBox);  // todo: log failure
        }
        if (bIsInbox) {
            pPaymentInbox->ReleaseSignatures();
            pPaymentInbox->SignContract(*transport_nym);
            pPaymentInbox->SaveContract();

            const bool bSavedInbox =
                pPaymentInbox->SavePaymentInbox();  // todo: log failure

            if (!bSavedInbox)
                otErr << "/n" << __FUNCTION__
                      << ": Unable to Save PaymentInbox./n";
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to remove from payment "
              << (bIsInbox ? "inbox" : "outbox") << " based on index " << nIndex
              << ".\n";
        return false;
    }

    return true;
}

// Notice that since the Nym ID is sometimes also passed as the account ID
// (in the case of Nym recordbox, versus Account recordbox...) which means
// sometimes the Notary ID is the notary where a payment instrument was
// transported, versus being a notary where a payment instrument was deposited.
// (Often a different notary).
//
//
bool OT_API::ClearRecord(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& ACCOUNT_ID,  // NYM_ID can be passed here as well.
    std::int32_t nIndex,
    bool bClearAll) const  // if true, nIndex is ignored.
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return false;
    }

    auto nym = context->Nym();
    std::unique_ptr<Ledger> pRecordBox(
        LoadRecordBox(NOTARY_ID, NYM_ID, ACCOUNT_ID));

    if (pRecordBox) {
        pRecordBox.reset(Ledger::GenerateLedger(
            NYM_ID, ACCOUNT_ID, NOTARY_ID, Ledger::recordBox, true));

        if (pRecordBox) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to load or create record box "
                     "(and thus unable to do anything with it.)\n";
            return false;
        }
    }

    if (bClearAll) {
        pRecordBox->ReleaseTransactions();
        pRecordBox->ReleaseSignatures();
        pRecordBox->SignContract(*nym);
        pRecordBox->SaveContract();
        pRecordBox->SaveRecordBox();
        return true;
    }
    // Okay, it's not "clear all" but "clear at index" ...
    //
    const std::int32_t nTransCount = pRecordBox->GetTransactionCount();

    if ((nIndex < 0) || (nIndex >= nTransCount)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Index out of bounds (highest allowed index for "
                 "this record box is "
              << nTransCount - 1 << ".)\n";
        return false;
    }
    OTTransaction* transaction = pRecordBox->GetTransactionByIndex(nIndex);
    bool bRemoved = false;

    if (nullptr != transaction) {
        const std::int64_t lTransactionNum = transaction->GetTransactionNum();

        if (!pRecordBox->DeleteBoxReceipt(lTransactionNum)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed trying to delete the box receipt for a "
                     "transaction being removed from a record box: "
                  << lTransactionNum << "\n";
        }
        bRemoved = pRecordBox->RemoveTransaction(lTransactionNum);
    }
    if (bRemoved) {
        pRecordBox->ReleaseSignatures();
        pRecordBox->SignContract(*nym);
        pRecordBox->SaveContract();
        pRecordBox->SaveRecordBox();
        return true;
    } else {
        const std::int32_t nTemp = nIndex;
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to clear a record from the record "
                 "box at index: "
              << nTemp << "\n";
    }
    return false;
}

// This function assumes you have already downloaded the latest copy of your
// Nymbox,
// and that you have already retrieved theMessageNym from the
// registerNymResponse
// message, with both objects being loaded and passed as arguments here, ready
// to go.
//
bool OT_API::ResyncNymWithServer(
    NymFile& theNym,
    const Ledger& theNymbox,
    const Nym& theMessageNym) const
{
    return false;

    // rLock lock(lock_);

    if (Ledger::nymbox != theNymbox.GetType()) {
        otErr << "OT_API::ResyncNymWithServer: Error: Expected a Nymbox, "
                 "but you passed in a "
              << theNymbox.GetTypeString() << ".\n";
        return false;
    }
    if (!theNym.CompareID(theNymbox.GetNymID())) {
        const String id1(theNym.GetConstID()), id2(theNymbox.GetNymID());
        otErr << "OT_API::ResyncNymWithServer: Error: NymID of Nym (" << id1
              << ") "
                 "doesn't match NymID on (supposedly) his own Nymbox "
                 "("
              << id2 << ").\n";
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

Message* OT_API::GetSentMessage(
    const std::int64_t& lRequestNumber,
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    OT_ASSERT_MSG(
        (m_pClient != nullptr), "Not initialized; call OT_API::Init first.");
    OT_ASSERT_MSG(
        lRequestNumber > 0,
        "OT_API::GetSentMessage: lRequestNumber is less than 1.");

    const String strNotaryID(NOTARY_ID), strNymID(NYM_ID);

    return m_pClient->GetMessageOutbuffer().GetSentMessage(
        lRequestNumber, strNotaryID, strNymID);  // doesn't delete.
}

bool OT_API::RemoveSentMessage(
    const std::int64_t& lRequestNumber,
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    OT_ASSERT_MSG(
        m_pClient != nullptr, "Not initialized; call OT_API::Init first.");
    OT_ASSERT_MSG(
        lRequestNumber > 0,
        "OT_API::RemoveSentMessage: lRequestNumber is less than 1.");

    const String strNotaryID(NOTARY_ID), strNymID(NYM_ID);

    return m_pClient->GetMessageOutbuffer().RemoveSentMessage(
        lRequestNumber, strNotaryID, strNymID);  // deletes.
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
// Therefore this function will be called only after getNymboxResponse (in
// OTClient),
// where each
// replyNotice in the Nymbox will be removed from the sent messages
// individually, and then
// the rest of the sent messages can be flushed
//
//
// OT_API::FlushSentMessages
//
// Make sure to call this directly after a successful getNymboxResponse.
// (And ONLY at that time.)
//
// This empties the buffer of sent messages.
// (Harvesting any transaction numbers that are still there.)
//
// NOTE: You normally ONLY call this immediately after receiving
// a successful getNymboxResponse. It's only then that you can see which
// messages a server actually received or not -- which transactions
// it processed (success or fail) vs which transactions did NOT
// process (and thus did NOT leave any success/fail receipt in the
// nymbox.)
//
// I COULD have just flushed myself IN the getNymboxResponse code (where
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
// to call this function after a successful getNymboxResponse!
//
void OT_API::FlushSentMessages(
    bool bHarvestingForRetry,
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Ledger& THE_NYMBOX) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.mutable_ServerContext(NYM_ID, NOTARY_ID);
    auto nym = wallet_.Nym(NYM_ID);

    if (false == bool(nym)) return;

    const String strNotaryID(NOTARY_ID), strNymID(NYM_ID);

    if ((THE_NYMBOX.GetNymID() != NYM_ID) ||
        (THE_NYMBOX.GetPurportedNotaryID() != NOTARY_ID)) {
        const String strLedger(THE_NYMBOX);
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure, Bad input data: NymID (" << strNymID
              << ") or NotaryID "
                 "("
              << strNotaryID << ") failed to match Nymbox:\n\n"
              << strLedger << "\n\n";
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
        const OTTransaction* transaction = it.second;
        OT_ASSERT(nullptr != transaction);

        if (OTTransaction::replyNotice == transaction->GetType()) {
            Message* pMessage =
                m_pClient->GetMessageOutbuffer().GetSentMessage(*transaction);

            if (nullptr != pMessage)  // It WAS there in my sent buffer!
            {
                // Since it IS in my Nymbox already as a replyNotice,
                // therefore I'm safe to erase it from my sent queue.
                //
                m_pClient->GetMessageOutbuffer().RemoveSentMessage(
                    *transaction);
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
        strNotaryID,
        strNymID,
        bHarvestingForRetry,
        context.It(),
        nym->ID());  // FYI: This HARVESTS any sent messages that need
                     // harvesting, before flushing them all.
}

bool OT_API::HaveAlreadySeenReply(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const RequestNumber& lRequestNumber) const
{
    auto context = wallet_.ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << NYM_ID.str()
              << " is not registered on " << NOTARY_ID.str() << std::endl;

        return false;
    }

    return context->VerifyAcknowledgedNumber(lRequestNumber);
}

// IS BASKET CURRENCY ?
//
// Tells you whether or not a given instrument definition is actually a basket
// currency.
//
bool OT_API::IsBasketCurrency(const Identifier& BASKET_INSTRUMENT_DEFINITION_ID)
    const  // returns true or false.
{
    std::shared_ptr<proto::UnitDefinition> contract;

    bool loaded =
        storage_.Load(BASKET_INSTRUMENT_DEFINITION_ID.str(), contract, true);

    if (!loaded) {
        return false;
    }

    return (proto::UNITTYPE_BASKET == contract->type());
}

// Get Basket Count (of member currency types.)
//
// Returns the number of instrument definitions that make up this basket.
// (Or zero.)
//
std::int32_t OT_API::GetBasketMemberCount(
    const Identifier& BASKET_INSTRUMENT_DEFINITION_ID) const
{
    std::shared_ptr<proto::UnitDefinition> serialized;
    storage_.Load(BASKET_INSTRUMENT_DEFINITION_ID.str(), serialized, true);

    if (!serialized) {
        return 0;
    }

    if (proto::UNITTYPE_BASKET != serialized->type()) {
        return 0;
    }

    return serialized->basket().item_size();
}

// Get Basket Member Asset Type
//
// Returns one of the instrument definitions that make up this basket,
// by index, and true.
// (Or false.)
//
bool OT_API::GetBasketMemberType(
    const Identifier& BASKET_INSTRUMENT_DEFINITION_ID,
    std::int32_t nIndex,
    Identifier& theOutputMemberType) const
{
    std::shared_ptr<proto::UnitDefinition> serialized;
    storage_.Load(BASKET_INSTRUMENT_DEFINITION_ID.str(), serialized, true);

    if (!serialized) {
        return false;
    }

    if (proto::UNITTYPE_BASKET != serialized->type()) {
        return false;
    }

    if ((nIndex >= serialized->basket().item_size()) || (nIndex < 0)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Index out of bounds: " << nIndex << std::endl;

        return false;
    }

    theOutputMemberType =
        Identifier::Factory(serialized->basket().item(nIndex).unit());

    return true;
}

// Get Basket Member Minimum Transfer Amount
//
// Returns the minimum transfer amount for one of the instrument definitions
// that
// makes up this basket, by index.
// (Or 0.)
//
std::int64_t OT_API::GetBasketMemberMinimumTransferAmount(
    const Identifier& BASKET_INSTRUMENT_DEFINITION_ID,
    std::int32_t nIndex) const
{
    std::shared_ptr<proto::UnitDefinition> serialized;
    storage_.Load(BASKET_INSTRUMENT_DEFINITION_ID.str(), serialized, true);

    if (!serialized) {
        return 0;
    }

    if (proto::UNITTYPE_BASKET != serialized->type()) {
        return 0;
    }

    if ((nIndex >= serialized->basket().item_size()) || (nIndex < 0)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Index out of bounds: " << nIndex << std::endl;
        return 0;
    }

    return serialized->basket().item(nIndex).weight();
}

// Get Basket Minimum Transfer Amount
//
// Returns the minimum transfer amount for the basket.
// (Or 0.)
//
std::int64_t OT_API::GetBasketMinimumTransferAmount(
    const Identifier& BASKET_INSTRUMENT_DEFINITION_ID) const
{
    std::shared_ptr<proto::UnitDefinition> serialized;
    storage_.Load(BASKET_INSTRUMENT_DEFINITION_ID.str(), serialized, true);

    if (!serialized) {
        return 0;
    }

    if (proto::UNITTYPE_BASKET != serialized->type()) {
        return 0;
    }

    return serialized->basket().weight();
}

// ADD BASKET CREATION ITEM
//
// Used for creating a request to generate a new basket currency.
bool OT_API::AddBasketCreationItem(
    proto::UnitDefinition& basketTemplate,
    const String& currencyID,
    const std::uint64_t weight) const
{
    auto item = basketTemplate.mutable_basket()->add_item();

    if (nullptr == item) {
        return false;
    }

    item->set_version(1);
    item->set_weight(weight);
    item->set_unit(currencyID.Get());

    return true;
}

// Create a basket account, which is like an issuer account, but based on a
// basket of other instrument definitions. This way, users can trade with what
// is apparently a single currency, when in fact the issuence is delegated and
// distributed across multiple issuers.
//
// The user signs and saves the contract, but once the server gets it, the
// server releases signatures and signs it, calculating the hash from the
// result, in order to form the ID.
//
// The result is the same as any other currency contract, but with the server's
// signature on it (and thus it must store the server's public key). The server
// handles all transactions in and out of the basket currency based upon the
// rules set up by the user.
//
// The user who created the currency has no more control over it. The server
// reserves the right to exchange out to the various users and close the basket.
CommandResult OT_API::issueBasket(
    ServerContext& context,
    const proto::UnitDefinition& basket) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::issueBasket,
        OTASCIIArmor(proto::ProtoAsData(basket)),
        Identifier::Factory(),
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

// GENERATE BASKET EXCHANGE REQUEST
//
// Creates a new request (for exchanging funds in/out of a Basket).
// (Each currency in this request will be added with
// subsequent calls to OT_API::GenerateBasketItem()).
//
// (Caller is responsible to delete.)
Basket* OT_API::GenerateBasketExchange(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& BASKET_INSTRUMENT_DEFINITION_ID,
    const Identifier& accountID,
    std::int32_t TRANSFER_MULTIPLE) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.mutable_ServerContext(NYM_ID, NOTARY_ID);
    auto nym = context.It().Nym();

    auto contract =
        GetBasketContract(BASKET_INSTRUMENT_DEFINITION_ID, __FUNCTION__);

    if (nullptr == contract) return nullptr;
    // By this point, contract is a good pointer, and is on the wallet. (No
    // need to cleanup.)
    auto account = GetOrLoadAccount(*nym, accountID, NOTARY_ID, __FUNCTION__);

    if (false == bool(account)) {

        return nullptr;
    }

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    if (BASKET_INSTRUMENT_DEFINITION_ID !=
        account->GetInstrumentDefinitionID()) {
        const String strAcctID(accountID),
            strAcctTypeID(BASKET_INSTRUMENT_DEFINITION_ID);
        otErr
            << "OT_API::GenerateBasketExchange: Wrong instrument definition ID "
               "on account "
            << strAcctID << " (expected type to be " << strAcctTypeID << ")\n";
        return nullptr;
    }
    // By this point, I know that everything checks out. Signature and Account
    // ID.
    // account is good, and no need to clean it up.
    String strNotaryID(NOTARY_ID);

    std::int32_t nTransferMultiple = 1;

    if (TRANSFER_MULTIPLE > 0) nTransferMultiple = TRANSFER_MULTIPLE;

    // Next load the Basket object out of that contract.
    Basket* pRequestBasket = nullptr;

    // We need a transaction number just to send this thing. Plus, we need a
    // number for each sub-account to the basket, as well as the basket's main
    // account. That is: 1 + theBasket.Count() + 1
    const std::size_t currencies = contract->Currencies().size();

    if (context.It().AvailableNumbers() < (2 + currencies)) {
        otErr << "OT_API::GenerateBasketExchange: you don't have "
                 "enough transaction numbers to perform the "
                 "exchange.\n";
    } else {
        pRequestBasket = new Basket(currencies, contract->Weight());
        OT_ASSERT_MSG(
            nullptr != pRequestBasket,
            "OT_API::GenerateBasketExchange: Error allocating "
            "memory in the OT API");

        pRequestBasket->SetTransferMultiple(
            nTransferMultiple);  // This stays in this function.

        // Make sure the server knows where to put my new basket currency
        // funds,
        // once the exchange is done.
        pRequestBasket->SetRequestAccountID(accountID);  // This stays too

        // Export the Basket object into a string, add it as
        // a payload on my request, and send to server.
        pRequestBasket->SignContract(*nym);
        pRequestBasket->SaveContract();
    }  // *nymfile apparently has enough transaction numbers to exchange the
    // basket.

    return pRequestBasket;
}

// ADD BASKET EXCHANGE ITEM
bool OT_API::AddBasketExchangeItem(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    Basket& theBasket,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& ASSET_ACCOUNT_ID) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.mutable_ServerContext(NYM_ID, NOTARY_ID);
    auto nym = context.It().Nym();

    auto contract = wallet_.UnitDefinition(INSTRUMENT_DEFINITION_ID);

    if (!contract) {
        return false;
    }

    auto account =
        GetOrLoadAccount(*nym, ASSET_ACCOUNT_ID, NOTARY_ID, __FUNCTION__);

    if (false == bool(account)) {

        return false;
    }

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    if (context.It().AvailableNumbers() < 1) {
        otErr << "OT_API::AddBasketExchangeItem: you need at least one "
                 "transaction number to add this exchange item.\n";
        return false;
    }
    if (INSTRUMENT_DEFINITION_ID != account->GetInstrumentDefinitionID()) {
        const String strInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID),
            strAcctID(ASSET_ACCOUNT_ID);
        otErr
            << "OT_API::AddBasketExchangeItem: Wrong instrument definition ID "
               "on account "
            << strAcctID << " (expected to find instrument definition "
            << strInstrumentDefinitionID << ")\n";
        return false;
    }
    // By this point, I know that everything checks out. Signature and Account
    // ID.
    // account is good, and no need to clean it up.
    const String strNotaryID(NOTARY_ID);
    const auto number =
        context.It().NextTransactionNumber(MessageType::notarizeTransaction);

    if (false == number.Valid()) {
        otErr << "OT_API::AddBasketExchangeItem: Failed getting next "
                 "transaction number. \n";

        return false;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << number << std::endl;
    number.SetSuccess(true);
    theBasket.AddRequestSubContract(
        INSTRUMENT_DEFINITION_ID, ASSET_ACCOUNT_ID, number);
    theBasket.ReleaseSignatures();
    theBasket.SignContract(*nym);
    theBasket.SaveContract();

    return true;
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
    before even having a chance to call the NotarizeTransaction code.

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
CommandResult OT_API::exchangeBasket(
    ServerContext& context,
    const Identifier& BASKET_INSTRUMENT_DEFINITION_ID,
    const String& BASKET_INFO,
    bool bExchangeInOrOut  // exchanging in == true, out == false.
    ) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    auto contract =
        GetBasketContract(BASKET_INSTRUMENT_DEFINITION_ID, __FUNCTION__);

    if (nullptr == contract) {

        return output;
    }

    Basket basket;
    bool validBasket = (0 < BASKET_INFO.GetLength());
    validBasket &= basket.LoadContractFromString(BASKET_INFO);

    if (false == validBasket) {

        return output;
    }

    const Identifier& accountID(basket.GetRequestAccountID());

    auto account = GetOrLoadAccount(nym, accountID, serverID, __FUNCTION__);

    if (nullptr == account) {

        return output;
    }

    std::unique_ptr<Ledger> inbox(account->LoadInbox(nym));

    if (false == bool(inbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading inbox for "
              << "account " << String(accountID) << std::endl;

        return output;
    }

    std::unique_ptr<Ledger> outbox(account->LoadOutbox(nym));

    if (nullptr == outbox) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading outbox for "
              << "account " << String(accountID) << std::endl;

        return output;
    }

    // We need a transaction number just to send this thing. Plus, we need a
    // number for each sub-account to the basket, as well as the basket's main
    // account. That is: 1 + theBasket.Count() + 1 Total of 2, since
    // theBasket.Count() worth of numbers were already added in the calls to
    // OT_API::AddBasketExchangeItem.

    if (context.AvailableNumbers() < 2) {
        otErr << OT_METHOD << __FUNCTION__ << ": you don't have enough "
              << "transaction numbers to perform the exchange." << std::endl;
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    std::set<ServerContext::ManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber.Valid()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": No transaction numbers were available. "
                 "Try requesting the server for a new one.\n";
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << managedNumber << std::endl;
    transactionNum = managedNumber;
    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            accountID,
            serverID,
            OTTransaction::exchangeBasket,
            originType::not_applicable,
            transactionNum));

    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(
        Item::CreateItemFromTransaction(*transaction, Item::exchangeBasket));

    if (false == bool(item)) {

        return output;
    }

    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& closingNumber = *managed.rbegin();

    if (false == closingNumber.Valid()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": No transaction numbers were available. "
                 "Try requesting the server for a new one.\n";

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << closingNumber << std::endl;
    basket.SetClosingNum(closingNumber);
    basket.SetExchangingIn(bExchangeInOrOut);
    basket.ReleaseSignatures();
    basket.SignContract(nym);
    basket.SaveContract();
    String strBasketInfo;
    basket.SaveContractRaw(strBasketInfo);
    item->SetAttachment(strBasketInfo);
    item->SignContract(nym);
    item->SaveContract();
    transaction->AddItem(*item.release());
    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        0, *transaction, context, *account, *outbox));

    if (false == bool(balanceItem)) {

        return output;
    }

    transaction->AddItem(*balanceItem.release());
    transaction->SignContract(nym);
    transaction->SaveContract();
    Ledger ledger(nymID, accountID, serverID);
    ledger.GenerateLedger(accountID, serverID, Ledger::message);
    ledger.AddTransaction(*transaction.release());
    ledger.SignContract(nym);
    ledger.SaveContract();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message(managed, context, *message);

    return output;
}

CommandResult OT_API::getTransactionNumbers(ServerContext& context) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto nCount = context.AvailableNumbers();
    // TODO no hardcoding. (max transaction nums allowed out at a single time.)
    const std::size_t nMaxCount = 50;

    if (nCount > nMaxCount) {
        otErr << "OT_API::getTransactionNumbers: Failure: That Nym already has "
              << "more than " << nMaxCount << " transaction numbers signed out."
              << " (Use those first.)" << std::endl;
        requestNum = 1;
        status = SendResult::UNNECESSARY;

        return output;
    }

    Message message;
    requestNum = m_pClient->ProcessUserCommand(
        MessageType::getTransactionNumbers, context, message);

    if (1 > requestNum) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error processing "
              << "getTransactionNumber command. Return value: " << requestNum
              << std::endl;

        return output;
    }

    result = send_message({}, context, message);

    return output;
}

#if OT_CASH
CommandResult OT_API::notarizeWithdrawal(
    ServerContext& context,
    const Identifier& accountID,
    const Amount amount) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    const auto& serverNym = context.RemoteNym();
    const auto& serverNymID = serverNym.ID();
    auto wallet = GetWallet(__FUNCTION__);

    if (nullptr == wallet) {

        return output;
    }

    auto account = GetOrLoadAccount(nym, accountID, serverID, __FUNCTION__);

    if (nullptr == account) {

        return output;
    }

    const auto& contractID = account->GetInstrumentDefinitionID();
    const bool exists =
        OTDB::Exists(OTFolders::Mint().Get(), serverID.str(), contractID.str());

    if (false == exists) {
        otErr << OT_METHOD << __FUNCTION__
              << ": File does not exist: " << OTFolders::Mint()
              << Log::PathSeparator() << String(serverID)
              << Log::PathSeparator() << String(contractID) << std::endl;

        return output;
    }

    std::unique_ptr<Mint> mint{nullptr};
    mint.reset(Mint::MintFactory(String(serverID), String(contractID)));

    if (false == bool(mint)) {

        return output;
    }

    const bool validMint = mint->LoadMint() && mint->VerifyMint(serverNym);

    if (false == validMint) {

        return output;
    }

    std::unique_ptr<Ledger> inbox(account->LoadInbox(nym));

    if (false == bool(inbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading inbox for "
              << "account " << String(accountID) << std::endl;

        return output;
    }

    std::unique_ptr<Ledger> outbox(account->LoadOutbox(nym));

    if (nullptr == outbox) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading outbox for "
              << "account " << String(accountID) << std::endl;

        return output;
    }

    std::set<ServerContext::ManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber.Valid()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": No transaction numbers were available. "
                 "Try requesting the server for a new one.\n";
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << managedNumber << std::endl;
    transactionNum = managedNumber;
    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            accountID,
            serverID,
            OTTransaction::withdrawal,
            originType::not_applicable,
            transactionNum));

    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(
        Item::CreateItemFromTransaction(*transaction, Item::withdrawal));

    if (false == bool(item)) {

        return output;
    }

    item->SetNote(String("Gimme cash!"));
    std::unique_ptr<Purse> purse(new Purse(serverID, contractID, serverNymID));

    if (false == bool(purse)) {

        return output;
    }

    std::unique_ptr<Purse> purseCopy(
        new Purse(serverID, contractID, serverNymID));

    if (false == bool(purseCopy)) {

        return output;
    }

    // Create all the necessary tokens for the withdrawal amount. Push copies of
    // each token into a purse to be sent to the server, as well as a purse to
    // be kept for unblinding when we receive the server response. (Coin private
    // unblinding keys are not sent to the server, obviously.)
    const Amount totalAmount(amount);
    Amount workingAmount(totalAmount);
    Amount tokenAmount = 0;

    while ((tokenAmount = mint->GetLargestDenomination(workingAmount)) > 0) {
        workingAmount -= tokenAmount;
        // Create the relevant token request with same server/instrument
        // definition id as the purse. the purse does NOT own the token at this
        // point. The token's constructor just uses it to copy some IDs, since
        // they must match.
        std::unique_ptr<Token> token(Token::InstantiateAndGenerateTokenRequest(
            *purse, nym, *mint, tokenAmount));

        if (false == bool(token)) {

            return output;
        }

        token->SignContract(nym);
        token->SaveContract();
        // Now the proto-token is generated, let's add it to a purse By pushing
        // *token into purse with *pServerNym, I encrypt it to pServerNym.
        // So now only the server Nym can decrypt that token and pop it out
        // of that purse.
        purse->Push(serverNym, *token);
        // I'm saving my own copy of all this, encrypted to my nym instead of
        // the server's, so I can get to my private coin data. The server's copy
        // of token is already Pushed, so I can re-use the variable now for my
        // own purse.
        token->ReleaseSignatures();
        token->SetSavePrivateKeys();
        token->SignContract(nym);
        token->SaveContract();
        purseCopy->Push(nym, *token);
    }

    String purseAttachment;
    purse->SignContract(nym);
    purse->SaveContract();
    purse->SaveContractRaw(purseAttachment);
    item->SetAttachment(purseAttachment);
    purseCopy->SignContract(nym);
    purseCopy->SaveContract();
    // This thing is neat and tidy. The wallet can just save it as an
    // ascii-armored string as a purse field inside the wallet file.  It doesn't
    // do that for now (TODO) but it easily could. Add the purse to the wallet
    // (We will need it to look up the private coin info for unblinding the
    // token, when the response comes from the server.)
    wallet->AddPendingWithdrawal(*purseCopy);
    item->SignContract(nym);
    item->SaveContract();
    transaction->AddItem(*item.release());
    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        totalAmount * (-1), *transaction, context, *account, *outbox));

    if (false == bool(balanceItem)) {

        return output;
    }

    transaction->AddItem(*balanceItem.release());
    transaction->SignContract(nym);
    transaction->SaveContract();
    Ledger ledger(nymID, accountID, serverID);
    ledger.GenerateLedger(accountID, serverID, Ledger::message);
    ledger.AddTransaction(*transaction.release());
    ledger.SignContract(nym);
    ledger.SaveContract();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message(managed, context, *message);

    return output;
}

// Request the server to accept some digital cash and deposit it to an asset
// account.
CommandResult OT_API::notarizeDeposit(
    ServerContext& context,
    const Identifier& accountID,
    const String& THE_PURSE) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    const auto& serverNym = context.RemoteNym();
    const String reason(
        "Depositing a cash purse. Enter passphrase for the purse.");
    const OTPasswordData cashPasswordReason(
        "Depositing a cash purse. Enter master passphrase for wallet.");
    auto account = GetOrLoadAccount(nym, accountID, serverID, __FUNCTION__);

    if (nullptr == account) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading account "
              << String(accountID) << std::endl;

        return output;
    }

    std::unique_ptr<Ledger> inbox(account->LoadInbox(nym));

    if (false == bool(inbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading inbox for "
              << "account " << String(accountID) << std::endl;

        return output;
    }

    std::unique_ptr<Ledger> outbox(account->LoadOutbox(nym));

    if (nullptr == outbox) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading outbox for "
              << "account " << String(accountID) << std::endl;

        return output;
    }

    std::set<ServerContext::ManagedNumber> managed;
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber.Valid()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": No transaction numbers were available. "
                 "Try requesting the server for a new one.\n";
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << managedNumber << std::endl;
    transactionNum = managedNumber;

    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            accountID,
            serverID,
            OTTransaction::deposit,
            originType::not_applicable,
            transactionNum));

    if (false == bool(transaction)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed generate deposit txn. "
              << "account " << String(accountID) << std::endl;

        return output;
    }

    std::unique_ptr<Item> item(
        Item::CreateItemFromTransaction(*transaction, Item::deposit));

    if (false == bool(item)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed generate deposit txn "
                 "item. "
              << "account " << String(accountID) << std::endl;

        return output;
    }

    const auto& contractID = account->GetInstrumentDefinitionID();
    Purse outputPurse(serverID, contractID, serverNym.ID());

    // What's going on here?
    // A purse can be encrypted by a private key (controlled by a Nym) or by a
    // symmetric key (embedded inside the purse along with a corresponding
    // master key.) The below function is what actually loads up pPurse from
    // string (THE_PURSE) and this call also returns pOwner, which is a pointer
    // to a special wrapper class (which you must delete, when you're done with
    // it) which contains a pointer EITHER to the owner Nym for that purse, OR
    // to the "owner" symmetric key for that purse.
    //
    // This way, any subsequent purse operations can use pOwner, regardless of
    // whether there is actually a Nym inside, or a symmetric key. (None of the
    // purse operations will care, since they can use pOwner either way.)
    OTPassword pursePassword;
    Purse sourcePurse(outputPurse);
    std::unique_ptr<OTNym_or_SymmetricKey> purseOwner(LoadPurseAndOwnerForMerge(
        THE_PURSE,
        sourcePurse,
        pursePassword,
        false,   // MUST be private, if a nym.
        &nymID,  // This can be nullptr, *IF* purse is password-protected.
                 // (It's just ignored in that case.) Otherwise if it's
                 // Nym-protected, the purse will have a NymID on it
                 // already, which is what LoadPurseAndOwnerForMerge will
                 // try first. If not (it's optional), then nymID is the
                 // ID it will try next, before failing altogether.
        &reason));

    if (false == bool(purseOwner)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed loading purse and owner. " << std::endl;

        return output;
    }

    OTNym_or_SymmetricKey serverNymAsOwner(serverNym);

    while (false == sourcePurse.IsEmpty()) {
        std::unique_ptr<Token> token(sourcePurse.Pop(*purseOwner));

        if (false == bool(token)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error loading token from purse." << std::endl;

            return output;
        }

        // TODO need 2-recipient envelopes. My request to the server is
        // encrypted to the server's nym, but it should be encrypted to my
        // Nym also, so both have access to decrypt it.
        //
        // Now the token is ready, let's add it to a purse By pushing
        // theToken into outputPurse with serverNym, I encrypt it to
        // pServerNym. So now only the server Nym can decrypt that token and
        // pop it out of that purse.
        if (false == token->ReassignOwnership(*purseOwner, serverNymAsOwner)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error re-assigning ownership of token (to server.)"
                  << std::endl;

            return output;
        }

        otWarn << OT_METHOD << __FUNCTION__
               << ": Success re-assigning ownership of token (to server.)"
               << std::endl;
        token->ReleaseSignatures();
        token->SignContract(nym);
        token->SaveContract();

        Token* pToken = token.get();
        outputPurse.Push(serverNymAsOwner, *token.release());

        item->SetAmount(item->GetAmount() + pToken->GetDenomination());
    }

    String purseAttachment;
    outputPurse.SignContract(nym);
    outputPurse.SaveContract();
    outputPurse.SaveContractRaw(purseAttachment);
    item->SetAttachment(purseAttachment);
    item->SignContract(nym);
    item->SaveContract();

    Item* pItem = item.get();

    transaction->AddItem(*item.release());
    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        pItem->GetAmount(), *transaction, context, *account, *outbox));

    if (false == bool(balanceItem)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to generate balance statement." << std::endl;

        return output;
    }

    transaction->AddItem(*balanceItem.release());
    transaction->SignContract(nym);
    transaction->SaveContract();
    Ledger ledger(nymID, accountID, serverID);
    ledger.GenerateLedger(accountID, serverID, Ledger::message);
    ledger.AddTransaction(*transaction.release());
    ledger.SignContract(nym);
    ledger.SaveContract();

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to generate deposit message." << std::endl;

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to finalize server command for deposit message."
              << std::endl;

        return output;
    }

    result = send_message(managed, context, *message);

    return output;
}
#endif  // OT_CASH

// Let's pretend you are paying a dollar dividend for Pepsi shares...
// Therefore accountID needs to be a dollar account, and
// SHARES_INSTRUMENT_DEFINITION_ID needs
// to be the Pepsi instrument definition ID. (NOT the dollar instrument
// definition ID...)
CommandResult OT_API::payDividend(
    ServerContext& context,
    const Identifier& DIVIDEND_FROM_accountID,  // if dollars paid for pepsi
                                                // shares, then this is the
                                                // issuer's dollars account.
    const Identifier& SHARES_INSTRUMENT_DEFINITION_ID,  // if dollars paid for
                                                        // pepsi shares, then
                                                        // this is the pepsi
                                                        // shares asset type id.
    const String& DIVIDEND_MEMO,  // a message attached to the payout request.
    const Amount& AMOUNT_PER_SHARE) const  // number of dollars to be paid out
                                           // PER SHARE (multiplied by total
                                           // number of shares issued.)
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    auto dividendAccount =
        GetOrLoadAccount(nym, DIVIDEND_FROM_accountID, serverID, __FUNCTION__);

    if (nullptr == dividendAccount) {

        return output;
    }

    auto contract = wallet_.UnitDefinition(SHARES_INSTRUMENT_DEFINITION_ID);

    if (false == bool(contract)) {

        return output;
    }

    // wallet is not owned by this function
    auto wallet = GetWallet(__FUNCTION__);

    if (nullptr == wallet) {

        return output;
    }

    // issuerAccount is not owned by this function
    auto issuerAccount =
        wallet->GetIssuerAccount(SHARES_INSTRUMENT_DEFINITION_ID);

    if (nullptr == issuerAccount) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: Unable to find issuer account for the shares "
              << "instrument definition. Are you sure you're the issuer?"
              << std::endl;

        return output;
    }

    if (false == dividendAccount->VerifyOwner(nym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: Nym doesn't verify as owner of the source "
              << "account for the dividend payout." << std::endl;

        return output;
    }

    if (false == issuerAccount->VerifyOwner(nym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: Nym doesn't verify as owner of issuer "
              << "account for the shares (the shares we're paying the dividend "
              << "on...)" << std::endl;

        return output;
    }

    OT_ASSERT(issuerAccount->GetBalance() <= 0);

    if (0 == issuerAccount->GetBalance()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failure: There are no shares "
              << "issued for that instrument definition." << std::endl;
        requestNum = 0;
        status = SendResult::UNNECESSARY;

        return output;
    }

    if (AMOUNT_PER_SHARE <= 0) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: The amount per share must be larger than zero."
              << std::endl;

        return output;
    }

    // If there are 100,000 Pepsi shares, then the Pepsi issuer acct will have
    // -100,000 balance. Therefore we multiply by -1, resulting in 100,000.
    // Then we multiple that by the amount to be paid per share, let's say $5,
    // resulting in a totalCost of $500,000 that must be available in the dollar
    // account (in order to successfully pay this dividend.)
    const Amount totalCost =
        ((-1) * issuerAccount->GetBalance()) * AMOUNT_PER_SHARE;

    if (dividendAccount->GetBalance() < totalCost) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failure: There's not enough ("
              << dividendAccount->GetBalance() << ") in the source account, to "
              << "cover the total cost of the dividend (" << totalCost << ".)"
              << std::endl;

        return output;
    }

    std::set<ServerContext::ManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber.Valid()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": No transaction numbers were available. "
                 "Try requesting the server for a new one.\n";
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << managedNumber << std::endl;
    transactionNum = managedNumber;

    const auto SHARES_ISSUER_accountID = Identifier::Factory(*issuerAccount);
    // Expiration (ignored by server -- it sets its own for its vouchers.)
    const time64_t VALID_FROM = OTTimeGetCurrentTime();
    const time64_t VALID_TO = OTTimeAddTimeInterval(
        VALID_FROM, OTTimeGetSecondsFromTime(OT_TIME_SIX_MONTHS_IN_SECONDS));

    // The server only uses the amount and instrument definition from this
    // cheque when it constructs the actual voucher (to the dividend payee.) And
    // remember there might be a hundred shareholders, so the server would
    // create a hundred vouchers in that case.
    Cheque theRequestVoucher(serverID, SHARES_INSTRUMENT_DEFINITION_ID);
    const bool bIssueCheque = theRequestVoucher.IssueCheque(
        AMOUNT_PER_SHARE,  // <====== Server needs this (AMOUNT_PER_SHARE.)
        transactionNum,    // server actually ignores this and supplies its own
                           // transaction number for any vouchers.
        VALID_FROM,
        VALID_TO,
        SHARES_ISSUER_accountID,
        nymID,
        DIVIDEND_MEMO,
        Identifier::Factory(nymID));

    if (!bIssueCheque) {

        return output;
    }

    /*
        NOTE: The above cheque isn't actually USED for anything, except as a
        vehicle to send additional data to the server. For example, the server
        will need to know the asset type ID for the shares. It gets that
        information from this voucher's instrument definition ID. It will
        also need to know the amount-per-share, which is also on this voucher,
        as its amount. The voucher code already does a similar thing, and this
        code already copied the voucher code since they were so similar, so
        we're just using the same mechanism here. It's consistent. On the server
        side, we'll also need to know the issuer account ID for the shares
        instrument definition, so we set that as the "from" account on the
        request voucher (again, just as a way of transmitting it.)
        */

    std::unique_ptr<Ledger> inbox(dividendAccount->LoadInbox(nym));

    if (false == bool(inbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading inbox for "
              << "account " << String(DIVIDEND_FROM_accountID) << std::endl;

        return output;
    }

    std::unique_ptr<Ledger> outbox(dividendAccount->LoadOutbox(nym));

    if (nullptr == outbox) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading outbox for "
              << "account " << String(DIVIDEND_FROM_accountID) << std::endl;

        return output;
    }

    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            DIVIDEND_FROM_accountID,
            serverID,
            OTTransaction::payDividend,
            originType::not_applicable,
            transactionNum));

    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(
        Item::CreateItemFromTransaction(*transaction, Item::payDividend));

    if (false == bool(item)) {

        return output;
    }

    // Notice, while the CHEQUE is for AMOUNT_PER_SHARE, the item's AMOUNT is
    // set to totalCost. The server just needs both of those, so that's how we
    // send them (Similar to the voucher code.)
    item->SetAmount(totalCost);
    item->SetNote(String("Pay Dividend: "));
    theRequestVoucher.SignContract(nym);
    theRequestVoucher.SaveContract();
    item->SetAttachment(String(theRequestVoucher));
    item->SignContract(nym);
    item->SaveContract();
    transaction->AddItem(*item.release());
    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        totalCost * (-1), *transaction, context, *dividendAccount, *outbox));

    if (false == bool(balanceItem)) {

        return output;
    }

    // Notice the balance agreement is made for the "total cost of the
    // dividend", which we calculated as the issuer's account balance, times -1,
    // times the amount per share. So for 100,000 shares of Pepsi, at a dividend
    // payout of $2 per share, then $200,000 must be removed from my dollar
    // account, in order to cover it. Therefore I sign a balance agreement for
    // $200,000. The server removes it all at once, and then iterates through a
    // loop, sending vouchers to people. If any fail, or there is any left over,
    // then vouchers are sent back to nym again, containingv the difference.
    // TODO failsafe: We can't just loop, std::int64_t-term, and send a
    // voucher at the end. What if it crashes halfway through the loop? It seems
    // that the dividend payout still needs to be "REGISTERED" somewhere until
    // successfully completed. (And therefore, that this concept must be
    // repeated throughout OT for other transactions, not just this example.)
    // This is already done with Cron, but just thinking about how to best do it
    // for "single action" transactions.

    transaction->AddItem(*balanceItem.release());
    transaction->SignContract(nym);
    transaction->SaveContract();
    Ledger ledger(nymID, DIVIDEND_FROM_accountID, serverID);
    ledger.GenerateLedger(DIVIDEND_FROM_accountID, serverID, Ledger::message);
    ledger.AddTransaction(*transaction.release());
    ledger.SignContract(nym);
    ledger.SaveContract();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        DIVIDEND_FROM_accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message(managed, context, *message);

    return output;
}

// Request the server to withdraw from an asset account and issue a voucher
// (cashier's cheque)
CommandResult OT_API::withdrawVoucher(
    ServerContext& context,
    const Identifier& accountID,
    const Identifier& RECIPIENT_NYM_ID,
    const String& CHEQUE_MEMO,
    const Amount amount) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    auto account = GetOrLoadAccount(nym, accountID, serverID, __FUNCTION__);

    if (nullptr == account) {

        return output;
    }

    auto contractID = Identifier::Factory();
    String strContractID;
    contractID = account->GetInstrumentDefinitionID();
    contractID->GetString(strContractID);
    Message theMessage;
    const auto withdrawalNumber =
        context.NextTransactionNumber(MessageType::notarizeTransaction);
    const auto voucherNumber =
        context.NextTransactionNumber(MessageType::notarizeTransaction);

    if ((!withdrawalNumber.Valid()) || (!voucherNumber.Valid())) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Not enough Transaction Numbers were available. "
                 "(Suggest requesting the server for more.)\n";

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << withdrawalNumber << " for withdrawal" << std::endl;

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << voucherNumber << " for voucher" << std::endl;

    const String strChequeMemo(CHEQUE_MEMO);
    const String strRecipientNymID(RECIPIENT_NYM_ID);
    // Expiration (ignored by server -- it sets its own for its vouchers.)
    const time64_t VALID_FROM =
        OTTimeGetCurrentTime();  // This time is set to TODAY NOW
    const time64_t VALID_TO = OTTimeAddTimeInterval(
        VALID_FROM,
        OTTimeGetSecondsFromTime(OT_TIME_SIX_MONTHS_IN_SECONDS));  // 6 months.
    // The server only uses the memo, amount, and recipient from this cheque
    // when it
    // constructs the actual voucher.
    Cheque theRequestVoucher(serverID, contractID);
    bool bIssueCheque = theRequestVoucher.IssueCheque(
        amount,
        voucherNumber,
        VALID_FROM,
        VALID_TO,
        accountID,
        nymID,
        strChequeMemo,
        *((strRecipientNymID.GetLength() > 2) ? &(RECIPIENT_NYM_ID) : nullptr));
    std::unique_ptr<Ledger> inbox(account->LoadInbox(nym));
    std::unique_ptr<Ledger> outbox(account->LoadOutbox(nym));

    if (nullptr == inbox) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading inbox for acct "
              << String(accountID) << "\n";

        return output;
    }

    if (nullptr == outbox) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed loading outbox for acct " << String(accountID)
              << "\n";

        return output;
    }

    if (!bIssueCheque) {

        return output;
    }

    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            accountID,
            serverID,
            OTTransaction::withdrawal,
            originType::not_applicable,
            withdrawalNumber));
    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(
        Item::CreateItemFromTransaction(*transaction, Item::withdrawVoucher));

    if (false == bool(item)) {

        return output;
    }

    item->SetAmount(amount);
    item->SetNote(String(" "));
    theRequestVoucher.SignContract(nym);
    theRequestVoucher.SaveContract();
    String strVoucher(theRequestVoucher);
    item->SetAttachment(strVoucher);
    item->SignContract(nym);
    item->SaveContract();
    transaction->AddItem(*item.release());
    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        amount * (-1), *transaction, context, *account, *outbox));

    if (false == bool(item)) {

        return output;
    }

    transaction->AddItem(*balanceItem.release());
    transaction->SignContract(nym);
    transaction->SaveContract();
    Ledger ledger(nymID, accountID, serverID);
    ledger.GenerateLedger(accountID, serverID, Ledger::message);
    ledger.AddTransaction(*transaction.release());
    ledger.SignContract(nym);
    ledger.SaveContract();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
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
// internally by the high-level API, for certain special harvesting cases where
// the cheque hasn't possibly been sent or used anywhere when it's discarded.
//
// Voucher update: now that the remitter's transaction number is used on
// vouchers, you would think that we would have to retrofit this function to
// support vouchers as well. However, that's not the case. The reason is because
// vouchers must be withdrawn at the server, which means creating a voucher
// automatically implies that the server has already marked the transaction
// number as "in use." BUT ACTUALLY I'M WRONG ABOUT THAT! The server doesn't
// mark it as "in use" until it's DEPOSITED -- and then marks it as "closed"
// once the voucherReceipt is processed. That might seem strange -- the server
// issues a voucher, drawn on its own account, without marking its transaction
// number as "in use" ? Reason is, the instrument still cannot actually be used
// without depositing it, at which time the transaction number WILL be marked as
// "in use." Until then, you could recover the number and use it somewhere else
// instead. But why the hell would you do that? Because once you do that, you
// can no longer use it with the voucher, which means you can no longer recover
// any of the money that you sent to the server, when you purchased that voucher
// in the first place. Therefore you would NEVER want to just "discard" a
// voucher like you might with a cheque -- that voucher cost you money!
// Basically you would DEFINITELY want to REFUND that voucher and get that money
// BACK, and not merely re-use the number on it. Therefore vouchers will never
// just be "discarded" but rather, stored in the outpayment box and REFUNDED if
// necessary. (Therefore we won't be retrofitting this function for vouchers.)
bool OT_API::DiscardCheque(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const Identifier& accountID,
    const String& THE_CHEQUE) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = wallet_.mutable_ServerContext(NYM_ID, NOTARY_ID);
    auto nym = context.It().Nym();

    auto pServer = wallet_.Server(NOTARY_ID);

    if (!pServer) {
        return false;
    }

    auto account = GetOrLoadAccount(*nym, accountID, NOTARY_ID, __FUNCTION__);

    if (false == bool(account)) {

        return false;
    }

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    auto contractID = Identifier::Factory();
    String strContractID;

    contractID = account->GetInstrumentDefinitionID();
    contractID->GetString(strContractID);
    // By this point, nymfile is a good pointer, and is on the wallet.
    //  (No need to cleanup.)  pServer and account are also good.
    //
    const String strNotaryID(NOTARY_ID), strNymID(NYM_ID);

    Cheque theCheque(NOTARY_ID, contractID);

    if (!theCheque.LoadContractFromString(THE_CHEQUE)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load cheque from string. Sorry. "
                 "Cheque contents:\n\n"
              << THE_CHEQUE << "\n\n";
        return false;
    } else if (
        (theCheque.GetNotaryID() == NOTARY_ID) &&
        (theCheque.GetInstrumentDefinitionID() == contractID) &&
        (theCheque.GetSenderNymID() == NYM_ID) &&
        (theCheque.GetSenderAcctID() == accountID)) {
        // we only "add it back" if it was really there in the first place.
        if (context.It().VerifyIssuedNumber(theCheque.GetTransactionNum())) {
            context.It().RecoverAvailableNumber(theCheque.GetTransactionNum());

            return true;
        } else  // No point adding it back as available to use, if nymfile
                // doesn't
                // even have it signed out!
        {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed attempt to claw back a transaction number that "
                     "wasn't signed out to nymfile in the first place. "
                     "Cheque contents:\n\n"
                  << THE_CHEQUE << "\n\n";
            return false;
        }
    }  // bSuccess
    else
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to verify cheque's server ID, instrument definition "
                 "ID, user ID, or acct ID. Sorry. "
                 "Cheque contents:\n\n"
              << THE_CHEQUE << "\n\n";
    return false;
}

// DEPOSIT CHEQUE
//
// NOTE: If THE_CHEQUE is drawn on the account denoted by accountID (meaning
// it's being deposited back into the same account that originally wrote
// the cheque) this means the original cheque writer is CANCELLING the
// cheque, to prevent the recipient from depositing it.
CommandResult OT_API::depositCheque(
    ServerContext& context,
    const Identifier& accountID,
    const Cheque& theCheque) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    auto account = GetOrLoadAccount(nym, accountID, serverID, __FUNCTION__);

    if (nullptr == account) {

        return output;
    }

    if (theCheque.GetNotaryID() != serverID) {
        otErr << OT_METHOD << __FUNCTION__ << ": NotaryID on cheque ("
              << String(theCheque.GetNotaryID())
              << ") doesn't "
                 "match notaryID where it's being deposited to ("
              << String(serverID) << ").";

        return output;
    }

    std::unique_ptr<Ledger> inbox(account->LoadInbox(nym));

    if (false == bool(inbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed loading inbox for acct "
              << String(accountID) << "\n";

        return output;
    }

    // If bCancellingCheque==true, we're actually cancelling the cheque by
    // "depositing" it back into the same account it's drawn on.
    bool bCancellingCheque{false};
    Cheque copy(serverID, account->GetInstrumentDefinitionID());

    if (theCheque.HasRemitter()) {
        bCancellingCheque =
            ((theCheque.GetRemitterAcctID() == accountID) &&
             (theCheque.GetRemitterNymID() == nymID));
    } else {
        bCancellingCheque =
            ((theCheque.GetSenderAcctID() == accountID) &&
             (theCheque.GetSenderNymID() == nymID));
        if (bCancellingCheque)
            bCancellingCheque = theCheque.VerifySignature(nym);
    }

    // By this point he's definitely TRYING to cancel the cheque.
    if (bCancellingCheque) {
        bCancellingCheque =
            context.VerifyIssuedNumber(theCheque.GetTransactionNum());

        // If we TRIED to cancel the cheque (being in this block...) yet the
        // signature fails to verify, or the transaction number isn't even
        // issued, then our attempt to cancel the cheque is going to fail.
        if (!bCancellingCheque) {
            // This is the "tried and failed" block.
            otErr << OT_METHOD << __FUNCTION__
                  << ": Cannot cancel this cheque. Either the "
                     "signature fails to verify,\n"
                     "or the transaction number is already closed "
                     "out. (Failure.) Cheque contents:\n\n"
                  << String(theCheque) << "\n\n";

            return output;
        }

        // Else we succeeded in verifying signature and issued num.
        // Let's just make sure there isn't a chequeReceipt or
        // voucherReceipt already sitting in the inbox, for this same cheque.
        OTTransaction* pChequeReceipt =
            inbox->GetChequeReceipt(theCheque.GetTransactionNum());

        if (nullptr != pChequeReceipt)  // Hmm looks like there's ALREADY a
                                        // chequeReceipt in the inbox (so we
                                        // can't cancel it.)
        {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Cannot cancel this cheque. There is "
                     "already a "
                  << (theCheque.HasRemitter() ? "voucherReceipt"
                                              : "chequeReceipt")
                  << " for it in the inbox. "
                     "(Failure.) Cheque contents:\n\n"
                  << String(theCheque) << "\n\n";

            return output;
        }

        if (!copy.LoadContractFromString(String(theCheque))) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unable to load cheque from string. Sorry. Contents:\n\n"
                  << String(theCheque) << "\n\n";

            return output;
        }
    }

    // By this point, we're either NOT cancelling the cheque, or if we are,
    // we've already verified the signature and transaction number on the
    // cheque. (AND we've already verified that there aren't any chequeReceipts
    // for this cheque, in the inbox.)
    const bool cancel = (bCancellingCheque && !theCheque.HasRemitter());

    if (cancel) {
        copy.CancelCheque();       // Sets the amount to zero.
        copy.ReleaseSignatures();  // Usually when you deposit a
                                   // cheque,
                                   // it's signed by someone else.
        copy.SignContract(nym);    // But if we are CANCELING a cheque,
                                   // that means we wrote that cheque
        copy.SaveContract();       // originally, so we are the original
                                   // signer.
    }                              // cancelling cheque

    const auto& cheque = cancel ? copy : theCheque;
    std::set<ServerContext::ManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber.Valid()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": No transaction numbers were available. "
                 "Try requesting the server for a new one.\n";
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << managedNumber << std::endl;
    transactionNum = managedNumber;
    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            accountID,
            serverID,
            OTTransaction::deposit,
            originType::not_applicable,
            transactionNum));

    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(
        Item::CreateItemFromTransaction(*transaction, Item::depositCheque));

    if (false == bool(item)) {

        return output;
    }

    const String strNote(
        bCancellingCheque ? "Cancel this cheque, please!"
                          : "Deposit this cheque, please!");  // TODO
    item->SetNote(strNote);
    item->SetAttachment(String(cheque));
    item->SignContract(nym);
    item->SaveContract();

    transaction->AddItem(*item.release());
    std::unique_ptr<Ledger> outbox(account->LoadOutbox(nym));

    if (false == bool(outbox)) {
        otErr << "OT_API::depositCheque: Failed loading outbox for acct "
              << String(accountID) << "\n";

        return output;
    }

    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        cheque.GetAmount(), *transaction, context, *account, *outbox));

    if (false == bool(balanceItem)) {

        return output;
    }

    transaction->AddItem(*balanceItem.release());
    transaction->SignContract(nym);
    transaction->SaveContract();

    Ledger ledger(nymID, accountID, serverID);
    const bool generated =
        ledger.GenerateLedger(accountID, serverID, Ledger::message);

    if (false == generated) {

        return output;
    }

    ledger.AddTransaction(*transaction.release());

    // sign the ledger
    ledger.SignContract(nym);
    ledger.SaveContract();

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message(managed, context, *message);

    if (0 < cheque.GetAmount()) {
        const auto workflowUpdated = workflow_.DepositCheque(
            nymID, accountID, cheque, *message, reply.get());

        if (workflowUpdated) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Successfully updated workflow" << std::endl;
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to update workflow"
                  << std::endl;
        }
    }

    return output;
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
CommandResult OT_API::depositPaymentPlan(
    ServerContext& context,
    const String& THE_PAYMENT_PLAN) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    OTPaymentPlan plan;
    const bool validPlan = plan.LoadContractFromString(THE_PAYMENT_PLAN) &&
                           plan.VerifySignature(nym);

    if (false == validPlan) {
        otErr << OT_METHOD << __FUNCTION__
              << "Unable to load payment plan from string, or verify it."
              << std::endl;

        return output;
    }

    const bool bCancelling = (plan.GetRecipientNymID() == nymID);

    if (bCancelling) {
        if (plan.IsCanceled()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error: attempted to cancel (pre-emptively, "
                     "before activation) a payment plan "
                     "that was already set as canceled.\n";

            return output;
        }

        if (!plan.CancelBeforeActivation(nym)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error: attempted to cancel (pre-emptively, "
                     "before activation) a payment plan, "
                     "but the attempt somehow failed.\n";

            return output;
        }
    }

    // The logic for accountID comes about because normally the sender is the
    // one
    // who activates the payment plan. BUT the recipient might ALSO activate it
    // as a way of CANCELLING it. So we check to see if the  recipient user is
    // the same as nymID. If so, then we use the RECIPIENT  account here (it's
    // being cancelled.) Otherwise, we use the sender account ID here as normal
    // (it's being activated.)
    const auto accountID = Identifier::Factory(
        bCancelling ? plan.GetRecipientAcctID() : plan.GetSenderAcctID());
    auto account = GetOrLoadAccount(nym, accountID, serverID, __FUNCTION__);

    if (nullptr == account) {

        return output;
    }

    const auto openingNumber = plan.GetOpeningNumber(nymID);
    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            accountID,
            serverID,
            OTTransaction::paymentPlan,
            originType::origin_payment_plan,
            openingNumber));

    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(
        Item::CreateItemFromTransaction(*transaction, Item::paymentPlan));

    if (false == bool(item)) {

        return output;
    }

    item->SetAttachment(String(plan));
    item->SignContract(nym);
    item->SaveContract();
    transaction->AddItem(*item.release());
    auto statement = context.Statement(*transaction);

    if (false == bool(statement)) {

        return output;
    }

    transaction->AddItem(*statement.release());
    transaction->SignContract(nym);
    transaction->SaveContract();
    Ledger ledger(nymID, accountID, serverID);
    ledger.GenerateLedger(accountID, serverID, Ledger::message);
    ledger.AddTransaction(*transaction.release());
    ledger.SignContract(nym);
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

// If a smart contract is already running on a specific server, and the Nym in
// question (NYM_ID) is an authorized agent for that smart contract, then he
// can trigger clauses. All he needs is the transaction ID for the smart
// contract, and the name of the clause.
CommandResult OT_API::triggerClause(
    ServerContext& context,
    const TransactionNumber& transactionNumber,
    const String& strClauseName,
    const String* pStrParam) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = transactionNumber;
    status = SendResult::ERROR;
    reply.reset();
    OTASCIIArmor payload{};

    // Optional string parameter. Available as "param_string" inside the script.
    if ((nullptr != pStrParam) && (pStrParam->Exists())) {
        payload.SetString(*pStrParam);
    }

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::triggerClause, payload, Identifier::Factory(), requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_lTransactionNum = transactionNumber;
    message->m_strNymID2 = strClauseName;

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::activateSmartContract(
    ServerContext& context,
    const String& THE_SMART_CONTRACT) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    OTSmartContract contract(serverID);

    if (false == contract.LoadContractFromString(THE_SMART_CONTRACT)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load smart contract from string:\n"
              << THE_SMART_CONTRACT << std::endl;

        return output;
    }

    OTAgent* agent{nullptr};
    OTParty* party = contract.FindPartyBasedOnNymAsAuthAgent(nym, &agent);

    if (nullptr == party) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failure: NYM_ID *IS* a valid "
              << "Nym, but that Nym is not the authorizing agent for any of "
              << "the parties on this contract. Try calling ConfirmParty() "
              << "first." << std::endl;

        return output;
    }

    OT_ASSERT(nullptr != agent);

    // BELOW THIS POINT, agent and party are both valid, and no need to clean
    // them up. Note: Usually, payment plan or market offer will load up the Nym
    // and accounts, and verify ownership, etc. But in this case, the Nym who
    // actually activates the smart contract is merely the authorizing agent for
    // a single party, where there may be a dozen parties listed on the actual
    // contract.
    //
    // It would be unreasonable to expect a party to have EVERY nym and EVERY
    // account to the entire contract. As long as the Nym is legitimately the
    // authorizing agent for one of the parties, then we allow him to send the
    // activation message to the server.
    //
    // (Let the server sort out the trouble of loading all the nyms, loading all
    // the accounts, verifying all the instrument definition IDs, verifying all
    // the agent names, making sure there are no stashes already created, ETC
    // ETC.)
    //
    // ONE THING I should verify, is that all of the parties are confirmed. I
    // mean, it's not even worth the bother to send the damn thing until then,
    // right? And the Notary ID.

    if (false == contract.AllPartiesHaveSupposedlyConfirmed()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Not all parties to smart "
              << "contract are confirmed. Treating this as a request for "
              << "cancelation..." << std::endl;

        if (contract.IsCanceled()) {
            otErr << OT_METHOD << __FUNCTION__ << ": Error: attempted to "
                  << "cancel (pre-emptively, before activation) a smart "
                  << "contract that was already set as canceled." << std::endl;

            return output;
        }

        if (false == contract.CancelBeforeActivation(nym)) {
            otErr << OT_METHOD << __FUNCTION__ << ": Error: attempted to "
                  << "cancel (pre-emptively, before activation) a smart "
                  << "contract, but the attempt somehow failed." << std::endl;

            return output;
        }
    }

    if (serverID != contract.GetNotaryID()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed. The server ID passed "
              << "in doesn't match the one on the contract itself."
              << std::endl;

        return output;
    }

    if (1 > party->GetAccountCount()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed. The activating Nym "
              << "must not only be the authorizing agent for one of the "
              << "parties, but must also be the authorized agent for one of "
              << "that party's asset accounts. That party must have at least "
              << "one asset account for this reason. (See code comment below "
              << "this message, in the code.)" << std::endl;

        return output;
    }

    // REQUIREMENT: The ACTIVATOR aka the Originator Nym (the party who
    // activates the smart contract on the server) must have at least one
    // asset account as part of the smart contract, for which the
    // authorizing agent for that party is also the authorized agent for
    // that account. This is in order to make sure that the smart contracts
    // will work with the existing infrastructure, so that functions like
    // "GetOpeningNumber()" and "GetClosingNumber()" will continue to work
    // the way they are expected to, and so that there is always at least
    // ONE closing transaction number supplied for the smart contract (at
    // least ONE finalReceipt will actually have to be signed for once it is
    // deactivated) and so we can know who the owner of that account, and
    // what acct # it will be.
    //
    // This is also a requirement because the existing infrastructure uses
    // the transaction system for activating cron items. It was simply not
    // anticipated in the past that cron items, based on valid transactions,
    // wouldn't also therefore be associated with at least one asset
    // account. In fact, the original difference between transactions (vs
    // normal messages) was that transactions dealt with asset accounts,
    // whereas normal messages did not. (Such as, "checkNym" or
    // "getRequestNumber".)
    //
    // A big piece of this is the BALANCE AGREEMENT. Obviously it wasn't
    // anticipated that "balance agreements" would ever be needed for
    // transactions, yet asset accounts wouldn't be. So the below
    // TRANSACTION STATEMENT, for example, expects an asset account, as does
    // the verification code associated with it. Even transaction statements
    // were originally made for situations where the balance wasn't actually
    // changing, (and so a balance agreement wasn't needed), but where a
    // signature on the transaction numbers was still necessary, since one
    // had to be burned in order to prove that the instrument was valid.
    //
    // Unfortunately, transaction statements still expected an asset account
    // to at least EXIST, since they involved market offers (which do not
    // immediately change the balance) or payment plans (which also do not
    // immediately change the balance.) Yet the account's ID was still at
    // least present in the receipt, and a transaction # was still used.
    //
    // In the case of smart contracts, I will probably someday lift this
    // restriction (that the activating nym must also be the authorized
    // agent on one of the asset accounts on the smart contract). But I will
    // have to go over all the plumbing first and deal with that. In
    // the meantime:
    //
    // --- THE ACTIVATING NYM *MUST* ALSO BE THE AUTHORIZED AGENT FOR AT
    // LEAST ONE ASSET ACCOUNT, FOR THAT PARTY. ---

    auto account = party->GetAccountByAgent(agent->GetName().Get());

    if (nullptr == account) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed. The activating Nym "
              << "must not only be the authorizing agent for one of the "
              << "parties, but must also be the authorized agent for one of "
              << "that party's asset accounts. That party must have at least "
              << "one asset account for this reason. (See code comment above "
              << "this message, in the code.)" << std::endl;

        return output;
    }

    const auto accountID = Identifier::Factory(account->GetAcctID());

    if (accountID->empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed. The Account ID is "
              << "blank for asset acct (" << account->GetName() << ") for "
              << "party (" << party->GetPartyName() << "). Did you confirm "
              << "this account, before trying to activate this contract?"
              << std::endl;

        return output;
    }

    const auto openingNumber = party->GetOpeningTransNo();
    const auto closingNumber = account->GetClosingTransNo();

    if ((openingNumber <= 0) || (closingNumber <= 0)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed. Opening Transaction # "
              << "(" << openingNumber << ") or Closing # (" << closingNumber
              << ") were invalid for asset acct (" << account->GetName()
              << ") for party (" << party->GetPartyName() << "). Did you "
              << "confirm this account and party, before trying to activate "
              << "this contract?" << std::endl;

        return output;
    }

    if (false == context.VerifyIssuedNumber(openingNumber)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed. Opening Transaction # "
              << "(" << openingNumber << ") wasn't valid/issued to this Nym, "
              << "for asset acct (" << account->GetName() << ") for party ("
              << party->GetPartyName() << ") on server (" << String(serverID)
              << "). Did you confirm this account and party, before trying to "
              << "activate this contract?" << std::endl;

        return output;
    }

    if (false == context.VerifyIssuedNumber(closingNumber)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed. Closing Transaction # "
              << "(" << closingNumber << ") wasn't issued to this Nym, for "
              << "asset acct (" << account->GetName() << ") for party ("
              << party->GetPartyName() << "). Did you confirm this account and "
              << "party, before trying to activate this contract?" << std::endl;

        return output;
    }

    // These numbers (the opening and closing transaction #s) were already
    // available on the smart contract -- specifically an opening number is
    // stored for each party, taken from the authorizing agent for that party,
    // and a closing number is stored for each asset account, taken from the
    // authorized agent for that asset account.
    //
    // Now we set the appropriate opening/closing number onto the smart
    // contract, in the way of the existing CronItem system, so that it will
    // work properly within that infrastructure. (This is what makes the
    // activating Nym slightly different / more than the other authorizing
    // agent nyms for each party to the contract. Not only does it have
    // opening/closing numbers on its party like all the others, but its numbers
    // are also those used for the cron item itself.)
    contract.PrepareToActivate(openingNumber, closingNumber, nymID, accountID);
    // This call changes the contract slightly, so it must be re-signed (in
    // order to save changes.)
    contract.ReleaseSignatures();

    if (false == agent->SignContract(contract)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed re-signing contract, "
              << "after calling PrepareToActivate()." << std::endl;

        return output;
    }

    contract.SaveContract();
    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            accountID,
            serverID,
            OTTransaction::smartContract,
            originType::not_applicable,
            contract.GetTransactionNum()));

    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(
        Item::CreateItemFromTransaction(*transaction, Item::smartContract));

    if (false == bool(item)) {

        return output;
    }

    item->SetAttachment(String(contract));
    item->SignContract(nym);
    item->SaveContract();
    transaction->AddItem(*item.release());
    auto statement = context.Statement(*transaction);

    if (false == bool(statement)) {

        return output;
    }

    transaction->AddItem(*statement.release());
    transaction->SignContract(nym);
    transaction->SaveContract();
    Ledger ledger(nymID, accountID, serverID);
    ledger.GenerateLedger(accountID, serverID, Ledger::message);
    ledger.AddTransaction(*transaction.release());
    ledger.SignContract(nym);
    ledger.SaveContract();

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
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
CommandResult OT_API::cancelCronItem(
    ServerContext& context,
    const Identifier& ASSET_ACCOUNT_ID,
    const TransactionNumber& lTransactionNum) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();

    if (context.AvailableNumbers() < 1) {
        otErr << OT_METHOD << __FUNCTION__
              << ": At least 1 Transaction Number is necessary to cancel any "
              << "cron item. Try requesting the server for more numbers (you "
              << "are low.)" << std::endl;
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    std::set<ServerContext::ManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber.Valid()) {
        otErr << "No transaction numbers were available. Suggest "
                 "requesting the server for one.\n";
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << managedNumber << std::endl;
    transactionNum = managedNumber;
    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            ASSET_ACCOUNT_ID,
            serverID,
            OTTransaction::cancelCronItem,
            originType::not_applicable,
            transactionNum));

    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(
        Item::CreateItemFromTransaction(*transaction, Item::cancelCronItem));

    if (false == bool(item)) {

        return output;
    }

    item->SetReferenceToNum(transactionNum);
    transaction->SetReferenceToNum(transactionNum);
    item->SignContract(nym);
    item->SaveContract();
    transaction->AddItem(*item.release());
    auto statement = context.Statement(*transaction);

    if (false == bool(statement)) {

        return output;
    }

    transaction->AddItem(*statement.release());
    transaction->SignContract(nym);
    transaction->SaveContract();

    // set up the ledger
    Ledger ledger(nymID, ASSET_ACCOUNT_ID, serverID);
    ledger.GenerateLedger(ASSET_ACCOUNT_ID, serverID, Ledger::message);
    ledger.AddTransaction(*transaction.release());
    ledger.SignContract(nym);
    ledger.SaveContract();

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        ASSET_ACCOUNT_ID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message(managed, context, *message);

    return output;
}

// Create an Offer object and add it to one of the server's Market objects.
// This will also create a Trade object and add it to the server's Cron
// object. (The Trade provides the payment authorization for the Offer, as well
// as the rules for processing and expiring it.)
CommandResult OT_API::issueMarketOffer(
    ServerContext& context,
    const Identifier& ASSET_ACCOUNT_ID,
    const Identifier& CURRENCY_ACCOUNT_ID,
    const std::int64_t& MARKET_SCALE,       // Defaults to minimum of 1. Market
                                            // granularity.
    const std::int64_t& MINIMUM_INCREMENT,  // This will be multiplied by the
                                            // Scale.
                                            // Min 1.
    const std::int64_t& TOTAL_ASSETS_ON_OFFER,  // Total assets available for
                                                // sale or purchase. Will be
                                                // multiplied by minimum
                                                // increment.
    const Amount PRICE_LIMIT,                   // Per Minimum Increment...
    bool bBuyingOrSelling,                //  BUYING == false, SELLING == true.
    time64_t tLifespanInSeconds,          // 86400 == 1 day.
    char STOP_SIGN,                       // For stop orders, set to '<' or '>'
    const Amount ACTIVATION_PRICE) const  // For stop orders, this is
                                          // threshhold price.
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    auto assetAccount =
        GetOrLoadAccount(nym, ASSET_ACCOUNT_ID, serverID, __FUNCTION__);

    if (nullptr == assetAccount) {

        return output;
    }

    auto currencyAccount =
        GetOrLoadAccount(nym, CURRENCY_ACCOUNT_ID, serverID, __FUNCTION__);

    if (nullptr == currencyAccount) {

        return output;
    }

    const auto& assetContractID = assetAccount->GetInstrumentDefinitionID();
    const auto& currencyContractID =
        currencyAccount->GetInstrumentDefinitionID();

    if (assetContractID == currencyContractID) {
        otErr << OT_METHOD << __FUNCTION__
              << ": The asset account and currency account cannot "
                 "have the same instrument definition ID. (You can't, for "
                 "example, trade dollars against other dollars. Why "
                 "bother trading them in the first place?)"
              << std::endl;

        return output;
    }

    if (context.AvailableNumbers() < 3) {
        otErr << OT_METHOD << __FUNCTION__
              << ": At least 3 Transaction Numbers are necessary to issue a "
              << "market offer. Try requesting the server for more (you are "
              << "low.)" << std::endl;
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    std::set<ServerContext::ManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& openingNumber = *managed.rbegin();

    if (false == openingNumber.Valid()) {
        otErr << "No transaction numbers were available. Suggest "
                 "requesting the server for one.\n";

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << openingNumber << std::endl;
    transactionNum = openingNumber;
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& assetClosingNumber = *managed.rbegin();

    if (false == openingNumber.Valid()) {
        otErr << "No assetClosingNumber numbers were available. Suggest "
                 "requesting the server for one.\n";

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << assetClosingNumber << std::endl;
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& currencyClosingNumber = *managed.rbegin();

    if (false == currencyClosingNumber.Valid()) {
        otErr << "No transaction numbers were available. Suggest "
                 "requesting the server for one.\n";

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << currencyClosingNumber << std::endl;

    // defaults to RIGHT NOW aka OT_API_GetTime() if set to 0 anyway.
    const time64_t VALID_FROM = GetTime();
    // defaults to 24 hours (a "Day Order") aka OT_API_GetTime() + 86,400
    const time64_t VALID_TO = OTTimeAddTimeInterval(
        VALID_FROM,
        OTTimeGetSecondsFromTime(
            OT_TIME_ZERO == tLifespanInSeconds ? OT_TIME_DAY_IN_SECONDS
                                               : tLifespanInSeconds));
    std::int64_t lTotalAssetsOnOffer = 1, lMinimumIncrement = 1,
                 lPriceLimit = 0,  // your price limit, per scale of assets.
        lMarketScale = 1, lActivationPrice = 0;

    if (TOTAL_ASSETS_ON_OFFER > 0) {
        lTotalAssetsOnOffer =
            TOTAL_ASSETS_ON_OFFER;  // otherwise, defaults to 1.
    }

    if (MARKET_SCALE > 0) {
        lMarketScale = MARKET_SCALE;  // otherwise, defaults to 1.
    }

    if (MINIMUM_INCREMENT > 0) {
        lMinimumIncrement = MINIMUM_INCREMENT;  // otherwise, defaults to 1.
    }

    if (PRICE_LIMIT > 0) {
        lPriceLimit = PRICE_LIMIT;  // otherwise, defaults to 0. (0 Being a
                                    // market order.)
    }

    char cStopSign = 0;

    if ((ACTIVATION_PRICE > 0) && (('<' == STOP_SIGN) || ('>' == STOP_SIGN))) {
        cStopSign = STOP_SIGN;
        lActivationPrice = ACTIVATION_PRICE;
    }

    lMinimumIncrement *= lMarketScale;  // minimum increment is PER SCALE.
    String strOfferType("market order");

    if (lPriceLimit > 0) {
        strOfferType = "limit order";
    }

    if (0 != cStopSign) {
        if (lPriceLimit > 0) {
            strOfferType.Format(
                "stop limit order, at threshhold: %c%" PRId64,
                cStopSign,
                lActivationPrice);
        } else {
            strOfferType.Format(
                "stop order, at threshhold: %c%" PRId64,
                cStopSign,
                lActivationPrice);
        }
    }

    String strPrice("");

    if (lPriceLimit > 0) {
        strPrice.Format("Price: %" PRId64 "\n", lPriceLimit);
    }

    OTOffer offer(serverID, assetContractID, currencyContractID, lMarketScale);
    OTTrade trade(
        serverID,
        assetContractID,
        ASSET_ACCOUNT_ID,
        nymID,
        currencyContractID,
        CURRENCY_ACCOUNT_ID);
    bool bCreateOffer = offer.MakeOffer(
        bBuyingOrSelling,     // True == SELLING, False == BUYING
        lPriceLimit,          // Per Minimum Increment...
        lTotalAssetsOnOffer,  // Total assets available for sale or
                              // purchase.
        lMinimumIncrement,    // The minimum increment that must be bought or
                              // sold for each transaction
        openingNumber,        // Transaction number matches on
                              // transaction, item, offer, and trade.
        VALID_FROM,           // defaults to RIGHT NOW aka OT_API_GetTime()
        VALID_TO);            // defaults to 24 hours (a "Day Order") aka
                              // OT_API_GetTime() + 86,400

    if (false == bCreateOffer) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create offer or issue trade." << std::endl;

        return output;
    }

    bCreateOffer = offer.SignContract(nym);

    if (false == bCreateOffer) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create offer or issue trade." << std::endl;

        return output;
    }

    bCreateOffer = offer.SaveContract();

    if (false == bCreateOffer) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create offer or issue trade." << std::endl;

        return output;
    }

    bool bIssueTrade = trade.IssueTrade(offer, cStopSign, lActivationPrice);

    if (false == bIssueTrade) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create offer or issue trade." << std::endl;

        return output;
    }

    bIssueTrade = trade.SignContract(nym);

    if (false == bIssueTrade) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create offer or issue trade." << std::endl;

        return output;
    }

    bIssueTrade = trade.SaveContract();

    if (false == bIssueTrade) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to create offer or issue trade." << std::endl;

        return output;
    }

    // This is new: the closing transaction number is now used for CLOSING
    // recurring cron items, like market offers and payment plans. It's also
    // useful for baskets. Since this is a market offer, it needs a closing
    // number (for later cancellation or expiration.) assetClosingNumber=0,
    // currencyClosingNumber=0;
    trade.AddClosingTransactionNo(assetClosingNumber);
    trade.AddClosingTransactionNo(currencyClosingNumber);
    otErr << "Placing market offer " << openingNumber
          << ", type: " << (bBuyingOrSelling ? "selling" : "buying") << ", "
          << strOfferType << "\n"
          << strPrice << "Assets for sale/purchase: " << lTotalAssetsOnOffer
          << "\nIn minimum increments of: " << lMinimumIncrement
          << "\nAt market of scale: " << lMarketScale
          << "\nValid From: " << OTTimeGetSecondsFromTime(VALID_FROM)
          << "  To: " << OTTimeGetSecondsFromTime(VALID_TO) << std::endl;
    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            ASSET_ACCOUNT_ID,
            serverID,
            OTTransaction::marketOffer,
            originType::origin_market_offer,
            openingNumber));

    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(Item::CreateItemFromTransaction(
        *transaction, Item::marketOffer, &CURRENCY_ACCOUNT_ID));

    if (false == bool(item)) {

        return output;
    }

    String tradeAttachment;
    trade.SaveContractRaw(tradeAttachment);
    item->SetAttachment(tradeAttachment);
    item->SignContract(nym);
    item->SaveContract();
    transaction->AddItem(*item.release());
    auto statement = context.Statement(*transaction);

    if (false == bool(statement)) {

        return output;
    }

    transaction->AddItem(*statement.release());
    transaction->SignContract(nym);
    transaction->SaveContract();
    Ledger ledger(nymID, ASSET_ACCOUNT_ID, serverID);
    ledger.GenerateLedger(ASSET_ACCOUNT_ID, serverID, Ledger::message);
    ledger.AddTransaction(*transaction.release());
    ledger.SignContract(nym);
    ledger.SaveContract();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        ASSET_ACCOUNT_ID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message(managed, context, *message);

    return output;
}

/// GET MARKET LIST
///
/// Connect to a specific server, as a specific Nym, and request the list of
/// markets. (Flush the buffer before calling this. Then after you make this
/// call, wait 50 ms and then pop the buffer and check the server reply for
/// success. From there you can either read the reply data directly out of the
/// reply message, or you can load it from storage (OT will probably auto-store
/// the reply to storage, for your convenience.)
CommandResult OT_API::getMarketList(ServerContext& context) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] =
        context.InitializeServerCommand(MessageType::getMarketList, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

/// GET ALL THE OFFERS ON A SPECIFIC MARKET
///
/// A specific Nym is requesting the Server to send a list of the offers on a
/// specific Market ID-- the bid/ask, and prices/amounts, basically--(up to
/// lDepth or server Max)
CommandResult OT_API::getMarketOffers(
    ServerContext& context,
    const Identifier& MARKET_ID,
    const std::int64_t& lDepth) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::getMarketOffers, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strNymID2 = String(MARKET_ID);
    message->m_lDepth = lDepth;

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

///-------------------------------------------------------
/// GET RECENT TRADES FOR A SPECIFIC MARKET ID
///
/// Most likely, ticker data will be made available through a separate ZMQ
/// instance, which will use the publisher/subscriber model to distribute ticker
/// data. From there, those privileged subscribers can distribute it via RSS,
/// store it for future analysis, display charts, etc.
///
/// (So this function is not here to usurp that purpose.)
CommandResult OT_API::getMarketRecentTrades(
    ServerContext& context,
    const Identifier& MARKET_ID) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::getMarketRecentTrades, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strNymID2 = String(MARKET_ID);

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

///-------------------------------------------------------
/// GET ALL THE ACTIVE (in Cron) MARKET OFFERS FOR A SPECIFIC NYM. (ON A
/// SPECIFIC SERVER, OBVIOUSLY.) Remember to use Flush/Call/Wait/Pop to check
/// the server reply for success or fail. Hmm for size reasons, this really will
/// have to return a list of transaction #s, and then I request them one-by-one
/// after that...
CommandResult OT_API::getNymMarketOffers(ServerContext& context) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::getNymMarketOffers, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

// Request the server to transfer from one account to another.
CommandResult OT_API::notarizeTransfer(
    ServerContext& context,
    const Identifier& ACCT_FROM,
    const Identifier& ACCT_TO,
    const Amount amount,
    const String& NOTE) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    auto account = GetOrLoadAccount(nym, ACCT_FROM, serverID, __FUNCTION__);

    // account is not owned by this function
    if (nullptr == account) {

        return output;
    }

    std::set<ServerContext::ManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber.Valid()) {
        otErr << "No transaction numbers were available. Suggest "
                 "requesting the server for one.\n";
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
          << managedNumber << std::endl;
    transactionNum = managedNumber;
    std::unique_ptr<OTTransaction> transaction(
        OTTransaction::GenerateTransaction(
            nymID,
            ACCT_FROM,
            serverID,
            OTTransaction::transfer,
            originType::not_applicable,
            transactionNum));

    if (false == bool(transaction)) {

        return output;
    }

    std::unique_ptr<Item> item(Item::CreateItemFromTransaction(
        *transaction, Item::transfer, &ACCT_TO));

    if (false == bool(item)) {

        return output;
    }

    item->SetAmount(amount);

    // The user can include a note here for the recipient.
    if (NOTE.Exists() && NOTE.GetLength() > 2) {
        item->SetNote(NOTE);
    }

    // sign the item
    item->SignContract(nym);
    item->SaveContract();
    std::unique_ptr<Ledger> inbox(account->LoadInbox(nym));
    std::unique_ptr<Ledger> outbox(account->LoadOutbox(nym));

    if (false == bool(inbox)) {
        otErr << "OT_API::notarizeTransfer: Failed loading inbox for acct: "
              << String(ACCT_FROM) << std::endl;

        return output;
    }

    if (nullptr == outbox) {
        otErr << "OT_API::notarizeTransfer: Failed loading outbox for acct: "
              << String(ACCT_FROM) << std::endl;
        return output;
    }

    // Need to setup a dummy outbox transaction (to mimic the one that
    // will be on the server side when this pending transaction is
    // actually put into the real outbox.)
    // When the server adds its own, and then compares the two, they
    // should both show the same pending transaction, in order for this
    // balance agreement to be valid..
    // Otherwise the server would have to refuse it for being inaccurate
    // (server can't sign something inaccurate!) So I throw a dummy on
    // there before generating balance statement.
    std::unique_ptr<OTTransaction> outboxTransaction(
        OTTransaction::GenerateTransaction(
            *outbox,
            OTTransaction::pending,
            originType::not_applicable,
            /* TODO pick some number that everyone agrees doesn't matter, like
             * 1. The referring-to is the important number in this case, and
             * perhaps server should update this value too before signing and
             * returning.*/
            1));  // TODO use a constant instead of '1'

    OT_ASSERT(outboxTransaction);

    outboxTransaction->SetReferenceString(String(*item));
    outboxTransaction->SetReferenceToNum(item->GetTransactionNum());
    outbox->AddTransaction(*outboxTransaction.release());
    transaction->AddItem(*item.release());

    // BALANCE AGREEMENT
    // balanceItem is signed and saved within this call. No need to do
    // that twice.
    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        amount * (-1), *transaction, context, *account, *outbox));

    OT_ASSERT(balanceItem);

    transaction->AddItem(*balanceItem.release());
    transaction->SignContract(nym);
    transaction->SaveContract();

    // set up the ledger
    Ledger ledger(nymID, ACCT_FROM, serverID);
    ledger.GenerateLedger(
        ACCT_FROM,
        serverID,
        Ledger::message);  // bGenerateLedger
                           // defaults to false,
                           // which is correct.
    ledger.AddTransaction(*transaction.release());

    // sign the ledger
    ledger.SignContract(nym);
    ledger.SaveContract();

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        OTASCIIArmor(String(ledger)),
        ACCT_FROM,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message(managed, context, *message);

    return output;
}

// Grab a copy of my nymbox (contains messages and new transaction numbers)
CommandResult OT_API::getNymbox(ServerContext& context) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] =
        context.InitializeServerCommand(MessageType::getNymbox, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::processNymbox(ServerContext& context) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();
    Ledger nymbox(nymID, nymID, serverID);

    if (false == nymbox.LoadNymbox()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed loading Nymbox: " << String(nymID) << std::endl;

        return output;
    }

    if (false == nymbox.VerifyAccount(nym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed verifying Nymbox: " << String(nymID) << std::endl;

        return output;
    }

    if (1 > nymbox.GetTransactionCount()) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Nymbox (" << String(nymID)
               << ") is empty (so, skipping processNymbox.)" << std::endl;

        requestNum = 0;
        status = SendResult::UNNECESSARY;

        return output;
    }

    Message message;
    const auto accepted =
        m_pClient->AcceptEntireNymbox(nymbox, context, message);
    requestNum = atoi(message.m_strRequestNum.Get());

    if (false == accepted) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed trying to accept the "
              << "entire Nymbox. (And no, it's not empty.)" << std::endl;

        return output;
    }

    context.LocalNymboxHash()->GetString(message.m_strNymboxHash);

    if (false == context.FinalizeServerCommand(message)) {

        return output;
    }

    result = send_message({}, context, message);

    return output;
}

CommandResult OT_API::processInbox(
    ServerContext& context,
    const Identifier& accountID,
    const String& ACCT_LEDGER) const
{
    const auto& nymID = context.Nym()->ID();
    const auto& serverID = context.Server();
    rLock lock(lock_callback_({nymID.str(), serverID.str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    auto account = GetOrLoadAccount(nym, accountID, serverID, __FUNCTION__);

    if (nullptr == account) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load account"
              << std::endl;

        return output;
    }

    std::unique_ptr<Ledger> inbox(LoadInbox(serverID, nymID, accountID));

    if (false == bool(inbox)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load inbox"
              << std::endl;

        return output;
    }

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::processInbox,
        OTASCIIArmor(ACCT_LEDGER),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);
    const auto cheques =
        extract_cheques(nymID, accountID, serverID, ACCT_LEDGER, *inbox);

    for (const auto& cheque : cheques) {
        OT_ASSERT(cheque)

        const auto workflowUpdated =
            workflow_.FinishCheque(*cheque, *message, reply.get());

        if (workflowUpdated) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Successfully updated workflow" << std::endl;
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to update workflow"
                  << std::endl;
        }
    }

    return output;
}

CommandResult OT_API::registerInstrumentDefinition(
    ServerContext& context,
    const proto::UnitDefinition& THE_CONTRACT) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto contract = wallet_.UnitDefinition(THE_CONTRACT);

    if (false == bool(contract)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Invalid verifying asset contract." << std::endl;

        return output;
    }

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::registerInstrumentDefinition, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    OTIdentifier newID = contract->ID();
    newID->GetString(message->m_strInstrumentDefinitionID);
    message->m_ascPayload.SetData(
        proto::ProtoAsData<proto::UnitDefinition>(contract->PublicContract()));

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::getInstrumentDefinition(
    ServerContext& context,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::getInstrumentDefinition, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strInstrumentDefinitionID = String(INSTRUMENT_DEFINITION_ID);

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::getMint(
    ServerContext& context,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto unitDefinition = wallet_.UnitDefinition(INSTRUMENT_DEFINITION_ID);

    if (false == bool(unitDefinition)) {

        return output;
    }

    auto[newRequestNumber, message] =
        context.InitializeServerCommand(MessageType::getMint, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strInstrumentDefinitionID = String(INSTRUMENT_DEFINITION_ID);

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

// Sends a list of instrument definition ids to the server, which replies with a
// list of the actual receipts with the issuer's signature on them, from when
// the currency was issued.
//
// So you can tell for sure whether or not they are actually issued on that
// server.
//
// Map input: key is instrument definition ID, and value is blank (server reply
// puts issuer's receipts in that spot.)
CommandResult OT_API::queryInstrumentDefinitions(
    ServerContext& context,
    const OTASCIIArmor& ENCODED_MAP) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::queryInstrumentDefinitions, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_ascPayload = ENCODED_MAP;

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

// Create an asset account for a certain notaryID, NymID, and Instrument
// Definition ID. These accounts are where users actually store their digital
// assets of various types. Account files are stored on user's computer, signed
// by notary server. Server also maintains its own copy. Users can create an
// unlimited number of accounts for any instrument definition that they choose.
CommandResult OT_API::registerAccount(
    ServerContext& context,
    const Identifier& INSTRUMENT_DEFINITION_ID) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto contract = wallet_.UnitDefinition(INSTRUMENT_DEFINITION_ID);

    if (false == bool(contract)) {

        return output;
    }

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::registerAccount, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strInstrumentDefinitionID = String(INSTRUMENT_DEFINITION_ID);

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::deleteAssetAccount(
    ServerContext& context,
    const Identifier& ACCOUNT_ID) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& serverID = context.Server();
    auto account = GetOrLoadAccount(nym, ACCOUNT_ID, serverID, __FUNCTION__);

    if (nullptr == account) {

        return output;
    }

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::unregisterAccount, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strAcctID = String(ACCOUNT_ID);

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

bool OT_API::DoesBoxReceiptExist(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,      // Unused here for now, but still convention.
    const Identifier& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then pass
                                   // NYM_ID in this field also.
    std::int32_t nBoxType,         // 0/nymbox, 1/inbox, 2/outbox
    const TransactionNumber& lTransactionNum) const
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));

    // static
    return VerifyBoxReceiptExists(
        NOTARY_ID,
        NYM_ID,      // Unused here for now, but still convention.
        ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then pass NYM_ID in this
                     // field also.
        nBoxType,    // 0/nymbox, 1/inbox, 2/outbox
        lTransactionNum);
}

CommandResult OT_API::getBoxReceipt(
    ServerContext& context,
    const Identifier& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then pass
                                   // NYM_ID in this field also.
    std::int32_t nBoxType,         // 0/nymbox, 1/inbox, 2/outbox
    const TransactionNumber& lTransactionNum) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Server();

    if (nymID != ACCOUNT_ID) {
        auto account =
            GetOrLoadAccount(nym, ACCOUNT_ID, serverID, __FUNCTION__);

        if (nullptr == account) {

            return output;
        }
    }

    auto[newRequestNumber, message] =
        context.InitializeServerCommand(MessageType::getBoxReceipt, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strAcctID = String(ACCOUNT_ID);
    message->m_lDepth = static_cast<std::int64_t>(nBoxType);
    message->m_lTransactionNum = lTransactionNum;

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::getAccountData(
    ServerContext& context,
    const Identifier& accountID) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& serverID = context.Server();
    auto account = GetOrLoadAccount(nym, accountID, serverID, __FUNCTION__);

    if (nullptr == account) {

        return output;
    }

    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::getAccountData, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strAcctID = String(accountID);

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::usageCredits(
    ServerContext& context,
    const Identifier& NYM_ID_CHECK,
    std::int64_t lAdjustment) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::usageCredits, NYM_ID_CHECK, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_lDepth = lAdjustment;

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

// Request a user's public key based on Nym ID included with the request. (If
// you want to send him cash or a check, your wallet will encrypt portions of
// the tokens, etc, to the Nym of the recipient.)
CommandResult OT_API::checkNym(
    ServerContext& context,
    const Identifier& targetNymID) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::checkNym, targetNymID, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::registerContract(
    ServerContext& context,
    const ContractType TYPE,
    const Identifier& CONTRACT) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::registerContract, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->enum_ = static_cast<std::uint8_t>(TYPE);

    switch (TYPE) {
        case (ContractType::NYM): {
            const auto contract = wallet_.Nym(CONTRACT);

            if (!contract) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Nym not found: " << String(CONTRACT) << std::endl;

                return output;
            }

            message->m_ascPayload = proto::ProtoAsData(contract->asPublicNym());
        } break;
        case (ContractType::SERVER): {
            const auto contract = wallet_.Server(CONTRACT);

            if (!contract) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Server not found: " << String(CONTRACT)
                      << std::endl;

                return output;
            }

            message->m_ascPayload =
                proto::ProtoAsData(contract->PublicContract());
        } break;
        case (ContractType::UNIT): {
            const auto contract = wallet_.UnitDefinition(CONTRACT);

            if (!contract) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unit definition not found: " << String(CONTRACT)
                      << std::endl;

                return output;
            }

            message->m_ascPayload = proto::ProtoAsData(contract->Contract());
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid contract type."
                  << std::endl;

            return output;
        }
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::sendNymObject(
    ServerContext& context,
    std::unique_ptr<Message>& message,
    const Identifier& recipientNymID,
    const PeerObject& object,
    const RequestNumber provided) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, request] = context.InitializeServerCommand(
        MessageType::sendNymMessage, recipientNymID, provided);
    message.reset(request.release());
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    String plaintext = proto::ProtoAsArmored(object.Serialize(), "PEER OBJECT");
    OTEnvelope theEnvelope;
    auto recipient = wallet_.Nym(recipientNymID);

    if (false == bool(recipient)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Recipient Nym credentials not found in "
              << "local storage. DOWNLOAD IT FROM THE SERVER FIRST, BEFORE "
              << "CALLING THIS FUNCTION." << std::endl;

        return output;
    }

    if (!theEnvelope.Seal(*recipient, plaintext)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed sealing envelope."
              << std::endl;

        return output;
    }

    if (!theEnvelope.GetCiphertext(message->m_ascPayload)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed sealing envelope."
              << std::endl;

        return output;
    }

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

/// WARNING: Make sure you download the public Nym of the recipient before
/// calling this function. Just because you have someone's Nym ID doesn't mean
/// you have his public key. Make sure you can load him up, and if you can't
/// then download him, THEN call this function.
CommandResult OT_API::sendNymMessage(
    ServerContext& context,
    const Identifier& recipientNymID,
    const std::string& THE_MESSAGE,
    Identifier& messageID) const
{
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto object = PeerObject::Create(context.Nym(), THE_MESSAGE);

    if (false == bool(object)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to create peer object"
              << std::endl;

        return output;
    }

    std::unique_ptr<Message> request{nullptr};
    output =
        sendNymObject(context, request, recipientNymID, *object, requestNum);

    if (SendResult::VALID_REPLY != status) {

        return output;
    }

    OT_ASSERT(reply)

    if (false == reply->m_bSuccess) {

        return output;
    }

    // store a copy in the outmail.
    std::unique_ptr<Message> sent(new Message);

    OT_ASSERT(sent);

    sent->m_strCommand = "outmailMessage";
    sent->m_strNymID = String(nymID);
    sent->m_strNymID2 = String(recipientNymID);
    sent->m_strNotaryID = String(context.Server());
    sent->m_strRequestNum.Format("%" PRId64, requestNum);
    auto copy = PeerObject::Create(nullptr, THE_MESSAGE);

    OT_ASSERT(copy);

    String plaintext = proto::ProtoAsArmored(copy->Serialize(), "PEER OBJECT");
    OTEnvelope theEnvelope;

    if (!theEnvelope.Seal(nym, plaintext)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed sealing envelope."
              << std::endl;

        return output;
    }

    if (!theEnvelope.GetCiphertext(sent->m_ascPayload)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed sealing envelope."
              << std::endl;

        return output;
    }

    sent->SignContract(nym);
    sent->SaveContract();
    messageID = Identifier::Factory(
        activity_.Mail(nymID, *sent, StorageBox::MAILOUTBOX));

    return output;
}

/// UPDATE: Sometimes you want to send something to yourself, meaning just put a
/// copy in your outpayments box, without sending anything to anyone.
/// (Specifically,
/// after a withdrawVoucher is performed, you want to save a copy in your outbox
/// since the transaction number on it is signed out to you.)
/// So I'm updating this function so that if NYM_ID and NYM_ID_RECIPIENT are the
/// same, it puts a copy in your outpayment box, without sending anything at
/// all.
///
/// WARNING: Make sure you download the public Nym of the recipient before
/// calling this function. Just because you have someone's Nym ID doesn't mean
/// you have his public key. Make sure you can load him up, and if you can't
/// then download him, THEN call this function.
///
/// pINSTRUMENT_FOR_SENDER
/// This is only used for cash purses. It's a copy of the same purse that's in
/// THE_INSTRUMENT, except all the tokens are already encrypted to the sender's
/// public key, instead of the recipient's public key (as THE_INSTRUMENT is.)
/// This is what we put in the sender's outpayments, so he can retrieve those
/// tokens if he needs to.
///
CommandResult OT_API::sendNymInstrument(
    ServerContext& context,
    std::unique_ptr<Message>& request,
    const Identifier& recipientNymID,
    const OTPayment& instrument,
    bool storeOutpayment,
    const OTPayment* senderCopy) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto recipientNym = wallet_.Nym(recipientNymID);

    if (false == bool(recipientNym)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Recipient Nym public key not "
                 "found in local storage. DOWNLOAD IT FROM THE SERVER FIRST, "
                 "BEFORE CALLING THIS FUNCTION.\n";

        return output;
    }

    const OTAsymmetricKey& recipientPubkey = recipientNym->GetPublicEncrKey();
    String strInstrumentForRecipient;
    String strInstrumentForSender;
    const bool bGotPaymentContents =
        instrument.GetPaymentContents(strInstrumentForRecipient);

    if (!bGotPaymentContents) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed attempt to send a blank payment.\n";

        return output;
    }

    const bool bGotSenderPmntContents =
        (nullptr == senderCopy)
            ? false
            : senderCopy->GetPaymentContents(strInstrumentForSender);

    const String& strInstrumentForLocalCopy{
        (bGotSenderPmntContents ? strInstrumentForSender
                                : strInstrumentForRecipient)};

    // REMOVE OLDER DUPLICATE (if applicable) FROM OUTPAYMENTS BOX.
    if ((false == instrument.IsPurse()) && storeOutpayment) {
        auto nymfile = context.mutable_Nymfile(__FUNCTION__);
        std::int64_t lInstrumentOpeningNum{0};
        const bool bGotTransNum =
            instrument.GetOpeningNum(lInstrumentOpeningNum, nymID);

        if (bGotTransNum) {
            nymfile.It().RemoveOutpaymentsByTransNum(lInstrumentOpeningNum);
        }
    }

    Message theMessage;
    theMessage.m_strCommand = "sendNymInstrument";
    theMessage.m_strNymID = String(nymID);
    theMessage.m_strNymID2 = String(recipientNymID);
    theMessage.m_strNotaryID = String(context.Server());

    // ADD OUTPAYMENT COPY, and _then_ SEND if that was successful.
    // That way if send fails, at least you still already have your
    // own outgoing copy, and thus retain the theoretical ability
    // to later attempt a re-send.
    std::unique_ptr<Message> pMessageLocalCopy(new Message);
    pMessageLocalCopy->m_strCommand = "outpaymentsMessage";
    pMessageLocalCopy->m_strNymID = String(nymID);
    pMessageLocalCopy->m_strNymID2 = String(recipientNymID);
    pMessageLocalCopy->m_strNotaryID = String(context.Server());
    pMessageLocalCopy->m_ascPayload.SetString(strInstrumentForLocalCopy);

    // We only actually SEND if sender and recipient are different nyms.
    // (If they're the same Nym, we instead only save a copy in the outbox.)
    if (nymID != recipientNymID) {
        bool bSendIt{false};
        // Grab the requestNum here from the context. sendNymObject
        // doesn't grab its own new request number if a non-zero one is
        // already passed in. That way we can set it on our outpayments
        // copy, and save that, before trying to send the message.
        requestNum = context.Request();
        pMessageLocalCopy->m_strRequestNum.Format("%" PRId64, requestNum);
        theMessage.m_strRequestNum.Format("%" PRId64, requestNum);
        OTEnvelope theEnvelope;
        const bool encrypted =
            theEnvelope.Seal(recipientPubkey, strInstrumentForRecipient) &&
            theEnvelope.GetCiphertext(theMessage.m_ascPayload);

        if (encrypted) {
            context.IncrementRequest();
            theMessage.SignContract(nym);
            theMessage.SaveContract();

            if (storeOutpayment) {
                // Back to the outpayments message...
                // (We may want it saved in the outpayment box, before
                // the reply from the above message arrives. Actually
                // that might be wrong, since we care more about the
                // receipt from the recipient depositing the instrument,
                // then we do about the reply for the sendInstrument
                // itself. Anyway, better safe than sorry...)
                pMessageLocalCopy->SignContract(nym);
                pMessageLocalCopy->SaveContract();
                auto nymfile = context.mutable_Nymfile(__FUNCTION__);
                nymfile.It().AddOutpayments(*(pMessageLocalCopy.release()));
            }

            bSendIt = true;
        }

        if (bSendIt) {
            const String strMessageForRecipient(theMessage);
            // Create the peer object we'll be sending to the recipient.
            const auto object = PeerObject::Create(
                context.Nym(),
                strMessageForRecipient.Get(),
                true);  // isPayment=true on creation.

            OT_NEW_ASSERT_MSG(
                true == bool(object), "Failed trying to create a PeerObject.");

            output = sendNymObject(
                context, request, recipientNymID, *object, requestNum);
        }
    } else {
        // You may be wondering why this code seems to repeat?
        // The answer is because above, it needs to happen BEFORE
        // the message is sent, SINCE the processing of the server
        // reply may expect the outpayments copy to already exist.
        //
        // (NOTE: That may actually not be true. That is more true for
        // when the receipt comes in, from the recipient depositing
        // the cheque, versus the server reply to the sendInstrument
        // itself. Anyway...)
        //
        // Whereas here, it needs to happen in the case where the
        // message is NOT sent. (USER as RECIPIENT means "just put
        // a copy in my outpayments box", so nothing needs to be sent.)
        //
        // Notice if it DOES happen BEFORE, then we want to properly
        // add the request number to it (even though we apparently
        // don't use the request number on outpayments messages).
        // Whereas here, we don't HAVE a request number, (since nothing
        // was even sent) so it just gets set to 0 instead, which is also
        // what's returned.
        requestNum = 0;

        if (storeOutpayment) {
            pMessageLocalCopy->m_strRequestNum.Format("%" PRId64, requestNum);
            pMessageLocalCopy->SignContract(nym);
            pMessageLocalCopy->SaveContract();
            auto nymfile = context.mutable_Nymfile(__FUNCTION__);
            nymfile.It().AddOutpayments(*(pMessageLocalCopy.release()));
        }

        status = SendResult::UNNECESSARY;
    }

    return output;
}

// Returns number of transactions within, or -1 for error.
//
// This function seems strangely unnecessary and bare, but
// if you look at the rest of the ledger functions here,
// they all line up.
//
std::int32_t OT_API::Ledger_GetCount(const Ledger& ledger) const
{
    return ledger.GetTransactionCount();
}

std::set<std::int64_t> OT_API::Ledger_GetTransactionNums(
    const Ledger& ledger) const
{
    return ledger.GetTransactionNums();
}

// Creates a new 'response' ledger, set up with the right Notary ID, etc, so you
// can add the 'response' items to it, one by one. (Pass in the original ledger
// that you are responding to, as it uses the data from it to set up the
// response.)
// The original ledger is your inbox. Inbox receipts are the only things you
// would ever create a "response" to, as part of your "process inbox" process.
//
OT_API::ProcessInbox OT_API::Ledger_CreateResponse(
    const Identifier& theNotaryID,
    const Identifier& theNymID,
    const Identifier& theAccountID) const
{
    OT_VERIFY_OT_ID(theNotaryID);
    OT_VERIFY_OT_ID(theNymID);
    OT_VERIFY_OT_ID(theAccountID);

    auto nym = wallet_.Nym(theNymID);  // Sanity check.

    OT_ASSERT(nym);

    auto context = wallet_.ServerContext(theNymID, theNotaryID);

    if (false == bool(context)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << theNymID.str()
              << " is not registered on " << theNotaryID.str() << std::endl;

        OT_FAIL;
    }

    auto response = CreateProcessInbox(theAccountID, *context);

    // response is of type "ProcessInbox":
    //
    // typedef std::pair<std::unique_ptr<Ledger>,
    //                  std::unique_ptr<Ledger>> ProcessInbox;
    // 'First' is the "processInbox" message ledger we're CREATING.
    // 'Second' is the original inbox that it's RESPONDING TO.
    // ------------------------------------------
    // "processInbox" is a unique_ptr<Ledger>. It's the request ledger,
    // which is the client's reply to the nymbox contents, signing to
    // officially "receive" those contents so the server can erase them
    // out of the box.
    //
    auto& processInbox = std::get<0>(response);
    // ------------------------------------------
    // The inbox we're responding to in our client request.
    //
    // TODO: Almost certainly we're supposed to be deleting
    // the "inbox" variable here. It comes from a function that
    // explicitly warns "caller needs to delete".
    // Yet there are places that call CreateProcessInbox, which
    // do not perform any such delete...
    //
    auto& inbox = std::get<1>(response);
    // delete inbox; inbox = nullptr;
    // TODO THIS SHOULD BE UNCOMMENTED, OR IT'S A MEMORY LEAK.
    // UPDATE: Actually I switched this to a unique pointer to fix all that, so
    // should no longer be necessary. I'll test, then remove this comment.
    // ------------------------------------------
    if (processInbox && inbox) {
        return response;
    }
    return {};
}

// Lookup a transaction (from within a ledger) based on index.
// May return an abbreviated version, if that's all we have.
// Returns a full version if possible, even if it needs to load
// it up from local storage.
//
// Do NOT delete the return value, since the transaction is already
// owned by ledger.
//
OTTransaction* OT_API::Ledger_GetTransactionByIndex(
    Ledger& ledger,
    const std::int32_t& nIndex) const  // returns transaction by index (from
                                       // ledger)
{
    OT_VERIFY_BOUNDS(nIndex, 0, ledger.GetTransactionCount());

    OTTransaction* transaction = ledger.GetTransactionByIndex(nIndex);

    if (nullptr == transaction) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failure: good index but uncovered empty pointer. Index: "
              << nIndex << "\n";
        return nullptr;  // Weird. Should never happen.
    }

    const std::int64_t lTransactionNum = transaction->GetTransactionNum();
    // At this point, I actually have the transaction pointer, so let's return
    // it in string form...

    // Update: for transactions in ABBREVIATED form, the string is empty, since
    // it has never actually been signed (in fact the whole point with
    // abbreviated transactions in a ledger is that they take up very little
    // room, and have no signature of their own, but exist merely as XML tags on
    // their parent ledger.)
    //
    // THEREFORE I must check to see if this transaction is abbreviated and if
    // so, sign it in order to force the UpdateContents() call, so the
    // programmatic user of this API will be able to load it up.
    //
    if (transaction->IsAbbreviated()) {
        transaction = nullptr;

        // const bool bLoadedBoxReceipt =
        ledger.LoadBoxReceipt(static_cast<std::int64_t>(lTransactionNum));
        transaction = ledger.GetTransaction(lTransactionNum);
        if (nullptr == transaction) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": good index but uncovered null pointer "
                     "after trying to load full version of "
                     "receipt (from abbreviated). Probably haven't "
                     "yet downloaded the box receipt. "
                     "Index: "
                  << nIndex << "\n";
            return nullptr;
        }
    }
    return transaction;
}

// Returns transaction by ID (transaction numbers are std::int64_t ints.
//
// Note: If this function returns an abbreviated transaction instead of the
// full version, then you probably just need to download it. (The box receipts
// are stored in separate files and downloaded separately as well.)
//
// Returns a full version if possible, even if it needs to load
// the box receipt up from local storage.
//
// Do NOT delete the return value, since the transaction is owned
// by the ledger.
//
OTTransaction* OT_API::Ledger_GetTransactionByID(
    Ledger& ledger,
    const std::int64_t& TRANSACTION_NUMBER) const
{
    OT_VERIFY_MIN_BOUND(TRANSACTION_NUMBER, 1);

    OTTransaction* transaction = ledger.GetTransaction(TRANSACTION_NUMBER);
    // No need to cleanup this transaction, the ledger owns it already.

    if (nullptr == transaction) {
        otErr << OT_METHOD << __FUNCTION__
              << ": No transaction found in ledger with that number : "
              << TRANSACTION_NUMBER << ".\n";
        return nullptr;  // Maybe he was just looking; this isn't
                         // necessarily an error.
    }

    // At this point, I actually have the transaction pointer, so
    // let's return it...
    //
    const std::int64_t lTransactionNum = transaction->GetTransactionNum();
    OT_ASSERT(lTransactionNum == TRANSACTION_NUMBER);

    // Update: for transactions in ABBREVIATED form, the string is empty,
    // since it has never actually been signed (in fact the whole point
    // with abbreviated transactions in a ledger is that they take up very
    // little room, and have no signature of their own, but exist merely
    // as XML tags on their parent ledger.)
    //
    if (transaction->IsAbbreviated()) {
        // Update:
        // In case the LoadBoxReceipt fails in such a way that transaction
        // is no longer a good pointer, we pre-emptively set it here to null.
        //
        transaction = nullptr;

        // First we see if we are able to load the full version of this box
        // receipt.
        // (Perhaps it has already been downloaded sometime in the past, and
        // simply
        // needs to be loaded up. Worth a shot.)
        //
        // const bool bLoadedBoxReceipt =
        ledger.LoadBoxReceipt(static_cast<std::int64_t>(
            lTransactionNum));  // I still want it to send
                                // the abbreviated form, if
                                // this fails.

        // Grab this pointer again, since the object was re-instantiated
        // in the case of a successful LoadBoxReceipt.
        // If on the other hand, it's still the abbreviated one from before,
        // and the LoadBoxReceipt failed (perhaps because it hasn't been
        // downloaded yet) well that's fine because here we make sure the
        // pointer is set to the abbreviated one in that case, OR NULL, if
        // it's not available for whatever reason.
        //
        transaction =
            ledger.GetTransaction(static_cast<std::int64_t>(lTransactionNum));

        if (nullptr == transaction) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": good ID, but uncovered nullptr "
                     "after trying to load full version of "
                     "receipt (from abbreviated.) Probably "
                     "just need to download the box receipt...\n";

            return nullptr;  // Weird.
        }
    }
    return transaction;
}

// OTAPI_Exec::Ledger_GetInstrument (by index)
//
// Lookup a financial instrument (from within a transaction that is inside
// a paymentInbox ledger) based on index.
/*
sendNymInstrument does this:
-- Puts instrument (a contract string) as encrypted Payload on an OTMessage(1).
-- Also puts instrument (same contract string) as CLEAR payload on an
OTMessage(2).
-- (1) is sent to server, and (2) is added to Outpayments messages.
-- (1) gets added to recipient's Nymbox as "in ref to" string on a
"instrumentNotice" transaction.
-- When recipient processes Nymbox, the "instrumentNotice" transaction
(containing (1) in its "in ref to"
field) is copied and added to the recipient's paymentInbox.
-- When recipient iterates through paymentInbox transactions, they are ALL
"instrumentNotice" OTMessages.
Each transaction contains an OTMessage in its "in ref to" field, and that
OTMessage object contains an
encrypted payload of the instrument itself (a contract string.)
-- When sender gets Outpayments contents, the original instrument (contract
string) is stored IN THE
CLEAR as payload on an OTMessage.

THEREFORE:
TO EXTRACT INSTRUMENT FROM PAYMENTS INBOX:
-- Iterate through the transactions in the payments inbox.
-- (They should all be "instrumentNotice" transactions.)
-- Each transaction contains (1) OTMessage in the "in ref to" field, which in
turn contains an encrypted
instrument in the messagePayload field.
-- *** Therefore, this function, based purely on ledger index (as we iterate)
extracts the
OTMessage from the Transaction "in ref to" field (for the transaction at that
index), then decrypts
the payload on that message and returns the decrypted cleartext.
*/

// DONE:  Move most of the code in the below function into
// OTLedger::GetInstrument. UPDATE: Which is now "Helpers.GetInstrument"
//
// DONE: Finish writing OTClient::ProcessDepositResponse

std::unique_ptr<OTPayment> OT_API::Ledger_GetInstrument(
    const Identifier& theNymID,
    const Ledger& ledger,
    const std::int32_t& nIndex) const  // returns financial instrument by index.
{
    OT_VERIFY_OT_ID(theNymID);
    OT_VERIFY_BOUNDS(nIndex, 0, ledger.GetTransactionCount());

    auto nym = wallet_.Nym(theNymID);
    OT_ASSERT(nym);
    // ------------------------------------
    std::unique_ptr<OTPayment> pPayment{GetInstrumentByIndex(
        *nym,
        nIndex,
        const_cast<Ledger&>(ledger)  // Todo justus has fix for this const_cast.
        )};
    // ------------------------------------
    if ((!pPayment) || !pPayment->IsValid()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": ledger.GetInstrument "
                 "either returned nullptr, or an invalid instrument.\n";

        return {};
    }
    // ------------------------------------
    return pPayment;
}

// returns financial instrument, by transNum of the
// receipt (that contains the instrument) in the ledger.
//
std::unique_ptr<OTPayment> OT_API::Ledger_GetInstrumentByReceiptID(
    const Identifier& theNymID,
    const Ledger& ledger,
    const std::int64_t& lReceiptId) const
{
    OT_VERIFY_OT_ID(theNymID);
    OT_VERIFY_MIN_BOUND(lReceiptId, 1);

    auto nym = wallet_.Nym(theNymID);
    OT_ASSERT(nym);
    // ------------------------------------
    std::unique_ptr<OTPayment> pPayment{GetInstrumentByReceiptID(
        *nym,
        lReceiptId,
        const_cast<Ledger&>(ledger)  // Todo justus has fix for this const_cast.
        )};
    // ------------------------------------
    if ((!pPayment) || !pPayment->IsValid()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": ledger.GetInstrumentByReceiptID either "
                 "returned nullptr, or an invalid instrument.\n";

        return {};
    }
    // ------------------------------------
    return pPayment;
}

// Returns a transaction number, or -1 for error.
std::int64_t OT_API::Ledger_GetTransactionIDByIndex(
    const Ledger& ledger,
    const std::int32_t& nIndex) const  // returns transaction number by index.
{
    OT_VERIFY_BOUNDS(nIndex, 0, ledger.GetTransactionCount());

    //    OT_ASSERT_MSG(
    //        nIndex >= 0 && nIndex < ledger.GetTransactionCount(),
    //        "OTAPI_Exec::Ledger_GetTransactionIDByIndex: "
    //        "nIndex out of bounds.");

    std::int64_t lTransactionNumber{0};
    OTTransaction* transaction = ledger.GetTransactionByIndex(nIndex);

    if (nullptr == transaction) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Uncovered null pointer at otherwise good index: " << nIndex
              << "\n";
        return -1;
    }
    // NO NEED TO CLEANUP the transaction, since it is already "owned" by
    // ledger.
    // ----------------------------------------------
    // At this point, I actually have the transaction pointer, so let's get the
    // ID...
    lTransactionNumber = transaction->GetTransactionNum();
    if (0 >= lTransactionNumber) {  // Should never happen.
        otErr << OT_METHOD << __FUNCTION__
              << ": negative or zero transaction number on transaction object: "
              << lTransactionNumber << "\n";
        return -1;
    }
    return lTransactionNumber;
}

// Add a transaction to a ledger.
// (Returns bool for succes or failure.)
//
bool OT_API::Ledger_AddTransaction(
    const Identifier& theNymID,
    Ledger& ledger,  // ledger takes ownership of transaction.
    std::unique_ptr<OTTransaction>& transaction) const
{
    OT_VERIFY_OT_ID(theNymID);

    OT_NEW_ASSERT_MSG(
        true == bool(transaction),
        "Expected newly allocated transaction "
        "(ready to add to ledger...). Got nullptr instead.");

    auto nym = wallet_.Nym(theNymID);

    if (false == bool(nym)) {
        return false;
    }
    // --------------------------------------------------
    if (!ledger.VerifyAccount(*nym)) {
        const Identifier& theAccountID = ledger.GetPurportedAccountID();
        String strAcctID(theAccountID);
        otErr << OT_METHOD << __FUNCTION__
              << ": Error verifying ledger with Acct ID: " << strAcctID << "\n";
        return false;
    }
    // At this point, I know ledger loaded and verified successfully.
    // --------------------------------------------------
    if (!transaction->VerifyAccount(*nym)) {
        const Identifier& theAccountID = ledger.GetPurportedAccountID();
        String strAcctID(theAccountID);
        otErr << OT_METHOD << __FUNCTION__
              << ": Error verifying transaction. "
                 "Acct ID: "
              << strAcctID << "\n";
        return false;
    }
    // At this point, I know transaction verified successfully. So let's
    // add it to the ledger, save, and return updated ledger in string form.
    // --------------------------------------------------

    // todo:
    // bool OTTransactionType::IsSameAccount(const OTTransactionType& rhs)
    // const;

    // --------------------------------------------------

    ledger.AddTransaction(*(transaction.release()));  // Ledger now "owns"
                                                      // it and will handle
                                                      // cleanup.

    ledger.ReleaseSignatures();
    ledger.SignContract(*nym);
    ledger.SaveContract();

    return true;
}

// Create a 'response' transaction, that will be used to indicate my acceptance
// or rejection of another transaction. Usually an entire ledger full of these
// is sent to the server as I process the various transactions in my inbox.
//
// The original transaction is passed in, and I generate a response transaction
// based on it. Also, the response ledger is passed in, so I load it, add the
// response transaction, save it back to string, and return the string.
// (So the return string is the updated response ledger).
//
// This way, users can call this function multiple times, adding transactions
// until done.
//
bool OT_API::Transaction_CreateResponse(
    const Identifier& theNotaryID,
    const Identifier& theNymID,
    const Identifier& accountID,
    Ledger& responseLedger,
    OTTransaction& originalTransaction,  // Responding to
    const bool& BOOL_DO_I_ACCEPT) const
{
    OT_VERIFY_OT_ID(theNotaryID);
    OT_VERIFY_OT_ID(theNymID);
    OT_VERIFY_OT_ID(accountID);

    rLock lock(lock_callback_({theNymID.str(), theNotaryID.str()}));
    auto context = wallet_.mutable_ServerContext(theNymID, theNotaryID);
    const auto& nym = context.It().Nym();
    const bool validReponse = responseLedger.VerifyAccount(*nym);

    if (!validReponse) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to verify response ledger." << std::endl;

        return false;
    }
    // ----------------------------------------------
    // Make sure we're not just looking at the abbreviated version.
    OTTransaction* pOriginalTransaction{nullptr};
    std::unique_ptr<OTTransaction> theTransAngel;

    if (originalTransaction.IsAbbreviated()) {
        pOriginalTransaction = LoadBoxReceipt(
            originalTransaction, static_cast<std::int64_t>(Ledger::inbox));

        if (nullptr == pOriginalTransaction) {
            String strAcctID(accountID);
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error loading full transaction from "
                     "abbreviated version of inbox receipt. "
                     "Acct ID: "
                  << strAcctID << "\n";
            return false;
        }
        theTransAngel.reset(pOriginalTransaction);
    } else {
        pOriginalTransaction = &originalTransaction;
    }
    // Note: BELOW THIS POINT, only use pOriginalTransaction,
    // not originalTransaction.
    // ----------------------------------------------
    const bool responded = IncludeResponse(
        accountID,
        BOOL_DO_I_ACCEPT,
        context.It(),
        *pOriginalTransaction,
        responseLedger);

    if (false == responded) {
        otErr << OT_METHOD << __FUNCTION__
              << "Failed in call to IncludeResponse.\n";

        return false;
    }
    return true;
}

// (Response Ledger) LEDGER FINALIZE RESPONSE
//
// AFTER you have set up all the transaction responses, call THIS function to
// finalize them by adding a BALANCE AGREEMENT.
//
// MAKE SURE you have the latest copy of the account file, inbox file, and
// outbox file, since we will need those in here to create the balance statement
// properly.
//
// (Client software may wish to check those things, when downloaded, against the
// local copies and the local signed receipts. In this way, clients can protect
// themselves against malicious servers.)
//
bool OT_API::Ledger_FinalizeResponse(
    const Identifier& theNotaryID,
    const Identifier& theNymID,
    const Identifier& accountID,
    Ledger& responseLedger) const
{
    OT_VERIFY_OT_ID(theNotaryID);
    OT_VERIFY_OT_ID(theNymID);
    OT_VERIFY_OT_ID(accountID);

    rLock lock(lock_callback_({theNymID.str(), theNotaryID.str()}));
    auto context = wallet_.mutable_ServerContext(theNymID, theNotaryID);
    const auto& nym = context.It().Nym();
    const bool validReponse = responseLedger.VerifyAccount(*nym);

    if (!validReponse) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to verify response ledger." << std::endl;

        return false;
    }
    // -------------------------------------------------------
    std::unique_ptr<Ledger> inbox(LoadInbox(theNotaryID, theNymID, accountID));

    if (!inbox) {
        String strAcctID(accountID);
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load inbox."
                 " Acct ID: "
              << strAcctID << std::endl;

        return false;
    }
    // -------------------------------------------------------
    if (false == inbox->VerifyAccount(*nym)) {
        String strAcctID(accountID);
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to verify inbox."
                 " Acct ID: "
              << strAcctID << std::endl;

        return false;
    }
    // -------------------------------------------------------

    // todo:
    //    bool OTTransactionType::IsSameAccount(const OTTransactionType& rhs)
    //    const;

    // This function performs any necessary signing/saving of
    // the processInbox transaction and the request ledger that
    // contains it.
    //
    return FinalizeProcessInbox(
        accountID, context.It(), responseLedger, *inbox);
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

CommandResult OT_API::registerNym(ServerContext& context) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] =
        context.InitializeServerCommand(MessageType::registerNym, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    const auto& nym = *context.Nym();

    if (nym.GetMasterCredentialCount() < 1) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid nym" << std::endl;

        return output;
    }

    message->m_ascPayload.SetData(proto::ProtoAsData(nym.asPublicNym()));

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    if (SendResult::VALID_REPLY == status) {
        OT_ASSERT(reply);

        if (reply->m_bSuccess) {
            context.SetRevision(nym.Revision());
        }
    }

    return output;
}

CommandResult OT_API::unregisterNym(ServerContext& context) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    Message message;
    requestNum = m_pClient->ProcessUserCommand(
        MessageType::unregisterNym, context, message);

    if (0 < requestNum) {
        result = send_message({}, context, message);
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Error in "
              << "m_pClient->ProcessUserCommand()" << std::endl;
    }

    return output;
}

NetworkReplyMessage OT_API::send_message(
    const std::set<ServerContext::ManagedNumber>& pending,
    ServerContext& context,
    const Message& message) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));

    m_pClient->QueueOutgoingMessage(message);
    auto result = context.Connection().Send(message);

    if (SendResult::VALID_REPLY == result.first) {
        m_pClient->processServerReply(pending, context, result.second);
    }

    return result;
}

CommandResult OT_API::initiatePeerRequest(
    ServerContext& context,
    const Identifier& recipient,
    const std::shared_ptr<PeerRequest>& peerRequest) const
{
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();

    if (false == bool(peerRequest)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid request." << std::endl;

        return output;
    }

    const bool saved =
        wallet_.PeerRequestCreate(nymID, peerRequest->Contract());

    if (false == saved) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to save request in wallet." << std::endl;

        return output;
    }

    const auto itemID = peerRequest->ID();
    auto object = PeerObject::Create(peerRequest, peerRequest->Version());

    if (false == bool(object)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to create peer object"
              << std::endl;
        wallet_.PeerRequestCreateRollback(nymID, itemID);

        return output;
    }

    std::unique_ptr<Message> request{nullptr};
    output = sendNymObject(context, request, recipient, *object, requestNum);

    if (SendResult::VALID_REPLY != status) {
        wallet_.PeerRequestCreateRollback(nymID, itemID);
    }

    return output;
}

CommandResult OT_API::initiatePeerReply(
    ServerContext& context,
    const Identifier& recipient,
    const Identifier& request,
    const std::shared_ptr<PeerReply>& peerReply) const
{
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();

    if (false == bool(peerReply)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid reply." << std::endl;

        return output;
    }

    std::time_t time{0};
    auto serializedRequest = wallet_.PeerRequest(
        nymID, request, StorageBox::INCOMINGPEERREQUEST, time);

    if (false == bool(serializedRequest)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load request."
              << std::endl;

        return output;
    }

    auto recipientNym = wallet_.Nym(recipient);
    std::unique_ptr<PeerRequest> instantiatedRequest(
        PeerRequest::Factory(recipientNym, *serializedRequest));

    if (false == bool(instantiatedRequest)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to instantiate request."
              << std::endl;

        return output;
    }

    const bool saved = wallet_.PeerReplyCreate(
        nymID, *serializedRequest, peerReply->Contract());

    if (false == saved) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to save reply in wallet." << std::endl;

        return output;
    }

    const auto itemID = peerReply->ID();
    auto pRequest = std::shared_ptr<PeerRequest>(instantiatedRequest.release());
    const auto version = pRequest->Version() > peerReply->Version()
                             ? pRequest->Version()
                             : peerReply->Version();
    auto object = PeerObject::Create(pRequest, peerReply, version);

    if (false == bool(object)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to create peer object"
              << std::endl;
        wallet_.PeerReplyCreateRollback(nymID, request, itemID);

        return output;
    }

    std::unique_ptr<Message> requestMessage{nullptr};
    output =
        sendNymObject(context, requestMessage, recipient, *object, requestNum);

    if (SendResult::VALID_REPLY != status) {
        wallet_.PeerReplyCreateRollback(nymID, request, itemID);
    }

    return output;
}

CommandResult OT_API::requestAdmin(
    ServerContext& context,
    const std::string& PASSWORD) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] =
        context.InitializeServerCommand(MessageType::requestAdmin, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strAcctID = PASSWORD.c_str();

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

CommandResult OT_API::serverAddClaim(
    ServerContext& context,
    const std::string& section,
    const std::string& type,
    const std::string& value,
    const bool primary) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    CommandResult output{};
    auto & [ requestNum, transactionNum, result ] = output;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();
    auto[newRequestNumber, message] =
        context.InitializeServerCommand(MessageType::addClaim, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) {

        return output;
    }

    message->m_strNymID2 = section.c_str();
    message->m_strInstrumentDefinitionID = type.c_str();
    message->m_strAcctID = value.c_str();
    message->m_bBool = primary;

    if (false == context.FinalizeServerCommand(*message)) {

        return output;
    }

    result = send_message({}, context, *message);

    return output;
}

ConnectionState OT_API::CheckConnection(const std::string& server) const
{
    return zeromq_.Status(server);
}

std::string OT_API::AddChildKeyCredential(
    const Identifier& nymID,
    const Identifier& masterID,
    const NymParameters& nymParameters) const
{
    std::string output;
    auto nymdata = wallet_.mutable_Nym(nymID);

    output = nymdata.AddChildKeyCredential(masterID, nymParameters);

    return output;
}

std::unique_ptr<proto::ContactData> OT_API::GetContactData(
    const Identifier& nymID) const
{
    std::unique_ptr<proto::ContactData> output;
    const auto nym = wallet_.Nym(nymID);

    if (nym) {
        output.reset(new proto::ContactData(nym->Claims().Serialize(true)));
    }

    return output;
}

std::list<std::string> OT_API::BoxItemCount(
    const Identifier& NYM_ID,
    const StorageBox box) const
{
    const auto list = activity_.Mail(NYM_ID, box);
    std::list<std::string> output;

    for (auto& item : list) {
        output.push_back(item.first);
    }

    return output;
}

std::string OT_API::BoxContents(
    const Identifier& nymID,
    const Identifier& id,
    const StorageBox box) const
{
    return *activity_.MailText(nymID, id, box);
}

OT_API::ProcessInbox OT_API::CreateProcessInbox(
    const Identifier& accountID,
    const ServerContext& context) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    const std::string account = accountID.str();
    const auto& serverID = context.Server();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.GetConstID();
    OT_API::ProcessInbox output{};
    auto & [ processInbox, inbox ] = output;
    inbox.reset(LoadInbox(serverID, nymID, accountID));

    if (false == bool(inbox)) {
        otWarn << OT_METHOD << __FUNCTION__
               << ": Unable to load inbox for account " << account << std::endl;

        return {};
    }

    processInbox.reset(
        Ledger::GenerateLedger(nymID, accountID, serverID, Ledger::message));

    if (false == bool(processInbox)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error generating process inbox ledger for account "
              << account << std::endl;
        return {};
    }

    return output;
}

bool OT_API::IncludeResponse(
    const Identifier& accountID,
    const bool accept,
    ServerContext& context,
    OTTransaction& source,
    Ledger& response) const
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    const auto serverID = Identifier::Factory(context.Server());
    const auto type = source.GetType();

    switch (type) {
        case OTTransaction::pending:
        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt:
        case OTTransaction::transferReceipt:
        case OTTransaction::marketReceipt:
        case OTTransaction::paymentReceipt:
        case OTTransaction::finalReceipt:
        case OTTransaction::basketReceipt: {
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": wrong transaction type: " << source.GetTypeString()
                  << std::endl;

            return false;
        }
    }

    auto& nym = *context.Nym();
    auto& serverNym = context.RemoteNym();
    const bool validSource = source.VerifyAccount(serverNym);

    if (!validSource) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to verify source transaction." << std::endl;

        return false;
    }

    auto processInbox =
        get_or_create_process_inbox(accountID, context, response);

    if (nullptr == processInbox) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to find or create processInbox." << std::endl;

        return false;
    }

    const auto sourceType = source.GetType();
    String note;
    const auto originNumber = get_origin(serverID, source, note);
    const bool acceptItemAdded = add_accept_item(
        response_type(sourceType, accept),
        originNumber,
        source.GetTransactionNum(),
        note,
        nym,
        source.GetReceiptAmount(),
        *processInbox);

    if (false == acceptItemAdded) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to add response item to process inbox transaction."
              << std::endl;

        return false;
    }

    return true;
}

bool OT_API::FinalizeProcessInbox(
    const Identifier& accountID,
    ServerContext& context,
    Ledger& response,
    Ledger& inbox) const
{
    class Cleanup
    {
    public:
        Cleanup(OTTransaction& processInbox, ServerContext& context)
            : context_(context)
            , success_(false)
            , number_(processInbox.GetTransactionNum())
        {
        }
        Cleanup() = delete;

        void SetSuccess(const bool success) { success_ = success; }

        ~Cleanup()
        {
            if ((0 != number_) && (false == success_)) {
                context_.RecoverAvailableNumber(number_);
            }
        }

    private:
        ServerContext& context_;
        bool success_{false};
        TransactionNumber number_{0};
    };

    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Server().str()}));
    auto nym = context.Nym();
    auto& nymID = nym->GetConstID();
    auto& serverID = context.Server();
    auto processInbox = response.GetTransaction(OTTransaction::processInbox);

    if (nullptr == processInbox) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Response ledger does not contain processInbox."
              << std::endl;

        return false;
    }

    std::unique_ptr<Item> balanceStatement(
        processInbox->GetItem(Item::balanceStatement));

    if (true == bool(balanceStatement)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": this response has already been finalized." << std::endl;

        return false;
    }

    // Below this point, any failure will result in the transaction number being
    // recovered
    Cleanup cleanup(*processInbox, context);
    auto account = GetAccount(accountID, __FUNCTION__);

    if (nullptr == account) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load account."
              << std::endl;

        return false;
    }

    bool boxesLoaded = true;
    Ledger outbox(nymID, accountID, serverID);
    boxesLoaded &= (boxesLoaded && outbox.LoadOutbox());
    boxesLoaded &= (boxesLoaded && outbox.VerifyAccount(*nym));

    if (!boxesLoaded) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load or verify outbox for account "
              << accountID.str() << std::endl;

        return false;
    }

    bool allFound{true};
    Amount totalAccepted{0};
    std::set<TransactionNumber> issuedClosing{};
    std::set<TransactionNumber> inboxRemoving{};

    for (auto& acceptItem : processInbox->GetItemList()) {
        OT_ASSERT(acceptItem);

        const auto inboxNumber = acceptItem->GetReferenceToNum();
        auto inboxItem = inbox.GetTransaction(inboxNumber);

        if (nullptr == inboxItem) {
            otErr << OT_METHOD << __FUNCTION__ << ": Expected receipt "
                  << inboxNumber << " not found." << std::endl;

            return false;
        }

        const auto originalNumber = inboxItem->GetReferenceToNum();
        const auto type = acceptItem->GetType();
        String typeDescription{};
        acceptItem->GetTypeString(typeDescription);

        switch (type) {
            case Item::acceptPending:
            case Item::acceptItemReceipt: {
                allFound &= find_standard(
                    context,
                    *acceptItem,
                    originalNumber,
                    *inboxItem,
                    inbox,
                    totalAccepted,
                    issuedClosing);
            } break;
            case Item::acceptCronReceipt:
            case Item::acceptFinalReceipt:
            case Item::acceptBasketReceipt: {
                allFound &= find_cron(
                    context,
                    *acceptItem,
                    *processInbox,
                    *inboxItem,
                    inbox,
                    totalAccepted,
                    issuedClosing);
            } break;
            default: {
                String type;
                acceptItem->GetTypeString(type);
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unexpected item type: " << type << std::endl;

                return false;
            }
        }

        inboxRemoving.insert(inboxNumber);
    }

    if (false == allFound) {
        otErr << OT_METHOD << __FUNCTION__ << ": Transactions in processInbox "
              << "message do not match actual inbox." << std::endl;

        return false;
    }

    for (const auto& remove : inboxRemoving) {
        if (!inbox.RemoveTransaction(remove)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed removing receipt from temporary inbox: "
                  << remove << std::endl;

            return false;
        }
    }

    balanceStatement.reset(inbox.GenerateBalanceStatement(
        totalAccepted,
        *processInbox,
        context,
        *account,
        outbox,
        issuedClosing));

    if (false == bool(balanceStatement)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to generate balance statement." << std::endl;

        return false;
    }

    bool output = true;
    processInbox->AddItem(*balanceStatement.release());
    processInbox->ReleaseSignatures();
    output &= (output && processInbox->SignContract(*nym));
    output &= (output && processInbox->SaveContract());

    if (output) {
        response.ReleaseSignatures();
        output &= (output && response.SignContract(*nym));
        output &= (output && response.SaveContract());
    }

    if (output) {
        cleanup.SetSuccess(true);
    }

    return output;
}

bool OT_API::find_cron(
    const ServerContext& context,
    const Item& item,
    OTTransaction& processInbox,
    OTTransaction& serverTransaction,
    Ledger& inbox,
    Amount& amount,
    std::set<TransactionNumber>& closing) const
{
    const auto type = item.GetType();

    switch (type) {
        case Item::acceptCronReceipt: {
        } break;
        case Item::acceptFinalReceipt: {
            const auto openingNumber = serverTransaction.GetReferenceToNum();
            const auto inboxCount = static_cast<std::size_t>(
                inbox.GetTransactionCountInRefTo(openingNumber));
            std::set<TransactionNumber> referenceNumbers;

            for (const auto& acceptItem : processInbox.GetItemList()) {
                OT_ASSERT(nullptr != acceptItem);

                const auto itemNumber = acceptItem->GetReferenceToNum();
                auto transaction = inbox.GetTransaction(itemNumber);

                if (nullptr == transaction) {
                    continue;
                }

                if (transaction->GetReferenceToNum() == openingNumber) {
                    referenceNumbers.insert(itemNumber);
                }
            }

            if (referenceNumbers.size() != inboxCount) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": In order to process a finalReceipt, all related "
                      << "receipts must also be processed." << std::endl;

                return false;
            }
        }  // intentional fallthrough
        case Item::acceptBasketReceipt: {
            const auto closingNumber = serverTransaction.GetClosingNum();
            const bool verified = context.VerifyIssuedNumber(closingNumber);

            if (verified) {
                closing.insert(closingNumber);
            } else {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Trying to remove closing number (" << closingNumber
                      << ") that already wasn't on my issued list."
                      << std::endl;

                return false;
            }
        } break;
        default: {
            String type;
            item.GetTypeString(type);
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unexpected item type: " << type << std::endl;

            return false;
        }
    }

    return true;
}

bool OT_API::find_standard(
    const ServerContext& context,
    const Item& item,
    const TransactionNumber number,
    OTTransaction& serverTransaction,
    Ledger& inbox,
    Amount& amount,
    std::set<TransactionNumber>& closing) const
{
    const auto& notaryID = context.Server();
    const auto referenceNum = item.GetReferenceToNum();
    const auto type = item.GetType();

    switch (type) {
        case Item::acceptPending: {
            amount += serverTransaction.GetReceiptAmount();
        } break;
        case Item::acceptItemReceipt: {
            String reference;
            serverTransaction.GetReferenceString(reference);
            std::unique_ptr<Item> original(
                Item::CreateItemFromString(reference, notaryID, number));

            if (false == bool(original)) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Unable to load original item while accepting "
                      << "item receipt: " << referenceNum << std::endl;

                return false;
            }

            TransactionNumber number{0};
            const auto originalType = original->GetType();

            switch (originalType) {
                case Item::depositCheque: {
                    String attachment;
                    original->GetAttachment(attachment);
                    Cheque cheque;

                    if (3 > attachment.GetLength()) {
                        otErr << OT_METHOD << __FUNCTION__
                              << ": Invalid attachment (cheque)." << std::endl;

                        return false;
                    }

                    if (false == cheque.LoadContractFromString(attachment)) {
                        otErr << OT_METHOD << __FUNCTION__
                              << ": Unable to instantiate cheque." << std::endl;

                        return false;
                    }

                    number = cheque.GetTransactionNum();
                } break;
                case Item::acceptPending: {
                    number = original->GetNumberOfOrigin();
                } break;
                default: {
                    String type;
                    original->GetTypeString(type);
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Unexpected original item type: " << type
                          << std::endl;

                    return false;
                }
            }

            const bool verified = context.VerifyIssuedNumber(number);

            if (verified) {
                closing.insert(number);
            } else {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Trying to remove number (" << number
                      << ") that already wasn't on my "
                      << "issued list." << std::endl;

                return false;
            }
        } break;
        default: {
            String type;
            item.GetTypeString(type);
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unexpected item type: " << type << std::endl;

            return false;
        }
    }

    return true;
}

bool OT_API::add_accept_item(
    const Item::itemType type,
    const TransactionNumber originNumber,
    const TransactionNumber referenceNumber,
    const String& note,
    const Nym& nym,
    const Amount amount,
    OTTransaction& processInbox) const
{
    std::unique_ptr<Item> acceptItem(
        Item::CreateItemFromTransaction(processInbox, type));

    if (false == bool(acceptItem)) {
        return false;
    }

    acceptItem->SetNumberOfOrigin(originNumber);
    acceptItem->SetReferenceToNum(referenceNumber);
    acceptItem->SetAmount(amount);

    if (note.Exists()) {
        acceptItem->SetNote(note);
    }

    bool output = true;
    output &= acceptItem->SignContract(nym);
    output &= (output && acceptItem->SaveContract());

    if (output) {
        processInbox.AddItem(*acceptItem.release());
    }

    return output;
}

OTTransaction* OT_API::get_or_create_process_inbox(
    const Identifier& accountID,
    ServerContext& context,
    Ledger& response) const
{
    const auto& nymID = context.Nym()->GetConstID();
    const auto& serverID = context.Server();
    auto processInbox = response.GetTransaction(OTTransaction::processInbox);

    if (nullptr == processInbox) {
        const auto number =
            context.NextTransactionNumber(MessageType::processInbox);

        if (false == number.Valid()) {
            otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID.str()
                  << " is all out of transaction numbers." << std::endl;

            return {};
        }

        otErr << OT_METHOD << __FUNCTION__ << ": Allocated transaction number "
              << number << std::endl;

        std::unique_ptr<OTTransaction> newProcessInbox(
            OTTransaction::GenerateTransaction(
                nymID,
                accountID,
                serverID,
                OTTransaction::processInbox,
                originType::not_applicable,
                number));

        if (false == bool(newProcessInbox)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Error generating processInbox transaction for AcctID: "
                  << accountID.str() << std::endl;

            return {};
        }

        // Above this line, the transaction number will be recovered
        // automatically
        number.SetSuccess(true);
        processInbox = newProcessInbox.get();
        response.AddTransaction(*newProcessInbox.release());
    }

    return processInbox;
}

Item::itemType OT_API::response_type(
    const OTTransaction::transactionType sourceType,
    const bool success) const
{
    switch (sourceType) {
        case OTTransaction::pending: {
            if (success) {
                return Item::acceptPending;
            } else {
                return Item::rejectPending;
            }
        } break;
        case OTTransaction::marketReceipt:
        case OTTransaction::paymentReceipt: {
            if (success) {
                return Item::acceptCronReceipt;
            } else {
                return Item::disputeCronReceipt;
            }
        } break;
        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt:
        case OTTransaction::transferReceipt: {
            if (success) {
                return Item::acceptItemReceipt;
            } else {
                return Item::disputeItemReceipt;
            }
        } break;
        case OTTransaction::finalReceipt: {
            if (success) {
                return Item::acceptFinalReceipt;
            } else {
                return Item::disputeFinalReceipt;
            }
        } break;
        case OTTransaction::basketReceipt: {
            if (success) {
                return Item::acceptBasketReceipt;
            } else {
                return Item::disputeBasketReceipt;
            }
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unexpected transaction type." << std::endl;

            return Item::error_state;
        }
    }
}

TransactionNumber OT_API::get_origin(
    const Identifier& notaryID,
    const OTTransaction& source,
    String& note) const
{
    const auto sourceType = source.GetType();
    TransactionNumber originNumber{0};

    switch (sourceType) {
        case OTTransaction::marketReceipt:
        case OTTransaction::paymentReceipt:
        case OTTransaction::finalReceipt:
        case OTTransaction::basketReceipt: {
            originNumber = source.GetReferenceToNum();
        } break;
        case OTTransaction::transferReceipt:
        case OTTransaction::pending:
        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt: {
            String reference;
            source.GetReferenceString(reference);

            if (false == reference.Exists()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": No reference string found on transaction."
                      << std::endl;

                return {};
            }

            std::unique_ptr<Item> original(Item::CreateItemFromString(
                reference, notaryID, source.GetReferenceToNum()));

            if (false == bool(original)) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed loading transaction item from string."
                      << std::endl;

                return {};
            }

            if (Item::request != original->GetStatus()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Wrong status on original item." << std::endl;

                return {};
            }

            const auto originalType = original->GetType();

            switch (originalType) {
                case Item::acceptPending:
                case Item::depositCheque: {
                    break;
                }
                case Item::transfer: {
                    original->GetNote(note);
                } break;
                default: {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Wrong type on original item." << std::endl;

                    return {};
                }
            }

            originNumber = original->GetNumberOfOrigin();
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unexpected transaction type in: "
                  << source.GetTypeString() << std::endl;

            return {};
        }
    }

    return originNumber;
}

std::set<std::unique_ptr<Cheque>> OT_API::extract_cheques(
    const Identifier& nymID,
    const Identifier& accountID,
    const Identifier& serverID,
    const String& serializedProcessInbox,
    Ledger& inbox) const
{
    std::set<std::unique_ptr<Cheque>> output;
    std::unique_ptr<Ledger> ledger(new Ledger(nymID, accountID, serverID));

    OT_ASSERT(ledger);

    ledger->LoadLedgerFromString(serializedProcessInbox);
    auto processInbox = ledger->GetTransaction(OTTransaction::processInbox);

    OT_ASSERT(nullptr != processInbox);

    for (const auto& acceptItem : processInbox->GetItemList()) {
        OT_ASSERT(acceptItem);

        if (Item::acceptItemReceipt != acceptItem->GetType()) {
            continue;
        }

        const auto inboxNumber = acceptItem->GetReferenceToNum();
        const auto inboxItem = inbox.GetTransaction(inboxNumber);

        OT_ASSERT(nullptr != inboxItem)

        String reference;
        inboxItem->GetReferenceString(reference);
        std::unique_ptr<Item> item(
            Item::CreateItemFromString(reference, serverID, inboxNumber));

        OT_ASSERT(item)

        if (Item::depositCheque != item->GetType()) {
            continue;
        }

        output.emplace(Cheque::CreateFromReceipt(*inboxItem));
    }

    return output;
}

OT_API::~OT_API()
{
    if (nullptr != m_pWallet) {
        delete m_pWallet;
        m_pWallet = nullptr;
    }

    m_pClient.reset();
    Cleanup();
    // this must be last!
    pid_.reset();
}
}  // namespace opentxs
