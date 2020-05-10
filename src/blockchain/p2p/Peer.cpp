// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "blockchain/p2p/Peer.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <type_traits>

#include "internal/api/Api.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

#define OT_BLOCKCHAIN_PEER_PING_SECONDS 30
#define OT_BLOCKCHAIN_PEER_DISCONNECT_SECONDS 40
#define OT_BLOCKCHAIN_PEER_DOWNLOAD_ADDRESSES_MINUTES 10

#define OT_METHOD "opentxs::blockchain::p2p::implementation::Peer::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::implementation
{
Peer::Peer(
    const api::internal::Core& api,
    const client::internal::Network& network,
    const client::internal::PeerManager& manager,
    const blockchain::client::internal::IO& context,
    const int id,
    const std::string& shutdown,
    const std::size_t headerSize,
    const std::size_t bodySize,
    std::unique_ptr<internal::Address> address) noexcept
    : Executor(api, std::bind(&Peer::state_machine, this))
    , network_(network)
    , manager_(manager)
    , endpoint_(
          make_endpoint(address->Type(), address->Bytes(), address->Port()))
    , send_promise_()
    , send_future_(send_promise_.get_future())
    , address_(std::move(address))
    , download_peers_()
    , header_(make_buffer(headerSize))
    , outgoing_handshake_(false)
    , incoming_handshake_(false)
    , verify_complete_(false)
    , started_subscribe_(false)
    , subscribe_()
    , header_bytes_(headerSize)
    , id_(id)
    , connection_id_()
    , shutdown_endpoint_(shutdown)
    , context_(context)
    , socket_(context_.operator boost::asio::io_context&())
    , outgoing_message_(Data::Factory())
    , connection_id_promise_()
    , connection_promise_()
    , connected_(connection_promise_.get_future())
    , handshake_promise_()
    , verify_promise_()
    , handshake_(handshake_promise_.get_future())
    , verify_(verify_promise_.get_future())
    , send_promises_()
    , activity_()
    , state_(State::Handshake)
    , cb_(zmq::ListenCallback::Factory([&](auto& in) { pipeline_d(in); }))
    , dealer_(api.ZeroMQ().DealerSocket(
          cb_,
          zmq::socket::Socket::Direction::Connect))
{
    auto future = connection_id_promise_.get_future();
    auto zmq = dealer_->Start(api_.Endpoints().InternalBlockchainAsioContext());

    OT_ASSERT(zmq);

    auto message = MakeWork(OTZMQWorkType{OT_ZMQ_REGISTER_SIGNAL});
    message->AddFrame(id_);
    zmq = dealer_->Send(message);

    OT_ASSERT(zmq);

    const auto status = future.wait_for(std::chrono::seconds(10));

    if (std::future_status::ready != status) { disconnect(); }
}

Peer::Activity::Activity() noexcept
    : lock_()
    , activity_(Clock::now())
{
}

Peer::Address::Address(std::unique_ptr<internal::Address> address) noexcept
    : lock_()
    , address_(std::move(address))
{
    OT_ASSERT(address_);
}

Peer::DownloadPeers::DownloadPeers() noexcept
    : downloaded_(Clock::now())
{
}

Peer::SendPromises::SendPromises() noexcept
    : lock_()
    , counter_(0)
    , map_()
{
}

bool Peer::verifying() noexcept { return (State::Verify == state_.load()); }

auto Peer::Activity::Bump() noexcept -> void
{
    Lock lock(lock_);
    activity_ = Clock::now();
}

auto Peer::Activity::get() const noexcept -> Time
{
    Lock lock(lock_);

    return activity_;
}

auto Peer::Address::Bytes() const noexcept -> OTData
{
    Lock lock(lock_);

    return address_->Bytes();
}

auto Peer::Address::Chain() const noexcept -> blockchain::Type
{
    Lock lock(lock_);

    return address_->Chain();
}

auto Peer::Address::Display() const noexcept -> std::string
{
    Lock lock(lock_);

    return address_->Display();
}

auto Peer::Address::ID() const noexcept -> OTIdentifier
{
    Lock lock(lock_);

    return address_->ID();
}

auto Peer::Address::Port() const noexcept -> std::uint16_t
{
    Lock lock(lock_);

    return address_->Port();
}

auto Peer::Address::Services() const noexcept -> std::set<Service>
{
    Lock lock(lock_);

    return address_->Services();
}

auto Peer::Address::Type() const noexcept -> Network
{
    Lock lock(lock_);

    return address_->Type();
}

auto Peer::Address::UpdateServices(
    const std::set<p2p::Service>& services) noexcept -> pointer
{
    Lock lock(lock_);
    address_->SetServices(services);

    return address_->clone_internal();
}

auto Peer::Address::UpdateTime(const Time& time) noexcept -> pointer
{
    Lock lock(lock_);
    address_->SetLastConnected(time);

    return address_->clone_internal();
}

auto Peer::DownloadPeers::Bump() noexcept -> void
{
    downloaded_ = Clock::now();
}

auto Peer::DownloadPeers::get() const noexcept -> Time { return downloaded_; }

auto Peer::SendPromises::Break() -> void
{
    Lock lock(lock_);

    for (auto& [id, promise] : map_) { promise = {}; }
}

auto Peer::SendPromises::NewPromise() -> std::pair<std::future<bool>, int>
{
    Lock lock(lock_);
    auto [it, added] = map_.emplace(++counter_, std::promise<bool>());

    if (false == added) { return {}; }

    return {it->second.get_future(), it->first};
}

auto Peer::SendPromises::SetPromise(const int promise, const bool value) -> void
{
    Lock lock(lock_);

    auto it = map_.find(promise);

    if (map_.end() != it) {
        try {
            it->second.set_value(value);
        } catch (...) {
        }

        map_.erase(it);
    }
}

auto Peer::Subscriptions::Push(value_type& tasks) noexcept -> void
{
    Lock lock(lock_);
    tasks_.swap(tasks);
}

auto Peer::Subscriptions::Pop() noexcept -> value_type
{
    auto output = value_type{};
    Lock lock(lock_);
    tasks_.swap(output);

    return output;
}

auto Peer::break_promises() noexcept -> void
{
    handshake_promise_ = {};
    connection_promise_ = {};
    verify_promise_ = {};
    send_promise_ = {};
    send_promises_.Break();
}

auto Peer::check_activity() noexcept -> void
{
    const auto interval = Clock::now() - activity_.get();
    const bool disconnect =
        std::chrono::seconds(OT_BLOCKCHAIN_PEER_DISCONNECT_SECONDS) <= interval;
    const bool ping =
        std::chrono::seconds(OT_BLOCKCHAIN_PEER_PING_SECONDS) <= interval;

    if (disconnect) {
        this->disconnect();
    } else if (ping) {
        this->ping();
    }
}

auto Peer::check_download_peers() noexcept -> void
{
    const auto interval = Clock::now() - download_peers_.get();
    const bool download =
        std::chrono::minutes(OT_BLOCKCHAIN_PEER_DOWNLOAD_ADDRESSES_MINUTES) <=
        interval;

    if (download) { request_addresses(); }
}

auto Peer::check_handshake() noexcept -> void
{
    if (outgoing_handshake_ && incoming_handshake_) {
        try {
            state_.store(State::Verify);
            handshake_promise_.set_value();
            update_address_activity();
            LogNormal("Connected to ")(blockchain::internal::DisplayString(
                address_.Chain()))(" peer at ")(address_.Display())
                .Flush();
            LogNormal("Advertised services: ").Flush();

            for (const auto& service : address_.Services()) {
                LogNormal(" * ")(p2p::DisplayService(service)).Flush();
            }

        } catch (...) {
        }
    }
}

auto Peer::check_verify() noexcept -> void
{
    if (verify_complete_) {
        try {
            state_.store(State::Subscribe);
            verify_promise_.set_value();
        } catch (...) {
        }
    }
}

auto Peer::check_subscribe() noexcept -> void
{
    if (started_subscribe_) {
        try {
            state_.store(State::Run);
        } catch (...) {
        }

        request_headers();
        request_addresses();
    }
}

auto Peer::connect() noexcept -> void
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Connecting to ")(
        endpoint_.address().to_string())(":")(endpoint_.port())
        .Flush();
    context_.Connect(connection_id_, endpoint_, socket_);
}

