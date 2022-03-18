// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/p2p/peer/Peer.hpp"  // IWYU pragma: associated

#include <chrono>
#include <stdexcept>
#include <string_view>

#include "blockchain/DownloadTask.hpp"
#include "blockchain/p2p/peer/ConnectionManager.hpp"
#include "internal/util/Future.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/asio/Socket.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::implementation
{
Peer::Peer(
    const api::Session& api,
    const node::internal::Config& config,
    const node::internal::Mempool& mempool,
    const node::internal::Network& network,
    const node::internal::FilterOracle& filter,
    const node::internal::BlockOracle& block,
    const node::internal::PeerManager& manager,
    node::internal::PeerDatabase& database,
    const int id,
    const UnallocatedCString& shutdown,
    const std::size_t headerSize,
    const std::size_t bodySize,
    std::unique_ptr<internal::Address> address) noexcept
    : Worker(api, 10ms)
    , network_(network)
    , filter_(filter)
    , block_(block)
    , manager_(manager)
    , database_(database)
    , mempool_(mempool)
    , chain_(address->Chain())
    , display_chain_(print(chain_))
    , header_probe_(false)
    , cfilter_probe_(false)
    , address_(std::move(address))
    , download_peers_()
    , state_()
    , cfheader_job_()
    , cfilter_job_()
    , block_job_()
    , known_transactions_()
    , init_start_(Clock::now())
    , verify_filter_checkpoint_(config.download_cfilters_)
    , id_(id)
    , shutdown_endpoint_(shutdown)
    , untrusted_connection_id_(pipeline_.ConnectionIDDealer())
    , connection_(init_connection_manager(
          api_,
          id_,
          manager_,
          *this,
          running_,
          address_,
          headerSize))
    , send_promises_()
    , activity_(api_, pipeline_)
    , init_promise_()
    , init_(init_promise_.get_future())
{
    OT_ASSERT(connection_);

    connection_->init();
    init_executor({manager_.Endpoint(Task::Heartbeat), shutdown_endpoint_});
    trigger();
}

auto Peer::activity_timeout() noexcept -> void
{
    LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
        address_.Display())(" due to activity timeout.")
        .Flush();
    disconnect();
}

auto Peer::break_promises() noexcept -> void
{
    state_.break_promises();
    send_promises_.Break();
}

auto Peer::check_download_peers() noexcept -> void
{
    const auto interval = Clock::now() - download_peers_.get();
    const bool download =
        std::chrono::minutes(peer_download_interval_) <= interval;

    if (download) { request_addresses(); }
}

auto Peer::check_init() noexcept -> void
{
    if (connection_->is_initialized()) {
        LogTrace()(display_chain_)(" peer ")(address_.Display())(" initialized")
            .Flush();
        state_.value_.store(State::Connect);
        connect();
    } else {
        static constexpr auto limit = 30s;
        const auto elapsed = Clock::now() - init_start_;

        if (limit <= elapsed) {
            LogVerbose()("Disconnecting ")(display_chain_)(" peer ")(
                address_.Display())(" due to connection timeout.")
                .Flush();
            disconnect();
        }
    }
}

auto Peer::check_jobs() noexcept -> void
{
    constexpr auto limit = std::chrono::minutes(1);

    if (auto& job = cfheader_job_; job) {
        if (job.Elapsed() >= limit) { reset_cfheader_job(); }
    } else if (cfilter_probe_) {
        reset_cfheader_job();
    }

    if (auto& job = cfilter_job_; job) {
        if (job.Elapsed() >= limit) { reset_cfilter_job(); }
    } else if (cfilter_probe_) {
        reset_cfilter_job();
    }

    if (auto& job = block_job_; job) {
        if (job.Elapsed() >= limit) { reset_block_job(); }
    } else if (header_probe_) {
        reset_block_job();
    }
}

