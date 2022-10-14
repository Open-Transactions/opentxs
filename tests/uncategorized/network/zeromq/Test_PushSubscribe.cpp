// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>
#include <future>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Pimpl.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class Test_PushSubscribe : public ::testing::Test
{
public:
    const zmq::Context& context_;
    const ot::UnallocatedCString test_message_;
    const ot::UnallocatedCString endpoint_1_;
    const ot::UnallocatedCString endpoint_2_;
    std::atomic<int> counter_1_;
    std::atomic<int> counter_2_;
    std::atomic<int> counter_3_;

    Test_PushSubscribe()
        : context_(ot::Context().ZMQ())
        , test_message_("zeromq test message")
        , endpoint_1_("inproc://opentxs/test/push_subscribe_test")
        , endpoint_2_("inproc://opentxs/test/publish_subscribe_test")
        , counter_1_(0)
        , counter_2_(0)
        , counter_3_(0)
    {
    }
};

TEST_F(Test_PushSubscribe, Push_Subscribe)
{
    auto promise = std::promise<bool>{};
    auto future = promise.get_future();
    auto callback =
        zmq::ListenCallback::Factory([&promise, this](auto&& input) {
            const auto body = input.Body();

            EXPECT_GT(body.size(), 0);

            if (0 < body.size()) {
                const auto compare = test_message_.compare(
                    ot::UnallocatedCString{body.at(0).Bytes()});

                EXPECT_EQ(compare, 0);

                promise.set_value(0 == compare);
            } else {
                promise.set_value(false);
            }
        });

    auto sender = context_.Internal().PushSocket(zmq::socket::Direction::Bind);
    auto receiver = context_.Internal().SubscribeSocket(callback);
    auto message = opentxs::network::zeromq::Message{};
    message.StartBody();
    message.AddFrame(test_message_);

    ASSERT_TRUE(sender->Start(endpoint_1_));
    ASSERT_TRUE(receiver->Start(endpoint_1_));
    ASSERT_TRUE(sender->Send(std::move(message)));

    const auto result = future.wait_for(10s);

    ASSERT_EQ(result, std::future_status::ready);
    EXPECT_TRUE(future.get());
}

TEST_F(Test_PushSubscribe, Push_Publish_Subscribe)
{
    const ot::UnallocatedCString message1{"1"};
    const ot::UnallocatedCString message2{"2"};
    auto promise1 = std::promise<void>{};
    auto promise2 = std::promise<void>{};
    auto promise3 = std::promise<void>{};
    auto promise4 = std::promise<void>{};
    auto future1 = promise1.get_future();
    auto future2 = promise2.get_future();
    auto future3 = promise3.get_future();
    auto future4 = promise4.get_future();
    auto callback1 = zmq::ListenCallback::Factory([&](auto&& input) {
        ++counter_1_;

        if (0 == message1.compare(
                     ot::UnallocatedCString{input.Body_at(0).Bytes()})) {
            promise1.set_value();
        } else {
            promise4.set_value();
        }
    });
    auto callback2 = zmq::ListenCallback::Factory([&](auto&& input) {
        ++counter_2_;

        if (0 == message1.compare(
                     ot::UnallocatedCString{input.Body_at(0).Bytes()})) {
            promise2.set_value();
        } else {
            promise4.set_value();
        }
    });
    auto callback3 = zmq::ListenCallback::Factory([&](auto&& input) {
        ++counter_3_;

        if (0 == message1.compare(
                     ot::UnallocatedCString{input.Body_at(0).Bytes()})) {
            promise3.set_value();
        } else {
            promise4.set_value();
        }
    });
    auto sender1 = context_.Internal().PublishSocket();
    auto sender2 = context_.Internal().PushSocket(zmq::socket::Direction::Bind);
    auto receiver1 = context_.Internal().SubscribeSocket(callback1);
    auto receiver2 = context_.Internal().SubscribeSocket(callback2);
    auto receiver3 = context_.Internal().SubscribeSocket(callback3);

    ASSERT_TRUE(sender1->Start(endpoint_2_));
    ASSERT_TRUE(sender2->Start(endpoint_1_));
    ASSERT_TRUE(receiver1->Start(endpoint_1_));
    ASSERT_TRUE(receiver1->Start(endpoint_2_));
    ASSERT_TRUE(receiver2->Start(endpoint_1_));
    ASSERT_TRUE(receiver2->Start(endpoint_2_));
    ASSERT_TRUE(receiver3->Start(endpoint_1_));
    ASSERT_TRUE(receiver3->Start(endpoint_2_));

    ASSERT_EQ(counter_1_.load(), 0);
    ASSERT_EQ(counter_2_.load(), 0);
    ASSERT_EQ(counter_3_.load(), 0);

    // All receivers should get the message
    ASSERT_TRUE(sender1->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(message1);

        return out;
    }()));

    const auto result1 = future1.wait_for(10s);
    const auto result2 = future2.wait_for(2s);
    const auto result3 = future3.wait_for(2s);

    ASSERT_EQ(result1, std::future_status::ready);
    ASSERT_EQ(result2, std::future_status::ready);
    ASSERT_EQ(result3, std::future_status::ready);
    ASSERT_EQ(1, counter_1_.load());
    ASSERT_EQ(1, counter_2_.load());
    ASSERT_EQ(1, counter_3_.load());

    // One receivers should get the message
    ASSERT_TRUE(sender2->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(message2);

        return out;
    }()));

    const auto result4 = future4.wait_for(10s);

    ASSERT_EQ(result4, std::future_status::ready);
    ASSERT_EQ(4, counter_1_.load() + counter_2_.load() + counter_3_.load());
}
}  // namespace ottest
