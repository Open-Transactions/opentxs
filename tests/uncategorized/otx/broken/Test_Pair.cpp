// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Notary

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <span>
#include <utility>

#include "internal/api/session/Client.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/UI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/PairEvent.hpp"
#include "internal/core/contract/peer/PairEventType.hpp"  // IWYU pragma: keep
#include "internal/interface/ui/AccountSummary.hpp"
#include "internal/interface/ui/AccountSummaryItem.hpp"
#include "internal/interface/ui/IssuerItem.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/otx/client/Issuer.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"

#include "ottest/fixtures/otx/broken/Pair.hpp"

#define UNIT_DEFINITION_CONTRACT_VERSION 2
#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_TLA "USD"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT ot::UnitType::Usd

namespace ottest
{
Counter account_summary_{};

TEST_F(Pair, init_ot) {}

TEST_F(Pair, init_ui)
{
    account_summary_.expected_ = 0;
    api_chris_.UI().Internal().AccountSummary(
        chris_.nym_id_,
        ot::UnitType::Usd,
        make_cb(account_summary_, "account summary USD"));
}

TEST_F(Pair, initial_state)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));

    const auto& widget = chris_.api_->UI().Internal().AccountSummary(
        chris_.nym_id_, ot::UnitType::Usd);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Pair, issue_dollars)
{
    const auto contract = api_issuer_.Wallet().Internal().CurrencyContract(
        issuer_.nym_id_.asBase58(api_issuer_.Crypto()),
        UNIT_DEFINITION_CONTRACT_NAME,
        UNIT_DEFINITION_TERMS,
        UNIT_DEFINITION_UNIT_OF_ACCOUNT,
        1,
        issuer_.Reason());

    EXPECT_EQ(UNIT_DEFINITION_CONTRACT_VERSION, contract->Version());
    EXPECT_EQ(ot::contract::UnitType::Currency, contract->Type());
    EXPECT_EQ(UNIT_DEFINITION_UNIT_OF_ACCOUNT, contract->UnitOfAccount());
    EXPECT_TRUE(unit_id_.empty());

    unit_id_.Assign(contract->ID());

    EXPECT_FALSE(unit_id_.empty());

    {
        auto issuer =
            api_issuer_.Wallet().mutable_Nym(issuer_.nym_id_, issuer_.Reason());
        issuer.AddPreferredOTServer(
            server_1_.id_.asBase58(api_issuer_.Crypto()),
            true,
            issuer_.Reason());
    }

    auto task = api_issuer_.OTX().IssueUnitDefinition(
        issuer_.nym_id_, server_1_.id_, unit_id_, ot::UnitType::Usd);
    auto& [taskID, future] = task;
    const auto result = future.get();

    EXPECT_NE(0, taskID);
    EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, result.first);
    ASSERT_TRUE(result.second);

    EXPECT_TRUE(issuer_.SetAccount(
        UNIT_DEFINITION_TLA, result.second->acct_id_->Get()));
    EXPECT_FALSE(issuer_.Account(UNIT_DEFINITION_TLA).empty());

    api_issuer_.OTX().ContextIdle(issuer_.nym_id_, server_1_.id_).get();

    {
        const auto pNym = api_issuer_.Wallet().Nym(issuer_.nym_id_);

        ASSERT_TRUE(pNym);

        const auto& nym = *pNym;
        const auto& claims = nym.Claims();
        const auto pSection =
            claims.Section(ot::identity::wot::claim::SectionType::Contract);

        ASSERT_TRUE(pSection);

        const auto& section = *pSection;
        const auto pGroup =
            section.Group(ot::identity::wot::claim::ClaimType::Usd);

        ASSERT_TRUE(pGroup);

        const auto& group = *pGroup;
        const auto& pClaim = group.PrimaryClaim();

        EXPECT_EQ(1, group.Size());
        ASSERT_TRUE(pClaim);

        const auto& claim = *pClaim;

        EXPECT_EQ(claim.Value(), unit_id_.asBase58(api_issuer_.Crypto()));
    }
}

