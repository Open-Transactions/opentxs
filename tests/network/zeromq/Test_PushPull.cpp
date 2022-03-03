// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <ctime>

#include "opentxs/OT.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class Test_PushPull : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString testMessage_{"zeromq test message"};
    const ot::UnallocatedCString testMessage2_{"zeromq test message 2"};
    const ot::UnallocatedCString testMessage3_{"zeromq test message 3"};

    const ot::UnallocatedCString endpoint_{
        "inproc://opentxs/test/push_pull_test"};

    Test_PushPull()
        : context_(ot::Context().ZMQ())
    {
    }
};

TEST_F(Test_PushPull, Push_Pull)
{
    bool callbackFinished{false};

    auto pullCallback = zmq::ListenCallback::Factory(
        [this, &callbackFinished](auto&& input) -> void {
            EXPECT_EQ(1, input.size());
            const auto inputString =
                ot::UnallocatedCString{input.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &pullCallback.get());

    auto pullSocket =
        context_.PullSocket(pullCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &pullSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pull, pullSocket->Type());

    pullSocket->SetTimeouts(0ms, 30000ms, -1ms);
    pullSocket->Start(endpoint_);

    auto pushSocket = context_.PushSocket(zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &pushSocket.get());
    ASSERT_EQ(zmq::socket::Type::Push, pushSocket->Type());

    pushSocket->SetTimeouts(0ms, -1ms, 30000ms);
    pushSocket->Start(endpoint_);

    auto sent = pushSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) { ot::Sleep(100ms); }

    ASSERT_TRUE(callbackFinished);
}
}  // namespace ottest
