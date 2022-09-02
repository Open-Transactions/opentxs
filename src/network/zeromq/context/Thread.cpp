// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "network/zeromq/context/Thread.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <thread>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pool.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Signals.hpp"
#include "internal/util/Thread.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::network::zeromq::context
{
Thread::Items::Items(alloc::Default alloc) noexcept
    : items_(alloc)
    , data_(alloc)
{
}

Thread::Items::Items(Items&& rhs) noexcept
    : items_(rhs.items_.get_allocator())
    , data_(rhs.data_.get_allocator())
{
    swap(items_, rhs.items_);
    swap(data_, rhs.data_);
}

Thread::Items::~Items() = default;
}  // namespace opentxs::network::zeromq::context

namespace opentxs::network::zeromq::context
{
using namespace std::literals;

Thread::Thread(
    const unsigned int index,
    zeromq::internal::Pool& parent,
    std::string_view endpoint) noexcept
    : index_(index)
    , parent_(parent)
    , alloc_()
    , shutdown_(false)
    , control_([&] {
        auto out = parent_.Parent().Internal().RawSocket(socket::Type::Pull);
        const auto rc = out.Connect(endpoint.data());

        assert(rc);

        return out;
    }())
    , data_([&] {
        auto out = Items{std::addressof(alloc_)};
        auto& item = out.items_.emplace_back();
        item.socket = control_.Native();
        item.events = ZMQ_POLLIN;
        out.data_.emplace_back([](auto&&) { std::abort(); });

        assert(out.items_.size() == out.data_.size());

        return out;
    }())
    , thread_name_()
    , id_promise_()
    , id_(id_promise_.get_future())
    , thread_(
          [] {
              auto out = boost::thread::attributes{};
              out.set_stack_size(thread_pool_stack_size_);

              return out;
          }(),
          [this] { run(); })
{
    thread_.detach();
}

auto Thread::ID() const noexcept -> std::thread::id { return id_.get(); }

auto Thread::modify(Message&& message) noexcept -> void
{
    const auto body = message.Body();

    switch (body.at(0).as<Operation>()) {
        case Operation::add_socket: {
            const auto batch = body.at(1).as<BatchID>();

            for (auto [socket, cb] : parent_.GetStartArgs(batch)) {
                assert(cb);

                data_.data_.emplace_back(std::move(cb));
                auto& s = data_.items_.emplace_back();
                s.socket = socket->Native();
                s.events = ZMQ_POLLIN;

                assert(data_.items_.size() == data_.data_.size());
            }
        } break;
        case Operation::remove_socket: {
            const auto batch = body.at(1).as<BatchID>();
            const auto set = parent_.GetStopArgs(batch);
            auto s = data_.items_.begin();
            auto c = data_.data_.begin();

            while ((s != data_.items_.end()) && (c != data_.data_.end())) {
                auto* socket = s->socket;

                if (0_uz == set.count(socket)) {
                    ++s;
                    ++c;
                } else {
                    s = data_.items_.erase(s);
                    c = data_.data_.erase(c);
                }
            }

            assert(data_.items_.size() == data_.data_.size());
        } break;
        case Operation::change_socket: {
            const auto socketID = body.at(1).as<SocketID>();
            parent_.DoModify(socketID);
        } break;
        case Operation::shutdown: {
            shutdown_ = true;
        } break;
        default: {
            std::abort();
        }
    }
}

auto Thread::poll() noexcept -> void
{
    if (!thread_name_.empty()) { SetThisThreadsName(thread_name_); }

    static constexpr auto timeout = 100ms;
    const auto events = ::zmq_poll(
        data_.items_.data(),
        static_cast<int>(data_.items_.size()),
        timeout.count());

    if (0 > events) {
        std::cout << OT_PRETTY_CLASS() << ::zmq_strerror(::zmq_errno())
                  << std::endl;

        return;
    } else if (0 == events) {

        return;
    }

    const auto& v = data_.items_;
    auto c = data_.data_.begin();
    auto i = 0_uz;
    auto modify{false};

    for (auto s = v.begin(), end = v.end(); s != end; ++s, ++c, ++i) {
        const auto& item = *s;

        if (ZMQ_POLLIN != item.revents) { continue; }

        switch (i) {
            case 0_uz: {
                // NOTE control socket
                modify = true;
            } break;
            default: {
                // NOTE regular sockets
                const auto& socket = item.socket;
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

    // NOTE wait until we are no longer iterating over the vectors before adding
    // or removing items
    if (modify) {
        assert(false == v.empty());

        auto* socket = v.begin()->socket;
        auto message = Message{};
        const auto rc = receive_message(socket, message);

        assert(rc);

        this->modify(std::move(message));
    }
}

auto Thread::receive_message(void* socket, Message& message) noexcept -> bool
{
    auto receiving{true};

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

auto Thread::run() noexcept -> void
{
    Signals::Block();
    id_promise_.set_value(std::this_thread::get_id());

    while (false == shutdown_) { poll(); }

    data_.items_.clear();
    data_.data_.clear();
    control_.Close();
    parent_.ReportShutdown(index_);
}

Thread::~Thread() = default;
}  // namespace opentxs::network::zeromq::context
