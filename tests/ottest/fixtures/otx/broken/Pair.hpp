// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Notary

#pragma once

#include <opentxs/opentxs.hpp>

#include "internal/core/contract/peer/PairEventType.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT Pair : public IntegrationFixture
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

    Pair();
    void subscribe_sockets();
    void chris_rename_notary(ot::network::zeromq::Message&& in);
    void issuer_peer_request(ot::network::zeromq::Message&& in);
};
}  // namespace ottest
