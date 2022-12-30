// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <span>
#include <string_view>

namespace ot = opentxs;

namespace ottest
{
using namespace std::literals;

TEST(Message, Factory)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    ASSERT_NE(nullptr, &multipartMessage);
}

TEST(Message, AddFrame)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    auto& message = multipartMessage.AddFrame();
    ASSERT_EQ(1, multipartMessage.get().size());
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(message.size(), 0);
}

TEST(Message, AddFrame_Data)
{
    auto multipartMessage = ot::network::zeromq::Message{};
    static constexpr auto string = "testString"sv;
    auto& message = multipartMessage.AddFrame(string.data(), string.size());
    ASSERT_EQ(multipartMessage.get().size(), 1);
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(message.size(), string.size());

    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(Message, AddFrame_string)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    auto& message = multipartMessage.AddFrame("testString");
    ASSERT_EQ(multipartMessage.get().size(), 1);
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

    auto& message = multipartMessage.get()[0];
    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg1", messageString.c_str());

    ot::network::zeromq::Frame& message2 = multipartMessage.get()[1];
    messageString = message2.Bytes();
    ASSERT_STREQ("msg2", messageString.c_str());

    ot::network::zeromq::Frame& message3 = multipartMessage.get()[2];
    messageString = message3.Bytes();
    ASSERT_STREQ("msg3", messageString.c_str());
}

TEST(Message, at_const)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame("msg3");

    const auto& message = multipartMessage.get()[0];
    auto messageString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg1", messageString.c_str());

    const ot::network::zeromq::Frame& message2 = multipartMessage.get()[1];
    messageString = message2.Bytes();
    ASSERT_STREQ("msg2", messageString.c_str());

    const ot::network::zeromq::Frame& message3 = multipartMessage.get()[2];
    messageString = message3.Bytes();
    ASSERT_STREQ("msg3", messageString.c_str());
}

TEST(Message, Body)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame();
    multipartMessage.AddFrame("msg3");
    multipartMessage.AddFrame("msg4");

    const auto bodySection = multipartMessage.Payload();
    ASSERT_EQ(bodySection.size(), 2);

    const auto& message = bodySection[1];
    auto msgString = ot::UnallocatedCString{message.Bytes()};
    ASSERT_STREQ("msg4", msgString.c_str());
}

TEST(Message, size)
{
    auto multipartMessage = ot::network::zeromq::Message{};

    std::size_t size = multipartMessage.get().size();
    ASSERT_EQ(size, 0);

    multipartMessage.AddFrame("msg1");
    multipartMessage.AddFrame("msg2");
    multipartMessage.AddFrame("msg3");

    size = multipartMessage.get().size();
    ASSERT_EQ(size, 3);
}
}  // namespace ottest
