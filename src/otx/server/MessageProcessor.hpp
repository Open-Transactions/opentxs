// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ServerRequest.pb.h>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <utility>

#include "internal/network/zeromq/Handle.hpp"
#include "internal/otx/server/MessageProcessor.hpp"
#include "internal/util/Lockable.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Notary;
}  // namespace session
}  // namespace api

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

class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::server
{
class OPENTXS_NO_EXPORT MessageProcessor::Imp final : Lockable
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

    Imp(Server& server, const PasswordPrompt& reason) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final;

private:
    // connection identifier, old format
    using ConnectionData = std::pair<network::zeromq::Envelope, bool>;

    static constexpr auto zap_domain_{"opentxs-otx"};

    const api::session::Notary& api_;
    Server& server_;
    const PasswordPrompt& reason_;
    std::atomic<bool> running_;
    zmq::internal::Handle zmq_handle_;
    zmq::internal::Batch& zmq_batch_;
    zmq::socket::Raw& frontend_;
    zmq::socket::Raw& notification_;
    zmq::internal::Thread* zmq_thread_;
    const std::size_t frontend_id_;
    std::thread thread_;
    mutable std::mutex counter_lock_;
    mutable int drop_incoming_;
    mutable int drop_outgoing_;
    UnallocatedMap<identifier::Nym, ConnectionData> active_connections_;
    mutable std::shared_mutex connection_map_lock_;

    auto extract_proto(const network::zeromq::Frame& incoming) const noexcept
        -> proto::ServerRequest;

    auto associate_connection(
        const bool oldFormat,
        const identifier::Nym& nymID,
        const network::zeromq::Envelope& connection) noexcept -> void;
    auto old_pipeline(zmq::Message&& message) noexcept -> void;
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
        const network::zeromq::Envelope& id,
        const bool tagged,
        network::zeromq::Message&& incoming) noexcept -> void;
    auto process_message(
        const UnallocatedCString& messageString,
        UnallocatedCString& reply) noexcept -> bool;
    auto process_notification(network::zeromq::Message&& incoming) noexcept
        -> void;
    auto process_proto(
        const network::zeromq::Envelope& id,
        const bool oldFormat,
        network::zeromq::Message&& incoming) noexcept -> void;
    auto query_connection(const identifier::Nym& nymID) noexcept
        -> const ConnectionData&;
    auto run() noexcept -> void;
};
}  // namespace opentxs::server
