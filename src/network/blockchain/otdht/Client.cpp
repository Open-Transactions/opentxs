// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/otdht/Client.hpp"  // IWYU pragma: associated

#include <span>
#include <stdexcept>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/otdht/Acknowledgement.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::network::blockchain::otdht
{
Client::Client(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> node,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : Actor(api, node, batchID, alloc)
    , best_remote_position_()
    , processing_position_()
    , queue_(alloc)
    , processing_(false)
{
}

auto Client::best_position() const noexcept
    -> opentxs::blockchain::block::Position
{
    if (queue_.empty()) {
        if (processing_) {

            return processing_position_;
        } else {

            return local_position();
        }
    } else {

        return queue_.back().second;
    }
}

auto Client::can_connect(const network::otdht::Data& data) const noexcept
    -> bool
{
    const auto& log = log_;
    const auto best = best_position();
    const auto max = best.height_ + 1;
    const auto start = data.FirstPosition(api_);

    if (start.height_ > max) {
        log()(name_)(": incoming data starts at height ")(start.height_)(
            " however local data stops at height ")(best.height_)
            .Flush();

        return false;
    }

    return true;
}

auto Client::do_work() noexcept -> bool
{
    drain_queue();
    fill_queue();

    return false;
}

auto Client::drain_queue() noexcept -> void
{
    const auto& log = log_;

    if (processing_) {
        log()(name_)(": still processing last message").Flush();
    } else if (queue_.empty()) {
        log()(name_)(": no queued messages to process").Flush();
    } else {
        auto post = ScopeGuard{[&] { queue_.pop_front(); }};
        auto& [msg, position] = queue_.front();
        processing_ = true;
        processing_position_ = std::move(position);
        to_blockchain().SendDeferred(std::move(msg));
    }
}

auto Client::fill_queue() noexcept -> void
{
    const auto& log = log_;
    const auto& target = best_remote_position_;
    const auto local = local_position();

    if (have_outstanding_request()) {
        log()(name_)(
            ": waiting for existing request to arrive or time out before "
            "requesting new data")
            .Flush();
    } else if (queue_.size() >= queue_limit_) {
        log()(name_)(": avoiding requests for new data while queue is full")
            .Flush();
    } else if (local.NotReplacedBy(target)) {
        log()(name_)(": no peers report data better than current position ")(
            local)
            .Flush();
    } else {
        const auto best = best_position();
        log()(name_)(": requesting data for blocks after ")(best).Flush();
        send_request(best);
    }
}

auto Client::process_ack(
    const Message& msg,
    const network::otdht::Acknowledgement& ack) noexcept -> void
{
    try {
        process_state(msg, ack.State(chain_));
    } catch (const std::exception& e) {
        LogError()()(name_)(": ")(e.what()).Flush();
    }
}

auto Client::process_data(
    Message&& msg,
    const network::otdht::Data& data) noexcept -> void
{
    const auto& log = log_;

    if (false == process_state(msg, data.State())) {
        LogError()()(name_)(": received data for wrong chain").Flush();

        return;
    } else if (data.Blocks().empty()) {
        log()(name_)(": ignoring empty update").Flush();

        return;
    } else if (false == can_connect(data)) {
        log()(name_)(": ignoring non-contiguous update").Flush();

        return;
    }

    const auto& [_, position] =
        queue_.emplace_back(std::move(msg), data.LastPosition(api_));
    log()(name_)(": queued data for block range ending at ")(position).Flush();
}

auto Client::process_job_processed(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    if (2 >= body.size()) { LogAbort()()(name_)(": invalid message").Abort(); }

    processing_ = false;
    processing_position_ = {};
    update_oracle_position(
        {body[1].as<opentxs::blockchain::block::Height>(), body[2].Bytes()});
}

auto Client::process_state(
    const Message& msg,
    const network::otdht::State& state) noexcept -> bool
{
    if (state.Chain() != chain_) { return false; }

    const auto peer = get_peer(msg);
    const auto& position = state.Position();
    update_peer_position(peer, position);
    update_remote_position(position);

    return true;
}

auto Client::process_sync_peer(Message&& msg) noexcept -> void
{
    try {
        const auto peer = get_peer(msg);
        const auto sync = api_.Factory().BlockchainSyncMessage(msg);
        const auto type = sync->Type();
        using Type = opentxs::network::otdht::MessageType;
        auto finish{false};

        switch (type) {
            case Type::sync_ack: {
                process_ack(msg, sync->asAcknowledgement());
            } break;
            case Type::sync_reply: {
                finish = true;
                [[fallthrough]];
            }
            case Type::new_block_header: {
                process_data(std::move(msg), sync->asData());
            } break;
            case Type::error:
            case Type::sync_request:
            case Type::query:
            case Type::publish_contract:
            case Type::publish_ack:
            case Type::contract_query:
            case Type::contract:
            case Type::pushtx:
            case Type::pushtx_reply: {
                const auto error =
                    CString{}
                        .append("unsupported message type on external socket: ")
                        .append(print(type));

                throw std::runtime_error{error.c_str()};
            }
            default: {
                const auto error =
                    CString{}
                        .append("unknown message type: ")
                        .append(std::to_string(
                            static_cast<network::otdht::TypeEnum>(type)));

                throw std::runtime_error{error.c_str()};
            }
        }

        if (finish) { finish_request(peer); }
    } catch (const std::exception& e) {
        LogError()()(name_)(": ")(e.what()).Flush();
    }
}

auto Client::update_remote_position(
    const opentxs::blockchain::block::Position& incoming) noexcept -> void
{
    update_position(incoming, best_remote_position_);
    log_()(name_)(": best remote position is ")(best_remote_position_).Flush();
}

Client::~Client() = default;
}  // namespace opentxs::network::blockchain::otdht