auto Peer::disconnect() noexcept -> void
{
    try {
        connection_promise_.set_value(false);
    } catch (...) {
    }

    manager_.Disconnect(id_);
}

auto Peer::handshake() noexcept -> void
{
    static const auto limit = std::chrono::seconds(15);
    static const auto wait = std::chrono::milliseconds(10);
    start_handshake();
    auto disconnect{true};
    auto done{handshake_};
    const auto start = Clock::now();

    try {
        while (running_.get() && (limit > (Clock::now() - start))) {
            if (std::future_status::ready == done.wait_for(wait)) {
                disconnect = false;
                break;
            }
        }
    } catch (...) {
    }

    if (disconnect) { this->disconnect(); }
}

auto Peer::subscribe() noexcept -> void
{
    try {
        auto subscribe = Subscriptions::value_type{};

        const auto services_network_ =
            (1 == address_.Services().count(p2p::Service::Network));
        const auto services_limited_ =
            (1 == address_.Services().count(p2p::Service::Limited));
        const auto services_cfilters_ =
            (1 == address_.Services().count(p2p::Service::CompactFilters));

        if (services_network_ || services_limited_) {
            subscribe.emplace_back(Task::Getheaders);
            subscribe.emplace_back(Task::Getblock);
        }

        if (services_cfilters_) {
            subscribe.emplace_back(Task::Getcfheaders);
            subscribe.emplace_back(Task::Getcfilters);
        }

        subscribe_.Push(subscribe);
        started_subscribe_ = true;
        Trigger();
    } catch (...) {
        disconnect();
    }
}

