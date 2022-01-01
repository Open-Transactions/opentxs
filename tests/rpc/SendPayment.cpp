// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "rpc/Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <future>

#include "integration/Helpers.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/PaymentType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/SendPayment.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/SendPayment.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "paymentcode/VectorsV3.hpp"

namespace ot = opentxs;
namespace rpc = opentxs::rpc;

namespace ottest
{
constexpr auto issuer_{
    "ot2xuVPJDdweZvKLQD42UMCzhCmT3okn3W1PktLgCbmQLRnaKy848sX"};
constexpr auto brian_{
    "ot2xuUvJU5kP6hePQMrwFcm5MiM5g1eHnQjiY82M8LmLtNM9AAzGqE5"};
constexpr auto chris_{
    "ot2xuUYiopaGdTbiwspzvkXRANcbDi6hVsSU4uf94Zf8RAKJmRpCmfv"};

TEST_F(RPC_fixture, preconditions)
{
    {
        const auto& session = StartNotarySession(0);
        const auto instance = session.Instance();
        const auto& seeds = seed_map_.at(instance);
        const auto& nyms = local_nym_map_.at(instance);

        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 1);
    }
    {
        const auto& server = ot_.NotarySession(0);
        const auto& session = StartClient(0);
        session.OTX().DisableAutoaccept();
        session.InternalClient().Pair().Stop().get();
        const auto instance = session.Instance();
        const auto& nyms = local_nym_map_.at(instance);
        const auto& seeds = seed_map_.at(instance);
        const auto seed = ImportBip39(session, GetVectors3().alice_.words_);

        EXPECT_FALSE(seed.empty());
        EXPECT_TRUE(SetIntroductionServer(session, server));

        const auto& issuer = CreateNym(session, "issuer", seed, 0);
        const auto& brian = CreateNym(session, "brian", seed, 1);
        const auto& chris = CreateNym(session, "chris", seed, 2);

        EXPECT_EQ(issuer.nym_id_->str(), issuer_);
        EXPECT_EQ(brian.nym_id_->str(), brian_);
        EXPECT_EQ(chris.nym_id_->str(), chris_);
        EXPECT_TRUE(RegisterNym(server, issuer));
        EXPECT_TRUE(RegisterNym(server, brian));
        EXPECT_TRUE(RegisterNym(server, chris));

        const auto unit = IssueUnit(
            server,
            issuer,
            "Mt Gox USD",
            "YOLO",
            ot::core::UnitType::USD,
            {u8"USD", {{u8"dollers", {u8"$", u8"", {{10, 0}}, 2, 3}}}});

        EXPECT_FALSE(unit.empty());

        const auto account1 =
            RegisterAccount(server, brian, unit, "brian's dollars");
        const auto account2 =
            RegisterAccount(server, chris, unit, "chris's dollars");

        EXPECT_FALSE(account1.empty());
        EXPECT_FALSE(account2.empty());
        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 3);
        EXPECT_EQ(created_units_.size(), 1);
    }
}

TEST_F(RPC_fixture, bad_client_session)
{
    constexpr auto index{88};
    const auto& api = ot_.ClientSession(0);
    const auto& source = registered_accounts_.at(issuer_).front();
    const auto& destination = registered_accounts_.at(brian_).front();
    const auto recipient =
        api.Contacts().ContactID(api.Factory().NymID(brian_));

    ASSERT_FALSE(recipient->empty());

    const auto command = ot::rpc::request::SendPayment{
        index, source, destination, recipient->str(), 100};
    const auto& list = command.asSendPayment();
    const auto base = ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::send_payment);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(base->Cookie(), command.Cookie());
    EXPECT_EQ(base->Session(), command.Session());
    EXPECT_EQ(base->Type(), command.Type());
    EXPECT_EQ(base->Version(), command.Version());
    EXPECT_EQ(response.Cookie(), base->Cookie());
    EXPECT_EQ(response.ResponseCodes(), base->ResponseCodes());
    EXPECT_EQ(response.Session(), base->Session());
    EXPECT_EQ(response.Type(), base->Type());
    EXPECT_EQ(response.Version(), base->Version());
    EXPECT_EQ(response.Version(), command.Version());
    EXPECT_EQ(response.Cookie(), command.Cookie());
    EXPECT_EQ(response.Session(), command.Session());
    EXPECT_EQ(response.Type(), command.Type());
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::bad_session);
}

TEST_F(RPC_fixture, bad_server_session)
{
    constexpr auto index{99};
    const auto& api = ot_.ClientSession(0);
    const auto& source = registered_accounts_.at(issuer_).front();
    const auto& destination = registered_accounts_.at(brian_).front();
    const auto recipient =
        api.Contacts().ContactID(api.Factory().NymID(brian_));

    ASSERT_FALSE(recipient->empty());

    const auto command = ot::rpc::request::SendPayment{
        index, source, destination, recipient->str(), 100};
    const auto& list = command.asSendPayment();
    const auto base = ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::send_payment);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(base->Cookie(), command.Cookie());
    EXPECT_EQ(base->Session(), command.Session());
    EXPECT_EQ(base->Type(), command.Type());
    EXPECT_EQ(base->Version(), command.Version());
    EXPECT_EQ(response.Cookie(), base->Cookie());
    EXPECT_EQ(response.ResponseCodes(), base->ResponseCodes());
    EXPECT_EQ(response.Session(), base->Session());
    EXPECT_EQ(response.Type(), base->Type());
    EXPECT_EQ(response.Version(), base->Version());
    EXPECT_EQ(response.Version(), command.Version());
    EXPECT_EQ(response.Cookie(), command.Cookie());
    EXPECT_EQ(response.Session(), command.Session());
    EXPECT_EQ(response.Type(), command.Type());
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::bad_session);
}

