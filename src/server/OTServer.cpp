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

#include "opentxs/server/OTServer.hpp"

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/app/Identity.hpp"
#include "opentxs/core/app/Settings.hpp"
#include "opentxs/core/app/Wallet.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/crypto/Bip39.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#ifdef ANDROID
#include "opentxs/core/util/android_string.hpp"
#endif // ANDROID
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/server/ConfigLoader.hpp"
#include "opentxs/server/Transactor.hpp"

#include <inttypes.h>
#include <stdint.h>
#include <sys/types.h>
#include <fstream>
#include <string>

#define SERVER_PID_FILENAME "ot.pid"

namespace opentxs
{

#ifdef _WIN32
int32_t OTCron::__trans_refill_amount = 500;  // The number of transaction
                                              // numbers Cron will grab for
                                              // itself, when it
                                              // gets low, before each round.
int32_t OTCron::__cron_ms_between_process =
    10000;                                      // The number of milliseconds
                                                // (ideally) between each
                                                // "Cron Process" event.
int32_t OTCron::__cron_max_items_per_nym = 10;  // The maximum number of cron
// items any given Nym can have
// active at the same time.
#endif

void OTServer::ActivateCron()
{
    Log::vOutput(
        1,
        "OTServer::ActivateCron: %s \n",
        m_Cron.ActivateCron() ? "(STARTED)" : "FAILED");
}

/// Currently the test server calls this 10 times per second.
/// It also processes all the input/output at the same rate.
/// It sleeps in between. (See testserver.cpp for the call
/// and OTLog::Sleep() for the sleep code.)
///
void OTServer::ProcessCron()
{
    if (!m_Cron.IsActivated()) return;

    bool bAddedNumbers = false;

    // Cron requires transaction numbers in order to process.
    // So every time before I call Cron.Process(), I make sure to replenish
    // first.
    while (m_Cron.GetTransactionCount() < OTCron::GetCronRefillAmount()) {
        int64_t lTransNum = 0;
        bool bSuccess = transactor_.issueNextTransactionNumber(lTransNum);

        if (bSuccess) {
            m_Cron.AddTransactionNumber(lTransNum);
            bAddedNumbers = true;
        } else
            break;
    }

    if (bAddedNumbers) {
        m_Cron.SaveCron();
    }

    m_Cron.ProcessCronItems();  // This needs to be called regularly for trades,
                                // markets, payment plans, etc to process.

    // NOTE:  TODO:  OTHER RE-OCCURRING SERVER FUNCTIONS CAN GO HERE AS WELL!!
    //
    // Such as sweeping server accounts after expiration dates, etc.
}

const Nym& OTServer::GetServerNym() const { return m_nymServer; }

bool OTServer::IsFlaggedForShutdown() const { return m_bShutdownFlag; }

OTServer::OTServer()
    : mainFile_(this)
    , notary_(this)
    , transactor_(this)
    , userCommandProcessor_(this)
    , m_bReadOnly(false)
    , m_bShutdownFlag(false)
{
}

OTServer::~OTServer()
{
    // PID -- Set it to 0 in the lock file so the next time we run OT, it knows
    // there isn't
    // another copy already running (otherwise we might wind up with two copies
    // trying to write
    // to the same data folder simultaneously, which could corrupt the data...)
    //
    //    OTLog::vError("m_strDataPath: %s\n", m_strDataPath.Get());
    //    OTLog::vError("SERVER_PID_FILENAME: %s\n", SERVER_PID_FILENAME);

    String strDataPath;
    const bool bGetDataFolderSuccess = OTDataFolder::Get(strDataPath);
    if (!m_bReadOnly && bGetDataFolderSuccess) {
        String strPIDPath;
        OTPaths::AppendFile(strPIDPath, strDataPath, SERVER_PID_FILENAME);

        std::ofstream pid_outfile(strPIDPath.Get());

        if (pid_outfile.is_open()) {
            uint32_t the_pid = 0;
            pid_outfile << the_pid;
            pid_outfile.close();
        } else
            Log::vError(
                "Failed trying to open data locking file (to wipe "
                "PID back to 0): %s\n",
                strPIDPath.Get());
    }
}

void OTServer::CreateMainFile(
    bool& mainFileExists,
    std::map<std::string, std::string>& args)
{
    NymParameters nymParameters;
    std::unique_ptr<Nym> newNym(new Nym(nymParameters));

    if (!newNym) {
        Log::vError("Error: Failed to create server nym\n");
        OT_FAIL;
    }

    if (!newNym->VerifyPseudonym()) {
        OT_FAIL;
    }

    for (auto it : args) {
        otErr << "Argument: " << it.first << " = " << it.second << std::endl;
    }

    String serverNymID;
    newNym->GetIdentifier(serverNymID);
    const std::string strNymID(serverNymID.Get(), serverNymID.GetLength());

    const std::string defaultTerms = "This is an example server contract.";
    const std::string& userTerms = args["terms"];
    std::string terms = userTerms;

    if (1 > userTerms.size()) {
        terms = defaultTerms;
    }

    const std::string defaultExternalIP = DEFAULT_EXTERNAL_IP;
    const std::string& userExternalIP = args["externalip"];
    std::string hostname = userExternalIP;

    if (5 > hostname.size()) {
        hostname = defaultExternalIP;
    }

    const std::string defaultBindIP = DEFAULT_BIND_IP;
    const std::string& userBindIP = args["bindip"];
    std::string bindIP = userBindIP;

    if (5 > bindIP.size()) {
        bindIP = defaultBindIP;
    }

    bool notUsed = false;
    App::Me().Config().Set_str("Listen", "bindip", String(bindIP), notUsed);

    const std::uint32_t defaultCommandPort = DEFAULT_COMMAND_PORT;
    const std::string& userCommandPort = args["commandport"];
    std::uint32_t commandPort = 0;
    bool needPort = true;

    while (needPort) {
        try {
            commandPort = std::stoi(userCommandPort.c_str());
        } catch (std::invalid_argument) {
            commandPort = defaultCommandPort;
            needPort = false;
        } catch (std::out_of_range) {
            commandPort = defaultCommandPort;
            needPort = false;
        }
        commandPort =
            (MAX_TCP_PORT < commandPort) ? defaultCommandPort : commandPort;
        commandPort =
            (MIN_TCP_PORT > commandPort) ? defaultCommandPort : commandPort;
        needPort = false;
    }

    const std::string& userListenCommand = args["listencommand"];
    uint32_t listenCommand = 0;
    bool needListenCommand = true;

    while (needListenCommand) {
        try {
            listenCommand = std::stoi(userListenCommand.c_str());
        } catch (std::invalid_argument) {
            listenCommand = defaultCommandPort;
            needListenCommand = false;
        } catch (std::out_of_range) {
            listenCommand = defaultCommandPort;
            needListenCommand = false;
        }
        listenCommand =
            (MAX_TCP_PORT < listenCommand) ? defaultCommandPort : listenCommand;
        listenCommand =
            (MIN_TCP_PORT > listenCommand) ? defaultCommandPort : listenCommand;
        needListenCommand = false;
    }

    App::Me().Config().Set_str(
        "Listen", "command", String(std::to_string(listenCommand)), notUsed);

    const uint32_t defaultNotificationPort = DEFAULT_NOTIFY_PORT;

    const std::string& userListenNotification = args["listennotify"];
    uint32_t listenNotification = 0;
    bool needListenNotification = true;

    while (needListenNotification) {
        try {
            listenNotification = std::stoi(userListenNotification.c_str());
        } catch (std::invalid_argument) {
            listenNotification = defaultNotificationPort;
            needListenNotification = false;
        } catch (std::out_of_range) {
            listenNotification = defaultNotificationPort;
            needListenNotification = false;
        }
        listenNotification = (MAX_TCP_PORT < listenNotification)
                                 ? defaultNotificationPort
                                 : listenNotification;
        listenNotification = (MIN_TCP_PORT > listenNotification)
                                 ? defaultNotificationPort
                                 : listenNotification;
        needListenNotification = false;
    }

    App::Me().Config().Set_str(
        "Listen",
        "notification",
        String(std::to_string(listenNotification)),
        notUsed);

    const std::string defaultName = DEFAULT_NAME;
    const std::string& userName = args["name"];
    std::string name = userName;

    if (1 > name.size()) {
        name = defaultName;
    }

    std::list<ServerContract::Endpoint> endpoints;
    ServerContract::Endpoint ipv4{proto::ADDRESSTYPE_IPV4,
                                  proto::PROTOCOLVERSION_LEGACY,
                                  hostname,
                                  commandPort,
                                  1};
    endpoints.push_back(ipv4);
    const std::string& onion = args["onion"];

    if (0 < onion.size()) {
        ServerContract::Endpoint tor{proto::ADDRESSTYPE_ONION,
                                     proto::PROTOCOLVERSION_LEGACY,
                                     onion,
                                     commandPort,
                                     1};
        endpoints.push_back(tor);
    }

    const std::string& eep = args["eep"];

    if (0 < eep.size()) {
        ServerContract::Endpoint i2p{proto::ADDRESSTYPE_EEP,
                                     proto::PROTOCOLVERSION_LEGACY,
                                     eep,
                                     commandPort,
                                     1};
            endpoints.push_back(i2p);
    }

    auto pContract =
        App::Me().Contract().Server(strNymID, name, terms, endpoints);
    std::string strNotaryID;

    if (pContract) {
        std::string strHostname;
        uint32_t nPort = 0;

        if (!pContract->ConnectInfo(strHostname, nPort)) {
            otOut << __FUNCTION__
                  << ": Unable to retrieve connection info from "
                     "this contract. Please fix that first; see "
                     "the sample data. (Failure.)\n";
            OT_FAIL;
        }
        strNotaryID = String(pContract->ID()).Get();
    } else {
        OT_FAIL;
    }

    std::string strCachedKey;

    if (OTCachedKey::It()->IsGenerated()) {
        OTASCIIArmor ascMasterContents;

        if (OTCachedKey::It()->SerializeTo(ascMasterContents)) {
            strCachedKey.assign(
                ascMasterContents.Get(), ascMasterContents.GetLength());
        } else
            OT_FAIL;
    } else {
        OT_FAIL;
    }

    const Claim nameClaim{
        "",
        proto::CONTACTSECTION_SCOPE,
        proto::CITEMTYPE_SERVER,
        name,
        0,
        0,
        {proto::CITEMATTR_ACTIVE, proto::CITEMATTR_PRIMARY}};
    const Claim serverClaim{
        "",
        proto::CONTACTSECTION_IDENTIFIER,
        proto::CITEMTYPE_COMMONNAME,
        String(pContract->ID()).Get(),
        0,
        0,
        {proto::CITEMATTR_ACTIVE, proto::CITEMATTR_PRIMARY}};

    if (!App::Me().Identity().AddClaim(*newNym, nameClaim)) {
        OT_FAIL
    }

    if (!App::Me().Identity().AddClaim(*newNym, serverClaim)) {
        OT_FAIL
    }

    newNym.reset();
    const OTData signedContract =
        proto::ProtoAsData<proto::ServerContract>(pContract->PublicContract());
    OTASCIIArmor ascContract(signedContract);
    opentxs::String strBookended;
    ascContract.WriteArmoredString(strBookended, "SERVER CONTRACT");
    OTDB::StorePlainString(strBookended.Get(), "NEW_SERVER_CONTRACT.otc");

    otOut << "Your server contract has been saved as " << std::endl
          << " NEW_SERVER_CONTRACT.otc in the server data directory."
          << std::endl;


#if OT_CRYPTO_SUPPORTED_KEY_HD
    const std::string defaultFingerprint = App::Me().DB().DefaultSeed();

    const std::string words =
        App::Me().Crypto().BIP39().Words(defaultFingerprint);
    const std::string passphrase =
        App::Me().Crypto().BIP39().Passphrase(defaultFingerprint);
#else
    const std::string words;
    const std::string passphrase;
#endif

    std::string json;
    json += "{ \"passphrase\": \"";
    json += passphrase;
    json += "\", \"words\": \"";
    json += words;
    json += "\" }\n";

    OTDB::StorePlainString(json, "seed_backup.json");

    mainFileExists = mainFile_.CreateMainFile(
        strBookended.Get(), strNotaryID, "", strNymID, strCachedKey);

    App::Me().Config().Save();
}

void OTServer::Init(std::map<std::string, std::string>& args, bool readOnly)
{
    m_bReadOnly = readOnly;

    if (!OTDataFolder::IsInitialized()) {
        Log::vError("Unable to Init data folders!");
        OT_FAIL;
    }
    if (!ConfigLoader::load(m_strWalletFilename)) {
        Log::vError("Unable to Load Config File!");
        OT_FAIL;
    }

    String dataPath;
    bool bGetDataFolderSuccess = OTDataFolder::Get(dataPath);

    // PID -- Make sure we're not running two copies of OT on the same data
    // simultaneously here.
    if (bGetDataFolderSuccess) {
        // If we want to WRITE to the data location, then we can't be in
        // read-only mode.
        if (!readOnly) {
            // 1. READ A FILE STORING THE PID. (It will already exist, if OT is
            // already running.)
            //
            // We open it for reading first, to see if it already exists. If it
            // does, we read the number. 0 is fine, since we overwrite with 0 on
            // shutdown. But any OTHER number means OT is still running. Or it
            // means it was killed while running and didn't shut down properly,
            // and that you need to delete the pid file by hand before running
            // OT again. (This is all for the purpose of preventing two copies
            // of OT running at the same time and corrupting the data folder.)
            //
            String strPIDPath;
            OTPaths::AppendFile(strPIDPath, dataPath, SERVER_PID_FILENAME);

            std::ifstream pid_infile(strPIDPath.Get());

            // 2. (IF FILE EXISTS WITH ANY PID INSIDE, THEN DIE.)
            if (pid_infile.is_open()) {
                uint32_t old_pid = 0;
                pid_infile >> old_pid;
                pid_infile.close();

                // There was a real PID in there.
                if (old_pid != 0) {
                    uint64_t lPID = old_pid;
                    Log::vError(
                        "\n\n\nIS OPEN-TRANSACTIONS ALREADY RUNNING?\n\n"
                        "I found a PID (%" PRIu64
                        ") in the data lock file, located "
                        "at: %s\n\n"
                        "If the OT process with PID %" PRIu64
                        " is truly not running "
                        "anymore, "
                        "then just ERASE THAT FILE and then RESTART.\n",
                        lPID,
                        strPIDPath.Get(),
                        lPID);
                    exit(-1);
                }
                // Otherwise, though the file existed, the PID within was 0.
                // (Meaning the previous instance of OT already set it to 0 as
                // it was shutting down.)
            }
            // Next let's record our PID to the same file, so other copies of OT
            // can't trample on US.

            // 3. GET THE CURRENT (ACTUAL) PROCESS ID.
            uint64_t the_pid = 0;

#ifdef _WIN32
            the_pid = GetCurrentProcessId();
#else
            the_pid = getpid();
#endif

            // 4. OPEN THE FILE IN WRITE MODE, AND SAVE THE PID TO IT.
            std::ofstream pid_outfile(strPIDPath.Get());

            if (pid_outfile.is_open()) {
                pid_outfile << the_pid;
                pid_outfile.close();
            } else {
                Log::vError(
                    "Failed trying to open data locking file (to "
                    "store PID %" PRIu64 "): %s\n",
                    the_pid,
                    strPIDPath.Get());
            }
        }
    }
    OTDB::InitDefaultStorage(OTDB_DEFAULT_STORAGE, OTDB_DEFAULT_PACKER);

    // Load up the transaction number and other OTServer data members.
    bool mainFileExists = m_strWalletFilename.Exists()
                              ? OTDB::Exists(".", m_strWalletFilename.Get())
                              : false;

    if (!mainFileExists) {
        if (readOnly) {
            Log::vError(
                "Error: Main file non-existent (%s). "
                "Plus, unable to create, since read-only flag is set.\n",
                m_strWalletFilename.Get());
            OT_FAIL;
        } else {
            CreateMainFile(mainFileExists, args);
        }
    }

    if (mainFileExists) {
        if (!mainFile_.LoadMainFile(readOnly)) {
            Log::vError("Error in Loading Main File!\n");
            OT_FAIL;
        }
    }

    // With the Server's private key loaded, and the latest transaction number
    // loaded, and all the various other data (contracts, etc) the server is now
    // ready for operation!
}

// msg, the request msg from payer, which is attached WHOLE to the Nymbox
// receipt. contains payment already.
// or pass pPayment instead: we will create our own msg here (with payment
// inside) to be attached to the receipt.
// szCommand for passing payDividend (as the message command instead of
// sendNymInstrument, the default.)
bool OTServer::SendInstrumentToNym(
    const Identifier& NOTARY_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_NYM_ID,
    Message* pMsg,              // the request msg from payer, which is attached
                                // WHOLE to the Nymbox receipt. contains payment
                                // already.
    const OTPayment* pPayment,  // or pass this instead: we will create
                                // our own msg here (with message
                                // inside) to be attached to the
                                // receipt.
    const char* szCommand)
{
    OT_ASSERT_MSG(
        !((nullptr == pMsg) && (nullptr == pPayment)),
        "pMsg and pPayment -- these can't BOTH be nullptr.\n");  // must provide
                                                                 // one
    // or the other.
    OT_ASSERT_MSG(
        !((nullptr != pMsg) && (nullptr != pPayment)),
        "pMsg and pPayment -- these can't BOTH be not-nullptr.\n");  // can't
                                                                     // provide
                                                                     // both.
    OT_ASSERT_MSG(
        (nullptr == pPayment) || ((nullptr != pPayment) && pPayment->IsValid()),
        "OTServer::SendInstrumentToNym: You can only pass a valid "
        "payment here.");
    // If a payment was passed in (for us to use it to construct pMsg, which is
    // nullptr in the case where payment isn't nullptr)
    // Then we grab it in string form, so we can pass it on...
    String strPayment;
    if (nullptr != pPayment) {
        const bool bGotPaymentContents =
            pPayment->GetPaymentContents(strPayment);
        if (!bGotPaymentContents)
            Log::vError("%s: Error GetPaymentContents Failed", __FUNCTION__);
    }
    const bool bDropped = DropMessageToNymbox(
        NOTARY_ID,
        SENDER_NYM_ID,
        RECIPIENT_NYM_ID,
        OTTransaction::instrumentNotice,
        pMsg,
        (nullptr != pMsg) ? nullptr : &strPayment,
        szCommand);

    return bDropped;
}

// Can't be static (transactor_.issueNextTransactionNumber is called...)
//
// About pMsg...
// (Normally) when you send a cheque to someone, you encrypt it inside an
// envelope, and that
// envelope is attached to a OTMessage (sendNymInstrument) and sent to the
// server. The server
// takes your entire OTMessage and attaches it to an instrumentNotice
// (OTTransaction) which is
// added to the recipient's Nymbox.
// In that case, just pass the pointer to the incoming message here as pMsg, and
// the OT Server
// will attach it as described.
// But let's say you are paying a dividend. The server can't just attach your
// dividend request in
// that case. Normally the recipient's cheque is already in the request. But
// with a dividend, there
// could be a thousand recipients, and their individual vouchers are only
// generated and sent AFTER
// the server receives the "pay dividend" request.
// Therefore in that case, nullptr would be passed for pMsg, meaning that inside
// this function we have
// to generate our own OTMessage "from the server" instead of "from the sender".
// After all, the server's
// private key is the only signer we have in here. And the recipient will be
// expecting to have to
// open a message, so we must create one to give him. So if pMsg is nullptr,
// then
// this function will
// create a message "from the server", containing the instrument, and drop it
// into the recipient's nymbox
// as though it were some incoming message from a normal user.
// This message, in the case of payDividend, should be an "payDividendResponse"
// message, "from" the server
// and "to" the recipient. The payment instrument must be attached to that new
// message, and therefore it
// must be passed into this function.
// Of course, if pMsg was not nullptr, that means the message (and instrument
// inside of it) already exist,
// so no instrument would need to be passed. But if pMsg IS nullptr, that means
// the
// msg must be generated,
// and thus the instrument MUST be passed in, so that that can be done.
// Therefore the instrument will sometimes be passed in, and sometimes not.
// Therefore the instrument must
// be passed as a pointer.
//
// Conclusion: if pMsg is passed in, then pass a null instrument. (Since the
// instrument is already on pMsg.)
//                (And since the instrument defaults to nullptr, this makes pMsg
// the final argument in the call.)
//         but if pMsg is nullptr, then you must pass the payment instrument as
// the
// next argument. (So pMsg can be created with it.)
// Note: you cannot pass BOTH, or the instrument will simply be ignored, since
// it's already assumed to be in pMsg.
// You might ask: what about the original request then, doesn't the recipient
// get a copy of that? Well, maybe we
// pass it in here and attach it to the new message. Or maybe we just set it as
// the voucher memo.
//
bool OTServer::DropMessageToNymbox(
    const Identifier& NOTARY_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_NYM_ID,
    OTTransaction::transactionType theType,
    Message* pMsg,
    const String* pstrMessage,
    const char* szCommand)  // If you pass
                            // something here, it
                            // will
// replace pMsg->m_strCommand below
{
    OT_ASSERT_MSG(
        !((nullptr == pMsg) && (nullptr == pstrMessage)),
        "pMsg and pstrMessage -- these can't BOTH be nullptr.\n");  // must
                                                                    // provide
    // one or the
    // other.
    OT_ASSERT_MSG(
        !((nullptr != pMsg) && (nullptr != pstrMessage)),
        "pMsg and pstrMessage -- these can't BOTH be not-nullptr.\n");  // can't
    // provide
    // both.
    const char* szFunc = "OTServer::DropMessageToNymbox";
    int64_t lTransNum = 0;
    const bool bGotNextTransNum =
        transactor_.issueNextTransactionNumber(lTransNum);

    if (!bGotNextTransNum) {
        Log::vError(
            "%s: Error: failed trying to get next transaction number.\n",
            szFunc);
        return false;
    }
    switch (theType) {
        case OTTransaction::message:
            break;
        case OTTransaction::instrumentNotice:
            break;
        default:
            Log::vError(
                "%s: Unexpected transactionType passed here (expected message "
                "or instrumentNotice.)\n",
                szFunc);
            return false;
    }
    // If pMsg was not already passed in here, then
    // create pMsg using pstrMessage.
    //
    std::unique_ptr<Message> theMsgAngel;

    if (nullptr == pMsg)  // we have to create it ourselves.
    {
        pMsg = new Message;
        theMsgAngel.reset(pMsg);
        if (nullptr != szCommand)
            pMsg->m_strCommand = szCommand;
        else {
            switch (theType) {
                case OTTransaction::message:
                    pMsg->m_strCommand = "sendNymMessage";
                    break;
                case OTTransaction::instrumentNotice:
                    pMsg->m_strCommand = "sendNymInstrument";
                    break;
                default:
                    break;  // should never happen.
            }
        }
        pMsg->m_strNotaryID = m_strNotaryID;
        pMsg->m_bSuccess = true;
        SENDER_NYM_ID.GetString(pMsg->m_strNymID);
        RECIPIENT_NYM_ID.GetString(pMsg->m_strNymID2);  // set the recipient ID
                                                        // in pMsg to match our
                                                        // recipient ID.
        // Load up the recipient's public key (so we can encrypt the envelope
        // to him that will contain the payment instrument.)
        //
        Nym nymRecipient(RECIPIENT_NYM_ID);

        bool bLoadedNym =
            nymRecipient.LoadPublicKey();  // Old style (deprecated.) But this
                                           // function calls the new style,
                                           // LoadCredentials, at the top.
                                           // Eventually we'll just call that
                                           // here directly.
        if (!bLoadedNym) {
            Log::vError(
                "%s: Failed trying to load public key for recipient.\n",
                szFunc);
            return false;
        } else if (!nymRecipient.VerifyPseudonym()) {
            Log::vError(
                "%s: Failed trying to verify Nym for recipient.\n", szFunc);
            return false;
        }
        const OTAsymmetricKey& thePubkey = nymRecipient.GetPublicEncrKey();
        // Wrap the message up into an envelope and attach it to pMsg.
        //
        OTEnvelope theEnvelope;

        pMsg->m_ascPayload.Release();

        if ((nullptr != pstrMessage) && pstrMessage->Exists() &&
            theEnvelope.Seal(thePubkey, *pstrMessage) &&  // Seal pstrMessage
                                                          // into theEnvelope,
            // using nymRecipient's
            // public key.
            theEnvelope.GetCiphertext(
                pMsg->m_ascPayload))  // Grab the sealed version as
                                      // base64-encoded string, into
                                      // pMsg->m_ascPayload.
        {
            pMsg->SignContract(m_nymServer);
            pMsg->SaveContract();
        } else {
            Log::vError(
                "%s: Failed trying to seal envelope containing message "
                "(or while grabbing the base64-encoded result.)\n",
                szFunc);
            return false;
        }

        // By this point, pMsg is all set up, signed and saved. Its payload
        // contains
        // the envelope (as base64) containing the encrypted message.
    }
    //  else // pMsg was passed in, so it's not nullptr. No need to create it
    // ourselves like above. (pstrMessage should be nullptr anyway in this
    // case.)
    //  {
    //       // Apparently no need to do anything in here at all.
    //  }
    // Grab a string copy of pMsg.
    //
    const String strInMessage(*pMsg);
    Ledger theLedger(
        RECIPIENT_NYM_ID,
        RECIPIENT_NYM_ID,
        NOTARY_ID);  // The recipient's Nymbox.
    // Drop in the Nymbox
    if ((theLedger.LoadNymbox() &&  // I think this loads the box receipts too,
                                    // since I didn't call "LoadNymboxNoVerify"
         //          theLedger.VerifyAccount(m_nymServer)    &&    // This loads
         // all the Box Receipts, which is unnecessary.
         theLedger.VerifyContractID() &&  // Instead, we'll verify the IDs and
                                          // Signature only.
         theLedger.VerifySignature(m_nymServer))) {
        // Create the instrumentNotice to put in the Nymbox.
        OTTransaction* pTransaction =
            OTTransaction::GenerateTransaction(theLedger, theType, lTransNum);

        if (nullptr !=
            pTransaction)  // The above has an OT_ASSERT within, but I
                           // just like to check my pointers.
        {
            // NOTE: todo: SHOULD this be "in reference to" itself? The reason,
            // I assume we are doing this
            // is because there is a reference STRING so "therefore" there must
            // be a reference # as well. Eh?
            // Anyway, it must be understood by those involved that a message is
            // stored inside. (Which has no transaction #.)

            pTransaction->SetReferenceToNum(lTransNum);  // <====== Recipient
                                                         // RECEIVES entire
                                                         // incoming message as
                                                         // string here, which
                                                         // includes the sender
                                                         // user ID,
            pTransaction->SetReferenceString(
                strInMessage);  // and has an OTEnvelope in the payload. Message
            // is signed by sender, and envelope is encrypted
            // to recipient.

            pTransaction->SignContract(m_nymServer);
            pTransaction->SaveContract();
            theLedger.AddTransaction(*pTransaction);  // Add the message
                                                      // transaction to the
                                                      // nymbox. (It will
                                                      // cleanup.)

            theLedger.ReleaseSignatures();
            theLedger.SignContract(m_nymServer);
            theLedger.SaveContract();
            theLedger.SaveNymbox();  // We don't grab the Nymbox hash here,
                                     // since
            // nothing important changed (just a message
            // was sent.)

            // Any inbox/nymbox/outbox ledger will only itself contain
            // abbreviated versions of the receipts, including their hashes.
            //
            // The rest is stored separately, in the box receipt, which is
            // created
            // whenever a receipt is added to a box, and deleted after a receipt
            // is removed from a box.
            //
            pTransaction->SaveBoxReceipt(theLedger);

            return true;
        } else  // should never happen
        {
            const String strRecipientNymID(RECIPIENT_NYM_ID);
            Log::vError(
                "%s: Failed while trying to generate transaction in order to "
                "add a message to Nymbox: %s\n",
                szFunc,
                strRecipientNymID.Get());
        }
    } else {
        const String strRecipientNymID(RECIPIENT_NYM_ID);
        Log::vError(
            "%s: Failed while trying to load or verify Nymbox: %s\n",
            szFunc,
            strRecipientNymID.Get());
    }

    return false;
}

bool OTServer::GetConnectInfo(std::string& strHostname, uint32_t& nPort) const
{
    bool notUsed = false;
    int64_t port = 0;

    const bool haveIP = App::Me().Config().CheckSet_str(
        "Listen", "bindip", String(DEFAULT_BIND_IP), strHostname, notUsed);

    const bool havePort = App::Me().Config().CheckSet_long(
        "Listen", "command", DEFAULT_COMMAND_PORT, port, notUsed);

    port = (MAX_TCP_PORT < port) ? DEFAULT_COMMAND_PORT : port;
    port = (MIN_TCP_PORT > port) ? DEFAULT_COMMAND_PORT : port;

    nPort = port;

    App::Me().Config().Save();

    return (haveIP && havePort);
}

zcert_t* OTServer::GetTransportKey() const
{
    auto contract = App::Me().Contract().Server(Identifier(m_strNotaryID));

    OT_ASSERT(contract);

    return contract->PrivateTransportKey();
}

}  // namespace opentxs
