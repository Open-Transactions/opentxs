// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/manager/SendPromises.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::node::manager
{
SendPromises::SendPromises() noexcept
    : counter_(-1)
    , map_()
{
}

auto SendPromises::finish(int index) noexcept -> std::promise<SendOutcome>
{
    auto it = map_.find(index);

    OT_ASSERT(map_.end() != it);

    auto output{std::move(it->second)};
    map_.erase(it);

    return output;
}

auto SendPromises::get() noexcept -> std::pair<int, Manager::PendingOutgoing>
{
    const auto counter = ++counter_;
    auto& promise = map_[counter];

    return std::make_pair(counter, promise.get_future());
}
}  // namespace opentxs::blockchain::node::manager
