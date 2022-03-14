// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/node/base/SyncClient.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>

#include "internal/api/network/Blockchain.hpp"
#include "internal/network/p2p/Types.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Signals.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/p2p/Acknowledgement.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/BlockchainP2PChainState.pb.h"
#include "util/Backoff.hpp"
#include "util/ByteLiterals.hpp"
#include "util/Work.hpp"

namespace bcsync = opentxs::network::p2p;

namespace opentxs::blockchain::node::base
{
class SyncClient::Imp
{
public:
    enum class State { Init, Sync, Run };

    const UnallocatedCString endpoint_;
    const api::Session& api_;
    const Type chain_;

    Imp(const api::Session& api, const Type chain) noexcept
        : endpoint_(network::zeromq::MakeArbitraryInproc())
        , api_(api)
        , chain_(chain)
        , lock_()
        , state_(State::Init)
        , begin_sync_()
        , activity_()
        , processing_(false)
        , dealer_([&] {
            auto out = Socket{
                ::zmq_socket(api_.Network().ZeroMQ(), ZMQ_DEALER), ::zmq_close};
            auto rc = ::zmq_setsockopt(
                out.get(), ZMQ_LINGER, &linger_, sizeof(linger_));

            OT_ASSERT(0 == rc);

            const auto endpoint =
                CString{api_.Network().Blockchain().Internal().SyncEndpoint()};
            rc = ::zmq_connect(out.get(), endpoint.data());

            OT_ASSERT(0 == rc);

            LogTrace()(OT_PRETTY_CLASS())("internal dealer connected to ")(
                endpoint)
                .Flush();

            return out;
        }())
        , subscribe_([&] {
            auto out = Socket{
                ::zmq_socket(api_.Network().ZeroMQ(), ZMQ_SUB), ::zmq_close};
            auto rc = ::zmq_setsockopt(
                out.get(), ZMQ_LINGER, &linger_, sizeof(linger_));

            OT_ASSERT(0 == rc);

            rc = ::zmq_setsockopt(out.get(), ZMQ_SUBSCRIBE, "", 0);

            OT_ASSERT(0 == rc);

            rc = ::zmq_connect(out.get(), api_.Endpoints().Shutdown().data());

            OT_ASSERT(0 == rc);

            return out;
        }())
        , pair_([&] {
            auto out = Socket{
                ::zmq_socket(api_.Network().ZeroMQ(), ZMQ_PAIR), ::zmq_close};
            auto rc = ::zmq_setsockopt(
                out.get(), ZMQ_LINGER, &linger_, sizeof(linger_));

            OT_ASSERT(0 == rc);

            rc = ::zmq_bind(out.get(), endpoint_.c_str());

            OT_ASSERT(0 == rc);

            return out;
        }())
        , local_position_(blank(api_))
        , remote_position_(blank(api_))
        , queue_position_(blank(api_))
        , queued_bytes_()
        , queue_()
        , timer_()
        , running_(true)
        , thread_(&Imp::thread, this)
    {
    }

    ~Imp()
    {
        running_ = false;

        if (thread_.joinable()) { thread_.join(); }
    }

private:
    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;
    using OTSocket = network::zeromq::socket::implementation::Socket;
    using Task = opentxs::network::p2p::Job;

    static constexpr int linger_{0};
    static constexpr std::size_t limit_{32_MiB};
    static constexpr std::chrono::seconds retry_interval_{4};

    mutable std::mutex lock_;
    std::atomic<State> state_;
    Time begin_sync_;
    Time activity_;
    bool processing_;
    Socket dealer_;
    Socket subscribe_;
    Socket pair_;
    block::Position local_position_;
    block::Position remote_position_;
    block::Position queue_position_;
    std::size_t queued_bytes_;
    std::queue<network::zeromq::Message> queue_;
    Backoff timer_;
    std::atomic_bool running_;
    std::thread thread_;

    static auto blank(const api::Session& api) noexcept
        -> const block::Position&
    {
        static const auto output = block::Position{-1, api.Factory().Data()};

        return output;
    }

