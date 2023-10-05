// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/DealerRouter.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <ctime>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

DealerRouter::DealerRouter()
    : context_(OTTestEnvironment::GetOT().ZMQ())
{
}

void DealerRouter::dealerSocketThread(const ot::UnallocatedCString& msg)
{
    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](auto&& input) -> void {
            EXPECT_EQ(2, input.get().size());

            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool match =
                inputString == test_message2_ || inputString == test_message3_;
            EXPECT_TRUE(match);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.Internal().DealerSocket(
        dealerCallback, zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    dealerSocket->Start(endpoint_);

    auto sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) { ot::Sleep(100ms); }

    EXPECT_TRUE(replyProcessed);
}

void DealerRouter::routerSocketThread(const ot::UnallocatedCString& endpoint)
{
    auto replyMessage = opentxs::network::zeromq::Message{};

    bool replyProcessed{false};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage, &replyProcessed](auto&& input) -> void {
            EXPECT_EQ(3, input.get().size());

            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool match =
                inputString == test_message2_ || inputString == test_message3_;
            EXPECT_TRUE(match);

            replyMessage = ot::network::zeromq::reply_to_message(input);
            for (const auto& frame : input.Payload()) {
                replyMessage.AddFrame(frame);
            }

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.Internal().RouterSocket(
        routerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(0ms, 30000ms, -1ms);
    routerSocket->Start(endpoint);

    auto end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) { ot::Sleep(100ms); }

    ASSERT_TRUE(replyProcessed);

    routerSocket->Send(std::move(replyMessage));

    // Give the router socket time to send the message.
    ot::Sleep(500ms);
}
}  // namespace ottest
