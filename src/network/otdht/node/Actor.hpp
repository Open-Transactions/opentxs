// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <exception>
#include <memory>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/network/otdht/Node.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Timer.hpp"
#include "network/otdht/node/Shared.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class Node::Actor final : public opentxs::Actor<Node::Actor, NodeJob>
{
public:
    auto Init(boost::shared_ptr<Actor> self) noexcept -> void
    {
        signal_startup(self);
    }

    Actor(
        std::shared_ptr<const api::Session> api,
        boost::shared_ptr<Shared> shared,
        zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<Node::Actor, NodeJob>;

    using PeerData = std::tuple<CString, CString, zeromq::socket::Raw>;
    using Peers = Map<CString, PeerData>;

    std::shared_ptr<const api::Session> api_p_;
    boost::shared_ptr<Shared> shared_p_;
    const api::Session& api_;
    Shared::Guarded& data_;
    zeromq::socket::Raw& publish_;
    zeromq::socket::Raw& router_;
    Peers peers_;
    Peers listeners_;
    Set<CString> listen_endpoints_;

    auto get_peers() const noexcept -> Set<CString>;
    auto get_peers(Message& out) const noexcept -> void;

    auto do_shutdown() noexcept -> void;
    auto do_startup() noexcept -> bool;
    auto load_peers() noexcept -> void;
    auto load_positions() noexcept -> void;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto pipeline_other(const Work work, Message&& msg) noexcept -> void;
    auto pipeline_router(const Work work, Message&& msg) noexcept -> void;
    auto process_add_listener(Message&& msg) noexcept -> void;
    auto process_cfilter(
        opentxs::blockchain::Type chain,
        opentxs::blockchain::block::Position&& tip) noexcept -> void;
    auto process_chain_state(Message&& msg) noexcept -> void;
    auto process_new_cfilter(Message&& msg) noexcept -> void;
    auto process_new_peer(Message&& msg) noexcept -> void;
    auto process_registration(Message&& msg) noexcept -> void;
    auto process_peer(std::string_view endpoint) noexcept -> void;
    auto publish_peers() noexcept -> void;
    auto send_to_peers(Message&& msg) noexcept -> void;
    auto work() noexcept -> bool;
};
}  // namespace opentxs::network::otdht
