// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"

#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_deferred_guarded.h>
#include <cs_shared_guarded.h>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/Subchain.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Process.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Progress.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Rescan.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Scan.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Actor.hpp"
#include "util/JobCounter.hpp"
#include "util/LMDB.hpp"

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
namespace bitcoin
{
class Block;
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace crypto
{
class Element;
class Subaccount;
}  // namespace crypto

namespace node
{
namespace internal
{
struct Mempool;
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
class Index;
class Job;
class ScriptForm;
class Work;
}  // namespace wallet
}  // namespace node

class GCS;
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class SubchainStateData
    : virtual public Subchain,
      public Actor<SubchainJobs>,
      public boost::enable_shared_from_this<SubchainStateData>
{
public:
    using WalletDatabase = node::internal::WalletDatabase;
    using SubchainIndex = WalletDatabase::pSubchainIndex;
    using ElementCache =
        libguarded::shared_guarded<wallet::ElementCache, std::shared_mutex>;
    using MatchCache =
        libguarded::shared_guarded<wallet::MatchCache, std::shared_mutex>;
    using FinishedCallback =
        std::function<void(const Vector<block::Position>&)>;

    const api::Session& api_;
    const node::internal::Network& node_;
    node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_oracle_;
    const crypto::Subaccount& subaccount_;
    const OTNymID owner_;
    const crypto::SubaccountType account_type_;
    const OTIdentifier id_;
    const crypto::Subchain subchain_;
    const Type chain_;
    const cfilter::Type filter_type_;
    const SubchainIndex db_key_;
    const block::Position null_position_;
    const block::Position genesis_;
    const CString from_ssd_endpoint_;
    const CString to_ssd_endpoint_;
    const CString to_index_endpoint_;
    const CString to_scan_endpoint_;
    const CString to_rescan_endpoint_;
    const CString to_process_endpoint_;
    const CString to_progress_endpoint_;
    const CString shutdown_endpoint_;
    const block::Height scan_threshold_;
    mutable ElementCache element_cache_;
    mutable MatchCache match_cache_;
    mutable std::atomic_bool scan_dirty_;
    mutable std::atomic<std::size_t> process_queue_;
    mutable std::atomic<block::Height> rescan_progress_;

    auto ChangeState(const State state, StateSequence reorg) noexcept
        -> bool final;
    virtual auto CheckCache(const std::size_t outstanding, FinishedCallback cb)
        const noexcept -> void = 0;
    auto IndexElement(
        const cfilter::Type type,
        const blockchain::crypto::Element& input,
        const Bip32Index index,
        WalletDatabase::ElementMap& output) const noexcept -> void;
    auto ProcessBlock(
        const block::Position& position,
        const block::bitcoin::Block& block) const noexcept -> bool;
    auto ProcessTransaction(
        const block::bitcoin::Transaction& tx) const noexcept -> void;
    auto ReportScan(const block::Position& pos) const noexcept -> void;
    auto Rescan(
        const block::Position best,
        const block::Height stop,
        block::Position& highestTested,
        Vector<ScanStatus>& out) const noexcept
        -> std::optional<block::Position>;
    auto Scan(
        const block::Position best,
        const block::Height stop,
        block::Position& highestTested,
        Vector<ScanStatus>& out) const noexcept
        -> std::optional<block::Position>;

    auto Init(boost::shared_ptr<SubchainStateData> me) noexcept -> void final;
    auto ProcessReorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& ancestor) noexcept -> void final;

    ~SubchainStateData() override;

protected:
    auto do_startup() noexcept -> void override;
    auto do_shutdown() noexcept -> void override;
    auto pipeline(const Work work, Message&& msg) noexcept -> void override;
    auto work() noexcept -> bool override;

    using TXOs = WalletDatabase::TXOs;
    auto set_key_data(block::bitcoin::Transaction& tx) const noexcept -> void;

    SubchainStateData(
        const api::Session& api,
        const node::internal::Network& node,
        node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const crypto::Subaccount& subaccount,
        const cfilter::Type filter,
        const crypto::Subchain subchain,
        const network::zeromq::BatchID batch,
        const std::string_view parent,
        allocator_type alloc) noexcept;


private:
    using Transactions =
        Vector<std::shared_ptr<const block::bitcoin::Transaction>>;
    using Task = node::internal::Wallet::Task;
    using Patterns = WalletDatabase::Patterns;
    using Targets = GCS::Targets;
    using Tested = WalletDatabase::MatchingIndices;
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
        libguarded::deferred_guarded<AsyncResults, std::shared_mutex>;

