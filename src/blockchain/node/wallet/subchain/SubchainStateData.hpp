// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_ordered_guarded.h>
#include <cs_plain_guarded.h>
#include <cs_shared_guarded.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/ReorgSlave.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/Subchain.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Actor.hpp"
#include "util/JobCounter.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Element;
class Subaccount;
}  // namespace crypto

namespace database
{
class Wallet;
}  // namespace database

namespace node
{
namespace internal
{
class Mempool;
struct HeaderOraclePrivate;
}  // namespace internal

namespace wallet
{
namespace statemachine
{
class Job;
}  // namespace statemachine

class ScriptForm;
}  // namespace wallet

class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class SubchainStateData
    : virtual public Subchain,
      public opentxs::Actor<SubchainStateData, SubchainJobs>,
      public boost::enable_shared_from_this<SubchainStateData>
{
private:
    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;

public:
    using ElementCache =
        libguarded::shared_guarded<wallet::ElementCache, std::shared_mutex>;
    using MatchCache =
        libguarded::shared_guarded<wallet::MatchCache, std::shared_mutex>;
    using ProgressPosition =
        libguarded::plain_guarded<std::optional<block::Position>>;
    using FinishedCallback =
        std::function<void(const Vector<block::Position>&)>;
    using State = JobState;

    const api::Session& api_;
    const node::Manager& node_;
    database::Wallet& db_;
    const node::internal::Mempool& mempool_oracle_;
    const crypto::Subaccount& subaccount_;
    const identifier::Nym owner_;
    const crypto::SubaccountType account_type_;
    const identifier::Account id_;
    const crypto::Subchain subchain_;
    const Type chain_;
    const cfilter::Type filter_type_;
    const block::SubchainID db_key_;
    const block::Position null_position_;
    const block::Position genesis_;
    const CString from_ssd_endpoint_;
    const CString to_ssd_endpoint_;
    const CString to_index_endpoint_;
    const CString to_scan_endpoint_;
    const CString to_rescan_endpoint_;
    const CString to_process_endpoint_;
    const CString to_progress_endpoint_;
    const CString from_parent_;
    const block::Height scan_threshold_;
    const std::size_t maximum_scan_;
    mutable ElementCache element_cache_;
    mutable MatchCache match_cache_;
    mutable std::atomic_bool scan_dirty_;
    mutable std::atomic_bool need_reorg_;
    mutable std::atomic<std::size_t> process_queue_;
    mutable ProgressPosition progress_position_;

    virtual auto CheckCache(const std::size_t outstanding, FinishedCallback cb)
        const noexcept -> void = 0;
    auto FinishRescan() const noexcept -> block::Height
    {
        return scan_threshold_ + maximum_scan_;
    }
    auto GetReorg() const noexcept -> wallet::Reorg& { return reorg_; }
    auto IndexElement(
        const cfilter::Type type,
        const blockchain::crypto::Element& input,
        const Bip32Index index,
        database::ElementMap& output) const noexcept -> void;
    auto ProcessBlock(
        const block::Position& position,
        const block::Block& block,
        allocator_type monotonic) const noexcept -> bool;
    auto ProcessTransaction(
        const block::Transaction& tx,
        const Log& log,
        allocator_type monotonic) const noexcept -> void;
    auto ReorgTarget(
        const block::Position& reorg,
        const block::Position& current) const noexcept -> block::Position;
    auto ReportScan(const block::Position& pos) const noexcept -> void;
    auto Rescan(
        const block::Position best,
        const block::Height stop,
        block::Position& highestTested,
        Vector<ScanStatus>& out,
        allocator_type monotonic) const noexcept
        -> std::optional<block::Position>;
    auto RescanFinished() const noexcept -> void;
    auto Scan(
        const block::Position best,
        const block::Height stop,
        block::Position& highestTested,
        Vector<ScanStatus>& out,
        allocator_type monotonic) const noexcept
        -> std::optional<block::Position>;
    auto TriggerRescan() const noexcept -> void;

    auto Init(boost::shared_ptr<SubchainStateData> me) noexcept -> void final;

    SubchainStateData() = delete;
    SubchainStateData(const SubchainStateData&) = delete;
    SubchainStateData(SubchainStateData&&) = delete;
    auto operator=(const SubchainStateData&) -> SubchainStateData& = delete;
    auto operator=(SubchainStateData&&) -> SubchainStateData& = delete;

    ~SubchainStateData() override;

protected:
    using TXOs = database::TXOs;
    auto set_key_data(block::Transaction& tx, allocator_type monotonic)
        const noexcept -> void;

    virtual auto do_startup(allocator_type monotonic) noexcept -> bool;
    virtual auto work(allocator_type monotonic) noexcept -> bool;

    SubchainStateData(
        Reorg& reorg,
        const crypto::Subaccount& subaccount,
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        crypto::Subchain subchain,
        std::string_view fromParent,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;

private:
    friend opentxs::Actor<SubchainStateData, SubchainJobs>;
    friend statemachine::Job;

    using Transactions = Vector<block::Transaction>;
    using Patterns = block::Patterns;
    using Targets = cfilter::Targets;
    using Tested = database::MatchingIndices;
    using Elements = wallet::ElementCache::Elements;
    using HandledReorgs = Set<StateSequence>;
    using SelectedKeyElement = std::pair<Vector<Bip32Index>, Targets>;
    using SelectedTxoElement = std::pair<Vector<block::Outpoint>, Targets>;
    using SelectedElements = std::tuple<
        SelectedKeyElement,  // 20 byte
        SelectedKeyElement,  // 32 byte
        SelectedKeyElement,  // 33 byte
        SelectedKeyElement,  // 64 byte
        SelectedKeyElement,  // 65 byte
        SelectedTxoElement>;
    using BlockTarget = std::pair<block::Hash, SelectedElements>;
    using BlockTargets = Vector<BlockTarget>;
    using BlockHashes = HeaderOracle::Hashes;
    using MatchesToTest = std::pair<Patterns, Patterns>;
    using Positions = Set<block::Position>;
    using FilterMap = Map<block::Height, std::uint32_t>;
    using AsyncResults = std::tuple<Positions, Positions, FilterMap>;
    using MatchResults =
        libguarded::ordered_guarded<AsyncResults, std::shared_mutex>;

    class PrehashData;

    static constexpr auto cfilter_size_window_ = 1000_uz;

    network::zeromq::socket::Raw& to_block_oracle_;
    network::zeromq::socket::Raw& to_children_;
    network::zeromq::socket::Raw& to_scan_;
    std::atomic<State> pending_state_;
    std::atomic<State> state_;
    mutable Deque<std::size_t> filter_sizes_;
    mutable std::atomic<std::size_t> elements_per_cfilter_;
    mutable JobCounter job_counter_;
    HandledReorgs reorgs_;
    Map<JobType, Time> child_activity_;
    Timer watchdog_;
    mutable ReorgSlave reorg_;
    mutable std::atomic<bool> rescan_pending_;

    static auto describe(
        const crypto::Subaccount& account,
        const crypto::Subchain subchain,
        allocator_type alloc) noexcept -> CString;
    static auto highest_clean(
        const AsyncResults& results,
        block::Position& highestTested) noexcept
        -> std::optional<block::Position>;

    auto choose_thread_count(std::size_t elements) const noexcept
        -> std::size_t;
    auto get_account_targets(const Elements& elements, alloc::Default alloc)
        const noexcept -> Targets;
    virtual auto get_index(const boost::shared_ptr<const SubchainStateData>& me)
        const noexcept -> void = 0;
    auto get_targets(const Elements& elements, Targets& targets) const noexcept
        -> void;
    auto get_targets(const TXOs& utxos, Targets& targets) const noexcept
        -> void;
    virtual auto handle_confirmed_matches(
        const block::Block& block,
        const block::Position& position,
        const block::Matches& confirmed,
        const Log& log,
        allocator_type monotonic) const noexcept -> void = 0;
    virtual auto handle_mempool_matches(
        const block::Matches& matches,
        block::Transaction tx,
        allocator_type monotonic) const noexcept -> void = 0;
    auto reorg_children() const noexcept -> std::size_t;
    auto supported_scripts(const crypto::Element& element) const noexcept
        -> UnallocatedVector<ScriptForm>;
    auto scan(
        const bool rescan,
        const block::Position best,
        const block::Height stop,
        block::Position& highestTested,
        Vector<ScanStatus>& out,
        allocator_type monotonic) const noexcept
        -> std::optional<block::Position>;
    auto scan(
        const Log& log,
        const Time start,
        const bool rescan,
        const block::Position& best,
        const block::Height stop,
        const block::Height startHeight,
        const std::string_view procedure,
        std::atomic_bool& atLeastOnce,
        std::optional<block::Position>& highestClean,
        block::Position& highestTested,
        wallet::MatchCache::Results& results,
        Vector<ScanStatus>& out,
        allocator_type monotonic) const noexcept(false) -> void;
    auto select_all(
        const block::Position& block,
        const Elements& in,
        MatchesToTest& matched) const noexcept -> void;
    auto select_matches(
        const std::optional<wallet::MatchCache::Index>& matches,
        const block::Position& block,
        const Elements& in,
        MatchesToTest& matched) const noexcept -> bool;
    auto select_targets(
        const wallet::ElementCache& cache,
        const BlockHashes& hashes,
        const Elements& in,
        block::Height height,
        BlockTargets& targets) const noexcept -> void;
    auto select_targets(
        const wallet::ElementCache& cache,
        const block::Position& block,
        const Elements& in,
        BlockTargets& targets) const noexcept -> void;
    auto start_rescan() const noexcept -> void;
    auto to_patterns(const Elements& in, allocator_type alloc) const noexcept
        -> Patterns;
    auto translate(const TXOs& utxos, allocator_type alloc) const noexcept
        -> Patterns;

    auto do_reorg(
        const node::HeaderOracle& oracle,
        const node::internal::HeaderOraclePrivate& data,
        Reorg::Params& params) noexcept -> bool;
    auto do_shutdown() noexcept -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto process_prepare_reorg(Message&& in) noexcept -> void;
    auto process_rescan(Message&& in) noexcept -> void;
    auto process_watchdog_ack(Message&& in) noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_pre_shutdown(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal() noexcept -> void;
    auto transition_state_pre_shutdown() noexcept -> void;
    auto transition_state_reorg(StateSequence id) noexcept -> void;

    SubchainStateData(
        Reorg& reorg,
        const crypto::Subaccount& subaccount,
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        crypto::Subchain subchain,
        network::zeromq::BatchID batch,
        CString&& fromParent,
        CString&& fromChildren,
        CString&& toChildren,
        CString&& toScan,
        CString&& toProgress,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