auto Peer::verify() noexcept -> void
{
    static const auto limit = std::chrono::seconds(15);
    static const auto wait = std::chrono::milliseconds(10);
    start_verify();
    auto disconnect{true};
    auto done{verify_};
    const auto start = Clock::now();

    try {
        while (running_.get() && (limit > (Clock::now() - start))) {
            if (std::future_status::ready == done.wait_for(wait)) {
                disconnect = false;
                break;
            }
        }
    } catch (...) {
    }

    if (disconnect) { this->disconnect(); }
}

auto Peer::init() noexcept -> void { connect(); }

auto Peer::init_send_promise() noexcept -> void
{
    send_promise_ = SendPromise{};
    send_future_ = send_promise_.get_future();
}

auto Peer::local_endpoint() noexcept -> tcp::socket::endpoint_type
{
    try {
        return socket_.local_endpoint();
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
        OT_FAIL;
    }
}

auto Peer::make_buffer(const std::size_t size) noexcept -> OTData
{
    auto output = Data::Factory();
    output->SetSize(size);

    return output;
}

auto Peer::make_endpoint(
    const Network type,
    const Data& raw,
    const std::uint16_t port) noexcept -> tcp::endpoint
{
    auto output = ip::address_v6{};

    switch (type) {
        case p2p::Network::ipv6: {
            auto bytes = ip::address_v6::bytes_type{};

            OT_ASSERT(bytes.size() == raw.size());

            std::memcpy(&bytes, raw.data(), bytes.size());
            output = ip::make_address_v6(bytes);
        } break;
        case p2p::Network::ipv4: {
            auto output4 = ip::address_v4{};
            auto bytes = ip::address_v4::bytes_type{};

            OT_ASSERT(bytes.size() == raw.size());

            std::memcpy(&bytes, raw.data(), bytes.size());
            output4 = ip::make_address_v4(bytes);
            output = ip::make_address_v6(
                std::string("::ffff:") + output4.to_string());
        } break;
        default: {
            OT_FAIL;
        }
    }

    return ip::tcp::endpoint{output, port};
}

