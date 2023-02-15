// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/Factory.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
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
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr
    using blockchain::node::peermanager::Actor;
    auto actor = boost::allocate_shared<Actor>(
        alloc::PMR<Actor>{alloc}, api, node, db, peers, batchID);

    OT_ASSERT(actor);

    actor->Init(actor);
}
}  // namespace opentxs::factory