    auto blank() const noexcept -> const block::Position&
    {
        return blank(api_);
    }
    auto is_idle() const noexcept -> bool
    {
        if (local_position_ != remote_position_) { return false; }

        if (0 < queue_.size()) { return false; }

        return true;
    }
    auto next_position() const noexcept -> const block::Position&
    {
        return (blank() == queue_position_) ? local_position_ : queue_position_;
    }
    auto register_chain() const noexcept -> void
    {
        auto msg = MakeWork(Task::Register);
        msg.AddFrame(chain_);
        LogDebug()(OT_PRETTY_CLASS())("registering ")(print(chain_))(
            " with high level api")
            .Flush();
        auto lock = Lock{lock_};
        OTSocket::send_message(lock, dealer_.get(), std::move(msg));
    }
    auto request(const block::Position& position) const noexcept -> void
    {
        if (blank() == position) { return; }

        auto msg = MakeWork(Task::Request);
        msg.AddFrame(chain_);
        msg.Internal().AddFrame([&] {
            auto proto = proto::BlockchainP2PChainState{};
            const auto state = bcsync::State{chain_, position};
            state.Serialize(proto);

            return proto;
        }());
        LogVerbose()(OT_PRETTY_CLASS())("requesting sync data for ")(
            print(chain_))(" starting from block ")(position.first)
            .Flush();
        auto lock = Lock{lock_};
        OTSocket::send_message(lock, dealer_.get(), std::move(msg));
    }

