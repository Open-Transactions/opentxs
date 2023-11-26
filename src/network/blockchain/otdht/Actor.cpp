// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/otdht/Actor.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <compare>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>

#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "network/blockchain/otdht/Client.hpp"
#include "network/blockchain/otdht/Server.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/PushTransaction.hpp"
#include "opentxs/network/otdht/PushTransactionReply.hpp"
#include "opentxs/network/otdht/Request.hpp"
#include "opentxs/network/otdht/State.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::network::blockchain
{
OTDHT::Actor::PeerData::PeerData(Type type, allocator_type alloc) noexcept
    : type_(type)
    , position_()
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
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

OTDHT::Actor::Actor(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> node,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : opentxs::Actor<OTDHT::Actor, DHTJob>(
          api->Self(),
          LogTrace(),
          [&] {
              return CString{print(node->Internal().Chain()), alloc}.append(
                  " OTDHT node");
          }(),
          0ms,
          batchID,
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
              {node->Internal().Endpoints().shutdown_publish_, Connect},
              {node->Internal().Endpoints().new_filter_publish_, Connect},
              {api->Endpoints().Internal().OTDHTNodePublish(), Connect},
              {api->Endpoints().Internal().BlockchainReportStatus(), Connect},
              {api->Endpoints().Internal().BlockchainSyncChecksumFailure(),
               Connect},
          },
          {
              {node->Internal().Endpoints().otdht_pull_, Bind},
          },
          {
              {api->Endpoints().Internal().OTDHTNodeRouter(), Connect},
          },
          {
              {Router,
               Internal,
               {
                   {api->Endpoints().Internal().OTDHTBlockchain(
                        node->Internal().Chain()),
                    Bind},
               }},
              {Push,
               Internal,
               {
                   {node->Internal().Endpoints().manager_pull_, Connect},
               }},
              {Push,
               Internal,
               {
                   {api->Endpoints().Internal().BlockchainMessageRouter(),
                    Connect},
               }},
          })
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , api_(api_p_->Self())
    , node_(*node_p_)
    , chain_(node_.Internal().Chain())
    , filter_type_(node_.FilterOracle().DefaultType())
    , to_dht_(pipeline_.Internal().ExtraSocket(0_uz))
    , to_blockchain_(pipeline_.Internal().ExtraSocket(1_uz))
    , to_api_(pipeline_.Internal().ExtraSocket(2_uz))
    , known_peers_(alloc)
    , peers_(alloc)
    , rand_(std::random_device{}())
    , registered_with_node_(false)
    , oracle_position_()
    , last_request_(std::nullopt)
    , peer_index_()
    , registration_timer_(api_.Network().Asio().Internal().GetTimer())
    , request_timer_(api_.Network().Asio().Internal().GetTimer())
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
            log()(name_)(": discovered new peer ")(*endpoint).Flush();
        }
    }
}

auto OTDHT::Actor::calculate_weight(const Samples& samples) noexcept -> Weight
{
    const auto average =
        std::reduce(samples.begin(), samples.end(), 0_z) / samples.size();

    return std::max<Weight>(min_weight_, average);
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
    if (false == have_outstanding_request()) {
        log_()(name_)(": no outstanding request").Flush();

        return;
    }

    const auto& [time, peer] = *last_request_;

    if ((sClock::now() - time) > request_timeout_) {
        log_()(name_)(": request timeout").Flush();
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
            range += peers_.at(id).weight_;
            map.emplace(range, id);
        }

        assert_true(map.size() == count);

        auto dist = std::uniform_int_distribution<Weight>{0_z, range - 1_z};

        const auto i = map.upper_bound(dist(rand_));

        assert_true(map.end() != i);

        return i->second;
    } catch (const std::exception& e) {
        LogAbort()()(name_)(": ")(e.what()).Abort();
    }
}

auto OTDHT::Actor::do_shutdown() noexcept -> void
{
    request_timer_.Cancel();
    registration_timer_.Cancel();
    node_p_.reset();
    api_p_.reset();
}

auto OTDHT::Actor::do_startup(allocator_type) noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (node_.Internal().ShuttingDown())) {

        return true;
    }

    update_oracle_position(node_.FilterOracle().FilterTip(filter_type_));
    do_work();

    return false;
}