    class PrehashData;

    static constexpr auto cfilter_size_window_ = std::size_t{1000u};

    network::zeromq::socket::Raw& to_block_oracle_;
    network::zeromq::socket::Raw& to_children_;
    std::atomic<State> pending_state_;
    std::atomic<State> state_;
    mutable Deque<std::size_t> filter_sizes_;
    mutable std::atomic<std::size_t> elements_per_cfilter_;
    mutable JobCounter job_counter_;
    HandledReorgs reorgs_;
    std::optional<wallet::Progress> progress_;
    std::optional<wallet::Rescan> rescan_;
    std::optional<wallet::Index> index_;
    std::optional<wallet::Process> process_;
    std::optional<wallet::Scan> scan_;
    bool have_children_;
    Map<JobType, Time> child_activity_;
    Timer watchdog_;

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
    auto clear_children() noexcept -> void;
    auto get_account_targets(const Elements& elements, alloc::Resource* alloc)
        const noexcept -> Targets;
    virtual auto get_index(const boost::shared_ptr<const SubchainStateData>& me)
        const noexcept -> Index = 0;
    auto get_targets(const Elements& elements, Targets& targets) const noexcept
        -> void;
    auto get_targets(const TXOs& utxos, Targets& targets) const noexcept
        -> void;
    virtual auto handle_confirmed_matches(
        const block::bitcoin::Block& block,
        const block::Position& position,
        const block::Matches& confirmed) const noexcept -> void = 0;
    virtual auto handle_mempool_matches(
        const block::Matches& matches,
        std::unique_ptr<const block::bitcoin::Transaction> tx) const noexcept
        -> void = 0;
    auto reorg_children() const noexcept -> std::size_t;
    auto supported_scripts(const crypto::Element& element) const noexcept
        -> UnallocatedVector<ScriptForm>;
    auto scan(
        const bool rescan,
        const block::Position best,
        const block::Height stop,
        block::Position& highestTested,
        Vector<ScanStatus>& out) const noexcept
        -> std::optional<block::Position>;
    auto select_all(
        const block::Position& block,
        const Elements& in,
        MatchesToTest& out) const noexcept -> void;
    auto select_matches(
        const std::optional<wallet::MatchCache::Index>& matches,
        const block::Position& block,
        const Elements& in,
        MatchesToTest& out) const noexcept -> bool;
    auto select_targets(
        const wallet::ElementCache& cache,
        const BlockHashes& hashes,
        const Elements& in,
        block::Height height,
        BlockTargets& out) const noexcept -> void;
    auto select_targets(
        const wallet::ElementCache& cache,
        const block::Position& block,
        const Elements& in,
        BlockTargets& out) const noexcept -> void;
    auto to_patterns(const Elements& in, allocator_type alloc) const noexcept
        -> Patterns;
    auto translate(const TXOs& utxos, Patterns& outpoints) const noexcept
        -> void;

    auto do_reorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position ancestor) noexcept -> void;
    auto process_prepare_reorg(Message&& in) noexcept -> void;
    auto process_watchdog_ack(Message&& in) noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal() noexcept -> bool;
    auto transition_state_reorg(StateSequence id) noexcept -> bool;
    auto transition_state_shutdown() noexcept -> bool;

    SubchainStateData(
        const api::Session& api,
        const node::internal::Network& node,
        node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const crypto::Subaccount& subaccount,
        const cfilter::Type filter,
        const crypto::Subchain subchain,
        const network::zeromq::BatchID batch,
        const std::string_view parent,
        CString&& fromChildren,
        CString&& toChildren,
        CString&& toScan,
        CString&& toProgress,
        allocator_type alloc) noexcept;

    SubchainStateData() = delete;
    SubchainStateData(const SubchainStateData&) = delete;
    SubchainStateData(SubchainStateData&&) = delete;
    SubchainStateData& operator=(const SubchainStateData&) = delete;
    SubchainStateData& operator=(SubchainStateData&&) = delete;
};
}  // namespace opentxs::blockchain::node::wallet