TEST_F(Pair, pair_untrusted)
{
    account_summary_.expected_ += 5;

    ASSERT_TRUE(api_chris_.InternalClient().Pair().AddIssuer(
        chris_.nym_id_, issuer_.nym_id_, ""));
    EXPECT_TRUE(issuer_data_.bailment_.get());

    api_chris_.InternalClient().Pair().Wait().get();

    {
        const auto pIssuer = api_chris_.Wallet().Internal().Issuer(
            chris_.nym_id_, issuer_.nym_id_);

        ASSERT_TRUE(pIssuer);

        const auto& issuer = *pIssuer;

        EXPECT_EQ(1, issuer.AccountList(ot::UnitType::Usd, unit_id_).size());
        EXPECT_FALSE(issuer.BailmentInitiated(unit_id_));
        EXPECT_EQ(3, issuer.BailmentInstructions(api_chris_, unit_id_).size());
        EXPECT_EQ(
            issuer
                .ConnectionInfo(
                    api_chris_, ot::contract::peer::ConnectionInfoType::Bitcoin)
                .size(),
            0);
        EXPECT_EQ(
            issuer
                .ConnectionInfo(
                    api_chris_, ot::contract::peer::ConnectionInfoType::BtcRpc)
                .size(),
            0);
        EXPECT_EQ(

            issuer
                .ConnectionInfo(
                    api_chris_,
                    ot::contract::peer::ConnectionInfoType::BitMessage)
                .size(),
            0);
        EXPECT_EQ(

            issuer
                .ConnectionInfo(
                    api_chris_,
                    ot::contract::peer::ConnectionInfoType::BitMessageRPC)
                .size(),
            0);
        EXPECT_EQ(
            issuer
                .ConnectionInfo(
                    api_chris_, ot::contract::peer::ConnectionInfoType::SSH)
                .size(),
            0);
        EXPECT_EQ(
            issuer
                .ConnectionInfo(
                    api_chris_, ot::contract::peer::ConnectionInfoType::CJDNS)
                .size(),
            0);
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::Bitcoin));
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::BtcRpc));
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::BitMessage));
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::BitMessageRPC));
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::SSH));
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::CJDNS));
        EXPECT_EQ(issuer_.nym_id_, issuer.IssuerID());
        EXPECT_EQ(chris_.nym_id_, issuer.LocalNymID());
        EXPECT_FALSE(issuer.Paired());
        EXPECT_TRUE(issuer.PairingCode().empty());
        EXPECT_EQ(server_1_.id_, issuer.PrimaryServer());
        EXPECT_FALSE(issuer.StoreSecretComplete());
        EXPECT_FALSE(issuer.StoreSecretInitiated());
    }
}

TEST_F(Pair, pair_untrusted_state)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));

    const auto& widget = chris_.api_->UI().Internal().AccountSummary(
        chris_.nym_id_, ot::UnitType::Usd);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_TRUE(row->ConnectionState());
    EXPECT_EQ(row->Name(), "localhost");
    EXPECT_FALSE(row->Trusted());

    {
        const auto subrow = row->First();

        ASSERT_TRUE(subrow->Valid());
        EXPECT_FALSE(subrow->AccountID().empty());
        EXPECT_EQ(subrow->Balance(), 0);
        EXPECT_EQ(subrow->DisplayBalance(), "$0.00");
        EXPECT_FALSE(subrow->AccountID().empty());
        EXPECT_TRUE(subrow->Last());

        chris_.SetAccount("USD", subrow->AccountID());
    }

    EXPECT_TRUE(row->Last());
}

