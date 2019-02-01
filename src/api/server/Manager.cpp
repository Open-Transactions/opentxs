// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#if OT_CASH
#include "opentxs/blind/Mint.hpp"
#endif  // OT_CASH
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include "api/storage/StorageInternal.hpp"
#include "api/Core.hpp"
#include "internal/api/Internal.hpp"
#include "server/MessageProcessor.hpp"
#include "server/Server.hpp"
#include "server/ServerSettings.hpp"

#include <atomic>
#include <chrono>
#include <ctime>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include "Manager.hpp"

#if OT_CASH
#define SERIES_DIVIDER "."
#define PUBLIC_SERIES ".PUBLIC"
#define MAX_MINT_SERIES 10000
#define MINT_EXPIRE_MONTHS 6
#define MINT_VALID_MONTHS 12
#define MINT_GENERATE_DAYS 7
#endif  // OT_CASH

#define OT_METHOD "opentxs::api::server::implementation::Manager::"

namespace opentxs
{
api::server::Manager* Factory::ServerManager(
    const api::Native& parent,
    Flag& running,
    const ArgList& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const opentxs::network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance)
{
    api::server::implementation::Manager* manager =
        new api::server::implementation::Manager(
            parent,
            running,
            args,
            crypto,
            config,
            context,
            dataFolder,
            instance);
    if (nullptr != manager) {
        try {
            manager->Init();
        } catch (const std::invalid_argument& e) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": There was a problem creating the server. The server "
                "contract will be deleted. Error: ")(e.what())
                .Flush();

            const std::string datafolder = manager->DataFolder();

            delete manager;

            OTDB::EraseValueByKey(
                datafolder, ".", "NEW_SERVER_CONTRACT.otc", "", "");
            OTDB::EraseValueByKey(datafolder, ".", "notaryServer.xml", "", "");
            OTDB::EraseValueByKey(datafolder, ".", "seed_backup.json", "", "");

            std::rethrow_exception(std::current_exception());
        }
    }

    return manager;
}
}  // namespace opentxs

namespace opentxs::api::server::implementation
{
Manager::Manager(
    const api::Native& parent,
    Flag& running,
    const ArgList& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const opentxs::network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance)
    : Core(
          parent,
          running,
          args,
          crypto,
          config,
          context,
          dataFolder,
          instance,
          true)
    , server_p_(new opentxs::server::Server(*this))
    , server_(*server_p_)
    , message_processor_p_(
          new opentxs::server::MessageProcessor(server_, context, running_))
    , message_processor_(*message_processor_p_)
#if OT_CASH
    , mint_thread_()
    , mint_lock_()
    , mint_update_lock_()
    , mint_scan_lock_()
    , mints_()
    , mints_to_check_()
    , mint_key_size_(OT_MINT_KEY_SIZE_DEFAULT)
#endif  // OT_CASH
{
    wallet_.reset(opentxs::Factory::Wallet(*this));

    OT_ASSERT(wallet_);
    OT_ASSERT(server_p_);
    OT_ASSERT(message_processor_p_);
}

void Manager::Cleanup()
{
    LogDetail(OT_METHOD)(__FUNCTION__)(": Shutting down and cleaning up.")
        .Flush();
    message_processor_.cleanup();
    message_processor_p_.reset();
    server_p_.reset();
    Core::cleanup();
}

void Manager::DropIncoming(const int count) const
{
    return message_processor_.DropIncoming(count);
}

void Manager::DropOutgoing(const int count) const
{
    return message_processor_.DropOutgoing(count);
}

#if OT_CASH
void Manager::generate_mint(
    const std::string& serverID,
    const std::string& unitID,
    const std::uint32_t series) const
{
    auto mint = GetPrivateMint(Identifier::Factory(unitID), series);

    if (mint) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Mint already exists.").Flush();

        return;
    }

    const std::string nymID{NymID().str()};
    const std::string seriesID =
        std::string(SERIES_DIVIDER) + std::to_string(series);
    mint.reset(
        factory_
            ->Mint(
                String::Factory(nymID.c_str()), String::Factory(unitID.c_str()))
            .release());

    OT_ASSERT(mint)

    const auto& nym = server_.GetServerNym();
    const std::time_t now = std::time(nullptr);
    const std::chrono::seconds expireInterval(
        std::chrono::hours(MINT_EXPIRE_MONTHS * 30 * 24));
    const std::chrono::seconds validInterval(
        std::chrono::hours(MINT_VALID_MONTHS * 30 * 24));
    const std::time_t expires = now + expireInterval.count();
    const std::time_t validTo = now + validInterval.count();

    if (false == verify_mint_directory(serverID)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create mint directory.")
            .Flush();

        return;
    }

    mint->GenerateNewMint(
        *wallet_,
        series,
        now,
        validTo,
        expires,
        Identifier::Factory(unitID),
        Identifier::Factory(serverID),
        nym,
        1,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000,
        mint_key_size_.load());
    Lock mintLock(mint_lock_);

    if (mints_.end() != mints_.find(unitID)) {
        mints_.at(unitID).erase(PUBLIC_SERIES);
    }