auto Peer::check_handshake() noexcept -> void
{
    auto& state = state_.handshake_;

    if (state.first_action_ && state.second_action_ &&
        (false == state.done())) {
        LogDetail()(
            address_.Incoming() ? "Incoming connection from "
                                : "Connected to ")(print(address_.Chain()))(
            " peer at ")(address_.Display())
            .Flush();
        LogVerbose()("Advertised services: ").Flush();

        for (const auto& service : address_.Services()) {
            LogVerbose()(" * ")(p2p::DisplayService(service)).Flush();
        }

        update_address_activity();
        state.promise_.set_value();

        OT_ASSERT(state.done());
    }

    trigger();
}

auto Peer::check_verify() noexcept -> void
{
    auto& state = state_.verify_;

    if (state.first_action_ &&
        (state.second_action_ || (false == verify_filter_checkpoint_)) &&
        (false == state.done())) {
        state.promise_.set_value();
    }

    trigger();
}

auto Peer::connect() noexcept -> void { connection_->connect(); }

auto Peer::disconnect() noexcept -> void
{
    try {
        state_.connect_.promise_.set_value(false);
    } catch (...) {
    }

    LogVerbose()(
        address_.Incoming() ? "Dropping incoming connection "
                            : "Disconnecting from ")(connection_->host())(":")(
        connection_->port())
        .Flush();
    manager_.Disconnect(id_);
}

auto Peer::init() noexcept -> void { init_promise_.set_value(); }

auto Peer::init_connection_manager(
    const api::Session& api,
    const int id,
    const node::internal::PeerManager& manager,
    Peer& parent,
    const std::atomic<bool>& running,
    const peer::Address& address,
    const std::size_t headerSize) noexcept
    -> std::unique_ptr<peer::ConnectionManager>
{
    if (Network::zmq == address.Type()) {
        if (address.Incoming()) {

            return peer::ConnectionManager::ZMQIncoming(
                api,
                id,
                parent,
                parent.pipeline_,
                running,
                address,
                headerSize);
        } else {

            return peer::ConnectionManager::ZMQ(
                api,
                id,
                parent,
                parent.pipeline_,
                running,
                address,
                headerSize);
        }
    } else {
        if (address.Incoming()) {

            return peer::ConnectionManager::TCPIncoming(
                api,
                id,
                parent,
                parent.pipeline_,
                running,
                address,
                headerSize,
                manager.LookupIncomingSocket(id));
        } else {

            return peer::ConnectionManager::TCP(
                api,
                id,
                parent,
                parent.pipeline_,
                running,
                address,
                headerSize);
        }
    }
}

auto Peer::on_connect() noexcept -> void
{
    LogTrace()(display_chain_)(" peer ")(address_.Display())(" connected")
        .Flush();

    try {
        state_.connect_.promise_.set_value(true);
    } catch (...) {
    }

    state_.value_.store(
        address_.Incoming() ? State::Listening : State::Handshake);
    trigger();
}

auto Peer::on_init() noexcept -> void { check_init(); }

