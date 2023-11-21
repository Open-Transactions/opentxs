// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <optional>
#include <string_view>

#include "internal/network/blockchain/Peer.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BlockchainPeerBitcoin(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> network,
    network::blockchain::bitcoin::message::Nonce nonce,
    int peerID,
    network::blockchain::Address address,
    const Set<network::blockchain::Address>& gossip,
    std::string_view fromParent,
    std::optional<network::asio::Socket> socket) -> void;
}  // namespace opentxs::factory
