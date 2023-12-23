// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/blockchain/Imp.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

#include "api/network/blockchain/Actor.hpp"
#include "api/network/blockchain/BlockchainHandle.hpp"
#include "blockchain/database/common/Database.hpp"
#include "blockchain/node/stats/Imp.hpp"
#include "blockchain/node/stats/Shared.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/api/session/Endpoints.hpp"
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

namespace opentxs::api::network::implementation
{
BlockchainImp::BlockchainImp(
    const api::session::Client& api,
    const api::session::Endpoints& endpoints,
    const opentxs::network::zeromq::Context& zmq) noexcept
    : api_(api)
    , crypto_(nullptr)
    , chain_state_publisher_([&] {
        auto out = zmq.Internal().PublishSocket();
        auto rc = out->Start(endpoints.BlockchainStateChange().data());

        assert_true(rc);

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
    if (false == opentxs::blockchain::is_supported(type)) {
        LogError()()("Unsupported chain").Flush();

        return false;
    }

    stop(lock, type);

    if (db_.get().Disable(type)) { return true; }

    LogError()()("Database update failure").Flush();

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
    if (false == opentxs::blockchain::is_supported(type)) {
        LogError()()("Unsupported chain").Flush();

        return false;
    }

    if (false == db_.get().Enable(type, seednode)) {
        LogError()()("Database error").Flush();

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
    std::ranges::transform(
        data, std::inserter(out, out.begin()), [](const auto value) {
            return value.first;
        });

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
    std::shared_ptr<const api::session::internal::Client> api,
    const api::crypto::Blockchain& crypto,
    const api::internal::Paths& legacy,
    const std::filesystem::path& dataFolder,
    const Options& options) noexcept -> void
{
    assert_false(nullptr == api);

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
                LogAbort()()("invalid profile").Abort();
            }
        }

        return output;
    }());
    db_.set_value(DB{api_, crypto, legacy, dataFolder, options});
    {
        const auto& zmq = api->Network().ZeroMQ().Context().Internal();
        const auto batchID = zmq.PreallocateBatch();
        auto* alloc = zmq.Alloc(batchID);
        auto actor = std::allocate_shared<blockchain::Actor>(
            alloc::PMR<blockchain::Actor>{alloc}, api, batchID);

        assert_false(nullptr == actor);

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
            network->Internal().Shutdown();
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
        if (false == opentxs::blockchain::is_supported(type)) {
            LogError()()("Unsupported chain").Flush();

            return false;
        }
    }

    if (networks_.contains(type)) {
        LogVerbose()()("Chain already running").Flush();

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

                assert_true(added);

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
            node.Internal().Start(
                api_.Internal().asClient().SharedClient(), pnode);

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
    assert_false(nullptr == stats_);

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

    assert_false(nullptr == it->second);

    it->second->Internal().Shutdown();
    networks_.erase(it);
    LogVerbose()()("stopped chain ")(print(type)).Flush();
    publish_chain_state(type, false);

    return true;
}

BlockchainImp::~BlockchainImp() = default;
}  // namespace opentxs::api::network::implementation
