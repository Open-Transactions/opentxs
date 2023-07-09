// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

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
class Session;
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
}  // namespace blockchain

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::internal
{
class Blockchain
{
public:
    using Endpoints = Vector<CString>;

    virtual auto AddSyncServer(const std::string_view endpoint) const noexcept
        -> bool = 0;
    virtual auto Database() const noexcept
        -> const opentxs::blockchain::database::common::Database& = 0;
    virtual auto DeleteSyncServer(
        const std::string_view endpoint) const noexcept -> bool = 0;
    virtual auto GetSyncServers(alloc::Strategy alloc = {}) const noexcept
        -> Endpoints = 0;
    virtual auto IsEnabled(const opentxs::blockchain::Type chain) const noexcept
        -> bool = 0;
    virtual auto RestoreNetworks() const noexcept -> void = 0;

    virtual auto Init(
        std::shared_ptr<const api::Session> api,
        const api::crypto::Blockchain& crypto,
        const api::Legacy& legacy,
        const std::filesystem::path& dataFolder,
        const Options& args) noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    auto operator=(const Blockchain&) -> Blockchain& = delete;
    auto operator=(Blockchain&&) -> Blockchain& = delete;

    virtual ~Blockchain() = default;

protected:
    Blockchain() = default;
};
}  // namespace opentxs::api::network::internal
