// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "api/session/notary/Notary.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <cstddef>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "api/session/Session.hpp"
#include "api/session/base/Scheduler.hpp"
#include "api/session/base/Storage.hpp"
#include "core/Shutdown.hpp"
#include "internal/api/Context.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/otx/blind/Mint.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"  // IWYU pragma: keep
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "otx/common/OTStorage.hpp"
#include "otx/server/MessageProcessor.hpp"
#include "otx/server/Server.hpp"
#include "otx/server/ServerSettings.hpp"

namespace opentxs
{
constexpr auto SERIES_DIVIDER = ".";
constexpr auto PUBLIC_SERIES = ".PUBLIC";
constexpr auto MAX_MINT_SERIES = 10000;
constexpr auto MINT_EXPIRE_MONTHS = 6;
constexpr auto MINT_VALID_MONTHS = 12;
constexpr auto MINT_GENERATE_DAYS = 7;
}  // namespace opentxs

namespace opentxs::factory
{
auto NotarySession(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const opentxs::network::zeromq::Context& context,
    const UnallocatedCString& dataFolder,
    const int instance) -> std::unique_ptr<api::session::Notary>
{
    using ReturnType = api::session::imp::Notary;

    try {
        auto output = std::make_unique<ReturnType>(
            parent,
            running,
            std::move(args),
            crypto,
            config,
            context,
            dataFolder,
            instance);

        if (output) {
            try {
                output->Init();
            } catch (const std::invalid_argument& e) {
                LogError()("opentxs::factory::")(__func__)(
                    ": There was a problem creating the server. The server "
                    "contract will be deleted. Error: ")(e.what())
                    .Flush();
                const UnallocatedCString datafolder = output->DataFolder();
                OTDB::EraseValueByKey(
                    *output,
                    datafolder,
                    ".",
                    "NEW_SERVER_CONTRACT.otc",
                    "",
                    "");
                OTDB::EraseValueByKey(
                    *output, datafolder, ".", "notaryServer.xml", "", "");
                OTDB::EraseValueByKey(
                    *output, datafolder, ".", "seed_backup.json", "", "");
                std::rethrow_exception(std::current_exception());
            }
        }

        return std::move(output);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::api::session
{
auto Notary::DefaultMintKeyBytes() noexcept -> std::size_t { return 1536u; }
}  // namespace opentxs::api::session

namespace opentxs::api::session::imp
{
Notary::Notary(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const opentxs::network::zeromq::Context& context,
    const UnallocatedCString& dataFolder,
    const int instance)
    : Session(
          parent,
          running,
          std::move(args),
          crypto,
          config,
          context,
          dataFolder,
          instance,
          [&](const auto& zmq, const auto& endpoints, auto& config) {
              return factory::NetworkAPI(
                  *this,
                  parent.Asio(),
                  zmq,
                  endpoints,
                  factory::BlockchainNetworkAPINull(),
                  config,
                  true);
          },
          factory::SessionFactoryAPI(*this))
    , reason_(factory_.PasswordPrompt("Notary operation"))
    , server_p_(new opentxs::server::Server(*this, reason_))
    , server_(*server_p_)
    , message_processor_p_(
          new opentxs::server::MessageProcessor(server_, reason_))
    , message_processor_(*message_processor_p_)
    , mint_thread_()
    , mint_lock_()
    , mint_update_lock_()
    , mint_scan_lock_()
    , mints_()
    , mints_to_check_()
    , mint_key_size_(args_.DefaultMintKeyBytes())
{
    wallet_ = factory::WalletAPI(*this);

    OT_ASSERT(wallet_);
    OT_ASSERT(server_p_);
    OT_ASSERT(message_processor_p_);
}

void Notary::Cleanup()
{
    LogDetail()(OT_PRETTY_CLASS())("Shutting down and cleaning up.").Flush();
    shutdown_sender_.Activate();
    message_processor_.cleanup();
    message_processor_p_.reset();
    server_p_.reset();
    Session::cleanup();
}

void Notary::DropIncoming(const int count) const
{
    return message_processor_.DropIncoming(count);
}

void Notary::DropOutgoing(const int count) const
{
    return message_processor_.DropOutgoing(count);
}

void Notary::generate_mint(
    const UnallocatedCString& serverID,
    const UnallocatedCString& unitID,
    const std::uint32_t series) const
{
    const auto unit = Factory().UnitID(unitID);
    const auto server = Factory().ServerID(serverID);
    const auto& nym = server_.GetServerNym();
    auto& mint = GetPrivateMint(unit, series);

    if (mint) {
        LogError()(OT_PRETTY_CLASS())("Mint already exists.").Flush();

        return;
    }

    const UnallocatedCString seriesID =
        UnallocatedCString(SERIES_DIVIDER) + std::to_string(series);
    mint = factory_.Mint(server, nym.ID(), unit);

    OT_ASSERT(mint)

    const auto now = Clock::now();
    const std::chrono::seconds expireInterval(
        std::chrono::hours(MINT_EXPIRE_MONTHS * 30 * 24));
    const std::chrono::seconds validInterval(
        std::chrono::hours(MINT_VALID_MONTHS * 30 * 24));
    const auto expires = now + expireInterval;
    const auto validTo = now + validInterval;

    if (false == verify_mint_directory(serverID)) {
        LogError()(OT_PRETTY_CLASS())("Failed to create mint directory.")
            .Flush();

        return;
    }

    auto& internal = mint.Internal();
    internal.GenerateNewMint(
        *wallet_,
        series,
        now,
        validTo,
        expires,
        unit,
        server,
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
        mint_key_size_.load(),
        reason_);
    opentxs::Lock mintLock(mint_lock_);

    if (mints_.end() != mints_.find(unitID)) {
        mints_.at(unitID).erase(PUBLIC_SERIES);
    }

    internal.SetSavePrivateKeys(true);
    internal.SignContract(nym, reason_);
    internal.SaveContract();
    internal.SaveMint(seriesID.c_str());
    internal.SetSavePrivateKeys(false);
    internal.ReleaseSignatures();
    internal.SignContract(nym, reason_);
    internal.SaveContract();
    internal.SaveMint(PUBLIC_SERIES);
    internal.SaveMint();
}

auto Notary::GetAdminNym() const -> UnallocatedCString
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

auto Notary::GetAdminPassword() const -> UnallocatedCString
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

auto Notary::GetPrivateMint(
    const identifier::UnitDefinition& unitID,
    std::uint32_t index) const noexcept -> otx::blind::Mint&
{
    auto lock = opentxs::Lock{mint_lock_};
    const UnallocatedCString id{unitID.str()};
    const UnallocatedCString seriesID =
        UnallocatedCString(SERIES_DIVIDER) + std::to_string(index);
    auto& seriesMap = mints_[id];
    // Modifying the private version may invalidate the public version
    seriesMap.erase(PUBLIC_SERIES);

    auto& output = [&]() -> auto&
    {
        if (auto it = seriesMap.find(seriesID); seriesMap.end() != it) {

            return it->second;
        }

        auto [it, added] = seriesMap.emplace(seriesID, *this);

        OT_ASSERT(added);

        return it->second;
    }
    ();

    if (!output) { output = load_private_mint(lock, id, seriesID); }

    return output;
}

auto Notary::GetPublicMint(const identifier::UnitDefinition& unitID)
    const noexcept -> otx::blind::Mint&
{
    auto lock = opentxs::Lock{mint_lock_};
    const UnallocatedCString id{unitID.str()};
    const UnallocatedCString seriesID{PUBLIC_SERIES};
    auto& output = [&]() -> auto&
    {
        auto& map = mints_[id];

        if (auto it = map.find(seriesID); map.end() != it) {

            return it->second;
        }

        auto [it, added] = map.emplace(seriesID, *this);

        OT_ASSERT(added);

        return it->second;
    }
    ();

    if (!output) { output = load_public_mint(lock, id, seriesID); }

    return output;
}

auto Notary::GetUserName() const -> UnallocatedCString
{
    return args_.NotaryName();
}

auto Notary::GetUserTerms() const -> UnallocatedCString
{
    return args_.NotaryTerms();
}

auto Notary::ID() const -> const identifier::Notary&
{
    return server_.GetServerID();
}

void Notary::Init()
{
    mint_thread_ = std::thread(&Notary::mint, this);
    Scheduler::Start(storage_.get(), network_.DHT());
    Storage::init(factory_, crypto_.Seed());

    Start();
}

auto Notary::InprocEndpoint() const -> UnallocatedCString
{
    return opentxs::network::zeromq::MakeDeterministicInproc(
        "notary", instance_, 1);
}

auto Notary::last_generated_series(
    const UnallocatedCString& serverID,
    const UnallocatedCString& unitID) const -> std::int32_t
{
    std::uint32_t output{0};

    for (output = 0; output < MAX_MINT_SERIES; ++output) {
        const UnallocatedCString filename =
            unitID + SERIES_DIVIDER + std::to_string(output);
        const auto exists = OTDB::Exists(
            *this,
            data_folder_,
            parent_.Internal().Legacy().Mint(),
            serverID.c_str(),
            filename.c_str(),
            "");

        if (false == exists) { return output - 1; }
    }

    return -1;
}

auto Notary::load_private_mint(
    const opentxs::Lock& lock,
    const UnallocatedCString& unitID,
    const UnallocatedCString seriesID) const -> otx::blind::Mint
{
    return verify_mint(
        lock,
        unitID,
        seriesID,
        factory_.Mint(ID(), NymID(), Factory().UnitID(unitID)));
}

auto Notary::load_public_mint(
    const opentxs::Lock& lock,
    const UnallocatedCString& unitID,
    const UnallocatedCString seriesID) const -> otx::blind::Mint
{
    return verify_mint(
        lock, unitID, seriesID, factory_.Mint(ID(), Factory().UnitID(unitID)));
}

void Notary::mint() const
{
    opentxs::Lock updateLock(mint_update_lock_, std::defer_lock);

    const UnallocatedCString serverID{server_.GetServerID().str()};

    OT_ASSERT(false == serverID.empty());

    while (running_) {
        Sleep(250ms);

        if (false == opentxs::server::ServerSettings::__cmd_get_mint) {
            continue;
        }

        UnallocatedCString unitID{""};
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

        auto& mint = GetPrivateMint(Factory().UnitID(unitID), last);

        if (!mint) {
            LogError()(OT_PRETTY_CLASS())("Failed to load existing series.")
                .Flush();

            continue;
        }

        const auto now = Clock::now();
        const auto expires = mint.GetExpiration();
        const std::chrono::seconds limit(
            std::chrono::hours(24 * MINT_GENERATE_DAYS));
        const bool generate = ((now + limit) > expires);

        if (generate) {
            generate_mint(serverID, unitID, next);
        } else {
            LogDetail()(OT_PRETTY_CLASS())("Existing mint file for ")(
                unitID)(" is still valid.")
                .Flush();
        }
    }
}

auto Notary::NymID() const -> const identifier::Nym&
{
    return server_.GetServerNym().ID();
}

void Notary::ScanMints() const
{
    opentxs::Lock scanLock(mint_scan_lock_);
    opentxs::Lock updateLock(mint_update_lock_, std::defer_lock);
    const auto units = wallet_->UnitDefinitionList();

    for (const auto& it : units) {
        const auto& id = it.first;
        updateLock.lock();
        mints_to_check_.push_front(id);
        updateLock.unlock();
    }
}

void Notary::Start()
{
    server_.Init();
    server_.ActivateCron();
    UnallocatedCString hostname{};
    std::uint32_t port{0};
    AddressType type{AddressType::Inproc};
    const auto connectInfo = server_.GetConnectInfo(type, hostname, port);

    OT_ASSERT(connectInfo);

    auto pubkey = Data::Factory();
    auto privateKey = server_.TransportKey(pubkey);
    message_processor_.init((AddressType::Inproc == type), port, privateKey);
    message_processor_.Start();
    ScanMints();
}

void Notary::UpdateMint(const identifier::UnitDefinition& unitID) const
{
    opentxs::Lock updateLock(mint_update_lock_);
    mints_to_check_.push_front(unitID.str());
}

auto Notary::verify_lock(const opentxs::Lock& lock, const std::mutex& mutex)
    const -> bool
{
    if (lock.mutex() != &mutex) {
        LogError()(OT_PRETTY_CLASS())("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()(OT_PRETTY_CLASS())("Lock not owned.").Flush();

        return false;
    }

    return true;
}

auto Notary::verify_mint(
    const opentxs::Lock& lock,
    const UnallocatedCString& unitID,
    const UnallocatedCString seriesID,
    otx::blind::Mint&& mint) const -> otx::blind::Mint
{
    OT_ASSERT(verify_lock(lock, mint_lock_));

    if (mint) {
        auto& internal = mint.Internal();

        if (false == internal.LoadMint(seriesID.c_str())) {
            UpdateMint(Factory().UnitID(unitID));

            return otx::blind::Mint{*this};
        }

        if (false == internal.VerifyMint(server_.GetServerNym())) {
            LogError()(OT_PRETTY_CLASS())("Invalid mint for ")(unitID).Flush();

            return otx::blind::Mint{*this};
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("Missing mint for ")(unitID).Flush();
    }

    return std::move(mint);
}

auto Notary::verify_mint_directory(const UnallocatedCString& serverID) const
    -> bool
{
    auto serverDir = String::Factory();
    auto mintDir = String::Factory();
    const auto haveMint = parent_.Internal().Legacy().AppendFolder(
        mintDir,
        String::Factory(data_folder_.c_str()),
        String::Factory(parent_.Internal().Legacy().Mint()));
    const auto haveServer = parent_.Internal().Legacy().AppendFolder(
        serverDir, mintDir, String::Factory(serverID.c_str()));

    OT_ASSERT(haveMint)
    OT_ASSERT(haveServer)

    return parent_.Internal().Legacy().BuildFolderPath(serverDir);
}

Notary::~Notary()
{
    running_.Off();

    if (mint_thread_.joinable()) { mint_thread_.join(); }

    Cleanup();
}
}  // namespace opentxs::api::session::imp
