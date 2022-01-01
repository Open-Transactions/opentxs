// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
#include <set>
#include <string>
#include <thread>

#include "internal/network/zeromq/Pool.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::network::zeromq::context
{
Thread::Thread(zeromq::internal::Pool& parent) noexcept
    : parent_(parent)
    , null_(factory::ZMQSocketNull())
    , gate_()
    , poll_()
    , thread_()
    , modify_()
{
}

auto Thread::Add(BatchID id, StartArgs&& args) noexcept -> bool
{
    const auto ticket = gate_.get();

    if (ticket) { return false; }

    auto sockets = ThreadStartArgs{};
    std::transform(
        args.begin(), args.end(), std::back_inserter(sockets), [](auto& val) {
            auto& [sID, socket, cb] = val;

            return std::make_pair(socket, std::move(cb));
        });
    auto cb = [this, data = std::move(sockets)](auto&) {
        for (auto [socket, cb] : data) {
            assert(cb);

            poll_.data_.emplace_back(std::move(cb));
            auto& s = poll_.items_.emplace_back();
            s.socket = socket->Native();
            s.events = ZMQ_POLLIN;

            assert(poll_.items_.size() == poll_.data_.size());
        }

        if (false == thread_.running_) {
            join();
            thread_.handle_ = std::thread{&Thread::run, this};
            thread_.running_ = true;
        }
    };

    if (Modify(null_.ID(), std::move(cb)).first) {
        parent_.UpdateIndex(id, std::move(args));

        return true;
    } else {

        return false;
    }
}

auto Thread::join() noexcept -> void
{
    if (thread_.handle_.joinable()) { thread_.handle_.join(); }
}

auto Thread::Modify(SocketID socket, ModifyCallback cb) noexcept -> AsyncResult
{
    if (!cb) {
        std::cerr << (OT_PRETTY_CLASS()) << "invalid callback" << std::endl;

        return {};
    }

    auto output = [&] {
        auto lock = Lock{modify_.lock_};
        auto& data = modify_.queue_.emplace(socket, std::move(cb));

        return data.promise_.get_future();
    }();

    if (auto lock = Lock{thread_.lock_}; false == thread_.running_) {
        modify();
    }

    return std::make_pair(true, std::move(output));
}

auto Thread::modify() noexcept -> void
{
    auto& queue = modify_.queue_;
    auto lock = Lock{modify_.lock_};

    while (0u < queue.size()) {
        auto& modification = queue.front();
        const auto& id = modification.socket_;
        auto& cb = modification.callback_;

        if (null_.ID() == id) {
            try {
                cb(null_);
                modification.promise_.set_value(true);
            } catch (...) {
                modification.promise_.set_value(false);
            }
        } else {
            modification.promise_.set_value(parent_.DoModify(id, cb));
        }

        queue.pop();
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
            auto zerr = zmq_errno();
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

auto Thread::Remove(BatchID id, std::pmr::vector<socket::Raw*>&& data) noexcept
    -> std::future<bool>
{
    auto p = std::make_shared<std::promise<bool>>();
    auto future = p->get_future();
    auto cb =
        [this, id, sockets = std::move(data), promise = std::move(p)](auto&) {
            const auto set = [&] {
                auto out = std::pmr::set<void*>{};
                std::transform(
                    sockets.begin(),
                    sockets.end(),
                    std::inserter(out, out.end()),
                    [](auto* socket) { return socket->Native(); });

                return out;
            }();
            auto s = poll_.items_.begin();
            auto c = poll_.data_.begin();

            while ((s != poll_.items_.end()) && (c != poll_.data_.end())) {
                auto* socket = s->socket;

                if (0u == set.count(socket)) {
                    ++s;
                    ++c;
                } else {
                    s = poll_.items_.erase(s);
                    c = poll_.data_.erase(c);
                }
            }

            assert(poll_.items_.size() == poll_.data_.size());

            parent_.UpdateIndex(id);
            promise->set_value(true);
        };

    Modify(null_.ID(), std::move(cb));

    return future;
}

auto Thread::run() noexcept -> void
{
    auto post = ScopeGuard{[this] {
        auto lock = Lock{thread_.lock_};
        modify();
        thread_.running_ = false;
    }};

    while (true) {
        auto done = ScopeGuard{[&] { modify(); }};

        if (0u == poll_.items_.size()) { break; }

        static constexpr auto timeout = std::chrono::milliseconds{100};
        const auto events = ::zmq_poll(
            poll_.items_.data(),
            static_cast<int>(poll_.items_.size()),
            timeout.count());

        if (0 > events) {
            const auto error = ::zmq_errno();
            std::cout << OT_PRETTY_CLASS() << error << std::endl;

            continue;
        } else if (0 == events) {

            continue;
        }

        const auto& v = poll_.items_;
        auto c = poll_.data_.begin();

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
}

auto Thread::Shutdown() noexcept -> void
{
    wait();
    modify();
}

auto Thread::wait() noexcept -> void
{
    gate_.shutdown();
    join();
}

Thread::Items::~Items() = default;

Thread::~Thread() { wait(); }
}  // namespace opentxs::network::zeromq::context
