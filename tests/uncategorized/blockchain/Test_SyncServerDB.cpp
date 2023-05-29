// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/blockchain/Basic.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;

class SyncServerDB : public ::testing::Test
{
protected:
    using Endpoints = ot::Vector<ot::CString>;

    static constexpr auto first_server_{"tcp://example.com:1"};
    static constexpr auto second_server_{"tcp://example.com:2"};
    static constexpr auto other_server_{"tcp://example.com:3"};
    static std::unique_ptr<Listener> listener_p_;

    const ot::api::session::Client& api_;
    Listener& listener_;

    static auto count(
        const Endpoints& endpoints,
        std::string_view value) noexcept -> std::size_t
    {
        return std::count(endpoints.begin(), endpoints.end(), value);
    }

    auto cleanup() noexcept { listener_p_.reset(); }

    SyncServerDB()
        : api_(OTTestEnvironment::GetOT().StartClientSession(0))
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
