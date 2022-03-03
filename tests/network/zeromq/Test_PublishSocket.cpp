// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include "opentxs/OT.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_PublishSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    Test_PublishSocket()
        : context_(ot::Context().ZMQ())
    {
    }
};

TEST_F(Test_PublishSocket, PublishSocket_Factory)
{
    auto publishSocket = context_.PublishSocket();

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(zmq::socket::Type::Publish, publishSocket->Type());
}
}  // namespace ottest

// TODO: Add tests for other public member functions: SetPrivateKey
