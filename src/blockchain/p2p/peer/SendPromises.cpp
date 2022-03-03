// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/p2p/peer/Peer.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::p2p::implementation
{
Peer::SendPromises::SendPromises() noexcept
    : lock_()
    , counter_(0)
    , map_()
{
}

auto Peer::SendPromises::Break() -> void
{
    Lock lock(lock_);

    for (auto& [id, promise] : map_) { promise = {}; }
}

auto Peer::SendPromises::NewPromise() -> std::pair<std::future<bool>, int>
{
    Lock lock(lock_);
    auto [it, added] = map_.emplace(++counter_, std::promise<bool>());

    if (false == added) { return {}; }

    return {it->second.get_future(), it->first};
}

auto Peer::SendPromises::SetPromise(const int promise, const bool value) -> void
{
    Lock lock(lock_);

    auto it = map_.find(promise);

    if (map_.end() != it) {
        try {
            it->second.set_value(value);
        } catch (...) {
        }

        map_.erase(it);
    }
}
}  // namespace opentxs::blockchain::p2p::implementation
