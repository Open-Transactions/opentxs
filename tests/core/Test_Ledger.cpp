// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace ot = opentxs;

namespace ottest
{
ot::OTNymID nym_id_{ot::identifier::Nym::Factory()};
ot::OTNotaryID server_id_{ot::identifier::Notary::Factory()};

struct Ledger : public ::testing::Test {
    const ot::api::session::Client& client_;
    const ot::api::session::Notary& server_;
    ot::OTPasswordPrompt reason_c_;
    ot::OTPasswordPrompt reason_s_;

    Ledger()
        : client_(ot::Context().StartClientSession(0))
        , server_(ot::Context().StartNotarySession(0))
        , reason_c_(client_.Factory().PasswordPrompt(__func__))
        , reason_s_(server_.Factory().PasswordPrompt(__func__))
    {
    }
};

TEST_F(Ledger, init)
{
    nym_id_ = client_.Wallet().Nym(reason_c_, "Alice")->ID();

    ASSERT_FALSE(nym_id_->empty());

    const auto serverContract = server_.Wallet().Server(server_.ID());
    auto bytes = ot::Space{};
    serverContract->Serialize(ot::writer(bytes), true);
    client_.Wallet().Server(ot::reader(bytes));
    server_id_->SetString(serverContract->ID()->str());

    ASSERT_FALSE(server_id_->empty());
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