    mint->SetSavePrivateKeys(true);
    mint->SignContract(nym);
    mint->SaveContract();
    mint->SaveMint(seriesID.c_str());
    mint->SetSavePrivateKeys(false);
    mint->ReleaseSignatures();
    mint->SignContract(nym);
    mint->SaveContract();
    mint->SaveMint(PUBLIC_SERIES);
    mint->SaveMint();
}
#endif  // OT_CASH
const std::string Manager::get_arg(const std::string& argName) const
{
    auto argIt = args_.find(argName);

    if (args_.end() != argIt) {
        const auto& argItems = argIt->second;

        OT_ASSERT(2 > argItems.size());
        OT_ASSERT(0 < argItems.size());

        return *argItems.cbegin();
    }

    return {};
}

std::string Manager::GetAdminNym() const
{
    auto output = String::Factory();
    bool exists{false};
    const auto success = config_.Check_str(
        String::Factory("permissions"),
        String::Factory("override_nym_id"),
        output,
        exists);

    if (success && exists) { return output->Get(); }

    return {};
}

std::string Manager::GetAdminPassword() const
{
    auto output = String::Factory();
    bool exists{false};
    const auto success = config_.Check_str(
        String::Factory("permissions"),
        String::Factory("admin_password"),
        output,
        exists);

    if (success && exists) { return output->Get(); }

    return {};
}

std::string Manager::GetCommandPort() const
{
    return get_arg(OPENTXS_ARG_COMMANDPORT);
}

std::string Manager::GetDefaultBindIP() const
{
    return get_arg(OPENTXS_ARG_BINDIP);
}

std::string Manager::GetEEP() const { return get_arg(OPENTXS_ARG_EEP); }

std::string Manager::GetExternalIP() const
{
    return get_arg(OPENTXS_ARG_EXTERNALIP);
}

std::string Manager::GetInproc() const { return get_arg(OPENTXS_ARG_INPROC); }

std::string Manager::GetListenCommand() const
{
    return get_arg(OPENTXS_ARG_LISTENCOMMAND);
}

std::string Manager::GetListenNotify() const
{
    return get_arg(OPENTXS_ARG_LISTENNOTIFY);
}

std::string Manager::GetOnion() const { return get_arg(OPENTXS_ARG_ONION); }

#if OT_CASH
std::shared_ptr<blind::Mint> Manager::GetPrivateMint(
    const Identifier& unitID,
    std::uint32_t index) const
{
    Lock lock(mint_lock_);
    const std::string id{unitID.str()};
    const std::string seriesID =
        std::string(SERIES_DIVIDER) + std::to_string(index);
    auto& seriesMap = mints_[id];
    // Modifying the private version may invalidate the public version
    seriesMap.erase(PUBLIC_SERIES);
    auto& output = seriesMap[seriesID];

    if (false == bool(output)) {
        output = load_private_mint(lock, id, seriesID);
    }

    return output;
}

std::shared_ptr<const blind::Mint> Manager::GetPublicMint(
    const Identifier& unitID) const
{
    Lock lock(mint_lock_);
    const std::string id{unitID.str()};
    const std::string seriesID{PUBLIC_SERIES};
    auto& output = mints_[id][seriesID];

    if (false == bool(output)) {
        output = load_public_mint(lock, id, seriesID);
    }

    return output;
}
#endif  // OT_CASH

std::string Manager::GetUserName() const { return get_arg(OPENTXS_ARG_NAME); }

std::string Manager::GetUserTerms() const { return get_arg(OPENTXS_ARG_TERMS); }

const Identifier& Manager::ID() const { return server_.GetServerID(); }

void Manager::Init()
{
    OT_ASSERT(dht_);
    OT_ASSERT(seeds_);

#if OT_CASH
    mint_thread_ = std::thread(&Manager::mint, this);
#endif  // OT_CASH

    Scheduler::Start(storage_.get(), dht_.get());
    StorageParent::init(*seeds_);
    Start();
}

#if OT_CASH
std::int32_t Manager::last_generated_series(
    const std::string& serverID,
    const std::string& unitID) const
{
    std::uint32_t output{0};

    for (output = 0; output < MAX_MINT_SERIES; ++output) {
        const std::string filename =
            unitID + SERIES_DIVIDER + std::to_string(output);
        const auto exists = OTDB::Exists(
            data_folder_,
            OTFolders::Mint().Get(),
            serverID.c_str(),
            filename.c_str(),
            "");

        if (false == exists) { return output - 1; }
    }

    return -1;
}

std::shared_ptr<blind::Mint> Manager::load_private_mint(
    const Lock& lock,
    const std::string& unitID,
    const std::string seriesID) const
{
    OT_ASSERT(verify_lock(lock, mint_lock_));

    std::shared_ptr<blind::Mint> mint{factory_->Mint(
        String::Factory(ID()),
        String::Factory(NymID()),
        String::Factory(unitID.c_str()))};

    OT_ASSERT(mint);

    return verify_mint(lock, unitID, seriesID, mint);
}

