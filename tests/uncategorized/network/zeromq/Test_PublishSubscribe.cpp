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

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/fixtures/zeromq/PublishSubscribe.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

TEST_F(PublishSubscribe, Publish_Subscribe)
{
    auto publishSocket = context_.Internal().PublishSocket();

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(zmq::socket::Type::Publish, publishSocket->Type());

    auto set = publishSocket->SetTimeouts(0ms, 30000ms, -1ms);

    EXPECT_TRUE(set);

    set = publishSocket->Start(endpoint_);

    ASSERT_TRUE(set);

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this](ot::network::zeromq::Message&& input) -> void {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = context_.Internal().SubscribeSocket(listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(zmq::socket::Type::Subscribe, subscribeSocket->Type());

    set = subscribeSocket->SetTimeouts(0ms, -1ms, 30000ms);

    EXPECT_TRUE(set);

    set = subscribeSocket->Start(endpoint_);

    ASSERT_TRUE(set);

    const bool sent = publishSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 30;

    while ((1 > callback_finished_count_) && (std::time(nullptr) < end)) {
        ot::Sleep(1ms);
    }

    EXPECT_EQ(1, callback_finished_count_);
}

TEST_F(PublishSubscribe, Publish_1_Subscribe_2)
{
    subscribe_thread_count_ = 2;
    callback_count_ = 2;

    auto publishSocket = context_.Internal().PublishSocket();

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(zmq::socket::Type::Publish, publishSocket->Type());

    publishSocket->SetTimeouts(0ms, 30000ms, -1ms);
    publishSocket->Start(endpoint_);

    std::thread subscribeSocketThread1(
        &PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_}),
        ot::UnallocatedSet<ot::UnallocatedCString>({test_message_}));
    std::thread subscribeSocketThread2(
        &PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_}),
        ot::UnallocatedSet<ot::UnallocatedCString>({test_message_}));

    auto end = std::time(nullptr) + 30;
    while (subscribe_thread_started_count_ < subscribe_thread_count_ &&
           std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    ASSERT_EQ(subscribe_thread_count_, subscribe_thread_started_count_);

    const bool sent = publishSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_TRUE(sent);

    subscribeSocketThread1.join();
    subscribeSocketThread2.join();
}

TEST_F(PublishSubscribe, Publish_2_Subscribe_1)
{
    subscribe_thread_count_ = 1;
    callback_count_ = 2;

    std::thread publishSocketThread1(
        &PublishSubscribe::publishSocketThread, this, endpoint_, test_message_);
    std::thread publishSocketThread2(
        &PublishSubscribe::publishSocketThread,
        this,
        endpoint2_,
        test_message2_);

    auto end = std::time(nullptr) + 15;
    while (publish_thread_started_count_ < 2 && std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    ASSERT_EQ(2, publish_thread_started_count_);

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this](ot::network::zeromq::Message&& input) -> void {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool const match =
                inputString == test_message_ || inputString == test_message2_;
            EXPECT_TRUE(match);
            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = context_.Internal().SubscribeSocket(listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(zmq::socket::Type::Subscribe, subscribeSocket->Type());

    subscribeSocket->SetTimeouts(0ms, -1ms, 30000ms);
    subscribeSocket->Start(endpoint_);
    subscribeSocket->Start(endpoint2_);

    ++subscribe_thread_started_count_;

    end = std::time(nullptr) + 30;
    while (callback_finished_count_ < callback_count_ &&
           std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    ASSERT_EQ(callback_count_, callback_finished_count_);

    publishSocketThread1.join();
    publishSocketThread2.join();
}

TEST_F(PublishSubscribe, Publish_2_Subscribe_2)
{
    subscribe_thread_count_ = 2;
    callback_count_ = 4;

    std::thread publishSocketThread1(
        &PublishSubscribe::publishSocketThread, this, endpoint_, test_message_);
    std::thread publishSocketThread2(
        &PublishSubscribe::publishSocketThread,
        this,
        endpoint2_,
        test_message2_);

    auto end = std::time(nullptr) + 15;
    while (publish_thread_started_count_ < 2 && std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    ASSERT_EQ(2, publish_thread_started_count_);

    std::thread subscribeSocketThread1(
        &PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_, endpoint2_}),
        ot::UnallocatedSet<ot::UnallocatedCString>(
            {test_message_, test_message2_}));
    std::thread subscribeSocketThread2(
        &PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_, endpoint2_}),
        ot::UnallocatedSet<ot::UnallocatedCString>(
            {test_message_, test_message2_}));

    end = std::time(nullptr) + 30;
    while (subscribe_thread_started_count_ < subscribe_thread_count_ &&
           std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    ASSERT_EQ(subscribe_thread_count_, subscribe_thread_started_count_);

    publishSocketThread1.join();
    publishSocketThread2.join();

    subscribeSocketThread1.join();
    subscribeSocketThread2.join();
}
}  // namespace ottest
