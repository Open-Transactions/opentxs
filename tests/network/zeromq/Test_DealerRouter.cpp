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
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

using namespace opentxs;

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_DealerRouter : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string endpoint_{"inproc://opentxs/test/dealer_router_test"};
    const std::string endpoint2_{"inproc://opentxs/test/dealer_router_test2"};

    std::atomic_int callbackFinishedCount_{0};

    int callbackCount_{0};

    void dealerSocketThread(const std::string& msg);
    void routerSocketThread(const std::string& endpoint);

    Test_DealerRouter()
        : context_(Context().ZMQ())
    {
    }
};

void Test_DealerRouter::dealerSocketThread(const std::string& msg)
{
    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](auto&& input) -> void {
            EXPECT_EQ(2, input.size());

            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.DealerSocket(
        dealerCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);

    auto sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}

void Test_DealerRouter::routerSocketThread(const std::string& endpoint)
{
    auto replyMessage = opentxs::network::zeromq::Message{};

    bool replyProcessed{false};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage, &replyProcessed](auto&& input) -> void {
            EXPECT_EQ(3, input.size());

            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            replyMessage = ot::network::zeromq::reply_to_message(input);
            for (const auto& frame : input.Body()) {
                replyMessage.AddFrame(frame);
            }

            replyProcessed = true;
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
    routerSocket->Start(endpoint);

    auto end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(replyProcessed);

    routerSocket->Send(std::move(replyMessage));

    // Give the router socket time to send the message.
    Sleep(std::chrono::milliseconds(500));
}

TEST_F(Test_DealerRouter, Dealer_Router)
{
    auto replyMessage = opentxs::network::zeromq::Message{};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](auto&& input) -> void {
            EXPECT_EQ(3, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

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

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](auto&& input) -> void {
            EXPECT_EQ(2, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.DealerSocket(
        dealerCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);

    auto sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (0 == replyMessage.size() && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_NE(0, replyMessage.size());

    sent = routerSocket->Send(std::move(replyMessage));

    ASSERT_TRUE(sent);

    end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}

TEST_F(Test_DealerRouter, Dealer_2_Router_1)
{
    callbackCount_ = 2;

    std::pmr::map<std::string, network::zeromq::Message> replyMessages{
        std::pair<std::string, network::zeromq::Message>(testMessage2_, {}),
        std::pair<std::string, network::zeromq::Message>(testMessage3_, {})};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessages](auto&& input) -> void {
            EXPECT_EQ(3, input.size());

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
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    routerSocket->Start(endpoint_);

    std::thread dealerSocketThread1(
        &Test_DealerRouter::dealerSocketThread, this, testMessage2_);
    std::thread dealerSocketThread2(
        &Test_DealerRouter::dealerSocketThread, this, testMessage3_);

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

    dealerSocketThread1.join();
    dealerSocketThread2.join();
}

TEST_F(Test_DealerRouter, Dealer_1_Router_2)
{
    callbackCount_ = 2;

    std::thread routerSocketThread1(
        &Test_DealerRouter::routerSocketThread, this, endpoint_);
    std::thread routerSocketThread2(
        &Test_DealerRouter::routerSocketThread, this, endpoint2_);

    auto dealerCallback =
        zmq::ListenCallback::Factory([this](zmq::Message&& input) -> void {
            EXPECT_EQ(2, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.DealerSocket(
        dealerCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);
    dealerSocket->Start(endpoint2_);

    auto sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage2_);

        return out;
    }());

    ASSERT_TRUE(sent);

    sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage3_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);

    routerSocketThread1.join();
    routerSocketThread2.join();
}

TEST_F(Test_DealerRouter, Dealer_Router_Multipart)
{
    auto replyMessage = opentxs::network::zeromq::Message{};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](auto&& input) -> void {
            EXPECT_EQ(5, input.size());
            // Original header + identity frame.
            EXPECT_EQ(2, input.Header().size());
            EXPECT_EQ(2, input.Body().size());

            auto originalFound{false};
            for (const auto& frame : input.Header()) {
                if (testMessage_ == frame.Bytes()) { originalFound = true; }
            }

            EXPECT_TRUE(originalFound);

            for (const auto& frame : input.Body()) {
                bool match = frame.Bytes() == testMessage2_ ||
                             frame.Bytes() == testMessage3_;
                EXPECT_TRUE(match);
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

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](auto&& input) -> void {
            EXPECT_EQ(4, input.size());
            EXPECT_EQ(1, input.Header().size());
            EXPECT_EQ(2, input.Body().size());
            for (const auto& frame : input.Header()) {
                EXPECT_EQ(testMessage_, frame.Bytes());
            }
            for (const auto& frame : input.Body()) {
                bool match = frame.Bytes() == testMessage2_ ||
                             frame.Bytes() == testMessage3_;
                EXPECT_TRUE(match);
            }

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.DealerSocket(
        dealerCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);

    auto multipartMessage = opentxs::network::zeromq::Message{};
    multipartMessage.AddFrame(testMessage_);
    multipartMessage.StartBody();
    multipartMessage.AddFrame(testMessage2_);
    multipartMessage.AddFrame(testMessage3_);

    auto sent = dealerSocket->Send(std::move(multipartMessage));

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (0 == replyMessage.size() && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_NE(0, replyMessage.size());

    sent = routerSocket->Send(std::move(replyMessage));

    ASSERT_TRUE(sent);

    end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}
}  // namespace ottest