TEST_F(Pair, pair_trusted)
{
    account_summary_.expected_ += 2;

    ASSERT_TRUE(api_chris_.InternalClient().Pair().AddIssuer(
        chris_.nym_id_, issuer_.nym_id_, server_1_.password_));

    api_chris_.InternalClient().Pair().Wait().get();

    {
        const auto pIssuer = api_chris_.Wallet().Internal().Issuer(
            chris_.nym_id_, issuer_.nym_id_);

        ASSERT_TRUE(pIssuer);

        const auto& issuer = *pIssuer;

        EXPECT_EQ(1, issuer.AccountList(ot::UnitType::Usd, unit_id_).size());
        EXPECT_FALSE(issuer.BailmentInitiated(unit_id_));
        EXPECT_EQ(3, issuer.BailmentInstructions(api_chris_, unit_id_).size());
        EXPECT_EQ(
            issuer
                .ConnectionInfo(
                    api_chris_, ot::contract::peer::ConnectionInfoType::Bitcoin)
                .size(),
            0);
        EXPECT_EQ(
            issuer
                .ConnectionInfo(
                    api_chris_, ot::contract::peer::ConnectionInfoType::BtcRpc)
                .size(),
            0);
        EXPECT_EQ(

            issuer
                .ConnectionInfo(
                    api_chris_,
                    ot::contract::peer::ConnectionInfoType::BitMessage)
                .size(),
            0);
        EXPECT_EQ(

            issuer
                .ConnectionInfo(
                    api_chris_,
                    ot::contract::peer::ConnectionInfoType::BitMessageRPC)
                .size(),
            0);
        EXPECT_EQ(
            issuer
                .ConnectionInfo(
                    api_chris_, ot::contract::peer::ConnectionInfoType::SSH)
                .size(),
            0);
        EXPECT_EQ(
            issuer
                .ConnectionInfo(
                    api_chris_, ot::contract::peer::ConnectionInfoType::CJDNS)
                .size(),
            0);
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::Bitcoin));
        EXPECT_TRUE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::BtcRpc));
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::BitMessage));
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::BitMessageRPC));
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::SSH));
        EXPECT_FALSE(issuer.ConnectionInfoInitiated(
            ot::contract::peer::ConnectionInfoType::CJDNS));
        EXPECT_EQ(issuer_.nym_id_, issuer.IssuerID());
        EXPECT_EQ(chris_.nym_id_, issuer.LocalNymID());
        EXPECT_TRUE(issuer.Paired());
        EXPECT_EQ(issuer.PairingCode(), server_1_.password_);
        EXPECT_EQ(server_1_.id_, issuer.PrimaryServer());
        EXPECT_FALSE(issuer.StoreSecretComplete());

        if (ot::api::crypto::HaveHDKeys()) {
            EXPECT_TRUE(issuer.StoreSecretInitiated());
        } else {
            EXPECT_FALSE(issuer.StoreSecretInitiated());
        }
    }
}

TEST_F(Pair, pair_trusted_state)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));

    const auto& widget = chris_.api_->UI().Internal().AccountSummary(
        chris_.nym_id_, ot::UnitType::Usd);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_TRUE(row->ConnectionState());
    EXPECT_EQ(row->Name(), issuer_data_.new_notary_name_);
    EXPECT_TRUE(row->Trusted());

    {
        const auto subrow = row->First();

        ASSERT_TRUE(subrow->Valid());
        EXPECT_FALSE(subrow->AccountID().empty());
        EXPECT_EQ(subrow->Balance(), 0);
        EXPECT_EQ(subrow->DisplayBalance(), "$0.00");
        EXPECT_FALSE(subrow->AccountID().empty());
        EXPECT_TRUE(subrow->Last());
    }

    EXPECT_TRUE(row->Last());
}

TEST_F(Pair, shutdown)
{
    api_issuer_.OTX().ContextIdle(issuer_.nym_id_, server_1_.id_).get();
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();

    // TODO EXPECT_EQ(account_summary_.expected_, account_summary_.updated_);
}
}  // namespace ottest