auto Peer::pipeline(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto header = message.Header();

    OT_ASSERT(0 < header.size());

    if (1 < header.size()) {
        auto* promise = reinterpret_cast<std::promise<void>*>(
            header.at(1).as<std::uintptr_t>());

        OT_ASSERT(nullptr != promise);

        promise->set_value();
    }

    switch (header.at(0).as<Task>()) {
        case Task::Getheaders: {
            request_headers();
        } break;
        case Task::Getcfheaders: {
            request_cfheaders(message);
        } break;
        case Task::Getcfilters: {
            request_cfilter(message);
        } break;
        case Task::Heartbeat: {
            Trigger();
        } break;
        case Task::Getblock: {
            request_block(message);
        } break;
        case Task::SendMessage: {
            transmit(message);
        } break;
        case Task::ReceiveMessage: {
            process_message(message);
        } break;
        case Task::StateMachine: {
            process_state_machine();
        } break;
        case Task::Shutdown: {
            shutdown(shutdown_promise_);
            Stop();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Peer::pipeline_d(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto header = message.Header();

    OT_ASSERT(0 < header.size());

    switch (header.at(0).as<Task>()) {
        case Task::Register: {
            const auto body = message.Body();

            OT_ASSERT(0 < body.size());

            const auto& id = body.at(0);
            const auto start = static_cast<const std::byte*>(id.data());
            const_cast<Space&>(connection_id_).assign(start, start + id.size());
            connection_id_promise_.set_value();
        } break;
        case Task::Connect: {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Connect to ")(
                endpoint_.address().to_string())(":")(endpoint_.port())(
                " successful")
                .Flush();
            connection_promise_.set_value(true);
            init_executor(
                {manager_.Endpoint(Task::Heartbeat), shutdown_endpoint_});
            Trigger();
            run();
        } break;
        case Task::Disconnect: {
            disconnect();
        } break;
        case Task::Header: {
            const auto body = message.Body();

            OT_ASSERT(0 < body.size());

            const auto& messageHeader = body.at(0);
            activity_.Bump();
            const auto size = get_body_size(messageHeader);

            if (0 < size) {
                header_->Assign(messageHeader.Bytes());
                context_.Receive(
                    connection_id_,
                    static_cast<OTZMQWorkType>(Task::Body),
                    size,
                    socket_);
            } else {
                auto message = MakeWork(Task::ReceiveMessage);
                message->AddFrame(messageHeader);
                message->AddFrame();
                pipeline_->Push(message);
                run();
            }
        } break;
        case Task::Body: {
            const auto body = message.Body();

            OT_ASSERT(0 < body.size());

            auto message = MakeWork(Task::ReceiveMessage);
            message->AddFrame(header_);
            message->AddFrame(body.at(0));
            pipeline_->Push(message);
            run();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Peer::process_state_machine() noexcept -> void
{
    switch (state_.load()) {
        case State::Verify: {
            check_verify();
        } break;
        case State::Subscribe: {
            check_subscribe();
        } break;
        case State::Run: {
            check_activity();
            check_download_peers();
        } break;
        default: {
        }
    }
}

auto Peer::run() noexcept -> void
{
    if (running_.get()) {
        context_.Receive(
            connection_id_,
            static_cast<OTZMQWorkType>(Task::Header),
            header_bytes_,
            socket_);
    }
}

auto Peer::send(OTData in) noexcept -> SendStatus
{
    try {
        if (false == connected_.get()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(
                ": Unable to send to disconnected peer")
                .Flush();

            return {};
        }
    } catch (...) {
        return {};
    }

    if (running_.get()) {
        auto [future, promise] = send_promises_.NewPromise();
        auto message = MakeWork(Task::SendMessage);
        message->AddFrame(in);
        message->AddFrame(Data::Factory(&promise, sizeof(promise)));
        pipeline_->Push(message);

        return std::move(future);
    } else {
        return {};
    }
}

auto Peer::Shutdown() noexcept -> std::shared_future<void>
{
    dealer_->Close();

    return stop_executor();
}

auto Peer::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (running_->Off()) {
        const auto state = state_.exchange(State::Shutdown);

        try {
            state_machine_.set_value(false);
        } catch (...) {
        }

        try {
            socket_.shutdown(tcp::socket::shutdown_both);
        } catch (...) {
        }

        break_promises();

        try {
            socket_.close();
        } catch (...) {
        }

        if (State::Handshake != state) { update_address_activity(); }

        LogVerbose("Disconnected from ")(address_.Display()).Flush();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto Peer::state_machine() noexcept -> bool
{
    if (false == running_.get()) { return false; }

    switch (state_.load()) {
        case State::Handshake: {
            handshake();
        } break;
        case State::Verify: {
            verify();
        } break;
        case State::Subscribe: {
            subscribe();
        } break;
        case State::Run: {
            subscribe_work();
            pipeline_->Push(MakeWork(Task::StateMachine));
        } break;
        case State::Shutdown: {
            shutdown(shutdown_promise_);
            pipeline_->Close();
        } break;
        default: {
        }
    }

    return false;
}

void Peer::subscribe_work() noexcept
{
    for (const auto& task : subscribe_.Pop()) {
        switch (task) {
            case Task::Getheaders: {
                pipeline_->Start(manager_.Endpoint(Task::Getheaders));
            } break;
            case Task::Getcfheaders: {
                pipeline_->Start(manager_.Endpoint(Task::Getcfheaders));
            } break;
            case Task::Getcfilters: {
                pipeline_->Start(manager_.Endpoint(Task::Getcfilters));
            } break;
            case Task::Getblock: {
                pipeline_->Start(manager_.Endpoint(Task::Getblock));
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
}

auto Peer::transmit(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    struct Cleanup {
        Cleanup(const bool& sucess, const zmq::Frame& frame, Peer& parent)
            : success_(sucess)
            , parent_(parent)
            , index_(0)
        {
            OT_ASSERT(sizeof(index_) == frame.size());

            std::memcpy(&index_, frame.data(), frame.size());
        }

        ~Cleanup() { parent_.send_promises_.SetPromise(index_, success_); }

    private:
        const bool& success_;
        Peer& parent_;
        int index_;
    };

    if (2 < message.Body().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        return;
    }

    const auto& payload = message.Body_at(0);
    const auto& promiseFrame = message.Body_at(1);
    auto success = bool{false};
    const Cleanup cleanup{success, promiseFrame, *this};
    LogTrace(OT_METHOD)(__FUNCTION__)(": Sending ")(payload.size())(
        " byte message:")
        .Flush();
    LogTrace(Data::Factory(message.at(0))->asHex()).Flush();
    init_send_promise();
    auto work = [this, &payload]() -> void {
        auto cb = [this](auto& error, auto bytes) -> void {
            try {
                this->send_promise_.set_value({error, bytes});
            } catch (...) {
            }
        };
        asio::async_write(
            this->socket_, asio::buffer(payload.data(), payload.size()), cb);
    };

    auto& asio = context_.operator boost::asio::io_context&();
    asio.post(work);
    auto result = SendResult{};

    try {
        while (running_.get()) {
            const auto status =
                send_future_.wait_for(std::chrono::milliseconds(5));

            if (std::future_status::ready == status) {
                result = send_future_.get();
                break;
            }
        }
    } catch (...) {
        return;
    }

    const auto& [error, bytes] = result;

    if (error) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(error.message()).Flush();
        success = false;
        disconnect();
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Sent ")(bytes)(" bytes").Flush();
        success = true;
    }
}

auto Peer::update_address_activity() noexcept -> void
{
    manager_.Database().AddOrUpdate(address_.UpdateTime(activity_.get()));
}

auto Peer::update_address_services(
    const std::set<p2p::Service>& services) noexcept -> void
{
    manager_.Database().AddOrUpdate(address_.UpdateServices(services));
}

Peer::~Peer() { Shutdown().get(); }
}  // namespace opentxs::blockchain::p2p::implementation
