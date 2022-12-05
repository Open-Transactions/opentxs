// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
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
class HeaderOracle;
class Manager;
class Wallet;
struct Endpoints;
}  // namespace node
}  // namespace blockchain
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
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const blockchain::node::Manager> node,
    blockchain::database::Peer& db,
    std::string_view peers) noexcept -> void;
auto HeaderOracle(
    const api::Session& api,
    const blockchain::node::Manager& node) noexcept
    -> blockchain::node::internal::HeaderOracle;
}  // namespace opentxs::factory
