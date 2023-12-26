// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/RouterDealer.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <ctime>
#include <span>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

RouterDealer::RouterDealer()
    : context_(OTTestEnvironment::GetOT().ZMQ())
    , router_socket_(nullptr)
    , dealer_socket_(nullptr)
    , test_message_("zeromq test message")
    , test_message2_("zeromq test message 2")
    , test_message3_("zeromq test message 3")
    , router_endpoint_("inproc://opentxs/test/router")
    , dealer_endpoint_("inproc://opentxs/test/dealer")
    , callback_finished_count_(0)
{
}

void RouterDealer::requestSocketThread(
    const ot::UnallocatedCString& endpoint,
    const ot::UnallocatedCString& msg)
{
    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);

    const auto messageString =
        ot::UnallocatedCString{message.Payload().begin()->Bytes()};
    ASSERT_EQ(msg, messageString);
}

void RouterDealer::dealerSocketThread(
    const ot::UnallocatedCString& endpoint,
    const ot::UnallocatedCString& msg)
{
    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [&replyProcessed, msg](ot::network::zeromq::Message&& input) -> void {
            EXPECT_EQ(2, input.get().size());

            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};

            EXPECT_EQ(msg, inputString);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.Internal().DealerSocket(
        dealerCallback, zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    dealerSocket->Start(endpoint);

    auto sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) { ot::sleep(100ms); }

    ASSERT_TRUE(replyProcessed);
}
}  // namespace ottest
