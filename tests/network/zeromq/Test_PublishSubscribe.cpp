// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <ctime>
#include <thread>

#include "opentxs/OT.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class Test_PublishSubscribe : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString testMessage_{"zeromq test message"};
    const ot::UnallocatedCString testMessage2_{"zeromq test message 2"};

    const ot::UnallocatedCString endpoint_{
        "inproc://opentxs/test/publish_subscribe_test"};
    const ot::UnallocatedCString endpoint2_{
        "inproc://opentxs/test/publish_subscribe_test2"};

    std::atomic_int callbackFinishedCount_{0};
    std::atomic_int subscribeThreadStartedCount_{0};
    std::atomic_int publishThreadStartedCount_{0};

    int subscribeThreadCount_{0};
    int callbackCount_{0};

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
            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = context_.SubscribeSocket(listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(zmq::socket::Type::Subscribe, subscribeSocket->Type());

    subscribeSocket->SetTimeouts(0ms, -1ms, 30000ms);
    for (auto endpoint : endpoints) { subscribeSocket->Start(endpoint); }

    ++subscribeThreadStartedCount_;

    auto end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(1s);

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);
}

void Test_PublishSubscribe::publishSocketThread(
    const ot::UnallocatedCString& endpoint,
    const ot::UnallocatedCString& msg)
{
    auto publishSocket = context_.PublishSocket();
    ;

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(zmq::socket::Type::Publish, publishSocket->Type());

    publishSocket->SetTimeouts(0ms, 30000ms, -1ms);
    publishSocket->Start(endpoint);

    ++publishThreadStartedCount_;

    auto end = std::time(nullptr) + 15;
    while (subscribeThreadStartedCount_ < subscribeThreadCount_ &&
           std::time(nullptr) < end)
        std::this_thread::sleep_for(1s);

    bool sent = publishSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_TRUE(sent);

    end = std::time(nullptr) + 15;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(1s);

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);
}

TEST_F(Test_PublishSubscribe, Publish_Subscribe)
{
    auto publishSocket = context_.PublishSocket();
    ;

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

            EXPECT_EQ(testMessage_, inputString);

            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = context_.SubscribeSocket(listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(zmq::socket::Type::Subscribe, subscribeSocket->Type());

    set = subscribeSocket->SetTimeouts(0ms, -1ms, 30000ms);

    EXPECT_TRUE(set);

    set = subscribeSocket->Start(endpoint_);

    ASSERT_TRUE(set);

    bool sent = publishSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 30;

    while ((1 > callbackFinishedCount_) && (std::time(nullptr) < end)) {
        ot::Sleep(1ms);
    }

    EXPECT_EQ(1, callbackFinishedCount_);
}

TEST_F(Test_PublishSubscribe, Publish_1_Subscribe_2)
{
    subscribeThreadCount_ = 2;
    callbackCount_ = 2;

    auto publishSocket = context_.PublishSocket();

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(zmq::socket::Type::Publish, publishSocket->Type());

    publishSocket->SetTimeouts(0ms, 30000ms, -1ms);
    publishSocket->Start(endpoint_);

    std::thread subscribeSocketThread1(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_}),
        ot::UnallocatedSet<ot::UnallocatedCString>({testMessage_}));
    std::thread subscribeSocketThread2(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_}),
        ot::UnallocatedSet<ot::UnallocatedCString>({testMessage_}));

    auto end = std::time(nullptr) + 30;
    while (subscribeThreadStartedCount_ < subscribeThreadCount_ &&
           std::time(nullptr) < end)
        std::this_thread::sleep_for(1s);

    ASSERT_EQ(subscribeThreadCount_, subscribeThreadStartedCount_);

    bool sent = publishSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    subscribeSocketThread1.join();
    subscribeSocketThread2.join();
}

TEST_F(Test_PublishSubscribe, Publish_2_Subscribe_1)
{
    subscribeThreadCount_ = 1;
    callbackCount_ = 2;

    std::thread publishSocketThread1(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint_,
        testMessage_);
    std::thread publishSocketThread2(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint2_,
        testMessage2_);

    auto end = std::time(nullptr) + 15;
    while (publishThreadStartedCount_ < 2 && std::time(nullptr) < end)
        std::this_thread::sleep_for(1s);

    ASSERT_EQ(2, publishThreadStartedCount_);

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this](ot::network::zeromq::Message&& input) -> void {
            const auto inputString =
                ot::UnallocatedCString{input.Body().begin()->Bytes()};
            bool match =
                inputString == testMessage_ || inputString == testMessage2_;
            EXPECT_TRUE(match);
            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = context_.SubscribeSocket(listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(zmq::socket::Type::Subscribe, subscribeSocket->Type());

    subscribeSocket->SetTimeouts(0ms, -1ms, 30000ms);
    subscribeSocket->Start(endpoint_);
    subscribeSocket->Start(endpoint2_);

    ++subscribeThreadStartedCount_;

    end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(1s);

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);

    publishSocketThread1.join();
    publishSocketThread2.join();
}

TEST_F(Test_PublishSubscribe, Publish_2_Subscribe_2)
{
    subscribeThreadCount_ = 2;
    callbackCount_ = 4;

    std::thread publishSocketThread1(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint_,
        testMessage_);
    std::thread publishSocketThread2(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint2_,
        testMessage2_);

    auto end = std::time(nullptr) + 15;
    while (publishThreadStartedCount_ < 2 && std::time(nullptr) < end)
        std::this_thread::sleep_for(1s);

    ASSERT_EQ(2, publishThreadStartedCount_);

    std::thread subscribeSocketThread1(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_, endpoint2_}),
        ot::UnallocatedSet<ot::UnallocatedCString>(
            {testMessage_, testMessage2_}));
    std::thread subscribeSocketThread2(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        ot::UnallocatedSet<ot::UnallocatedCString>({endpoint_, endpoint2_}),
        ot::UnallocatedSet<ot::UnallocatedCString>(
            {testMessage_, testMessage2_}));

    end = std::time(nullptr) + 30;
    while (subscribeThreadStartedCount_ < subscribeThreadCount_ &&
           std::time(nullptr) < end)
        std::this_thread::sleep_for(1s);

    ASSERT_EQ(subscribeThreadCount_, subscribeThreadStartedCount_);

    publishSocketThread1.join();
    publishSocketThread2.join();

    subscribeSocketThread1.join();
    subscribeSocketThread2.join();
}
}  // namespace ottest
