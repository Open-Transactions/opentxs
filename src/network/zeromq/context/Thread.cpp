// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "network/zeromq/context/Thread.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <thread>
#include <utility>

#include "internal/network/zeromq/Pool.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Signals.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::network::zeromq::context
{
Thread::Thread(zeromq::internal::Pool& parent) noexcept
    : parent_(parent)
    , null_(factory::ZMQSocketNull())
    , alloc_()
    , gate_()
    , thread_()
    , data_()
{
}

auto Thread::Add(BatchID id, StartArgs&& args) noexcept -> bool
{
    const auto ticket = gate_.get();

    if (ticket) { return false; }

    auto sockets = [&] {
        auto out = ThreadStartArgs{};
        std::transform(
            args.begin(), args.end(), std::back_inserter(out), [](auto& val) {
                auto& [sID, socket, cb] = val;

                return std::make_pair(socket, std::move(cb));
            });

        return out;
    }();
    parent_.UpdateIndex(id, std::move(args));
    data_.modify_detach([data = std::move(sockets)](auto& guarded) {
        for (auto [socket, cb] : data) {
            assert(cb);

            guarded.data_.emplace_back(std::move(cb));
            auto& s = guarded.items_.emplace_back();
            s.socket = socket->Native();
            s.events = ZMQ_POLLIN;

            assert(guarded.items_.size() == guarded.data_.size());
        }
    });
    start();

    return true;
}

auto Thread::join() noexcept -> void
{
    if (thread_.handle_.joinable()) { thread_.handle_.join(); }
}

auto Thread::Modify(SocketID socket, ModifyCallback cb) noexcept -> AsyncResult
{
    const auto ticket = gate_.get();

    if (ticket) { return {}; }

    if (!cb) {
        std::cerr << (OT_PRETTY_CLASS()) << "invalid callback" << std::endl;

        return {};
    }

    auto promise = std::make_shared<std::promise<bool>>();
    auto output = std::make_pair(true, promise->get_future());
    data_.modify_detach([=](auto& data) {
        if (null_.ID() == socket) {
            try {
                cb(null_);
                promise->set_value(true);
            } catch (...) {
                promise->set_value(false);
            }
        } else {
            promise->set_value(parent_.DoModify(socket, cb));
        }
    });

    return output;
}

auto Thread::poll(Items& data) noexcept -> void
{
    if (0u == data.items_.size()) {
        thread_.running_ = false;

        return;
    }

    static constexpr auto timeout = 100ms;
    const auto events = ::zmq_poll(
        data.items_.data(),
        static_cast<int>(data.items_.size()),
        timeout.count());

    if (0 > events) {
        std::cout << OT_PRETTY_CLASS() << ::zmq_strerror(::zmq_errno())
                  << std::endl;

        return;
    } else if (0 == events) {

        return;
    }

    const auto& v = data.items_;
    auto c = data.data_.begin();

    for (auto s = v.begin(), end = v.end(); s != end; ++s, ++c) {
        auto& item = *s;

        if (ZMQ_POLLIN != item.revents) { continue; }

        auto& socket = item.socket;
        auto message = Message{};

        if (receive_message(socket, message)) {
            const auto& callback = *c;

            try {
                callback(std::move(message));
            } catch (...) {
            }
        }
    }
}

auto Thread::receive_message(void* socket, Message& message) noexcept -> bool
{
    bool receiving{true};

    while (receiving) {
        auto& frame = message.AddFrame();
        const bool received =
            (-1 != ::zmq_msg_recv(frame, socket, ZMQ_DONTWAIT));

        if (false == received) {
            auto zerr = ::zmq_errno();
            if (EAGAIN == zerr) {
                std::cerr
                    << (OT_PRETTY_CLASS())
                    << "zmq_msg_recv returns EAGAIN. This should never happen."
                    << std::endl;
            } else {
                std::cerr << (OT_PRETTY_CLASS())
                          << ": Receive error: " << ::zmq_strerror(zerr)
                          << std::endl;
            }

            return false;
        }

        int option{0};
        std::size_t optionBytes{sizeof(option)};

        const bool haveOption =
            (-1 !=
             ::zmq_getsockopt(socket, ZMQ_RCVMORE, &option, &optionBytes));

        if (false == haveOption) {
            std::cerr << (OT_PRETTY_CLASS())
                      << "Failed to check socket options error:\n"
                      << ::zmq_strerror(zmq_errno()) << std::endl;

            return false;
        }

        assert(optionBytes == sizeof(option));

        if (1 != option) { receiving = false; }
    }

    return true;
}

auto Thread::Remove(BatchID id, UnallocatedVector<socket::Raw*>&& data) noexcept
    -> std::future<bool>
{
    auto p = std::make_shared<std::promise<bool>>();
    auto future = p->get_future();
    data_.modify_detach(
        [this, id, sockets = std::move(data), promise = std::move(p)](
            auto& guarded) {
            const auto set = [&] {
                auto out = UnallocatedSet<void*>{};
                std::transform(
                    sockets.begin(),
                    sockets.end(),
                    std::inserter(out, out.end()),
                    [](auto* socket) { return socket->Native(); });

                return out;
            }();
            auto s = guarded.items_.begin();
            auto c = guarded.data_.begin();

            while ((s != guarded.items_.end()) && (c != guarded.data_.end())) {
                auto* socket = s->socket;

                if (0u == set.count(socket)) {
                    ++s;
                    ++c;
                } else {
                    s = guarded.items_.erase(s);
                    c = guarded.data_.erase(c);
                }
            }

            assert(guarded.items_.size() == guarded.data_.size());

            parent_.UpdateIndex(id);
            promise->set_value(true);
        });

    return future;
}

auto Thread::run() noexcept -> void
{
    Signals::Block();

    while (thread_.running_) {
        data_.modify_detach([this](auto& data) { poll(data); });
    }
}

auto Thread::Shutdown() noexcept -> void { wait(); }

auto Thread::start() noexcept -> void
{
    if (auto running = thread_.running_.exchange(true); false == running) {
        join();
        thread_.handle_ = std::thread{&Thread::run, this};
    }
}

auto Thread::wait() noexcept -> void
{
    gate_.shutdown();
    join();
}

Thread::Items::~Items() = default;

Thread::~Thread() { wait(); }
}  // namespace opentxs::network::zeromq::context
