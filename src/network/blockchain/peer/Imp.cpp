// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "network/blockchain/peer/Imp.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include "blockchain/DownloadTask.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Peer.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/PeerManager.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/network/blockchain/ConnectionManager.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/peer/HasJob.hpp"
#include "network/blockchain/peer/JobType.hpp"
#include "network/blockchain/peer/RunJob.hpp"
#include "network/blockchain/peer/UpdateBlockJob.hpp"
#include "network/blockchain/peer/UpdateCfheaderJob.hpp"
#include "network/blockchain/peer/UpdateCfilterJob.hpp"
#include "network/blockchain/peer/UpdateGetHeadersJob.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::network::blockchain
{
auto print(PeerJob job) noexcept -> std::string_view
{
    using namespace std::literals;

    try {
        using Job = PeerJob;
        static const auto map = Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::blockheader, "blockheader"sv},
            {Job::reorg, "reorg"sv},
            {Job::blockbatch, "blockbatch"sv},
            {Job::mempool, "mempool"sv},
            {Job::registration, "registration"sv},
            {Job::connect, "connect"sv},
            {Job::disconnect, "disconnect"sv},
            {Job::sendresult, "sendresult"sv},
            {Job::p2p, "p2p"sv},
            {Job::getheaders, "getheaders"sv},
            {Job::broadcasttx, "broadcasttx"sv},
            {Job::jobavailablecfheaders, "jobavailablecfheaders"sv},
            {Job::jobavailablecfilters, "jobavailablecfilters"sv},
            {Job::dealerconnected, "dealerconnected"sv},
            {Job::jobtimeout, "jobtimeout"sv},
            {Job::needpeers, "needpeers"sv},
            {Job::statetimeout, "statetimeout"sv},
            {Job::activitytimeout, "activitytimeout"sv},
            {Job::needping, "needping"sv},
            {Job::body, "body"sv},
            {Job::header, "header"sv},
            {Job::jobavailableblock, "jobavailableblock"sv},
            {Job::jobavailableblockbatch, "jobavailableblockbatch"sv},
            {Job::heartbeat, "heartbeat"sv},
            {Job::block, "block"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid PeerJob: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
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
auto Peer::Imp::update_job(Visitor& visitor) noexcept -> bool
{
    const auto [isJob, isFinished] = std::visit(visitor, job_);

    if (isJob) {
        if (isFinished) {
            finish_job();
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
    opentxs::blockchain::Type chain,
    int peerID,
    std::unique_ptr<opentxs::blockchain::p2p::internal::Address> address,
    std::chrono::milliseconds pingInterval,
    std::chrono::milliseconds inactivityInterval,
    std::chrono::milliseconds peersInterval,
    std::size_t headerBytes,
    const opentxs::blockchain::node::Endpoints& endpoints,
    std::string_view fromParent,
    zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Actor(
          *api,
          LogTrace(),
          [&] {
              using opentxs::blockchain::print;
              auto out = CString{print(chain), alloc};

              OT_ASSERT(address);

              if (address->Incoming()) {
                  out.append(" incoming"sv);
              } else {
                  out.append(" outgoing"sv);
              }

              out.append(" peer "sv);
              out.append(address->Display());
              out.append(" ("sv);
              out.append(std::to_string(peerID));
              out.append(")"sv);

              return out;
          }(),
          0ms,
          batch,
          alloc,
          [&] {
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(api->Endpoints().Shutdown(), Direction::Connect);
              sub.emplace_back(
                  network->Internal().Endpoints().shutdown_publish_,
                  Direction::Connect);
              sub.emplace_back(
                  api->Endpoints().BlockchainReorg(), Direction::Connect);

              return sub;
          }(),
          [&] {
              auto pull = network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(fromParent, Direction::Connect);

              return pull;
          }(),
          {},
          [&] {
              auto out = Vector<network::zeromq::SocketData>{alloc};
              out.emplace_back(SocketType::Push, [&] {
                  auto extra = Vector<network::zeromq::EndpointArg>{alloc};
                  extra.emplace_back(
                      network->Internal().Endpoints().block_cache_pull_,
                      Direction::Connect);

                  return extra;
              }());

              return out;
          }())
    , api_p_(api)
    , network_p_(network)
    , api_(*api_p_)
    , network_(*network_p_)
    , parent_(network_.Internal().PeerManager())
    , config_(network_.Internal().GetConfig())
    , header_oracle_(network_.HeaderOracle())
    , block_oracle_(network_.BlockOracle())
    , filter_oracle_(network_.FilterOracle())
    , chain_(chain)
    , dir_([&] {
        if (address->Incoming()) {

            return Dir::incoming;
        } else {

            return Dir::outgoing;
        }
    }())
    , database_(network_.Internal().DB())
    , to_block_cache_(pipeline_.Internal().ExtraSocket(0))
    , id_(peerID)
    , untrusted_connection_id_(pipeline_.ConnectionIDDealer())
    , ping_interval_(std::move(pingInterval))
    , inactivity_interval_(std::move(inactivityInterval))
    , peers_interval_(std::move(peersInterval))
    , address_p_(std::move(address))
    , address_(*address_p_)
    , connection_p_(init_connection_manager(
          api_,
          *this,
          parent_,
          address_,
          log_,
          id_,
          headerBytes))
    , connection_(*connection_p_)
    , state_(State::pre_init)
    , last_activity_()
    , state_timer_(api_.Network().Asio().Internal().GetTimer())
    , ping_timer_(api_.Network().Asio().Internal().GetTimer())
    , activity_timer_(api_.Network().Asio().Internal().GetTimer())
    , peers_timer_(api_.Network().Asio().Internal().GetTimer())
    , job_timer_(api_.Network().Asio().Internal().GetTimer())
    , known_transactions_()
    , known_blocks_()
    , local_position_()
    , remote_position_()
    , job_()
    , is_caught_up_(false)
    , block_header_capability_(false)
    , cfilter_capability_(false)
{
    OT_ASSERT(api_p_);
    OT_ASSERT(network_p_);
    OT_ASSERT(connection_p_);
    OT_ASSERT(address_p_);
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

auto Peer::Imp::check_jobs() noexcept -> void
{
    const auto& filter = filter_oracle_.Internal();
    const auto& block = block_oracle_.Internal();

    if (has_job()) {

        return;
    } else if (auto hJob = filter.GetHeaderJob(); hJob) {
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(hJob))(" ")(
            hJob.id_)
            .Flush();
        job_ = std::move(hJob);
    } else if (auto fJob = filter.GetFilterJob(); fJob) {
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(fJob))(" ")(
            fJob.id_)
            .Flush();
        job_ = std::move(fJob);
    } else if (auto bBatch = block.GetBlockBatch(get_allocator()); bBatch) {
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(bBatch))(" ")(
            bBatch.ID())
            .Flush();
        job_ = std::move(bBatch);
    } else if (auto bJob = block.GetBlockJob(get_allocator()); bBatch) {
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(bJob))(" ")(
            bJob.ID())
            .Flush();
        job_ = std::move(bJob);
    } else if (false == network_.Internal().IsSynchronized()) {
        job_ = GetHeadersJob{};
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name()).Flush();
    }

    if (has_job()) { run_job(); }
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

auto Peer::Imp::connect() noexcept -> void
{
    transition_state_connect();
    const auto [connected, endpoint] = connection_.do_connect();

    if (endpoint.has_value()) {
        connect_dealer(endpoint.value(), Work::connect);
    }

    if (connected) { process_connect(); }
}

auto Peer::Imp::connect_dealer(std::string_view endpoint, Work work) noexcept
    -> void
{
    OT_ASSERT(valid(endpoint));

    log_(OT_PRETTY_CLASS())(name_)(": connecting dealer socket to ")(endpoint)
        .Flush();
    pipeline_.ConnectDealer(endpoint, [id = id_, work](auto) {
        const zeromq::SocketID header = id;
        auto out = zeromq::Message{};
        out.AddFrame(header);
        out.StartBody();
        out.AddFrame(work);

        return out;
    });
}

auto Peer::Imp::disconnect(std::string_view why) noexcept -> void
{
    log_(OT_PRETTY_CLASS())("disconnecting ")(name_);

    if (valid(why)) { log_(": ")(why); }

    log_.Flush();
    do_disconnect();
    transition_state_shutdown();
    shutdown_actor();
}

auto Peer::Imp::do_disconnect() noexcept -> void
{
    connection_.stop_external();
    cancel_timers();
    finish_job(true);
    connection_.shutdown_external();
    parent_.Disconnect(id_);

    switch (state_) {
        case State::verify:
        case State::run: {
            update_address();
        } break;
        default: {
        }
    }
}

auto Peer::Imp::do_shutdown() noexcept -> void
{
    do_disconnect();
    network_p_.reset();
    api_p_.reset();
}

auto Peer::Imp::do_startup() noexcept -> bool
{
    if (api_.Internal().ShuttingDown() || network_.Internal().ShuttingDown()) {

        return true;
    }

    update_local_position(header_oracle_.BestChain());
    transition_state_init();

    if (const auto endpoint = connection_.do_init(); endpoint.has_value()) {
        connect_dealer(endpoint.value(), Work::dealerconnected);
    } else {
        connect();
    }

    return false;
}

auto Peer::Imp::finish_job(bool shutdown) noexcept -> void
{
    job_timer_.Cancel();
    job_ = std::monostate{};

    if (false == shutdown) { check_jobs(); }
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

auto Peer::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto Peer::Imp::init_connection_manager(
    const api::Session& api,
    const Imp& parent,
    const opentxs::blockchain::node::internal::PeerManager& manager,
    const opentxs::blockchain::p2p::internal::Address& address,
    const Log& log,
    int id,
    std::size_t headerBytes) noexcept -> std::unique_ptr<ConnectionManager>
{
    if (opentxs::blockchain::p2p::Network::zmq == address.Type()) {
        if (address.Incoming()) {

            return network::blockchain::ConnectionManager::ZMQIncoming(
                api, log, id, address, headerBytes);
        } else {

            return network::blockchain::ConnectionManager::ZMQ(
                api, log, id, address, headerBytes);
        }
    } else {
        if (address.Incoming()) {

            return network::blockchain::ConnectionManager::TCPIncoming(
                api,
                log,
                id,
                address,
                headerBytes,
                [&](const auto& h) { return parent.extract_body_size(h); },
                manager.LookupIncomingSocket(id));
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
    if (Work::shutdown == work) { return true; }

    switch (state_) {
        case State::pre_init: {
            switch (work) {
                case Work::blockheader:
                case Work::reorg:
                case Work::init: {

                    return true;
                }
                default: {

                    return false;
                }
            }
        }
        case State::init: {
            switch (work) {
                case Work::blockheader:
                case Work::reorg:
                case Work::registration:
                case Work::disconnect:
                case Work::dealerconnected:
                case Work::statetimeout: {

                    return true;
                }
                default: {

                    return false;
                }
            }
        }
        case State::connect: {
            switch (work) {
                case Work::blockheader:
                case Work::reorg:
                case Work::connect:
                case Work::disconnect:
                case Work::statetimeout: {

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
                case Work::blockheader:
                case Work::reorg:
                case Work::disconnect:
                case Work::sendresult:
                case Work::p2p:
                case Work::statetimeout:
                case Work::activitytimeout:
                case Work::needping:
                case Work::body:
                case Work::header: {

                    return true;
                }
                default: {

                    return false;
                }
            }
        }
        case State::run: {
            switch (work) {
                case Work::blockheader:
                case Work::reorg:
                case Work::mempool:
                case Work::blockbatch:
                case Work::disconnect:
                case Work::sendresult:
                case Work::p2p:
                case Work::getheaders:
                case Work::broadcasttx:
                case Work::jobavailablecfheaders:
                case Work::jobavailablecfilters:
                case Work::jobtimeout:
                case Work::needpeers:
                case Work::statetimeout:
                case Work::activitytimeout:
                case Work::needping:
                case Work::body:
                case Work::header:
                case Work::jobavailableblock:
                case Work::jobavailableblockbatch:
                case Work::heartbeat:
                case Work::block: {

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

auto Peer::Imp::job_name() const noexcept -> std::string_view
{
    return std::visit(JobType::get(), job_);
}

auto Peer::Imp::pipeline(const Work work, zeromq::Message&& msg) noexcept
    -> void
{
    if (State::shutdown == state_) { return; }

    const auto connectionID = [&] {
        const auto header = msg.Header();

        OT_ASSERT(0 < header.size());

        return header.at(0).as<std::size_t>();
    }();

    if (false == is_allowed_state(work)) {
        LogError()(OT_PRETTY_CLASS())(name_)(" received ")(print(work))(
            " message in ")(print_state(state_))(" state")
            .Flush();

        OT_FAIL;
    }

    if (connectionID == untrusted_connection_id_) {
        pipeline_untrusted(work, std::move(msg));
    } else {
        pipeline_trusted(work, std::move(msg));
    }
}

auto Peer::Imp::pipeline_trusted(
    const Work work,
    zeromq::Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown: {
            shutdown_actor();
        } break;
        case Work::blockheader: {
            process_blockheader(std::move(msg));
        } break;
        case Work::reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::blockbatch: {
            process_blockbatch(std::move(msg));
        } break;
        case Work::mempool: {
            process_mempool(std::move(msg));
        } break;
        case Work::connect: {
            process_connect(true);
        } break;
        case Work::getheaders: {
            process_getheaders(std::move(msg));
        } break;
        case Work::broadcasttx: {
            process_broadcasttx(std::move(msg));
        } break;
        case Work::jobavailablecfheaders: {
            process_jobavailablecfheaders(std::move(msg));
        } break;
        case Work::jobavailablecfilters: {
            process_jobavailablecfilters(std::move(msg));
        } break;
        case Work::dealerconnected: {
            process_dealerconnected(std::move(msg));
        } break;
        case Work::jobtimeout: {
            process_jobtimeout(std::move(msg));
        } break;
        case Work::needpeers: {
            process_needpeers(std::move(msg));
        } break;
        case Work::statetimeout: {
            process_statetimeout(std::move(msg));
        } break;
        case Work::activitytimeout: {
            process_activitytimeout(std::move(msg));
        } break;
        case Work::needping: {
            process_needping(std::move(msg));
        } break;
        case Work::block: {
            process_block(std::move(msg));
        } break;
        case Work::jobavailableblock: {
            process_jobavailableblock(std::move(msg));
        } break;
        case Work::jobavailableblockbatch: {
            process_jobavailableblockbatch(std::move(msg));
        } break;
        case Work::init: {
            do_init();
        } break;
        case Work::heartbeat:
        case Work::statemachine: {
            do_work();
        } break;
        case Work::registration:
        case Work::disconnect:
        case Work::sendresult:
        case Work::p2p:
        case Work::body:
        case Work::header:
        default: {
            LogError()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Peer::Imp::pipeline_untrusted(
    const Work work,
    zeromq::Message&& msg) noexcept -> void
{
    if (State::shutdown == state_) {
        shutdown_actor();

        return;
    }

    switch (work) {
        case Work::registration: {
            process_registration(std::move(msg));
        } break;
        case Work::connect: {
            process_connect(true);
        } break;
        case Work::disconnect: {
            process_disconnect(std::move(msg));
        } break;
        case Work::sendresult: {
            process_sendresult(std::move(msg));
        } break;
        case Work::p2p: {
            process_p2p(std::move(msg));
        } break;
        case Work::body: {
            process_body(std::move(msg));
        } break;
        case Work::header: {
            process_header(std::move(msg));
        } break;
        default: {
            const auto why =
                CString{name_, get_allocator()}
                    .append(" sent an internal control message of type "sv)
                    .append(print(work))
                    .append(" instead of a valid protocol message"sv);
            disconnect(why);

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

auto Peer::Imp::process_activitytimeout(Message&& msg) noexcept -> void
{
    disconnect("activity timeout"sv);
}

auto Peer::Imp::process_block(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(2 < body.size());

    process_block(opentxs::blockchain::block::Hash{body.at(2).Bytes()});
}

auto Peer::Imp::process_block(opentxs::blockchain::block::Hash&& hash) noexcept
    -> void
{
    log_(OT_PRETTY_CLASS())(name_)(": received block oracle update message")
        .Flush();

    if (false == is_caught_up_) {
        log_(OT_PRETTY_CLASS())(name_)(
            ": ignoring block oracle update message until local block header "
            "chain is synchronized with remote")
            .Flush();

        return;
    }

    if (auto h = header_oracle_.LoadHeader(hash); false == h.operator bool()) {
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
        transmit_block_hash(std::move(hash));
    }
}

auto Peer::Imp::process_blockbatch(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(1 < body.size());

    if (body.at(1).as<decltype(chain_)>() != chain_) { return; }

    check_jobs();
}

auto Peer::Imp::process_blockheader(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(3 < body.size());

    if (body.at(1).as<decltype(chain_)>() != chain_) { return; }

    using Height = opentxs::blockchain::block::Height;
    auto hash = opentxs::blockchain::block::Hash{body.at(2).Bytes()};
    update_local_position({body.at(3).as<Height>(), hash});
    process_block(std::move(hash));
}

auto Peer::Imp::process_body(Message&& msg) noexcept -> void
{
    update_activity();
    auto m = connection_.on_body(std::move(msg));

    if (m.has_value()) { process_protocol(std::move(m.value())); }
}

auto Peer::Imp::process_connect() noexcept -> void
{
    if (is_allowed_state(Work::connect)) {
        process_connect(true);
    } else {

        OT_FAIL;
    }
}

auto Peer::Imp::process_connect(bool) noexcept -> void
{
    log_(name_)(" connected").Flush();
    connection_.on_connect();
    transition_state_handshake();
}

auto Peer::Imp::process_dealerconnected(Message&& msg) noexcept -> void
{
    pipeline_.Send(connection_.on_init());
}

auto Peer::Imp::process_disconnect(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();
    const auto why = [&]() -> std::string_view {
        if (2 < body.size()) {

            return body.at(2).Bytes();
        } else {

            return "received disconnect message"sv;
        }
    }();
    disconnect(why);
}

auto Peer::Imp::process_getheaders(Message&& msg) noexcept -> void
{
    if (has_job()) {
        log_(OT_PRETTY_CLASS())(name_)(": already have ")(job_name()).Flush();

        return;
    } else {
        job_ = GetHeadersJob{};
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name()).Flush();
        run_job();
    }
}

auto Peer::Imp::process_header(Message&& msg) noexcept -> void
{
    update_activity();
    auto m = connection_.on_header(std::move(msg));

    if (m.has_value()) { process_protocol(std::move(m.value())); }
}

auto Peer::Imp::process_jobavailableblock(Message&& msg) noexcept -> void
{
    if (has_job()) {
        log_(OT_PRETTY_CLASS())(name_)(": already have ")(job_name()).Flush();

        return;
    }

    auto job = block_oracle_.Internal().GetBlockJob(get_allocator());

    if (0_uz < job.Remaining()) {
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(job))(" ")(
            job.ID())
            .Flush();
        job_ = std::move(job);
        run_job();
    } else {
        log_(OT_PRETTY_CLASS())(name_)(": job already accepted by another peer")
            .Flush();
    }
}

auto Peer::Imp::process_jobavailableblockbatch(Message&& msg) noexcept -> void
{
    if (has_job()) {
        log_(OT_PRETTY_CLASS())(name_)(": already have ")(job_name()).Flush();

        return;
    }

    auto job = block_oracle_.Internal().GetBlockBatch(get_allocator());

    if (0_uz < job.Remaining()) {
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(job))(" ")(
            job.ID())
            .Flush();
        job_ = std::move(job);
        run_job();
    } else {
        log_(OT_PRETTY_CLASS())(name_)(": job already accepted by another peer")
            .Flush();
    }
}

auto Peer::Imp::process_jobavailablecfheaders(Message&& msg) noexcept -> void
{
    if (has_job()) {
        log_(OT_PRETTY_CLASS())(name_)(": already have ")(job_name()).Flush();

        return;
    }

    auto job = filter_oracle_.Internal().GetHeaderJob();

    if (job.operator bool()) {
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(job))(" ")(
            job.id_)
            .Flush();
        job_ = std::move(job);
        run_job();
    } else {
        log_(OT_PRETTY_CLASS())(name_)(": job already accepted by another peer")
            .Flush();
    }
}

auto Peer::Imp::process_jobavailablecfilters(Message&& msg) noexcept -> void
{
    if (has_job()) {
        log_(OT_PRETTY_CLASS())(name_)(": already have ")(job_name()).Flush();

        return;
    }

    auto job = filter_oracle_.Internal().GetFilterJob();

    if (job.operator bool()) {
        log_(OT_PRETTY_CLASS())(name_)(": accepted ")(job_name(job))(" ")(
            job.id_)
            .Flush();
        job_ = std::move(job);
        run_job();
    } else {
        log_(OT_PRETTY_CLASS())(name_)(": job already accepted by another peer")
            .Flush();
    }
}

auto Peer::Imp::process_jobtimeout(Message&& msg) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(": cancelling ")(job_name())(" due to ")(
        std::chrono::duration_cast<std::chrono::nanoseconds>(job_timeout_))(
        " of inactivity")
        .Flush();
    finish_job();
}

auto Peer::Imp::process_mempool(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(1 < body.size());

    if (body.at(1).as<decltype(chain_)>() != chain_) { return; }

    const auto txid = Txid{body.at(2).Bytes()};
    const auto isNew = add_known_tx(txid);

    if (isNew) { transmit_txid(txid); }
}

auto Peer::Imp::process_needpeers(Message&& msg) noexcept -> void
{
    transmit_request_peers();

    if (0s < peers_interval_) { reset_peers_timer(); }
}

auto Peer::Imp::process_needping(Message&& msg) noexcept -> void
{
    transmit_ping();
}

auto Peer::Imp::process_p2p(Message&& msg) noexcept -> void
{
    update_activity();
    process_protocol(std::move(msg));
}

auto Peer::Imp::process_registration(Message&& msg) noexcept -> void
{
    connection_.on_register(std::move(msg));
    connect();
}

auto Peer::Imp::process_reorg(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(5 < body.size());

    if (body.at(1).as<decltype(chain_)>() != chain_) { return; }

    using Height = opentxs::blockchain::block::Height;
    using Hash = opentxs::blockchain::block::Hash;
    using Position = opentxs::blockchain::block::Position;
    const auto parent = Position{body.at(3).as<Height>(), body.at(2).Bytes()};
    auto hash = Hash{body.at(4).Bytes()};
    auto tip = Position{body.at(5).as<Height>(), hash};
    auto positions = header_oracle_.Ancestors(parent, tip);
    update_local_position(std::move(tip));

    for (auto& position : positions) {
        process_block(std::move(position.hash_));
    }
}

auto Peer::Imp::process_sendresult(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(2 < body.size());

    static constexpr auto error = std::byte{0x00};

    if (error == body.at(2).as<std::byte>()) {
        const auto why = [&] {
            if (3 < body.size()) {

                return body.at(3).Bytes();
            } else {

                return "unspecified send error"sv;
            }
        }();
        disconnect(why);
    }
}

auto Peer::Imp::process_statetimeout(Message&& msg) noexcept -> void
{
    const auto why = CString{name_}
                         .append(" failed to transition out of state "sv)
                         .append(print_state(state_));
    disconnect(why);
}

auto Peer::Imp::reset_activity_timer() noexcept -> void
{
    reset_timer(inactivity_interval_, activity_timer_, Work::activitytimeout);
}

auto Peer::Imp::reset_job_timer() noexcept -> void
{
    OT_ASSERT(has_job());

    reset_timer(job_timeout_, job_timer_, Work::jobtimeout);
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

auto Peer::Imp::run_job() noexcept -> void
{
    OT_ASSERT(has_job());

    auto visitor = RunJob{*this};
    std::visit(visitor, job_);
    reset_job_timer();
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
    state_timer_.Cancel();
    state_ = state;
    log_(OT_PRETTY_CLASS())(name_)(": transitioned to ")(print_state(state))(
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

auto Peer::Imp::transition_state_handshake() noexcept -> void
{
    transition_state(State::handshake, 30s);
}

auto Peer::Imp::transition_state_run() noexcept -> void
{
    const auto [network, limited, cfilter, bloom] = [&] {
        using Service = opentxs::blockchain::p2p::Service;
        const auto services = address_.Services();
        auto network = (1 == services.count(Service::Network));
        auto limited = (1 == services.count(Service::Limited));
        auto cfilter = (1 == services.count(Service::CompactFilters));
        auto bloom = (1 == services.count(Service::Bloom));

        return std::make_tuple(network, limited, cfilter, bloom);
    }();
    using Task = opentxs::blockchain::node::PeerManagerJobs;
    auto& pipeline = pipeline_.Internal();

    pipeline.SubscribeFromThread(parent_.Endpoint(Task::Heartbeat));
    pipeline.SubscribeFromThread(api_.Endpoints().BlockchainMempool());

    if (network || limited || block_header_capability_) {
        pipeline.PullFromThread(parent_.Endpoint(Task::Getheaders));
        pipeline.PullFromThread(parent_.Endpoint(Task::BroadcastTransaction));
        pipeline.SubscribeFromThread(
            network_.Internal().Endpoints().block_cache_job_ready_publish_);
        pipeline.SubscribeFromThread(
            api_.Endpoints().BlockchainBlockDownloadQueue());
        pipeline.SubscribeFromThread(block_oracle_.Internal().Endpoint());
    }

    if (cfilter || cfilter_capability_) {
        pipeline.SubscribeFromThread(
            parent_.Endpoint(Task::JobAvailableCfheaders));
        pipeline.SubscribeFromThread(
            parent_.Endpoint(Task::JobAvailableCfilters));
    }

    if (BlockchainProfile::server == config_.profile_) {
        pipeline.SubscribeFromThread(
            api_.Endpoints().Internal().BlockchainBlockUpdated(chain_));
    }

    transition_state(State::run);
    parent_.VerifyPeer(id_, address_.Display());
    reset_peers_timer(0s);

    if (bloom) { transmit_request_mempool(); }

    job_ = GetHeadersJob{};
    run_job();
}

auto Peer::Imp::transition_state_shutdown() noexcept -> void
{
    transition_state(State::shutdown);
}

auto Peer::Imp::transition_state_verify() noexcept -> void
{
    transition_state(State::verify, 60s);
}

auto Peer::Imp::transmit(
    std::pair<zeromq::Frame, zeromq::Frame>&& data) noexcept -> void
{
    switch (state_) {
        case State::connect:
        case State::handshake:
        case State::verify:
        case State::run: {
        } break;
        default: {
            LogAbort()(OT_PRETTY_CLASS())("attempting to transmit in state ")(
                static_cast<int>(state_))
                .Abort();
        }
    }

    transmit([&] {
        auto& [header, payload] = data;
        auto out = MakeWork(OT_ZMQ_SEND_SIGNAL);
        out.AddFrame(std::move(header));
        out.AddFrame(std::move(payload));

        return out;
    }());
}

auto Peer::Imp::transmit(Message&& message) noexcept -> void
{
    OT_ASSERT(2 < message.Body().size());

    auto body = message.Body();
    auto& header = body.at(1);
    auto& payload = body.at(2);
    const auto bytes = header.size() + payload.size();
    log_(OT_PRETTY_CLASS())("transmitting ")(bytes)(" byte message to ")(
        name_)(": ")
        .asHex(payload.Bytes())
        .Flush();
    auto msg =
        connection_.transmit(std::move(header), std::move(payload), nullptr);

    if (msg.has_value()) { pipeline_.Send(std::move(msg.value())); }
}

auto Peer::Imp::update_activity() noexcept -> void
{
    last_activity_ = Clock::now();

    if (State::run == state_) { reset_ping_timer(); }

    reset_activity_timer();
}

auto Peer::Imp::update_address() noexcept -> void
{
    address_.SetLastConnected(last_activity_);
    database_.AddOrUpdate(address_.clone_internal());
}

auto Peer::Imp::update_address(
    const UnallocatedSet<opentxs::blockchain::p2p::Service>& services) noexcept
    -> void
{
    address_.SetServices(services);
    database_.AddOrUpdate(address_.clone_internal());
}

auto Peer::Imp::update_block_job(const ReadView block) noexcept -> bool
{
    auto visitor = UpdateBlockJob{block};

    return update_job(visitor);
}

auto Peer::Imp::update_cfheader_job(
    opentxs::blockchain::cfilter::Type type,
    opentxs::blockchain::block::Position&& block,
    opentxs::blockchain::cfilter::Hash&& hash) noexcept -> void
{
    auto visitor =
        UpdateCfheaderJob{std::move(type), std::move(block), std::move(hash)};
    update_job(visitor);
}

auto Peer::Imp::update_cfilter_job(
    opentxs::blockchain::cfilter::Type type,
    opentxs::blockchain::block::Position&& block,
    opentxs::blockchain::GCS&& filter) noexcept -> void
{
    auto visitor =
        UpdateCfilterJob{std::move(type), std::move(block), std::move(filter)};
    update_job(visitor);
}

auto Peer::Imp::update_get_headers_job() noexcept -> void
{
    static const auto visitor = UpdateGetHeadersJob{};
    update_job(visitor);
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

auto Peer::Imp::work() noexcept -> bool
{
    check_jobs();

    return false;
}

Peer::Imp::~Imp() = default;
}  // namespace opentxs::network::blockchain::internal