auto Peer::pipeline(zmq::Message&& message) noexcept -> void
{
    if (false == IsReady(init_)) {
        pipeline_.Push(std::move(message));

        return;
    }

    if (false == running_.load()) { return; }

    const auto header = message.Header();
    const auto body = message.Body();

    OT_ASSERT(0 < header.size());
    OT_ASSERT(0 < body.size());

    const auto connectionID = header.at(0).as<std::size_t>();
    const auto task = [&] {
        try {

            return body.at(0).as<Task>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            // TODO It's impossible for this exception to happen but it does
            // anyway from time to time. Somebody really ought to figure out why
            // someday.

            return Task::Disconnect;
        }
    }();

    if (connectionID == untrusted_connection_id_) {
        if (false == connection_->filter(task)) {
            LogError()(OT_PRETTY_CLASS())("Peer ")(address_.Display())(
                " sent an internal control message instead of a valid protocol "
                "message")
                .Flush();
            disconnect();

            return;
        }
    }

    switch (task) {
        case Task::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        case Task::Mempool: {
            process_mempool(message);
        } break;
        case Task::Register: {
            connection_->on_register(std::move(message));
        } break;
        case Task::Connect: {
            connection_->on_connect(std::move(message));
        } break;
        case Task::Disconnect: {
            disconnect();
        } break;
        case Task::Getheaders: {
            if (State::Run == state_.value_.load()) { request_headers(); }
        } break;
        case Task::Getblock: {
            request_block(std::move(message));
        } break;
        case Task::BroadcastTransaction: {
            broadcast_transaction(std::move(message));
        } break;
        case Task::BroadcastBlock: {
            broadcast_block(std::move(message));
        } break;
        case Task::JobAvailableCfheaders: {
            if (State::Run == state_.value_.load()) {
                if (cfheader_job_) { break; }

                reset_cfheader_job();
            }
        } break;
        case Task::JobAvailableCfilters: {
            if (State::Run == state_.value_.load()) {
                if (cfilter_job_) { break; }

                reset_cfilter_job();
            }
        } break;
        case Task::JobAvailableBlock: {
            if (State::Run == state_.value_.load()) {
                if (block_job_) { break; }

                reset_block_job();
            }
        } break;
        case Task::ActivityTimeout: {
            activity_timeout();
        } break;
        case Task::NeedPing: {
            if (State::Run == state_.value_.load()) { ping(); }
        } break;
        case Task::Body: {
            activity_.Bump();
            connection_->on_body(std::move(message));
        } break;
        case Task::Header: {
            activity_.Bump();
            connection_->on_header(std::move(message));
        } break;
        case Task::P2P:
        case Task::ReceiveMessage: {
            activity_.Bump();
            process_message(std::move(message));
        } break;
        case Task::SendMessage: {
            transmit(std::move(message));
        } break;
        case Task::Init: {
            connection_->on_init(std::move(message));
        } break;
        case Task::Heartbeat:
        case Task::StateMachine: {
            do_work();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Peer::process_mempool(const zmq::Message& message) noexcept -> void
{
    const auto body = message.Body();

    if (body.at(1).as<Type>() != chain_) { return; }

    const auto hash = UnallocatedCString{body.at(2).Bytes()};

    if (0 < known_transactions_.count(hash)) { return; }

    broadcast_inv_transaction(hash);
}

auto Peer::process_state_machine() noexcept -> void
{
    switch (state_.value_.load()) {
        case State::Init: {
            check_init();
        } break;
        case State::Run: {
            check_jobs();
            check_download_peers();
        } break;
        default: {
        }
    }
}

auto Peer::reset_block_job() noexcept -> void
{
    auto& job = block_job_;
    job = {};

    if (header_probe_) { job = block_.GetBlockJob(); }

    if (job) { request_blocks(); }
}

auto Peer::reset_cfheader_job() noexcept -> void
{
    auto& job = cfheader_job_;
    job = {};

    if (cfilter_probe_) { job = filter_.GetHeaderJob(); }

    if (job) { request_cfheaders(); }
}

auto Peer::reset_cfilter_job() noexcept -> void
{
    auto& job = cfilter_job_;
    job = {};

    if (cfilter_probe_) { job = filter_.GetFilterJob(); }

    if (job) { request_cfilter(); }
}

auto Peer::send(std::pair<zmq::Frame, zmq::Frame>&& frames) noexcept
    -> SendStatus
{
    try {
        if (false == state_.connect_.future_.get()) {
            LogVerbose()(OT_PRETTY_CLASS())(
                "Unable to send to disconnected peer")
                .Flush();

            return {};
        }
    } catch (...) {
        return {};
    }

    if (running_.load()) {
        auto data = send_promises_.NewPromise();
        pipeline_.Push([&] {
            auto& [future, promise] = data;
            auto& [header, payload] = frames;
            auto out = MakeWork(Task::SendMessage);
            out.AddFrame(std::move(header));
            out.AddFrame(std::move(payload));
            out.AddFrame(promise);

            return out;
        }());
        auto& [future, promise] = data;

        return std::move(future);
    } else {
        return {};
    }
}

auto Peer::Shutdown() noexcept -> std::shared_future<void>
{
    return signal_shutdown();
}

auto Peer::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        init_.get();
        pipeline_.Close();
        const auto state = state_.value_.exchange(State::Shutdown);
        connection_->stop_external();
        break_promises();
        connection_->shutdown_external();

        if ((State::Handshake != state) && (State::Listening != state)) {
            update_address_activity();
        }

        LogVerbose()("Disconnected from ")(address_.Display()).Flush();
        promise.set_value();
    }
}

auto Peer::start_verify() noexcept -> void
{
    if (address_.Incoming()) {
        LogVerbose()(OT_PRETTY_CLASS())("incoming peer ")(address_.Display())(
            " is not required to validate checkpoints")
            .Flush();
        state_.value_.store(State::Subscribe);
        do_work();
    } else {
        request_checkpoint_block_header();

        if (verify_filter_checkpoint_) {
            LogVerbose()(OT_PRETTY_CLASS())("outgoing peer ")(
                address_.Display())(
                " must validate block header and cfheader checkpoints")
                .Flush();
            request_checkpoint_filter_header();
        } else {
            LogVerbose()(OT_PRETTY_CLASS())("outgoing peer ")(
                address_.Display())(
                " must validate block header checkpoints only")
                .Flush();
        }
    }
}

auto Peer::state_machine() noexcept -> bool
{
    LogTrace()(OT_PRETTY_CLASS()).Flush();

    if (false == running_.load()) { return false; }

    try {
        switch (state_.value_.load()) {
            case State::Listening: {
                OT_ASSERT(address_.Incoming());

                LogVerbose()(OT_PRETTY_CLASS())(
                    "verifying incoming handshake protocol for ")(
                    address_.Display())
                    .Flush();
                static constexpr auto timeout = 20s;
                const auto disconnect = state_.handshake_.run(
                    timeout, [] {}, State::Verify);

                if (disconnect) {
                    throw std::runtime_error{
                        UnallocatedCString{
                            "failure to complete handshake within "} +
                        std::to_string(timeout.count()) +
                        " seconds (incoming)"};
                }
            } break;
            case State::Handshake: {
                LogVerbose()(OT_PRETTY_CLASS())(
                    "verifying outgoing handshake protocol for ")(
                    address_.Display())
                    .Flush();
                static constexpr auto timeout = 20s;
                const auto disconnect = state_.handshake_.run(
                    timeout, [this] { start_handshake(); }, State::Verify);

                if (disconnect) {
                    throw std::runtime_error{
                        UnallocatedCString{
                            "failure to complete handshake within "} +
                        std::to_string(timeout.count()) +
                        " seconds (outgoing)"};
                }
            } break;
            case State::Verify: {
                LogVerbose()(OT_PRETTY_CLASS())("verifying checkpoints for ")(
                    address_.Display())
                    .Flush();
                static constexpr auto timeout = 30s;
                const auto disconnect = state_.verify_.run(
                    timeout, [this] { start_verify(); }, State::Subscribe);

                if (disconnect) {
                    throw std::runtime_error{
                        UnallocatedCString{
                            "failure to respond to checkpoint requests "
                            "within "} +
                        std::to_string(timeout.count()) + " seconds"};
                }
            } break;
            case State::Subscribe: {
                LogVerbose()(OT_PRETTY_CLASS())(
                    "achieved subscribe state for ")(address_.Display())
                    .Flush();
                subscribe();
                state_.value_.store(State::Run);
                manager_.VerifyPeer(id_, address_.Display());
                [[fallthrough]];
            }
            case State::Run: {
                LogVerbose()(OT_PRETTY_CLASS())("achieved run state for ")(
                    address_.Display())
                    .Flush();
                process_state_machine();
            } break;
            case State::Shutdown: {
                throw std::runtime_error{"session shutdown in progress"};
            }
            default: {
            }
        }
    } catch (const std::exception& e) {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" due to ")(e.what())
            .Flush();
        disconnect();
    }

    return false;
}

