// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "blockchain/node/stats/Data.hpp"  // IWYU pragma: associated

#include "internal/blockchain/node/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
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
    OT_ASSERT(false == to_actor_.has_value());

    using Type = network::zeromq::socket::Type;
    auto& socket = to_actor_.emplace(
        api.Network().ZeroMQ().Internal().RawSocket(Type::Push));
    const auto rc = socket.Bind(endpoint.data());

    OT_ASSERT(rc);
}

auto Data::Trigger() const noexcept -> void
{
    if (to_actor_.has_value()) {
        to_actor_->SendDeferred(
            MakeWork(StatsJobs::statemachine), __FILE__, __LINE__);
    }
}

Data::~Data() = default;
}  // namespace opentxs::blockchain::node::stats
