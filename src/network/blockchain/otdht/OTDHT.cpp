// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/OTDHT.hpp"  // IWYU pragma: associated

#include "internal/network/zeromq/Context.hpp"
#include "network/blockchain/otdht/Actor.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::network::blockchain
{
OTDHT::OTDHT(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> node) noexcept
{
    assert_false(nullptr == api);
    assert_false(nullptr == node);

    const auto& zmq = api->Network().ZeroMQ().Context().Internal();
    const auto batchID = zmq.PreallocateBatch();
    Actor::Factory(api, node, batchID);
}

auto OTDHT::Init() noexcept -> void
{
    // NOTE this function intentionally left blank
}

OTDHT::~OTDHT() = default;
}  // namespace opentxs::network::blockchain
