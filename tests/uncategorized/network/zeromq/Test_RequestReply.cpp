// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <ctime>
#include <string_view>
#include <thread>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ReplyCallback.hpp"
#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Pimpl.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class Test_RequestReply : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString test_message_{"zeromq test message"};
    const ot::UnallocatedCString test_message2_{"zeromq test message 2"};
    const ot::UnallocatedCString test_message3_{"zeromq test message 3"};

    const ot::UnallocatedCString endpoint_{
        "inproc://opentxs/test/request_reply_test"};
    const ot::UnallocatedCString endpoint2_{
        "inproc://opentxs/test/request_reply_test2"};

    void requestSocketThread(const ot::UnallocatedCString& msg);
    void replySocketThread(const ot::UnallocatedCString& endpoint);

    Test_RequestReply()
        : context_(ot::Context().ZMQ())
    {
    }
};

void Test_RequestReply::requestSocketThread(const ot::UnallocatedCString& msg)
{
    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);

    const auto messageString =
        ot::UnallocatedCString{message.Body().begin()->Bytes()};
    ASSERT_EQ(msg, messageString);
}

void Test_RequestReply::replySocketThread(
    const ot::UnallocatedCString& endpoint)
{
    bool replyReturned{false};

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this,
         &replyReturned](zmq::Message&& input) -> ot::network::zeromq::Message {
            const auto inputString =
                ot::UnallocatedCString{input.Body().begin()->Bytes()};
            bool match =
                inputString == test_message2_ || inputString == test_message3_;
            EXPECT_TRUE(match);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);
            replyReturned = true;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.Internal().ReplySocket(
        replyCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(0ms, 30000ms, -1ms);
    replySocket->Start(endpoint);

    auto end = std::time(nullptr) + 15;
    while (!replyReturned && std::time(nullptr) < end) { ot::Sleep(100ms); }

    EXPECT_TRUE(replyReturned);
}

TEST_F(Test_RequestReply, Request_Reply)
{
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](zmq::Message&& input) -> ot::network::zeromq::Message {
            const auto inputString =
                ot::UnallocatedCString{input.Body().begin()->Bytes()};
            EXPECT_EQ(test_message_, inputString);

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

    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);

    const auto messageString =
        ot::UnallocatedCString{message.Body().begin()->Bytes()};
    ASSERT_EQ(test_message_, messageString);
}

TEST_F(Test_RequestReply, Request_2_Reply_1)
{
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](zmq::Message&& input) -> ot::network::zeromq::Message {
            const auto inputString =
                ot::UnallocatedCString{input.Body().begin()->Bytes()};
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

    std::thread requestSocketThread1(
        &Test_RequestReply::requestSocketThread, this, test_message2_);
    std::thread requestSocketThread2(
        &Test_RequestReply::requestSocketThread, this, test_message3_);

    requestSocketThread1.join();
    requestSocketThread2.join();
}

TEST_F(Test_RequestReply, Request_1_Reply_2)
{
    std::thread replySocketThread1(
        &Test_RequestReply::replySocketThread, this, endpoint_);
    std::thread replySocketThread2(
        &Test_RequestReply::replySocketThread, this, endpoint2_);

    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint_);
    requestSocket->Start(endpoint2_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message2_);

        return out;
    }());

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);

    auto messageString =
        ot::UnallocatedCString{message.Body().begin()->Bytes()};
    ASSERT_EQ(test_message2_, messageString);

    auto [result2, message2] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message3_);

        return out;
    }());

    ASSERT_EQ(result2, ot::otx::client::SendResult::VALID_REPLY);

    messageString = message2.Body().begin()->Bytes();
    ASSERT_EQ(test_message3_, messageString);

    replySocketThread1.join();
    replySocketThread2.join();
}

TEST_F(Test_RequestReply, Request_Reply_Multipart)
{
    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const auto& input) -> ot::network::zeromq::Message {
            EXPECT_EQ(4, input.size());
            EXPECT_EQ(1, input.Header().size());
            EXPECT_EQ(2, input.Body().size());

            for (const auto& frame : input.Header()) {
                EXPECT_EQ(test_message_, ot::UnallocatedCString{frame.Bytes()});
            }

            for (const auto& frame : input.Body()) {
                const auto str = ot::UnallocatedCString{frame.Bytes()};
                bool match = (str == test_message2_) || (str == test_message3_);

                EXPECT_TRUE(match);
            }

            auto reply = ot::network::zeromq::reply_to_message(input);
            for (const auto& frame : input.Body()) { reply.AddFrame(frame); }
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.Internal().ReplySocket(
        replyCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(0ms, 30000ms, -1ms);
    replySocket->Start(endpoint_);

    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint_);

    auto multipartMessage = opentxs::network::zeromq::Message{};
    multipartMessage.AddFrame(test_message_);
    multipartMessage.StartBody();
    multipartMessage.AddFrame(test_message2_);
    multipartMessage.AddFrame(test_message3_);

    auto [result, message] = requestSocket->Send(std::move(multipartMessage));

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);

    const auto messageHeader =
        ot::UnallocatedCString{message.Header().begin()->Bytes()};

    ASSERT_EQ(test_message_, messageHeader);

    for (const auto& frame : message.Body()) {
        bool match = (frame.Bytes() == test_message2_) ||
                     (frame.Bytes() == test_message3_);
        ASSERT_TRUE(match);
    }
}
}  // namespace ottest
