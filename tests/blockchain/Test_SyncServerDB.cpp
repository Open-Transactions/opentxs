// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string_view>

#include "Helpers.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"

namespace ot = opentxs;

namespace ottest
{
class SyncServerDB : public ::testing::Test
{
protected:
    using Endpoints = ot::UnallocatedVector<ot::UnallocatedCString>;

    static constexpr auto first_server_{"tcp://example.com:1"};
    static constexpr auto second_server_{"tcp://example.com:2"};
    static constexpr auto other_server_{"tcp://example.com:3"};
    static std::unique_ptr<Listener> listener_p_;

    const ot::api::session::Client& api_;
    Listener& listener_;

    static auto count(
        const Endpoints& endpoints,
        const ot::UnallocatedCString& value) noexcept -> std::size_t
    {
        return std::count(endpoints.begin(), endpoints.end(), value);
    }

    auto cleanup() noexcept { listener_p_.reset(); }

    SyncServerDB()
        : api_(ot::Context().StartClientSession(0))
        , listener_([&]() -> auto& {
            if (!listener_p_) {
                listener_p_ = std::make_unique<Listener>(
                    api_,
                    api_.Endpoints().BlockchainSyncServerUpdated().data());
            }

            return *listener_p_;
        }())
    {
    }
};

std::unique_ptr<Listener> SyncServerDB::listener_p_{};

static constexpr auto default_server_count_ = std::size_t{2u};

TEST_F(SyncServerDB, init_library) {}

TEST_F(SyncServerDB, empty_db)
{
    const auto endpoints = api_.Network().Blockchain().GetSyncServers();

    EXPECT_EQ(endpoints.size(), default_server_count_);
}

TEST_F(SyncServerDB, import_first_server)
{
    EXPECT_TRUE(api_.Network().Blockchain().AddSyncServer(first_server_));

    const auto endpoints = api_.Network().Blockchain().GetSyncServers();

    EXPECT_EQ(endpoints.size(), default_server_count_ + 1u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 0);
    EXPECT_EQ(count(endpoints, other_server_), 0);

    const auto& message = listener_.get(0);
    const auto body = message.Body();

    ASSERT_EQ(body.size(), 3);
    EXPECT_EQ(body.at(0).as<ot::WorkType>(), ot::WorkType::SyncServerUpdated);
    EXPECT_STREQ(body.at(1).Bytes().data(), first_server_);
    EXPECT_TRUE(body.at(2).as<bool>());
}

TEST_F(SyncServerDB, import_second_server)
{
    EXPECT_TRUE(api_.Network().Blockchain().AddSyncServer(second_server_));

    const auto endpoints = api_.Network().Blockchain().GetSyncServers();

    EXPECT_EQ(endpoints.size(), default_server_count_ + 2u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);

    const auto& message = listener_.get(1);
    const auto body = message.Body();

    ASSERT_EQ(body.size(), 3);
    EXPECT_EQ(body.at(0).as<ot::WorkType>(), ot::WorkType::SyncServerUpdated);
    EXPECT_STREQ(body.at(1).Bytes().data(), second_server_);
    EXPECT_TRUE(body.at(2).as<bool>());
}

TEST_F(SyncServerDB, import_existing_server)
{
    EXPECT_TRUE(api_.Network().Blockchain().AddSyncServer(second_server_));

    const auto endpoints = api_.Network().Blockchain().GetSyncServers();

    EXPECT_EQ(endpoints.size(), default_server_count_ + 2u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);
}

TEST_F(SyncServerDB, import_empty_string)
{
    EXPECT_FALSE(api_.Network().Blockchain().AddSyncServer(""));

    const auto endpoints = api_.Network().Blockchain().GetSyncServers();

    EXPECT_EQ(endpoints.size(), default_server_count_ + 2u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);
}

TEST_F(SyncServerDB, delete_non_existing)
{
    EXPECT_TRUE(api_.Network().Blockchain().DeleteSyncServer(other_server_));

    const auto endpoints = api_.Network().Blockchain().GetSyncServers();

    EXPECT_EQ(endpoints.size(), default_server_count_ + 2u);
    EXPECT_EQ(count(endpoints, first_server_), 1);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);
}

TEST_F(SyncServerDB, delete_existing)
{
    EXPECT_TRUE(api_.Network().Blockchain().DeleteSyncServer(first_server_));

    const auto endpoints = api_.Network().Blockchain().GetSyncServers();

    EXPECT_EQ(endpoints.size(), default_server_count_ + 1u);
    EXPECT_EQ(count(endpoints, first_server_), 0);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);

    const auto& message = listener_.get(2);
    const auto body = message.Body();

    ASSERT_EQ(body.size(), 3);
    EXPECT_EQ(body.at(0).as<ot::WorkType>(), ot::WorkType::SyncServerUpdated);
    EXPECT_STREQ(body.at(1).Bytes().data(), first_server_);
    EXPECT_FALSE(body.at(2).as<bool>());
}

TEST_F(SyncServerDB, delete_empty_string)
{
    EXPECT_FALSE(api_.Network().Blockchain().DeleteSyncServer(""));

    const auto endpoints = api_.Network().Blockchain().GetSyncServers();

    EXPECT_EQ(endpoints.size(), default_server_count_ + 1u);
    EXPECT_EQ(count(endpoints, first_server_), 0);
    EXPECT_EQ(count(endpoints, second_server_), 1);
    EXPECT_EQ(count(endpoints, other_server_), 0);
}

TEST_F(SyncServerDB, cleanup) { cleanup(); }
}  // namespace ottest
