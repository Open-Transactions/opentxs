// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "api/session/notary/Notary.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <chrono>
#include <compare>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <utility>

#include "api/session/Session.hpp"
#include "api/session/base/Scheduler.hpp"
#include "api/session/base/Storage.hpp"
#include "api/session/notary/Actor.hpp"
#include "api/session/notary/Shared.hpp"
#include "internal/api/Context.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/otx/blind/Mint.hpp"
#include "internal/otx/server/MessageProcessor.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"  // IWYU pragma: keep
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "otx/common/OTStorage.hpp"
#include "otx/server/Server.hpp"
#include "otx/server/ServerSettings.hpp"
#include "util/Work.hpp"

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
    const std::filesystem::path& dataFolder,
    const int instance) -> std::shared_ptr<api::session::Notary>
{
    using ReturnType = api::session::imp::Notary;

    try {
        auto output = std::make_shared<ReturnType>(
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
                output->Init(output);
            } catch (const std::invalid_argument& e) {
                LogError()("opentxs::factory::")(__func__)(
                    ": There was a problem creating the server. The server "
                    "contract will be deleted. Error: ")(e.what())
                    .Flush();
                const UnallocatedCString datafolder =
                    output->DataFolder().string();
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

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::api::session
{
auto Notary::DefaultMintKeyBytes() noexcept -> std::size_t { return 1536_uz; }
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
    const std::filesystem::path& dataFolder,
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
                  factory::BlockchainNetworkAPINull());
          },
          factory::SessionFactoryAPI(*this))
    , reason_(factory_.PasswordPrompt("Notary operation"))
    , shared_p_(boost::make_shared<notary::Shared>(context))
    , server_p_(new opentxs::server::Server(*this, reason_))
    , message_processor_p_(
          new opentxs::server::MessageProcessor(*server_p_, reason_))
    , shared_(*shared_p_)
    , server_(*server_p_)
    , message_processor_(*message_processor_p_)
    , mint_key_size_(args_.DefaultMintKeyBytes())
{
    wallet_ = factory::WalletAPI(*this);

    OT_ASSERT(wallet_);
    OT_ASSERT(shared_p_);
    OT_ASSERT(server_p_);
    OT_ASSERT(message_processor_p_);
}

auto Notary::CheckMint(const identifier::UnitDefinition& unitID) noexcept
    -> void
{
    if (unitID.empty()) { return; }

    const auto& serverID = server_.GetServerID();
    auto handle = shared_.data_.lock();
    auto& data = *handle;
    const auto last = last_generated_series(data, serverID, unitID);
    const auto next = last + 1;

    if (0 > last) {
        generate_mint(data, serverID, unitID, 0);

        return;
    }

    auto& mint = get_private_mint(data, unitID, last);

    if (!mint) {
        LogError()(OT_PRETTY_CLASS())("Failed to load existing series.")
            .Flush();

        return;
    }

    const auto now = Clock::now();
    const auto expires = mint.GetExpiration();
    const std::chrono::seconds limit(
        std::chrono::hours(24 * MINT_GENERATE_DAYS));
    const bool generate = ((now + limit) > expires);

    if (generate) {
        generate_mint(data, serverID, unitID, next);
    } else {
        LogDetail()(OT_PRETTY_CLASS())("Existing mint file for ")(
            unitID)(" is still valid.")
            .Flush();
    }
}

auto Notary::Cleanup() -> void
{
    LogDetail()(OT_PRETTY_CLASS())("Shutting down and cleaning up.").Flush();
    message_processor_.cleanup();
    message_processor_p_.reset();
    server_p_.reset();
    Session::cleanup();
}

auto Notary::DropIncoming(const int count) const -> void
{
    return message_processor_.DropIncoming(count);
}

auto Notary::DropOutgoing(const int count) const -> void
{
    return message_processor_.DropOutgoing(count);
}

auto Notary::generate_mint(
    notary::Shared::Map& data,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID,
    const std::uint32_t series) const -> void
{
    const auto& nym = server_.GetServerNym();
    auto& mint = get_private_mint(data, unitID, series);

    if (mint) {
        LogError()(OT_PRETTY_CLASS())("Mint already exists.").Flush();

        return;
    }

    const auto seriesID =
        notary::MintSeriesID{SERIES_DIVIDER}.append(std::to_string(series));
    mint = factory_.Mint(serverID, nym.ID(), unitID);

    OT_ASSERT(mint);

    const auto now = Clock::now();
    const std::chrono::seconds expireInterval(
        std::chrono::hours(MINT_EXPIRE_MONTHS * 30 * 24));
    const std::chrono::seconds validInterval(
        std::chrono::hours(MINT_VALID_MONTHS * 30 * 24));
    const auto expires = now + expireInterval;
    const auto validTo = now + validInterval;

    if (false == verify_mint_directory(data, serverID)) {
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
        unitID,
        serverID,
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

    if (auto i = data.find(unitID); data.end() != i) {
        i->second.erase(PUBLIC_SERIES);
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
    internal.SaveMint({});
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
    auto handle = shared_.data_.lock();
    auto& mints = *handle;

    return get_private_mint(mints, unitID, index);
}

auto Notary::get_private_mint(
    notary::Shared::Map& mints,
    const identifier::UnitDefinition& unitID,
    std::uint32_t index) const noexcept -> otx::blind::Mint&
{
    const auto seriesID =
        notary::MintSeriesID{SERIES_DIVIDER}.append(std::to_string(index));
    auto& seriesMap = mints[unitID];
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

    if (!output) { output = load_private_mint(mints, unitID, seriesID); }

    return output;
}

auto Notary::GetPublicMint(const identifier::UnitDefinition& unitID)
    const noexcept -> otx::blind::Mint&
{
    static const auto seriesID = notary::MintSeriesID{PUBLIC_SERIES};
    auto handle = shared_.data_.lock();
    auto& mints = *handle;
    auto& output = [&]() -> auto&
    {
        auto& map = mints[unitID];

        if (auto it = map.find(seriesID); map.end() != it) {

            return it->second;
        }

        auto [it, added] = map.emplace(seriesID, *this);

        OT_ASSERT(added);

        return it->second;
    }
    ();

    if (!output) { output = load_public_mint(mints, unitID, seriesID); }

    return output;
}

auto Notary::GetUserName() const -> UnallocatedCString
{
    return UnallocatedCString{args_.NotaryName()};
}

auto Notary::GetUserTerms() const -> UnallocatedCString
{
    return UnallocatedCString{args_.NotaryTerms()};
}

auto Notary::ID() const -> const identifier::Notary&
{
    return server_.GetServerID();
}

void Notary::Init(std::shared_ptr<session::Notary> me)
{
    Scheduler::Start(storage_.get());
    Storage::init(factory_, crypto_.Seed());
    Start(me);
}

auto Notary::InprocEndpoint() const -> UnallocatedCString
{
    return opentxs::network::zeromq::MakeDeterministicInproc(
        "notary", instance_, 1);
}

auto Notary::last_generated_series(
    notary::Shared::Map& data,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID) const -> std::int32_t
{
    std::uint32_t output{0};

    for (output = 0; output < MAX_MINT_SERIES; ++output) {
        const UnallocatedCString filename =
            unitID.asBase58(crypto_) + SERIES_DIVIDER + std::to_string(output);
        const auto exists = OTDB::Exists(
            *this,
            data_folder_.string(),
            parent_.Internal().Legacy().Mint(),
            serverID.asBase58(crypto_).c_str(),
            filename.c_str(),
            "");

        if (false == exists) { return output - 1; }
    }

    return -1;
}

auto Notary::load_private_mint(
    notary::Shared::Map& data,
    const identifier::UnitDefinition& unitID,
    const notary::MintSeriesID& seriesID) const -> otx::blind::Mint
{
    return verify_mint(
        data, unitID, seriesID, factory_.Mint(ID(), NymID(), unitID));
}

auto Notary::load_public_mint(
    notary::Shared::Map& data,
    const identifier::UnitDefinition& unitID,
    const notary::MintSeriesID& seriesID) const -> otx::blind::Mint
{
    return verify_mint(data, unitID, seriesID, factory_.Mint(ID(), unitID));
}

auto Notary::NymID() const -> const identifier::Nym&
{
    return server_.GetServerNym().ID();
}

auto Notary::Start(std::shared_ptr<session::Notary> me) -> void
{
    server_.Init();
    server_.ActivateCron();
    UnallocatedCString hostname{};
    std::uint32_t port{0};
    AddressType type{AddressType::Inproc};
    const auto connectInfo = server_.GetConnectInfo(type, hostname, port);

    OT_ASSERT(connectInfo);

    auto pubkey = ByteArray{};
    auto privateKey = server_.TransportKey(pubkey);
    message_processor_.init((AddressType::Inproc == type), port, privateKey);
    message_processor_.Start();

    if (opentxs::server::ServerSettings::_cmd_get_mint) {
        OT_ASSERT(me);

        // TODO the version of libc++ present in android ndk 23.0.7599858 has a
        // broken std::allocate_shared function so we're using boost::shared_ptr
        // instead of std::shared_ptr
        auto actor = boost::allocate_shared<notary::Actor>(
            alloc::PMR<notary::Actor>{shared_.get_allocator()}, me, shared_p_);

        OT_ASSERT(actor);

        actor->Init(actor);
    }
}

auto Notary::UpdateMint(const identifier::UnitDefinition& unitID) const -> void
{
    shared_.to_actor_.lock()->SendDeferred(
        [&] {
            auto out = MakeWork(notary::Job::queue_unitid);
            out.AddFrame(unitID);

            return out;
        }(),
        __FILE__,
        __LINE__);
}

auto Notary::verify_mint(
    notary::Shared::Map& data,
    const identifier::UnitDefinition& unitID,
    const notary::MintSeriesID& seriesID,
    otx::blind::Mint&& mint) const -> otx::blind::Mint
{
    if (mint) {
        auto& internal = mint.Internal();

        if (false == internal.LoadMint(seriesID.c_str())) {
            UpdateMint(unitID);

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

auto Notary::verify_mint_directory(
    notary::Shared::Map& data,
    const identifier::Notary& serverID) const -> bool
{
    auto serverDir = std::filesystem::path{};
    auto mintDir = std::filesystem::path{};
    const auto haveMint = parent_.Internal().Legacy().AppendFolder(
        mintDir, data_folder_, parent_.Internal().Legacy().Mint());
    const auto haveServer = parent_.Internal().Legacy().AppendFolder(
        serverDir, mintDir, serverID.asBase58(crypto_).c_str());

    OT_ASSERT(haveMint);
    OT_ASSERT(haveServer);

    return parent_.Internal().Legacy().BuildFolderPath(serverDir);
}

Notary::~Notary()
{
    running_.Off();
    Cleanup();
    shutdown_complete();
}
}  // namespace opentxs::api::session::imp
