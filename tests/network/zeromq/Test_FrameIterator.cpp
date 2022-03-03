// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include "opentxs/Version.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST(FrameIterator, constructors)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    auto frameIterator{multipartMessage.begin()};
    ASSERT_EQ(multipartMessage.begin(), frameIterator);

    multipartMessage.AddFrame(ot::UnallocatedCString{"msg1"});
    multipartMessage.AddFrame(ot::UnallocatedCString{"msg2"});

    auto frameIterator3(++frameIterator);
    auto& message = *frameIterator3;
    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg2", messageString.c_str());
}

TEST(FrameIterator, assignment_operator)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    auto frameIterator{multipartMessage.begin()};
    ASSERT_EQ(multipartMessage.begin(), frameIterator);

    multipartMessage.AddFrame(ot::UnallocatedCString{"msg1"});
    multipartMessage.AddFrame(ot::UnallocatedCString{"msg2"});

    auto frameIterator2{++frameIterator};
    auto& frameIterator3 = frameIterator2;
    auto& message = *frameIterator3;
    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg2", messageString.c_str());
}

TEST(FrameIterator, operator_asterisk)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame(ot::UnallocatedCString{"msg1"});

    auto frameIterator{multipartMessage.begin()};
    auto& message = *frameIterator;
    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg1", messageString.c_str());
}

TEST(FrameIterator, operator_asterisk_const)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame(ot::UnallocatedCString{"msg1"});

    const auto frameIterator = multipartMessage.begin();
    auto& message = *frameIterator;
    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg1", messageString.c_str());
}

TEST(FrameIterator, operator_equal)
{
    auto message = ot::network::zeromq::Message{};

    EXPECT_TRUE(message.begin() == message.end());

    message.AddFrame(ot::UnallocatedCString{"msg1"});

    EXPECT_FALSE(message.begin() == message.end());

    auto frameIterator2{message.begin()};
    auto& frameIterator3 = frameIterator2;

    EXPECT_TRUE(frameIterator2 == frameIterator3);
    EXPECT_FALSE(message.end() == frameIterator2);
}

TEST(FrameIterator, operator_notEqual)
{
    auto message = ot::network::zeromq::Message{};

    EXPECT_FALSE(message.begin() != message.end());

    message.AddFrame(ot::UnallocatedCString{"msg1"});

    EXPECT_TRUE(message.begin() != message.end());

    auto frameIterator2{message.begin()};
    auto& frameIterator3 = frameIterator2;

    EXPECT_FALSE(frameIterator2 != frameIterator3);
    EXPECT_TRUE(message.begin() == frameIterator2);
}

TEST(FrameIterator, operator_pre_increment)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame(ot::UnallocatedCString{"msg1"});
    multipartMessage.AddFrame(ot::UnallocatedCString{"msg2"});

    auto frameIterator{multipartMessage.begin()};
    auto& frameIterator2 = ++frameIterator;

    auto& message = *frameIterator2;
    auto stringMessage = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg2", stringMessage.c_str());

    auto& message2 = *frameIterator;
    stringMessage = message2.Bytes();
    ASSERT_STREQ("msg2", stringMessage.c_str());
}

TEST(FrameIterator, operator_post_increment)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame(ot::UnallocatedCString{"msg1"});
    multipartMessage.AddFrame(ot::UnallocatedCString{"msg2"});

    auto frameIterator{multipartMessage.begin()};
    auto frameIterator2 = frameIterator++;

    auto& message = *frameIterator2;
    auto stringMessage = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg1", stringMessage.c_str());

    auto& message2 = *frameIterator;
    stringMessage = message2.Bytes();
    ASSERT_STREQ("msg2", stringMessage.c_str());
}
}  // namespace ottest
