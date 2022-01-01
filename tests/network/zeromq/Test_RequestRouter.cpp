// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <ctime>
#include <map>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"

using namespace opentxs;

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_RequestRouter : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string endpoint_{"inproc://opentxs/test/request_router_test"};

    std::atomic_int callbackFinishedCount_{0};

    int callbackCount_{0};

    void requestSocketThread(const std::string& msg);
    void requestSocketThreadMultipart();

    Test_RequestRouter()
        : context_(Context().ZMQ())
    {
    }
};

void Test_RequestRouter::requestSocketThread(const std::string& msg)
{
    auto requestSocket = context_.RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_EQ(result, SendResult::VALID_REPLY);
    // RouterSocket removes the identity frame and RequestSocket removes the
    // delimiter.
    ASSERT_EQ(1, message.size());

    const auto messageString = std::string{message.Body().begin()->Bytes()};
    ASSERT_EQ(msg, messageString);
}

void Test_RequestRouter::requestSocketThreadMultipart()
{
    auto requestSocket = context_.RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint_);

    auto multipartMessage = opentxs::network::zeromq::Message{};
    multipartMessage.AddFrame(testMessage_);
    multipartMessage.StartBody();
    multipartMessage.AddFrame(testMessage2_);
    multipartMessage.AddFrame(testMessage3_);

    auto [result, message] = requestSocket->Send(std::move(multipartMessage));

    ASSERT_EQ(result, SendResult::VALID_REPLY);
    // RouterSocket removes the identity frame and RequestSocket removes the
    // delimiter.
    ASSERT_EQ(4, message.size());

    const auto messageHeader = std::string{message.Header().begin()->Bytes()};

    ASSERT_EQ(testMessage_, messageHeader);

    for (const auto& frame : message.Body()) {
        bool match =
            frame.Bytes() == testMessage2_ || frame.Bytes() == testMessage3_;
        ASSERT_TRUE(match);
    }
}

TEST_F(Test_RequestRouter, Request_Router)
{
    auto replyMessage = opentxs::network::zeromq::Message{};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](zmq::Message&& input) -> void {
            // RequestSocket prepends a delimiter and RouterSocket prepends an
            // identity frame.
            EXPECT_EQ(3, input.size());
            EXPECT_EQ(1, input.Header().size());
            EXPECT_EQ(1, input.Body().size());

            const auto inputString = std::string{input.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            replyMessage = ot::network::zeromq::reply_to_message(input);
            for (const auto& frame : input.Body()) {
                replyMessage.AddFrame(frame);
            }

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.RouterSocket(
        routerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    routerSocket->Start(endpoint_);

    // Send the request on a separate thread so this thread can continue and
    // wait for the ListenCallback to finish, then send the reply.
    std::thread requestSocketThread1(
        &Test_RequestRouter::requestSocketThread, this, testMessage_);

    auto end = std::time(nullptr) + 5;
    while (!callbackFinishedCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(1, callbackFinishedCount_);

    routerSocket->Send(std::move(replyMessage));

    requestSocketThread1.join();
}

TEST_F(Test_RequestRouter, Request_2_Router_1)
{
    callbackCount_ = 2;

    std::pmr::map<std::string, network::zeromq::Message> replyMessages{
        std::pair<std::string, network::zeromq::Message>(testMessage2_, {}),
        std::pair<std::string, network::zeromq::Message>(testMessage3_, {})};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessages](auto&& input) -> void {
            // RequestSocket prepends a delimiter and RouterSocket prepends an
            // identity frame.
            EXPECT_EQ(3, input.size());
            EXPECT_EQ(1, input.Header().size());
            EXPECT_EQ(1, input.Body().size());

            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            auto& replyMessage = replyMessages.at(inputString);
            replyMessage = ot::network::zeromq::reply_to_message(input);
            for (const auto& frame : input.Body()) {
                replyMessage.AddFrame(frame);
            }

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.RouterSocket(
        routerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    routerSocket->Start(endpoint_);

    std::thread requestSocketThread1(
        &Test_RequestRouter::requestSocketThread, this, testMessage2_);
    std::thread requestSocketThread2(
        &Test_RequestRouter::requestSocketThread, this, testMessage3_);

    auto& replyMessage1 = replyMessages.at(testMessage2_);
    auto& replyMessage2 = replyMessages.at(testMessage3_);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinishedCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool message1Sent{false};
    if (0 != replyMessage1.size()) {
        routerSocket->Send(std::move(replyMessage1));
        message1Sent = true;
    } else {
        routerSocket->Send(std::move(replyMessage2));
    }

    end = std::time(nullptr) + 15;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (false == message1Sent) {
        routerSocket->Send(std::move(replyMessage1));
    } else {
        routerSocket->Send(std::move(replyMessage2));
    }

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);

    requestSocketThread1.join();
    requestSocketThread2.join();
}

TEST_F(Test_RequestRouter, Request_Router_Multipart)
{
    auto replyMessage = opentxs::network::zeromq::Message{};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](auto&& input) -> void {
            // RequestSocket prepends a delimiter and RouterSocket prepends an
            // identity frame.
            EXPECT_EQ(6, input.size());
            // Identity frame.
            EXPECT_EQ(1, input.Header().size());
            // Original message: header, delimiter, two body parts.
            EXPECT_EQ(4, input.Body().size());

            for (const auto& frame : input.Body()) {
                bool match = frame.Bytes() == testMessage_ ||
                             frame.Bytes() == testMessage2_ ||
                             frame.Bytes() == testMessage3_;
                EXPECT_TRUE(match || frame.size() == 0);
            }

            replyMessage = ot::network::zeromq::reply_to_message(input);
            for (auto& frame : input.Body()) { replyMessage.AddFrame(frame); }
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.RouterSocket(
        routerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    routerSocket->Start(endpoint_);

    // Send the request on a separate thread so this thread can continue and
    // wait for the ListenCallback to finish, then send the reply.
    std::thread requestSocketThread1(
        &Test_RequestRouter::requestSocketThreadMultipart, this);

    auto end = std::time(nullptr) + 15;
    while (0 == replyMessage.size() && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto sent = routerSocket->Send(std::move(replyMessage));

    ASSERT_TRUE(sent);

    requestSocketThread1.join();
}
}  // namespace ottest
