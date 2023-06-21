// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ReplyCallback.hpp"
#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_ReplySocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    Test_ReplySocket()
        : context_(OTTestEnvironment::GetOT().ZMQ())
    {
    }
};

TEST_F(Test_ReplySocket, ReplySocket_Factory)
{
    auto replyCallback = zmq::ReplyCallback::Factory(
        [](zmq::Message&& input) -> ot::network::zeromq::Message {
            return zmq::Message{};
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.Internal().ReplySocket(
        replyCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());
}
}  // namespace ottest

// TODO: Add tests for other public member functions: SetPrivateKey
