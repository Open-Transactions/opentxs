// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/Factory.hpp"  // IWYU pragma: associated

#include <memory>
#include <string_view>

#include "blockchain/node/peermanager/PeerManager.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::factory
{
auto BlockchainPeerManager(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const blockchain::node::Manager> node,
    blockchain::database::Peer& db,
    std::string_view peers) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(node);

    const auto& zmq = api->Network().ZeroMQ().Internal();
    const auto batchID = zmq.PreallocateBatch();
    auto* alloc = zmq.Alloc(batchID);
    using blockchain::node::peermanager::Actor;
    auto actor = std::allocate_shared<Actor>(
        alloc::PMR<Actor>{alloc}, api, node, db, peers, batchID);

    OT_ASSERT(actor);

    actor->Init(actor);
}
}  // namespace opentxs::factory