TEST_F(RPC_fixture, transfer)
{
    constexpr auto index{0};
    const auto& api = ot_.ClientSession(0);
    const auto& server = ot_.NotarySession(0);
    const auto& source = registered_accounts_.at(issuer_).front();
    const auto& destination = registered_accounts_.at(brian_).front();
    const auto recipient =
        api.Contacts().ContactID(api.Factory().NymID(brian_));

    ASSERT_FALSE(recipient->empty());

    const auto command = ot::rpc::request::SendPayment{
        index, source, recipient->str(), destination, 100};
    const auto& list = command.asSendPayment();
    const auto base = ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::send_payment);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(base->Cookie(), command.Cookie());
    EXPECT_EQ(base->Session(), command.Session());
    EXPECT_EQ(base->Type(), command.Type());
    EXPECT_EQ(base->Version(), command.Version());
    EXPECT_EQ(response.Cookie(), base->Cookie());
    EXPECT_EQ(response.ResponseCodes(), base->ResponseCodes());
    EXPECT_EQ(response.Session(), base->Session());
    EXPECT_EQ(response.Type(), base->Type());
    EXPECT_EQ(response.Version(), base->Version());
    EXPECT_EQ(response.Version(), command.Version());
    EXPECT_EQ(response.Cookie(), command.Cookie());
    EXPECT_EQ(response.Session(), command.Session());
    EXPECT_EQ(response.Type(), command.Type());
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::queued);
    ASSERT_TRUE(push_.wait(0));
    // TODO validate each push message

    RefreshAccount(api, api.Factory().NymID(brian_), server.ID());
    RefreshAccount(api, api.Factory().NymID(issuer_), server.ID());

    ASSERT_TRUE(push_.wait(2));
    // TODO validate each push message
}

TEST_F(RPC_fixture, cheque)
{
    constexpr auto index{0};
    const auto& api = ot_.ClientSession(0);
    const auto& server = ot_.NotarySession(0);
    constexpr auto type = ot::rpc::PaymentType::cheque;
    const auto& source = registered_accounts_.at(brian_).front();
    const auto recipient =
        api.Contacts().ContactID(api.Factory().NymID(chris_));

    ASSERT_FALSE(recipient->empty());

    const auto command = ot::rpc::request::SendPayment{
        index, type, source, recipient->str(), 75};
    const auto& list = command.asSendPayment();
    const auto base = ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::send_payment);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(base->Cookie(), command.Cookie());
    EXPECT_EQ(base->Session(), command.Session());
    EXPECT_EQ(base->Type(), command.Type());
    EXPECT_EQ(base->Version(), command.Version());
    EXPECT_EQ(response.Cookie(), base->Cookie());
    EXPECT_EQ(response.ResponseCodes(), base->ResponseCodes());
    EXPECT_EQ(response.Session(), base->Session());
    EXPECT_EQ(response.Type(), base->Type());
    EXPECT_EQ(response.Version(), base->Version());
    EXPECT_EQ(response.Version(), command.Version());
    EXPECT_EQ(response.Cookie(), command.Cookie());
    EXPECT_EQ(response.Session(), command.Session());
    EXPECT_EQ(response.Type(), command.Type());
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::queued);
    ASSERT_TRUE(push_.wait(3));
    // TODO validate each push message

    RefreshAccount(api, api.Factory().NymID(chris_), server.ID());

    ASSERT_TRUE(push_.wait(4));

    EXPECT_EQ(DepositCheques(api, server, chris_), 1);

    RefreshAccount(api, api.Factory().NymID(brian_), server.ID());

    ASSERT_TRUE(push_.wait(1));
    // TODO validate each push message
}

TEST_F(RPC_fixture, postconditions)
{
    const auto& api = ot_.ClientSession(0);
    auto account1 =
        api.Wallet().Internal().Account(api.Factory().IdentifierFromBase58(
            registered_accounts_.at(issuer_).front()));
    auto account2 =
        api.Wallet().Internal().Account(api.Factory().IdentifierFromBase58(
            registered_accounts_.at(brian_).front()));
    auto account3 =
        api.Wallet().Internal().Account(api.Factory().IdentifierFromBase58(
            registered_accounts_.at(chris_).front()));

    EXPECT_EQ(account1.get().GetBalance(), -100);
    EXPECT_EQ(account2.get().GetBalance(), 25);
    EXPECT_EQ(account3.get().GetBalance(), 75);
}

// TODO test other instruments
// TODO add test cases for all failure modes

TEST_F(RPC_fixture, cleanup) { Cleanup(); }
}  // namespace ottest
