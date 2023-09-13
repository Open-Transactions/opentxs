// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/PublishSubscribe.hpp"  // IWYU pragma: associated

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
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

PublishSubscribe::PublishSubscribe()
    : context_(OTTestEnvironment::GetOT().ZMQ())
{
}

void PublishSubscribe::subscribeSocketThread(
    const ot::UnallocatedSet<ot::UnallocatedCString>& endpoints,
    const ot::UnallocatedSet<ot::UnallocatedCString>& msgs)
{
    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [this, msgs](ot::network::zeromq::Message&& input) -> void {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
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

void PublishSubscribe::publishSocketThread(
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
}  // namespace ottest
