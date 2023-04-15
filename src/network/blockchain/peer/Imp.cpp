// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/peer/Imp.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>  // IWYU pragma: keep
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <sodium.h>
#include <zmq.h>
#include <algorithm>
#include <compare>
#include <iterator>
#include <span>
#include <tuple>

#include "internal/api/network/Asio.hpp"
#include "internal/api/session/FactoryAPI.hpp"  // IWYU pragma: keep
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Peer.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/network/blockchain/Address.hpp"
#include "internal/network/blockchain/ConnectionManager.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/peer/HasJob.hpp"
#include "network/blockchain/peer/JobType.hpp"
#include "network/blockchain/peer/RunJob.hpp"
#include "network/blockchain/peer/UpdateBlockJob.hpp"
#include "network/blockchain/peer/UpdateGetHeadersJob.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Service.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Work.hpp"

namespace opentxs::network::blockchain
{
using namespace std::literals;

auto print(PeerJob in) noexcept -> std::string_view
{
    using enum PeerJob;
    static constexpr auto map =
        frozen::make_unordered_map<PeerJob, std::string_view>({
            {shutdown, "shutdown"sv},
            {blockheader, "blockheader"sv},
            {reorg, "reorg"sv},
            {mempool, "mempool"sv},
            {registration, "registration"sv},
            {connect, "connect"sv},
            {disconnect, "disconnect"sv},
            {sendresult, "sendresult"sv},
            {p2p, "p2p"sv},
            {gossip_address, "gossip_address"sv},
            {jobtimeout, "jobtimeout"sv},
            {needpeers, "needpeers"sv},
            {statetimeout, "statetimeout"sv},
            {activitytimeout, "activitytimeout"sv},
            {needping, "needping"sv},
            {body, "body"sv},
            {header, "header"sv},
            {broadcasttx, "broadcasttx"sv},
            {jobavailablegetheaders, "jobavailablegetheaders"sv},
            {jobavailableblock, "jobavailableblock"sv},
            {block, "block"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid PeerJob: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}
}  // namespace opentxs::network::blockchain

namespace opentxs::network::blockchain::internal
{
template <typename J>
auto Peer::Imp::job_name(const J& job) noexcept -> std::string_view
{
    return JobType::get()(job);
}

template <typename Visitor>
auto Peer::Imp::update_job(Visitor& visitor, allocator_type monotonic) noexcept
    -> bool
{
    const auto [isJob, isFinished] = std::visit(visitor, job_);

    if (isJob) {
        if (isFinished) {
            finish_job(monotonic);
        } else {
            reset_job_timer();
        }
    }

    return isJob;
}
}  // namespace opentxs::network::blockchain::internal

namespace opentxs::network::blockchain::internal
{
Peer::Imp::Imp(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> network,
    int peerID,
    blockchain::Address address,
    Set<network::blockchain::Address> gossip,
    std::chrono::milliseconds pingInterval,
    std::chrono::milliseconds inactivityInterval,
    std::chrono::milliseconds peersInterval,
    std::size_t headerBytes,
    std::string_view fromParent,
    std::optional<asio::Socket> socket,
    zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Actor(
          *api,
          LogTrace(),
          [&] {
              using opentxs::blockchain::print;
              auto out = CString{print(network->Internal().Chain()), alloc};

              OT_ASSERT(address.IsValid());

              if (address.Internal().Incoming()) {
                  out.append(" incoming"sv);
              } else {
                  out.append(" outgoing"sv);
              }

              out.append(" peer "sv);
              out.append(address.Display());
              out.append(" ("sv);
              out.append(std::to_string(peerID));
              out.append(")"sv);

              return out;
          }(),
          0ms,
          batch,
          alloc,
          [&] {
              using enum zeromq::socket::Direction;
              auto sub = zeromq::EndpointArgs{alloc};
              sub.clear();
              sub.emplace_back(api->Endpoints().BlockchainReorg(), Connect);
              sub.emplace_back(api->Endpoints().Shutdown(), Connect);
              sub.emplace_back(
                  network->Internal().Endpoints().peer_manager_publish_,
                  Connect);
              sub.emplace_back(
                  network->Internal().Endpoints().shutdown_publish_, Connect);

              return sub;
          }(),
          [&] {
              using enum zeromq::socket::Direction;
              auto pull = zeromq::EndpointArgs{alloc};
              pull.clear();
              pull.emplace_back(fromParent, Connect);

              return pull;
          }(),
          {},
          [&] {
              using enum zeromq::socket::Direction;
              using enum zeromq::socket::Type;
              auto extra = Vector<zeromq::SocketData>{alloc};
              extra.emplace_back(
                  Push,
                  [&] {
                      auto args = Vector<zeromq::EndpointArg>{alloc};
                      args.clear();
                      args.emplace_back(
                          network->Internal().Endpoints().block_oracle_pull_,
                          Connect);

                      return args;
                  }(),
                  false);  // NOTE to_block_oracle_
              extra.emplace_back(
                  Push,
                  [&] {
                      auto args = Vector<zeromq::EndpointArg>{alloc};
                      args.clear();
                      args.emplace_back(
                          network->Internal().Endpoints().header_oracle_pull_,
                          Connect);

                      return args;
                  }(),
                  false);  // NOTE to_header_oracle_
              extra.emplace_back(
                  Push,
                  [&] {
                      auto args = Vector<zeromq::EndpointArg>{alloc};
                      args.clear();
                      args.emplace_back(
                          network->Internal().Endpoints().peer_manager_pull_,
                          Connect);

                      return args;
                  }(),
                  false);  // NOTE to_peer_manager_
              extra.emplace_back(
                  Dealer,
                  [&] {
                      auto args = Vector<zeromq::EndpointArg>{alloc};
                      args.clear();

                      return args;
                  }(),
                  true);  // NOTE external_

              return extra;
          }())
    , api_p_(api)
    , network_p_(network)
    , api_(*api_p_)
    , network_(*network_p_)
    , config_(network_.Internal().GetConfig())
    , header_oracle_(network_.HeaderOracle())
    , block_oracle_(network_.BlockOracle())
    , filter_oracle_(network_.FilterOracle())
    , chain_(network->Internal().Chain())
    , dir_([&] {
        if (address.Internal().Incoming()) {

            return Dir::incoming;
        } else {

            return Dir::outgoing;
        }
    }())
    , siphash_key_([] {
        auto out = decltype(siphash_key_){};
        static_assert(
            decltype(out)::payload_size_ == crypto_shorthash_KEYBYTES);
        ::crypto_shorthash_keygen(static_cast<unsigned char*>(out.data()));

        return out;
    }())
    , database_(network_.Internal().DB())
    , to_block_oracle_(pipeline_.Internal().ExtraSocket(0))
    , to_header_oracle_(pipeline_.Internal().ExtraSocket(1))
    , to_peer_manager_(pipeline_.Internal().ExtraSocket(2))
    , id_(peerID)
    , curve_keys_([&] {
        auto out =
            std::make_pair(api_.Factory().Secret(41_uz), FixedByteArray<41>{});
        auto& [sec, pub] = out;
        const auto rc = ::zmq_curve_keypair(
            static_cast<char*>(pub.data()), static_cast<char*>(sec.data()));

        OT_ASSERT(0 == rc);

        return out;
    }())
    , external_(pipeline_.Internal().ExtraSocket(3))
    , untrusted_connection_id_(external_.ID())
    , ping_interval_(std::move(pingInterval))
    , inactivity_interval_(std::move(inactivityInterval))
    , peers_interval_(std::move(peersInterval))
    , remote_address_(std::move(address))
    , connection_p_(init_connection_manager(
          api_,
          network_,
          *this,
          remote_address_,
          log_,
          id_,
          headerBytes,
          std::move(socket)))
    , connection_(*connection_p_)
    , state_(State::pre_init)
    , last_activity_()
    , state_timer_(api_.Network().Asio().Internal().GetTimer())
    , ping_timer_(api_.Network().Asio().Internal().GetTimer())
    , activity_timer_(api_.Network().Asio().Internal().GetTimer())
    , peers_timer_(api_.Network().Asio().Internal().GetTimer())
    , job_timer_(api_.Network().Asio().Internal().GetTimer())
    , known_transactions_(alloc)
    , known_blocks_(alloc)
    , known_addresses_(alloc)
    , gossip_address_queue_([&] {
        auto out = decltype(gossip_address_queue_){alloc};
        out.reserve(gossip.size());
        out.clear();
        std::move(gossip.begin(), gossip.end(), std::back_inserter(out));

        return out;
    }())
    , local_position_()
    , remote_position_()
    , job_()
    , is_caught_up_(false)
    , block_header_capability_(false)
    , cfilter_capability_(false)
    , failed_peer_(false)
{
    OT_ASSERT(api_p_);
    OT_ASSERT(network_p_);
    OT_ASSERT(connection_p_);

    known_addresses_.emplace(hash(this->address()));
}

auto Peer::Imp::add_known_address(
    std::span<const blockchain::Address> addresses) noexcept -> void
{
    for (const auto& address : addresses) {
        known_addresses_.emplace(hash(address));
    }
}

auto Peer::Imp::add_known_block(opentxs::blockchain::block::Hash hash) noexcept
    -> bool
{
    const auto [i, added] = known_blocks_.emplace(std::move(hash));

    return added;
}

auto Peer::Imp::add_known_tx(const Txid& txid) noexcept -> bool
{
    return add_known_tx(Txid{txid});
}

auto Peer::Imp::add_known_tx(Txid&& txid) noexcept -> bool
{
    const auto [i, added] = known_transactions_.emplace(std::move(txid));

    return added;
}

auto Peer::Imp::cancel_timers() noexcept -> void
{
    state_timer_.Cancel();
    ping_timer_.Cancel();
    activity_timer_.Cancel();
    peers_timer_.Cancel();
    job_timer_.Cancel();
}

auto Peer::Imp::check_addresses(allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;

    if (gossip_address_queue_.empty()) {
        log(OT_PRETTY_CLASS())(name_)(": address queue is empty").Flush();

        return;
    }

    process_gossip_address(gossip_address_queue_, monotonic);
    gossip_address_queue_.clear();
}

auto Peer::Imp::check_jobs(allocator_type monotonic) noexcept -> void
{
    const auto& block = block_oracle_.Internal();
    const auto& header = header_oracle_.Internal();
    auto alloc = get_allocator();
    const auto& log = log_;

    if (has_job()) {
        log(OT_PRETTY_CLASS())(name_)(": already have ")(job_name()).Flush();

        return;
    } else if (auto bhJob = header.GetJob(alloc); bhJob) {
        log(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(bhJob)).Flush();
        job_ = std::move(bhJob);
    } else if (auto bJob = block.GetWork(alloc); bJob) {
        log(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(bJob))(" ")(
            bJob.ID())
            .Flush();
        job_ = std::move(bJob);
    }

    if (has_job()) { run_job(monotonic); }
}

auto Peer::Imp::check_positions() noexcept -> void
{
    if (false == is_caught_up_) {
        is_caught_up_ = local_position_ >= remote_position_;

        if (is_caught_up_) {
            log_(OT_PRETTY_CLASS())(name_)(": local tip ")(
                local_position_)(" caught up to remote tip: ")(remote_position_)
                .Flush();
        }
    }
}

auto Peer::Imp::connect(allocator_type monotonic) noexcept -> void
{
    transition_state_connect();
    const auto [connected, endpoint] = connection_.do_connect();

    if (endpoint.has_value()) {
        connect_dealer(endpoint.value(), false, monotonic);
    }

    if (connected) { process_connect(monotonic); }
}

auto Peer::Imp::connect_dealer(
    std::string_view endpoint,
    bool init,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;

    OT_ASSERT(valid(endpoint));

    using enum blockchain::Transport;

    const auto curveClient =
        (address().Type() == zmq) && (false == address().Internal().Incoming());

    if (curveClient) {
        const auto pubkey = [&, this] {
            auto out = CString{monotonic};
            const auto rc =
                api_.Crypto().Encode().Z85Encode(address().Key(), writer(out));

            OT_ASSERT(rc);

            return out;
        }();
        log(OT_PRETTY_CLASS())(name_)(": enabling CurveZMQ for ")(
            endpoint)(" using remote pubkey ")(pubkey)
            .Flush();
        const auto& [sec, pub] = curve_keys_;
        auto rc = external_.EnableCurveClient(
            address().Key(), pub.Bytes(), sec.Bytes());

        OT_ASSERT(rc);

        rc = external_.SetZAPDomain("blockchain");

        OT_ASSERT(rc);
    } else {
        log(OT_PRETTY_CLASS())(name_)(": skipping CurveZMQ for ")(endpoint)
            .Flush();
    }

    log(OT_PRETTY_CLASS())(name_)(": connecting dealer socket to ")(endpoint)
        .Flush();
    const auto ep = CString{endpoint, monotonic};
    external_.Connect(ep.c_str());

    if (init) {
        external_.SendDeferred(connection_.on_init(), __FILE__, __LINE__);
    } else {
        process_connect(true, monotonic);
    }
}

auto Peer::Imp::disconnect(
    std::string_view why,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;
    log(OT_PRETTY_CLASS())("disconnecting ")(name_);

    if (valid(why)) { log(": ")(why); }

    log.Flush();

    using enum State;

    switch (state_) {
        case init:
        case connect:
        case handshake:
        case verify: {
            failed_peer_ = true;
            database_.Fail(address().ID());
        } break;
        case pre_init:
        case run:
        case shutdown:
        default: {
        }
    }

    do_disconnect(monotonic);
    transition_state_shutdown();
    shutdown_actor();
}

auto Peer::Imp::do_disconnect(allocator_type monotonic) noexcept -> void
{
    if (auto m = connection_.stop_external(); m.has_value()) {
        external_.SendDeferred(std::move(*m), __FILE__, __LINE__);
    }

    cancel_timers();
    finish_job(monotonic, true);
    connection_.shutdown_external();

    switch (state_) {
        case State::verify:
        case State::run: {
            update_address();
        } break;
        default: {
        }
    }

    database_.Release(address().ID());
    to_peer_manager_.SendDeferred(
        [&] {
            using enum opentxs::blockchain::node::PeerManagerJobs;
            auto out = MakeWork(disconnect);
            out.AddFrame(id_);
            out.AddFrame(address().Display());

            return out;
        }(),
        __FILE__,
        __LINE__);
}

auto Peer::Imp::do_shutdown() noexcept -> void
{
    do_disconnect({});
    network_p_.reset();
    api_p_.reset();
}

auto Peer::Imp::do_startup(allocator_type monotonic) noexcept -> bool
{
    if (api_.Internal().ShuttingDown() || network_.Internal().ShuttingDown()) {

        return true;
    }

    update_local_position(header_oracle_.BestChain());
    transition_state_init();

    if (const auto endpoint = connection_.do_init(); endpoint.has_value()) {
        connect_dealer(endpoint.value(), true, monotonic);
    } else {
        connect(monotonic);
    }

    return false;
}

auto Peer::Imp::finish_job(allocator_type monotonic, bool shutdown) noexcept
    -> void
{
    job_timer_.Cancel();
    job_ = std::monostate{};

    if (false == shutdown) { check_jobs(monotonic); }
}

auto Peer::Imp::get_known_tx(alloc::Default alloc) const noexcept -> Set<Txid>
{
    auto out = Set<Txid>{alloc};
    std::copy(
        known_transactions_.begin(),
        known_transactions_.end(),
        std::inserter(out, out.end()));

    return out;
}

auto Peer::Imp::has_job() const noexcept -> bool
{
    static const auto visitor = HasJob{};

    return std::visit(visitor, job_);
}

auto Peer::Imp::hash(const blockchain::Address& addr) const noexcept
    -> KnownAddresses::value_type
{
    auto out = KnownAddresses::value_type{};
    static_assert(sizeof(out) == crypto_shorthash_BYTES);
    const auto bytes = addr.Bytes();
    const auto view = bytes.Bytes();
    ::crypto_shorthash(
        reinterpret_cast<unsigned char*>(std::addressof(out)),
        reinterpret_cast<const unsigned char*>(view.data()),
        view.size(),
        reinterpret_cast<const unsigned char*>(siphash_key_.data()));

    return out;
}

auto Peer::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto Peer::Imp::init_connection_manager(
    const api::Session& api,
    const opentxs::blockchain::node::Manager& node,
    const Imp& parent,
    const blockchain::Address& address,
    const Log& log,
    int id,
    std::size_t headerBytes,
    std::optional<asio::Socket> socket) noexcept
    -> std::unique_ptr<ConnectionManager>
{
    if (opentxs::network::blockchain::Transport::zmq == address.Type()) {
        if (address.Internal().Incoming()) {

            return network::blockchain::ConnectionManager::ZMQIncoming(
                api, node, log, id, address, headerBytes);
        } else {

            return network::blockchain::ConnectionManager::ZMQ(
                api, log, id, address, headerBytes);
        }
    } else {
        if (address.Internal().Incoming()) {
            OT_ASSERT(socket);

            return network::blockchain::ConnectionManager::TCPIncoming(
                api,
                log,
                id,
                address,
                headerBytes,
                [&](const auto& h) { return parent.extract_body_size(h); },
                std::move(*socket));
        } else {

            return network::blockchain::ConnectionManager::TCP(
                api, log, id, address, headerBytes, [&](const auto& h) {
                    return parent.extract_body_size(h);
                });
        }
    }
}

auto Peer::Imp::is_allowed_state(Work work) const noexcept -> bool
{
    using enum PeerJob;

    if (shutdown == work) { return true; }

    switch (state_) {
        case State::pre_init: {
            switch (work) {
                case blockheader:
                case reorg:
                case gossip_address:
                case init: {

                    return true;
                }
                default: {

                    return false;
                }
            }
        }
        case State::init: {
            switch (work) {
                case blockheader:
                case reorg:
                case registration:
                case disconnect:
                case gossip_address:
                case statetimeout: {

                    return true;
                }
                default: {

                    return false;
                }
            }
        }
        case State::connect: {
            switch (work) {
                case blockheader:
                case reorg:
                case connect:
                case disconnect:
                case gossip_address:
                case statetimeout: {

                    return true;
                }
                default: {

                    return false;
                }
            }
        }
        case State::handshake:
        case State::verify: {
            switch (work) {
                case blockheader:
                case reorg:
                case disconnect:
                case sendresult:
                case p2p:
                case gossip_address:
                case statetimeout:
                case activitytimeout:
                case needping:
                case body:
                case header: {

                    return true;
                }
                default: {

                    return false;
                }
            }
        }
        case State::run: {
            switch (work) {
                case blockheader:
                case reorg:
                case mempool:
                case disconnect:
                case sendresult:
                case p2p:
                case gossip_address:
                case jobtimeout:
                case needpeers:
                case statetimeout:
                case activitytimeout:
                case needping:
                case body:
                case header:
                case broadcasttx:
                case jobavailablegetheaders:
                case jobavailableblock:
                case block: {

                    return true;
                }
                default: {

                    return false;
                }
            }
        }
        case State::shutdown:
        default: {

            OT_FAIL;
        }
    }
}

auto Peer::Imp::is_known(const blockchain::Address& address) const noexcept
    -> bool
{
    return known_addresses_.contains(hash(address));
}

auto Peer::Imp::job_name() const noexcept -> std::string_view
{
    return std::visit(JobType::get(), job_);
}

auto Peer::Imp::pipeline(
    const Work work,
    zeromq::Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    if (State::shutdown == state_) { return; }

    if (false == is_allowed_state(work)) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(" received ")(print(work))(
            " message in ")(print_state(state_))(" state")
            .Abort();
    }

    if (const auto id = connection_id(msg); id == untrusted_connection_id_) {
        pipeline_untrusted(work, std::move(msg), monotonic);
    } else {
        pipeline_trusted(work, std::move(msg), monotonic);
    }
}

auto Peer::Imp::pipeline_trusted(
    const Work work,
    zeromq::Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::blockheader: {
            process_blockheader(std::move(msg), monotonic);
        } break;
        case Work::reorg: {
            process_reorg(std::move(msg), monotonic);
        } break;
        case Work::mempool: {
            process_mempool(std::move(msg), monotonic);
        } break;
        case Work::connect: {
            process_connect(true, monotonic);
        } break;
        case Work::gossip_address: {
            process_gossip_address(std::move(msg), monotonic);
            do_work(monotonic);
        } break;
        case Work::jobtimeout: {
            process_jobtimeout(std::move(msg), monotonic);
        } break;
        case Work::needpeers: {
            process_needpeers(std::move(msg), monotonic);
        } break;
        case Work::statetimeout: {
            process_statetimeout(std::move(msg), monotonic);
        } break;
        case Work::activitytimeout: {
            process_activitytimeout(std::move(msg), monotonic);
        } break;
        case Work::needping: {
            process_needping(std::move(msg), monotonic);
        } break;
        case Work::broadcasttx: {
            process_broadcasttx(std::move(msg), monotonic);
        } break;
        case Work::jobavailablegetheaders: {
            process_jobavailablegetheaders(std::move(msg), monotonic);
        } break;
        case Work::jobavailableblock: {
            process_jobavailableblock(std::move(msg), monotonic);
        } break;
        case Work::block: {
            process_block(std::move(msg), monotonic);
        } break;
        case Work::shutdown:
        case Work::registration:
        case Work::disconnect:
        case Work::sendresult:
        case Work::p2p:
        case Work::body:
        case Work::header:
        case Work::init:
        case Work::statemachine: {
            unhandled_type(work, "on trusted socket"sv);
        }
        default: {
            unknown_type(work);
        }
    }
}

auto Peer::Imp::pipeline_untrusted(
    const Work work,
    zeromq::Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    if (State::shutdown == state_) {
        shutdown_actor();

        return;
    }

    switch (work) {
        case Work::registration: {
            process_registration(std::move(msg), monotonic);
        } break;
        case Work::connect: {
            process_connect(true, monotonic);
        } break;
        case Work::disconnect: {
            process_disconnect(std::move(msg), monotonic);
        } break;
        case Work::sendresult: {
            process_sendresult(std::move(msg), monotonic);
        } break;
        case Work::p2p: {
            process_p2p(std::move(msg), monotonic);
            do_work(monotonic);
        } break;
        case Work::body: {
            process_body(std::move(msg), monotonic);
            do_work(monotonic);
        } break;
        case Work::header: {
            process_header(std::move(msg), monotonic);
        } break;
        case Work::shutdown:
        case Work::blockheader:
        case Work::reorg:
        case Work::mempool:
        case Work::gossip_address:
        case Work::jobtimeout:
        case Work::needpeers:
        case Work::statetimeout:
        case Work::activitytimeout:
        case Work::needping:
        case Work::broadcasttx:
        case Work::jobavailablegetheaders:
        case Work::jobavailableblock:
        case Work::block:
        case Work::init:
        case Work::statemachine: {
            unhandled_type(work, "on untrusted socket"sv);
        }
        default: {
            const auto why =
                CString{name_, get_allocator()}
                    .append(" sent an internal control message of type "sv)
                    .append(print(work))
                    .append(" instead of a valid protocol message"sv);
            disconnect(why, monotonic);

            return;
        }
    }
}

auto Peer::Imp::print_state(State state) noexcept -> std::string_view
{
    try {
        static const auto map = Map<State, std::string_view>{
            {State::pre_init, "pre_init"sv},
            {State::init, "init"sv},
            {State::connect, "connect"sv},
            {State::handshake, "handshake"sv},
            {State::verify, "verify"sv},
            {State::run, "run"sv},
            {State::shutdown, "shutdown"sv},
        };

        return map.at(state);
    } catch (...) {
        LogError()(OT_PRETTY_STATIC(Imp))("invalid State: ")(
            static_cast<int>(state))
            .Flush();

        OT_FAIL;
    }
}

auto Peer::Imp::process_activitytimeout(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    disconnect("activity timeout"sv, monotonic);
}

auto Peer::Imp::process_block(Message&& msg, allocator_type monotonic) noexcept
    -> void
{
    const auto body = msg.Payload();

    OT_ASSERT(2 < body.size());

    process_block(opentxs::blockchain::block::Hash{body[2].Bytes()}, monotonic);
}

auto Peer::Imp::process_block(
    opentxs::blockchain::block::Hash&& hash,
    allocator_type monotonic) noexcept -> void
{
    if (State::run != state_) { return; }

    log_(OT_PRETTY_CLASS())(name_)(": received block oracle update message")
        .Flush();

    if (false == is_caught_up_) {
        log_(OT_PRETTY_CLASS())(name_)(
            ": ignoring block oracle update message until local block header "
            "chain is synchronized with remote")
            .Flush();

        return;
    }

    if (auto h = header_oracle_.LoadHeader(hash); false == h.IsValid()) {
        log_(OT_PRETTY_CLASS())(name_)(": block ")
            .asHex(hash)(" can not be loaded")
            .Flush();

        return;
    }

    if (0_uz == known_blocks_.count(hash)) {
        log_(OT_PRETTY_CLASS())(name_)(
            ": remote peer does not know about block ")
            .asHex(hash)
            .Flush();
        transmit_block_hash(std::move(hash), monotonic);
    }
}

auto Peer::Imp::process_blockheader(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto body = msg.Payload();

    OT_ASSERT(3 < body.size());

    if (body[1].as<decltype(chain_)>() != chain_) { return; }

    using Height = opentxs::blockchain::block::Height;
    auto hash = opentxs::blockchain::block::Hash{body[2].Bytes()};
    update_local_position({body[3].as<Height>(), hash});
    process_block(std::move(hash), monotonic);
}

auto Peer::Imp::process_body(Message&& msg, allocator_type monotonic) noexcept
    -> void
{
    update_activity();
    auto m = connection_.on_body(std::move(msg));

    if (m.has_value()) { process_protocol(std::move(m.value()), monotonic); }
}

auto Peer::Imp::process_connect(allocator_type monotonic) noexcept -> void
{
    if (is_allowed_state(Work::connect)) {
        process_connect(true, monotonic);
    } else {

        OT_FAIL;
    }
}

auto Peer::Imp::process_connect(bool, allocator_type monotonic) noexcept -> void
{
    log_(name_)(" connected").Flush();
    connection_.on_connect();
    transition_state_handshake(monotonic);
}

auto Peer::Imp::process_disconnect(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto body = msg.Payload();
    const auto why = [&]() -> std::string_view {
        if (2 < body.size()) {

            return body[2].Bytes();
        } else {

            return "received disconnect message"sv;
        }
    }();
    disconnect(why, monotonic);
}

auto Peer::Imp::process_gossip_address(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;
    const auto payload = msg.Payload();

    if (payload.empty()) { return; }

    const auto frames = payload.subspan(1_uz);
    auto& out = gossip_address_queue_;
    out.reserve(out.size() + payload.size());
    std::transform(
        frames.begin(),
        frames.end(),
        std::back_inserter(out),
        [this](const auto& frame) {
            return api_.Factory().InternalSession().BlockchainAddress(
                proto::Factory<proto::BlockchainPeerAddress>(frame));
        });
    log(OT_PRETTY_CLASS())(name_)(": address queue contains ")(out.size())(
        " items")
        .Flush();
}

auto Peer::Imp::process_gossip_address(
    std::span<network::blockchain::Address> addresses,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;
    auto out = Vector<network::blockchain::Address>{monotonic};
    out.reserve(addresses.size());
    out.clear();
    std::copy_if(
        std::make_move_iterator(addresses.begin()),
        std::make_move_iterator(addresses.end()),
        std::back_inserter(out),
        [this](const auto& addr) { return false == is_known(addr); });
    log(OT_PRETTY_CLASS())(name_)(": ")(out.size())(" of ")(addresses.size())(
        " received addresses are not previously seen by this peer")
        .Flush();

    if (out.empty()) { return; }

    transmit_addresses(out, monotonic);
}

auto Peer::Imp::process_header(Message&& msg, allocator_type monotonic) noexcept
    -> void
{
    update_activity();
    auto m = connection_.on_header(std::move(msg));

    if (m.has_value()) { process_protocol(std::move(m.value()), monotonic); }
}

auto Peer::Imp::process_jobavailableblock(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;

    if (has_job()) {
        log(OT_PRETTY_CLASS())(name_)(": already have ")(job_name()).Flush();

        return;
    }

    auto job = block_oracle_.Internal().GetWork(get_allocator());

    if (0_uz < job.Remaining()) {
        log(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(job))(" ")(
            job.ID())
            .Flush();
        job_ = std::move(job);
        run_job(monotonic);
    } else {
        log(OT_PRETTY_CLASS())(name_)(": job already accepted by another peer")
            .Flush();
    }
}

auto Peer::Imp::process_jobavailablegetheaders(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    if (has_job()) {
        log_(OT_PRETTY_CLASS())(name_)(": already have ")(job_name()).Flush();

        return;
    }

    auto job = header_oracle_.Internal().GetJob(get_allocator());

    if (job.operator bool()) {
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(job))(" ")
            .Flush();
        job_ = std::move(job);
        run_job(monotonic);
    } else {
        log_(OT_PRETTY_CLASS())(name_)(": job already accepted by another peer")
            .Flush();
    }
}

auto Peer::Imp::process_jobtimeout(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;
    log(OT_PRETTY_CLASS())(name_)(": cancelling ")(job_name())(" due to ")(
        std::chrono::duration_cast<std::chrono::nanoseconds>(job_timeout_))(
        " of inactivity")
        .Flush();
    finish_job(monotonic);
}

auto Peer::Imp::process_mempool(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto body = msg.Payload();

    OT_ASSERT(1 < body.size());

    if (body[1].as<decltype(chain_)>() != chain_) { return; }

    const auto txid = Txid{body[2].Bytes()};
    const auto isNew = add_known_tx(txid);

    if (isNew) { transmit_txid(txid, monotonic); }
}

auto Peer::Imp::process_needpeers(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    transmit_request_peers(monotonic);

    if (0s < peers_interval_) { reset_peers_timer(); }
}

auto Peer::Imp::process_needping(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    transmit_ping(monotonic);
}

auto Peer::Imp::process_p2p(Message&& msg, allocator_type monotonic) noexcept
    -> void
{
    update_activity();
    process_protocol(std::move(msg), monotonic);
}

auto Peer::Imp::process_registration(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    connection_.on_register(std::move(msg));
    connect(monotonic);
}

auto Peer::Imp::process_reorg(Message&& msg, allocator_type monotonic) noexcept
    -> void
{
    const auto body = msg.Payload();

    OT_ASSERT(5 < body.size());

    if (body[1].as<decltype(chain_)>() != chain_) { return; }

    using Height = opentxs::blockchain::block::Height;
    using Hash = opentxs::blockchain::block::Hash;
    using Position = opentxs::blockchain::block::Position;
    const auto parent = Position{body[3].as<Height>(), body[2].Bytes()};
    auto hash = Hash{body[4].Bytes()};
    auto tip = Position{body[5].as<Height>(), hash};
    auto positions = header_oracle_.Ancestors(parent, tip);
    update_local_position(std::move(tip));

    for (auto& position : positions) {
        process_block(std::move(position.hash_), monotonic);
    }
}

auto Peer::Imp::process_sendresult(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto body = msg.Payload();

    OT_ASSERT(2 < body.size());

    static constexpr auto error = std::byte{0x00};

    if (error == body[2].as<std::byte>()) {
        const auto why = [&] {
            if (3 < body.size()) {

                return body[3].Bytes();
            } else {

                return "unspecified send error"sv;
            }
        }();
        disconnect(why, monotonic);
    }
}

auto Peer::Imp::process_statetimeout(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto why = CString{name_}
                         .append(" failed to transition out of state "sv)
                         .append(print_state(state_));
    disconnect(why, monotonic);
}

auto Peer::Imp::reset_activity_timer() noexcept -> void
{
    reset_timer(inactivity_interval_, activity_timer_, Work::activitytimeout);
}

auto Peer::Imp::reset_job_timer() noexcept -> void
{
    if (has_job()) { reset_timer(job_timeout_, job_timer_, Work::jobtimeout); }
}

auto Peer::Imp::reset_peers_timer() noexcept -> void
{
    reset_peers_timer(peers_interval_);
}

auto Peer::Imp::reset_peers_timer(std::chrono::microseconds value) noexcept
    -> void
{
    reset_timer(value, peers_timer_, Work::needpeers);
}

auto Peer::Imp::reset_ping_timer() noexcept -> void
{
    reset_timer(ping_interval_, ping_timer_, Work::needping);
}

auto Peer::Imp::reset_state_timer(std::chrono::microseconds value) noexcept
    -> void
{
    reset_timer(value, state_timer_, Work::statetimeout);
}

auto Peer::Imp::run_job(allocator_type monotonic) noexcept -> void
{
    OT_ASSERT(has_job());

    auto visitor = RunJob{*this, monotonic};
    std::visit(visitor, job_);
    reset_job_timer();
}

auto Peer::Imp::send_good_addresses(allocator_type monotonic) noexcept -> void
{
    auto good = database_.Good(get_allocator(), monotonic);
    std::move(
        std::begin(good),
        std::end(good),
        std::back_inserter(gossip_address_queue_));
}

auto Peer::Imp::set_block_header_capability(bool value) noexcept -> void
{
    block_header_capability_ = value;
}

auto Peer::Imp::set_cfilter_capability(bool value) noexcept -> void
{
    cfilter_capability_ = value;
}

auto Peer::Imp::transition_state(
    State state,
    std::optional<std::chrono::microseconds> timeout) noexcept -> void
{
    const auto& log = log_;
    state_timer_.Cancel();
    state_ = state;
    log(OT_PRETTY_CLASS())(name_)(": transitioned to ")(print_state(state))(
        " state")
        .Flush();

    if (timeout.has_value()) { reset_state_timer(timeout.value()); }
}

auto Peer::Imp::transition_state_connect() noexcept -> void
{
    transition_state(State::connect, 30s);
}

auto Peer::Imp::transition_state_init() noexcept -> void
{
    transition_state(State::init, 10s);
}

auto Peer::Imp::transition_state_handshake(allocator_type) noexcept -> void
{
    transition_state(State::handshake, 30s);
}

auto Peer::Imp::transition_state_run(allocator_type monotonic) noexcept -> void
{
    const auto [network, limited, cfilter, bloom] = [&] {
        using Service = opentxs::network::blockchain::bitcoin::Service;
        const auto services = address().Services();
        auto net = (services.contains(Service::Network));
        auto limit = (services.contains(Service::Limited));
        auto filter = (services.contains(Service::CompactFilters));
        auto blm = (services.contains(Service::Bloom));

        return std::make_tuple(net, limit, filter, blm);
    }();
    auto& pipeline = pipeline_.Internal();
    pipeline.SubscribeFromThread(api_.Endpoints().BlockchainMempool());

    if (network || limited || block_header_capability_) {
        pipeline.PullFromThread(
            network_.Internal().Endpoints().peer_manager_push_);
        pipeline.SubscribeFromThread(
            network_.Internal().Endpoints().header_oracle_job_ready_);
        pipeline.SubscribeFromThread(
            network_.Internal().Endpoints().block_oracle_publish_);
    }

    if (BlockchainProfile::server == config_.profile_) {
        pipeline.SubscribeFromThread(
            network_.Internal().Endpoints().block_tip_publish_);
    }

    database_.Confirm(address().ID());
    transition_state(State::run);
    to_peer_manager_.SendDeferred(
        [&] {
            using enum opentxs::blockchain::node::PeerManagerJobs;
            auto out = MakeWork(verifypeer);
            out.AddFrame(id_);
            out.AddFrame(address().Display());

            return out;
        }(),
        __FILE__,
        __LINE__);
    reset_peers_timer(0s);

    if (bloom) { transmit_request_mempool(monotonic); }

    transmit_request_block_headers(monotonic);
    do_work(monotonic);
}

auto Peer::Imp::transition_state_shutdown() noexcept -> void
{
    transition_state(State::shutdown);
}

auto Peer::Imp::transition_state_verify(allocator_type) noexcept -> void
{
    transition_state(State::verify, 60s);
}

auto Peer::Imp::transmit(
    const bitcoin::message::internal::Message& msg) noexcept(false) -> void
{
    switch (state_) {
        case State::handshake:
        case State::verify:
        case State::run: {
        } break;
        case State::pre_init:
        case State::init:
        case State::connect:
        case State::shutdown:
        default: {
            LogAbort()(OT_PRETTY_CLASS())("attempting to transmit in state ")(
                print_state(state_))
                .Abort();
        }
    }

    transmit([&] {
        auto out = connection_.send();
        msg.Transmit(address().Type(), out);

        return out;
    }());
}

auto Peer::Imp::transmit(Message&& message) noexcept -> void
{
    if (auto m = connection_.transmit(std::move(message)); m.has_value()) {
        external_.SendDeferred(std::move(*m), __FILE__, __LINE__);
    }
}

auto Peer::Imp::update_activity() noexcept -> void
{
    last_activity_ = Clock::now();

    if (State::run == state_) { reset_ping_timer(); }

    reset_activity_timer();
}

auto Peer::Imp::update_address() noexcept -> void
{
    if (false == failed_peer_) {
        remote_address_.Internal().SetLastConnected(last_activity_);
        database_.AddOrUpdate({remote_address_});
    }
}

auto Peer::Imp::update_address(
    Set<opentxs::network::blockchain::bitcoin::Service> services) noexcept
    -> void
{
    remote_address_.Internal().SetServices(services);
    database_.AddOrUpdate({remote_address_});
}

auto Peer::Imp::update_block_job(
    const ReadView block,
    allocator_type monotonic) noexcept -> bool
{
    auto visitor = UpdateBlockJob{block};

    return update_job(visitor, monotonic);
}

auto Peer::Imp::update_get_headers_job(allocator_type monotonic) noexcept
    -> void
{
    static const auto visitor = UpdateGetHeadersJob{};
    update_job(visitor, monotonic);
}

auto Peer::Imp::update_local_position(
    opentxs::blockchain::block::Position pos) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(": local position updated to ")(pos).Flush();
    update_position(local_position_, std::move(pos));
}

auto Peer::Imp::update_position(
    opentxs::blockchain::block::Position& target,
    opentxs::blockchain::block::Position pos) noexcept -> void
{
    target = std::move(pos);
    check_positions();
}

auto Peer::Imp::update_remote_position(
    opentxs::blockchain::block::Position pos) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(": remote position updated to ")(pos)
        .Flush();
    update_position(remote_position_, std::move(pos));
}

auto Peer::Imp::work(allocator_type monotonic) noexcept -> bool
{
    switch (state_) {
        case State::run: {
            check_addresses(monotonic);
            check_jobs(monotonic);
        } break;
        default: {
        }
    }

    return false;
}

Peer::Imp::~Imp() = default;
}  // namespace opentxs::network::blockchain::internal
