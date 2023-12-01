// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/NotaryPrivate.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <compare>
#include <functional>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/otx/blind/Mint.hpp"
#include "internal/otx/server/MessageProcessor.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Context.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Network.internal.hpp"
#include "opentxs/api/Paths.internal.hpp"
#include "opentxs/api/SessionPrivate.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Settings.internal.hpp"
#include "opentxs/api/internal.factory.hpp"
#include "opentxs/api/network/Blockchain.hpp"  // IWYU pragma: keep
#include "opentxs/api/network/internal.factory.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/api/session/base/Storage.hpp"
#include "opentxs/api/session/internal.factory.hpp"
#include "opentxs/api/session/notary/Actor.hpp"
#include "opentxs/api/session/notary/Shared.hpp"
#include "opentxs/core/AddressType.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"  // IWYU pragma: keep
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.internal.hpp"
#include "otx/common/OTStorage.hpp"
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

namespace opentxs::api::session
{
auto Notary::DefaultMintKeyBytes() noexcept -> std::size_t { return 1536_uz; }
}  // namespace opentxs::api::session

namespace opentxs::api::session
{
NotaryPrivate::NotaryPrivate(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const opentxs::network::zeromq::Context& context,
    const std::filesystem::path& dataFolder,
    const int instance)
    : SessionPrivate(
          parent,
          running,
          std::move(args),
          crypto,
          config,
          context,
          dataFolder,
          instance,
          [&](const auto& zmq, const auto& endpoints, auto& scheduler) {
              return factory::NetworkAPI(
                  *this,
                  parent.Asio(),
                  zmq,
                  parent.ZAP(),
                  endpoints,
                  factory::BlockchainNetworkAPINull());
          },
          factory::SessionFactoryAPI(
              *static_cast<internal::Notary*>(this),
              parent.Factory()))
    , self_(this)
    , reason_(factory_.PasswordPrompt("Notary operation"))
    , shared_p_(std::make_shared<notary::Shared>(context))
    , server_p_(new opentxs::server::Server(self_, reason_))
    , message_processor_p_(
          new opentxs::server::MessageProcessor(*server_p_, reason_))
    , shared_(*shared_p_)
    , server_(*server_p_)
    , message_processor_(*message_processor_p_)
    , mint_key_size_(args_.DefaultMintKeyBytes())
    , me_()
{
    wallet_ = factory::WalletAPI(self_);

    assert_false(nullptr == wallet_);
    assert_false(nullptr == shared_p_);
    assert_false(nullptr == server_p_);
    assert_false(nullptr == message_processor_p_);
}

auto NotaryPrivate::CheckMint(const identifier::UnitDefinition& unitID) noexcept
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
        LogError()()("Failed to load existing series.").Flush();

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
        LogDetail()()("Existing mint file for ")(unitID, crypto_)(
            " is still valid.")
            .Flush();
    }
}

auto NotaryPrivate::Cleanup() -> void
{
    LogDetail()()("Shutting down and cleaning up.").Flush();
    message_processor_.cleanup();
    message_processor_p_.reset();
    server_p_.reset();
    SessionPrivate::cleanup();
}

auto NotaryPrivate::DropIncoming(const int count) const -> void
{
    return message_processor_.DropIncoming(count);
}

auto NotaryPrivate::DropOutgoing(const int count) const -> void
{
    return message_processor_.DropOutgoing(count);
}

