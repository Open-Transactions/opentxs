// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <span>
#include <utility>

#include "internal/network/zeromq/ReplyCallback.hpp"
#include "ottest/fixtures/zeromq/ReplyCallback.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST_F(ReplyCallback, ReplyCallback_Factory)
{
    auto replyCallback = ot::network::zeromq::ReplyCallback::Factory(
        [](const ot::network::zeromq::Message&& input)
            -> ot::network::zeromq::Message {
            return ot::network::zeromq::reply_to_message(input);
        });

    ASSERT_NE(nullptr, &replyCallback.get());
}

TEST_F(ReplyCallback, ReplyCallback_Process)
{
    auto replyCallback = ot::network::zeromq::ReplyCallback::Factory(
        [this](ot::network::zeromq::Message&& input)
            -> ot::network::zeromq::Message {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            EXPECT_EQ(test_message_, inputString);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto testMessage = ot::network::zeromq::Message{};
    testMessage.AddFrame(test_message_);

    ASSERT_NE(nullptr, &testMessage);

    auto message = replyCallback->Process(std::move(testMessage));

    const auto messageString =
        ot::UnallocatedCString{message.Payload().begin()->Bytes()};
    ASSERT_EQ(test_message_, messageString);
}
}  // namespace ottest
