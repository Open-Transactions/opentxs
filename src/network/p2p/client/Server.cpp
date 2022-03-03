// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "network/p2p/client/Server.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <iterator>
#include <utility>

#include "opentxs/network/p2p/State.hpp"

namespace opentxs::network::p2p::client
{
Server::Server(const std::string_view endpoint) noexcept
    : endpoint_(endpoint)
    , last_sent_()
    , last_received_()
    , connected_(false)
    , active_(false)
    , waiting_(false)
    , new_local_handler_(false)
    , publisher_()
    , chains_()
{
}

Server::Server() noexcept
    : Server(std::string_view{})
{
}

auto Server::Chains() const noexcept -> Vector<Chain>
{
    auto out = Vector<Chain>{};
    out.reserve(chains_.size());
    std::transform(
        chains_.begin(),
        chains_.end(),
        std::back_inserter(out),
        [](const auto& value) { return value.first; });

    return out;
}

auto Server::is_stalled() const noexcept -> bool
{
    static constexpr auto limit = std::chrono::minutes{5};
    const auto wait = Clock::now() - last_received_;

    return (wait >= limit);
}

auto Server::needs_query() noexcept -> bool
{
    if (new_local_handler_) {
        new_local_handler_ = false;

        return true;
    }

    static constexpr auto zero = 0s;
    static constexpr auto timeout = 30s;
    static constexpr auto keepalive = std::chrono::minutes{2};
    const auto wait = Clock::now() - last_received_;

    if (zero > wait) { return true; }

    return (waiting_ || (!active_)) ? (wait >= timeout) : (wait >= keepalive);
}

auto Server::needs_retry() noexcept -> bool
{
    static constexpr auto interval = std::chrono::minutes{2};
    const auto elapsed = Clock::now() - last_sent_;

    return (elapsed > interval);
}

auto Server::ProcessState(const opentxs::network::p2p::State& state) noexcept
    -> Height
{
    const auto chain = state.Chain();

    if (auto it = chains_.find(chain); it != chains_.end()) {
        it->second = state.Position();

        return it->second.first;
    } else {
        auto [i, added] = chains_.try_emplace(chain, state.Position());

        return i->second.first;
    }
}

auto Server::SetStalled() noexcept -> void
{
    active_ = false;
    connected_ = false;
    chains_.clear();
}
}  // namespace opentxs::network::p2p::client
