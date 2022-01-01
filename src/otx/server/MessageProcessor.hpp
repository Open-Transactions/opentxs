// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <utility>

#include "Proto.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "serialization/protobuf/ServerRequest.pb.h"

namespace opentxs
{
namespace api
{
namespace session
{
class Notary;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Thread;
}  // namespace internal

namespace socket
{
class Raw;
}  // namespace socket

class Frame;
}  // namespace zeromq
}  // namespace network

namespace server
{
class Server;
}  // namespace server

class OTPassword;
class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::server
{
class MessageProcessor final : Lockable
{
public:
    auto DropIncoming(const int count) const noexcept -> void;
    auto DropOutgoing(const int count) const noexcept -> void;

    auto cleanup() noexcept -> void;
    auto init(
        const bool inproc,
        const int port,
        const Secret& privkey) noexcept(false) -> void;
    auto Start() noexcept -> void;

    MessageProcessor(Server& server, const PasswordPrompt& reason) noexcept;

    ~MessageProcessor() final;

private:
    // connection identifier, old format
    using ConnectionData = std::pair<OTData, bool>;

    static constexpr auto zap_domain_{"opentxs-otx"};

    const api::session::Notary& api_;
    Server& server_;
    const PasswordPrompt& reason_;
    std::atomic<bool> running_;
    zmq::internal::Batch& zmq_batch_;
    zmq::socket::Raw& frontend_;
    zmq::socket::Raw& notification_;
    zmq::internal::Thread* zmq_thread_;
    const std::size_t frontend_id_;
    std::thread thread_;
    mutable std::mutex counter_lock_;
    mutable int drop_incoming_;
    mutable int drop_outgoing_;
    std::pmr::map<OTNymID, ConnectionData> active_connections_;
    mutable std::shared_mutex connection_map_lock_;

    static auto get_connection(
        const network::zeromq::Message& incoming) noexcept -> OTData;

    auto extract_proto(const network::zeromq::Frame& incoming) const noexcept
        -> proto::ServerRequest;

    auto associate_connection(
        const bool oldFormat,
        const identifier::Nym& nymID,
        const Data& connection) noexcept -> void;
    auto pipeline(zmq::Message&& message) noexcept -> void;
    auto process_backend(
        const bool tagged,
        network::zeromq::Message&& incoming) noexcept
        -> network::zeromq::Message;
    auto process_command(
        const proto::ServerRequest& request,
        identifier::Nym& nymID) noexcept -> bool;
    auto process_frontend(network::zeromq::Message&& incoming) noexcept -> void;
    auto process_internal(network::zeromq::Message&& incoming) noexcept -> void;
    auto process_legacy(
        const Data& id,
        const bool tagged,
        network::zeromq::Message&& incoming) noexcept -> void;
    auto process_message(
        const std::string& messageString,
        std::string& reply) noexcept -> bool;
    auto process_notification(network::zeromq::Message&& incoming) noexcept
        -> void;
    auto process_proto(
        const Data& id,
        const bool oldFormat,
        network::zeromq::Message&& incoming) noexcept -> void;
    auto query_connection(const identifier::Nym& nymID) noexcept
        -> const ConnectionData&;
    auto run() noexcept -> void;

    MessageProcessor() = delete;
    MessageProcessor(const MessageProcessor&) = delete;
    MessageProcessor(MessageProcessor&&) = delete;
    auto operator=(const MessageProcessor&) -> MessageProcessor& = delete;
    auto operator=(MessageProcessor&&) -> MessageProcessor& = delete;
};
}  // namespace opentxs::server
