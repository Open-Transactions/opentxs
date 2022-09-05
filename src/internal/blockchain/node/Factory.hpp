// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
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
class Cfilter;
class Header;
class Peer;
class Wallet;
}  // namespace database

namespace node
{
namespace internal
{
class BlockOracle;
class HeaderOracle;
class Mempool;
class PeerManager;
struct Config;
}  // namespace internal

class BlockOracle;
class FilterOracle;
class Manager;
class Wallet;
struct Endpoints;
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BlockchainFilterOracle(
    const api::Session& api,
    const blockchain::node::Manager& node,
    const blockchain::cfilter::Type filter) noexcept
    -> std::unique_ptr<blockchain::node::FilterOracle>;
auto BlockchainNetworkBitcoin(
    const api::Session& api,
    const blockchain::Type type,
    const blockchain::node::internal::Config& config,
    std::string_view seednode) noexcept
    -> std::shared_ptr<blockchain::node::Manager>;
auto BlockchainPeerManager(
    const api::Session& api,
    const blockchain::node::internal::Config& config,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::node::Manager& node,
    const blockchain::node::HeaderOracle& headers,
    const blockchain::node::FilterOracle& filter,
    const blockchain::node::BlockOracle& block,
    blockchain::database::Peer& database,
    const blockchain::Type type,
    std::string_view seednode,
    const blockchain::node::Endpoints& endpoints) noexcept
    -> std::unique_ptr<blockchain::node::internal::PeerManager>;
auto HeaderOracle(
    const api::Session& api,
    const blockchain::node::Manager& node) noexcept
    -> blockchain::node::internal::HeaderOracle;
}  // namespace opentxs::factory
