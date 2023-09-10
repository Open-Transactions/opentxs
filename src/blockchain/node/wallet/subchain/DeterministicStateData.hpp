// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_ordered_guarded.h>
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
template <class T>
class shared_ptr;
}  // namespace boost

namespace opentxs
{
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

namespace crypto
{
class Deterministic;
}  // namespace crypto

namespace node
{
namespace wallet
{
class Reorg;
}  // namespace wallet

class Manager;
}  // namespace node
}  // namespace blockchain
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class DeterministicStateData final : public SubchainStateData
{
public:
    const crypto::Deterministic& deterministic_;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    DeterministicStateData(
        Reorg& reorg,
        const crypto::Deterministic& subaccount,
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        crypto::Subchain subchain,
        std::string_view fromParent,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    DeterministicStateData() = delete;
    DeterministicStateData(const DeterministicStateData&) = delete;
    DeterministicStateData(DeterministicStateData&&) = delete;
    auto operator=(const DeterministicStateData&)
        -> DeterministicStateData& = delete;
    auto operator=(DeterministicStateData&&)
        -> DeterministicStateData& = delete;

    ~DeterministicStateData() final;

private:
    using CacheData = std::pair<Time, database::BatchedMatches>;
    using Cache = libguarded::ordered_guarded<CacheData, std::shared_mutex>;

    mutable Cache cache_;

    auto CheckCache(const std::size_t outstanding, FinishedCallback cb)
        const noexcept -> void final;
    auto flush_cache(database::BatchedMatches& matches, FinishedCallback cb)
        const noexcept -> bool;

    auto get_index(const boost::shared_ptr<const SubchainStateData>& me)
        const noexcept -> void final;
    auto handle_confirmed_matches(
        const block::Block& block,
        const block::Position& position,
        const block::Matches& confirmed,
        const Log& log,
        allocator_type monotonic) const noexcept -> void final;
    auto handle_mempool_matches(
        const block::Matches& matches,
        block::Transaction tx,
        allocator_type monotonic) const noexcept -> void final;
    auto process(
        const block::Match match,
        block::Transaction tx,
        database::MatchedTransaction& matched,
        allocator_type monotonic) const noexcept -> void;
};
}  // namespace opentxs::blockchain::node::wallet
