// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/Types.hpp"

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <span>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "ottest/fixtures/blockchain/Basic.hpp"
#include "ottest/fixtures/blockchain/SyncServerDB.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

static constexpr auto default_server_count_ = 0_uz;

TEST_F(SyncServerDB, init_library) {}

TEST_F(SyncServerDB, empty_db)
{
    const auto endpoints = api_.Network().OTDHT().KnownPeers({});

    EXPECT_EQ(endpoints.size(), default_server_count_);
}

TEST_F(SyncServerDB, import_first_server)
{
    EXPECT_TRUE(api_.Network().OTDHT().AddPeer(first_server_));

    const auto endpoints = api_.Network().OTDHT().KnownPeers({});

    EXPECT_EQ(endpoints.size(), default_server_count_ + 1u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 0);
    EXPECT_EQ(count(endpoints, other_server_), 0);

    const auto& message = listener_.get(0);
    const auto body = message.Payload();

    ASSERT_EQ(body.size(), 3);
    EXPECT_EQ(body[0].as<ot::WorkType>(), ot::WorkType::SyncServerUpdated);
    EXPECT_STREQ(body[1].Bytes().data(), first_server_);
    EXPECT_TRUE(body[2].as<bool>());
}

TEST_F(SyncServerDB, import_second_server)
{
    EXPECT_TRUE(api_.Network().OTDHT().AddPeer(second_server_));

    const auto endpoints = api_.Network().OTDHT().KnownPeers({});

    EXPECT_EQ(endpoints.size(), default_server_count_ + 2u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);

    const auto& message = listener_.get(1);
    const auto body = message.Payload();

    ASSERT_EQ(body.size(), 3);
    EXPECT_EQ(body[0].as<ot::WorkType>(), ot::WorkType::SyncServerUpdated);
    EXPECT_STREQ(body[1].Bytes().data(), second_server_);
    EXPECT_TRUE(body[2].as<bool>());
}

TEST_F(SyncServerDB, import_existing_server)
{
    EXPECT_TRUE(api_.Network().OTDHT().AddPeer(second_server_));

    const auto endpoints = api_.Network().OTDHT().KnownPeers({});

    EXPECT_EQ(endpoints.size(), default_server_count_ + 2u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);
}

TEST_F(SyncServerDB, import_empty_string)
{
    EXPECT_FALSE(api_.Network().OTDHT().AddPeer(""));

    const auto endpoints = api_.Network().OTDHT().KnownPeers({});

    EXPECT_EQ(endpoints.size(), default_server_count_ + 2u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);
}

TEST_F(SyncServerDB, delete_non_existing)
{
    EXPECT_TRUE(api_.Network().OTDHT().DeletePeer(other_server_));

    const auto endpoints = api_.Network().OTDHT().KnownPeers({});

    EXPECT_EQ(endpoints.size(), default_server_count_ + 2u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);
}

TEST_F(SyncServerDB, delete_existing)
{
    EXPECT_TRUE(api_.Network().OTDHT().DeletePeer(first_server_));

    const auto endpoints = api_.Network().OTDHT().KnownPeers({});

    EXPECT_EQ(endpoints.size(), default_server_count_ + 1u);
    EXPECT_EQ(count(endpoints, first_server_), 0);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);

    const auto& message = listener_.get(2);
    const auto body = message.Payload();

    ASSERT_EQ(body.size(), 3);
    EXPECT_EQ(body[0].as<ot::WorkType>(), ot::WorkType::SyncServerUpdated);
    EXPECT_STREQ(body[1].Bytes().data(), first_server_);
    EXPECT_FALSE(body[2].as<bool>());
}

TEST_F(SyncServerDB, delete_empty_string)
{
    EXPECT_FALSE(api_.Network().OTDHT().DeletePeer(""));

    const auto endpoints = api_.Network().OTDHT().KnownPeers({});

    EXPECT_EQ(endpoints.size(), default_server_count_ + 1u);
    EXPECT_EQ(count(endpoints, first_server_), 0);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);
}

TEST_F(SyncServerDB, cleanup) { cleanup(); }
}  // namespace ottest
