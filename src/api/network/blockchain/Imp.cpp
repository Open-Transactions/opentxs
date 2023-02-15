// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/blockchain/Imp.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <future>
#include <iterator>
#include <utility>

#include "api/network/blockchain/Actor.hpp"
#include "api/network/blockchain/BlockchainHandle.hpp"
#include "blockchain/database/common/Database.hpp"
#include "blockchain/node/stats/Imp.hpp"
#include "blockchain/node/stats/Shared.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/blockchain/Protocol.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::api::network::implementation
{
BlockchainImp::BlockchainImp(
    const api::Session& api,
    const api::session::Endpoints& endpoints,
    const opentxs::network::zeromq::Context& zmq) noexcept
    : api_(api)
    , crypto_(nullptr)
    , chain_state_publisher_([&] {
        auto out = zmq.Internal().PublishSocket();
        auto rc = out->Start(endpoints.BlockchainStateChange().data());

        OT_ASSERT(rc);

        return out;
    }())
    , startup_publisher_(endpoints, zmq)
    , base_config_()
    , db_()
    , lock_()
    , config_()
    , networks_()
    , stats_()
    , running_(true)
{
}

auto BlockchainImp::AddSyncServer(
    const std::string_view endpoint) const noexcept -> bool
{
    return db_.get().AddSyncServer(endpoint);
}

auto BlockchainImp::DeleteSyncServer(
    const std::string_view endpoint) const noexcept -> bool
{
    return db_.get().DeleteSyncServer(endpoint);
}

auto BlockchainImp::Disable(const Chain type) const noexcept -> bool
{
    auto lock = Lock{lock_};

    return disable(lock, type);
}

auto BlockchainImp::disable(const Lock& lock, const Chain type) const noexcept
    -> bool
{
    if (0 == opentxs::blockchain::SupportedChains().count(type)) {
        LogError()(OT_PRETTY_CLASS())("Unsupported chain").Flush();

        return false;
    }

    stop(lock, type);

    if (db_.get().Disable(type)) { return true; }

    LogError()(OT_PRETTY_CLASS())("Database update failure").Flush();

    return false;
}

auto BlockchainImp::Enable(const Chain type, const std::string_view seednode)
    const noexcept -> bool
{
    auto lock = Lock{lock_};

    return enable(lock, type, seednode);
}

auto BlockchainImp::enable(
    const Lock& lock,
    const Chain type,
    const std::string_view seednode) const noexcept -> bool
{
    if (0 == opentxs::blockchain::SupportedChains().count(type)) {
        LogError()(OT_PRETTY_CLASS())("Unsupported chain").Flush();

        return false;
    }

    if (false == db_.get().Enable(type, seednode)) {
        LogError()(OT_PRETTY_CLASS())("Database error").Flush();

        return false;
    }

    return start(lock, type, seednode);
}

auto BlockchainImp::EnabledChains(alloc::Default alloc) const noexcept
    -> Set<Chain>
{
    auto out = Set<Chain>{alloc};
    const auto data = [&] {
        auto lock = Lock{lock_};

        return db_.get().LoadEnabledChains();
    }();
    std::transform(
        data.begin(),
        data.end(),
        std::inserter(out, out.begin()),
        [](const auto value) { return value.first; });

    return out;
}

auto BlockchainImp::GetChain(const Chain type) const noexcept(false)
    -> BlockchainHandle
{
    auto lock = Lock{lock_};

    return std::make_unique<BlockchainHandle::Imp>(networks_.at(type))
        .release();
}

auto BlockchainImp::GetSyncServers(alloc::Default alloc) const noexcept
    -> Endpoints
{
    return db_.get().GetSyncServers(alloc);
}

auto BlockchainImp::Init(
    std::shared_ptr<const api::Session> api,
    const api::crypto::Blockchain& crypto,
    const api::Legacy& legacy,
    const std::filesystem::path& dataFolder,
    const Options& options) noexcept -> void
{
    OT_ASSERT(api);

    crypto_ = &crypto;
    base_config_.set_value([&] {
        auto output = Config{};
        output.profile_ = options.BlockchainProfile();

        switch (output.profile_) {
            case BlockchainProfile::mobile:
            case BlockchainProfile::desktop:
            case BlockchainProfile::desktop_native: {
                output.disable_wallet_ = !options.BlockchainWalletEnabled();
            } break;
            case BlockchainProfile::server: {
                if (options.ProvideBlockchainSyncServer()) {
                    output.provide_sync_server_ = true;
                    output.disable_wallet_ = true;
                } else {
                    output.disable_wallet_ = !options.BlockchainWalletEnabled();
                }
            } break;
            default: {
                LogAbort()(OT_PRETTY_CLASS())("invalid profile").Abort();
            }
        }

        return output;
    }());
    db_.set_value(DB{api_, crypto, legacy, dataFolder, options});
    {
        const auto& zmq = api->Network().ZeroMQ().Internal();
        const auto batchID = zmq.PreallocateBatch();
        auto* alloc = zmq.Alloc(batchID);
        // TODO the version of libc++ present in android ndk 23.0.7599858 has a
        // broken std::allocate_shared function so we're using boost::shared_ptr
        // instead of std::shared_ptr
        auto actor = boost::allocate_shared<blockchain::Actor>(
            alloc::PMR<blockchain::Actor>{alloc}, api, batchID);

        OT_ASSERT(actor);

        actor->Init(actor);
    }
    stats_ = std::make_shared<opentxs::blockchain::node::stats::Shared>();
    stats_->Start(api, stats_);
}

auto BlockchainImp::IsEnabled(
    const opentxs::blockchain::Type chain) const noexcept -> bool
{
    auto lock = Lock{lock_};

    for (const auto& [enabled, peer] : db_.get().LoadEnabledChains()) {
        if (chain == enabled) { return true; }
    }

    return false;
}

auto BlockchainImp::Profile() const noexcept -> BlockchainProfile
{
    return base_config_.get().profile_;
}

auto BlockchainImp::publish_chain_state(Chain type, bool state) const -> void
{
    chain_state_publisher_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::BlockchainStateChange, true);
        work.AddFrame(type);
        work.AddFrame(state);

        return work;
    }());
}

