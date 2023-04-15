// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/circular_buffer.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_shared_guarded.h>
#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <random>
#include <shared_mutex>
#include <span>
#include <utility>

#include "internal/network/blockchain/OTDHT.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Actor.hpp"

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
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Frame;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain
{
class OTDHT::Actor : public opentxs::Actor<OTDHT::Actor, DHTJob>
{
public:
    static auto Factory(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const opentxs::blockchain::node::Manager> node,
        network::zeromq::BatchID batchID) noexcept -> void;

    auto Init(boost::shared_ptr<Actor> self) noexcept -> void
    {
        signal_startup(self);
    }

    Actor(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const opentxs::blockchain::node::Manager> node,
        network::zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() override;

protected:
    using PeerID = CString;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const opentxs::blockchain::node::Manager> node_p_;
    const api::Session& api_;
    const opentxs::blockchain::node::Manager& node_;
    const opentxs::blockchain::Type chain_;
    const opentxs::blockchain::cfilter::Type filter_type_;

    auto have_outstanding_request() const noexcept -> bool;
    virtual auto local_position() const noexcept
        -> opentxs::blockchain::block::Position
    {
        return oracle_position();
    }
    auto oracle_position() const noexcept
        -> opentxs::blockchain::block::Position
    {
        return *oracle_position_.lock_shared();
    }

    virtual auto do_work() noexcept -> bool { return false; }
    auto finish_request(PeerID peer) noexcept -> void;
    auto get_peer(const Message& message) noexcept -> PeerID;
    auto send_request(const opentxs::blockchain::block::Position& best) noexcept
        -> void;
    auto send_to_listeners(Message msg) noexcept -> void;
    auto to_api() noexcept -> zeromq::socket::Raw& { return to_api_; }
    auto to_blockchain() noexcept -> zeromq::socket::Raw&
    {
        return to_blockchain_;
    }
    auto to_dht() noexcept -> zeromq::socket::Raw& { return to_dht_; }
    auto update_oracle_position(
        opentxs::blockchain::block::Position position) noexcept -> void;
    auto update_peer_position(
        const PeerID& peer,
        opentxs::blockchain::block::Position position) noexcept -> void;
    auto update_position(
        const opentxs::blockchain::block::Position& incoming,
        opentxs::blockchain::block::Position& existing) noexcept -> void;

private:
    friend opentxs::Actor<OTDHT::Actor, DHTJob>;

    using Weight = std::ptrdiff_t;
    using Samples = boost::circular_buffer<Weight, alloc::PMR<Weight>>;
    using ScoreInterval = std::chrono::milliseconds;

    struct PeerData final : public opentxs::Allocated {
        enum class Type : std::uint8_t {
            outgoing = network::otdht::outgoing_peer_,
            incoming = network::otdht::incoming_peer_,
        };

        const Type type_;
        opentxs::blockchain::block::Position position_;
        Samples samples_;
        Weight weight_;

        auto get_allocator() const noexcept -> allocator_type final;

        PeerData(Type type, allocator_type alloc) noexcept;
        PeerData() = delete;
        PeerData(const PeerData&) = delete;
        PeerData(PeerData&&) = delete;
        auto operator=(const PeerData&) -> PeerData& = delete;
        auto operator=(PeerData&& rhs) -> PeerData& = delete;

        ~PeerData() final = default;
    };

    using Peers = Map<PeerID, PeerData>;
    using GuardedPosition = libguarded::
        shared_guarded<opentxs::blockchain::block::Position, std::shared_mutex>;

    static constexpr auto max_samples_ = 8_uz;
    static constexpr auto min_weight_ = 1_z;
    static constexpr auto request_timeout_ = 15s;
    static constexpr auto start_weight_ = Weight{
        std::chrono::duration_cast<ScoreInterval>(request_timeout_).count()};

    zeromq::socket::Raw& to_dht_;
    zeromq::socket::Raw& to_blockchain_;
    zeromq::socket::Raw& to_api_;
    Set<PeerID> known_peers_;
    Peers peers_;
    std::mt19937_64 rand_;
    bool registered_with_node_;
    GuardedPosition oracle_position_;
    std::optional<std::pair<sTime, PeerID>> last_request_;
    Map<CString, PeerID> peer_index_;
    Timer registration_timer_;
    Timer request_timer_;

    static auto add_contribution(
        Samples& samples,
        Weight& weight,
        Weight value) noexcept -> void;
    static auto calculate_weight(const Samples& samples) noexcept -> Weight;
    static auto make_envelope(const PeerID& peer) noexcept -> Message;

    auto filter_peers(const opentxs::blockchain::block::Position& target)
        const noexcept -> Vector<PeerID>;
    auto get_peers() const noexcept -> Set<PeerID>;
    auto get_peers(std::span<const zeromq::Frame> body, std::ptrdiff_t offset)
        const noexcept -> Set<PeerID>;

    auto add_peers(Set<PeerID>&& peers) noexcept -> void;
    auto check_registration() noexcept -> bool;
    auto check_peers() noexcept -> bool;
    auto check_request_timer() noexcept -> void;
    auto choose_peer(
        const opentxs::blockchain::block::Position& target) noexcept
        -> std::optional<PeerID>;
    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto finish_request(bool success) noexcept -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto pipeline_other(const Work work, Message&& msg) noexcept -> void;
    auto pipeline_router(
        const Work work,
        Message&& msg,
        allocator_type monotonic) noexcept -> void;
    auto process_cfilter(Message&& msg) noexcept -> void;
    virtual auto process_checksum_failure(Message&& msg) noexcept -> void;
    virtual auto process_job_processed(Message&& msg) noexcept -> void;
    auto process_peer_list(Message&& msg) noexcept -> void;
    auto process_pushtx_external(
        Message&& msg,
        allocator_type monotonic) noexcept -> void;
    auto process_pushtx_internal(Message&& msg) noexcept -> void;
    auto process_registration_node(Message&& msg) noexcept -> void;
    auto process_registration_peer(Message&& msg) noexcept -> void;
    virtual auto process_report(Message&& msg) noexcept -> void {}
    auto process_response_peer(Message&& msg) noexcept -> void;
    virtual auto process_sync_peer(Message&& msg) noexcept -> void {}
    virtual auto process_sync_request(Message&& msg) noexcept -> void {}
    auto remove_peers(Set<PeerID>&& peers) noexcept -> void;
    auto reset_registration_timer() noexcept -> void;
    auto reset_request_timer() noexcept -> void;
    auto send_registration() noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::network::blockchain
