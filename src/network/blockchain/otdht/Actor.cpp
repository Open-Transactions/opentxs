// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "network/blockchain/otdht/Actor.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/otdht/Acknowledgement.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/MessageType.hpp"
#include "opentxs/network/otdht/PushTransactionReply.hpp"
#include "opentxs/network/otdht/Request.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::network::blockchain
{
OTDHT::Actor::PeerData::PeerData(allocator_type alloc) noexcept
    : position_()
    , samples_(max_samples_, start_weight_, alloc)
    , weight_(calculate_weight(samples_))
{
}

auto OTDHT::Actor::PeerData::get_allocator() const noexcept -> allocator_type
{
    return samples_.get_allocator();
}
}  // namespace opentxs::network::blockchain

namespace opentxs::network::blockchain
{
OTDHT::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> node,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : opentxs::Actor<OTDHT::Actor, DHTJob>(
          *api,
          LogTrace(),
          [&] {
              return CString{print(node->Internal().Chain()), alloc}.append(
                  " OTDHT node");
          }(),
          0ms,
          batchID,
          alloc,
          [&] {
              using Dir = network::zeromq::socket::Direction;
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  CString{api->Endpoints().Shutdown(), alloc}, Dir::Connect);
              sub.emplace_back(
                  CString{
                      node->Internal().Endpoints().shutdown_publish_, alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{
                      node->Internal().Endpoints().new_filter_publish_, alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{
                      api->Endpoints().Internal().OTDHTNodePublish(), alloc},
                  Dir::Connect);

              return sub;
          }(),
          [&] {
              using Dir = zeromq::socket::Direction;
              auto pull = zeromq::EndpointArgs{alloc};
              pull.emplace_back(
                  CString{node->Internal().Endpoints().otdht_pull_, alloc},
                  Dir::Bind);

              return pull;
          }(),
          [&] {
              using Dir = zeromq::socket::Direction;
              auto dealer = zeromq::EndpointArgs{alloc};
              dealer.emplace_back(
                  CString{api->Endpoints().Internal().OTDHTNodeRouter(), alloc},
                  Dir::Connect);

              return dealer;
          }(),
          [&] {
              auto out = Vector<zeromq::SocketData>{alloc};
              using Socket = zeromq::socket::Type;
              using Args = zeromq::EndpointArgs;
              using Dir = zeromq::socket::Direction;
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Router,
                  {
                      {CString{
                           api->Endpoints().Internal().OTDHTBlockchain(
                               node->Internal().Chain()),
                           alloc},
                       Dir::Bind},
                  }));
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Push,
                  {
                      {CString{
                           node->Internal().Endpoints().manager_pull_, alloc},
                       Dir::Connect},
                  }));

              return out;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , api_(*api_p_)
    , node_(*node_p_)
    , router_(pipeline_.Internal().ExtraSocket(0_uz))
    , to_node_(pipeline_.Internal().ExtraSocket(1_uz))
    , chain_(node_.Internal().Chain())
    , filter_type_(node_.FilterOracle().DefaultType())
    , mode_([&] {
        switch (node_.Internal().GetConfig().profile_) {
            case BlockchainProfile::mobile:
            case BlockchainProfile::desktop: {

                return Mode::client;
            }
            case BlockchainProfile::desktop_native: {

                return Mode::disabled;
            }
            case BlockchainProfile::server: {

                return Mode::server;
            }
            default: {
                LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid profile type")
                    .Abort();
            }
        }
    }())
    , known_peers_(alloc)
    , peers_(alloc)
    , local_position_()
    , best_remote_position_()
    , processing_position_()
    , processing_(false)
    , last_request_(std::nullopt)
    , registration_timer_(api_.Network().Asio().Internal().GetTimer())
    , request_timer_(api_.Network().Asio().Internal().GetTimer())
    , rand_(std::random_device{}())
    , registered_with_node_(false)
    , queue_(alloc)
{
}

auto OTDHT::Actor::add_contribution(
    Samples& samples,
    Weight& weight,
    Weight value) noexcept -> void
{
    samples.push_back(value);
    weight = calculate_weight(samples);
}

auto OTDHT::Actor::add_peers(Set<PeerID>&& peers) noexcept -> void
{
    const auto& log = log_;

    for (const auto& id : peers) {
        const auto [endpoint, added] = known_peers_.emplace(id);

        if (added) {
            log(OT_PRETTY_CLASS())(name_)(": discovered new peer ")(*endpoint)
                .Flush();
        }
    }
}

