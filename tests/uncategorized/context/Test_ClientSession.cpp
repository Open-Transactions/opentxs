// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <future>
#include <memory>

#include "internal/api/session/Wallet.hpp"
#include "ottest/Basic.hpp"

namespace ottest
{
ot::UnallocatedCString nym_id_{};
ot::UnallocatedCString server_id_{};

TEST(ClientSession, create)
{
    const auto& otx = ot::InitContext(Args(true));
    const auto& client = otx.StartClientSession(0);
    const auto reason = client.Factory().PasswordPrompt(__func__);
    const auto nym = client.Wallet().Nym(reason);

    ASSERT_TRUE(nym);

    nym_id_ = nym->ID().asBase58(otx.Crypto());

    EXPECT_FALSE(nym_id_.empty());

    ot::Cleanup();
}

TEST(ClientSession, restart)
{
    const auto& otx = ot::InitContext(Args(true));
    const auto& client = otx.StartClientSession(0);
    const auto reason = client.Factory().PasswordPrompt(__func__);
    const auto nym =
        client.Wallet().Nym(client.Factory().NymIDFromBase58(nym_id_));

    ASSERT_TRUE(nym);
    EXPECT_EQ(nym_id_, nym->ID().asBase58(otx.Crypto()));

    ot::Cleanup();
}

TEST(ClientSession, introduction_server)
{
    const auto& otx = ot::InitContext(Args(true));
    const auto& server = otx.StartNotarySession(0);
    const auto& client = otx.StartClientSession(0);
    const auto reasonS = server.Factory().PasswordPrompt(__func__);
    const auto reasonC = client.Factory().PasswordPrompt(__func__);
    const auto& serverID = server.ID();
    const auto nymID = client.Factory().NymIDFromBase58(nym_id_);
    server_id_ = serverID.asBase58(otx.Crypto());

    EXPECT_FALSE(server_id_.empty());

    {
        const auto contract = server.Wallet().Internal().Server(serverID);
        const auto id = client.OTX().SetIntroductionServer(contract);

        EXPECT_EQ(serverID, id);
    }

    {
        auto [id, future] =
            client.OTX().RegisterNymPublic(nymID, serverID, true);

        ASSERT_NE(0, id);
        ASSERT_EQ(
            std::future_status::ready,
            future.wait_for(std::chrono::minutes(1)));

        const auto [status, reply] = future.get();

        EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, status);
        EXPECT_TRUE(reply);
    }

    client.OTX().ContextIdle(nymID, serverID).get();
    ot::Cleanup();
}

// TODO restore this test after OTX refactor
// TEST(ClientSession, restart_after_registering)
// {
//     const auto& otx = ot::InitContext(Args(true));
//     const auto& server = otx.StartNotarySession(0);
//     const auto& client = otx.StartClientSession(0);
//     const auto& serverID = server.ID();
//     const auto nymID = client.Factory().NymID(nym_id_);
//     client.OTX().DownloadNymbox(nymID, serverID);
//     client.OTX().ContextIdle(nymID, serverID).get();
//     ot::Cleanup();
// }
}  // namespace ottest
