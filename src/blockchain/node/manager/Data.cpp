// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/manager/Data.hpp"  // IWYU pragma: associated

#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::manager
{
Data::Data(const api::Session& api, const node::Endpoints& endpoints) noexcept
    : to_actor_([&] {
        using enum network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Push);
        const auto rc = out.Connect(endpoints.manager_pull_.c_str());

        assert_true(rc);

        return out;
    }())
    , to_peer_manager_([&] {
        using enum network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Push);
        const auto rc = out.Connect(endpoints.peer_manager_pull_.c_str());

        assert_true(rc);

        return out;
    }())
    , to_dht_([&] {
        using enum network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Push);
        const auto rc = out.Connect(endpoints.otdht_pull_.c_str());

        assert_true(rc);

        return out;
    }())
    , self_()
{
}

Data::~Data() = default;
}  // namespace opentxs::blockchain::node::manager
