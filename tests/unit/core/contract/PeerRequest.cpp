// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <initializer_list>
#include <optional>
#include <span>

#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/core/contract/Peer.hpp"

namespace ottest
{
using enum opentxs::contract::peer::RequestType;

TEST_F(PeerRequests, init) {}

TEST_F(PeerRequests, bailment)
{
    constexpr auto type = Bailment;
    constexpr auto requestVersion = 4u;
    constexpr auto replyVersion = 4u;
    const auto request = client_1_.Factory().BailmentRequest(
        alex_.nym_, bob_.nym_id_, unit_, notary_, reason_);
    const auto& requestID = request.ID();

    EXPECT_TRUE(request.Alias().empty());
    EXPECT_TRUE(request.Alias({}).empty());

    {
        const auto& bailment = request.asBailment();

        EXPECT_TRUE(bailment.IsValid());
        EXPECT_EQ(bailment.Notary(), notary_);
        EXPECT_EQ(bailment.Unit(), unit_);
    }

    EXPECT_FALSE(request.asBailmentNotice().IsValid());
    EXPECT_FALSE(request.asConnection().IsValid());
    EXPECT_FALSE(request.asFaucet().IsValid());
    EXPECT_FALSE(request.asOutbailment().IsValid());
    EXPECT_FALSE(request.asStoreSecret().IsValid());
    EXPECT_FALSE(request.asVerification().IsValid());
    EXPECT_FALSE(requestID.empty());
    EXPECT_EQ(request.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(request.IsValid());
    EXPECT_TRUE(request.Name().empty());
    EXPECT_EQ(request.Responder(), bob_.nym_id_);
    EXPECT_TRUE(request.Signer());
    EXPECT_TRUE(request.Terms().empty());
    EXPECT_EQ(request.Type(), type);
    EXPECT_TRUE(request.Validate());
    EXPECT_EQ(request.Version(), requestVersion);

    const auto serializedRequest = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(request.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredRequest =
        client_1_.Factory().PeerRequest(serializedRequest.Bytes()).asBailment();

    ASSERT_TRUE(recoveredRequest.IsValid());
    EXPECT_EQ(recoveredRequest, request);
    EXPECT_TRUE(recoveredRequest.Alias().empty());
    EXPECT_TRUE(recoveredRequest.Alias({}).empty());

    {
        const auto& bailment = recoveredRequest.asBailment();

        EXPECT_TRUE(bailment.IsValid());
        EXPECT_EQ(bailment.Notary(), notary_);
        EXPECT_EQ(bailment.Unit(), unit_);
    }

    EXPECT_FALSE(recoveredRequest.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredRequest.asConnection().IsValid());
    EXPECT_FALSE(recoveredRequest.asFaucet().IsValid());
    EXPECT_FALSE(recoveredRequest.asOutbailment().IsValid());
    EXPECT_FALSE(recoveredRequest.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredRequest.asVerification().IsValid());
    EXPECT_EQ(recoveredRequest.ID(), request.ID());
    EXPECT_EQ(recoveredRequest.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Name().empty());
    EXPECT_EQ(recoveredRequest.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Signer());
    EXPECT_TRUE(recoveredRequest.Terms().empty());
    EXPECT_EQ(recoveredRequest.Type(), type);
    EXPECT_TRUE(recoveredRequest.Validate());
    EXPECT_EQ(recoveredRequest.Version(), requestVersion);

    const auto reply = client_1_.Factory().BailmentReply(
        bob_.nym_, alex_.nym_id_, requestID, terms_, reason_);

    EXPECT_TRUE(reply.Alias().empty());
    EXPECT_TRUE(reply.Alias({}).empty());

    {
        const auto& bailment = reply.asBailment();

        EXPECT_TRUE(bailment.IsValid());
        EXPECT_EQ(bailment.Instructions(), terms_);
    }

    EXPECT_FALSE(reply.asBailmentNotice().IsValid());
    EXPECT_FALSE(reply.asConnection().IsValid());
    EXPECT_FALSE(reply.asFaucet().IsValid());
    EXPECT_FALSE(reply.asOutbailment().IsValid());
    EXPECT_FALSE(reply.asStoreSecret().IsValid());
    EXPECT_FALSE(reply.asVerification().IsValid());
    EXPECT_FALSE(reply.ID().empty());
    EXPECT_EQ(reply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(reply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(reply.IsValid());
    EXPECT_TRUE(reply.Name().empty());
    EXPECT_EQ(reply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(reply.Signer());
    EXPECT_TRUE(reply.Terms().empty());
    EXPECT_EQ(reply.Type(), type);
    EXPECT_TRUE(reply.Validate());
    EXPECT_EQ(reply.Version(), replyVersion);

    const auto serializedReply = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(reply.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredReply =
        client_1_.Factory().PeerReply(serializedReply.Bytes()).asBailment();

    ASSERT_TRUE(recoveredReply.IsValid());
    EXPECT_EQ(recoveredReply, reply);
    EXPECT_TRUE(recoveredReply.Alias().empty());
    EXPECT_TRUE(recoveredReply.Alias({}).empty());

    {
        const auto& bailment = recoveredReply.asBailment();

        EXPECT_TRUE(bailment.IsValid());
        EXPECT_EQ(bailment.Instructions(), terms_);
    }

    EXPECT_FALSE(recoveredReply.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredReply.asConnection().IsValid());
    EXPECT_FALSE(recoveredReply.asFaucet().IsValid());
    EXPECT_FALSE(recoveredReply.asOutbailment().IsValid());
    EXPECT_FALSE(recoveredReply.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredReply.asVerification().IsValid());
    EXPECT_EQ(recoveredReply.ID(), reply.ID());
    EXPECT_EQ(recoveredReply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(recoveredReply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(recoveredReply.Name().empty());
    EXPECT_EQ(recoveredReply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredReply.Signer());
    EXPECT_TRUE(recoveredReply.Terms().empty());
    EXPECT_EQ(recoveredReply.Type(), type);
    EXPECT_TRUE(recoveredReply.Validate());
    EXPECT_EQ(recoveredReply.Version(), replyVersion);
}

TEST_F(PeerRequests, bailment_notice)
{
    constexpr auto type = PendingBailment;
    constexpr auto requestVersion = 6u;
    constexpr auto replyVersion = 4u;
    const auto bailmentID = client_1_.Factory().IdentifierFromRandom();
    const auto request = client_1_.Factory().BailmentNoticeRequest(
        alex_.nym_,
        bob_.nym_id_,
        unit_,
        notary_,
        bailmentID,
        txid_.Bytes(),
        amount_,
        reason_);
    const auto& requestID = request.ID();

    EXPECT_TRUE(request.Alias().empty());
    EXPECT_TRUE(request.Alias({}).empty());
    EXPECT_FALSE(request.asBailment().IsValid());

    {
        const auto& bailmentnotice = request.asBailmentNotice();

        EXPECT_EQ(bailmentnotice.Amount(), amount_);
        EXPECT_EQ(bailmentnotice.Description(), txid_.Bytes());
        EXPECT_EQ(bailmentnotice.InReferenceToRequest(), bailmentID);
        EXPECT_TRUE(bailmentnotice.IsValid());
        EXPECT_EQ(bailmentnotice.Notary(), notary_);
        EXPECT_EQ(bailmentnotice.Unit(), unit_);
    }

    EXPECT_FALSE(request.asConnection().IsValid());
    EXPECT_FALSE(request.asFaucet().IsValid());
    EXPECT_FALSE(request.asOutbailment().IsValid());
    EXPECT_FALSE(request.asStoreSecret().IsValid());
    EXPECT_FALSE(request.asVerification().IsValid());
    EXPECT_FALSE(requestID.empty());
    EXPECT_EQ(request.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(request.IsValid());
    EXPECT_TRUE(request.Name().empty());
    EXPECT_EQ(request.Responder(), bob_.nym_id_);
    EXPECT_TRUE(request.Signer());
    EXPECT_TRUE(request.Terms().empty());
    EXPECT_EQ(request.Type(), type);
    EXPECT_TRUE(request.Validate());
    EXPECT_EQ(request.Version(), requestVersion);

    const auto serializedRequest = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(request.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredRequest = client_1_.Factory()
                                      .PeerRequest(serializedRequest.Bytes())
                                      .asBailmentNotice();

    ASSERT_TRUE(recoveredRequest.IsValid());
    EXPECT_EQ(recoveredRequest, request);
    EXPECT_TRUE(recoveredRequest.Alias().empty());
    EXPECT_TRUE(recoveredRequest.Alias({}).empty());
    EXPECT_FALSE(recoveredRequest.asBailment().IsValid());

    {
        const auto& bailmentnotice = recoveredRequest.asBailmentNotice();

        EXPECT_EQ(bailmentnotice.Amount(), amount_);
        EXPECT_EQ(bailmentnotice.Description(), txid_.Bytes());
        EXPECT_EQ(bailmentnotice.InReferenceToRequest(), bailmentID);
        EXPECT_TRUE(bailmentnotice.IsValid());
        EXPECT_EQ(bailmentnotice.Notary(), notary_);
        EXPECT_EQ(bailmentnotice.Unit(), unit_);
    }

    EXPECT_FALSE(recoveredRequest.asConnection().IsValid());
    EXPECT_FALSE(recoveredRequest.asFaucet().IsValid());
    EXPECT_FALSE(recoveredRequest.asOutbailment().IsValid());
    EXPECT_FALSE(recoveredRequest.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredRequest.asVerification().IsValid());
    EXPECT_EQ(recoveredRequest.ID(), request.ID());
    EXPECT_EQ(recoveredRequest.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Name().empty());
    EXPECT_EQ(recoveredRequest.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Signer());
    EXPECT_TRUE(recoveredRequest.Terms().empty());
    EXPECT_EQ(recoveredRequest.Type(), type);
    EXPECT_TRUE(recoveredRequest.Validate());
    EXPECT_EQ(recoveredRequest.Version(), requestVersion);

    const auto reply = client_1_.Factory().BailmentNoticeReply(
        bob_.nym_, alex_.nym_id_, requestID, true, reason_);

    EXPECT_TRUE(reply.Alias().empty());
    EXPECT_TRUE(reply.Alias({}).empty());
    EXPECT_FALSE(reply.asBailment().IsValid());

    {
        const auto& bailmentnotice = reply.asBailmentNotice();

        EXPECT_TRUE(bailmentnotice.IsValid());
        EXPECT_TRUE(bailmentnotice.Value());
    }

    EXPECT_FALSE(reply.asConnection().IsValid());
    EXPECT_FALSE(reply.asFaucet().IsValid());
    EXPECT_FALSE(reply.asOutbailment().IsValid());
    EXPECT_FALSE(reply.asStoreSecret().IsValid());
    EXPECT_FALSE(reply.asVerification().IsValid());
    EXPECT_FALSE(reply.ID().empty());
    EXPECT_EQ(reply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(reply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(reply.IsValid());
    EXPECT_TRUE(reply.Name().empty());
    EXPECT_EQ(reply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(reply.Signer());
    EXPECT_TRUE(reply.Terms().empty());
    EXPECT_EQ(reply.Type(), type);
    EXPECT_TRUE(reply.Validate());
    EXPECT_EQ(reply.Version(), replyVersion);

    const auto serializedReply = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(reply.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredReply = client_1_.Factory()
                                    .PeerReply(serializedReply.Bytes())
                                    .asBailmentNotice();

    ASSERT_TRUE(recoveredReply.IsValid());
    EXPECT_EQ(recoveredReply, reply);
    EXPECT_TRUE(recoveredReply.Alias().empty());
    EXPECT_TRUE(recoveredReply.Alias({}).empty());
    EXPECT_FALSE(recoveredReply.asBailment().IsValid());

    {
        const auto& bailmentnotice = recoveredReply.asBailmentNotice();

        EXPECT_TRUE(bailmentnotice.IsValid());
        EXPECT_TRUE(bailmentnotice.Value());
    }

    EXPECT_FALSE(recoveredReply.asConnection().IsValid());
    EXPECT_FALSE(recoveredReply.asFaucet().IsValid());
    EXPECT_FALSE(recoveredReply.asOutbailment().IsValid());
    EXPECT_FALSE(recoveredReply.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredReply.asVerification().IsValid());
    EXPECT_EQ(recoveredReply.ID(), reply.ID());
    EXPECT_EQ(recoveredReply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(recoveredReply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(recoveredReply.Name().empty());
    EXPECT_EQ(recoveredReply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredReply.Signer());
    EXPECT_TRUE(recoveredReply.Terms().empty());
    EXPECT_EQ(recoveredReply.Type(), type);
    EXPECT_TRUE(recoveredReply.Validate());
    EXPECT_EQ(recoveredReply.Version(), replyVersion);
}

TEST_F(PeerRequests, connection)
{
    constexpr auto type = ConnectionInfo;
    constexpr auto requestVersion = 4u;
    constexpr auto replyVersion = 4u;
    using enum opentxs::contract::peer::ConnectionInfoType;
    constexpr auto kind = CJDNS;
    const auto request = client_1_.Factory().ConnectionRequest(
        alex_.nym_, bob_.nym_id_, kind, reason_);
    const auto& requestID = request.ID();

    EXPECT_TRUE(request.Alias().empty());
    EXPECT_TRUE(request.Alias({}).empty());
    EXPECT_FALSE(request.asBailment().IsValid());
    EXPECT_FALSE(request.asBailmentNotice().IsValid());

    {
        const auto& connection = request.asConnection();

        EXPECT_TRUE(connection.IsValid());
        EXPECT_EQ(connection.Kind(), kind);
    }

    EXPECT_FALSE(request.asFaucet().IsValid());
    EXPECT_FALSE(request.asOutbailment().IsValid());
    EXPECT_FALSE(request.asStoreSecret().IsValid());
    EXPECT_FALSE(request.asVerification().IsValid());
    EXPECT_FALSE(requestID.empty());
    EXPECT_EQ(request.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(request.IsValid());
    EXPECT_TRUE(request.Name().empty());
    EXPECT_EQ(request.Responder(), bob_.nym_id_);
    EXPECT_TRUE(request.Signer());
    EXPECT_TRUE(request.Terms().empty());
    EXPECT_EQ(request.Type(), type);
    EXPECT_TRUE(request.Validate());
    EXPECT_EQ(request.Version(), requestVersion);

    const auto serializedRequest = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(request.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredRequest = client_1_.Factory()
                                      .PeerRequest(serializedRequest.Bytes())
                                      .asConnection();

    ASSERT_TRUE(recoveredRequest.IsValid());
    EXPECT_EQ(recoveredRequest, request);
    EXPECT_TRUE(recoveredRequest.Alias().empty());
    EXPECT_TRUE(recoveredRequest.Alias({}).empty());
    EXPECT_FALSE(recoveredRequest.asBailment().IsValid());
    EXPECT_FALSE(recoveredRequest.asBailmentNotice().IsValid());

    {
        const auto& connection = request.asConnection();

        EXPECT_TRUE(connection.IsValid());
        EXPECT_EQ(connection.Kind(), kind);
    }

    EXPECT_FALSE(recoveredRequest.asFaucet().IsValid());
    EXPECT_FALSE(recoveredRequest.asOutbailment().IsValid());
    EXPECT_FALSE(recoveredRequest.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredRequest.asVerification().IsValid());
    EXPECT_EQ(recoveredRequest.ID(), request.ID());
    EXPECT_EQ(recoveredRequest.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Name().empty());
    EXPECT_EQ(recoveredRequest.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Signer());
    EXPECT_TRUE(recoveredRequest.Terms().empty());
    EXPECT_EQ(recoveredRequest.Type(), type);
    EXPECT_TRUE(recoveredRequest.Validate());
    EXPECT_EQ(recoveredRequest.Version(), requestVersion);

    const auto reply = client_1_.Factory().ConnectionReply(
        bob_.nym_,
        alex_.nym_id_,
        requestID,
        true,
        url_,
        login_,
        password_,
        key_,
        reason_);

    EXPECT_TRUE(reply.Alias().empty());
    EXPECT_TRUE(reply.Alias({}).empty());
    EXPECT_FALSE(reply.asBailment().IsValid());
    EXPECT_FALSE(reply.asBailmentNotice().IsValid());

    {
        const auto& connection = reply.asConnection();

        EXPECT_TRUE(connection.Accepted());
        EXPECT_EQ(connection.Endpoint(), url_);
        EXPECT_TRUE(connection.IsValid());
        EXPECT_EQ(connection.Key(), key_);
        EXPECT_EQ(connection.Login(), login_);
        EXPECT_EQ(connection.Password(), password_);
    }

    EXPECT_FALSE(reply.asFaucet().IsValid());
    EXPECT_FALSE(reply.asOutbailment().IsValid());
    EXPECT_FALSE(reply.asStoreSecret().IsValid());
    EXPECT_FALSE(reply.asVerification().IsValid());
    EXPECT_FALSE(reply.ID().empty());
    EXPECT_EQ(reply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(reply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(reply.IsValid());
    EXPECT_TRUE(reply.Name().empty());
    EXPECT_EQ(reply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(reply.Signer());
    EXPECT_TRUE(reply.Terms().empty());
    EXPECT_EQ(reply.Type(), type);
    EXPECT_TRUE(reply.Validate());
    EXPECT_EQ(reply.Version(), replyVersion);

    const auto serializedReply = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(reply.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredReply =
        client_1_.Factory().PeerReply(serializedReply.Bytes()).asConnection();

    ASSERT_TRUE(recoveredReply.IsValid());
    EXPECT_EQ(recoveredReply, reply);
    EXPECT_TRUE(recoveredReply.Alias().empty());
    EXPECT_TRUE(recoveredReply.Alias({}).empty());
    EXPECT_FALSE(recoveredReply.asBailment().IsValid());
    EXPECT_FALSE(recoveredReply.asBailmentNotice().IsValid());

    {
        const auto& connection = recoveredReply.asConnection();

        EXPECT_TRUE(connection.Accepted());
        EXPECT_EQ(connection.Endpoint(), url_);
        EXPECT_TRUE(connection.IsValid());
        EXPECT_EQ(connection.Key(), key_);
        EXPECT_EQ(connection.Login(), login_);
        EXPECT_EQ(connection.Password(), password_);
    }

    EXPECT_FALSE(recoveredReply.asFaucet().IsValid());
    EXPECT_FALSE(recoveredReply.asOutbailment().IsValid());
    EXPECT_FALSE(recoveredReply.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredReply.asVerification().IsValid());
    EXPECT_EQ(recoveredReply.ID(), reply.ID());
    EXPECT_EQ(recoveredReply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(recoveredReply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(recoveredReply.Name().empty());
    EXPECT_EQ(recoveredReply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredReply.Signer());
    EXPECT_TRUE(recoveredReply.Terms().empty());
    EXPECT_EQ(recoveredReply.Type(), type);
    EXPECT_TRUE(recoveredReply.Validate());
    EXPECT_EQ(recoveredReply.Version(), replyVersion);
}

TEST_F(PeerRequests, faucet)
{
    constexpr auto type = Faucet;
    constexpr auto requestVersion = 4u;
    constexpr auto replyVersion = 4u;
    using enum opentxs::UnitType;
    constexpr auto kind = Btc;
    const auto request = client_1_.Factory().FaucetRequest(
        alex_.nym_, bob_.nym_id_, kind, address_, reason_);
    const auto& requestID = request.ID();

    EXPECT_TRUE(request.Alias().empty());
    EXPECT_TRUE(request.Alias({}).empty());
    EXPECT_FALSE(request.asBailment().IsValid());
    EXPECT_FALSE(request.asBailmentNotice().IsValid());
    EXPECT_FALSE(request.asConnection().IsValid());

    {
        const auto& faucet = request.asFaucet();

        EXPECT_EQ(faucet.Currency(), kind);
        EXPECT_EQ(faucet.Instructions(), address_);
        EXPECT_TRUE(faucet.IsValid());
    }

    EXPECT_FALSE(request.asOutbailment().IsValid());
    EXPECT_FALSE(request.asStoreSecret().IsValid());
    EXPECT_FALSE(request.asVerification().IsValid());
    EXPECT_FALSE(requestID.empty());
    EXPECT_EQ(request.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(request.IsValid());
    EXPECT_TRUE(request.Name().empty());
    EXPECT_EQ(request.Responder(), bob_.nym_id_);
    EXPECT_TRUE(request.Signer());
    EXPECT_TRUE(request.Terms().empty());
    EXPECT_EQ(request.Type(), type);
    EXPECT_TRUE(request.Validate());
    EXPECT_EQ(request.Version(), requestVersion);

    const auto serializedRequest = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(request.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredRequest =
        client_1_.Factory().PeerRequest(serializedRequest.Bytes()).asFaucet();

    ASSERT_TRUE(recoveredRequest.IsValid());
    EXPECT_EQ(recoveredRequest, request);
    EXPECT_TRUE(recoveredRequest.Alias().empty());
    EXPECT_TRUE(recoveredRequest.Alias({}).empty());
    EXPECT_FALSE(recoveredRequest.asBailment().IsValid());
    EXPECT_FALSE(recoveredRequest.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredRequest.asConnection().IsValid());

    {
        const auto& faucet = request.asFaucet();

        EXPECT_EQ(faucet.Currency(), kind);
        EXPECT_EQ(faucet.Instructions(), address_);
        EXPECT_TRUE(faucet.IsValid());
    }

    EXPECT_FALSE(recoveredRequest.asOutbailment().IsValid());
    EXPECT_FALSE(recoveredRequest.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredRequest.asVerification().IsValid());
    EXPECT_EQ(recoveredRequest.ID(), request.ID());
    EXPECT_EQ(recoveredRequest.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Name().empty());
    EXPECT_EQ(recoveredRequest.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Signer());
    EXPECT_TRUE(recoveredRequest.Terms().empty());
    EXPECT_EQ(recoveredRequest.Type(), type);
    EXPECT_TRUE(recoveredRequest.Validate());
    EXPECT_EQ(recoveredRequest.Version(), requestVersion);

    const auto reply = client_1_.Factory().FaucetReply(
        bob_.nym_, alex_.nym_id_, requestID, tx_, reason_);

    EXPECT_TRUE(reply.Alias().empty());
    EXPECT_TRUE(reply.Alias({}).empty());
    EXPECT_FALSE(reply.asBailment().IsValid());
    EXPECT_FALSE(reply.asBailmentNotice().IsValid());
    EXPECT_FALSE(reply.asConnection().IsValid());

    {
        const auto& faucet = reply.asFaucet();

        EXPECT_TRUE(faucet.Accepted());
        EXPECT_TRUE(faucet.IsValid());
        EXPECT_EQ(faucet.Transaction(), tx_);
    }

    EXPECT_FALSE(reply.asOutbailment().IsValid());
    EXPECT_FALSE(reply.asStoreSecret().IsValid());
    EXPECT_FALSE(reply.asVerification().IsValid());
    EXPECT_FALSE(reply.ID().empty());
    EXPECT_EQ(reply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(reply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(reply.IsValid());
    EXPECT_TRUE(reply.Name().empty());
    EXPECT_EQ(reply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(reply.Signer());
    EXPECT_TRUE(reply.Terms().empty());
    EXPECT_EQ(reply.Type(), type);
    EXPECT_TRUE(reply.Validate());
    EXPECT_EQ(reply.Version(), replyVersion);

    const auto serializedReply = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(reply.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredReply =
        client_1_.Factory().PeerReply(serializedReply.Bytes()).asFaucet();

    ASSERT_TRUE(recoveredReply.IsValid());
    EXPECT_EQ(recoveredReply, reply);
    EXPECT_TRUE(recoveredReply.Alias().empty());
    EXPECT_TRUE(recoveredReply.Alias({}).empty());
    EXPECT_FALSE(recoveredReply.asBailment().IsValid());
    EXPECT_FALSE(recoveredReply.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredReply.asConnection().IsValid());

    {
        const auto& faucet = reply.asFaucet();

        EXPECT_TRUE(faucet.Accepted());
        EXPECT_TRUE(faucet.IsValid());
        EXPECT_EQ(faucet.Transaction(), tx_);
    }

    EXPECT_FALSE(recoveredReply.asOutbailment().IsValid());
    EXPECT_FALSE(recoveredReply.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredReply.asVerification().IsValid());
    EXPECT_EQ(recoveredReply.ID(), reply.ID());
    EXPECT_EQ(recoveredReply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(recoveredReply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(recoveredReply.Name().empty());
    EXPECT_EQ(recoveredReply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredReply.Signer());
    EXPECT_TRUE(recoveredReply.Terms().empty());
    EXPECT_EQ(recoveredReply.Type(), type);
    EXPECT_TRUE(recoveredReply.Validate());
    EXPECT_EQ(recoveredReply.Version(), replyVersion);
}

TEST_F(PeerRequests, outbailment)
{
    constexpr auto type = OutBailment;
    constexpr auto requestVersion = 4u;
    constexpr auto replyVersion = 4u;
    const auto request = client_1_.Factory().OutbailmentRequest(
        alex_.nym_, bob_.nym_id_, unit_, notary_, amount_, address_, reason_);
    const auto& requestID = request.ID();

    EXPECT_TRUE(request.Alias().empty());
    EXPECT_TRUE(request.Alias({}).empty());
    EXPECT_FALSE(request.asBailment().IsValid());
    EXPECT_FALSE(request.asBailmentNotice().IsValid());
    EXPECT_FALSE(request.asConnection().IsValid());
    EXPECT_FALSE(request.asFaucet().IsValid());

    {
        const auto& outbailment = request.asOutbailment();

        EXPECT_EQ(outbailment.Amount(), amount_);
        EXPECT_EQ(outbailment.Instructions(), address_);
        EXPECT_TRUE(outbailment.IsValid());
        EXPECT_EQ(outbailment.Notary(), notary_);
        EXPECT_EQ(outbailment.Unit(), unit_);
    }

    EXPECT_FALSE(request.asStoreSecret().IsValid());
    EXPECT_FALSE(request.asVerification().IsValid());
    EXPECT_FALSE(requestID.empty());
    EXPECT_EQ(request.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(request.IsValid());
    EXPECT_TRUE(request.Name().empty());
    EXPECT_EQ(request.Responder(), bob_.nym_id_);
    EXPECT_TRUE(request.Signer());
    EXPECT_TRUE(request.Terms().empty());
    EXPECT_EQ(request.Type(), type);
    EXPECT_TRUE(request.Validate());
    EXPECT_EQ(request.Version(), requestVersion);

    const auto serializedRequest = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(request.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredRequest = client_1_.Factory()
                                      .PeerRequest(serializedRequest.Bytes())
                                      .asOutbailment();

    ASSERT_TRUE(recoveredRequest.IsValid());
    EXPECT_EQ(recoveredRequest, request);
    EXPECT_TRUE(recoveredRequest.Alias().empty());
    EXPECT_TRUE(recoveredRequest.Alias({}).empty());
    EXPECT_FALSE(recoveredRequest.asBailment().IsValid());
    EXPECT_FALSE(recoveredRequest.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredRequest.asConnection().IsValid());
    EXPECT_FALSE(recoveredRequest.asFaucet().IsValid());

    {
        const auto& outbailment = recoveredRequest.asOutbailment();

        EXPECT_EQ(outbailment.Amount(), amount_);
        EXPECT_EQ(outbailment.Instructions(), address_);
        EXPECT_TRUE(outbailment.IsValid());
        EXPECT_EQ(outbailment.Notary(), notary_);
        EXPECT_EQ(outbailment.Unit(), unit_);
    }

    EXPECT_FALSE(recoveredRequest.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredRequest.asVerification().IsValid());
    EXPECT_EQ(recoveredRequest.ID(), request.ID());
    EXPECT_EQ(recoveredRequest.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Name().empty());
    EXPECT_EQ(recoveredRequest.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Signer());
    EXPECT_TRUE(recoveredRequest.Terms().empty());
    EXPECT_EQ(recoveredRequest.Type(), type);
    EXPECT_TRUE(recoveredRequest.Validate());
    EXPECT_EQ(recoveredRequest.Version(), requestVersion);

    const auto reply = client_1_.Factory().OutbailmentReply(
        bob_.nym_, alex_.nym_id_, requestID, txid_.asHex(), reason_);

    EXPECT_TRUE(reply.Alias().empty());
    EXPECT_TRUE(reply.Alias({}).empty());
    EXPECT_FALSE(reply.asBailment().IsValid());
    EXPECT_FALSE(reply.asBailmentNotice().IsValid());
    EXPECT_FALSE(reply.asConnection().IsValid());
    EXPECT_FALSE(reply.asFaucet().IsValid());

    {
        const auto& outbailment = reply.asOutbailment();

        EXPECT_EQ(outbailment.Description(), txid_.asHex());
        EXPECT_TRUE(outbailment.IsValid());
    }

    EXPECT_FALSE(reply.asStoreSecret().IsValid());
    EXPECT_FALSE(reply.asVerification().IsValid());
    EXPECT_FALSE(reply.ID().empty());
    EXPECT_EQ(reply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(reply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(reply.IsValid());
    EXPECT_TRUE(reply.Name().empty());
    EXPECT_EQ(reply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(reply.Signer());
    EXPECT_TRUE(reply.Terms().empty());
    EXPECT_EQ(reply.Type(), type);
    EXPECT_TRUE(reply.Validate());
    EXPECT_EQ(reply.Version(), replyVersion);

    const auto serializedReply = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(reply.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredReply =
        client_1_.Factory().PeerReply(serializedReply.Bytes()).asOutbailment();

    ASSERT_TRUE(recoveredReply.IsValid());
    EXPECT_EQ(recoveredReply, reply);
    EXPECT_TRUE(recoveredReply.Alias().empty());
    EXPECT_TRUE(recoveredReply.Alias({}).empty());
    EXPECT_FALSE(recoveredReply.asBailment().IsValid());
    EXPECT_FALSE(recoveredReply.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredReply.asConnection().IsValid());
    EXPECT_FALSE(recoveredReply.asFaucet().IsValid());

    {
        const auto& outbailment = reply.asOutbailment();

        EXPECT_EQ(outbailment.Description(), txid_.asHex());
        EXPECT_TRUE(outbailment.IsValid());
    }

    EXPECT_FALSE(recoveredReply.asStoreSecret().IsValid());
    EXPECT_FALSE(recoveredReply.asVerification().IsValid());
    EXPECT_EQ(recoveredReply.ID(), reply.ID());
    EXPECT_EQ(recoveredReply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(recoveredReply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(recoveredReply.Name().empty());
    EXPECT_EQ(recoveredReply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredReply.Signer());
    EXPECT_TRUE(recoveredReply.Terms().empty());
    EXPECT_EQ(recoveredReply.Type(), type);
    EXPECT_TRUE(recoveredReply.Validate());
    EXPECT_EQ(recoveredReply.Version(), replyVersion);
}

TEST_F(PeerRequests, storesecret)
{
    constexpr auto type = StoreSecret;
    constexpr auto requestVersion = 4u;
    constexpr auto replyVersion = 4u;
    static constexpr auto view = {secret_1_, secret_2_};
    using enum opentxs::contract::peer::SecretType;
    constexpr auto kind = Bip39;
    const auto request = client_1_.Factory().StoreSecretRequest(
        alex_.nym_, bob_.nym_id_, kind, view, reason_);
    const auto& requestID = request.ID();

    EXPECT_TRUE(request.Alias().empty());
    EXPECT_TRUE(request.Alias({}).empty());
    EXPECT_FALSE(request.asBailment().IsValid());
    EXPECT_FALSE(request.asBailmentNotice().IsValid());
    EXPECT_FALSE(request.asConnection().IsValid());
    EXPECT_FALSE(request.asFaucet().IsValid());
    EXPECT_FALSE(request.asFaucet().asOutbailment());

    {
        const auto& storesecret = request.asStoreSecret();
        const auto values = storesecret.Values();

        EXPECT_TRUE(storesecret.IsValid());
        EXPECT_EQ(storesecret.Kind(), kind);
        ASSERT_EQ(values.size(), view.size());

        auto n = std::size_t{0};

        for (const auto& item : view) {
            EXPECT_EQ(item, values[n]);
            ++n;
        }
    }

    EXPECT_FALSE(request.asVerification().IsValid());
    EXPECT_FALSE(requestID.empty());
    EXPECT_EQ(request.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(request.IsValid());
    EXPECT_TRUE(request.Name().empty());
    EXPECT_EQ(request.Responder(), bob_.nym_id_);
    EXPECT_TRUE(request.Signer());
    EXPECT_TRUE(request.Terms().empty());
    EXPECT_EQ(request.Type(), type);
    EXPECT_TRUE(request.Validate());
    EXPECT_EQ(request.Version(), requestVersion);

    const auto serializedRequest = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(request.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredRequest = client_1_.Factory()
                                      .PeerRequest(serializedRequest.Bytes())
                                      .asStoreSecret();

    ASSERT_TRUE(recoveredRequest.IsValid());
    EXPECT_EQ(recoveredRequest, request);
    EXPECT_TRUE(recoveredRequest.Alias().empty());
    EXPECT_TRUE(recoveredRequest.Alias({}).empty());
    EXPECT_FALSE(recoveredRequest.asBailment().IsValid());
    EXPECT_FALSE(recoveredRequest.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredRequest.asConnection().IsValid());
    EXPECT_FALSE(recoveredRequest.asFaucet().IsValid());
    EXPECT_FALSE(recoveredRequest.asFaucet().asOutbailment());

    {
        const auto& storesecret = recoveredRequest.asStoreSecret();
        const auto values = storesecret.Values();

        EXPECT_TRUE(storesecret.IsValid());
        EXPECT_EQ(storesecret.Kind(), kind);
        ASSERT_EQ(values.size(), view.size());

        auto n = std::size_t{0};

        for (const auto& item : view) {
            EXPECT_EQ(item, values[n]);
            ++n;
        }
    }

    EXPECT_FALSE(recoveredRequest.asVerification().IsValid());
    EXPECT_EQ(recoveredRequest.ID(), request.ID());
    EXPECT_EQ(recoveredRequest.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Name().empty());
    EXPECT_EQ(recoveredRequest.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Signer());
    EXPECT_TRUE(recoveredRequest.Terms().empty());
    EXPECT_EQ(recoveredRequest.Type(), type);
    EXPECT_TRUE(recoveredRequest.Validate());
    EXPECT_EQ(recoveredRequest.Version(), requestVersion);

    const auto reply = client_1_.Factory().StoreSecretReply(
        bob_.nym_, alex_.nym_id_, requestID, true, reason_);

    EXPECT_TRUE(reply.Alias().empty());
    EXPECT_TRUE(reply.Alias({}).empty());
    EXPECT_FALSE(reply.asBailment().IsValid());
    EXPECT_FALSE(reply.asBailmentNotice().IsValid());
    EXPECT_FALSE(reply.asConnection().IsValid());
    EXPECT_FALSE(reply.asFaucet().IsValid());
    EXPECT_FALSE(reply.asOutbailment().IsValid());

    {
        const auto& storesecret = reply.asStoreSecret();

        EXPECT_TRUE(storesecret.IsValid());
        EXPECT_TRUE(storesecret.Value());
    }

    EXPECT_FALSE(reply.asVerification().IsValid());
    EXPECT_FALSE(reply.ID().empty());
    EXPECT_EQ(reply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(reply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(reply.IsValid());
    EXPECT_TRUE(reply.Name().empty());
    EXPECT_EQ(reply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(reply.Signer());
    EXPECT_TRUE(reply.Terms().empty());
    EXPECT_EQ(reply.Type(), type);
    EXPECT_TRUE(reply.Validate());
    EXPECT_EQ(reply.Version(), replyVersion);

    const auto serializedReply = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(reply.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredReply =
        client_1_.Factory().PeerReply(serializedReply.Bytes()).asStoreSecret();

    ASSERT_TRUE(recoveredReply.IsValid());
    EXPECT_EQ(recoveredReply, reply);
    EXPECT_TRUE(recoveredReply.Alias().empty());
    EXPECT_TRUE(recoveredReply.Alias({}).empty());
    EXPECT_FALSE(recoveredReply.asBailment().IsValid());
    EXPECT_FALSE(recoveredReply.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredReply.asConnection().IsValid());
    EXPECT_FALSE(recoveredReply.asFaucet().IsValid());
    EXPECT_FALSE(recoveredReply.asFaucet().IsValid());

    {
        const auto& storesecret = recoveredReply.asStoreSecret();

        EXPECT_TRUE(storesecret.IsValid());
        EXPECT_TRUE(storesecret.Value());
    }

    EXPECT_FALSE(recoveredReply.asVerification().IsValid());
    EXPECT_EQ(recoveredReply.ID(), reply.ID());
    EXPECT_EQ(recoveredReply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(recoveredReply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(recoveredReply.Name().empty());
    EXPECT_EQ(recoveredReply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredReply.Signer());
    EXPECT_TRUE(recoveredReply.Terms().empty());
    EXPECT_EQ(recoveredReply.Type(), type);
    EXPECT_TRUE(recoveredReply.Validate());
    EXPECT_EQ(recoveredReply.Version(), replyVersion);
}

TEST_F(PeerRequests, verification)
{
    constexpr auto type = Verification;
    constexpr auto requestVersion = 4u;
    constexpr auto replyVersion = 4u;
    const auto request = client_1_.Factory().VerificationRequest(
        alex_.nym_, bob_.nym_id_, claim_, reason_);
    const auto& requestID = request.ID();

    EXPECT_TRUE(request.Alias().empty());
    EXPECT_TRUE(request.Alias({}).empty());
    EXPECT_FALSE(request.asBailment().IsValid());
    EXPECT_FALSE(request.asBailmentNotice().IsValid());
    EXPECT_FALSE(request.asConnection().IsValid());
    EXPECT_FALSE(request.asFaucet().IsValid());
    EXPECT_FALSE(request.asFaucet().asOutbailment());
    EXPECT_FALSE(request.asFaucet().asStoreSecret());

    {
        const auto& verification = request.asVerification();

        EXPECT_TRUE(verification.IsValid());
        EXPECT_EQ(verification.Claim(), claim_);
    }

    EXPECT_FALSE(requestID.empty());
    EXPECT_EQ(request.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(request.IsValid());
    EXPECT_TRUE(request.Name().empty());
    EXPECT_EQ(request.Responder(), bob_.nym_id_);
    EXPECT_TRUE(request.Signer());
    EXPECT_TRUE(request.Terms().empty());
    EXPECT_EQ(request.Type(), type);
    EXPECT_TRUE(request.Validate());
    EXPECT_EQ(request.Version(), requestVersion);

    const auto serializedRequest = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(request.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredRequest = client_1_.Factory()
                                      .PeerRequest(serializedRequest.Bytes())
                                      .asVerification();

    ASSERT_TRUE(recoveredRequest.IsValid());
    EXPECT_EQ(recoveredRequest, request);
    EXPECT_TRUE(recoveredRequest.Alias().empty());
    EXPECT_TRUE(recoveredRequest.Alias({}).empty());
    EXPECT_FALSE(recoveredRequest.asBailment().IsValid());
    EXPECT_FALSE(recoveredRequest.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredRequest.asConnection().IsValid());
    EXPECT_FALSE(recoveredRequest.asFaucet().IsValid());
    EXPECT_FALSE(recoveredRequest.asFaucet().asOutbailment());
    EXPECT_FALSE(recoveredRequest.asFaucet().asStoreSecret());

    {
        const auto& verification = recoveredRequest.asVerification();

        EXPECT_TRUE(verification.IsValid());
        EXPECT_EQ(verification.Claim(), claim_);
    }

    EXPECT_EQ(recoveredRequest.ID(), request.ID());
    EXPECT_EQ(recoveredRequest.Initiator(), alex_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Name().empty());
    EXPECT_EQ(recoveredRequest.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredRequest.Signer());
    EXPECT_TRUE(recoveredRequest.Terms().empty());
    EXPECT_EQ(recoveredRequest.Type(), type);
    EXPECT_TRUE(recoveredRequest.Validate());
    EXPECT_EQ(recoveredRequest.Version(), requestVersion);

    const auto reply = client_1_.Factory().VerificationReply(
        bob_.nym_, alex_.nym_id_, requestID, verification_, reason_);

    EXPECT_TRUE(reply.Alias().empty());
    EXPECT_TRUE(reply.Alias({}).empty());
    EXPECT_FALSE(reply.asBailment().IsValid());
    EXPECT_FALSE(reply.asBailmentNotice().IsValid());
    EXPECT_FALSE(reply.asConnection().IsValid());
    EXPECT_FALSE(reply.asFaucet().IsValid());
    EXPECT_FALSE(reply.asFaucet().asOutbailment());
    EXPECT_FALSE(reply.asFaucet().asStoreSecret());

    {
        const auto& verification = reply.asVerification();

        EXPECT_TRUE(verification.IsValid());
        EXPECT_TRUE(verification.Accepted());
        ASSERT_TRUE(verification.Response().has_value());
        EXPECT_EQ(*verification.Response(), verification_);
    }

    EXPECT_FALSE(reply.ID().empty());
    EXPECT_EQ(reply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(reply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(reply.IsValid());
    EXPECT_TRUE(reply.Name().empty());
    EXPECT_EQ(reply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(reply.Signer());
    EXPECT_TRUE(reply.Terms().empty());
    EXPECT_EQ(reply.Type(), type);
    EXPECT_TRUE(reply.Validate());
    EXPECT_EQ(reply.Version(), replyVersion);

    const auto serializedReply = [&] {
        auto out = opentxs::ByteArray{};

        EXPECT_TRUE(reply.Serialize(out.WriteInto()));

        return out;
    }();
    const auto recoveredReply =
        client_1_.Factory().PeerReply(serializedReply.Bytes()).asVerification();

    ASSERT_TRUE(recoveredReply.IsValid());
    EXPECT_EQ(recoveredReply, reply);
    EXPECT_TRUE(recoveredReply.Alias().empty());
    EXPECT_TRUE(recoveredReply.Alias({}).empty());
    EXPECT_FALSE(recoveredReply.asBailment().IsValid());
    EXPECT_FALSE(recoveredReply.asBailmentNotice().IsValid());
    EXPECT_FALSE(recoveredReply.asConnection().IsValid());
    EXPECT_FALSE(recoveredReply.asFaucet().IsValid());
    EXPECT_FALSE(recoveredReply.asFaucet().IsValid());
    EXPECT_FALSE(recoveredReply.asFaucet().asStoreSecret());

    {
        const auto& verification = recoveredReply.asVerification();

        EXPECT_TRUE(verification.IsValid());
        EXPECT_TRUE(verification.Accepted());
        ASSERT_TRUE(verification.Response().has_value());
        EXPECT_EQ(*verification.Response(), verification_);
    }

    EXPECT_EQ(recoveredReply.ID(), reply.ID());
    EXPECT_EQ(recoveredReply.Initiator(), alex_.nym_id_);
    EXPECT_EQ(recoveredReply.InReferenceToRequest(), requestID);
    EXPECT_TRUE(recoveredReply.Name().empty());
    EXPECT_EQ(recoveredReply.Responder(), bob_.nym_id_);
    EXPECT_TRUE(recoveredReply.Signer());
    EXPECT_TRUE(recoveredReply.Terms().empty());
    EXPECT_EQ(recoveredReply.Type(), type);
    EXPECT_TRUE(recoveredReply.Validate());
    EXPECT_EQ(recoveredReply.Version(), replyVersion);
}
}  // namespace ottest
