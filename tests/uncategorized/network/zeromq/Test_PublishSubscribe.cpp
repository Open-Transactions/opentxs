// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>
#include <ctime>
#include <ratio>
#include <thread>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/util/Pimpl.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class Test_PublishSubscribe : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString test_message_{"zeromq test message"};
    const ot::UnallocatedCString test_message2_{"zeromq test message 2"};

    const ot::UnallocatedCString endpoint_{
        "inproc://opentxs/test/publish_subscribe_test"};
    const ot::UnallocatedCString endpoint2_{
        "inproc://opentxs/test/publish_subscribe_test2"};

    std::atomic_int callback_finished_count_{0};
    std::atomic_int subscribe_thread_started_count_{0};
    std::atomic_int publish_thread_started_count_{0};

    int subscribe_thread_count_{0};
    int callback_count_{0};

    void subscribeSocketThread(
        const ot::UnallocatedSet<ot::UnallocatedCString>& endpoints,
        const ot::UnallocatedSet<ot::UnallocatedCString>& msgs);
    void publishSocketThread(
        const ot::UnallocatedCString& endpoint,
        const ot::UnallocatedCString& msg);

    Test_PublishSubscribe()
        : context_(ot::Context().ZMQ())
    {
    }
};

void Test_PublishSubscribe::subscribeSocketThread(
    const ot::UnallocatedSet<ot::UnallocatedCString>& endpoints,
    const ot::UnallocatedSet<ot::UnallocatedCString>& msgs)
{
    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, msgs](ot::network::zeromq::Message&& input) -> void {
            const auto inputString =
                ot::UnallocatedCString{input.Body().begin()->Bytes()};
            bool found = msgs.count(inputString);
            EXPECT_TRUE(found);
            ++callback_finished_count_;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = context_.Internal().SubscribeSocket(listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(zmq::socket::Type::Subscribe, subscribeSocket->Type());

    subscribeSocket->SetTimeouts(0ms, -1ms, 30000ms);
    for (auto endpoint : endpoints) { subscribeSocket->Start(endpoint); }

    ++subscribe_thread_started_count_;

    auto end = std::time(nullptr) + 30;
    while (callback_finished_count_ < callback_count_ &&
           std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    ASSERT_EQ(callback_count_, callback_finished_count_);
}

void Test_PublishSubscribe::publishSocketThread(
    const ot::UnallocatedCString& endpoint,
    const ot::UnallocatedCString& msg)
{
    auto publishSocket = context_.Internal().PublishSocket();

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(zmq::socket::Type::Publish, publishSocket->Type());

    publishSocket->SetTimeouts(0ms, 30000ms, -1ms);
    publishSocket->Start(endpoint);

    ++publish_thread_started_count_;

    auto end = std::time(nullptr) + 15;
    while (subscribe_thread_started_count_ < subscribe_thread_count_ &&
           std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    bool sent = publishSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_TRUE(sent);

    end = std::time(nullptr) + 15;
    while (callback_finished_count_ < callback_count_ &&
           std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    ASSERT_EQ(callback_count_, callback_finished_count_);
}

TEST_F(Test_PublishSubscribe, Publish_Subscribe)
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
                ot::UnallocatedCString{input.Body().begin()->Bytes()};

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

    bool sent = publishSocket->Send([&] {
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

TEST_F(Test_PublishSubscribe, Publish_1_Subscribe_2)
{
    subscribe_thread_count_ = 2;
    callback_count_ = 2;

    auto publishSocket = context_.Internal().PublishSocket();

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(zmq::socket::Type::Publish, publishSocket->Type());

    publishSocket->SetTimeouts(0ms, 30000ms, -1ms);
    publishSocket->Start(endpoint_);

    std::thread subscribeSocketThread1(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_}),
        ot::UnallocatedSet<ot::UnallocatedCString>({test_message_}));
    std::thread subscribeSocketThread2(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_}),
        ot::UnallocatedSet<ot::UnallocatedCString>({test_message_}));

    auto end = std::time(nullptr) + 30;
    while (subscribe_thread_started_count_ < subscribe_thread_count_ &&
           std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    ASSERT_EQ(subscribe_thread_count_, subscribe_thread_started_count_);

    bool sent = publishSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_TRUE(sent);

    subscribeSocketThread1.join();
    subscribeSocketThread2.join();
}

TEST_F(Test_PublishSubscribe, Publish_2_Subscribe_1)
{
    subscribe_thread_count_ = 1;
    callback_count_ = 2;

    std::thread publishSocketThread1(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint_,
        test_message_);
    std::thread publishSocketThread2(
        &Test_PublishSubscribe::publishSocketThread,
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
                ot::UnallocatedCString{input.Body().begin()->Bytes()};
            bool match =
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

TEST_F(Test_PublishSubscribe, Publish_2_Subscribe_2)
{
    subscribe_thread_count_ = 2;
    callback_count_ = 4;

    std::thread publishSocketThread1(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint_,
        test_message_);
    std::thread publishSocketThread2(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint2_,
        test_message2_);

    auto end = std::time(nullptr) + 15;
    while (publish_thread_started_count_ < 2 && std::time(nullptr) < end) {
        std::this_thread::sleep_for(1s);
    }

    ASSERT_EQ(2, publish_thread_started_count_);

    std::thread subscribeSocketThread1(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_, endpoint2_}),
        ot::UnallocatedSet<ot::UnallocatedCString>(
            {test_message_, test_message2_}));
    std::thread subscribeSocketThread2(
        &Test_PublishSubscribe::subscribeSocketThread,
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
