// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/DealerReply.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <ctime>
#include <span>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

DealerReply::DealerReply()
    : context_(OTTestEnvironment::GetOT().ZMQ())
{
}

void DealerReply::dealerSocketThread(const ot::UnallocatedCString& msg)
{
    bool replyProcessed{false};
    auto listenCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message&& input) -> void {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool const match =
                inputString == test_message2_ || inputString == test_message3_;
            EXPECT_TRUE(match);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto dealerSocket = context_.Internal().DealerSocket(
        listenCallback, zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    dealerSocket->Start(endpoint_);

    auto message = opentxs::network::zeromq::Message{};
    message.StartBody();
    message.AddFrame(msg);
    auto sent = dealerSocket->Send(std::move(message));

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) { ot::sleep(100ms); }

    EXPECT_TRUE(replyProcessed);
}
}  // namespace ottest
