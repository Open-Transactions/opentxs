// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cs_shared_guarded.h>
#include <cstddef>
#include <memory>
#include <shared_mutex>

#include "blockchain/node/stats/Data.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::stats
{
class Shared
{
public:
    const CString endpoint_;

    auto BlockHeaderTip(Type chain) const noexcept -> block::Position;
    auto BlockTip(Type chain) const noexcept -> block::Position;
    auto CfilterTip(Type chain) const noexcept -> block::Position;
    auto PeerCount(Type chain) const noexcept -> std::size_t;
    auto SyncTip(Type chain) const noexcept -> block::Position;

    auto SetBlockHeaderTip(Type chain, block::Position tip) noexcept -> void;
    auto SetBlockTip(Type chain, block::Position tip) noexcept -> void;
    auto SetCfilterTip(Type chain, block::Position tip) noexcept -> void;
    auto SetPeerCount(Type chain, std::size_t count) noexcept -> void;
    auto SetSyncTip(Type chain, block::Position tip) noexcept -> void;
    auto Start(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<Shared> me) noexcept -> void;

    Shared() noexcept;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared();

private:
    using GuardedData = libguarded::shared_guarded<Data, std::shared_mutex>;

    GuardedData data_;

    static auto get_position(
        const Data& data,
        const Data::PositionMap& map,
        Type chain) noexcept -> block::Position;
    static auto set_position(
        Data::PositionMap& map,
        Type chain,
        block::Position tip) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::stats