    auto add_to_queue(network::zeromq::Message&& msg) noexcept -> void
    {
        const auto base = api_.Factory().BlockchainSyncMessage(msg);
        const auto& data = base->asData();
        const auto& blocks = data.Blocks();
        update_remote_position(data.State());

        if (0u == blocks.size()) { return; }

        const auto bytes = msg.Total();
        LogDebug()(OT_PRETTY_CLASS())("buffering ")(
            bytes)(" bytes of sync data for ")(print(chain_))(" blocks ")(
            blocks.front().Height())(" to ")(blocks.back().Height())
            .Flush();
        queued_bytes_ += bytes;
        queue_.emplace(std::move(msg));
        update_queue_position(data);
    }
    auto do_init() noexcept -> void
    {
        if (timer_.test()) { register_chain(); }
    }
    auto do_run() noexcept -> void
    {
        do_sync();

        if (need_heartbeat()) { request(next_position()); }
    }
    auto do_sync() noexcept -> void
    {
        if (need_sync()) { request(next_position()); }

        if (processing_) { return; }

        if (0 < queue_.size()) {
            auto& msg = queue_.front();
            const auto bytes = msg.Total();
            auto lock = Lock{lock_};

            if (OTSocket::send_message(lock, pair_.get(), std::move(msg))) {
                processing_ = true;
                queued_bytes_ -= bytes;
                queue_.pop();
                update_queue_position();
            } else {
                OT_FAIL;
            }
        }
    }
    auto need_heartbeat() noexcept -> bool
    {
        if (false == is_idle()) { return false; }

        constexpr auto limit = std::chrono::minutes{2};
        const auto duration = Clock::now() - activity_;

        if (duration < limit) { return false; }

        if (timer_.test(retry_interval_)) {
            LogVerbose()(OT_PRETTY_CLASS())("more than ")(limit.count())(
                " minutes since data received for ")(print(chain_))
                .Flush();

            return true;
        } else {

            return false;
        }
    }
    auto need_sync() noexcept -> bool
    {
        if (local_position_ == remote_position_) {
            LogTrace()(OT_PRETTY_CLASS())(print(chain_))(" is up to date")
                .Flush();

            return false;
        }

        if (queue_position_ == remote_position_) {
            LogTrace()(OT_PRETTY_CLASS())(print(chain_))(" data already queued")
                .Flush();

            return false;
        }

        if (queued_bytes_ >= limit_) {
            LogTrace()(OT_PRETTY_CLASS())(print(chain_))(
                " buffer is full with ")(queued_bytes_)(" bytes ")
                .Flush();

            return false;
        }

        return timer_.test(retry_interval_);
    }
    auto process(void* socket) noexcept -> void
    {
        auto msg = [&] {
            auto output = network::zeromq::Message{};
            auto lock = Lock{lock_};
            OTSocket::receive_message(lock, socket, output);

            return output;
        }();

        if (0 == msg.size()) {
            LogTrace()(OT_PRETTY_CLASS())("Dropping empty message").Flush();

            return;
        }

        try {
            const auto body = msg.Body();

            if (0 == body.size()) {
                throw std::runtime_error{"Empty message body"};
            }

            const auto task = body.at(0).as<Task>();

            switch (task) {
                case Task::Shutdown: {
                    running_ = false;
                } break;
                case Task::SyncAck: {
                    const auto base = api_.Factory().BlockchainSyncMessage(msg);
                    const auto& ack = base->asAcknowledgement();
                    update_remote_position(ack.State(chain_));
                    activity_ = Clock::now();
                    LogVerbose()(OT_PRETTY_CLASS())("best chain tip for ")(
                        print(chain_))(" according to sync peer is ")(
                        remote_position_.second->asHex())(" at height ")(
                        remote_position_.first)
                        .Flush();

                    if (State::Init == state_.load()) {
                        timer_.reset();
                        begin_sync_ = Clock::now();
                        state_.store(State::Sync);
                    }
                } break;
                case Task::SyncReply: {
                    activity_ = Clock::now();
                    timer_.reset();
                    add_to_queue(std::move(msg));
                } break;
                case Task::SyncPush: {
                    activity_ = Clock::now();

                    switch (state_.load()) {
                        case State::Run: {
                            add_to_queue(std::move(msg));
                        } break;
                        case State::Init:
                        case State::Sync:
                        default: {
                            LogDebug()(OT_PRETTY_CLASS())("ignoring ")(
                                print(chain_))(
                                " push notification until sync is complete")
                                .Flush();
                        }
                    }
                } break;
                case Task::Processed: {
                    OT_ASSERT(2 < body.size());

                    local_position_ = {
                        body.at(1).as<block::Height>(),
                        api_.Factory().Data(body.at(2).Bytes())};
                    processing_ = false;
                } break;
                case Task::PushTransaction: {
                    auto lock = Lock{lock_};
                    OTSocket::send_message(lock, dealer_.get(), std::move(msg));
                } break;
                case Task::SyncServerUpdated:
                case Task::Register:
                case Task::Request:
                default: {
                    OT_FAIL;
                }
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return;
        }
    }
    auto state_machine() noexcept -> void
    {
        switch (state_.load()) {
            case State::Init: {
                do_init();
            } break;
            case State::Sync: {
                do_sync();

                if (is_idle()) {
                    const auto time =
                        std::chrono::duration_cast<std::chrono::seconds>(
                            Clock::now() - begin_sync_);
                    LogError()(print(chain_))(" sync completed in ")(
                        time.count())(" seconds.")
                        .Flush();
                    state_.store(State::Run);
                }
            } break;
            case State::Run:
            default: {
                do_run();
            }
        }
    }
    auto thread() noexcept -> void
    {
        Signals::Block();
        auto poll = [&] {
            auto output = std::array<::zmq_pollitem_t, 3>{};

            {
                auto& item = output.at(0);
                item.socket = dealer_.get();
                item.events = ZMQ_POLLIN;
            }
            {
                auto& item = output.at(1);
                item.socket = subscribe_.get();
                item.events = ZMQ_POLLIN;
            }
            {
                auto& item = output.at(2);
                item.socket = pair_.get();
                item.events = ZMQ_POLLIN;
            }

            return output;
        }();

        while (running_) {
            state_machine();
            constexpr auto timeout = 1000ms;
            const auto events =
                ::zmq_poll(poll.data(), poll.size(), timeout.count());

            if (0 > events) {
                const auto error = ::zmq_errno();
                LogError()(OT_PRETTY_CLASS())(::zmq_strerror(error)).Flush();

                continue;
            } else if (0 == events) {

                continue;
            }

            for (auto& item : poll) {
                if (ZMQ_POLLIN != item.revents) { continue; }

                process(item.socket);
            }
        }
    }
    auto update_queue_position() noexcept -> void
    {
        if (0 == queue_.size()) {
            queue_position_ = blank();

            return;
        }

        if (blank() == queue_position_) {
            const auto base =
                api_.Factory().BlockchainSyncMessage(queue_.back());
            update_queue_position(base->asData());
        }
    }
    auto update_queue_position(const bcsync::Data& data) noexcept -> void
    {
        const auto& blocks = data.Blocks();  // FIXME

        if (0u == blocks.size()) { return; }

        queue_position_ = data.LastPosition(api_);
    }
    auto update_remote_position(const bcsync::State& state) noexcept -> void
    {
        remote_position_ = state.Position();
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

SyncClient::SyncClient(const api::Session& api, const Type chain) noexcept
    : imp_(std::make_unique<Imp>(api, chain).release())
{
}

auto SyncClient::Endpoint() const noexcept -> const UnallocatedCString&
{
    return imp_->endpoint_;
}

SyncClient::~SyncClient()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::node::base
