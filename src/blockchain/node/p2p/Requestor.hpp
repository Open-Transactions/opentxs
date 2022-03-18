// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <zmq.h>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string_view>
#include <thread>

#include "internal/blockchain/node/p2p/Requestor.hpp"
#include "internal/network/p2p/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Actor.hpp"
#include "util/Backoff.hpp"
#include "util/ByteLiterals.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace p2p
{
class Data;
class State;
}  // namespace p2p

namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::p2p
{
class Requestor::Imp final : public Actor<Imp, network::p2p::Job>
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void
    {
        signal_startup(me);
    }
    auto Shutdown() noexcept -> void { signal_shutdown(); }

    Imp(const api::Session& api,
        const network::zeromq::BatchID batch,
        const Type chain,
        const std::string_view toParent,
        allocator_type alloc) noexcept;

    ~Imp() final;

private:
    friend Actor<Imp, network::p2p::Job>;

    enum class State { init, sync, run };

    static constexpr std::size_t limit_{32_MiB};
    static constexpr auto request_timeout_{45s};
    static constexpr auto remote_position_timeout_{2 * 60s};
    static constexpr auto heartbeat_timeout_{5 * 60s};

    const api::Session& api_;
    const Type chain_;
    network::zeromq::socket::Raw& to_parent_;
    State state_;
    Timer request_timer_;
    Timer heartbeat_timer_;
    Time last_remote_position_;
    Time begin_sync_;
    std::optional<Time> last_request_;
    block::Position remote_position_;
    block::Position local_position_;
    block::Position queue_position_;
    std::size_t queued_bytes_;
    std::size_t processed_bytes_;
    std::queue<Message> queue_;
    bool received_first_ack_;
    bool processing_;

    static auto blank(const api::Session& api) noexcept
        -> const block::Position&;

    auto blank() const noexcept -> const block::Position&;
    auto next_position() const noexcept -> const block::Position&;

    auto add_to_queue(const network::p2p::Data& data, Message&& msg) noexcept
        -> void;
    auto check_remote_position() noexcept -> void;
    auto do_common() noexcept -> void;
    auto do_init() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup() noexcept -> void { do_work(); }
    auto do_run() noexcept -> void;
    auto do_sync() noexcept -> void;
    auto have_pending_request() noexcept -> bool;
    auto need_sync() noexcept -> bool;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_push_tx(Message&& in) noexcept -> void;
    auto process_sync_ack(Message&& in) noexcept -> void;
    auto process_sync_processed(Message&& in) noexcept -> void;
    auto process_sync_push(Message&& in) noexcept -> void;
    auto process_sync_reply(Message&& in) noexcept -> void;
    auto register_chain() noexcept -> void;
    auto request(const block::Position& position) noexcept -> void;
    auto reset_heartbeat_timer(std::chrono::seconds interval) noexcept -> void;
    auto reset_request_timer(std::chrono::seconds interval) noexcept -> void;
    auto reset_timer(
        const std::chrono::seconds& interval,
        Timer& timer) noexcept -> void;
    auto state_init(const Work work, Message&& msg) noexcept -> void;
    auto state_run(const Work work, Message&& msg) noexcept -> void;
    auto state_sync(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_run() noexcept -> void;
    auto transition_state_sync() noexcept -> void;
    auto update_activity() noexcept -> void;
    auto update_queue_position() noexcept -> void;
    auto update_queue_position(const network::p2p::Data& data) noexcept -> void;
    auto update_remote_position(const Message& msg) noexcept -> void;
    auto update_remote_position(const network::p2p::Data& data) noexcept
        -> void;
    auto update_remote_position(const network::p2p::State& state) noexcept
        -> void;
    auto work() noexcept -> bool;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::blockchain::node::p2p
