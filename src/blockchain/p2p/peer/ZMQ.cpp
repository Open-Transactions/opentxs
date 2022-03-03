// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "blockchain/p2p/peer/ConnectionManager.hpp"  // IWYU pragma: associated

#include <chrono>

#include "blockchain/p2p/peer/Address.hpp"
#include "blockchain/p2p/peer/Peer.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::p2p::peer
{
struct ZMQConnectionManager : virtual public ConnectionManager {
    const api::Session& api_;
    const int id_;
    p2p::implementation::Peer& parent_;
    network::zeromq::Pipeline& pipeline_;
    const std::atomic<bool>& running_;
    EndpointData endpoint_;
    const UnallocatedCString zmq_;
    const std::size_t header_bytes_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_future_;

    auto address() const noexcept -> UnallocatedCString final
    {
        return "::1/128";
    }
    auto endpoint_data() const noexcept -> EndpointData final
    {
        return endpoint_;
    }
    auto filter(const Task type) const noexcept -> bool override
    {
        return Task::P2P == type;
    }
    auto host() const noexcept -> UnallocatedCString final
    {
        return endpoint_.first;
    }
    auto port() const noexcept -> std::uint16_t final
    {
        return endpoint_.second;
    }
    auto style() const noexcept -> p2p::Network final
    {
        return p2p::Network::zmq;
    }

    auto connect() noexcept -> void override
    {
        LogVerbose()(OT_PRETTY_CLASS())("Connecting to ")(zmq_).Flush();
        pipeline_.ConnectDealer(zmq_, [id = id_](auto) {
            const network::zeromq::SocketID header = id;
            auto out = zmq::Message{};
            out.AddFrame(header);
            out.StartBody();
            out.AddFrame(Task::Connect);

            return out;
        });
    }
    auto init() noexcept -> void override
    {
        try {
            init_promise_.set_value();
        } catch (...) {
        }

        parent_.on_init();
    }
    auto is_initialized() const noexcept -> bool final
    {
        static constexpr auto zero = 0ns;
        static constexpr auto ready = std::future_status::ready;

        return (ready == init_future_.wait_for(zero));
    }
    auto on_body(zmq::Message&&) noexcept -> void final { OT_FAIL; }
    auto on_connect(zmq::Message&&) noexcept -> void override
    {
        parent_.on_connect();
    }
    auto on_header(zmq::Message&&) noexcept -> void final { OT_FAIL; }
    auto on_init(zmq::Message&&) noexcept -> void override { OT_FAIL; }
    auto on_register(zmq::Message&&) noexcept -> void override { OT_FAIL; }
    auto shutdown_external() noexcept -> void final {}
    auto stop_external() noexcept -> void final {}
    auto transmit(
        zmq::Frame&& header,
        zmq::Frame&& payload,
        std::unique_ptr<SendPromise> promise) noexcept -> void final
    {
        OT_ASSERT(header_bytes_ <= header.size());

        const auto sent = pipeline_.Send([&] {
            auto out = network::zeromq::tagged_message(Task::P2P);
            out.AddFrame(std::move(header));
            out.AddFrame(std::move(payload));

            return out;
        }());

        try {
            if (promise) { promise->set_value(sent); }
        } catch (...) {
        }
    }

    ZMQConnectionManager(
        const api::Session& api,
        const int id,
        p2p::implementation::Peer& parent,
        network::zeromq::Pipeline& pipeline,
        const std::atomic<bool>& running,
        const Address& address,
        const std::size_t headerSize) noexcept
        : api_(api)
        , id_(id)
        , parent_(parent)
        , pipeline_(pipeline)
        , running_(running)
        , endpoint_(address.Bytes()->str(), address.Port())
        , zmq_(endpoint_.first + ':' + std::to_string(endpoint_.second))
        , header_bytes_(headerSize)
        , init_promise_()
        , init_future_(init_promise_.get_future())
    {
    }

    ~ZMQConnectionManager() override
    {
        stop_external();
        shutdown_external();
    }
};

struct ZMQIncomingConnectionManager final : public ZMQConnectionManager {
    auto connect() noexcept -> void final { parent_.on_connect(); }
    auto filter(const Task) const noexcept -> bool final { return true; }
    auto init() noexcept -> void final
    {
        pipeline_.ConnectDealer(zmq_, [id = id_](auto) {
            const network::zeromq::SocketID header = id;
            auto out = zmq::Message{};
            out.AddFrame(header);
            out.StartBody();
            out.AddFrame(Task::Init);

            return out;
        });
    }
    auto on_init(zmq::Message&&) noexcept -> void final
    {
        pipeline_.Send([&] {
            auto out = MakeWork(Task::Register);
            out.AddFrame(id_);

            return out;
        }());
    }
    auto on_register(zmq::Message&&) noexcept -> void final
    {
        try {
            init_promise_.set_value();
        } catch (...) {
        }

        parent_.on_init();
    }

    ZMQIncomingConnectionManager(
        const api::Session& api,
        const int id,
        p2p::implementation::Peer& parent,
        network::zeromq::Pipeline& pipeline,
        const std::atomic<bool>& running,
        const Address& address,
        const std::size_t headerSize) noexcept
        : ZMQConnectionManager(
              api,
              id,
              parent,
              pipeline,
              running,
              address,
              headerSize)
    {
    }

    ~ZMQIncomingConnectionManager() final = default;
};

auto ConnectionManager::ZMQ(
    const api::Session& api,
    const int id,
    p2p::implementation::Peer& parent,
    network::zeromq::Pipeline& pipeline,
    const std::atomic<bool>& running,
    const Address& address,
    const std::size_t headerSize) noexcept -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<ZMQConnectionManager>(
        api, id, parent, pipeline, running, address, headerSize);
}

auto ConnectionManager::ZMQIncoming(
    const api::Session& api,
    const int id,
    p2p::implementation::Peer& parent,
    network::zeromq::Pipeline& pipeline,
    const std::atomic<bool>& running,
    const Address& address,
    const std::size_t headerSize) noexcept -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<ZMQIncomingConnectionManager>(
        api, id, parent, pipeline, running, address, headerSize);
}
}  // namespace opentxs::blockchain::p2p::peer
