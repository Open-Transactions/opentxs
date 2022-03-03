// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <ctime>
#include <future>
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
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

#define TEST_ENDPOINT "inproc://opentxs/pairsocket_endpoint"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class Test_PairSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString testMessage_{"zeromq test message"};
    const ot::UnallocatedCString testMessage2_{"zeromq test message 2"};

    ot::OTZMQPairSocket* pairSocket_;

    void pairSocketThread(
        const ot::UnallocatedCString& msg,
        std::promise<void>* promise);

    Test_PairSocket()
        : context_(ot::Context().ZMQ())
        , pairSocket_(nullptr)
    {
    }
    Test_PairSocket(const Test_PairSocket&) = delete;
    Test_PairSocket(Test_PairSocket&&) = delete;
    Test_PairSocket& operator=(const Test_PairSocket&) = delete;
    Test_PairSocket& operator=(Test_PairSocket&&) = delete;
};

void Test_PairSocket::pairSocketThread(
    const ot::UnallocatedCString& message,
    std::promise<void>* promise)
{
    struct Cleanup {
        Cleanup(std::promise<void>& promise)
            : promise_(promise)
        {
        }

        ~Cleanup()
        {
            try {
                promise_.set_value();
            } catch (...) {
            }
        }

    private:
        std::promise<void>& promise_;
    };

    auto cleanup = Cleanup(*promise);
    bool callbackFinished = false;
    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [&callbackFinished,
         &message](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const auto inputString =
                ot::UnallocatedCString{msg.Body().begin()->Bytes()};

            EXPECT_EQ(message, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());
    ASSERT_NE(nullptr, pairSocket_);

    auto pairSocket = context_.PairSocket(listenCallback, *pairSocket_);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    promise->set_value();
    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) { ot::Sleep(100ms); }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Factory1)
{
    auto pairSocket =
        context_.PairSocket(ot::network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());
}

TEST_F(Test_PairSocket, PairSocket_Factory2)
{
    auto peer =
        context_.PairSocket(ot::network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    auto pairSocket = context_.PairSocket(
        ot::network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());
    ASSERT_EQ(pairSocket->Endpoint(), peer->Endpoint());
}

TEST_F(Test_PairSocket, PairSocket_Factory3)
{
    auto pairSocket = context_.PairSocket(
        ot::network::zeromq::ListenCallback::Factory(), TEST_ENDPOINT);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());
    ASSERT_EQ(pairSocket->Endpoint(), TEST_ENDPOINT);
}

TEST_F(Test_PairSocket, PairSocket_Send1)
{
    bool callbackFinished = false;

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const auto inputString =
                ot::UnallocatedCString{msg.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto peer = context_.PairSocket(listenCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    auto pairSocket = context_.PairSocket(
        ot::network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) { ot::Sleep(100ms); }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Send2)
{
    bool callbackFinished = false;

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const auto inputString =
                ot::UnallocatedCString{msg.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto peer = context_.PairSocket(listenCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    auto pairSocket = context_.PairSocket(
        ot::network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) { ot::Sleep(100ms); }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Send3)
{
    bool callbackFinished = false;

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const auto inputString =
                ot::UnallocatedCString{msg.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto peer = context_.PairSocket(listenCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    auto pairSocket = context_.PairSocket(
        ot::network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) { ot::Sleep(100ms); }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Send_Two_Way)
{
    bool peerCallbackFinished = false;

    auto peerCallback = ot::network::zeromq::ListenCallback::Factory(
        [this,
         &peerCallbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const auto inputString =
                ot::UnallocatedCString{msg.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage_, inputString);

            peerCallbackFinished = true;
        });

    ASSERT_NE(nullptr, &peerCallback.get());

    auto peer = context_.PairSocket(peerCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    bool callbackFinished = false;

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.size());
            const auto inputString =
                ot::UnallocatedCString{msg.Body().begin()->Bytes()};

            EXPECT_EQ(testMessage2_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto pairSocket = context_.PairSocket(listenCallback, peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    sent = peer->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage2_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!peerCallbackFinished && !callbackFinished &&
           std::time(nullptr) < end) {
        ot::Sleep(100ms);
    }

    ASSERT_TRUE(peerCallbackFinished);
    ASSERT_TRUE(callbackFinished);
}

TEST_F(Test_PairSocket, PairSocket_Send_Separate_Thread)
{
    auto pairSocket =
        context_.PairSocket(ot::network::zeromq::ListenCallback::Factory());
    pairSocket_ = &pairSocket;
    auto promise = std::promise<void>{};
    auto future = promise.get_future();

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    std::thread pairSocketThread1(
        &Test_PairSocket::pairSocketThread, this, testMessage_, &promise);
    future.get();
    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(testMessage_);

        return out;
    }());

    ASSERT_TRUE(sent);

    pairSocketThread1.join();
}
}  // namespace ottest
