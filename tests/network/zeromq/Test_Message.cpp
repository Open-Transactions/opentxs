// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstddef>
#include <iterator>

#include "opentxs/Version.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST(Message, Factory)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    ASSERT_NE(nullptr, &multipartMessage);
}

TEST(Message, AddFrame)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    auto& message = multipartMessage.AddFrame();
    ASSERT_EQ(1, multipartMessage.size());
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(message.size(), 0);
}

TEST(Message, AddFrame_Data)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    auto& message =
        multipartMessage.AddFrame(ot::Data::Factory("testString", 10));
    ASSERT_EQ(multipartMessage.size(), 1);
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(message.size(), 10);

    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(Message, AddFrame_string)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    auto& message = multipartMessage.AddFrame("testString");
    ASSERT_EQ(multipartMessage.size(), 1);
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(message.size(), 10);

    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(Message, at)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame("msg3");

    auto& message = multipartMessage.at(0);
    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg1", messageString.c_str());

    ot::network::zeromq::Frame& message2 = multipartMessage.at(1);
    messageString = message2.Bytes();
    ASSERT_STREQ("msg2", messageString.c_str());

    ot::network::zeromq::Frame& message3 = multipartMessage.at(2);
    messageString = message3.Bytes();
    ASSERT_STREQ("msg3", messageString.c_str());
}

TEST(Message, at_const)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame("msg3");

    const auto& message = multipartMessage.at(0);
    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg1", messageString.c_str());

    const ot::network::zeromq::Frame& message2 = multipartMessage.at(1);
    messageString = message2.Bytes();
    ASSERT_STREQ("msg2", messageString.c_str());

    const ot::network::zeromq::Frame& message3 = multipartMessage.at(2);
    messageString = message3.Bytes();
    ASSERT_STREQ("msg3", messageString.c_str());
}

TEST(Message, begin)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    auto it = multipartMessage.begin();
    ASSERT_EQ(multipartMessage.end(), it);
    ASSERT_EQ(std::distance(it, multipartMessage.end()), 0);

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame("msg3");

    ASSERT_NE(multipartMessage.end(), it);
    ASSERT_EQ(std::distance(it, multipartMessage.end()), 3);

    std::advance(it, 3);
    ASSERT_EQ(multipartMessage.end(), it);
    ASSERT_EQ(std::distance(it, multipartMessage.end()), 0);
}

TEST(Message, Body)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame();
    multipartMessage.AddFrame("msg3");
    multipartMessage.AddFrame("msg4");

    const ot::network::zeromq::FrameSection bodySection =
        multipartMessage.Body();
    ASSERT_EQ(bodySection.size(), 2);

    const auto& message = bodySection.at(1);
    auto msgString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg4", msgString.c_str());
}

TEST(Message, Body_at)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame();
    multipartMessage.AddFrame("msg3");
    multipartMessage.AddFrame("msg4");

    const auto& message = multipartMessage.Body_at(1);
    auto msgString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg4", msgString.c_str());
}

TEST(Message, Body_begin)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame();
    multipartMessage.AddFrame("msg3");
    multipartMessage.AddFrame("msg4");

    auto bodyBegin = multipartMessage.Body_begin();
    auto body = multipartMessage.Body();
    ASSERT_EQ(body.begin(), bodyBegin);
}

TEST(Message, Body_end)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame();
    multipartMessage.AddFrame("msg3");
    multipartMessage.AddFrame("msg4");

    auto bodyEnd = multipartMessage.Body_end();
    auto body = multipartMessage.Body();
    ASSERT_EQ(body.end(), bodyEnd);
}

TEST(Message, end)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    auto it = multipartMessage.end();
    ASSERT_EQ(multipartMessage.begin(), it);
    ASSERT_EQ(

        std::distance(multipartMessage.begin(), it), 0);

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame("msg3");

    auto it2 = multipartMessage.end();
    ASSERT_NE(multipartMessage.begin(), it2);
    ASSERT_EQ(

        std::distance(multipartMessage.begin(), it2), 3);
}

