// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "api/network/blockchain/SyncClient.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "Proto.tpp"
#include "internal/network/p2p/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/p2p/Acknowledgement.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/Query.hpp"
#include "opentxs/network/p2p/Request.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/p2p/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/BlockchainP2PChainState.pb.h"

namespace bc = opentxs::blockchain;

namespace opentxs::api::network::blockchain
{
struct SyncClient::Imp {
    using Chain = opentxs::blockchain::Type;

    auto Endpoint() const noexcept -> const std::string& { return endpoint_; }

    auto Init(const Blockchain& parent) noexcept -> void
    {
        first(parent);
        thread_ = std::thread{&Imp::thread, this};
    }

    Imp(const api::Session& api) noexcept
        : api_(api)
        , endpoint_(opentxs::network::zeromq::MakeArbitraryInproc())
        , monitor_endpoint_(opentxs::network::zeromq::MakeArbitraryInproc())
        , external_router_([&] {
            auto out = Socket{
                ::zmq_socket(api_.Network().ZeroMQ(), ZMQ_ROUTER), ::zmq_close};
            auto rc = ::zmq_setsockopt(
                out.get(), ZMQ_LINGER, &linger_, sizeof(linger_));

            OT_ASSERT(0 == rc);

            rc = ::zmq_setsockopt(
                out.get(), ZMQ_ROUTER_HANDOVER, &handover_, sizeof(handover_));

            OT_ASSERT(0 == rc);

            rc = ::zmq_socket_monitor(
                out.get(), monitor_endpoint_.c_str(), handshake_);

            OT_ASSERT(0 >= rc);

            return out;
        }())
        , monitor_([&] {
            auto out = Socket{
                ::zmq_socket(api_.Network().ZeroMQ(), ZMQ_PAIR), ::zmq_close};
            auto rc = ::zmq_setsockopt(
                out.get(), ZMQ_LINGER, &linger_, sizeof(linger_));

            OT_ASSERT(0 == rc);

            rc = ::zmq_connect(out.get(), monitor_endpoint_.c_str());

            OT_ASSERT(0 == rc);

            return out;
        }())
        , external_sub_([&] {
            auto out = Socket{
                ::zmq_socket(api_.Network().ZeroMQ(), ZMQ_SUB), ::zmq_close};
            auto rc = ::zmq_setsockopt(
                out.get(), ZMQ_LINGER, &linger_, sizeof(linger_));

            OT_ASSERT(0 == rc);

            rc = ::zmq_setsockopt(out.get(), ZMQ_SUBSCRIBE, "", 0);

            OT_ASSERT(0 == rc);

            return out;
        }())
        , internal_router_([&] {
            auto out = Socket{
                ::zmq_socket(api_.Network().ZeroMQ(), ZMQ_ROUTER), ::zmq_close};
            auto rc = ::zmq_setsockopt(
                out.get(), ZMQ_LINGER, &linger_, sizeof(linger_));

            OT_ASSERT(0 == rc);

            rc = ::zmq_bind(out.get(), endpoint_.c_str());

            OT_ASSERT(0 == rc);

            LogTrace()(OT_PRETTY_CLASS())("internal router bound to ")(
                endpoint_)
                .Flush();

            return out;
        }())
        , internal_sub_([&] {
            auto out = Socket{
                ::zmq_socket(api_.Network().ZeroMQ(), ZMQ_SUB), ::zmq_close};
            auto rc = ::zmq_setsockopt(
                out.get(), ZMQ_LINGER, &linger_, sizeof(linger_));

            OT_ASSERT(0 == rc);

            rc = ::zmq_setsockopt(out.get(), ZMQ_SUBSCRIBE, "", 0);

            OT_ASSERT(0 == rc);

            rc = ::zmq_connect(
                out.get(),
                api_.Endpoints().BlockchainSyncServerUpdated().c_str());

            OT_ASSERT(0 == rc);

            rc = ::zmq_connect(out.get(), api_.Endpoints().Shutdown().c_str());

            OT_ASSERT(0 == rc);

            return out;
        }())
        , servers_()
        , clients_()
        , providers_()
        , active_([&] {
            auto out = ActiveMap{};

            for (const auto& chain : opentxs::blockchain::DefinedChains()) {
                out.try_emplace(chain);
            }

            return out;
        }())
        , connected_servers_()
        , connected_count_(0)
        , running_(true)
        , thread_()
    {
    }

