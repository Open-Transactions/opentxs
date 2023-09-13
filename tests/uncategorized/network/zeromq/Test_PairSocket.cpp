// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <ctime>
#include <future>
#include <span>
#include <thread>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Pair.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/fixtures/zeromq/PairSocket.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

TEST_F(PairSocket, PairSocket_Factory1)
{
    auto pairSocket = context_.Internal().PairSocket(
        ot::network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());
}

TEST_F(PairSocket, PairSocket_Factory2)
{
    auto peer = context_.Internal().PairSocket(
        ot::network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    auto pairSocket = context_.Internal().PairSocket(
        ot::network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());
    ASSERT_EQ(pairSocket->Endpoint(), peer->Endpoint());
}

TEST_F(PairSocket, PairSocket_Factory3)
{
    auto pairSocket = context_.Internal().PairSocket(
        ot::network::zeromq::ListenCallback::Factory(),
        TEST_ENDPOINT,
        "PairSocket");

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());
    ASSERT_EQ(pairSocket->Endpoint(), TEST_ENDPOINT);
}

TEST_F(PairSocket, PairSocket_Send1)
{
    bool callbackFinished = false;

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.get().size());
            const auto inputString =
                ot::UnallocatedCString{msg.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto peer = context_.Internal().PairSocket(listenCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    auto pairSocket = context_.Internal().PairSocket(
        ot::network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) { ot::Sleep(100ms); }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(PairSocket, PairSocket_Send2)
{
    bool callbackFinished = false;

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.get().size());
            const auto inputString =
                ot::UnallocatedCString{msg.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto peer = context_.Internal().PairSocket(listenCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    auto pairSocket = context_.Internal().PairSocket(
        ot::network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) { ot::Sleep(100ms); }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(PairSocket, PairSocket_Send3)
{
    bool callbackFinished = false;

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.get().size());
            const auto inputString =
                ot::UnallocatedCString{msg.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto peer = context_.Internal().PairSocket(listenCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    auto pairSocket = context_.Internal().PairSocket(
        ot::network::zeromq::ListenCallback::Factory(), peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) { ot::Sleep(100ms); }

    ASSERT_TRUE(callbackFinished);
}

TEST_F(PairSocket, PairSocket_Send_Two_Way)
{
    bool peerCallbackFinished = false;

    auto peerCallback = ot::network::zeromq::ListenCallback::Factory(
        [this,
         &peerCallbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.get().size());
            const auto inputString =
                ot::UnallocatedCString{msg.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message_, inputString);

            peerCallbackFinished = true;
        });

    ASSERT_NE(nullptr, &peerCallback.get());

    auto peer = context_.Internal().PairSocket(peerCallback);

    ASSERT_NE(nullptr, &peer.get());
    ASSERT_EQ(zmq::socket::Type::Pair, peer->Type());

    bool callbackFinished = false;

    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.get().size());
            const auto inputString =
                ot::UnallocatedCString{msg.Payload().begin()->Bytes()};

            EXPECT_EQ(test_message2_, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto pairSocket = context_.Internal().PairSocket(listenCallback, peer);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_TRUE(sent);

    sent = peer->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message2_);

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

TEST_F(PairSocket, PairSocket_Send_Separate_Thread)
{
    auto pairSocket = context_.Internal().PairSocket(
        ot::network::zeromq::ListenCallback::Factory());
    pair_socket_ = &pairSocket;
    auto promise = std::promise<void>{};
    auto future = promise.get_future();

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    std::thread pairSocketThread1(
        &PairSocket::pairSocketThread, this, test_message_, &promise);
    future.get();
    auto sent = pairSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(test_message_);

        return out;
    }());

    ASSERT_TRUE(sent);

    pairSocketThread1.join();
}
}  // namespace ottest
