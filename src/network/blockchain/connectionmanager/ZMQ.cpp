// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/ConnectionManager.hpp"  // IWYU pragma: associated

#include <chrono>

#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::network::blockchain
{
struct ZMQConnectionManager : virtual public ConnectionManager {
    const api::Session& api_;
    const Log& log_;
    const int id_;
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
    auto host() const noexcept -> UnallocatedCString final
    {
        return endpoint_.first;
    }
    auto port() const noexcept -> std::uint16_t final
    {
        return endpoint_.second;
    }
    auto send() const noexcept -> zeromq::Message final
    {
        return network::zeromq::tagged_message(PeerJob::p2p);
    }
    auto style() const noexcept -> opentxs::network::blockchain::Transport final
    {
        return opentxs::network::blockchain::Transport::zmq;
    }

    auto do_connect() noexcept
        -> std::pair<bool, std::optional<std::string_view>> override
    {
        log_(OT_PRETTY_CLASS())("Connecting to ")(zmq_).Flush();

        return std::make_pair<bool, std::optional<std::string_view>>(
            false, zmq_);
    }
    auto do_init() noexcept -> std::optional<std::string_view> override
    {
        try {
            init_promise_.set_value();
        } catch (...) {
        }

        return std::nullopt;
    }
    auto is_initialized() const noexcept -> bool final
    {
        static constexpr auto zero = 0ns;
        static constexpr auto ready = std::future_status::ready;

        return (ready == init_future_.wait_for(zero));
    }
    auto on_body(zeromq::Message&&) noexcept
        -> std::optional<zeromq::Message> final
    {
        OT_FAIL;
    }
    auto on_connect() noexcept -> void override {}
    auto on_header(zeromq::Message&&) noexcept
        -> std::optional<zeromq::Message> final
    {
        OT_FAIL;
    }
    auto on_init() noexcept -> zeromq::Message override { OT_FAIL; }
    auto on_register(zeromq::Message&&) noexcept -> void override { OT_FAIL; }
    auto shutdown_external() noexcept -> void final {}
    auto stop_external() noexcept -> void final {}
    auto transmit(zeromq::Message&& message) noexcept
        -> std::optional<zeromq::Message> final
    {
        return std::move(message);
    }

    ZMQConnectionManager(
        const api::Session& api,
        const Log& log,
        const int id,
        const Address& address,
        const std::size_t headerSize,
        std::string_view zmq) noexcept
        : api_(api)
        , log_(log)
        , id_(id)
        , endpoint_(UnallocatedCString{address.Bytes().Bytes()}, address.Port())
        , zmq_([&]() -> decltype(zmq_) {
            if (zmq.empty()) {

                return endpoint_.first + ':' + std::to_string(endpoint_.second);
            } else {

                return decltype(zmq_){zmq};
            }
        }())
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
    auto do_connect() noexcept
        -> std::pair<bool, std::optional<std::string_view>> final
    {
        return std::make_pair(true, std::nullopt);
    }
    auto do_init() noexcept -> std::optional<std::string_view> final
    {
        log_(OT_PRETTY_CLASS())("Accepting incoming connection from ")(zmq_)
            .Flush();

        return zmq_;
    }
    auto on_init() noexcept -> zeromq::Message final
    {
        return [&] {
            auto out = MakeWork(PeerJob::registration);
            out.AddFrame(id_);

            return out;
        }();
    }
    auto on_register(zeromq::Message&&) noexcept -> void final
    {
        try {
            init_promise_.set_value();
        } catch (...) {
        }
    }

    ZMQIncomingConnectionManager(
        const api::Session& api,
        const Log& log,
        const int id,
        const Address& address,
        const std::size_t headerSize,
        std::string_view zmq) noexcept
        : ZMQConnectionManager(api, log, id, address, headerSize, zmq)
    {
    }

    ~ZMQIncomingConnectionManager() final = default;
};

auto ConnectionManager::ZMQ(
    const api::Session& api,
    const Log& log,
    const int id,
    const Address& address,
    const std::size_t headerSize) noexcept -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<ZMQConnectionManager>(
        api, log, id, address, headerSize, "");
}

auto ConnectionManager::ZMQIncoming(
    const api::Session& api,
    const opentxs::blockchain::node::Manager& node,
    const Log& log,
    const int id,
    const Address& address,
    const std::size_t headerSize) noexcept -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<ZMQIncomingConnectionManager>(
        api,
        log,
        id,
        address,
        headerSize,
        node.Internal().Endpoints().peer_manager_router_);
}
}  // namespace opentxs::network::blockchain
