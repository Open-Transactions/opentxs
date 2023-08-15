// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/container_fwd.hpp>  // IWYU pragma: keep
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>  // IWYU pragma: keep
#include <iterator>    // IWYU pragma: keep
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/GCS.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Outpoint;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class SubchainStateData::PrehashData
{
public:
    const std::size_t job_count_;

    auto Match(
        const std::string_view procedure,
        const Log& log,
        const Vector<cfilter::GCS>& cfilters,
        std::atomic_bool& atLeastOnce,
        wallet::MatchCache::Results& results,
        MatchResults& matched,
        alloc::Default monotonic) noexcept -> void;
    auto Prepare() noexcept -> void;

    PrehashData(
        const api::Session& api,
        const BlockTargets& targets,
        const std::string_view name,
        wallet::MatchCache::Results& results,
        block::Height start,
        std::size_t jobs,
        allocator_type alloc) noexcept;

private:
    using Hash = std::uint64_t;
    using Hashes = Vector<Hash>;
    using ElementHashMap = Map<Hash, Vector<const Bip32Index*>>;
    using TxoHashMap = Map<Hash, Vector<const block::Outpoint*>>;
    using ElementData = std::pair<Hashes, ElementHashMap>;
    using TxoData = std::pair<Hashes, TxoHashMap>;
    using BlockData = std::tuple<
        block::Height,
        ElementData,  // 20 byte
        ElementData,  // 32 byte
        ElementData,  // 33 byte
        ElementData,  // 64 byte
        ElementData,  // 65 byte
        TxoData>;
    using Data = Vector<BlockData>;

    const api::Session& api_;
    const BlockTargets& targets_;
    const std::string_view name_;
    Data data_;

    auto hash(const BlockTarget& target, BlockData& row) noexcept -> void;
    template <typename Input, typename Output>
    auto hash(
        const block::Hash& block,
        const std::pair<Vector<Input>, Targets>& targets,
        Output& dest) noexcept -> void;
    auto match(
        const std::string_view procedure,
        const Log& log,
        const Vector<cfilter::GCS>& cfilters,
        std::atomic_bool& atLeastOnce,
        const std::size_t job,
        wallet::MatchCache::Results& results,
        MatchResults& matched,
        alloc::Default monotonic) noexcept -> void;
    auto match(
        const std::string_view procedure,
        const Log& log,
        const block::Position& position,
        const cfilter::GCS& cfilter,
        const BlockTarget& targets,
        const BlockData& prehashed,
        AsyncResults& cache,
        wallet::MatchCache::Index& results,
        alloc::Default monotonic) const noexcept -> void;
    auto prepare(const std::size_t job) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::wallet
