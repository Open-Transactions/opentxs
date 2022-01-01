// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <ctime>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

using namespace opentxs;
using namespace opentxs::network;

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class Test_RouterDealer : public ::testing::Test
{
public:
    const zmq::Context& context_;
    OTZMQRouterSocket* routerSocket_;
    OTZMQDealerSocket* dealerSocket_;
    const std::string testMessage_;
    const std::string testMessage2_;
    const std::string testMessage3_;
    const std::string routerEndpoint_;
    const std::string dealerEndpoint_;
    std::atomic_int callbackFinishedCount_;

    void requestSocketThread(
        const std::string& endpoint,
        const std::string& msg);

    void dealerSocketThread(
        const std::string& endpoint,
        const std::string& msg);

    Test_RouterDealer()
        : context_(Context().ZMQ())
        , routerSocket_(nullptr)
        , dealerSocket_(nullptr)
        , testMessage_("zeromq test message")
        , testMessage2_("zeromq test message 2")
        , testMessage3_("zeromq test message 3")
        , routerEndpoint_("inproc://opentxs/test/router")
        , dealerEndpoint_("inproc://opentxs/test/dealer")
        , callbackFinishedCount_(0)
    {
    }
    Test_RouterDealer(const Test_RouterDealer&) = delete;
    Test_RouterDealer(Test_RouterDealer&&) = delete;
    Test_RouterDealer& operator=(const Test_RouterDealer&) = delete;
    Test_RouterDealer& operator=(Test_RouterDealer&&) = delete;
};

void Test_RouterDealer::requestSocketThread(
    const std::string& endpoint,
    const std::string& msg)
{
    auto requestSocket = context_.RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    requestSocket->Start(endpoint);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_EQ(result, SendResult::VALID_REPLY);

    const auto messageString = std::string{message.Body().begin()->Bytes()};
    ASSERT_EQ(msg, messageString);
}

void Test_RouterDealer::dealerSocketThread(
    const std::string& endpoint,
    const std::string& msg)
{
    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [&replyProcessed, msg](network::zeromq::Message&& input) -> void {
            EXPECT_EQ(2, input.size());

            const auto inputString = std::string{input.Body().begin()->Bytes()};

            EXPECT_EQ(msg, inputString);

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
    dealerSocket->Start(endpoint);

    auto sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) {
        Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(replyProcessed);
}

TEST_F(Test_RouterDealer, Router_Dealer)
{
    auto dealerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message&& input) -> void {
            EXPECT_EQ(3, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            auto sent = routerSocket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.DealerSocket(
        dealerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    dealerSocket->Start(dealerEndpoint_);

    dealerSocket_ = &dealerSocket;

    auto routerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message&& input) -> void {
            EXPECT_EQ(3, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            auto sent = dealerSocket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

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
    routerSocket->Start(routerEndpoint_);

    routerSocket_ = &routerSocket;

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const network::zeromq::Message&& input)
            -> network::zeromq::Message {
            const auto inputString = std::string{input.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);

            ++callbackFinishedCount_;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.ReplySocket(
        replyCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(dealerEndpoint_);

    // Send the request on a separate thread so this thread can continue and
    // wait for replyCallback to finish.
    std::thread requestSocketThread1(
        &Test_RouterDealer::requestSocketThread,
        this,
        routerEndpoint_,
        testMessage_);

    auto end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < 3 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(3, callbackFinishedCount_);

    requestSocketThread1.join();

    routerSocket_ = nullptr;
    dealerSocket_ = nullptr;
}

TEST_F(Test_RouterDealer, Dealer_3_Router_Dealer_Reply)
{
    auto dealerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message&& input) -> void {
            EXPECT_EQ(3, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto sent = routerSocket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.DealerSocket(
        dealerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    dealerSocket->Start(dealerEndpoint_);

    dealerSocket_ = &dealerSocket;

    auto routerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message&& input) -> void {
            EXPECT_EQ(3, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto sent = dealerSocket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

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
    routerSocket->Start(routerEndpoint_);

    routerSocket_ = &routerSocket;

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const network::zeromq::Message&& input)
            -> network::zeromq::Message {
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);

            ++callbackFinishedCount_;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.ReplySocket(
        replyCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(dealerEndpoint_);

    // Send the requests on separate threads so this thread can continue and
    // wait for clientRouterCallback to finish.
    std::thread dealerSocketThread1(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage_);

    std::thread dealerSocketThread2(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage2_);

    std::thread dealerSocketThread3(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage3_);

    auto end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < 9 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(9, callbackFinishedCount_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
    dealerSocketThread3.join();

    routerSocket_ = nullptr;
    dealerSocket_ = nullptr;
}

TEST_F(Test_RouterDealer, Dealer_3_Router_Dealer_Router)
{
    auto dealerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message&& input) -> void {
            EXPECT_EQ(3, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto sent = routerSocket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.DealerSocket(
        dealerCallback, zmq::socket::Socket::Direction::Bind);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    dealerSocket->Start(dealerEndpoint_);

    dealerSocket_ = &dealerSocket;

    auto routerCallback =
        zmq::ListenCallback::Factory([this](zeromq::Message&& input) -> void {
            EXPECT_EQ(3, input.size());
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto sent = dealerSocket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

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
    routerSocket->Start(routerEndpoint_);

    routerSocket_ = &routerSocket;

    std::pmr::vector<network::zeromq::Message> replyMessages;

    auto clientRouterCallback = zmq::ListenCallback::Factory(
        [this, &replyMessages](const network::zeromq::Message&& input) -> void {
            const auto inputString = std::string{input.Body().begin()->Bytes()};
            bool match = inputString == testMessage_ ||
                         inputString == testMessage2_ ||
                         inputString == testMessage3_;

            EXPECT_TRUE(match);

            auto replyMessage = zeromq::Message{};
            for (auto it = input.begin(); it != input.end(); ++it) {
                auto& frame = *it;
                if (0 < frame.size()) {
                    OTData data = Data::Factory(frame.data(), frame.size());
                    replyMessage.AddFrame(data.get());
                } else {
                    replyMessage.AddFrame();
                }
            }
            replyMessages.push_back(replyMessage);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &clientRouterCallback.get());

    auto clientRouterSocket = context_.RouterSocket(
        clientRouterCallback, zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &clientRouterSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, clientRouterSocket->Type());

    clientRouterSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    clientRouterSocket->Start(dealerEndpoint_);

    // Send the requests on separate threads so this thread can continue and
    // wait for clientRouterCallback to finish.
    std::thread dealerSocketThread1(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage_);

    std::thread dealerSocketThread2(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage2_);

    std::thread dealerSocketThread3(
        &Test_RouterDealer::dealerSocketThread,
        this,
        routerEndpoint_,
        testMessage3_);

    auto end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < 6 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(6, callbackFinishedCount_);

    for (auto replyMessage : replyMessages) {
        clientRouterSocket->Send(std::move(replyMessage));
    }

    end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < 9 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(9, callbackFinishedCount_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
    dealerSocketThread3.join();

    routerSocket_ = nullptr;
    dealerSocket_ = nullptr;
}
}  // namespace ottest
