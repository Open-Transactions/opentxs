// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <atomic>
#include <cstdint>
#include <deque>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "core/Executor.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/p2p/Peer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace asio = boost::asio;
namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::implementation
{
using tcp = asio::ip::tcp;

class Peer : virtual public internal::Peer, public Executor<Peer>
{
public:
    using SendStatus = std::future<bool>;

    auto AddressID() const noexcept -> OTIdentifier final
    {
        return address_.ID();
    }
    auto Connected() const noexcept -> ConnectionStatus final
    {
        return connected_;
    }
    auto HandshakeComplete() const noexcept -> Handshake final
    {
        return handshake_;
    }
    auto Shutdown() noexcept -> std::shared_future<void> final;

    ~Peer() override;

protected:
    using SendResult = std::pair<boost::system::error_code, std::size_t>;
    using SendPromise = std::promise<SendResult>;
    using SendFuture = std::future<SendResult>;
    using Task = client::internal::PeerManager::Task;

    struct Address {
        using pointer = std::unique_ptr<internal::Address>;

        auto Bytes() const noexcept -> OTData;
        auto Chain() const noexcept -> blockchain::Type;
        auto Display() const noexcept -> std::string;
        auto ID() const noexcept -> OTIdentifier;
        auto Port() const noexcept -> std::uint16_t;
        auto Services() const noexcept -> std::set<Service>;
        auto Type() const noexcept -> Network;

        auto UpdateServices(const std::set<p2p::Service>& services) noexcept
            -> pointer;
        auto UpdateTime(const Time& time) noexcept -> pointer;

        Address(pointer address) noexcept;

    private:
        mutable std::mutex lock_;
        pointer address_;
    };

    struct DownloadPeers {
        auto get() const noexcept -> Time;

        void Bump() noexcept;

        DownloadPeers() noexcept;

    private:
        Time downloaded_;
    };

    struct Subscriptions {
        using value_type = std::vector<Task>;

        auto Push(value_type& tasks) noexcept -> void;
        auto Pop() noexcept -> value_type;

    private:
        std::mutex lock_{};
        value_type tasks_{};
    };

    const client::internal::Network& network_;
    const client::internal::PeerManager& manager_;
    const tcp::endpoint endpoint_;
    SendPromise send_promise_;
    SendFuture send_future_;
    Address address_;
    DownloadPeers download_peers_;
    OTData header_;
    bool outgoing_handshake_;
    bool incoming_handshake_;
    bool verify_complete_;
    bool started_subscribe_;
    Subscriptions subscribe_;

    bool verifying() noexcept;
    void check_handshake() noexcept;
    void check_subscribe() noexcept;
    void check_verify() noexcept;
    void disconnect() noexcept;
    auto local_endpoint() noexcept -> tcp::socket::endpoint_type;
    // NOTE call init in every final child class constructor
    void init() noexcept;
    virtual void ping() noexcept = 0;
    virtual void pong() noexcept = 0;
    virtual void request_addresses() noexcept = 0;
    virtual void request_block(zmq::Message& message) noexcept = 0;
    virtual void request_headers() noexcept = 0;
    auto send(OTData message) noexcept -> SendStatus;
    auto state_machine() noexcept -> bool;
    void update_address_services(
        const std::set<p2p::Service>& services) noexcept;

    Peer(
        const api::internal::Core& api,
        const client::internal::Network& network,
        const client::internal::PeerManager& manager,
        const blockchain::client::internal::IO& io,
        const int id,
        const std::string& shutdown,
        const std::size_t headerSize,
        const std::size_t bodySize,
        std::unique_ptr<internal::Address> address) noexcept;

private:
    friend Executor<Peer>;

    enum class State : std::uint8_t {
        Handshake,
        Verify,
        Subscribe,
        Run,
        Shutdown,
    };

    struct Activity {
        auto get() const noexcept -> Time;

        void Bump() noexcept;

        Activity() noexcept;

    private:
        mutable std::mutex lock_;
        Time activity_;
    };

    struct SendPromises {
        void Break();
        auto NewPromise() -> std::pair<std::future<bool>, int>;
        void SetPromise(const int promise, const bool value);

        SendPromises() noexcept;

    private:
        std::mutex lock_;
        int counter_;
        std::map<int, std::promise<bool>> map_;
    };

    const std::size_t header_bytes_;
    const int id_;
    const Space connection_id_;
    const std::string shutdown_endpoint_;
    const blockchain::client::internal::IO& context_;
    tcp::socket socket_;
    OTData outgoing_message_;
    std::promise<void> connection_id_promise_;
    std::promise<bool> connection_promise_;
    std::shared_future<bool> connected_;
    std::promise<void> handshake_promise_;
    std::promise<void> verify_promise_;
    Handshake handshake_;
    Verify verify_;
    SendPromises send_promises_;
    Activity activity_;
    mutable std::atomic<State> state_;
    OTZMQListenCallback cb_;
    OTZMQDealerSocket dealer_;

    static auto make_buffer(const std::size_t size) noexcept -> OTData;
    static auto make_endpoint(
        const Network type,
        const Data& bytes,
        const std::uint16_t port) noexcept -> tcp::endpoint;

    auto get_activity() const noexcept -> Time;
    virtual auto get_body_size(const zmq::Frame& header) const noexcept
        -> std::size_t = 0;

    void break_promises() noexcept;
    void check_activity() noexcept;
    void check_download_peers() noexcept;
    void connect() noexcept;
    void handshake() noexcept;
    void init_send_promise() noexcept;
    void pipeline(zmq::Message& message) noexcept;
    void pipeline_d(zmq::Message& message) noexcept;
    virtual void process_message(const zmq::Message& message) noexcept = 0;
    void process_state_machine() noexcept;
    virtual void request_cfheaders(zmq::Message& message) noexcept = 0;
    virtual void request_cfilter(zmq::Message& message) noexcept = 0;
    void run() noexcept;
    void shutdown(std::promise<void>& promise) noexcept;
    virtual void start_handshake() noexcept = 0;
    virtual void start_verify() noexcept = 0;
    void subscribe() noexcept;
    void subscribe_work() noexcept;
    void transmit(zmq::Message& message) noexcept;
    void update_address_activity() noexcept;
    void verify() noexcept;

    Peer() = delete;
    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer &&) -> Peer& = delete;
};
}  // namespace opentxs::blockchain::p2p::implementation
