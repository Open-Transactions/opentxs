// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include "opentxs/OT.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_ReplySocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    Test_ReplySocket()
        : context_(ot::Context().ZMQ())
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

    auto replySocket =
        context_.ReplySocket(replyCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());
}
}  // namespace ottest

// TODO: Add tests for other public member functions: SetPrivateKey
