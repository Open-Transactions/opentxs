// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/ConnectionManager.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstddef>

#include "BoostAsio.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace opentxs::network::blockchain
{
struct TCPConnectionManager : virtual public ConnectionManager {
    const api::Session& api_;
    const Log& log_;
    const int id_;
    const network::asio::Endpoint endpoint_;
    const Space connection_id_;
    const std::size_t header_bytes_;
    const BodySize get_body_size_;
    std::promise<void> connection_id_promise_;
    std::shared_future<void> connection_id_future_;
    network::asio::Socket socket_;
    ByteArray header_;
    bool running_;

    auto address() const noexcept -> UnallocatedCString final
    {
        return endpoint_.GetMapped();
    }
    auto endpoint_data() const noexcept -> EndpointData final
    {
        return {address(), port()};
    }
    auto host() const noexcept -> UnallocatedCString final
    {
        return endpoint_.GetAddress();
    }
    auto port() const noexcept -> std::uint16_t final
    {
        return endpoint_.GetPort();
    }
    auto send() const noexcept -> zeromq::Message final { return {}; }
    auto style() const noexcept -> opentxs::network::blockchain::Transport final
    {
        return opentxs::network::blockchain::Transport::ipv6;
    }

    auto do_connect() noexcept
        -> std::pair<bool, std::optional<std::string_view>> override
    {
        OT_ASSERT(0 < connection_id_.size());

        log_()(OT_PRETTY_CLASS())("Connecting to ")(endpoint_.str()).Flush();
        socket_.Connect(reader(connection_id_));

        return std::make_pair(false, std::nullopt);
    }
    auto do_init() noexcept -> std::optional<std::string_view> final
    {
        return api_.Network().Asio().Internal().NotificationEndpoint();
    }
    auto is_initialized() const noexcept -> bool final
    {
        static constexpr auto zero = 0ns;
        static constexpr auto ready = std::future_status::ready;

        return (ready == connection_id_future_.wait_for(zero));
    }
    auto on_body(zeromq::Message&& message) noexcept
        -> std::optional<zeromq::Message> final
    {
        auto body = message.Body();

        OT_ASSERT(1 < body.size());

        run();

        return [&] {
            auto out = zeromq::Message{};
            out.StartBody();
            out.AddFrame(PeerJob::p2p);
            out.AddFrame(header_);
            out.AddFrame(std::move(body.at(1)));

            return out;
        }();
    }
    auto on_connect() noexcept -> void final
    {
        log_()(OT_PRETTY_CLASS())("Connect to ")(endpoint_.str())(" successful")
            .Flush();
        run();
    }
    auto on_header(zeromq::Message&& message) noexcept
        -> std::optional<zeromq::Message> final
    {
        auto body = message.Body();

        OT_ASSERT(1 < body.size());

        auto& header = body.at(1);
        const auto size = get_body_size_(header);

        if (0 < size) {
            header_.Assign(header.Bytes());
            receive(static_cast<OTZMQWorkType>(PeerJob::body), size);

            return std::nullopt;
        } else {
            run();

            return [&] {
                auto out = zeromq::Message{};
                out.StartBody();
                out.AddFrame(PeerJob::p2p);
                out.AddFrame(std::move(header));
                out.AddFrame();

                return out;
            }();
        }
    }
    auto on_init() noexcept -> zeromq::Message final
    {
        return [&] {
            auto out = MakeWork(WorkType::AsioRegister);
            out.AddFrame(id_);

            return out;
        }();
    }
    auto on_register(zeromq::Message&& message) noexcept -> void final
    {
        const auto body = message.Body();

        OT_ASSERT(1 < body.size());

        const auto& id = body.at(1);

        OT_ASSERT(0 < id.size());

        const auto* const start = static_cast<const std::byte*>(id.data());
        const_cast<Space&>(connection_id_).assign(start, start + id.size());

        OT_ASSERT(0 < connection_id_.size());

        try {
            connection_id_promise_.set_value();
        } catch (...) {
        }
    }
    auto receive(const OTZMQWorkType type, const std::size_t bytes) noexcept
        -> void
    {
        socket_.Receive(reader(connection_id_), type, bytes);
    }
    auto run() noexcept -> void
    {
        if (running_) {
            receive(static_cast<OTZMQWorkType>(PeerJob::header), header_bytes_);
        }
    }
    auto shutdown_external() noexcept -> void final
    {
        running_ = false;
        socket_.Close();
    }
    auto stop_external() noexcept -> void final
    {
        running_ = false;
        socket_.Close();
    }
    auto transmit(zeromq::Message&& message) noexcept
        -> std::optional<zeromq::Message> final
    {
        socket_.Transmit(reader(connection_id_), message.at(0).Bytes());

        return std::nullopt;
    }

    TCPConnectionManager(
        const api::Session& api,
        const Log& log,
        const int id,
        const Address& address,
        const std::size_t headerSize,
        BodySize&& gbs) noexcept
        : api_(api)
        , log_(log)
        , id_(id)
        , endpoint_(make_endpoint(address))
        , connection_id_()
        , header_bytes_(headerSize)
        , get_body_size_(std::move(gbs))
        , connection_id_promise_()
        , connection_id_future_(connection_id_promise_.get_future())
        , socket_(api_.Network().Asio().Internal().MakeSocket(endpoint_))
        , header_([&] {
            auto out = api_.Factory().Data();
            out.SetSize(headerSize);

            return out;
        }())
        , running_(true)
    {
        OT_ASSERT(get_body_size_);
    }

    ~TCPConnectionManager() override { socket_.Close(); }

protected:
    static auto make_endpoint(const Address& address) noexcept
        -> network::asio::Endpoint
    {
        using Type = network::asio::Endpoint::Type;

        switch (address.Type()) {
            case opentxs::network::blockchain::Transport::ipv6: {

                return {Type::ipv6, address.Bytes().Bytes(), address.Port()};
            }
            case opentxs::network::blockchain::Transport::ipv4: {

                return {Type::ipv4, address.Bytes().Bytes(), address.Port()};
            }
            default: {

                return {};
            }
        }
    }

    TCPConnectionManager(
        const api::Session& api,
        const Log& log,
        const int id,
        const std::size_t headerSize,
        network::asio::Endpoint&& endpoint,
        BodySize&& gbs,
        network::asio::Socket&& socket) noexcept
        : api_(api)
        , log_(log)
        , id_(id)
        , endpoint_(std::move(endpoint))
        , connection_id_()
        , header_bytes_(headerSize)
        , get_body_size_(std::move(gbs))
        , connection_id_promise_()
        , connection_id_future_(connection_id_promise_.get_future())
        , socket_(std::move(socket))
        , header_([&] {
            auto out = api_.Factory().Data();
            out.SetSize(headerSize);

            return out;
        }())
        , running_(true)
    {
        OT_ASSERT(get_body_size_);
    }
};

struct TCPIncomingConnectionManager final : public TCPConnectionManager {
    auto do_connect() noexcept
        -> std::pair<bool, std::optional<std::string_view>> final
    {
        return std::make_pair(false, std::nullopt);
    }

    TCPIncomingConnectionManager(
        const api::Session& api,
        const Log& log,
        const int id,
        const Address& address,
        const std::size_t headerSize,
        BodySize&& gbs,
        network::asio::Socket&& socket) noexcept
        : TCPConnectionManager(
              api,
              log,
              id,
              headerSize,
              make_endpoint(address),
              std::move(gbs),
              std::move(socket))
    {
    }

    ~TCPIncomingConnectionManager() final = default;
};

auto ConnectionManager::TCP(
    const api::Session& api,
    const Log& log,
    const int id,
    const Address& address,
    const std::size_t headerSize,
    BodySize&& gbs) noexcept -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<TCPConnectionManager>(
        api, log, id, address, headerSize, std::move(gbs));
}

auto ConnectionManager::TCPIncoming(
    const api::Session& api,
    const Log& log,
    const int id,
    const Address& address,
    const std::size_t headerSize,
    BodySize&& gbs,
    opentxs::network::asio::Socket&& socket) noexcept
    -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<TCPIncomingConnectionManager>(
        api, log, id, address, headerSize, std::move(gbs), std::move(socket));
}
}  // namespace opentxs::network::blockchain
