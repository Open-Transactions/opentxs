// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/stats/Data.hpp"  // IWYU pragma: associated

#include "internal/blockchain/node/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::stats
{
Data::Data() noexcept
    : header_tips_()
    , block_tips_()
    , cfilter_tips_()
    , sync_tips_()
    , peer_count_()
    , to_actor_(std::nullopt)
{
}

auto Data::Init(const api::Session& api, std::string_view endpoint) noexcept
    -> void
{
    assert_false(to_actor_.has_value());

    using Type = network::zeromq::socket::Type;
    auto& socket = to_actor_.emplace(
        api.Network().ZeroMQ().Context().Internal().RawSocket(Type::Push));
    const auto rc = socket.Bind(endpoint.data());

    assert_true(rc);
}

auto Data::Trigger() const noexcept -> void
{
    if (to_actor_.has_value()) {
        to_actor_->SendDeferred(MakeWork(StatsJobs::statemachine));
    }
}

Data::~Data() = default;
}  // namespace opentxs::blockchain::node::stats
