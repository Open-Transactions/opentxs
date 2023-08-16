// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Notary

#include "ottest/fixtures/otx/broken/Pair.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <future>
#include <span>
#include <utility>

#include "internal/api/session/Endpoints.hpp"
#include "internal/core/contract/peer/PairEvent.hpp"
#include "internal/core/contract/peer/PairEventType.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/util/LogMacros.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"

namespace ottest
{
namespace ot = opentxs;

ot::identifier::UnitDefinition Pair::unit_id_{};
Callbacks Pair::cb_chris_{OTTestEnvironment::GetOT(), chris_.name_};
const ot::UnallocatedCString Issuer::new_notary_name_{"Chris's Notary"};
Issuer Pair::issuer_data_{};

Pair::Pair()
    : api_issuer_(OTTestEnvironment::GetOT().StartClientSession(0))
    , api_chris_(OTTestEnvironment::GetOT().StartClientSession(1))
    , api_server_1_(OTTestEnvironment::GetOT().StartNotarySession(0))
    , issuer_peer_request_cb_(ot::network::zeromq::ListenCallback::Factory(
          [this](auto&& in) { issuer_peer_request(std::move(in)); }))
    , chris_rename_notary_cb_(ot::network::zeromq::ListenCallback::Factory(
          [this](auto&& in) { chris_rename_notary(std::move(in)); }))
    , issuer_peer_request_listener_(
          api_issuer_.Network().ZeroMQ().Internal().SubscribeSocket(
              issuer_peer_request_cb_))
    , chris_rename_notary_listener_(
          api_chris_.Network().ZeroMQ().Internal().SubscribeSocket(
              chris_rename_notary_cb_))
{
    subscribe_sockets();

    const_cast<Server&>(server_1_).init(api_server_1_);
    const_cast<User&>(issuer_).init(api_issuer_, server_1_);
    const_cast<User&>(chris_).init(api_chris_, server_1_);
}

void Pair::subscribe_sockets()
{
    ASSERT_TRUE(issuer_peer_request_listener_->Start(ot::UnallocatedCString{
        api_issuer_.Endpoints().Internal().PeerRequestUpdate()}));
    ASSERT_TRUE(chris_rename_notary_listener_->Start(
        ot::UnallocatedCString{api_chris_.Endpoints().Internal().PairEvent()}));
}

void Pair::chris_rename_notary(ot::network::zeromq::Message&& in)
{
    const auto body = in.Payload();

    EXPECT_EQ(1, body.size());

    if (1 != body.size()) { return; }

    const auto event = ot::contract::peer::internal::PairEvent(body[0].Bytes());
    EXPECT_EQ(1, event.version_);
    EXPECT_EQ(ot::contract::peer::internal::PairEventType::Rename, event.type_);
    EXPECT_EQ(issuer_.nym_id_.asBase58(api_issuer_.Crypto()), event.issuer_);
    EXPECT_TRUE(api_chris_.Wallet().SetServerAlias(
        server_1_.id_, issuer_data_.new_notary_name_));

    const auto result = api_chris_.OTX().DownloadNym(
        chris_.nym_id_, server_1_.id_, issuer_.nym_id_);

    EXPECT_NE(0, result.first);

    if (0 == result.first) { return; }
}

void Pair::issuer_peer_request(ot::network::zeromq::Message&& in)
{
    const auto body = in.Payload();

    EXPECT_EQ(2, body.size());

    if (2 != body.size()) { return; }

    EXPECT_EQ(issuer_.nym_id_.asBase58(api_issuer_.Crypto()), body[0].Bytes());

    const auto request = api_chris_.Factory().PeerRequest(body[1]);

    EXPECT_EQ(
        body[0].Bytes(), request.Responder().asBase58(api_issuer_.Crypto()));

    switch (request.Type()) {
        case ot::contract::peer::RequestType::Bailment: {
            const auto& bailment = request.asBailment();

            EXPECT_EQ(bailment.Notary(), server_1_.id_);
            EXPECT_EQ(bailment.Unit(), unit_id_);

            api_issuer_.OTX().AcknowledgeBailment(
                issuer_.nym_id_,
                bailment.Initiator(),
                bailment.ID(),
                std::to_string(++issuer_data_.bailment_counter_));

            if (issuer_data_.expected_bailments_ ==
                issuer_data_.bailment_counter_) {
                issuer_data_.bailment_promise_.set_value(true);
            }
        } break;
        case ot::contract::peer::RequestType::StoreSecret: {
            // TODO
        } break;
        case ot::contract::peer::RequestType::ConnectionInfo: {
            // TODO
        } break;
        default: {
            OT_FAIL;
        }
    }
}
}  // namespace ottest