auto OTDHT::Actor::Factory(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> node,
    network::zeromq::BatchID batchID) noexcept -> void
{
    const auto& zmq = api->Network().ZeroMQ().Context().Internal();
    auto actor = [&]() -> std::shared_ptr<Actor> {
        switch (node->Internal().GetConfig().profile_) {
            case BlockchainProfile::mobile:
            case BlockchainProfile::desktop: {
                using Type = blockchain::otdht::Client;

                return std::allocate_shared<Type>(
                    alloc::PMR<Type>{zmq.Alloc(batchID)}, api, node, batchID);
            }
            case BlockchainProfile::desktop_native: {
                using Type = Actor;

                return std::allocate_shared<Type>(
                    alloc::PMR<Type>{zmq.Alloc(batchID)}, api, node, batchID);
            }
            case BlockchainProfile::server: {
                using Type = blockchain::otdht::Server;

                return std::allocate_shared<Type>(
                    alloc::PMR<Type>{zmq.Alloc(batchID)}, api, node, batchID);
            }
            default: {
                LogAbort()()("invalid profile type").Abort();
            }
        }
    }();

    assert_false(nullptr == actor);

    actor->Init(actor);
}

auto OTDHT::Actor::filter_peers(
    const opentxs::blockchain::block::Position& target) const noexcept
    -> Vector<PeerID>
{
    const auto& log = log_;
    auto out = Vector<PeerID>{get_allocator()};
    out.clear();

    for (const auto& [id, peer] : peers_) {
        assert_false(id.empty());

        if (PeerData::Type::incoming == peer.type_) { continue; }

        if (peer.position_.IsReplacedBy(target)) {
            const auto& val = out.emplace_back(id);

            assert_false(val.empty());
        }
    }

    log()(name_)(": ")(out.size())(" of ")(peers_.size())(
        " connected peers report chain data better than local position ")(
        target)
        .Flush();

    return out;
}

auto OTDHT::Actor::finish_request(PeerID peer) noexcept -> void
{
    if (have_outstanding_request() && (peer == last_request_->second)) {
        finish_request(true);
    }
}

auto OTDHT::Actor::finish_request(bool success) noexcept -> void
{
    assert_true(have_outstanding_request());

    const auto& log = log_;

    try {
        const auto& [start, id] = *last_request_;
        const auto weight = [&]() {
            if (success) {
                const auto elapsed = std::chrono::duration_cast<ScoreInterval>(
                    sClock::now() - start);
                const auto remaining =
                    std::chrono::duration_cast<ScoreInterval>(
                        request_timeout_ - elapsed);

                return std::max<Weight>(
                    static_cast<Weight>(remaining.count()), min_weight_);
            } else {

                return min_weight_;
            }
        }();
        auto& peer = peers_.at(id);
        add_contribution(peer.samples_, peer.weight_, weight);
        log()(name_)(": weight for ")(id)(" updated to ")(peer.weight_).Flush();
    } catch (const std::exception& e) {
        LogAbort()()(name_)(": ")(e.what()).Abort();
    }

    last_request_.reset();
    request_timer_.Cancel();
}

auto OTDHT::Actor::get_peer(const Message& message) noexcept -> PeerID
{
    auto envelope = message.Envelope();

    assert_true(envelope.IsValid());

    return PeerID{envelope.get()[0].Bytes(), get_allocator()};
}

auto OTDHT::Actor::get_peers() const noexcept -> Set<PeerID>
{
    auto out = Set<PeerID>{get_allocator()};

    for (const auto& [endpoint, _] : peers_) { out.emplace(endpoint); }

    return out;
}

auto OTDHT::Actor::get_peers(
    std::span<const zeromq::Frame> body,
    std::ptrdiff_t offset) const noexcept -> Set<PeerID>
{
    auto out = Set<PeerID>{get_allocator()};
    out.clear();

    for (auto i = std::next(body.begin(), offset), stop = body.end(); i != stop;
         ++i) {
        out.emplace(i->Bytes());
    }

    return out;
}

auto OTDHT::Actor::have_outstanding_request() const noexcept -> bool
{
    return last_request_.has_value();
}

