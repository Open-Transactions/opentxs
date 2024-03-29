// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace database
{
class Cfilter;
class Header;
class Peer;
}  // namespace database

namespace node
{
namespace internal
{
class HeaderOracle;
struct Config;
}  // namespace internal

class FilterOracle;
class HeaderOracle;
class Manager;
struct Endpoints;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BlockchainFilterOracle(
    const api::Session& api,
    const blockchain::node::HeaderOracle& header,
    const blockchain::node::Endpoints& endpoints,
    const blockchain::node::internal::Config& config,
    blockchain::database::Cfilter& db,
    blockchain::Type chain,
    blockchain::cfilter::Type filter) noexcept
    -> std::unique_ptr<blockchain::node::FilterOracle>;
auto BlockchainNetworkBitcoin(
    const api::session::Client& api,
    const blockchain::Type type,
    const blockchain::node::internal::Config& config,
    std::string_view seednode) noexcept
    -> std::shared_ptr<blockchain::node::Manager>;
auto BlockchainPeerManager(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const blockchain::node::Manager> node,
    blockchain::database::Peer& db,
    std::string_view peers) noexcept -> void;
auto HeaderOracle(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::node::Endpoints& endpoints,
    blockchain::database::Header& database) noexcept
    -> blockchain::node::internal::HeaderOracle;
}  // namespace opentxs::factory