auto NotaryPrivate::generate_mint(
    notary::Shared::Map& data,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID,
    const std::uint32_t series) const -> void
{
    const auto& nym = server_.GetServerNym();
    auto& mint = get_private_mint(data, unitID, series);

    if (mint) {
        LogError()()("Mint already exists.").Flush();

        return;
    }

    const auto seriesID =
        notary::MintSeriesID{SERIES_DIVIDER}.append(std::to_string(series));
    mint = factory_.Mint(serverID, nym.ID(), unitID);

    assert_true(mint);

    const auto now = Clock::now();
    const std::chrono::seconds expireInterval(
        std::chrono::hours(MINT_EXPIRE_MONTHS * 30 * 24));
    const std::chrono::seconds validInterval(
        std::chrono::hours(MINT_VALID_MONTHS * 30 * 24));
    const auto expires = now + expireInterval;
    const auto validTo = now + validInterval;

    if (false == verify_mint_directory(data, serverID)) {
        LogError()()("Failed to create mint directory.").Flush();

        return;
    }

    auto& internal = mint.Internal();
    internal.GenerateNewMint(
        wallet_->Self(),
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

auto NotaryPrivate::GetAdminNym() const -> UnallocatedCString
{
    auto output = String::Factory();
    bool exists{false};
    const auto success = config_.Internal().Check_str(
        String::Factory("permissions"),
        String::Factory("override_nym_id"),
        output,
        exists);

    if (success && exists) { return output->Get(); }

    return {};
}

auto NotaryPrivate::GetAdminPassword() const -> UnallocatedCString
{
    auto output = String::Factory();
    bool exists{false};
    const auto success = config_.Internal().Check_str(
        String::Factory("permissions"),
        String::Factory("admin_password"),
        output,
        exists);

    if (success && exists) { return output->Get(); }

    return {};
}

auto NotaryPrivate::GetOwnerName() const -> std::string_view
{
    return args_.NotaryName();
}

auto NotaryPrivate::GetPrivateMint(
    const identifier::UnitDefinition& unitID,
    std::uint32_t index) const noexcept -> otx::blind::Mint&
{
    auto handle = shared_.data_.lock();
    auto& mints = *handle;

    return get_private_mint(mints, unitID, index);
}

auto NotaryPrivate::get_private_mint(
    notary::Shared::Map& mints,
    const identifier::UnitDefinition& unitID,
    std::uint32_t index) const noexcept -> otx::blind::Mint&
{
    const auto seriesID =
        notary::MintSeriesID{SERIES_DIVIDER}.append(std::to_string(index));
    auto& seriesMap = mints[unitID];
    // Modifying the private version may invalidate the public version
    seriesMap.erase(PUBLIC_SERIES);
    auto& output = [&]() -> auto& {
        if (auto it = seriesMap.find(seriesID); seriesMap.end() != it) {

            return it->second;
        }

        auto [it, added] = seriesMap.emplace(seriesID, self_);

        assert_true(added);

        return it->second;
    }();

    if (!output) { output = load_private_mint(mints, unitID, seriesID); }

    return output;
}

auto NotaryPrivate::GetPublicMint(const identifier::UnitDefinition& unitID)
    const noexcept -> otx::blind::Mint&
{
    static const auto seriesID = notary::MintSeriesID{PUBLIC_SERIES};
    auto handle = shared_.data_.lock();
    auto& mints = *handle;
    auto& output = [&]() -> auto& {
        auto& map = mints[unitID];

        if (auto it = map.find(seriesID); map.end() != it) {

            return it->second;
        }

        auto [it, added] = map.emplace(seriesID, self_);

        assert_true(added);

        return it->second;
    }();

    if (!output) { output = load_public_mint(mints, unitID, seriesID); }

    return output;
}

auto NotaryPrivate::GetShared() const noexcept
    -> std::shared_ptr<const api::internal::Session>
{
    wait_for_init();
    auto out = me_.lock();

    assert_false(nullptr == out);

    return out;
}

auto NotaryPrivate::GetUserTerms() const -> std::string_view
{
    return args_.NotaryTerms();
}

auto NotaryPrivate::ID() const -> const identifier::Notary&
{
    return server_.GetServerID();
}

void NotaryPrivate::Init(std::shared_ptr<internal::Notary> me)
{
    Storage::init(crypto_, factory_, crypto_.Seed());
    start(me);
}

auto NotaryPrivate::InprocEndpoint() const -> UnallocatedCString
{
    return opentxs::network::zeromq::MakeDeterministicInproc(
        "notary", instance_, 1);
}

auto NotaryPrivate::last_generated_series(
    notary::Shared::Map& data,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID) const -> std::int32_t
{
    std::uint32_t output{0};

    for (output = 0; output < MAX_MINT_SERIES; ++output) {
        const UnallocatedCString filename =
            unitID.asBase58(crypto_) + SERIES_DIVIDER + std::to_string(output);
        const auto exists = OTDB::Exists(
            self_,
            data_folder_.string(),
            parent_.Internal().Paths().Mint(),
            serverID.asBase58(crypto_).c_str(),
            filename.c_str(),
            "");

        if (false == exists) { return output - 1; }
    }

    return -1;
}

auto NotaryPrivate::load_private_mint(
    notary::Shared::Map& data,
    const identifier::UnitDefinition& unitID,
    const notary::MintSeriesID& seriesID) const -> otx::blind::Mint
{
    return verify_mint(
        data, unitID, seriesID, factory_.Mint(ID(), NymID(), unitID));
}

auto NotaryPrivate::load_public_mint(
    notary::Shared::Map& data,
    const identifier::UnitDefinition& unitID,
    const notary::MintSeriesID& seriesID) const -> otx::blind::Mint
{
    return verify_mint(data, unitID, seriesID, factory_.Mint(ID(), unitID));
}

auto NotaryPrivate::NymID() const -> const identifier::Nym&
{
    return server_.GetServerNym().ID();
}

auto NotaryPrivate::Start(std::shared_ptr<internal::Notary> api) noexcept
    -> void
{
    me_ = api;
    auto me = me_.lock();

    assert_false(nullptr == me);

    SessionPrivate::start(api);
    network_.Internal().Start(
        me,
        crypto_.Blockchain(),
        parent_.Internal().Paths(),
        data_folder_,
        args_);
}

auto NotaryPrivate::start(std::shared_ptr<internal::Notary> me) -> void
{
    server_.Init();
    server_.ActivateCron();
    UnallocatedCString hostname{};
    std::uint32_t port{0};
    AddressType type{AddressType::Inproc};
    const auto connectInfo = server_.GetConnectInfo(type, hostname, port);

    assert_true(connectInfo);

    auto pubkey = ByteArray{};
    auto privateKey = server_.TransportKey(pubkey);
    message_processor_.init((AddressType::Inproc == type), port, privateKey);
    message_processor_.Start();

    if (opentxs::server::ServerSettings::_cmd_get_mint) {
        auto actor = std::allocate_shared<notary::Actor>(
            alloc::PMR<notary::Actor>{shared_.get_allocator()}, me, shared_p_);

        assert_false(nullptr == actor);

        actor->Init(actor);
    }
}

auto NotaryPrivate::UpdateMint(const identifier::UnitDefinition& unitID) const
    -> void
{
    shared_.to_actor_.lock()->SendDeferred([&] {
        auto out = MakeWork(notary::Job::queue_unitid);
        out.AddFrame(unitID);

        return out;
    }());
}

auto NotaryPrivate::verify_mint(
    notary::Shared::Map& data,
    const identifier::UnitDefinition& unitID,
    const notary::MintSeriesID& seriesID,
    otx::blind::Mint&& mint) const -> otx::blind::Mint
{
    if (mint) {
        auto& internal = mint.Internal();

        if (false == internal.LoadMint(seriesID.c_str())) {
            UpdateMint(unitID);

            return otx::blind::Mint{self_};
        }

        if (false == internal.VerifyMint(server_.GetServerNym())) {
            LogError()()("Invalid mint for ")(unitID, crypto_).Flush();

            return otx::blind::Mint{self_};
        }
    } else {
        LogError()()("Missing mint for ")(unitID, crypto_).Flush();
    }

    return std::move(mint);
}

auto NotaryPrivate::verify_mint_directory(
    notary::Shared::Map& data,
    const identifier::Notary& serverID) const -> bool
{
    auto serverDir = std::filesystem::path{};
    auto mintDir = std::filesystem::path{};
    const auto haveMint = parent_.Internal().Paths().AppendFolder(
        mintDir, data_folder_, parent_.Internal().Paths().Mint());
    const auto haveServer = parent_.Internal().Paths().AppendFolder(
        serverDir, mintDir, serverID.asBase58(crypto_).c_str());

    assert_true(haveMint);
    assert_true(haveServer);

    return parent_.Internal().Paths().BuildFolderPath(serverDir);
}

NotaryPrivate::~NotaryPrivate()
{
    running_.Off();
    Cleanup();
    shutdown_complete();
    Detach(self_);
}
}  // namespace opentxs::api::session