auto OTDHT::Actor::make_envelope(const PeerID& peer) noexcept -> Message
{
    auto out = Message{};
    out.AddFrame(peer);
    out.StartBody();

    return out;
}

auto OTDHT::Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    if (const auto id = connection_id(msg); to_dht().ID() == id) {
        pipeline_router(work, std::move(msg), monotonic);
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
        case Work::checksum_failure: {
            process_checksum_failure(std::move(msg));
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
        case Work::report: {
            process_report(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::sync_request:
        case Work::sync_ack:
        case Work::sync_reply:
        case Work::sync_push:
        case Work::response:
        case Work::init:
        case Work::statemachine: {
            LogAbort()()(name_)(": unhandled message type ")(print(work))
                .Abort();
        }
        default: {
            LogAbort()()(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto OTDHT::Actor::pipeline_router(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::sync_request: {
            process_sync_request(std::move(msg));
        } break;
        case Work::sync_ack:
        case Work::sync_reply:
        case Work::sync_push: {
            process_sync_peer(std::move(msg));
        } break;
        case Work::response: {
            process_response_peer(std::move(msg));
        } break;
        case Work::push_tx: {
            process_pushtx_external(std::move(msg), monotonic);
        } break;
        case Work::registration: {
            process_registration_peer(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::job_processed:
        case Work::checksum_failure:
        case Work::report:
        case Work::peer_list:
        case Work::init:
        case Work::cfilter:
        case Work::statemachine: {
            LogAbort()()(name_)(": unhandled message type ")(print(work))
                .Abort();
        }
        default: {
            LogAbort()()(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto OTDHT::Actor::process_cfilter(Message&& msg) noexcept -> void
{
    const auto& log = log_;
    const auto body = msg.Payload();

    if (3 >= body.size()) { LogAbort()()(name_)(": invalid message").Abort(); }

    const auto type = body[1].as<opentxs::blockchain::cfilter::Type>();

    if (filter_type_ != type) {
        log()(name_)(": ignoring update for filter type ")(print(type)).Flush();

        return;
    }

    auto position = opentxs::blockchain::block::Position{
        body[2].as<opentxs::blockchain::block::Height>(), body[3].Bytes()};
    log()(name_)(": updated oracle position to ")(position).Flush();
    update_oracle_position(std::move(position));
}

auto OTDHT::Actor::process_job_processed(Message&& msg) noexcept -> void
{
    LogAbort()()(name_)(": invalid message").Abort();
}

auto OTDHT::Actor::process_checksum_failure(Message&& msg) noexcept -> void {}

auto OTDHT::Actor::process_peer_list(Message&& msg) noexcept -> void
{
    auto newPeers = get_peers(msg.Payload(), 1_z);
    add_peers([&] {
        auto out = Set<PeerID>{get_allocator()};
        std::ranges::set_difference(
            newPeers, known_peers_, std::inserter(out, out.end()));

        return out;
    }());
    remove_peers([&] {
        auto out = Set<PeerID>{get_allocator()};
        std::ranges::set_difference(
            known_peers_, newPeers, std::inserter(out, out.end()));

        return out;
    }());
    known_peers_.swap(newPeers);
}

auto OTDHT::Actor::process_pushtx_external(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto pBase = api_.Factory().BlockchainSyncMessage(msg);

    assert_false(nullptr == pBase);

    const auto& base = *pBase;
    const auto& pushtx = base.asPushTransaction();
    const auto chain = pushtx.Chain();
    auto success{false};

    try {
        const auto tx = api_.Factory().BlockchainTransaction(
            chain, pushtx.Payload(), false, Clock::now(), monotonic);

        if (false == tx.IsValid()) {
            throw std::runtime_error{"Invalid transaction"};
        }

        success = node_.Internal().BroadcastTransaction(tx);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
        success = false;
    }

    to_dht().SendExternal([&] {
        auto out = zeromq::reply_to_message(std::move(msg));
        const auto reply = factory::BlockchainSyncPushTransactionReply(
            chain, pushtx.ID(), success);
        reply.Serialize(out);

        return out;
    }());
}

auto OTDHT::Actor::process_pushtx_internal(Message&& msg) noexcept -> void
{
    for (const auto& [peerid, _] : peers_) {
        to_dht().SendDeferred([&]() {
            auto out = make_envelope(peerid);
            out.MoveFrames(msg.Payload());

            return out;
        }());
    }
}

auto OTDHT::Actor::process_registration_node(Message&& msg) noexcept -> void
{
    registered_with_node_ = true;
    add_peers(get_peers(msg.Payload(), 1_z));
}

auto OTDHT::Actor::process_registration_peer(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    if (1_uz >= body.size()) {
        LogAbort()()(name_)(": invalid message").Abort();
    }

    const auto& log = log_;
    const auto peer = get_peer(msg);
    log()(name_)(": received registration message from peer ")(peer).Flush();
    known_peers_.emplace(peer);

    if (auto i = peers_.find(peer); peers_.end() == i) {
        auto [j, added] =
            peers_.try_emplace(peer, body[1].as<PeerData::Type>());

        assert_true(added);
    } else {

        i->second.position_ = {};
    }

    using enum network::otdht::PeerJob;
    to_dht().SendDeferred([&] {
        auto out = tagged_reply_to_message(msg, registration);
        out.AddFrame(chain_);

        return out;
    }());
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
                log_()(name_)(": transaction ")
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
        LogError()()(name_)(": ")(e.what()).Flush();
    }
}

auto OTDHT::Actor::remove_peers(Set<PeerID>&& peers) noexcept -> void
{
    const auto& log = log_;

    for (const auto& id : peers) {
        log()(name_)(": removing stale peer ")(id).Flush();
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
    const auto local = local_position();
    pipeline_.Internal().SendFromThread([&] {
        auto out = MakeWork(network::otdht::NodeJob::registration);
        out.AddFrame(chain_);
        out.AddFrame(local.height_);
        out.AddFrame(local.hash_);

        for (const auto& endpoint : get_peers()) {

            assert_false(endpoint.empty());

            out.AddFrame(endpoint);
        }

        return out;
    }());
}

auto OTDHT::Actor::send_request(
    const opentxs::blockchain::block::Position& best) noexcept -> void
{
    const auto& log = log_;
    const auto peer = choose_peer(best);

    if (peer.has_value()) {
        log()(name_)(": requesting sync data starting from block ")(
            best.height_)(" from peer (") (*peer)(")")
            .Flush();
    } else {
        log()(name_)(": no known peers with positions higher than ")(best)
            .Flush();

        return;
    }

    const auto message = factory::BlockchainSyncRequest([&] {
        auto out = network::otdht::StateData{get_allocator()};
        out.emplace_back(chain_, best);

        return out;
    }());
    to_dht().SendDeferred([&] {
        auto out = make_envelope(*peer);
        const auto rc = message.Serialize(out);

        assert_true(rc);

        return out;
    }());
    last_request_ = std::make_pair(sClock::now(), *peer);
    reset_request_timer();
}

auto OTDHT::Actor::send_to_listeners(Message msg) noexcept -> void
{
    for (auto& [peerid, peer] : peers_) {
        if (PeerData::Type::incoming == peer.type_) {
            to_dht().SendDeferred([&]() {
                auto out = make_envelope(peerid);
                auto body = msg.Payload();

                for (auto& frame : body) { out.AddFrame(std::move(frame)); }

                return out;
            }());
        }
    }
}

auto OTDHT::Actor::update_oracle_position(
    opentxs::blockchain::block::Position position) noexcept -> void
{
    auto handle = oracle_position_.lock();
    *handle = std::move(position);
}

auto OTDHT::Actor::update_peer_position(
    const PeerID& peer,
    opentxs::blockchain::block::Position position) noexcept -> void
{
    try {
        peers_.at(peer).position_ = std::move(position);
    } catch (const std::exception& e) {
        LogAbort()()(name_)(": ")(e.what()).Abort();
    }
}

auto OTDHT::Actor::update_position(
    const opentxs::blockchain::block::Position& incoming,
    opentxs::blockchain::block::Position& existing) noexcept -> void
{
    existing = std::max(existing, incoming);
}

auto OTDHT::Actor::work(allocator_type monotonic) noexcept -> bool
{
    if (check_registration() || check_peers()) { reset_registration_timer(); }

    check_request_timer();
    do_work();

    return false;
}

OTDHT::Actor::~Actor() = default;
}  // namespace opentxs::network::blockchain