auto Peer::subscribe() noexcept -> void
{
    const auto network =
        (1 == address_.Services().count(p2p::Service::Network));
    const auto limited =
        (1 == address_.Services().count(p2p::Service::Limited));
    const auto cfilter =
        (1 == address_.Services().count(p2p::Service::CompactFilters));
    const auto bloom = (1 == address_.Services().count(p2p::Service::Bloom));

    if (network || limited || header_probe_) {
        pipeline_.PullFrom(manager_.Endpoint(Task::Getheaders));
        pipeline_.PullFrom(manager_.Endpoint(Task::Getblock));
        pipeline_.PullFrom(manager_.Endpoint(Task::BroadcastTransaction));
        pipeline_.SubscribeTo(manager_.Endpoint(Task::JobAvailableBlock));
    }
    if (cfilter || cfilter_probe_) {
        pipeline_.SubscribeTo(manager_.Endpoint(Task::JobAvailableCfheaders));
        pipeline_.SubscribeTo(manager_.Endpoint(Task::JobAvailableCfilters));
    }

    pipeline_.SubscribeTo(manager_.Endpoint(Task::BroadcastBlock));
    pipeline_.SubscribeTo(api_.Endpoints().BlockchainMempool());
    request_headers();
    request_addresses();
    if (bloom) { request_mempool(); }
}

