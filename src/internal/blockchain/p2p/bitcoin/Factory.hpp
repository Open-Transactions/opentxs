// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace client
{
namespace internal
{
class BlockOracle;
struct Config;
struct FilterOracle;
class HeaderOracle;
struct Network;
struct PeerManager;
}  // namespace internal
}  // namespace client

namespace node
{
namespace internal
{
class BlockOracle;
struct Config;
struct FilterOracle;
class HeaderOracle;
struct Mempool;
struct Network;
struct PeerManager;
}  // namespace internal
}  // namespace node

namespace p2p
{
namespace bitcoin
{
class Header;
struct Message;
}  // namespace bitcoin

namespace internal
{
struct Peer;
}  // namespace internal
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BitcoinP2PHeader(
    const api::Session& api,
    const blockchain::Type& chain,
    const network::zeromq::Frame& bytes) -> blockchain::p2p::bitcoin::Header*;
auto BitcoinP2PMessage(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload = nullptr,
    const std::size_t size = 0) -> blockchain::p2p::bitcoin::Message*;
auto BitcoinP2PPeerLegacy(
    const api::Session& api,
    const blockchain::node::internal::Config& config,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::node::internal::Network& network,
    const blockchain::node::HeaderOracle& header,
    const blockchain::node::internal::FilterOracle& filter,
    const blockchain::node::internal::BlockOracle& block,
    const blockchain::node::internal::PeerManager& manager,
    blockchain::node::internal::PeerDatabase& db,
    const blockchain::database::BlockStorage policy,
    const int id,
    std::unique_ptr<blockchain::p2p::internal::Address> address,
    const UnallocatedCString& shutdown,
    const blockchain::p2p::bitcoin::ProtocolVersion p2p_protocol_version)
    -> std::unique_ptr<blockchain::p2p::internal::Peer>;
}  // namespace opentxs::factory