auto OTDHT::Actor::best_position() const noexcept
    -> const opentxs::blockchain::block::Position&
{
    if (queue_.empty()) {
        if (processing_) {

            return processing_position_;
        } else {

            return local_position_;
        }
    } else {

        return queue_.back().second;
    }
}

auto OTDHT::Actor::calculate_weight(const Samples& samples) noexcept -> Weight
{
    const auto average =
        std::reduce(samples.begin(), samples.end(), 0_z) / samples.size();

    return std::max<Weight>(min_weight_, average);
}

auto OTDHT::Actor::can_connect(const otdht::Data& data) const noexcept -> bool
{
    const auto& log = log_;
    const auto& best = best_position();
    const auto max = best.height_ + 1;
    const auto start = data.FirstPosition(api_);

    if (start.height_ > max) {
        log(OT_PRETTY_CLASS())(name_)(": incoming data starts at height ")(
            start.height_)(" however local data stops at height ")(best.height_)
            .Flush();

        return false;
    }

    return true;
}

auto OTDHT::Actor::check_registration() noexcept -> bool
{
    if (registered_with_node_) {

        return false;
    } else {
        send_registration();

        return true;
    }
}

auto OTDHT::Actor::check_peers() noexcept -> bool
{
    return get_peers() != known_peers_;
}

auto OTDHT::Actor::check_request_timer() noexcept -> void
{
    if (false == last_request_.has_value()) {
        log_(OT_PRETTY_CLASS())(name_)(": no outstanding request").Flush();

        return;
    }

    const auto& [time, peer] = *last_request_;

    if ((sClock::now() - time) > request_timeout_) {
        log_(OT_PRETTY_CLASS())(name_)(": request timeout").Flush();
        finish_request(false);
    }
}

auto OTDHT::Actor::choose_peer(
    const opentxs::blockchain::block::Position& target) noexcept
    -> std::optional<PeerID>
{
    const auto peers = filter_peers(target);
    const auto count = peers.size();

    if (peers.empty()) { return std::nullopt; }

    try {
        // NOTE peer selection is performed via a weighted random sample where
        // each peer's weight increases as it fulfills requests promptly and
        // decreases if it times out
        auto range = Weight{0};
        auto map = Map<Weight, PeerID>{get_allocator()};

        for (const auto& id : peers) {
            const auto& [position, samples, weight] = peers_.at(id);
            range += weight;
            map.emplace(range, id);
        }

        OT_ASSERT(map.size() == count);

        auto dist = std::uniform_int_distribution<Weight>{0_z, range - 1_z};

        const auto i = map.upper_bound(dist(rand_));

        OT_ASSERT(map.end() != i);

        return i->second;
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Abort();
    }
}

auto OTDHT::Actor::do_shutdown() noexcept -> void
{
    request_timer_.Cancel();
    registration_timer_.Cancel();
    node_p_.reset();
    api_p_.reset();
}

auto OTDHT::Actor::do_startup() noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (node_.Internal().ShuttingDown())) {

        return true;
    }

    local_position_ = node_.FilterOracle().FilterTip(filter_type_);
    do_work();

    return false;
}

auto OTDHT::Actor::drain_queue() noexcept -> void
{
    const auto& log = log_;

    if (processing_) {
        log(OT_PRETTY_CLASS())(name_)(": still processing last message")
            .Flush();
    } else if (queue_.empty()) {
        log(OT_PRETTY_CLASS())(name_)(": no queued messages to process")
            .Flush();
    } else {
        auto post = ScopeGuard{[&] { queue_.pop_front(); }};
        auto& [msg, position] = queue_.front();
        processing_ = true;
        processing_position_ = std::move(position);
        to_node_.SendDeferred(std::move(msg), __FILE__, __LINE__);
    }
}

auto OTDHT::Actor::fill_queue() noexcept -> void
{
    const auto& log = log_;
    const auto& target = best_remote_position_;
    const auto& local = local_position_;

    if (last_request_.has_value()) {
        log(OT_PRETTY_CLASS())(name_)(
            ": waiting for existing request to arrive or time out before "
            "requesting new data")
            .Flush();
    } else if (queue_.size() >= queue_limit_) {
        log(OT_PRETTY_CLASS())(name_)(
            ": avoiding requests for new data while queue is full")
            .Flush();
    } else if (local >= target) {
        log(OT_PRETTY_CLASS())(name_)(
            ": no peers report data better than current position ")(local)
            .Flush();
    } else {
        const auto& best = best_position();
        log(OT_PRETTY_CLASS())(name_)(": requesting data for blocks after ")(
            best)
            .Flush();
        send_request(best);
    }
}

