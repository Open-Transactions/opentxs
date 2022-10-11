// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_DealerSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    Test_DealerSocket()
        : context_(ot::Context().ZMQ())
    {
    }
};

TEST_F(Test_DealerSocket, DealerSocket_Factory)
{
    auto dealerSocket = context_.Internal().DealerSocket(
        zmq::ListenCallback::Factory(), zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());
}
}  // namespace ottest

// TODO: Add tests for other public member functions: SetPublicKey,
// SetSocksProxy
