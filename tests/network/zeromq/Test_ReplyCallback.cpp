// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <utility>

#include "opentxs/Version.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_ReplyCallback : public ::testing::Test
{
public:
    const ot::UnallocatedCString testMessage_{"zeromq test message"};
};

TEST(ReplyCallback, ReplyCallback_Factory)
{
    auto replyCallback = ot::network::zeromq::ReplyCallback::Factory(
        [](const ot::network::zeromq::Message&& input)
            -> ot::network::zeromq::Message {
            return ot::network::zeromq::reply_to_message(input);
        });

    ASSERT_NE(nullptr, &replyCallback.get());
}

TEST_F(Test_ReplyCallback, ReplyCallback_Process)
{
    auto replyCallback = ot::network::zeromq::ReplyCallback::Factory(
        [this](ot::network::zeromq::Message&& input)
            -> ot::network::zeromq::Message {
            const auto inputString =
                ot::UnallocatedCString{input.Body().begin()->Bytes()};
            EXPECT_EQ(testMessage_, inputString);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto testMessage = ot::network::zeromq::Message{};
    testMessage.AddFrame(testMessage_);

    ASSERT_NE(nullptr, &testMessage);

    auto message = replyCallback->Process(std::move(testMessage));

    const auto messageString =
        ot::UnallocatedCString{message.Body().begin()->Bytes()};
    ASSERT_EQ(testMessage_, messageString);
}
}  // namespace ottest