    ~Imp()
    {
        running_ = false;

        if (thread_.joinable()) { thread_.join(); }
    }

private:
    struct Server {
        Time last_;
        bool connected_;
        bool active_;
        bool waiting_;
        bool new_local_handler_;
        std::pmr::set<Chain> chains_;
        std::string publisher_;

        auto is_stalled() const noexcept -> bool
        {
            static constexpr auto limit = std::chrono::minutes{5};
            const auto wait = Clock::now() - last_;

            return (wait >= limit);
        }

        auto needs_query() noexcept -> bool
        {
            if (new_local_handler_) {
                new_local_handler_ = false;

                return true;
            }

            static constexpr auto zero = std::chrono::seconds{0};
            static constexpr auto timeout = std::chrono::seconds{30};
            static constexpr auto keepalive = std::chrono::minutes{2};
            const auto wait = Clock::now() - last_;

            if (zero > wait) { return true; }

            return (waiting_ || (!active_)) ? (wait >= timeout)
                                            : (wait >= keepalive);
        }

        Server() noexcept
            : last_()
            , connected_(false)
            , active_(false)
            , waiting_(false)
            , new_local_handler_(false)
            , chains_()
            , publisher_()
        {
        }

    private:
        Server(const Server&) = delete;
        Server(Server&&) = delete;
        auto operator=(const Server&) -> Server& = delete;
        auto operator=(Server&&) -> Server& = delete;
    };

    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;
    using OTSocket = opentxs::network::zeromq::socket::implementation::Socket;
    using ServerMap = std::pmr::map<std::string, Server>;
    using ChainMap = std::pmr::map<Chain, std::string>;
    using ProviderMap = std::pmr::map<Chain, std::pmr::set<std::string>>;
    using ActiveMap = std::pmr::map<Chain, std::atomic<std::size_t>>;
    using Message = opentxs::network::zeromq::Message;

    static constexpr int linger_{0};
    static constexpr int handover_{1};
    static constexpr int handshake_{ZMQ_EVENT_HANDSHAKE_SUCCEEDED};

    const api::Session& api_;
    const std::string endpoint_;
    const std::string monitor_endpoint_;
    Socket external_router_;
    Socket monitor_;
    Socket external_sub_;
    Socket internal_router_;
    Socket internal_sub_;
    ServerMap servers_;
    ChainMap clients_;
    ProviderMap providers_;
    ActiveMap active_;
    std::pmr::set<std::string> connected_servers_;
    std::atomic<std::size_t> connected_count_;
    std::atomic_bool running_;
    std::thread thread_;

    auto get_chain(Chain chain) const noexcept -> std::string
    {
        try {

            return clients_.at(chain);
        } catch (...) {

            return {};
        }
    }
    auto get_provider(Chain chain) const noexcept -> std::string
    {
        try {
            const auto& providers = providers_.at(chain);
            static auto rand = std::mt19937{std::random_device{}()};
            auto result = std::pmr::vector<std::string>{};
            std::sample(
                providers.begin(),
                providers.end(),
                std::back_inserter(result),
                1,
                rand);

            if (0 < result.size()) {

                return result.front();
            } else {

                return {};
            }
        } catch (...) {

            return {};
        }
    }

