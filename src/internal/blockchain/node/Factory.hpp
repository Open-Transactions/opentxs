// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace network
{
class Blockchain;
}  // namespace network

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

namespace node
{
namespace internal
{
struct BlockDatabase;
struct BlockOracle;
struct Config;
struct FilterDatabase;
struct FilterOracle;
struct HeaderDatabase;
struct Mempool;
struct Network;
struct PeerDatabase;
struct PeerManager;
struct Wallet;
struct WalletDatabase;
}  // namespace internal

class HeaderOracle;
}  // namespace node

namespace internal
{
struct Database;
}  // namespace internal
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BlockchainDatabase(
    const api::Session& api,
    const blockchain::node::internal::Network& node,
    const blockchain::database::common::Database& db,
    const blockchain::Type chain,
    const blockchain::cfilter::Type filter) noexcept
    -> std::unique_ptr<blockchain::internal::Database>;
auto BlockchainFilterOracle(
    const api::Session& api,
    const blockchain::node::internal::Config& config,
    const blockchain::node::internal::Network& node,
    const blockchain::node::HeaderOracle& header,
    const blockchain::node::internal::BlockOracle& block,
    blockchain::node::internal::FilterDatabase& database,
    const blockchain::Type chain,
    const blockchain::cfilter::Type filter,
    const UnallocatedCString& shutdown) noexcept
    -> std::unique_ptr<blockchain::node::internal::FilterOracle>;
auto BlockchainNetworkBitcoin(
    const api::Session& api,
    const blockchain::Type type,
    const blockchain::node::internal::Config& config,
    const UnallocatedCString& seednode,
    const UnallocatedCString& syncEndpoint) noexcept
    -> std::unique_ptr<blockchain::node::internal::Network>;
auto BlockchainPeerManager(
    const api::Session& api,
    const blockchain::node::internal::Config& config,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::node::internal::Network& node,
    const blockchain::node::HeaderOracle& headers,
    const blockchain::node::internal::FilterOracle& filter,
    const blockchain::node::internal::BlockOracle& block,
    blockchain::node::internal::PeerDatabase& database,
    const blockchain::Type type,
    const blockchain::database::BlockStorage policy,
    const UnallocatedCString& seednode,
    const UnallocatedCString& shutdown) noexcept
    -> std::unique_ptr<blockchain::node::internal::PeerManager>;
auto BlockchainWallet(
    const api::Session& api,
    const blockchain::node::internal::Network& parent,
    blockchain::node::internal::WalletDatabase& db,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::Type chain,
    const std::string_view shutdown)
    -> std::unique_ptr<blockchain::node::internal::Wallet>;
auto BlockOracle(
    const api::Session& api,
    const blockchain::node::internal::Network& node,
    const blockchain::node::HeaderOracle& header,
    blockchain::node::internal::BlockDatabase& db,
    const blockchain::Type chain,
    const UnallocatedCString& shutdown) noexcept
    -> std::unique_ptr<blockchain::node::internal::BlockOracle>;
auto HeaderOracle(
    const api::Session& api,
    blockchain::node::internal::HeaderDatabase& database,
    const blockchain::Type type) noexcept
    -> std::unique_ptr<blockchain::node::HeaderOracle>;
}  // namespace opentxs::factory
