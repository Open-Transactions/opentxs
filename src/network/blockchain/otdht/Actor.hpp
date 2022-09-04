// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"

#pragma once

#include <boost/circular_buffer.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <chrono>
#include <exception>
#include <iosfwd>
#include <memory>
#include <optional>
#include <random>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/network/blockchain/OTDHT.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
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
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace otdht
{
class Acknowledgement;
class Data;
class State;
}  // namespace otdht

namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class FrameSection;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain
{
class OTDHT::Actor final : public opentxs::Actor<OTDHT::Actor, DHTJob>
{
public:
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

    ~Actor() final;

private:
    friend opentxs::Actor<OTDHT::Actor, DHTJob>;

    enum class Mode : int { disabled, client, server };
    using PeerID = CString;
    using Weight = std::ptrdiff_t;
    using Samples = boost::circular_buffer<Weight, alloc::PMR<Weight>>;

    struct PeerData final : public opentxs::Allocated {
        opentxs::blockchain::block::Position position_;
        Samples samples_;
        Weight weight_;

        auto get_allocator() const noexcept -> allocator_type final;

        PeerData(allocator_type alloc) noexcept;
        PeerData() = delete;
        PeerData(const PeerData&) = delete;
        PeerData(PeerData&&) = delete;
        auto operator=(const PeerData&) -> PeerData& = delete;
        auto operator=(PeerData&& rhs) -> PeerData& = delete;

        ~PeerData() final = default;
    };

    using Peers = Map<PeerID, PeerData>;
    using Pending = std::pair<Message, opentxs::blockchain::block::Position>;
    using ScoreInterval = std::chrono::milliseconds;

    static constexpr auto max_samples_ = 8_uz;
    static constexpr auto min_weight_ = 1_z;
    static constexpr auto queue_limit_ = 5_uz;
    static constexpr auto request_timeout_ = 15s;
    static constexpr auto start_weight_ = Weight{
        std::chrono::duration_cast<ScoreInterval>(request_timeout_).count()};

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const opentxs::blockchain::node::Manager> node_p_;
    const api::Session& api_;
    const opentxs::blockchain::node::Manager& node_;
    zeromq::socket::Raw& router_;
    zeromq::socket::Raw& to_node_;
    const opentxs::blockchain::Type chain_;
    const opentxs::blockchain::cfilter::Type filter_type_;
    const Mode mode_;
    Set<PeerID> known_peers_;
    Peers peers_;
    opentxs::blockchain::block::Position local_position_;
    opentxs::blockchain::block::Position best_remote_position_;
    opentxs::blockchain::block::Position processing_position_;
    bool processing_;
    std::optional<std::pair<sTime, PeerID>> last_request_;
    Timer registration_timer_;
    Timer request_timer_;
    std::mt19937_64 rand_;
    bool registered_with_node_;
    Deque<Pending> queue_;

    static auto add_contribution(
        Samples& samples,
        Weight& weight,
        Weight value) noexcept -> void;
    static auto calculate_weight(const Samples& samples) noexcept -> Weight;

    auto best_position() const noexcept
        -> const opentxs::blockchain::block::Position&;
    auto can_connect(const otdht::Data& data) const noexcept -> bool;
    auto filter_peers(const opentxs::blockchain::block::Position& target)
        const noexcept -> Vector<PeerID>;
    auto get_peer(const Message& msg) const noexcept -> ReadView;
    auto get_peers() const noexcept -> Set<PeerID>;
    auto get_peers(const zeromq::FrameSection& body, std::ptrdiff_t offset)
        const noexcept -> Set<PeerID>;

    auto add_peers(Set<PeerID>&& peers) noexcept -> void;
    auto check_registration() noexcept -> bool;
    auto check_peers() noexcept -> bool;
    auto check_request_timer() noexcept -> void;
    auto choose_peer(
        const opentxs::blockchain::block::Position& target) noexcept
        -> std::optional<PeerID>;
    auto do_shutdown() noexcept -> void;
    auto do_startup() noexcept -> bool;
    auto drain_queue() noexcept -> void;
    auto fill_queue() noexcept -> void;
    auto finish_request(bool success) noexcept -> void;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto pipeline_other(const Work work, Message&& msg) noexcept -> void;
    auto pipeline_router(const Work work, Message&& msg) noexcept -> void;
    auto process_ack(
        const Message& msg,
        const otdht::Acknowledgement& ack) noexcept -> void;
    auto process_cfilter(Message&& msg) noexcept -> void;
    auto process_data(Message&& msg, const otdht::Data& data) noexcept -> void;
    auto process_job_processed(Message&& msg) noexcept -> void;
    auto process_peer_list(Message&& msg) noexcept -> void;
    auto process_pushtx_internal(Message&& msg) noexcept -> void;
    auto process_registration_node(Message&& msg) noexcept -> void;
    auto process_registration_peer(Message&& msg) noexcept -> void;
    auto process_response_peer(Message&& msg) noexcept -> void;
    auto process_state(const Message& msg, const otdht::State& state) noexcept
        -> bool;
    auto process_sync_peer(Message&& msg) noexcept -> void;
    auto remove_peers(Set<PeerID>&& peers) noexcept -> void;
    auto reset_registration_timer() noexcept -> void;
    auto reset_request_timer() noexcept -> void;
    auto send_registration() noexcept -> void;
    auto send_request(const opentxs::blockchain::block::Position& best) noexcept
        -> void;
    auto update_position(
        const opentxs::blockchain::block::Position& incoming,
        opentxs::blockchain::block::Position& existing) noexcept -> void;
    auto update_remote_position(
        const opentxs::blockchain::block::Position& incoming) noexcept -> void;
    auto work() noexcept -> bool;
};
}  // namespace opentxs::network::blockchain
