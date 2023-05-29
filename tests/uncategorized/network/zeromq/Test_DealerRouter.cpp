// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>
#include <ctime>
#include <span>
#include <string_view>
#include <thread>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class Test_DealerRouter : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString test_message_{"zeromq test message"};
    const ot::UnallocatedCString test_message2_{"zeromq test message 2"};
    const ot::UnallocatedCString test_message3_{"zeromq test message 3"};

    const ot::UnallocatedCString endpoint_{
        "inproc://opentxs/test/dealer_router_test"};
    const ot::UnallocatedCString endpoint2_{
        "inproc://opentxs/test/dealer_router_test2"};

    std::atomic_int callback_finished_count_{0};

    int callback_count_{0};

    void dealerSocketThread(const ot::UnallocatedCString& msg);
    void routerSocketThread(const ot::UnallocatedCString& endpoint);

    Test_DealerRouter()
        : context_(OTTestEnvironment::GetOT().ZMQ())
    {
    }
};

void Test_DealerRouter::dealerSocketThread(const ot::UnallocatedCString& msg)
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

void Test_DealerRouter::routerSocketThread(
    const ot::UnallocatedCString& endpoint)
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

TEST_F(Test_DealerRouter, Dealer_Router)
{
    auto replyMessage = opentxs::network::zeromq::Message{};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](auto&& input) -> void {
            EXPECT_EQ(3, input.get().size());
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            replyMessage = ot::network::zeromq::reply_to_message(input);
            for (auto& frame : input.Payload()) {
                replyMessage.AddFrame(frame);
            }
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.Internal().RouterSocket(
        routerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(0ms, 30000ms, -1ms);
    routerSocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](auto&& input) -> void {
            EXPECT_EQ(2, input.get().size());
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

    auto sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (0 == replyMessage.get().size() && std::time(nullptr) < end) {
        ot::Sleep(100ms);
    }

    ASSERT_NE(0, replyMessage.get().size());

    sent = routerSocket->Send(std::move(replyMessage));

    ASSERT_TRUE(sent);

    end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) { ot::Sleep(100ms); }

    EXPECT_TRUE(replyProcessed);
}

TEST_F(Test_DealerRouter, Dealer_2_Router_1)
{
    callback_count_ = 2;

    ot::UnallocatedMap<ot::UnallocatedCString, ot::network::zeromq::Message>
        replyMessages{
            std::pair<ot::UnallocatedCString, ot::network::zeromq::Message>(
                test_message2_, {}),
            std::pair<ot::UnallocatedCString, ot::network::zeromq::Message>(
                test_message3_, {})};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessages](auto&& input) -> void {
            EXPECT_EQ(3, input.get().size());

            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool match =
                inputString == test_message2_ || inputString == test_message3_;
            EXPECT_TRUE(match);

            auto& replyMessage = replyMessages.at(inputString);
            replyMessage = ot::network::zeromq::reply_to_message(input);
            for (const auto& frame : input.Payload()) {
                replyMessage.AddFrame(frame);
            }

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.Internal().RouterSocket(
        routerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(0ms, 30000ms, -1ms);
    routerSocket->Start(endpoint_);

    std::thread dealerSocketThread1(
        &Test_DealerRouter::dealerSocketThread, this, test_message2_);
    std::thread dealerSocketThread2(
        &Test_DealerRouter::dealerSocketThread, this, test_message3_);

    const auto& replyMessage1 = replyMessages.at(test_message2_);
    const auto& replyMessage2 = replyMessages.at(test_message3_);

    auto end = std::time(nullptr) + 15;
    while (!callback_finished_count_ && std::time(nullptr) < end) {
        std::this_thread::sleep_for(100ms);
    }

    bool message1Sent{false};
    if (0 != replyMessage1.get().size()) {
        routerSocket->Send(ot::network::zeromq::Message{replyMessage1});
        message1Sent = true;
    } else {
        routerSocket->Send(ot::network::zeromq::Message{replyMessage2});
    }

    end = std::time(nullptr) + 15;
    while (callback_finished_count_ < callback_count_ &&
           std::time(nullptr) < end) {
        std::this_thread::sleep_for(100ms);
    }

    if (false == message1Sent) {
        routerSocket->Send(ot::network::zeromq::Message{replyMessage1});
    } else {
        routerSocket->Send(ot::network::zeromq::Message{replyMessage2});
    }

    ASSERT_EQ(callback_count_, callback_finished_count_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
}

TEST_F(Test_DealerRouter, Dealer_1_Router_2)
{
    callback_count_ = 2;

    std::thread routerSocketThread1(
        &Test_DealerRouter::routerSocketThread, this, endpoint_);
    std::thread routerSocketThread2(
        &Test_DealerRouter::routerSocketThread, this, endpoint2_);

    auto dealerCallback =
        zmq::ListenCallback::Factory([this](zmq::Message&& input) -> void {
            EXPECT_EQ(2, input.get().size());
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool match =
                inputString == test_message2_ || inputString == test_message3_;
            EXPECT_TRUE(match);

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.Internal().DealerSocket(
        dealerCallback, zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    dealerSocket->Start(endpoint_);
    dealerSocket->Start(endpoint2_);

    auto sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message2_);

        return out;
    }());

    ASSERT_TRUE(sent);

    sent = dealerSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message3_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (callback_finished_count_ < callback_count_ &&
           std::time(nullptr) < end) {
        std::this_thread::sleep_for(100ms);
    }

    ASSERT_EQ(callback_count_, callback_finished_count_);

    routerSocketThread1.join();
    routerSocketThread2.join();
}

TEST_F(Test_DealerRouter, Dealer_Router_Multipart)
{
    auto replyMessage = opentxs::network::zeromq::Message{};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](auto&& input) -> void {
            auto envelope = input.Envelope();
            auto payload = input.Payload();

            EXPECT_EQ(5, input.get().size());
            // Original header + identity frame.
            EXPECT_EQ(2, envelope.get().size());
            EXPECT_EQ(2, payload.size());

            auto originalFound{false};

            for (const auto& frame : envelope.get()) {
                if (test_message_ == frame.Bytes()) { originalFound = true; }
            }

            EXPECT_TRUE(originalFound);

            for (const auto& frame : payload) {
                bool match = frame.Bytes() == test_message2_ ||
                             frame.Bytes() == test_message3_;
                EXPECT_TRUE(match);
            }

            replyMessage = ot::network::zeromq::reply_to_message(
                std::move(envelope), true);
            replyMessage.MoveFrames(payload);
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.Internal().RouterSocket(
        routerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(0ms, 30000ms, -1ms);
    routerSocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](auto&& input) -> void {
            const auto envelope = input.Envelope();
            const auto payload = input.Payload();

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
    multipartMessage.AddFrame(test_message_);
    multipartMessage.StartBody();
    multipartMessage.AddFrame(test_message2_);
    multipartMessage.AddFrame(test_message3_);

    auto sent = dealerSocket->Send(std::move(multipartMessage));

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (0 == replyMessage.get().size() && std::time(nullptr) < end) {
        ot::Sleep(100ms);
    }

    ASSERT_NE(0, replyMessage.get().size());

    sent = routerSocket->Send(std::move(replyMessage));

    ASSERT_TRUE(sent);

    end = std::time(nullptr) + 15;
    while (!replyProcessed && std::time(nullptr) < end) { ot::Sleep(100ms); }

    EXPECT_TRUE(replyProcessed);
}
}  // namespace ottest
