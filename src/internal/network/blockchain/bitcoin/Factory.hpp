// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <optional>
#include <string_view>

#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/network/blockchain/Peer.hpp"
#include "opentxs/network/asio/Socket.hpp"

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

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BlockchainPeerBitcoin(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> network,
    blockchain::p2p::bitcoin::Nonce nonce,
    int peerID,
    blockchain::p2p::Address address,
    std::string_view fromParent,
    std::optional<network::asio::Socket> socket) -> void;
}  // namespace opentxs::factory