    auto first(const Blockchain& parent) noexcept -> void
    {
        for (const auto& ep : parent.GetSyncServers()) { process_server(ep); }
    }
    auto process(void* socket, bool monitor, bool internal) noexcept -> void
    {
        const auto msg = [&] {
            auto dummy = std::mutex{};
            auto lock = Lock{dummy};
            auto output = opentxs::network::zeromq::Message{};
            OTSocket::receive_message(lock, socket, output);

            return output;
        }();

        if (0 == msg.size()) {
            LogError()(OT_PRETTY_CLASS())("Dropping empty message").Flush();

            return;
        }

        if (monitor) {
            process_monitor(msg);
        } else if (internal) {
            process_internal(msg);
        } else {
            process_external(msg);
        }
    }
    auto process_external(const Message& msg) noexcept -> void
    {
        try {
            const auto body = msg.Body();

            if (0 == body.size()) {
                throw std::runtime_error{"Empty message body"};
            }

            const auto task = body.at(0).as<Task>();

            switch (task) {
                case Task::Ack:
                case Task::Reply:
                case Task::Push: {
                } break;
                case Task::Shutdown:
                case Task::Server:
                case Task::Register:
                case Task::Request:
                case Task::Processed:
                default: {
                    throw std::runtime_error{
                        std::string{"Unsupported task on external socket: "} +
                        std::to_string(static_cast<OTZMQWorkType>(task))};
                }
            }

            const auto endpoint = std::string{msg.at(0).Bytes()};
            auto& server = [&]() -> auto&
            {
                static auto blank = Server{};

                try {
                    auto& out = servers_.at(endpoint);
                    out.last_ = Clock::now();
                    out.waiting_ = false;

                    return out;
                } catch (...) {
                    // NOTE blank gets returned for messages that arrive on a
                    // subscribe socket rather than the router socket

                    return blank;
                }
            }
            ();
            const auto sync = api_.Factory().BlockchainSyncMessage(msg);
            const auto type = sync->Type();
            using Type = opentxs::network::p2p::MessageType;
            auto dummy = std::mutex{};
            auto lock = Lock{dummy};

            switch (type) {
                case Type::sync_ack: {
                    const auto& ack = sync->asAcknowledgement();
                    const auto& ep = ack.Endpoint();

                    if (server.publisher_.empty() && (false == ep.empty())) {
                        if (0 ==
                            ::zmq_connect(external_sub_.get(), ep.c_str())) {
                            server.publisher_ = ep;
                            LogDetail()("Subscribed to ")(
                                ep)(" for new block notifications")
                                .Flush();
                        } else {
                            LogError()(OT_PRETTY_CLASS())(
                                "failed to connect external subscriber to ")(ep)
                                .Flush();
                        }
                    }

                    for (const auto& state : ack.State()) {
                        const auto chain = state.Chain();
                        server.chains_.emplace(chain);
                        auto& providers = providers_[chain];
                        providers.emplace(endpoint);
                        active_.at(chain).store(providers.size());
                        LogVerbose()(endpoint)(" provides sync support for ")(
                            DisplayString(chain))
                            .Flush();
                        const auto& client = get_chain(chain);

                        if (client.empty()) { continue; }

                        namespace bcsync = opentxs::network::p2p;
                        auto data = [&] {
                            auto out = bcsync::StateData{};
                            const auto proto = [&] {
                                auto out = proto::BlockchainP2PChainState{};
                                state.Serialize(out);

                                return out;
                            }();

                            out.emplace_back(api_, proto);

                            return out;
                        }();
                        const auto notification =
                            factory::BlockchainSyncAcknowledgement(
                                std::move(data), ep);
                        auto msg = opentxs::network::zeromq::Message{};
                        msg.AddFrame(client);
                        msg.StartBody();

                        if (false == notification.Serialize(msg)) { OT_FAIL; }

                        OTSocket::send_message(
                            lock, internal_router_.get(), std::move(msg));
                    }

                    server.active_ = true;
                    connected_servers_.emplace(endpoint);
                    connected_count_.store(connected_servers_.size());
                } break;
                case Type::new_block_header:
                case Type::sync_reply: {
                    const auto& data = sync->asData();
                    const auto chain = data.State().Chain();
                    const auto identity = get_chain(chain);

                    if (identity.empty()) {
                        throw std::runtime_error{
                            std::string{"No active clients for "} +
                            DisplayString(chain)};
                    }

                    auto msg = opentxs::network::zeromq::Message{};
                    msg.AddFrame(identity);
                    msg.StartBody();

                    if (false == data.Serialize(msg)) { OT_FAIL; }

                    OTSocket::send_message(
                        lock, internal_router_.get(), std::move(msg));
                } break;
                default: {
                    throw std::runtime_error{
                        std::string{
                            "Unsupported message type on external socket: "} +
                        opentxs::print(type)};
                }
            }
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return;
        }
    }
    auto process_internal(const Message& msg) noexcept -> void
    {
        const auto body = msg.Body();

        if (0 == body.size()) { OT_FAIL; }

        const auto type = body.at(0).as<Task>();

        switch (type) {
            case Task::Shutdown: {
                running_ = false;
            } break;
            case Task::Server: {
                OT_ASSERT(2 < body.size());

                const auto endpoint = std::string{body.at(1).Bytes()};
                const auto added = body.at(2).as<bool>();

                if (added) {
                    // TODO limit the number of active endpoints in an
                    // intelligent way, attempting to ensure that all
                    // blockchains have at least one endpoint that can provide
                    // sync data
                    process_server(endpoint);
                } else {
                    // TODO decide whether or not to remove this server based on
                    // criteria such as whether or not this endpoint is the only
                    // provider of sync data for a particular chain
                }
            } break;
            case Task::Register: {
                const auto& identity = msg.at(0);

                OT_ASSERT(1 < body.size());

                const auto chain = body.at(1).as<Chain>();
                clients_[chain] = identity.Bytes();
                const auto& providers = providers_[chain];
                LogVerbose()(OT_PRETTY_CLASS())("querying ")(providers.size())(
                    " providers for ")(DisplayString(chain))
                    .Flush();

                for (const auto& endpoint : providers) {
                    auto& server = servers_.at(endpoint);
                    server.new_local_handler_ = true;
                }
            } break;
            case Task::Request: {
                OT_ASSERT(2 < body.size());

                const auto chain = body.at(1).as<Chain>();
                const auto provider = get_provider(chain);

                if (provider.empty()) {
                    LogError()(OT_PRETTY_CLASS())("no provider for ")(
                        DisplayString(chain))
                        .Flush();

                    return;
                }

                auto& server = servers_.at(provider);
                const auto request = [&] {
                    namespace otsync = opentxs::network::p2p;
                    const auto proto =
                        proto::Factory<proto::BlockchainP2PChainState>(
                            body.at(2));
                    auto states = [&] {
                        auto out = otsync::StateData{};
                        out.emplace_back(api_, proto);

                        return out;
                    }();

                    return factory::BlockchainSyncRequest(std::move(states));
                }();

                auto msg = opentxs::network::zeromq::Message{};
                msg.AddFrame(provider);
                msg.StartBody();

                if (false == request.Serialize(msg)) { OT_FAIL; }

                auto dummy = std::mutex{};
                auto lock = Lock{dummy};
                OTSocket::send_message(
                    lock, external_router_.get(), std::move(msg));
                server.last_ = Clock::now();
            } break;
            case Task::Ack:
            case Task::Reply:
            case Task::Push:
            case Task::Processed:
            default: {
                LogError()(OT_PRETTY_CLASS())(
                    "Unsupported message type on internal socket: ")(
                    static_cast<OTZMQWorkType>(type))
                    .Flush();

                OT_FAIL;
            }
        }
    }
    auto process_monitor(const Message& msg) noexcept -> void
    {
        OT_ASSERT(2u == msg.size());

        const auto event = [&] {
            auto out = int{};
            auto& frame = msg.at(0);

            OT_ASSERT(2u <= frame.size());

            std::memcpy(&out, frame.data(), 2u);

            return out;
        }();

        switch (event) {
            case handshake_: {
                const auto endpoint = std::string{msg.at(1).Bytes()};
                LogDetail()("Connected to sync server at ")(endpoint).Flush();
                servers_.at(endpoint).connected_ = true;
            } break;
            default: {
                LogError()(OT_PRETTY_CLASS())("Unexpected event type: ")(event)
                    .Flush();

                OT_FAIL;
            }
        }
    }
    auto process_server(const std::string& ep) noexcept -> void
    {
        if (0 != ::zmq_connect(external_router_.get(), ep.c_str())) {
            LogError()(OT_PRETTY_CLASS())("failed to connect router to ")(ep)
                .Flush();

            return;
        } else {
            servers_.try_emplace(ep);
            LogDetail()("Connecting to sync server at ")(ep).Flush();
        }
    }
    auto state_machine() noexcept -> void
    {
        auto dummy = std::mutex{};
        auto lock = Lock{dummy};

        for (auto& [endpoint, server] : servers_) {
            if (false == server.connected_) { continue; }

            if (server.needs_query()) {
                auto msg = opentxs::network::zeromq::Message{};
                msg.AddFrame(endpoint);
                msg.StartBody();
                const auto query = factory::BlockchainSyncQuery(0);

                if (false == query.Serialize(msg)) { OT_FAIL; }

                OTSocket::send_message(
                    lock, external_router_.get(), std::move(msg));
                server.last_ = Clock::now();
                server.waiting_ = true;
            }

            if (server.is_stalled()) {
                server.active_ = false;
                connected_servers_.erase(endpoint);
                connected_count_.store(connected_servers_.size());

                for (const auto& chain : server.chains_) {
                    auto& map = providers_[chain];
                    map.erase(endpoint);
                    active_.at(chain).store(map.size());
                }

                server.chains_.clear();

                if (false == server.publisher_.empty()) {
                    ::zmq_disconnect(
                        external_sub_.get(), server.publisher_.c_str());
                    server.publisher_.clear();
                }
            }
        }
    }
    auto thread() noexcept -> void
    {
        auto poll = [&] {
            auto output = std::array<::zmq_pollitem_t, 5>{};

            {
                auto& item = output.at(0);
                item.socket = monitor_.get();
                item.events = ZMQ_POLLIN;
            }
            {
                auto& item = output.at(1);
                item.socket = external_router_.get();
                item.events = ZMQ_POLLIN;
            }
            {
                auto& item = output.at(2);
                item.socket = external_sub_.get();
                item.events = ZMQ_POLLIN;
            }
            {
                auto& item = output.at(3);
                item.socket = internal_router_.get();
                item.events = ZMQ_POLLIN;
            }
            {
                auto& item = output.at(4);
                item.socket = internal_sub_.get();
                item.events = ZMQ_POLLIN;
            }

            return output;
        }();

        while (running_) {
            state_machine();
            constexpr auto timeout = std::chrono::milliseconds{1000};
            const auto events =
                ::zmq_poll(poll.data(), poll.size(), timeout.count());

            if (0 > events) {
                const auto error = ::zmq_errno();
                LogError()(OT_PRETTY_CLASS())(::zmq_strerror(error)).Flush();

                continue;
            } else if (0 == events) {

                continue;
            }

            for (auto i = std::size_t{0}; i < poll.size(); ++i) {
                auto& item = poll.at(i);

                if (0 == item.revents) { continue; }

                process(item.socket, (0 == i), (2 < i));
            }
        }
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

SyncClient::SyncClient(const api::Session& api) noexcept
    : imp_p_(std::make_unique<Imp>(api))
    , imp_(*imp_p_)
{
}

auto SyncClient::Endpoint() const noexcept -> const std::string&
{
    return imp_.Endpoint();
}

auto SyncClient::Init(const Blockchain& parent) noexcept -> void
{
    return imp_.Init(parent);
}

SyncClient::~SyncClient() = default;
}  // namespace opentxs::api::network::blockchain