auto Peer::transmit(zmq::Message&& message) noexcept -> void
{
    if (false == running_.load()) { return; }

    OT_ASSERT(message.Body().size() > 3)

    auto body = message.Body();
    auto& header = body.at(1);
    auto& payload = body.at(2);
    const auto& promiseFrame = body.at(3);
    const auto index = promiseFrame.as<int>();
    auto success = bool{false};
    auto postcondition =
        ScopeGuard{[&] { send_promises_.SetPromise(index, success); }};
    LogTrace()(OT_PRETTY_CLASS())("Sending ")(header.size() + payload.size())(
        " byte message:")
        .Flush();
    LogInsane()(Data::Factory(header)->asHex())(Data::Factory(payload)->asHex())
        .Flush();
    auto promise = std::make_unique<peer::ConnectionManager::SendPromise>();

    OT_ASSERT(promise);

    auto future = promise->get_future();
    connection_->transmit(
        std::move(header), std::move(payload), std::move(promise));
    auto result{false};
    const auto start = Clock::now();
    static const auto limit = 10s;

    try {
        while (running_.load()) {
            const auto status = future.wait_for(5ms);

            if (std::future_status::ready == status) {
                result = future.get();

                break;
            } else if (const auto time = Clock::now() - start; time >= limit) {
                LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
                    address_.Display())(" due to transmit timeout.")
                    .Flush();
                disconnect();

                return;
            }
        }
    } catch (const std::exception& e) {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" due to transmit error: ")(e.what())
            .Flush();
        disconnect();

        return;
    }

    if (result) {
        LogVerbose()(OT_PRETTY_CLASS())("Sent ")(payload.size())(" bytes")
            .Flush();
        success = true;
    } else {
        LogConsole()("Disconnecting ")(display_chain_)(" peer ")(
            address_.Display())(" due to unspecified transmit error.")
            .Flush();
        success = false;
        disconnect();
    }
}

auto Peer::update_address_activity() noexcept -> void
{
    database_.AddOrUpdate(address_.UpdateTime(activity_.get()));
}

auto Peer::update_address_services(
    const UnallocatedSet<p2p::Service>& services) noexcept -> void
{
    database_.AddOrUpdate(address_.UpdateServices(services));
}

Peer::~Peer() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::p2p::implementation