std::shared_ptr<blind::Mint> Manager::load_public_mint(
    const Lock& lock,
    const std::string& unitID,
    const std::string seriesID) const
{
    OT_ASSERT(verify_lock(lock, mint_lock_));

    std::shared_ptr<blind::Mint> mint{
        factory_->Mint(String::Factory(ID()), String::Factory(unitID.c_str()))};

    OT_ASSERT(mint);

    return verify_mint(lock, unitID, seriesID, mint);
}

void Manager::mint() const
{
    Lock updateLock(mint_update_lock_, std::defer_lock);

    while (server_.GetServerID().empty()) {
        Log::Sleep(std::chrono::milliseconds(50));
    }

    const std::string serverID{server_.GetServerID().str()};

    OT_ASSERT(false == serverID.empty());

    while (running_) {
        Log::Sleep(std::chrono::milliseconds(250));

        if (false == opentxs::server::ServerSettings::__cmd_get_mint) {
            continue;
        }

        std::string unitID{""};
        updateLock.lock();

        if (0 < mints_to_check_.size()) {
            unitID = mints_to_check_.back();
            mints_to_check_.pop_back();
        }

        updateLock.unlock();

        if (unitID.empty()) { continue; }

        const auto last = last_generated_series(serverID, unitID);
        const auto next = last + 1;

        if (0 > last) {
            generate_mint(serverID, unitID, 0);

            continue;
        }

        auto mint = GetPrivateMint(Identifier::Factory(unitID), last);

        if (false == bool(mint)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to load existing series.")
                .Flush();

            continue;
        }

        const auto now = std::time(nullptr);
        const std::time_t expires = mint->GetExpiration();
        const std::chrono::seconds limit(
            std::chrono::hours(24 * MINT_GENERATE_DAYS));
        const bool generate = ((now + limit.count()) > expires);

        if (generate) {
            generate_mint(serverID, unitID, next);
        } else {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Existing mint file for ")(
                unitID)(" is still valid.")
                .Flush();
        }
    }
}
#endif  // OT_CASH

const identifier::Nym& Manager::NymID() const
{
    return server_.GetServerNym().ID();
}

#if OT_CASH
void Manager::ScanMints() const
{
    Lock scanLock(mint_scan_lock_);
    Lock updateLock(mint_update_lock_, std::defer_lock);
    const auto units = wallet_->UnitDefinitionList();

    for (const auto& it : units) {
        const auto& id = it.first;
        updateLock.lock();
        mints_to_check_.push_front(id);
        updateLock.unlock();
    }
}
#endif  // OT_CASH

void Manager::Start()
{
    server_.Init();
    server_.ActivateCron();
    std::string hostname{};
    std::uint32_t port{0};
    proto::AddressType type{proto::ADDRESSTYPE_INPROC};
    const auto connectInfo = server_.GetConnectInfo(type, hostname, port);

    OT_ASSERT(connectInfo);

    auto pubkey = Data::Factory();
    auto privateKey = server_.TransportKey(pubkey);

    OT_ASSERT(privateKey);

    message_processor_.init(
        (proto::ADDRESSTYPE_INPROC == type), port, *privateKey);
    message_processor_.Start();
#if OT_CASH
    ScanMints();
#endif  // OT_CASH
}

#if OT_CASH
void Manager::UpdateMint(const Identifier& unitID) const
{
    Lock updateLock(mint_update_lock_);
    mints_to_check_.push_front(unitID.str());
}
#endif  // OT_CASH

bool Manager::verify_lock(const Lock& lock, const std::mutex& mutex) const
{
    if (lock.mutex() != &mutex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock not owned.").Flush();

        return false;
    }

    return true;
}

#if OT_CASH
std::shared_ptr<blind::Mint> Manager::verify_mint(
    const Lock& lock,
    const std::string& unitID,
    const std::string seriesID,
    std::shared_ptr<blind::Mint>& mint) const
{
    OT_ASSERT(verify_lock(lock, mint_lock_));

    if (false == mint->LoadMint(seriesID.c_str())) {
        UpdateMint(Identifier::Factory(unitID));

        return {};
    }

    if (false == mint->VerifyMint(server_.GetServerNym())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid mint for ")(unitID)(".")
            .Flush();

        return {};
    }

    return mint;
}

bool Manager::verify_mint_directory(const std::string& serverID) const
{
    bool created{false};
    auto serverDir = String::Factory();
    auto mintDir = String::Factory();
    const auto haveMint = OTPaths::AppendFolder(
        mintDir, String::Factory(data_folder_.c_str()), OTFolders::Mint());
    const auto haveServer = OTPaths::AppendFolder(
        serverDir, mintDir, String::Factory(serverID.c_str()));

    OT_ASSERT(haveMint)
    OT_ASSERT(haveServer)

    return OTPaths::BuildFolderPath(serverDir, created);
}
#endif  // OT_CASH

Manager::~Manager()
{
    running_.Off();
#if OT_CASH
    if (mint_thread_.joinable()) { mint_thread_.join(); }
#endif  // OT_CASH

    Cleanup();
}
}  // namespace opentxs::api::server::implementation
