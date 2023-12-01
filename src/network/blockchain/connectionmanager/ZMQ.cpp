// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/ConnectionManager.hpp"  // IWYU pragma: associated

#include <chrono>
#include <future>
#include <stdexcept>
#include <type_traits>

#include "BoostAsio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/asio/Types.hpp"
#include "internal/network/blockchain/Address.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/socket/Sender.hpp"  // IWYU pragma: keep
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.internal.hpp"

namespace opentxs::network::blockchain
{
struct ZMQConnectionManager : virtual public ConnectionManager {
    const api::Session& api_;
    const Log& log_;
    const int id_;
    const UnallocatedCString zmq_;
    const std::size_t header_bytes_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_future_;

    auto send() const noexcept -> zeromq::Message final
    {
        return network::zeromq::tagged_message(PeerJob::p2p, true);
    }

    auto do_connect() noexcept
        -> std::pair<bool, std::optional<std::string_view>> override
    {
        log_()("Connecting to ")(zmq_).Flush();

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
        LogAbort()().Abort();
    }
    auto on_connect() noexcept -> void override {}
    auto on_header(zeromq::Message&&) noexcept
        -> std::optional<zeromq::Message> final
    {
        LogAbort()().Abort();
    }
    auto on_init() noexcept -> zeromq::Message override
    {
        LogAbort()().Abort();
    }
    auto on_register(zeromq::Message&&) noexcept -> void override
    {
        LogAbort()().Abort();
    }
    auto shutdown_external() noexcept -> void final {}
    auto stop_external() noexcept -> std::optional<zeromq::Message> override
    {
        return std::nullopt;
    }
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
        , zmq_([&]() -> decltype(zmq_) {
            // NOLINTBEGIN(clang-analyzer-cplusplus.StringChecker)
            if (zmq.empty()) {
                auto out = std::remove_const<decltype(zmq_)>::type{};

                try {
                    using enum Transport;
                    using namespace boost::asio;
                    const auto serialized = address.Bytes();

                    switch (address.Subtype()) {
                        case ipv4:
                        case ipv6:
                        case cjdns: {
                            const auto addr =
                                asio::address_from_binary(serialized.Bytes());

                            if (addr.has_value()) {
                                out.append("tcp://");
                                out.append(addr->to_string());
                                out.append(":");
                                out.append(std::to_string(address.Port()));
                            } else {
                                const auto error =
                                    UnallocatedCString{
                                        "unable to encode address "}
                                        .append(address.Display());

                                throw std::runtime_error(error);
                            }
                        } break;
                        case zmq: {
                            out.append(serialized.Bytes());
                            out.append(":");
                            out.append(std::to_string(address.Port()));
                        } break;
                        default: {
                            const auto error =
                                UnallocatedCString{"unknown subtype "}.append(
                                    print(address.Subtype()));

                            throw std::runtime_error{error};
                        }
                    }
                } catch (const std::exception& e) {
                    LogError()()(e.what()).Flush();
                }

                return out;
            } else {

                return decltype(zmq_){zmq};
            }
            // NOLINTEND(clang-analyzer-cplusplus.StringChecker)
        }())
        , header_bytes_(headerSize)
        , init_promise_()
        , init_future_(init_promise_.get_future())
    {
    }

    ~ZMQConnectionManager() override = default;
};

struct ZMQIncomingConnectionManager final : public ZMQConnectionManager {
    auto do_connect() noexcept
        -> std::pair<bool, std::optional<std::string_view>> final
    {
        return std::make_pair(true, std::nullopt);
    }
    auto do_init() noexcept -> std::optional<std::string_view> final
    {
        log_()("Accepting incoming connection from ")(zmq_).Flush();

        return zmq_;
    }
    auto on_init() noexcept -> zeromq::Message final
    {
        return [&] {
            using enum network::otdht::NodeJob;
            auto out = MakeWork(connect_peer);
            out.AddFrame(chain_);
            out.AddFrame(cookie_);

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
    auto stop_external() noexcept -> std::optional<zeromq::Message> final
    {
        return [&] {
            using enum network::otdht::NodeJob;
            auto out = MakeWork(disconnect_peer);
            out.AddFrame(chain_);
            out.AddFrame(cookie_);

            return out;
        }();
    }

    ZMQIncomingConnectionManager(
        const api::Session& api,
        const Log& log,
        const int id,
        const Address& address,
        const std::size_t headerSize,
        std::string_view zmq) noexcept
        : ZMQConnectionManager(api, log, id, address, headerSize, zmq)
        , chain_(address.Chain())
        , cookie_(address.Internal().Cookie())
    {
    }

    ~ZMQIncomingConnectionManager() final = default;

private:
    const opentxs::blockchain::Type chain_;
    const ByteArray cookie_;
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

// NOLINTBEGIN(clang-analyzer-cplusplus.StringChecker)
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
        api.Endpoints().Internal().OTDHTNodeRouter());
}
// NOLINTEND(clang-analyzer-cplusplus.StringChecker)
}  // namespace opentxs::network::blockchain
