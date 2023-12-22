// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>
#include <ctime>
#include <span>
#include <thread>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/ReplyCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/fixtures/zeromq/RouterDealer.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

TEST_F(RouterDealer, Router_Dealer)
{
    auto dealerCallback =
        zmq::ListenCallback::Factory([this](auto&& input) -> void {
            EXPECT_EQ(3, input.get().size());

            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            auto sent = router_socket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.Internal().DealerSocket(
        dealerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(0ms, 30000ms, -1ms);
    dealerSocket->Start(dealer_endpoint_);

    dealer_socket_ = &dealerSocket;

    auto routerCallback =
        zmq::ListenCallback::Factory([this](auto&& input) -> void {
            EXPECT_EQ(3, input.get().size());

            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            auto sent = dealer_socket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.Internal().RouterSocket(
        routerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    routerSocket->Start(router_endpoint_);

    router_socket_ = &routerSocket;

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](auto&& input) -> ot::network::zeromq::Message {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);
            ++callback_finished_count_;

            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.Internal().ReplySocket(
        replyCallback, zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(0ms, 30000ms, -1ms);
    replySocket->Start(dealer_endpoint_);

    // Send the request on a separate thread so this thread can continue and
    // wait for replyCallback to finish.
    std::thread requestSocketThread1(
        &RouterDealer::requestSocketThread,
        this,
        router_endpoint_,
        test_message_);

    auto end = std::time(nullptr) + 30;
    while (callback_finished_count_ < 3 && std::time(nullptr) < end) {
        std::this_thread::sleep_for(100ms);
    }

    ASSERT_EQ(3, callback_finished_count_);

    requestSocketThread1.join();

    router_socket_ = nullptr;
    dealer_socket_ = nullptr;
}

TEST_F(RouterDealer, Dealer_3_Router_Dealer_Reply)
{
    auto dealerCallback =
        zmq::ListenCallback::Factory([this](auto&& input) -> void {
            EXPECT_EQ(3, input.get().size());
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool const match = inputString == test_message_ ||
                               inputString == test_message2_ ||
                               inputString == test_message3_;

            EXPECT_TRUE(match);

            auto sent = router_socket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.Internal().DealerSocket(
        dealerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(0ms, 30000ms, -1ms);
    dealerSocket->Start(dealer_endpoint_);

    dealer_socket_ = &dealerSocket;

    auto routerCallback =
        zmq::ListenCallback::Factory([this](auto&& input) -> void {
            EXPECT_EQ(3, input.get().size());
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool const match = inputString == test_message_ ||
                               inputString == test_message2_ ||
                               inputString == test_message3_;

            EXPECT_TRUE(match);

            auto sent = dealer_socket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.Internal().RouterSocket(
        routerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    routerSocket->Start(router_endpoint_);

    router_socket_ = &routerSocket;

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](auto&& input) -> ot::network::zeromq::Message {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool const match = inputString == test_message_ ||
                               inputString == test_message2_ ||
                               inputString == test_message3_;

            EXPECT_TRUE(match);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);

            ++callback_finished_count_;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.Internal().ReplySocket(
        replyCallback, zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(0ms, 30000ms, -1ms);
    replySocket->Start(dealer_endpoint_);

    // Send the requests on separate threads so this thread can continue and
    // wait for clientRouterCallback to finish.
    std::thread dealerSocketThread1(
        &RouterDealer::dealerSocketThread,
        this,
        router_endpoint_,
        test_message_);

    std::thread dealerSocketThread2(
        &RouterDealer::dealerSocketThread,
        this,
        router_endpoint_,
        test_message2_);

    std::thread dealerSocketThread3(
        &RouterDealer::dealerSocketThread,
        this,
        router_endpoint_,
        test_message3_);

    auto end = std::time(nullptr) + 30;
    while (callback_finished_count_ < 9 && std::time(nullptr) < end) {
        std::this_thread::sleep_for(100ms);
    }

    ASSERT_EQ(9, callback_finished_count_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
    dealerSocketThread3.join();

    router_socket_ = nullptr;
    dealer_socket_ = nullptr;
}

TEST_F(RouterDealer, Dealer_3_Router_Dealer_Router)
{
    auto dealerCallback =
        zmq::ListenCallback::Factory([this](auto&& input) -> void {
            EXPECT_EQ(3, input.get().size());
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool const match = inputString == test_message_ ||
                               inputString == test_message2_ ||
                               inputString == test_message3_;

            EXPECT_TRUE(match);

            auto sent = router_socket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = context_.Internal().DealerSocket(
        dealerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(0ms, 30000ms, -1ms);
    dealerSocket->Start(dealer_endpoint_);

    dealer_socket_ = &dealerSocket;

    auto routerCallback =
        zmq::ListenCallback::Factory([this](auto&& input) -> void {
            EXPECT_EQ(3, input.get().size());
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool const match = inputString == test_message_ ||
                               inputString == test_message2_ ||
                               inputString == test_message3_;

            EXPECT_TRUE(match);

            auto sent = dealer_socket_->get().Send(std::move(input));

            EXPECT_TRUE(sent);

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &routerCallback.get());

    auto routerSocket = context_.Internal().RouterSocket(
        routerCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &routerSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, routerSocket->Type());

    routerSocket->SetTimeouts(0ms, -1ms, 30000ms);
    routerSocket->Start(router_endpoint_);

    router_socket_ = &routerSocket;

    ot::UnallocatedVector<ot::network::zeromq::Message> replyMessages;

    auto clientRouterCallback = zmq::ListenCallback::Factory(
        [this,
         &replyMessages](const ot::network::zeromq::Message&& input) -> void {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool const match = inputString == test_message_ ||
                               inputString == test_message2_ ||
                               inputString == test_message3_;

            EXPECT_TRUE(match);

            auto replyMessage = ot::network::zeromq::Message{};
            for (const auto& frame : input.get()) {
                if (0 < frame.size()) {
                    replyMessage.AddFrame(frame);
                } else {
                    replyMessage.AddFrame();
                }
            }
            replyMessages.push_back(replyMessage);

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &clientRouterCallback.get());

    auto clientRouterSocket = context_.Internal().RouterSocket(
        clientRouterCallback, zmq::socket::Direction::Connect);

    ASSERT_NE(nullptr, &clientRouterSocket.get());
    ASSERT_EQ(zmq::socket::Type::Router, clientRouterSocket->Type());

    clientRouterSocket->SetTimeouts(0ms, 30000ms, -1ms);
    clientRouterSocket->Start(dealer_endpoint_);

    // Send the requests on separate threads so this thread can continue and
    // wait for clientRouterCallback to finish.
    std::thread dealerSocketThread1(
        &RouterDealer::dealerSocketThread,
        this,
        router_endpoint_,
        test_message_);

    std::thread dealerSocketThread2(
        &RouterDealer::dealerSocketThread,
        this,
        router_endpoint_,
        test_message2_);

    std::thread dealerSocketThread3(
        &RouterDealer::dealerSocketThread,
        this,
        router_endpoint_,
        test_message3_);

    auto end = std::time(nullptr) + 30;
    while (callback_finished_count_ < 6 && std::time(nullptr) < end) {
        std::this_thread::sleep_for(100ms);
    }

    ASSERT_EQ(6, callback_finished_count_);

    for (auto replyMessage : replyMessages) {
        clientRouterSocket->Send(std::move(replyMessage));
    }

    end = std::time(nullptr) + 30;
    while (callback_finished_count_ < 9 && std::time(nullptr) < end) {
        std::this_thread::sleep_for(100ms);
    }

    ASSERT_EQ(9, callback_finished_count_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
    dealerSocketThread3.join();

    router_socket_ = nullptr;
    dealer_socket_ = nullptr;
}
}  // namespace ottest
