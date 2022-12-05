// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/node/headeroracle/HeaderOraclePrivate.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Header.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::internal
{
HeaderOraclePrivate::HeaderOraclePrivate(
    const api::Session& api,
    const node::Manager& node) noexcept
    : api_(api)
    , chain_(node.Internal().Chain())
    , endpoint_(node.Internal().Endpoints().header_oracle_pull_)
    , checkpoint_height_(params::get(chain_).CheckpointPosition().height_)
    , database_(node.Internal().DB())
    , to_parent_([&] {
        using Type = network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto rc =
            out.Connect(node.Internal().Endpoints().manager_pull_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , to_actor_([&] {
        using Type = network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto rc = out.Connect(
            node.Internal().Endpoints().header_oracle_pull_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , best_(database_.CurrentBest()->Position())
    , have_outstanding_job_(false)
    , unknown_hashes_()
    , unknown_hash_index_()                           // TODO allocator
    , highest_peer_height_(checkpoint_height_)        // TODO allocator
    , effective_remote_height_(highest_peer_height_)  // TODO allocator
{
}

auto HeaderOraclePrivate::AddUnknownHash(const block::Hash& hash) noexcept
    -> void
{
    auto& map = unknown_hashes_;
    auto& index = unknown_hash_index_;
    const auto& best = best_.height_;
    auto& remote = effective_remote_height_;
    map[best].emplace(hash);
    index[hash] = best;
    remote = max_height_;
}

auto HeaderOraclePrivate::Genesis(blockchain::Type chain) noexcept
    -> const block::Position&
{
    static const auto map = [] {
        auto out = Map<blockchain::Type, block::Position>{};

        for (const auto chain : SupportedChains()) {
            out.try_emplace(chain, 0, params::get(chain).GenesisHash());
        }

        return out;
    }();

    try {

        return map.at(chain);
    } catch (...) {
        LogAbort()(OT_PRETTY_STATIC(HeaderOraclePrivate))("invalid chain ")(
            print(chain))
            .Abort();
    }
}

auto HeaderOraclePrivate::Genesis() const noexcept -> const block::Position&
{
    return Genesis(chain_);
}

auto HeaderOraclePrivate::IsSynchronized() const noexcept -> bool
{
    return best_.height_ >= effective_remote_height_;
}

auto HeaderOraclePrivate::JobIsAvailable() const noexcept -> bool
{
    if (have_outstanding_job_ || IsSynchronized()) {

        return false;
    } else {

        return true;
    }
}

auto HeaderOraclePrivate::PruneKnownHashes() noexcept -> void
{
    auto& map = unknown_hashes_;
    auto& index = unknown_hash_index_;

    for (auto i = index.begin(); index.end() != i;) {
        const auto& hash = i->first;

        if (database_.HeaderExists(hash)) {
            if (auto m = map.find(i->second); map.end() != m) {
                auto& set = m->second;
                set.erase(hash);

                if (set.empty()) { map.erase(m); }
            } else {
                OT_FAIL;
            }

            i = index.erase(i);
        } else {
            ++i;
        }
    }

    prune_unknown();

    if (index.empty() && (max_height_ == effective_remote_height_)) {
        effective_remote_height_ = checkpoint_height_;
    }
}

auto HeaderOraclePrivate::prune_unknown() noexcept -> void
{
    constexpr auto tolerance = block::Height{2};
    const auto target = best_.height_ + tolerance;
    auto& map = unknown_hashes_;
    auto& index = unknown_hash_index_;
    const auto start = map.begin();
    const auto stop = map.upper_bound(target);

    for (auto i = start; i != stop; ++i) {
        for (const auto& hash : i->second) { index.erase(hash); }
    }

    map.erase(start, stop);
}

auto HeaderOraclePrivate::Target() const noexcept -> block::Height
{
    const auto& best = best_.height_;
    const auto& peer = highest_peer_height_;
    const auto remote = [&] {
        auto value = Remote();

        if (HeaderOraclePrivate::max_height_ == value) {

            return std::max(best + 1, peer);
        } else {

            return value;
        }
    }();

    return std::max(best, remote);
}

auto HeaderOraclePrivate::UpdateRemoteHeight(block::Height value) noexcept
    -> bool
{
    auto& peer = highest_peer_height_;
    auto& effective = effective_remote_height_;
    const auto before{effective};
    peer = std::max(peer, value);

    if (max_height_ == before) { effective = -1; }

    effective = std::max(effective, peer);

    return value != before;
}

HeaderOraclePrivate::~HeaderOraclePrivate() = default;
}  // namespace opentxs::blockchain::node::internal
