// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <zmq.h>
#include <chrono>
#include <span>

#include "internal/util/P0330.hpp"
#include "ottest/fixtures/zeromq/Helpers.hpp"
#include "ottest/fixtures/zeromq/RouterRouter.hpp"

namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

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
    auto frames = msg.get();
    const auto parts = frames.size();
    using namespace opentxs::literals;
    using namespace std::literals;
    auto counter = 0_uz;

    for (auto& frame : frames) {
        int flags{0};

        if (++counter < parts) { flags = ZMQ_SNDMORE; }

        sent &= (-1 != ::zmq_msg_send(frame, client_.get(), flags));
    }

    EXPECT_TRUE(sent);

    const auto& received = queue_.get(0);

    ASSERT_EQ(received.get().size(), 3);
    EXPECT_STREQ(received.get()[2].Bytes().data(), "test");
}
}  // namespace ottest
