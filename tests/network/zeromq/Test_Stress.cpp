// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <atomic>
#include <cstddef>
#include <memory>

#include "opentxs/OT.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
TEST(Test_Stress, Pub_10000)
{
    const auto& ot = ot::Context();
    auto endpoints = ot::UnallocatedVector<ot::UnallocatedCString>{};
    auto pub = ot::UnallocatedVector<ot::OTZMQPublishSocket>{};

    for (auto i{0}; i < 10000; ++i) {
        auto& socket = pub.emplace_back(ot.ZMQ().PublishSocket()).get();
        auto& endpoint = endpoints.emplace_back(
            ot::UnallocatedCString{"inproc://Pub_10000/"} + std::to_string(i));

        EXPECT_TRUE(socket.Start(endpoint));
    }
}

TEST(Test_Stress, PubSub_100)
{
    const auto& ot = ot::Context();
    auto endpoints = ot::UnallocatedVector<ot::UnallocatedCString>{};
    auto pub = ot::UnallocatedVector<ot::OTZMQPublishSocket>{};

    for (auto i{0}; i < 100; ++i) {
        auto& socket = pub.emplace_back(ot.ZMQ().PublishSocket()).get();
        auto& endpoint = endpoints.emplace_back(
            ot::UnallocatedCString{"inproc://PubSub_100/"} + std::to_string(i));

        EXPECT_TRUE(socket.Start(endpoint));
    }

    auto results = std::atomic<std::size_t>{};
    auto callback =
        zmq::ListenCallback::Factory([&results](auto&&) { ++results; });
    auto sub = ot.ZMQ().SubscribeSocket(callback);

    for (const auto& endpoint : endpoints) {
        EXPECT_TRUE(sub->Start(endpoint));
    }

    for (const auto& socket : pub) {
        EXPECT_TRUE(socket->Send(opentxs::network::zeromq::Message{}));
    }

    while (pub.size() > results.load()) { ; }
}
}  // namespace ottest
