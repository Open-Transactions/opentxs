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
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class Test_RequestRouter : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString test_message_{"zeromq test message"};
    const ot::UnallocatedCString test_message2_{"zeromq test message 2"};
    const ot::UnallocatedCString test_message3_{"zeromq test message 3"};

    const ot::UnallocatedCString endpoint_{
        "inproc://opentxs/test/request_router_test"};

    std::atomic_int callback_finished_count_{0};

    int callback_count_{0};

    void requestSocketThread(const ot::UnallocatedCString& msg);
    void requestSocketThreadMultipart();

    Test_RequestRouter()
        : context_(OTTestEnvironment::GetOT().ZMQ())
    {
    }
};

void Test_RequestRouter::requestSocketThread(const ot::UnallocatedCString& msg)
{
    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = ot::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);
    // RouterSocket removes the identity frame and RequestSocket removes the
    // delimiter.
    ASSERT_EQ(1, message.get().size());

    const auto messageString =
        ot::UnallocatedCString{message.Payload().begin()->Bytes()};
    ASSERT_EQ(msg, messageString);
}

void Test_RequestRouter::requestSocketThreadMultipart()
{
    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint_);

    auto multipartMessage = ot::network::zeromq::Message{};
    multipartMessage.AddFrame(test_message_);
    multipartMessage.StartBody();
    multipartMessage.AddFrame(test_message2_);
    multipartMessage.AddFrame(test_message3_);

    auto [result, message] = requestSocket->Send(std::move(multipartMessage));

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);
    // RouterSocket removes the identity frame and RequestSocket removes the
    // delimiter.
    ASSERT_EQ(4, message.get().size());

    const auto messageHeader =
        ot::UnallocatedCString{message.Envelope().get()[0].Bytes()};

    ASSERT_EQ(test_message_, messageHeader);

    for (const auto& frame : message.Payload()) {
        bool match =
            frame.Bytes() == test_message2_ || frame.Bytes() == test_message3_;
        ASSERT_TRUE(match);
    }
}

TEST_F(Test_RequestRouter, Request_Router)
{
    auto replyMessage = ot::network::zeromq::Message{};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](zmq::Message&& input) -> void {
            // RequestSocket prepends a delimiter and RouterSocket prepends an
            // identity frame.
            EXPECT_EQ(3, input.get().size());
            EXPECT_EQ(1, input.Envelope().get().size());
            EXPECT_EQ(1, input.Payload().size());

            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

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

    // Send the request on a separate thread so this thread can continue and
    // wait for the ListenCallback to finish, then send the reply.
    std::thread requestSocketThread1(
        &Test_RequestRouter::requestSocketThread, this, test_message_);

    auto end = std::time(nullptr) + 5;
    while (!callback_finished_count_ && std::time(nullptr) < end) {
        std::this_thread::sleep_for(100ms);
    }

    ASSERT_EQ(1, callback_finished_count_);

    routerSocket->Send(std::move(replyMessage));

    requestSocketThread1.join();
}

TEST_F(Test_RequestRouter, Request_2_Router_1)
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
            // RequestSocket prepends a delimiter and RouterSocket prepends an
            // identity frame.
            EXPECT_EQ(3, input.get().size());
            EXPECT_EQ(1, input.Envelope().get().size());
            EXPECT_EQ(1, input.Payload().size());

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

    routerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    routerSocket->Start(endpoint_);

    std::thread requestSocketThread1(
        &Test_RequestRouter::requestSocketThread, this, test_message2_);
    std::thread requestSocketThread2(
        &Test_RequestRouter::requestSocketThread, this, test_message3_);

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

    requestSocketThread1.join();
    requestSocketThread2.join();
}

TEST_F(Test_RequestRouter, Request_Router_Multipart)
{
    auto replyMessage = ot::network::zeromq::Message{};

    auto routerCallback = zmq::ListenCallback::Factory(
        [this, &replyMessage](auto&& input) -> void {
            // RequestSocket prepends a delimiter and RouterSocket prepends an
            // identity frame.
            EXPECT_EQ(6, input.get().size());
            // Identity frame.
            EXPECT_EQ(1, input.Envelope().get().size());
            // Original message: header, delimiter, two body parts.
            EXPECT_EQ(4, input.Payload().size());

            for (const auto& frame : input.Payload()) {
                bool match = frame.Bytes() == test_message_ ||
                             frame.Bytes() == test_message2_ ||
                             frame.Bytes() == test_message3_;
                EXPECT_TRUE(match || frame.size() == 0);
            }

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

    // Send the request on a separate thread so this thread can continue and
    // wait for the ListenCallback to finish, then send the reply.
    std::thread requestSocketThread1(
        &Test_RequestRouter::requestSocketThreadMultipart, this);

    auto end = std::time(nullptr) + 15;
    while (0 == replyMessage.get().size() && std::time(nullptr) < end) {
        std::this_thread::sleep_for(100ms);
    }

    auto sent = routerSocket->Send(std::move(replyMessage));

    ASSERT_TRUE(sent);

    requestSocketThread1.join();
}
}  // namespace ottest
