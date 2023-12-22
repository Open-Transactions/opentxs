// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <zmq.h>
#include <string_view>

#include "ottest/fixtures/zeromq/Frame.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace std::literals;

TEST_F(Frame, Factory1)
{
    auto& frame = message_.AddFrame();

    EXPECT_NE(nullptr, frame.operator zmq_msg_t*());
}

TEST_F(Frame, Factory2)
{
    const auto data = ot::ByteArray{"0"sv};
    auto& frame = message_.AddFrame(data.data(), data.size());

    EXPECT_NE(nullptr, frame.operator zmq_msg_t*());
    EXPECT_EQ(data.Bytes(), frame.Bytes());
}

TEST_F(Frame, operator_string)
{
    auto& frame = message_.AddFrame(test_string_);
    const auto text = ot::UnallocatedCString{frame.Bytes()};

    EXPECT_EQ(text, test_string_);
}

TEST_F(Frame, data)
{
    auto& frame = message_.AddFrame();
    const auto* data = frame.data();

    EXPECT_NE(data, nullptr);
    EXPECT_EQ(data, ::zmq_msg_data(frame));
}

TEST_F(Frame, size)
{
    {
        auto& frame = message_.AddFrame();
        auto size = frame.size();

        EXPECT_EQ(size, 0);
        EXPECT_EQ(size, ::zmq_msg_size(frame));
    }
    {
        auto& frame = message_.AddFrame(test_string_);
        auto size = frame.size();

        EXPECT_EQ(size, 10);
        EXPECT_EQ(size, ::zmq_msg_size(frame));
    }
}

TEST_F(Frame, zmq_msg_t)
{
    auto& frame = message_.AddFrame();
    auto* ptr = frame.operator zmq_msg_t*();

    EXPECT_NE(ptr, nullptr);
}
}  // namespace ottest