auto OTDHT::Actor::filter_peers(
    const opentxs::blockchain::block::Position& target) const noexcept
    -> Vector<PeerID>
{
    const auto& log = log_;
    auto out = Vector<PeerID>{get_allocator()};
    out.clear();

    for (const auto& [id, peer] : peers_) {
        OT_ASSERT(0_uz < id.size());

        const auto& [position, samples, weight] = peer;

        if (position > target) {
            const auto& val = out.emplace_back(id);

            OT_ASSERT(0_uz < val.size());
        }
    }

    log(OT_PRETTY_CLASS())(name_)(": ")(out.size())(" of ")(peers_.size())(
        " connected peers report chain data better than local position ")(
        target)
        .Flush();

    return out;
}

auto OTDHT::Actor::finish_request(bool success) noexcept -> void
{
    OT_ASSERT(last_request_.has_value());

    const auto& log = log_;

    try {
        const auto& [start, id] = *last_request_;
        auto& [position, samples, weight] = peers_.at(id);

        if (success) {
            const auto elapsed = std::chrono::duration_cast<ScoreInterval>(
                sClock::now() - start);
            const auto remaining = std::chrono::duration_cast<ScoreInterval>(
                request_timeout_ - elapsed);
            const auto credit =
                std::max<Weight>(remaining.count(), min_weight_);
            add_contribution(samples, weight, credit);
        } else {
            add_contribution(samples, weight, min_weight_);
        }

        log(OT_PRETTY_CLASS())(name_)(": weight for ")(id)(" updated to ")(
            weight)
            .Flush();
    } catch (const std::exception& e) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Abort();
    }

    last_request_.reset();
    request_timer_.Cancel();
}

auto OTDHT::Actor::get_peer(const Message& msg) const noexcept -> ReadView
{
    const auto header = msg.Header();

    if (const auto size = header.size(); 0_uz == size) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    return header.at(0).Bytes();
}

auto OTDHT::Actor::get_peers() const noexcept -> Set<PeerID>
{
    auto out = Set<PeerID>{get_allocator()};

    for (const auto& [endpoint, _] : peers_) { out.emplace(endpoint); }

    return out;
}

auto OTDHT::Actor::get_peers(
    const zeromq::FrameSection& body,
    std::ptrdiff_t offset) const noexcept -> Set<PeerID>
{
    auto out = Set<PeerID>{get_allocator()};

    for (auto i = std::next(body.begin(), offset), stop = body.end(); i != stop;
         ++i) {
        out.emplace(i->Bytes());
    }

    return out;
}

auto OTDHT::Actor::pipeline(const Work work, Message&& msg) noexcept -> void
{
    const auto id = msg.Internal().ExtractFront().as<zeromq::SocketID>();

    if (router_.ID() == id) {
        pipeline_router(work, std::move(msg));
    } else {
        pipeline_other(work, std::move(msg));
    }

    do_work();
}

