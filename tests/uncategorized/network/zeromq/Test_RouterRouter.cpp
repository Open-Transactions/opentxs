// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <zmq.h>
#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <iostream>
#include <memory>
#include <ratio>
#include <thread>

#include "internal/util/P0330.hpp"
#include "internal/util/Signals.hpp"
#include "ottest/fixtures/zeromq/Helpers.hpp"

namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class RouterRouterF : public ::testing::Test
{
public:
    RouterRouterF(const RouterRouterF&) = delete;
    RouterRouterF(RouterRouterF&&) = delete;
    auto operator=(const RouterRouterF&) -> RouterRouterF& = delete;
    auto operator=(RouterRouterF&&) -> RouterRouterF& = delete;

protected:
    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;

    const zmq::Context& context_;
    const ot::UnallocatedCString endpoint_;
    const int linger_;
    Socket server_;
    Socket client_;
    ZMQQueue queue_;
    std::atomic_bool running_;
    std::thread thread_;

    RouterRouterF()
        : context_(ot::Context().ZMQ())
        , endpoint_("inproc://test_endpoint")
        , linger_(0)
        , server_(::zmq_socket(context_, ZMQ_ROUTER), ::zmq_close)
        , client_(::zmq_socket(context_, ZMQ_ROUTER), ::zmq_close)
        , queue_()
        , running_(true)
        , thread_(&RouterRouterF::thread, this)
    {
    }

    ~RouterRouterF() override
    {
        running_ = false;

        if (thread_.joinable()) { thread_.join(); }
    }

private:
    auto thread() noexcept -> void
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

            auto* socket = item.socket;
            auto receiving{true};
            auto msg = opentxs::network::zeromq::Message{};

            while (receiving) {
                auto& frame = msg.AddFrame();
                const auto received =
                    (-1 != zmq_msg_recv(frame, socket, ZMQ_DONTWAIT));

                if (false == received) {
                    auto zerr = zmq_errno();

                    if (EAGAIN == zerr) {
                        std::cerr << ": zmq_msg_recv returns EAGAIN. This "
                                     "should never happen."
                                  << std::endl;
                    } else {
                        std::cerr << ": Receive error: " << zmq_strerror(zerr)
                                  << std::endl;
                    }

                    continue;
                }

                auto option = int{0};
                auto optionBytes = sizeof(option);
                const auto haveOption =
                    (-1 != zmq_getsockopt(
                               socket, ZMQ_RCVMORE, &option, &optionBytes));

                if (false == haveOption) {
                    std::cerr << ": Failed to check socket options error:\n"
                              << zmq_strerror(zmq_errno()) << std::endl;

                    continue;
                }

                if (1 != option) { receiving = false; }
            }

            queue_.receive(msg);
        }
    }
};

TEST_F(RouterRouterF, test)
{
    EXPECT_EQ(
        ::zmq_setsockopt(client_.get(), ZMQ_LINGER, &linger_, sizeof(linger_)),
        0);
    EXPECT_EQ(
        ::zmq_setsockopt(server_.get(), ZMQ_LINGER, &linger_, sizeof(linger_)),
        0);
    EXPECT_EQ(
        ::zmq_setsockopt(
            server_.get(), ZMQ_ROUTING_ID, endpoint_.c_str(), endpoint_.size()),
        0);
    EXPECT_EQ(::zmq_bind(server_.get(), endpoint_.c_str()), 0);
    EXPECT_EQ(::zmq_connect(client_.get(), endpoint_.c_str()), 0);

    ot::Sleep(1s);

    auto msg = opentxs::network::zeromq::Message{};
    msg.AddFrame(endpoint_);
    msg.StartBody();
    msg.AddFrame("test");
    auto sent{true};
    const auto parts = msg.size();
    using namespace opentxs::literals;
    auto counter = 0_uz;

    for (auto& frame : msg) {
        int flags{0};

        if (++counter < parts) { flags = ZMQ_SNDMORE; }

        sent &=
            (-1 != ::zmq_msg_send(
                       const_cast<ot::network::zeromq::Frame&>(frame),
                       client_.get(),
                       flags));
    }

    EXPECT_TRUE(sent);

    const auto& received = queue_.get(0);

    ASSERT_EQ(received.size(), 3);
    EXPECT_STREQ(received.at(2).Bytes().data(), "test");
}
}  // namespace ottest
