// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string_view>

#include "api/network/blockchain/Blockchain.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Stats.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Legacy;
}  // namespace api

namespace blockchain
{
namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database

namespace node
{
class Manager;
class Stats;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::implementation
{
struct Blockchain::Imp : virtual public internal::Blockchain {
    using Chain = network::Blockchain::Chain;

    auto AddSyncServer([[maybe_unused]] const std::string_view endpoint)
        const noexcept -> bool override
    {
        return {};
    }
    auto Database() const noexcept
        -> const opentxs::blockchain::database::common::Database& override
    {
        OT_FAIL;
    }
    auto DeleteSyncServer([[maybe_unused]] const std::string_view endpoint)
        const noexcept -> bool override
    {
        return {};
    }
    virtual auto Disable([[maybe_unused]] const Chain type) const noexcept
        -> bool
    {
        return {};
    }
    virtual auto Enable(
        [[maybe_unused]] const Chain type,
        [[maybe_unused]] const std::string_view seednode) const noexcept -> bool
    {
        return {};
    }
    virtual auto EnabledChains(alloc::Default) const noexcept -> Set<Chain>
    {
        return {};
    }
    /// throws std::out_of_range if chain has not been started
    virtual auto GetChain([[maybe_unused]] const Chain type) const
        noexcept(false) -> BlockchainHandle
    {
        throw std::out_of_range("no blockchain support");
    }
    auto GetSyncServers(alloc::Default) const noexcept -> Endpoints override
    {
        return {};
    }
    auto IsEnabled([[maybe_unused]] const Chain chain) const noexcept
        -> bool override
    {
        return {};
    }
    virtual auto Profile() const noexcept -> BlockchainProfile { return {}; }
    auto RestoreNetworks() const noexcept -> void override {}
    virtual auto Start(
        [[maybe_unused]] const Chain type,
        [[maybe_unused]] const std::string_view seednode) const noexcept -> bool
    {
        return {};
    }
    virtual auto Stats() const noexcept -> opentxs::blockchain::node::Stats
    {
        return {};
    }
    virtual auto Stop([[maybe_unused]] const Chain type) const noexcept -> bool
    {
        return {};
    }

    auto Init(
        [[maybe_unused]] std::shared_ptr<const api::session::Client> api,
        [[maybe_unused]] const api::crypto::Blockchain& crypto,
        [[maybe_unused]] const api::Legacy& legacy,
        [[maybe_unused]] const std::filesystem::path& dataFolder,
        [[maybe_unused]] const Options& args) noexcept -> void override
    {
    }
    auto Shutdown() noexcept -> void override {}

    Imp() = default;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() override = default;
};
}  // namespace opentxs::api::network::implementation