auto OTDHT::Actor::pipeline_other(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::push_tx: {
            process_pushtx_internal(std::move(msg));
        } break;
        case Work::job_processed: {
            process_job_processed(std::move(msg));
        } break;
        case Work::peer_list: {
            process_peer_list(std::move(msg));
        } break;
        case Work::registration: {
            process_registration_node(std::move(msg));
        } break;
        case Work::cfilter: {
            process_cfilter(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::sync_ack:
        case Work::sync_reply:
        case Work::sync_push:
        case Work::response:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto OTDHT::Actor::pipeline_router(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::sync_ack:
        case Work::sync_reply:
        case Work::sync_push: {
            process_sync_peer(std::move(msg));
        } break;
        case Work::response: {
            process_response_peer(std::move(msg));
        } break;
        case Work::registration: {
            process_registration_peer(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::push_tx:
        case Work::job_processed:
        case Work::peer_list:
        case Work::init:
        case Work::cfilter:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto OTDHT::Actor::process_ack(
    const Message& msg,
    const otdht::Acknowledgement& ack) noexcept -> void
{
    try {
        process_state(msg, ack.State(chain_));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
    }
}

auto OTDHT::Actor::process_cfilter(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (3 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto type = body.at(1).as<opentxs::blockchain::cfilter::Type>();

    if (filter_type_ != type) {
        log_(OT_PRETTY_CLASS())(name_)(": ignoring update for filter type ")(
            print(type))
            .Flush();

        return;
    }

    local_position_ = {
        body.at(2).as<opentxs::blockchain::block::Height>(),
        body.at(3).Bytes()};
}

auto OTDHT::Actor::process_data(Message&& msg, const otdht::Data& data) noexcept
    -> void
{
    const auto& log = log_;

    if (false == process_state(msg, data.State())) {
        LogError()(OT_PRETTY_CLASS())(name_)(": received data for wrong chain")
            .Flush();

        return;
    } else if (data.Blocks().empty()) {
        log(OT_PRETTY_CLASS())(name_)(": ignoring empty update").Flush();

        return;
    } else if (false == can_connect(data)) {
        log(OT_PRETTY_CLASS())(name_)(": ignoring non-contiguous update")
            .Flush();

        return;
    }

    const auto& [_, position] =
        queue_.emplace_back(std::move(msg), data.LastPosition(api_));
    log(OT_PRETTY_CLASS())(name_)(": queued data for block range ending at ")(
        position)
        .Flush();
}

auto OTDHT::Actor::process_job_processed(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (2 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    processing_ = false;
    local_position_ = {
        body.at(1).as<opentxs::blockchain::block::Height>(),
        body.at(2).Bytes()};
    processing_position_ = {};
}

auto OTDHT::Actor::process_peer_list(Message&& msg) noexcept -> void
{
    auto newPeers = get_peers(msg.Body(), 1_z);
    add_peers([&] {
        auto out = Set<PeerID>{get_allocator()};
        std::set_difference(
            newPeers.begin(),
            newPeers.end(),
            known_peers_.begin(),
            known_peers_.end(),
            std::inserter(out, out.end()));

        return out;
    }());
    remove_peers([&] {
        auto out = Set<PeerID>{get_allocator()};
        std::set_difference(
            known_peers_.begin(),
            known_peers_.end(),
            newPeers.begin(),
            newPeers.end(),
            std::inserter(out, out.end()));

        return out;
    }());
    known_peers_.swap(newPeers);
}

auto OTDHT::Actor::process_pushtx_internal(Message&& msg) noexcept -> void
{
    // TODO c++20 capture structured binding
    for (const auto& [id, _] : peers_) {
        router_.SendDeferred(
            [&](const auto& id) {
                auto out = zeromq::reply_to_connection(id);

                OT_ASSERT(0_uz < out.Header().size());

                for (const auto& frame : msg.Body()) { out.AddFrame(frame); }

                return out;
            }(id),
            __FILE__,
            __LINE__);
    }
}

auto OTDHT::Actor::process_registration_node(Message&& msg) noexcept -> void
{
    registered_with_node_ = true;
    add_peers(get_peers(msg.Body(), 1_z));
}

auto OTDHT::Actor::process_registration_peer(Message&& msg) noexcept -> void
{
    const auto& log = log_;
    const auto peer = PeerID{get_peer(msg), get_allocator()};
    log(OT_PRETTY_CLASS())(name_)(": received registration message from peer ")(
        peer)
        .Flush();
    known_peers_.emplace(peer);
    peers_[peer].position_ = {};
    using PeerJob = otdht::PeerJob;
    router_.SendDeferred(
        [&] {
            auto out = tagged_reply_to_message(msg, PeerJob::registration);
            out.AddFrame(chain_);

            return out;
        }(),
        __FILE__,
        __LINE__);
}

auto OTDHT::Actor::process_response_peer(Message&& msg) noexcept -> void
{
    try {
        const auto base = api_.Factory().BlockchainSyncMessage(msg);

        if (!base) {
            throw std::runtime_error{"failed to instantiate response"};
        }

        using Type = opentxs::network::otdht::MessageType;
        const auto type = base->Type();

        switch (type) {
            case Type::pushtx_reply: {
                const auto& reply = base->asPushTransactionReply();
                log_(OT_PRETTY_CLASS())(name_)(": transaction ")
                    .asHex(reply.ID())(" broadcast ")(
                        reply.Success() ? "successfully" : "unsuccessfully")
                    .Flush();
                // TODO notify mempool
            } break;
            case Type::error:
            case Type::sync_request:
            case Type::sync_ack:
            case Type::sync_reply:
            case Type::new_block_header:
            case Type::query:
            case Type::publish_contract:
            case Type::publish_ack:
            case Type::contract_query:
            case Type::contract:
            case Type::pushtx:
            default: {
                const auto error =
                    CString{"Unsupported response type "}.append(print(type));

                throw std::runtime_error{error.c_str()};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
    }
}

auto OTDHT::Actor::process_state(
    const Message& msg,
    const otdht::State& state) noexcept -> bool
{
    if (state.Chain() != chain_) { return false; }

    const auto& position = state.Position();
    peers_[PeerID{get_peer(msg), get_allocator()}].position_ = position;
    update_remote_position(position);

    return true;
}

auto OTDHT::Actor::process_sync_peer(Message&& msg) noexcept -> void
{
    try {
        const auto peer = PeerID{get_peer(msg), get_allocator()};
        const auto sync = api_.Factory().BlockchainSyncMessage(msg);
        const auto type = sync->Type();
        using Type = opentxs::network::otdht::MessageType;
        auto finish{false};

        switch (type) {
            case Type::sync_ack: {
                process_ack(msg, sync->asAcknowledgement());
            } break;
            case Type::sync_reply: {
                finish = true;
                [[fallthrough]];
            }
            case Type::new_block_header: {
                process_data(std::move(msg), sync->asData());
            } break;
            case Type::error:
            case Type::sync_request:
            case Type::query:
            case Type::publish_contract:
            case Type::publish_ack:
            case Type::contract_query:
            case Type::contract:
            case Type::pushtx:
            case Type::pushtx_reply: {
                const auto error =
                    CString{}
                        .append("unsupported message type on external socket: ")
                        .append(print(type));

                throw std::runtime_error{error.c_str()};
            }
            default: {
                const auto error = CString{}
                                       .append("unknown message type: ")
                                       .append(std::to_string(
                                           static_cast<otdht::TypeEnum>(type)));

                throw std::runtime_error{error.c_str()};
            }
        }

        if (finish && last_request_.has_value()) {
            if (peer == last_request_->second) { finish_request(true); }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
    }
}

auto OTDHT::Actor::remove_peers(Set<PeerID>&& peers) noexcept -> void
{
    const auto& log = log_;

    for (const auto& id : peers) {
        log(OT_PRETTY_CLASS())(name_)(": removing stale peer ")(id).Flush();
        known_peers_.erase(id);
        peers_.erase(id);
    }
}

auto OTDHT::Actor::reset_registration_timer() noexcept -> void
{
    reset_timer(1s, registration_timer_, Work::statemachine);
}

auto OTDHT::Actor::reset_request_timer() noexcept -> void
{
    reset_timer(request_timeout_, request_timer_, Work::statemachine);
}

auto OTDHT::Actor::send_registration() noexcept -> void
{
    pipeline_.Internal().SendFromThread([&] {
        auto out = MakeWork(otdht::NodeJob::registration);
        out.AddFrame(chain_);
        out.AddFrame(local_position_.height_);
        out.AddFrame(local_position_.hash_);

        for (const auto& endpoint : get_peers()) { out.AddFrame(endpoint); }

        return out;
    }());
}

auto OTDHT::Actor::send_request(
    const opentxs::blockchain::block::Position& best) noexcept -> void
{
    const auto& log = log_;
    const auto peer = choose_peer(best);

    if (peer.has_value()) {
        log(OT_PRETTY_CLASS())(name_)(
            ": requesting sync data starting from block ")(best.height_)(
            " from peer (") (*peer)(")")
            .Flush();
    } else {
        log(OT_PRETTY_CLASS())(name_)(
            ": no known peers with positions higher than ")(best)
            .Flush();

        return;
    }

    const auto message = factory::BlockchainSyncRequest([&] {
        auto out = otdht::StateData{get_allocator()};
        out.emplace_back(chain_, best);

        return out;
    }());
    router_.SendDeferred(
        [&] {
            auto out = zeromq::reply_to_connection(*peer);

            OT_ASSERT(0_uz < out.Header().size());

            const auto rc = message.Serialize(out);

            OT_ASSERT(rc);

            return out;
        }(),
        __FILE__,
        __LINE__);
    last_request_ = std::make_pair(sClock::now(), *peer);
    reset_request_timer();
}

auto OTDHT::Actor::update_position(
    const opentxs::blockchain::block::Position& incoming,
    opentxs::blockchain::block::Position& existing) noexcept -> void
{
    existing = std::max(existing, incoming);
}

auto OTDHT::Actor::update_remote_position(
    const opentxs::blockchain::block::Position& incoming) noexcept -> void
{
    update_position(incoming, best_remote_position_);
    log_(OT_PRETTY_CLASS())(name_)(": best remote position is ")(
        best_remote_position_)
        .Flush();
}

auto OTDHT::Actor::work() noexcept -> bool
{
    const auto& log = log_;

    if (check_registration() || check_peers()) { reset_registration_timer(); }

    switch (mode_) {
        case Mode::client: {
            check_request_timer();
            drain_queue();
            fill_queue();
        } break;
        case Mode::disabled:
        case Mode::server: {
            log(OT_PRETTY_CLASS())(name_)(": no actions necessary in this mode")
                .Flush();
        } break;
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid mode")(
                static_cast<int>(mode_))
                .Abort();
        }
    }

    return false;
}

OTDHT::Actor::~Actor() = default;
}  // namespace opentxs::network::blockchain
