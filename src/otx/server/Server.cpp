// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/server/Server.hpp"  // IWYU pragma: associated

#include <OTXEnums.pb.h>
#include <OTXPush.pb.h>
#include <ServerContract.pb.h>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <regex>

#include "internal/api/crypto/Encode.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/crypto/Envelope.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/cron/OTCron.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Settings.internal.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Notary.internal.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/AddressType.hpp"  // IWYU pragma: keep
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ProtocolVersion.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Language.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/SeedStyle.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/IdentityType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/Options.hpp"
#include "otx/common/OTStorage.hpp"
#include "otx/server/ConfigLoader.hpp"
#include "otx/server/MainFile.hpp"
#include "otx/server/Transactor.hpp"

namespace opentxs
{
constexpr auto SEED_BACKUP_FILE = "seed_backup.json";
constexpr auto SERVER_CONTRACT_FILE = "NEW_SERVER_CONTRACT.otc";
constexpr auto SERVER_CONFIG_LISTEN_SECTION = "listen";
constexpr auto SERVER_CONFIG_BIND_KEY = "bindip";
constexpr auto SERVER_CONFIG_PORT_KEY = "command";
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::server
{
Server::Server(
    const opentxs::api::session::Notary& manager,
    const PasswordPrompt& reason)
    : api_(manager)
    , reason_(reason)
    , main_file_(*this, reason_)
    , notary_(*this, reason_, api_)
    , transactor_(*this, reason_)
    , user_command_processor_(*this, reason_, api_)
    , wallet_filename_(String::Factory())
    , read_only_(false)
    , shutdown_flag_(false)
    , notary_id_()
    , server_nym_id_()
    , nym_server_(nullptr)
    , cron_(manager.Factory().Internal().Session().Cron())
    , notification_socket_(
          api_.Network().ZeroMQ().Context().Internal().PushSocket(
              zmq::socket::Direction::Connect))
{
    const auto bound = notification_socket_->Start(
        api_.Endpoints().Internal().PushNotification().data());

    assert_true(bound);
}

void Server::ActivateCron()
{
    if (cron_->ActivateCron()) {
        LogVerbose()()("Activate Cron. (STARTED)").Flush();
    } else {
        LogConsole()()("Activate Cron. (FAILED)").Flush();
    }
}

/// Currently the test server calls this 10 times per second.
/// It also processes all the input/output at the same rate.
/// It sleeps in between. (See testserver.cpp for the call
/// and OTSleep() for the sleep code.)
///
void Server::ProcessCron()
{
    if (!cron_->IsActivated()) { return; }

    bool bAddedNumbers = false;

    // Cron requires transaction numbers in order to process.
    // So every time before I call Cron.Process(), I make sure to replenish
    // first.
    while (cron_->GetTransactionCount() < OTCron::GetCronRefillAmount()) {
        std::int64_t lTransNum = 0;
        const bool bSuccess = transactor_.issueNextTransactionNumber(lTransNum);

        if (bSuccess) {
            cron_->AddTransactionNumber(lTransNum);
            bAddedNumbers = true;
        } else {
            break;
        }
    }

    if (bAddedNumbers) { cron_->SaveCron(); }

    cron_->ProcessCronItems();  // This needs to be called regularly for
                                // trades, markets, payment plans, etc to
                                // process.

    // NOTE:  TODO:  OTHER RE-OCCURRING SERVER FUNCTIONS CAN GO HERE AS WELL!!
    //
    // Such as sweeping server accounts after expiration dates, etc.
}

auto Server::GetServerID() const noexcept -> const identifier::Notary&
{
    return notary_id_.get();
}

auto Server::GetServerNym() const -> const identity::Nym&
{
    return *nym_server_;
}

auto Server::IsFlaggedForShutdown() const -> bool { return shutdown_flag_; }

auto Server::parse_seed_backup(const UnallocatedCString& input) const
    -> std::pair<UnallocatedCString, UnallocatedCString>
{
    std::pair<UnallocatedCString, UnallocatedCString> output{};
    auto& phrase = output.first;
    auto& words = output.second;

    const std::regex reg("\"passphrase\": \"(.*)\", \"words\": \"(.*)\"");
    std::cmatch match{};

    if (std::regex_search(input.c_str(), match, reg)) {
        phrase = match[1];
        words = match[2];
    }

    return output;
}

void Server::CreateMainFile(bool& mainFileExists)
{
    auto seed = crypto::SeedID{};

    if (api::crypto::HaveHDKeys()) {
        const auto backup = OTDB::QueryPlainString(
            api_, api_.DataFolder().string(), SEED_BACKUP_FILE, "", "", "");

        if (false == backup.empty()) {
            LogError()()("Seed backup found. Restoring.").Flush();
            auto parsed = parse_seed_backup(backup);
            auto phrase = api_.Factory().SecretFromText(parsed.first);
            auto words = api_.Factory().SecretFromText(parsed.second);
            seed = api_.Crypto().Seed().ImportSeed(
                words,
                phrase,
                crypto::SeedStyle::BIP39,
                crypto::Language::en,
                reason_);

            if (seed.empty()) {
                LogError()()("Seed restoration failed.").Flush();
            } else {
                LogError()()("Seed ")(seed, api_.Crypto())(" restored.")
                    .Flush();
            }
        }
    }

    const UnallocatedCString defaultName = default_name_;
    auto name = UnallocatedCString{api_.GetOwnerName()};

    if (1 > name.size()) { name = defaultName; }

    auto nymParameters = crypto::Parameters{api_.Factory()};
    nymParameters.SetSeed(seed);
    nymParameters.SetNym(0);
    nymParameters.SetDefault(false);
    nym_server_ =
        api_.Wallet().Nym(nymParameters, identity::Type::server, reason_, name);

    if (false == bool(nym_server_)) {
        LogError()()("Error: Failed to create server nym.").Flush();
        LogAbort()().Abort();
    }

    if (!nym_server_->VerifyPseudonym()) { LogAbort()().Abort(); }

    const auto& nymID = nym_server_->ID();
    using namespace std::literals;
    const auto defaultTerms = "This is an example server contract."s;
    auto terms = UnallocatedCString{api_.GetUserTerms()};

    if (terms.empty()) { terms = defaultTerms; }

    const auto& args = api_.GetOptions();
    auto bindIP = UnallocatedCString{args.NotaryBindIP()};

    if (5 > bindIP.size()) { bindIP = default_bind_ip_; }

    bool notUsed = false;
    const auto& config = api_.Config().Internal();
    config.Set_str(
        String::Factory(SERVER_CONFIG_LISTEN_SECTION),
        String::Factory(SERVER_CONFIG_BIND_KEY),
        String::Factory(bindIP),
        notUsed);
    const auto publicPort = [&] {
        auto out = args.NotaryPublicPort();
        out = (max_tcp_port_ < out) ? default_port_ : out;
        out = (min_tcp_port_ > out) ? default_port_ : out;

        return out;
    }();
    const auto bindPort = [&] {
        auto out = args.NotaryBindPort();
        out = (max_tcp_port_ < out) ? default_port_ : out;
        out = (min_tcp_port_ > out) ? default_port_ : out;

        return out;
    }();
    config.Set_str(
        String::Factory(SERVER_CONFIG_LISTEN_SECTION),
        String::Factory(SERVER_CONFIG_PORT_KEY),
        String::Factory(std::to_string(bindPort)),
        notUsed);

    auto endpoints = UnallocatedList<contract::Server::Endpoint>{};
    const auto inproc = args.NotaryInproc();

    if (inproc) {
        LogConsole()("Creating inproc contract for instance ")(api_.Instance())
            .Flush();
        endpoints.emplace_back(
            AddressType::Inproc,
            contract::ProtocolVersion::Legacy,
            api_.Internal().asNotary().InprocEndpoint(),
            publicPort,
            2);
    } else {
        LogConsole()("Creating standard contract on port ")(publicPort).Flush();

        for (const auto& hostname : args.NotaryPublicIPv4()) {
            if (5 > hostname.size()) { continue; }

            LogConsole()("* Adding ipv4 endpoint: ")(hostname).Flush();
            endpoints.emplace_back(
                AddressType::IPV4,
                contract::ProtocolVersion::Legacy,
                hostname,
                publicPort,
                1);
        }

        for (const auto& hostname : args.NotaryPublicIPv6()) {
            if (5 > hostname.size()) { continue; }

            LogConsole()("* Adding ipv6 endpoint: ")(hostname).Flush();
            endpoints.emplace_back(
                AddressType::IPV6,
                contract::ProtocolVersion::Legacy,
                hostname,
                publicPort,
                1);
        }

        for (const auto& hostname : args.NotaryPublicOnion()) {
            if (5 > hostname.size()) { continue; }

            LogConsole()("* Adding onion endpoint: ")(hostname).Flush();
            endpoints.emplace_back(
                AddressType::Onion2,
                contract::ProtocolVersion::Legacy,
                hostname,
                publicPort,
                1);
        }

        for (const auto& hostname : args.NotaryPublicEEP()) {
            if (5 > hostname.size()) { continue; }

            LogConsole()("* Adding eep endpoint: ")(hostname).Flush();
            endpoints.emplace_back(
                AddressType::EEP,
                contract::ProtocolVersion::Legacy,
                hostname,
                publicPort,
                1);
        }

        if (0 == endpoints.size()) {
            LogConsole()("* Adding default endpoint: ")(default_external_ip_)
                .Flush();
            endpoints.emplace_back(
                AddressType::IPV4,
                contract::ProtocolVersion::Legacy,
                default_external_ip_,
                publicPort,
                1);
        }
    }

    const auto& wallet = api_.Wallet();
    const auto contract = [&] {
        const auto existing = String::Factory(OTDB::QueryPlainString(
                                                  api_,
                                                  api_.DataFolder().string(),
                                                  SERVER_CONTRACT_FILE,
                                                  "",
                                                  "",
                                                  "")
                                                  .data());

        if (existing->empty()) {

            return wallet.Internal().Server(
                nymID.asBase58(API().Crypto()),
                name,
                terms,
                endpoints,
                reason_,
                (inproc) ? std::max(2u, contract::Server::DefaultVersion)
                         : contract::Server::DefaultVersion);
        } else {
            LogError()()("Existing contract found. Restoring.").Flush();
            const auto serialized = proto::StringToProto<proto::ServerContract>(
                api_.Crypto(), existing);

            return wallet.Internal().Server(serialized);
        }
    }();
    UnallocatedCString strNotaryID{};
    UnallocatedCString strHostname{};
    std::uint32_t nPort{0};
    AddressType type{};

    if (!contract->ConnectInfo(strHostname, nPort, type, type)) {
        LogConsole()()("Unable to retrieve connection info from this contract.")
            .Flush();

        LogAbort()().Abort();
    }

    strNotaryID = String::Factory(contract->ID(), api_.Crypto())->Get();

    assert_false(nullptr == nym_server_);

    {
        auto nymData = api_.Wallet().mutable_Nym(nymID, reason_);

        if (false == nymData.SetCommonName(
                         contract->ID().asBase58(api_.Crypto()), reason_)) {
            LogAbort()().Abort();
        }
    }

    nym_server_ = api_.Wallet().Nym(nymID);

    assert_false(nullptr == nym_server_);

    auto proto = proto::ServerContract{};
    if (false == contract->Serialize(proto, true)) {
        LogConsole()()("Failed to serialize server contract.").Flush();

        LogAbort()().Abort();
    }
    const auto signedContract = api_.Factory().Internal().Data(proto);
    auto ascContract = api_.Factory().Internal().Armored(signedContract);
    auto strBookended = String::Factory();
    ascContract->WriteArmoredString(strBookended, "SERVER CONTRACT");
    OTDB::StorePlainString(
        api_,
        strBookended->Get(),
        api_.DataFolder().string(),
        SERVER_CONTRACT_FILE,
        "",
        "",
        "");

    LogConsole()("Your new server contract has been saved as ")(
        SERVER_CONTRACT_FILE)(" in the server data directory.")
        .Flush();

    const auto seedID = api_.Storage().DefaultSeed();
    const auto words = api_.Crypto().Seed().Words(seedID, reason_);
    const auto passphrase = api_.Crypto().Seed().Passphrase(seedID, reason_);
    UnallocatedCString json;
    json += R"({ "passphrase": ")";
    json += passphrase;
    json += R"(", "words": ")";
    json += words;
    json += "\" }\n";

    OTDB::StorePlainString(
        api_, json, api_.DataFolder().string(), SEED_BACKUP_FILE, "", "", "");

    mainFileExists = main_file_.CreateMainFile(
        strBookended->Get(), strNotaryID, nymID.asBase58(API().Crypto()));

    if (false == config.Save()) {
        LogAbort()()("failed to save config file").Abort();
    }
}

void Server::Init(bool readOnly)
{
    read_only_ = readOnly;
    const auto& settings = api_.Config();
    const auto& config = settings.Internal();

    if (!ConfigLoader::load(api_, settings, WalletFilename())) {
        LogError()()("Unable to Load Config File!").Flush();
        LogAbort()().Abort();
    }

    OTDB::InitDefaultStorage(OTDB_DEFAULT_STORAGE, OTDB_DEFAULT_PACKER);

    // Load up the transaction number and other Server data members.
    bool mainFileExists = WalletFilename().Exists()
                              ? OTDB::Exists(
                                    api_,
                                    api_.DataFolder().string(),
                                    ".",
                                    WalletFilename().Get(),
                                    "",
                                    "")
                              : false;

    if (false == mainFileExists) {
        if (readOnly) {
            LogError()()("Error: Main file non-existent (")(
                WalletFilename().Get())(
                "). Plus, unable to create, since read-only flag is set.")
                .Flush();
            LogAbort()().Abort();
        } else {
            CreateMainFile(mainFileExists);
        }
    }

    assert_true(mainFileExists);

    if (false == main_file_.LoadMainFile(readOnly)) {
        LogError()()("Error in Loading Main File, re-creating.").Flush();
        OTDB::EraseValueByKey(
            api_,
            api_.DataFolder().string(),
            ".",
            WalletFilename().Get(),
            "",
            "");
        CreateMainFile(mainFileExists);

        assert_true(mainFileExists);

        if (!main_file_.LoadMainFile(readOnly)) { LogAbort()().Abort(); }
    }

    auto password = api_.Crypto().Encode().InternalEncode().Nonce(16);
    auto notUsed = String::Factory();
    bool ignored;
    config.CheckSet_str(
        String::Factory("permissions"),
        String::Factory("admin_password"),
        password,
        notUsed,
        ignored);

    if (false == config.Save()) {
        LogAbort()()("failed to save config file").Abort();
    }

    // With the Server's private key loaded, and the latest transaction number
    // loaded, and all the various other data (contracts, etc) the server is now
    // ready for operation!
}

auto Server::LoadServerNym(const identifier::Nym& nymID) -> bool
{
    auto nym = api_.Wallet().Nym(nymID);

    if (false == bool(nym)) {
        LogError()()("Server nym ")(nymID, api_.Crypto())(" does not exist.")
            .Flush();

        return false;
    }

    nym_server_ = nym;

    assert_false(nullptr == nym_server_);

    return true;
}

// msg, the request msg from payer, which is attached WHOLE to the Nymbox
// receipt. contains payment already. or pass pPayment instead: we will create
// our own msg here (with payment inside) to be attached to the receipt.
// szCommand for passing payDividend (as the message command instead of
// sendNymInstrument, the default.)
auto Server::SendInstrumentToNym(
    const identifier::Notary& NOTARY_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const identifier::Nym& RECIPIENT_NYM_ID,
    const OTPayment& pPayment,
    const char* szCommand) -> bool
{
    assert_true(pPayment.IsValid());

    // If a payment was passed in (for us to use it to construct pMsg, which is
    // nullptr in the case where payment isn't nullptr)
    // Then we grab it in string form, so we can pass it on...
    auto strPayment = String::Factory();
    const bool bGotPaymentContents = pPayment.GetPaymentContents(strPayment);

    if (!bGotPaymentContents) {
        LogError()()("Error GetPaymentContents Failed!").Flush();
    }

    const bool bDropped = DropMessageToNymbox(
        NOTARY_ID,
        SENDER_NYM_ID,
        RECIPIENT_NYM_ID,
        transactionType::instrumentNotice,
        nullptr,
        strPayment,
        szCommand);

    return bDropped;
}

auto Server::SendInstrumentToNym(
    const identifier::Notary& NOTARY_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const identifier::Nym& RECIPIENT_NYM_ID,
    const Message& pMsg) -> bool
{
    return DropMessageToNymbox(
        NOTARY_ID,
        SENDER_NYM_ID,
        RECIPIENT_NYM_ID,
        transactionType::instrumentNotice,
        pMsg);
}

auto Server::DropMessageToNymbox(
    const identifier::Notary& notaryID,
    const identifier::Nym& senderNymID,
    const identifier::Nym& recipientNymID,
    transactionType transactionType,
    const Message& msg) -> bool
{
    return DropMessageToNymbox(
        notaryID, senderNymID, recipientNymID, transactionType, &msg);
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
auto Server::DropMessageToNymbox(
    const identifier::Notary& NOTARY_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const identifier::Nym& RECIPIENT_NYM_ID,
    transactionType theType,
    const Message* pMsg,
    const String& pstrMessage,
    const char* szCommand) -> bool  // If you pass something here, it will
{                                   // replace pMsg->command_ below.
    assert_false(
        (nullptr == pMsg) && (pstrMessage.empty()),
        "pMsg and pstrMessage -- these can't BOTH be nullptr.");
    // ^^^ Must provde one or the other.
    assert_false(
        (nullptr != pMsg) && (!pstrMessage.empty()),
        "pMsg and pstrMessage -- these can't BOTH be not-nullptr.");
    // ^^^ Can't provide both.
    std::int64_t lTransNum{0};
    const bool bGotNextTransNum =
        transactor_.issueNextTransactionNumber(lTransNum);

    if (!bGotNextTransNum) {
        LogError()()("Error: Failed trying to get next transaction number.")
            .Flush();
        return false;
    }
    switch (theType) {
        case transactionType::message:
            break;
        case transactionType::instrumentNotice:
            break;
        default:
            LogError()()(
                "Unexpected transactionType passed here (Expected message "
                "or instrumentNotice).")
                .Flush();
            return false;
    }
    // If pMsg was not already passed in here, then
    // create pMsg using pstrMessage.
    //
    std::unique_ptr<Message> theMsgAngel;
    const Message* message{nullptr};

    if (nullptr == pMsg) {
        theMsgAngel = api_.Factory().Internal().Session().Message();

        if (nullptr != szCommand) {
            theMsgAngel->command_ = String::Factory(szCommand);
        } else {
            switch (theType) {
                case transactionType::message:
                    theMsgAngel->command_ = String::Factory("sendNymMessage");
                    break;
                case transactionType::instrumentNotice:
                    theMsgAngel->command_ =
                        String::Factory("sendNymInstrument");
                    break;
                default:
                    break;  // should never happen.
            }
        }
        theMsgAngel->notary_id_ = String::Factory(notary_id_, api_.Crypto());
        theMsgAngel->success_ = true;
        SENDER_NYM_ID.GetString(API().Crypto(), theMsgAngel->nym_id_);
        RECIPIENT_NYM_ID.GetString(
            API().Crypto(),
            theMsgAngel->nym_id2_);  // set the recipient ID
                                     // in theMsgAngel to match our
                                     // recipient ID.
        // Load up the recipient's public key (so we can encrypt the envelope
        // to him that will contain the payment instrument.)
        //
        auto nymRecipient = api_.Wallet().Nym(RECIPIENT_NYM_ID);

        // Wrap the message up into an envelope and attach it to theMsgAngel.
        auto theEnvelope = api_.Factory().Internal().Session().Envelope();
        theMsgAngel->payload_->Release();

        if ((!pstrMessage.empty()) &&
            theEnvelope->Seal(
                *nymRecipient, pstrMessage.Bytes(), reason_) &&  // Seal
                                                                 // pstrMessage
                                                                 // into
                                                                 // theEnvelope,
            // using nymRecipient's
            // public key.
            theEnvelope->Armored(theMsgAngel->payload_))  // Grab the
                                                          // sealed
                                                          // version as
        // base64-encoded string, into
        // theMsgAngel->payload_.
        {
            theMsgAngel->SignContract(*nym_server_, reason_);
            theMsgAngel->SaveContract();
        } else {
            LogError()()(
                "Failed trying to seal envelope containing theMsgAngel "
                "(or while grabbing the base64-encoded result).")
                .Flush();
            return false;
        }

        // By this point, pMsg is all set up, signed and saved. Its payload
        // contains
        // the envelope (as base64) containing the encrypted message.

        message = theMsgAngel.get();
    } else {
        message = pMsg;
    }
    //  else // pMsg was passed in, so it's not nullptr. No need to create it
    // ourselves like above. (pstrMessage should be nullptr anyway in this
    // case.)
    //  {
    //       // Apparently no need to do anything in here at all.
    //  }
    // Grab a string copy of message.
    //
    const auto strInMessage = String::Factory(*message);
    auto theLedger{api_.Factory().Internal().Session().Ledger(
        RECIPIENT_NYM_ID, RECIPIENT_NYM_ID, NOTARY_ID)};  // The
                                                          // recipient's
                                                          // Nymbox.
    // Drop in the Nymbox
    if ((theLedger->LoadNymbox() &&  // I think this loads the box
                                     // receipts too, since I didn't call
                                     // "LoadNymboxNoVerify"
         //          theLedger.VerifyAccount(nym_server_)    &&    // This loads
         // all the Box Receipts, which is unnecessary.
         theLedger->VerifyContractID() &&  // Instead, we'll verify the IDs and
                                           // Signature only.
         theLedger->VerifySignature(*nym_server_))) {
        // Create the instrumentNotice to put in the Nymbox.
        auto pTransaction{api_.Factory().Internal().Session().Transaction(
            *theLedger, theType, originType::not_applicable, lTransNum)};

        if (false != bool(pTransaction)) {
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

            pTransaction->SignContract(*nym_server_, reason_);
            pTransaction->SaveContract();
            const std::shared_ptr<OTTransaction> transaction{
                pTransaction.release()};
            theLedger->AddTransaction(transaction);  // Add the message
                                                     // transaction to the
                                                     // nymbox. (It will
                                                     // cleanup.)

            theLedger->ReleaseSignatures();
            theLedger->SignContract(*nym_server_, reason_);
            theLedger->SaveContract();
            theLedger->SaveNymbox();  // We don't grab the
                                      // Nymbox hash here,
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
            transaction->SaveBoxReceipt(*theLedger);
            notification_socket_->Send(
                nymbox_push(RECIPIENT_NYM_ID, *transaction));

            return true;
        } else  // should never happen
        {
            const auto strRecipientNymID =
                String::Factory(RECIPIENT_NYM_ID, api_.Crypto());
            LogError()()(
                "Failed while trying to generate transaction in order to "
                "add a message to Nymbox: ")(strRecipientNymID->Get())(".")
                .Flush();
        }
    } else {
        const auto strRecipientNymID =
            String::Factory(RECIPIENT_NYM_ID, api_.Crypto());
        LogError()()("Failed while trying to load or verify "
                     "Nymbox: ")(strRecipientNymID->Get())(".")
            .Flush();
    }

    return false;
}

auto Server::GetConnectInfo(
    AddressType& type,
    UnallocatedCString& strHostname,
    std::uint32_t& nPort) const -> bool
{
    auto contract = api_.Wallet().Internal().Server(notary_id_);
    UnallocatedCString contractHostname{};
    std::uint32_t contractPort{};
    const auto haveEndpoints =
        contract->ConnectInfo(contractHostname, contractPort, type, type);

    assert_true(haveEndpoints);

    bool notUsed = false;
    std::int64_t port = 0;
    const auto& config = api_.Config().Internal();
    const bool haveIP = config.CheckSet_str(
        String::Factory(SERVER_CONFIG_LISTEN_SECTION),
        String::Factory("bindip"),
        String::Factory(default_bind_ip_),
        strHostname,
        notUsed);
    const bool havePort = config.CheckSet_long(
        String::Factory(SERVER_CONFIG_LISTEN_SECTION),
        String::Factory(SERVER_CONFIG_PORT_KEY),
        default_port_,
        port,
        notUsed);
    port = (max_tcp_port_ < port) ? default_port_ : port;
    port = (min_tcp_port_ > port) ? default_port_ : port;
    nPort = static_cast<std::uint32_t>(port);

    if (config.Save()) {

        return (haveIP && havePort);
    } else {
        LogError()()("failed to save config file").Flush();

        return false;
    }
}

auto Server::nymbox_push(
    const identifier::Nym& nymID,
    const OTTransaction& item) const -> network::zeromq::Message
{
    auto output = zmq::Message{};
    output.AddFrame(nymID.asBase58(API().Crypto()));
    proto::OTXPush push;
    push.set_version(OTX_PUSH_VERSION);
    push.set_type(proto::OTXPUSH_NYMBOX);
    push.set_item(String::Factory(item)->Get());
    output.Internal().AddFrame(push);

    return output;
}

auto Server::SetNotaryID(const identifier::Notary& id) noexcept -> void
{
    assert_false(id.empty());

    notary_id_.set_value(id);
}

auto Server::TransportKey(Data& pubkey) const -> Secret
{
    return api_.Wallet()
        .Internal()
        .Server(notary_id_)
        ->TransportKey(pubkey, reason_);
}

Server::~Server() = default;
}  // namespace opentxs::server
