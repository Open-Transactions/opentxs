// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/fixtures/zeromq/PublishSocket.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
TEST_F(PublishSocket, PublishSocket_Factory)
{
    auto publishSocket = context_.Internal().PublishSocket();

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(zmq::socket::Type::Publish, publishSocket->Type());
}
}  // namespace ottest

// TODO: Add tests for other public member functions: SetPrivateKey
