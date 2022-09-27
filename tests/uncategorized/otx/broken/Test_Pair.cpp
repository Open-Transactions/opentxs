// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Notary

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <utility>

#include "internal/api/session/Client.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/peer/Peer.hpp"
#include "internal/otx/client/Issuer.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"

#define UNIT_DEFINITION_CONTRACT_VERSION 2
#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_TLA "USD"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT ot::UnitType::Usd

namespace ottest
{
Counter account_summary_{};

class Test_Pair : public IntegrationFixture
{
public:
    static Callbacks cb_chris_;
    static Issuer issuer_data_;
    static ot::identifier::UnitDefinition unit_id_;

    const ot::api::session::Client& api_issuer_;
    const ot::api::session::Client& api_chris_;
    const ot::api::session::Notary& api_server_1_;
    ot::OTZMQListenCallback issuer_peer_request_cb_;
    ot::OTZMQListenCallback chris_rename_notary_cb_;
    ot::OTZMQSubscribeSocket issuer_peer_request_listener_;
    ot::OTZMQSubscribeSocket chris_rename_notary_listener_;

    Test_Pair()
        : api_issuer_(ot::Context().StartClientSession(0))
        , api_chris_(ot::Context().StartClientSession(1))
        , api_server_1_(ot::Context().StartNotarySession(0))
        , issuer_peer_request_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto&& in) { issuer_peer_request(std::move(in)); }))
        , chris_rename_notary_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto&& in) { chris_rename_notary(std::move(in)); }))
        , issuer_peer_request_listener_(
              api_issuer_.Network().ZeroMQ().SubscribeSocket(
                  issuer_peer_request_cb_))
        , chris_rename_notary_listener_(
              api_chris_.Network().ZeroMQ().SubscribeSocket(
                  chris_rename_notary_cb_))
    {
        subscribe_sockets();

        const_cast<Server&>(server_1_).init(api_server_1_);
        const_cast<User&>(issuer_).init(api_issuer_, server_1_);
        const_cast<User&>(chris_).init(api_chris_, server_1_);
    }

    void subscribe_sockets()
    {
        ASSERT_TRUE(issuer_peer_request_listener_->Start(ot::UnallocatedCString{
            api_issuer_.Endpoints().PeerRequestUpdate()}));
        ASSERT_TRUE(chris_rename_notary_listener_->Start(
            ot::UnallocatedCString{api_chris_.Endpoints().PairEvent()}));
    }

    void chris_rename_notary(ot::network::zeromq::Message&& in)
    {
        const auto body = in.Body();

        EXPECT_EQ(1, body.size());

        if (1 != body.size()) { return; }

        const auto event =
            ot::contract::peer::internal::PairEvent(body.at(0).Bytes());
        EXPECT_EQ(1, event.version_);
        EXPECT_EQ(
            ot::contract::peer::internal::PairEventType::Rename, event.type_);
        EXPECT_EQ(
            issuer_.nym_id_.asBase58(api_issuer_.Crypto()), event.issuer_);
        EXPECT_TRUE(api_chris_.Wallet().SetServerAlias(
            server_1_.id_, issuer_data_.new_notary_name_));

        const auto result = api_chris_.OTX().DownloadNym(
            chris_.nym_id_, server_1_.id_, issuer_.nym_id_);

        EXPECT_NE(0, result.first);

        if (0 == result.first) { return; }
    }

    void issuer_peer_request(ot::network::zeromq::Message&& in)
    {
        const auto body = in.Body();

        EXPECT_EQ(2, body.size());

        if (2 != body.size()) { return; }

        EXPECT_EQ(
            issuer_.nym_id_.asBase58(api_issuer_.Crypto()), body.at(0).Bytes());

        const auto nym_p = api_chris_.Wallet().Nym(chris_.nym_id_);
        const auto request =
            api_chris_.Factory().PeerRequest(nym_p, body.at(1).Bytes());

        EXPECT_EQ(
            body.at(0).Bytes(),
            request->Recipient().asBase58(api_issuer_.Crypto()));
        EXPECT_EQ(server_1_.id_, request->Server());

        switch (request->Type()) {
            case ot::contract::peer::PeerRequestType::Bailment: {
                const auto bailment = api_issuer_.Factory().BailmentRequest(
                    nym_p, body.at(1).Bytes());
                EXPECT_EQ(bailment->ServerID(), request->Server());
                EXPECT_EQ(bailment->UnitID(), unit_id_);

                api_issuer_.OTX().AcknowledgeBailment(
                    issuer_.nym_id_,
                    request->Server(),
                    request->Initiator(),
                    request->ID(),
                    std::to_string(++issuer_data_.bailment_counter_));

                if (issuer_data_.expected_bailments_ ==
                    issuer_data_.bailment_counter_) {
                    issuer_data_.bailment_promise_.set_value(true);
                }
            } break;
            case ot::contract::peer::PeerRequestType::StoreSecret: {
                // TODO
            } break;
            case ot::contract::peer::PeerRequestType::ConnectionInfo: {
                // TODO
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
};

ot::identifier::UnitDefinition Test_Pair::unit_id_{};
Callbacks Test_Pair::cb_chris_{ot::Context(), chris_.name_};
const ot::UnallocatedCString Issuer::new_notary_name_{"Chris's Notary"};
Issuer Test_Pair::issuer_data_{};

TEST_F(Test_Pair, init_ot) {}

TEST_F(Test_Pair, init_ui)
{
    account_summary_.expected_ = 0;
    api_chris_.UI().AccountSummary(
        chris_.nym_id_,
        ot::UnitType::Usd,
        make_cb(account_summary_, "account summary USD"));
}

TEST_F(Test_Pair, initial_state)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));

    const auto& widget =
        chris_.api_->UI().AccountSummary(chris_.nym_id_, ot::UnitType::Usd);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_Pair, issue_dollars)
{
    const auto contract = api_issuer_.Wallet().CurrencyContract(
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

TEST_F(Test_Pair, pair_untrusted)
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

TEST_F(Test_Pair, pair_untrusted_state)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));

    const auto& widget =
        chris_.api_->UI().AccountSummary(chris_.nym_id_, ot::UnitType::Usd);
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

TEST_F(Test_Pair, pair_trusted)
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

TEST_F(Test_Pair, pair_trusted_state)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));

    const auto& widget =
        chris_.api_->UI().AccountSummary(chris_.nym_id_, ot::UnitType::Usd);
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

TEST_F(Test_Pair, shutdown)
{
    api_issuer_.OTX().ContextIdle(issuer_.nym_id_, server_1_.id_).get();
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();

    // TODO EXPECT_EQ(account_summary_.expected_, account_summary_.updated_);
}
}  // namespace ottest
