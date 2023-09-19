// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/core/Ledger.hpp"

namespace ot = opentxs;

namespace ottest
{
ot::identifier::Nym nym_id_{};
ot::identifier::Notary server_id_{};

TEST_F(Ledger, init)
{
    nym_id_ = client_.Wallet().Nym(reason_c_, "Alice")->ID();

    ASSERT_FALSE(nym_id_.empty());

    const auto serverContract =
        server_.Wallet().Internal().Server(server_.ID());
    auto bytes = ot::Space{};
    serverContract->Serialize(ot::writer(bytes), true);
    client_.Wallet().Internal().Server(ot::reader(bytes));
    server_id_ = serverContract->ID();

    ASSERT_FALSE(server_id_.empty());
}

TEST_F(Ledger, create_nymbox)
{
    const auto nym = client_.Wallet().Nym(nym_id_);

    ASSERT_TRUE(nym);

    auto nymbox = client_.Factory().InternalSession().Ledger(
        nym_id_, nym_id_, server_id_, ot::ledgerType::nymbox, true);

    ASSERT_TRUE(nymbox);

    nymbox->ReleaseSignatures();

    EXPECT_TRUE(nymbox->SignContract(*nym, reason_c_));
    EXPECT_TRUE(nymbox->SaveContract());
    EXPECT_TRUE(nymbox->SaveNymbox());
}

TEST_F(Ledger, load_nymbox)
{
    auto nymbox = client_.Factory().InternalSession().Ledger(
        nym_id_, nym_id_, server_id_, ot::ledgerType::nymbox, false);

    ASSERT_TRUE(nymbox);
    EXPECT_TRUE(nymbox->LoadNymbox());
}
}  // namespace ottest
