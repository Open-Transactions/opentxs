// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <future>
#include <span>

#include "ottest/fixtures/zeromq/ZAP.hpp"

namespace ottest
{
using namespace std::literals;
using enum std::future_status;

TEST_F(ZAP, default_handler)
{
    const auto future = SendRequest(
        version_, request_, domain_, address_, identity_, mechanism_);

    ASSERT_EQ(future.wait_for(1min), ready);
    ASSERT_NO_THROW(future.get());

    const auto& reply = future.get();
    const auto payload = reply.Payload();

    ASSERT_GE(payload.size(), 5);

    EXPECT_EQ(payload[0].Bytes(), version_);
    EXPECT_EQ(payload[1].Bytes(), request_);
    EXPECT_EQ(payload[2].Bytes(), success_status_);
    EXPECT_EQ(payload[3].size(), 0);
    EXPECT_EQ(payload[4].size(), 0);
    EXPECT_EQ(payload[5].size(), 0);
}

TEST_F(ZAP, change_policy)
{
    using enum opentxs::api::network::ZAPPolicy;

    EXPECT_TRUE(ot_.ZAP().SetDefaultPolicy(Reject));

    const auto future = SendRequest(
        version_, request_, domain_, address_, identity_, mechanism_);

    ASSERT_EQ(future.wait_for(1min), ready);
    ASSERT_NO_THROW(future.get());

    const auto& reply = future.get();
    const auto payload = reply.Payload();

    ASSERT_GE(payload.size(), 5);

    EXPECT_EQ(payload[0].Bytes(), version_);
    EXPECT_EQ(payload[1].Bytes(), request_);
    EXPECT_EQ(payload[2].Bytes(), fail_status_);
    EXPECT_EQ(payload[3].size(), 0);
    EXPECT_EQ(payload[4].size(), 0);
    EXPECT_EQ(payload[5].size(), 0);
}

TEST_F(ZAP, custom_handler)
{
    auto successHandler = MakeHandler(domain_, alt_endpoint_1_, true);
    auto failHandler = MakeHandler(alt_domain_, alt_endpoint_2_, false);

    {
        const auto future = SendRequest(
            version_, request_, domain_, address_, identity_, mechanism_);

        ASSERT_EQ(future.wait_for(1min), ready);
        ASSERT_NO_THROW(future.get());

        const auto& reply = future.get();
        const auto payload = reply.Payload();

        ASSERT_GE(payload.size(), 5);

        EXPECT_EQ(payload[0].Bytes(), version_);
        EXPECT_EQ(payload[1].Bytes(), request_);
        EXPECT_EQ(payload[2].Bytes(), success_status_);
        EXPECT_EQ(payload[3].size(), 0);
        EXPECT_EQ(payload[4].size(), 0);
        EXPECT_EQ(payload[5].size(), 0);
    }
    {
        const auto future = SendRequest(
            version_, request_, alt_domain_, address_, identity_, mechanism_);

        ASSERT_EQ(future.wait_for(1min), ready);
        ASSERT_NO_THROW(future.get());

        const auto& reply = future.get();
        const auto payload = reply.Payload();

        ASSERT_GE(payload.size(), 5);

        EXPECT_EQ(payload[0].Bytes(), version_);
        EXPECT_EQ(payload[1].Bytes(), request_);
        EXPECT_EQ(payload[2].Bytes(), fail_status_);
        EXPECT_EQ(payload[3].size(), 0);
        EXPECT_EQ(payload[4].size(), 0);
        EXPECT_EQ(payload[5].size(), 0);
    }
    {
        const auto future = SendRequest(
            version_,
            request_,
            "different domain",
            address_,
            identity_,
            mechanism_);

        ASSERT_EQ(future.wait_for(1min), ready);
        ASSERT_NO_THROW(future.get());

        const auto& reply = future.get();
        const auto payload = reply.Payload();

        ASSERT_GE(payload.size(), 5);

        EXPECT_EQ(payload[0].Bytes(), version_);
        EXPECT_EQ(payload[1].Bytes(), request_);
        EXPECT_EQ(payload[2].Bytes(), fail_status_);
        EXPECT_EQ(payload[3].size(), 0);
        EXPECT_EQ(payload[4].size(), 0);
        EXPECT_EQ(payload[5].size(), 0);
    }
}
}  // namespace ottest
