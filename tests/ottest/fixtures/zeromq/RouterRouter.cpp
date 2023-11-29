// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/RouterRouter.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <zmq.h>
#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "internal/util/Signals.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/zeromq/Helpers.hpp"

namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

RouterRouterF::RouterRouterF()
    : context_(OTTestEnvironment::GetOT().ZMQ())
    , endpoint_("inproc://test_endpoint")
    , linger_(0)
    , server_(::zmq_socket(context_, ZMQ_ROUTER), ::zmq_close)
    , client_(::zmq_socket(context_, ZMQ_ROUTER), ::zmq_close)
    , queue_()
    , running_(true)
    , thread_(&RouterRouterF::thread, this)
{
}

RouterRouterF::~RouterRouterF()
{
    running_ = false;

    if (thread_.joinable()) { thread_.join(); }
}

auto RouterRouterF::thread() noexcept -> void
{
    ot::Signals::Block();
    auto poll = [&] {
        auto output = std::array<::zmq_pollitem_t, 1>{};

        {
            auto& item = output.at(0);
            item.socket = server_.get();
            item.events = ZMQ_POLLIN;
        }

        return output;
    }();

    while (running_) {
        constexpr auto timeout = 1000ms;
        const auto events =
            ::zmq_poll(poll.data(), poll.size(), timeout.count());

        if (0 > events) {
            const auto error = ::zmq_errno();
            std::cout << ::zmq_strerror(error) << '\n';

            continue;
        } else if (0 == events) {

            continue;
        }

        auto& item = poll.at(0);

        if (ZMQ_POLLIN != item.revents) { continue; }

        auto msg = opentxs::network::zeromq::Message{};
        using opentxs::network::zeromq::receive_to_message;

        if (receive_to_message(std::cerr, item.socket, msg, ZMQ_DONTWAIT)) {
            queue_.receive(msg);
        }
    }
}
}  // namespace ottest
