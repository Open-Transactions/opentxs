// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include <cs_deferred_guarded.h>
#include <atomic>
#include <cstddef>
#include <random>
#include <shared_mutex>
#include <string_view>

#include "internal/network/p2p/Client.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Timer.hpp"
#include "network/p2p/client/Server.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
class Blockchain;
}  // namespace network

class Session;
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

class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::network::p2p::Client::Imp
{
public:
    using Chain = opentxs::blockchain::Type;
    using Callback = zeromq::ListenCallback;
    using SocketType = zeromq::socket::Type;

    auto Endpoint() const noexcept -> std::string_view;

    auto Init(const api::network::Blockchain& parent) noexcept -> void;

    Imp(const api::Session& api, zeromq::internal::Handle&& handle) noexcept;

    ~Imp();

private:
    using ServerMap = Map<CString, client::Server>;
    using ChainMap = Map<Chain, CString>;
    using ProviderMap = Map<Chain, Set<CString>>;
    using ActiveMap = Map<Chain, std::atomic<std::size_t>>;
    using Height = opentxs::blockchain::block::Height;
    using HeightMap = Map<Chain, Height>;
    using Message = zeromq::Message;
    using GuardedSocket =
        libguarded::deferred_guarded<zeromq::socket::Raw, std::shared_mutex>;
    using QueuedMessages = Deque<Message>;
    using QueuedChainMessages = Map<Chain, QueuedMessages>;

    const api::Session& api_;
    const CString endpoint_;
    const CString monitor_endpoint_;
    const CString loopback_endpoint_;
    zeromq::internal::Handle handle_;
    zeromq::internal::Batch& batch_;
    const Callback& external_cb_;
    const Callback& internal_cb_;
    const Callback& monitor_cb_;
    const Callback& wallet_cb_;
    zeromq::socket::Raw& external_router_;
    zeromq::socket::Raw& monitor_;
    zeromq::socket::Raw& external_sub_;
    zeromq::socket::Raw& internal_router_;
    zeromq::socket::Raw& internal_sub_;
    zeromq::socket::Raw& loopback_;
    zeromq::socket::Raw& wallet_;
    mutable GuardedSocket to_loopback_;
    mutable std::random_device rd_;
    mutable std::default_random_engine eng_;
    client::Server blank_;
    Timer timer_;
    HeightMap progress_;
    ServerMap servers_;
    ChainMap clients_;
    ProviderMap providers_;
    ActiveMap active_;
    Set<CString> connected_servers_;
    QueuedMessages pending_;
    QueuedChainMessages pending_chain_;
    std::atomic<std::size_t> connected_count_;
    std::atomic_bool running_;
    zeromq::internal::Thread* thread_;

    auto get_chain(Chain chain) const noexcept -> CString;
    auto get_provider(Chain chain) const noexcept -> CString;
    auto get_required_height(Chain chain) const noexcept -> Height;

    auto flush_pending() noexcept -> void;
    auto flush_pending(Chain chain) noexcept -> void;
    auto forward_to_all(Chain chain, Message&& message) noexcept -> void;
    auto forward_to_all(Message&& message) noexcept -> void;
    auto ping_server(client::Server& server) noexcept -> void;
    auto process_external(Message&& msg) noexcept -> void;
    auto process_header(Message&& msg) noexcept -> void;
    auto process_internal(Message&& msg) noexcept -> void;
    auto process_monitor(Message&& msg) noexcept -> void;
    auto process_pushtx(Message&& msg) noexcept -> void;
    auto process_register(Message&& msg) noexcept -> void;
    auto process_request(Message&& msg) noexcept -> void;
    auto process_response(Message&& msg) noexcept -> void;
    auto process_server(Message&& msg) noexcept -> void;
    auto process_server(const CString ep) noexcept -> void;
    auto process_wallet(Message&& msg) noexcept -> void;
    auto reset_timer() noexcept -> void;
    auto server_is_active(client::Server& server) noexcept -> void;
    auto server_is_stalled(client::Server& server) noexcept -> void;
    auto shutdown() noexcept -> void;
    auto startup(const api::network::Blockchain& parent) noexcept -> void;
    auto state_machine() noexcept -> void;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