auto BlockchainImp::RestoreNetworks() const noexcept -> void
{
    auto lock = Lock{lock_};

    for (const auto& [chain, peer] : db_.get().LoadEnabledChains()) {
        start(lock, chain, peer, false);
    }

    for (auto& [chain, node] : networks_) { node->Internal().StartWallet(); }
}

auto BlockchainImp::Shutdown() noexcept -> void
{
    if (running_.exchange(false)) {
        LogVerbose()("Shutting down ")(networks_.size())(" blockchain clients")
            .Flush();

        for (auto& [chain, network] : networks_) {
            network->Internal().Shutdown().get();
        }

        networks_.clear();
    }

    Imp::Shutdown();
}

auto BlockchainImp::Start(const Chain type, const std::string_view seednode)
    const noexcept -> bool
{
    auto lock = Lock{lock_};

    return start(lock, type, seednode);
}

auto BlockchainImp::start(
    const Lock& lock,
    const Chain type,
    const std::string_view seednode,
    const bool startWallet) const noexcept -> bool
{
    if (Chain::UnitTest != type) {
        if (0 == opentxs::blockchain::SupportedChains().count(type)) {
            LogError()(OT_PRETTY_CLASS())("Unsupported chain").Flush();

            return false;
        }
    }

    if (0 != networks_.count(type)) {
        LogVerbose()(OT_PRETTY_CLASS())("Chain already running").Flush();

        return true;
    } else {
        LogConsole()("Starting ")(print(type))(" client").Flush();
    }

    using enum opentxs::network::blockchain::Protocol;

    switch (opentxs::blockchain::params::get(type).P2PDefaultProtocol()) {
        case bitcoin: {
            const auto& config = [&]() -> const Config& {
                {
                    auto it = config_.find(type);

                    if (config_.end() != it) { return it->second; }
                }

                auto [it, added] = config_.emplace(type, base_config_);

                OT_ASSERT(added);

                if (false == api_.GetOptions().TestMode()) {
                    switch (type) {
                        case Chain::UnitTest: {
                        } break;
                        default: {
                            auto& profile = it->second.profile_;

                            if (BlockchainProfile::desktop_native == profile) {
                                profile = BlockchainProfile::desktop;
                            }
                        }
                    }
                }

                return it->second;
            }();
            auto [it, added] = networks_.emplace(
                type,
                factory::BlockchainNetworkBitcoin(
                    api_, type, config, seednode));
            LogConsole()(print(type))(" client is running").Flush();
            publish_chain_state(type, true);
            auto& pnode = it->second;
            auto& node = *pnode;
            node.Internal().Start(pnode);

            if (startWallet) { node.Internal().StartWallet(); }

            return true;
        }
        case opentxs:
        case ethereum:
        default: {
        }
    }

    return false;
}

auto BlockchainImp::Stats() const noexcept -> opentxs::blockchain::node::Stats
{
    OT_ASSERT(stats_);

    using StatsImp = opentxs::blockchain::node::Stats::Imp;

    return std::make_unique<StatsImp>(stats_).release();
}

auto BlockchainImp::Stop(const Chain type) const noexcept -> bool
{
    auto lock = Lock{lock_};

    return stop(lock, type);
}

auto BlockchainImp::stop(const Lock& lock, const Chain type) const noexcept
    -> bool
{
    auto it = networks_.find(type);

    if (networks_.end() == it) { return true; }

    OT_ASSERT(it->second);

    it->second->Internal().Shutdown().get();
    networks_.erase(it);
    LogVerbose()(OT_PRETTY_CLASS())("stopped chain ")(print(type)).Flush();
    publish_chain_state(type, false);

    return true;
}

BlockchainImp::~BlockchainImp() = default;
}  // namespace opentxs::api::network::implementation