TEST(Message, EnsureDelimiter)
{
    // Empty message.
    auto message = ot::network::zeromq::Message{};

    ASSERT_EQ(message.size(), 0);

    message.EnsureDelimiter();  // Adds delimiter.

    ASSERT_EQ(message.size(), 1);

    message.AddFrame();

    ASSERT_EQ(message.size(), 2);

    message.EnsureDelimiter();  // Doesn't add delimiter

    ASSERT_EQ(message.size(), 2);

    // Message body only.
    auto message2 = opentxs::network::zeromq::Message{};
    message2.AddFrame("msg");

    ASSERT_EQ(static_cast<std::size_t>(1), message2.size());

    message2.EnsureDelimiter();  // Inserts delimiter.

    ASSERT_EQ(message2.size(), 2);
    ASSERT_EQ(message2.Header().size(), 0);
    ASSERT_EQ(message2.Body().size(), 1);

    message2.EnsureDelimiter();  // Doesn't add delimiter.

    ASSERT_EQ(message2.size(), 2);
    ASSERT_EQ(message2.Header().size(), 0);
    ASSERT_EQ(message2.Body().size(), 1);

    // Header and message body.
    auto message3 = opentxs::network::zeromq::Message{};
    message3.AddFrame("header");
    message3.AddFrame();
    message3.AddFrame("body");

    ASSERT_EQ(message3.size(), 3);
    ASSERT_EQ(message3.Header().size(), 1);
    ASSERT_EQ(message3.Body().size(), 1);

    message3.EnsureDelimiter();  // Doesn't add delimiter.

    ASSERT_EQ(message3.size(), 3);
    ASSERT_EQ(message3.Header().size(), 1);
    ASSERT_EQ(message3.Body().size(), 1);

    // Message body with 2 frames.
    auto message4 = opentxs::network::zeromq::Message{};
    message4.AddFrame("frame1");
    message4.AddFrame("frame2");

    ASSERT_EQ(message4.size(), 2);
    ASSERT_EQ(message4.Header().size(), 0);
    ASSERT_EQ(message4.Body().size(), 2);

    message4.EnsureDelimiter();

    ASSERT_EQ(message4.size(), 3);
    ASSERT_EQ(message4.Header().size(), 1);
    ASSERT_EQ(message4.Body().size(), 1);
}

TEST(Message, Header)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame();
    multipartMessage.AddFrame("msg3");
    multipartMessage.AddFrame("msg4");

    ot::network::zeromq::FrameSection headerSection = multipartMessage.Header();
    ASSERT_EQ(headerSection.size(), 2);

    const auto& message = headerSection.at(1);
    auto msgString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg2", msgString.c_str());
}

TEST(Message, Header_at)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame();
    multipartMessage.AddFrame("msg3");
    multipartMessage.AddFrame("msg4");

    const auto& message = multipartMessage.Header_at(1);
    auto msgString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg2", msgString.c_str());
}

TEST(Message, Header_begin)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame();
    multipartMessage.AddFrame("msg3");
    multipartMessage.AddFrame("msg4");

    auto headerBegin = multipartMessage.Header_begin();
    auto header = multipartMessage.Header();
    ASSERT_EQ(header.begin(), headerBegin);
}

TEST(Message, Header_end)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame();
    multipartMessage.AddFrame("msg3");
    multipartMessage.AddFrame("msg4");

    auto headerEnd = multipartMessage.Header_end();
    auto header = multipartMessage.Header();
    ASSERT_EQ(header.end(), headerEnd);
}

TEST(Message, size)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    std::size_t size = multipartMessage.size();
    ASSERT_EQ(size, 0);

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame("msg3");

    size = multipartMessage.size();
    ASSERT_EQ(size, 3);
}
}  // namespace ottest
