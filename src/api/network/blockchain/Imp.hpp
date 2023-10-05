// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/Stats.hpp"

#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string_view>

#include "api/network/blockchain/Base.hpp"
#include "api/network/blockchain/Blockchain.hpp"
#include "api/network/blockchain/StartupPublisher.hpp"
#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Client;
class Endpoints;
}  // namespace session

class Legacy;
}  // namespace api

namespace blockchain
{
namespace node
{
namespace stats
{
class Shared;
}  // namespace stats

class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::network::implementation
{
struct BlockchainImp final : public Blockchain::Imp {
    using Imp::AddSyncServer;
    auto AddSyncServer(const std::string_view endpoint) const noexcept
        -> bool final;
    auto Database() const noexcept
        -> const opentxs::blockchain::database::common::Database& final
    {
        return db_;
    }
    auto DeleteSyncServer(const std::string_view endpoint) const noexcept
        -> bool final;
    auto Disable(const Imp::Chain type) const noexcept -> bool final;
    auto Enable(const Imp::Chain type, const std::string_view seednode)
        const noexcept -> bool final;
    auto EnabledChains(alloc::Default) const noexcept -> Set<Imp::Chain> final;
    auto IsEnabled(const Chain chain) const noexcept -> bool final;
    auto GetChain(const Imp::Chain type) const noexcept(false)
        -> BlockchainHandle final;
    auto GetSyncServers(alloc::Default alloc) const noexcept
        -> Imp::Endpoints final;
    auto Profile() const noexcept -> BlockchainProfile final;
    auto RestoreNetworks() const noexcept -> void final;
    auto Start(const Imp::Chain type, const std::string_view seednode)
        const noexcept -> bool final;
    auto Stats() const noexcept -> opentxs::blockchain::node::Stats final;
    auto Stop(const Imp::Chain type) const noexcept -> bool final;

    auto Init(
        std::shared_ptr<const api::session::Client> api,
        const api::crypto::Blockchain& crypto,
        const api::Legacy& legacy,
        const std::filesystem::path& dataFolder,
        const Options& args) noexcept -> void final;
    auto Shutdown() noexcept -> void final;

    BlockchainImp(
        const api::session::Client& api,
        const api::session::Endpoints& endpoints,
        const opentxs::network::zeromq::Context& zmq) noexcept;
    BlockchainImp() = delete;
    BlockchainImp(const BlockchainImp&) = delete;
    BlockchainImp(BlockchainImp&&) = delete;
    auto operator=(const BlockchainImp&) -> BlockchainImp& = delete;
    auto operator=(BlockchainImp&&) -> BlockchainImp& = delete;

    ~BlockchainImp() final;

private:
    using Config = opentxs::blockchain::node::internal::Config;
    using pNode = std::shared_ptr<opentxs::blockchain::node::Manager>;
    using Chains = UnallocatedVector<Chain>;
    using DB = opentxs::blockchain::database::common::Database;

    const api::session::Client& api_;
    const api::crypto::Blockchain* crypto_;
    OTZMQPublishSocket chain_state_publisher_;
    blockchain::StartupPublisher startup_publisher_;
    DeferredConstruction<Config> base_config_;
    DeferredConstruction<DB> db_;
    mutable std::mutex lock_;
    mutable UnallocatedMap<Chain, Config> config_;
    mutable UnallocatedMap<Chain, pNode> networks_;
    std::shared_ptr<opentxs::blockchain::node::stats::Shared> stats_;
    std::atomic_bool running_;

    auto disable(const Lock& lock, const Chain type) const noexcept -> bool;
    auto enable(
        const Lock& lock,
        const Chain type,
        const std::string_view seednode) const noexcept -> bool;
    auto publish_chain_state(Chain type, bool state) const -> void;
    auto start(
        const Lock& lock,
        const Chain type,
        const std::string_view seednode,
        const bool startWallet = true) const noexcept -> bool;
    auto stop(const Lock& lock, const Chain type) const noexcept -> bool;
};
}  // namespace opentxs::api::network::implementation
