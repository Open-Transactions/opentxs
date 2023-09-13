// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <ctime>
#include <span>
#include <string_view>
#include <thread>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/ReplyCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/fixtures/zeromq/DealerReply.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

TEST_F(DealerReply, Dealer_Reply)
{
    bool replyReturned{false};
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this,
         &replyReturned](zmq::Message&& input) -> ot::network::zeromq::Message {
            EXPECT_EQ(1, input.get().size());
            EXPECT_EQ(input.Envelope().get().size(), 0);
            EXPECT_EQ(1, input.Payload().size());

            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            auto reply = ot::network::zeromq::reply_to_message(input);

            EXPECT_EQ(reply.get().size(), 0);
            EXPECT_EQ(reply.Envelope().get().size(), 0);
            EXPECT_EQ(reply.Payload().size(), 0);

            reply.AddFrame(inputString);

            EXPECT_EQ(1, reply.get().size());
            EXPECT_EQ(reply.Envelope().get().size(), 0);
            EXPECT_EQ(1, reply.Payload().size());
            EXPECT_EQ(
                inputString,
                ot::UnallocatedCString{reply.Payload()[0].Bytes()});

            replyReturned = true;

            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.Internal().ReplySocket(
        replyCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(0ms, 30000ms, -1ms);
    replySocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message&& input) -> void {
            EXPECT_EQ(2, input.get().size());
            EXPECT_EQ(input.Envelope().get().size(), 0);
            EXPECT_EQ(1, input.Payload().size());

            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.Internal().DealerSocket(
        dealerCallback, zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    dealerSocket->Start(endpoint_);

    auto message = opentxs::network::zeromq::Message{};
    message.StartBody();
    message.AddFrame(test_message_);

    ASSERT_TRUE(2 == message.get().size());
    ASSERT_TRUE(0 == message.Envelope().get().size());
    ASSERT_TRUE(1 == message.Payload().size());
    ASSERT_EQ(
        test_message_, ot::UnallocatedCString{message.Payload()[0].Bytes()});
    ASSERT_EQ(test_message_, ot::UnallocatedCString{message.get()[1].Bytes()});

    auto sent = dealerSocket->Send(std::move(message));

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyReturned && std::time(nullptr) < end) { ot::Sleep(100ms); }

    EXPECT_TRUE(replyReturned);

    end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) { ot::Sleep(100ms); }

    EXPECT_TRUE(replyProcessed);
}

TEST_F(DealerReply, Dealer_2_Reply_1)
{
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](zmq::Message&& input) -> ot::network::zeromq::Message {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool match =
                inputString == test_message2_ || inputString == test_message3_;
            EXPECT_TRUE(match);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.Internal().ReplySocket(
        replyCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(0ms, 30000ms, -1ms);
    replySocket->Start(endpoint_);

    std::thread dealerSocketThread1(
        &DealerReply::dealerSocketThread, this, test_message2_);
    std::thread dealerSocketThread2(
        &DealerReply::dealerSocketThread, this, test_message3_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
}

TEST_F(DealerReply, Dealer_Reply_Multipart)
{
    bool replyReturned{false};
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this,
         &replyReturned](zmq::Message&& input) -> ot::network::zeromq::Message {
            const auto envelope = input.Envelope();
            const auto payload = input.Payload();

            // ReplySocket removes the delimiter frame.
            EXPECT_EQ(4, input.get().size());
            EXPECT_EQ(1, envelope.get().size());
            EXPECT_EQ(2, payload.size());

            for (const auto& frame : envelope.get()) {
                EXPECT_EQ(test_message_, frame.Bytes());
            }

            for (const auto& frame : payload) {
                bool match = frame.Bytes() == test_message2_ ||
                             frame.Bytes() == test_message3_;

                EXPECT_TRUE(match);
            }

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.CopyFrames(payload);
            replyReturned = true;

            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.Internal().ReplySocket(
        replyCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(0ms, 30000ms, -1ms);
    replySocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message&& input) -> void {
            // ReplySocket puts the delimiter frame back when it sends the
            // reply.
            ASSERT_EQ(5, input.get().size());
            ASSERT_EQ(input.Envelope().get().size(), 0);
            ASSERT_EQ(4, input.Payload().size());

            const auto header = ot::UnallocatedCString{input.get()[1].Bytes()};

            ASSERT_EQ(test_message_, header);

            for (auto i{3u}; i < input.get().size(); ++i) {
                const auto& frame = input.get()[i].Bytes();
                bool match = frame == test_message2_ || frame == test_message3_;

                EXPECT_TRUE(match);
            }

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.Internal().DealerSocket(
        dealerCallback, zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    dealerSocket->Start(endpoint_);

    auto multipartMessage = opentxs::network::zeromq::Message{};
    multipartMessage.StartBody();
    multipartMessage.AddFrame(test_message_);
    multipartMessage.AddFrame();
    multipartMessage.AddFrame(test_message2_);
    multipartMessage.AddFrame(test_message3_);

    auto sent = dealerSocket->Send(std::move(multipartMessage));

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyReturned && std::time(nullptr) < end) { ot::Sleep(100ms); }

    EXPECT_TRUE(replyReturned);

    end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) { ot::Sleep(100ms); }

    EXPECT_TRUE(replyProcessed);
}
}  // namespace ottest
