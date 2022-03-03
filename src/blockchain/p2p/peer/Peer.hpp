// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <robin_hood.h>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <queue>
#include <type_traits>
#include <utility>

#include "blockchain/p2p/peer/Activity.hpp"
#include "blockchain/p2p/peer/Address.hpp"
#include "blockchain/p2p/peer/ConnectionManager.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Peer.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace peer
{
class ConnectionManager;
}  // namespace peer
}  // namespace p2p

namespace node
{
namespace internal
{
struct Mempool;
}  // namespace internal
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace asio
{
class Socket;
}  // namespace asio

namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::implementation
{
class Peer : virtual public internal::Peer, public Worker<Peer, api::Session>
{
public:
    using SendStatus = std::future<bool>;
    using Task = node::internal::PeerManager::Task;

    auto AddressID() const noexcept -> OTIdentifier final
    {
        return address_.ID();
    }
    auto Connected() const noexcept -> ConnectionStatus final
    {
        return state_.connect_.future_;
    }
    virtual auto get_body_size(const zmq::Frame& header) const noexcept
        -> std::size_t = 0;
    auto HandshakeComplete() const noexcept -> Handshake final
    {
        return state_.handshake_.future_;
    }

    auto on_connect() noexcept -> void;
    auto on_init() noexcept -> void;
    virtual auto process_message(zmq::Message&& message) noexcept -> void = 0;
    auto Shutdown() noexcept -> std::shared_future<void> final;

    ~Peer() override;

protected:
    using SendFuture = std::future<SendResult>;
    using KnownHashes = robin_hood::unordered_flat_set<UnallocatedCString>;

    enum class State {
        Init,
        Connect,
        Listening,
        Handshake,
        Verify,
        Subscribe,
        Run,
        Shutdown,
    };

    struct DownloadPeers {
        auto get() const noexcept -> Time;

        void Bump() noexcept;

        DownloadPeers() noexcept;

    private:
        Time downloaded_;
    };

    template <typename ValueType>
    struct StateData {
        std::atomic<State>& state_;
        bool first_run_;
        Time started_;
        std::atomic_bool first_action_;
        std::atomic_bool second_action_;
        std::promise<ValueType> promise_;
        std::shared_future<ValueType> future_;

        auto done() const noexcept -> bool
        {
            try {
                static constexpr auto zero = 0ns;
                static constexpr auto ready = std::future_status::ready;

                return ready == future_.wait_for(zero);
            } catch (...) {

                return false;
            }
        }

        auto break_promise() noexcept -> void { promise_ = {}; }
        auto run(
            const std::chrono::seconds limit,
            SimpleCallback firstAction,
            const State nextState) noexcept -> bool
        {
            if ((false == first_run_) && firstAction) {
                first_run_ = true;
                started_ = Clock::now();
                firstAction();

                return false;
            }

            auto disconnect{true};

            if (done()) {
                disconnect = false;
                state_.store(nextState);
            } else {
                disconnect = ((Clock::now() - started_) > limit);
            }

            if (disconnect) {
                LogVerbose()("State transition timeout exceeded.").Flush();
            }

            return disconnect;
        }

        StateData(std::atomic<State>& state) noexcept
            : state_(state)
            , first_run_(false)
            , started_()
            , first_action_(false)
            , second_action_(false)
            , promise_()
            , future_(promise_.get_future())
        {
        }
    };

    struct States {
        std::atomic<State> value_;
        StateData<bool> connect_;
        StateData<void> handshake_;
        StateData<void> verify_;

        auto break_promises() noexcept -> void
        {
            connect_.break_promise();
            handshake_.break_promise();
            verify_.break_promise();
        }

        States() noexcept
            : value_(State::Init)
            , connect_(value_)
            , handshake_(value_)
            , verify_(value_)
        {
        }
    };

    const node::internal::Network& network_;
    const node::internal::FilterOracle& filter_;
    const node::internal::BlockOracle& block_;
    const node::internal::PeerManager& manager_;
    const node::internal::Mempool& mempool_;
    const blockchain::Type chain_;
    const UnallocatedCString display_chain_;
    std::atomic_bool header_probe_;
    std::atomic_bool cfilter_probe_;
    peer::Address address_;
    DownloadPeers download_peers_;
    States state_;
    node::CfheaderJob cfheader_job_;
    node::CfilterJob cfilter_job_;
    node::BlockJob block_job_;
    KnownHashes known_transactions_;

    auto connection() const noexcept -> const peer::ConnectionManager&
    {
        return *connection_;
    }

    virtual auto broadcast_block(zmq::Message&& message) noexcept -> void = 0;
    virtual auto broadcast_inv_transaction(ReadView txid) noexcept -> void = 0;
    virtual auto broadcast_transaction(zmq::Message&& message) noexcept
        -> void = 0;
    auto check_handshake() noexcept -> void;
    auto check_verify() noexcept -> void;
    auto disconnect() noexcept -> void;
    // NOTE call init in every final child class constructor
    auto init() noexcept -> void;
    virtual auto ping() noexcept -> void = 0;
    virtual auto pong(bitcoin::Nonce) noexcept -> void = 0;
    virtual auto request_addresses() noexcept -> void = 0;
    virtual auto request_block(zmq::Message&& message) noexcept -> void = 0;
    virtual auto request_blocks() noexcept -> void = 0;
    virtual auto request_headers() noexcept -> void = 0;
    virtual auto request_mempool() noexcept -> void = 0;
    auto reset_block_job() noexcept -> void;
    auto reset_cfheader_job() noexcept -> void;
    auto reset_cfilter_job() noexcept -> void;
    auto send(std::pair<zmq::Frame, zmq::Frame>&& data) noexcept -> SendStatus;
    auto update_address_services(
        const UnallocatedSet<p2p::Service>& services) noexcept -> void;
    auto verifying() noexcept -> bool
    {
        return (State::Verify == state_.value_.load());
    }

    Peer(
        const api::Session& api,
        const node::internal::Config& config,
        const node::internal::Mempool& mempool,
        const node::internal::Network& network,
        const node::internal::FilterOracle& filter,
        const node::internal::BlockOracle& block,
        const node::internal::PeerManager& manager,
        const int id,
        const UnallocatedCString& shutdown,
        const std::size_t headerSize,
        const std::size_t bodySize,
        std::unique_ptr<internal::Address> address) noexcept;

private:
    friend Worker<Peer, api::Session>;

    struct SendPromises {
        void Break();
        auto NewPromise() -> std::pair<std::future<bool>, int>;
        void SetPromise(const int promise, const bool value);

        SendPromises() noexcept;

    private:
        std::mutex lock_;
        int counter_;
        UnallocatedMap<int, std::promise<bool>> map_;
    };

    static constexpr auto peer_download_interval_ = std::chrono::minutes{10};

    const Time init_start_;
    const bool verify_filter_checkpoint_;
    const int id_;
    const UnallocatedCString shutdown_endpoint_;
    const std::size_t untrusted_connection_id_;
    std::unique_ptr<peer::ConnectionManager> connection_;
    SendPromises send_promises_;
    peer::Activity activity_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    static auto init_connection_manager(
        const api::Session& api,
        const int id,
        const node::internal::PeerManager& manager,
        Peer& parent,
        const std::atomic<bool>& running,
        const peer::Address& address,
        const std::size_t headerSize) noexcept
        -> std::unique_ptr<peer::ConnectionManager>;

    auto get_activity() const noexcept -> Time;

    auto activity_timeout() noexcept -> void;
    auto break_promises() noexcept -> void;
    auto check_download_peers() noexcept -> void;
    auto check_init() noexcept -> void;
    auto check_jobs() noexcept -> void;
    auto connect() noexcept -> void;
    auto pipeline(zmq::Message&& message) noexcept -> void;
    auto process_mempool(const zmq::Message& message) noexcept -> void;
    auto process_state_machine() noexcept -> void;
    virtual auto request_cfheaders() noexcept -> void = 0;
    virtual auto request_cfilter() noexcept -> void = 0;
    virtual auto request_checkpoint_block_header() noexcept -> void = 0;
    virtual auto request_checkpoint_filter_header() noexcept -> void = 0;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    virtual auto start_handshake() noexcept -> void = 0;
    auto state_machine() noexcept -> bool;
    auto start_verify() noexcept -> void;
    auto subscribe() noexcept -> void;
    auto transmit(zmq::Message&& message) noexcept -> void;
    auto update_address_activity() noexcept -> void;
    auto verify() noexcept -> void;

    Peer() = delete;
    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer&&) -> Peer& = delete;
};
}  // namespace opentxs::blockchain::p2p::implementation
