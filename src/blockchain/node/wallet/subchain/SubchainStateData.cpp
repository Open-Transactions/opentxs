// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <compare>
#include <future>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "blockchain/node/wallet/subchain/PrehashData.hpp"
#include "blockchain/node/wallet/subchain/ScriptForm.hpp"
#include "blockchain/node/wallet/subchain/statemachine/MatchIndex.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Matches.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Process.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Progress.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Rescan.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Scan.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Thread.hpp"
#include "internal/util/alloc/MonotonicSync.hpp"
#include "opentxs/Context.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/cfilter/GCS.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Work.hpp"

namespace opentxs
{
// https://baptiste-wicht.com/posts/2014/07/compile-integer-square-roots-at-compile-time-in-cpp.html
static constexpr auto isqrt(std::size_t x, std::size_t r, std::size_t i)
    -> std::size_t
{
    return i == 0 ? r
                  : isqrt(
                        x >= r + i ? x - (r + i) : x,
                        x >= r + i ? (r + 2_uz * i) >> 1_uz : r >> 1_uz,
                        i >> 2_uz);
}

static constexpr auto isqrt_i(std::size_t x, std::size_t i) -> std::size_t
{
    return i <= x ? i : isqrt_i(x, i >> 2_uz);
}

static constexpr auto isqrt(std::size_t x) -> std::size_t
{
    return isqrt(x, 0_uz, isqrt_i(x, 1_uz << ((sizeof(x) * 8_uz) - 2_uz)));
}
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
auto print(SubchainJobs in) noexcept -> std::string_view
{
    using namespace std::literals;
    using enum SubchainJobs;
    static constexpr auto map =
        frozen::make_unordered_map<SubchainJobs, std::string_view>({
            {shutdown, "shutdown"sv},
            {filter, "filter"sv},
            {mempool, "mempool"sv},
            {start_scan, "start_scan"sv},
            {prepare_reorg, "prepare_reorg"sv},
            {update, "update"sv},
            {process, "process"sv},
            {watchdog, "watchdog"sv},
            {watchdog_ack, "watchdog_ack"sv},
            {reprocess, "reprocess"sv},
            {rescan, "rescan"sv},
            {do_rescan, "do_rescan"sv},
            {finish_reorg, "finish_reorg"sv},
            {block, "block"sv},
            {init, "init"sv},
            {key, "key"sv},
            {prepare_shutdown, "prepare_shutdown"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid SubchainJobs: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

SubchainStateData::SubchainStateData(
    Reorg& reorg,
    crypto::Subaccount& subaccount,
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const node::Manager> node,
    crypto::Subchain subchain,
    network::zeromq::BatchID batch,
    CString&& fromParent,
    CString&& fromChildren,
    CString&& toChildren,
    CString&& toScan,
    CString&& toProgress,
    allocator_type alloc) noexcept
    : Actor(
          api->Self(),
          LogTrace(),
          describe(subaccount, subchain, alloc),
          0ms,
          batch,
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
              {fromParent, Connect},
          },
          {
              {fromChildren, Bind},
          },
          {},
          {
              {Dealer,
               Internal,
               {
                   {node->Internal().Endpoints().block_oracle_router_, Connect},
               }},
              {Publish,
               Internal,
               {
                   {toChildren, Bind},
               }},
              {Push,
               Internal,
               {
                   {toScan, Connect},
               }},
          })
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , api_(api_p_->Self())
    , node_(*node_p_)
    , db_(node_.Internal().DB())
    , mempool_oracle_(node_.Internal().Mempool())
    , subaccount_(subaccount)
    , owner_(subaccount_.Parent().NymID())
    , account_type_(subaccount_.Type())
    , id_(subaccount_.ID())
    , subchain_(subchain)
    , chain_(node_.Internal().Chain())
    , filter_type_(node_.FilterOracle().DefaultType())
    , db_key_(db_.GetSubchainID(id_, subchain_))
    , null_position_(block::Position{})
    , genesis_(node_.HeaderOracle().GetPosition(0))
    , from_ssd_endpoint_(std::move(toChildren))
    , to_ssd_endpoint_(std::move(fromChildren))
    , to_index_endpoint_(network::zeromq::MakeArbitraryInproc(alloc))
    , to_scan_endpoint_(std::move(toScan))
    , to_rescan_endpoint_(network::zeromq::MakeArbitraryInproc(alloc))
    , to_process_endpoint_(network::zeromq::MakeArbitraryInproc(alloc))
    , to_progress_endpoint_(std::move(toProgress))
    , from_parent_(std::move(fromParent))
    , scan_threshold_(1000)
    , maximum_scan_(2000_uz)
    , element_cache_(
          db_.GetPatterns(db_key_, alloc),
          db_.GetUnspentOutputs(id_, subchain_, alloc),
          alloc)
    , match_cache_(alloc)
    , scan_dirty_(false)
    , need_reorg_(false)
    , process_queue_(0_uz)
    , progress_position_(std::nullopt)
    , to_block_oracle_(pipeline_.Internal().ExtraSocket(0))
    , to_children_(pipeline_.Internal().ExtraSocket(1))
    , to_scan_(pipeline_.Internal().ExtraSocket(2))
    , pending_state_(State::normal)
    , state_(State::normal)
    , filter_sizes_(alloc)
    , elements_per_cfilter_(0_uz)
    , job_counter_()
    , reorgs_(alloc)
    , child_activity_({
          {JobType::scan, {}},
          {JobType::process, {}},
          {JobType::index, {}},
          {JobType::rescan, {}},
          {JobType::progress, {}},
      })
    , watchdog_(api_.Network().Asio().Internal().GetTimer())
    , reorg_(reorg.GetSlave(pipeline_, name_, alloc))
    , rescan_pending_(false)
{
    assert_false(owner_.empty());
    assert_false(id_.empty());
}

SubchainStateData::SubchainStateData(
    Reorg& reorg,
    crypto::Subaccount& subaccount,
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const node::Manager> node,
    crypto::Subchain subchain,
    std::string_view fromParent,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : SubchainStateData(
          reorg,
          subaccount,
          std::move(api),
          std::move(node),
          std::move(subchain),
          std::move(batch),
          CString{fromParent, alloc},
          network::zeromq::MakeArbitraryInproc(alloc),
          network::zeromq::MakeArbitraryInproc(alloc),
          network::zeromq::MakeArbitraryInproc(alloc),
          network::zeromq::MakeArbitraryInproc(alloc),
          alloc)
{
}

auto SubchainStateData::choose_thread_count(std::size_t elements) const noexcept
    -> std::size_t
{
    // NOTE the target thread count is the square root of the number of
    // elements divided by 512. The minimum value is one and the maximum
    // value is one less than the number of hardware threads.
    static constexpr auto calc = [](auto num, std::size_t hardware) {
        const auto limit = std::max(isqrt(num >> 9_uz), 1_uz);

        return std::min(limit, std::max(hardware, 2_uz) - 1_uz);
    };

    static_assert(isqrt(0) == 0);
    static_assert(isqrt(1) == 1);
    static_assert(isqrt(3) == 1);
    static_assert(isqrt(4) == 2);
    static_assert(isqrt(5) == 2);
    static_assert(isqrt(8) == 2);
    static_assert(isqrt(9) == 3);
    static_assert(calc(0, 100) == 1);
    static_assert(calc(2047, 100) == 1);
    static_assert(calc(2048, 100) == 2);
    static_assert(calc(4608, 100) == 3);
    static_assert(calc(8192, 100) == 4);
    static_assert(calc(8192, 5) == 4);
    static_assert(calc(8192, 4) == 3);
    static_assert(calc(8192, 3) == 2);
    static_assert(calc(8192, 2) == 1);
    static_assert(calc(8192, 1) == 1);
    static_assert(calc(8192, 0) == 1);
    static_assert(calc(50000, 100) == 9);
    static_assert(calc(51200, 100) == 10);

    return calc(elements, MaxJobs());
}

auto SubchainStateData::describe(
    const crypto::Subaccount& account,
    const crypto::Subchain subchain,
    allocator_type alloc) noexcept -> CString
{
    // TODO c++20 use allocator
    auto out = std::stringstream{};
    out << account.Describe();
    out << ' ';
    out << print(subchain);
    out << " subchain";

    return CString{alloc} + out.str().c_str();
}

auto SubchainStateData::do_reorg(
    const node::HeaderOracle& oracle,
    const node::internal::HeaderOraclePrivate& data,
    Reorg::Params& params) noexcept -> bool
{
    auto alloc = alloc::Strategy{get_allocator()};  // TODO
    auto& [position, tx] = params;
    log_()(name_)(" processing reorg to ")(position).Flush();
    const auto tip = db_.SubchainLastScanned(db_key_);

    try {
        // TODO use position
        const auto reorg =
            oracle.Internal().CalculateReorg(data, tip, get_allocator());

        if (reorg.empty()) {
            log_()(name_)(" no action required for this subchain").Flush();
            need_reorg_ = false;

            return true;
        } else {
            log_()(name_)(" ")(reorg.size())(
                " previously mined blocks have been invalidated")
                .Flush();
            need_reorg_ = true;
        }

        if (db_.ReorgTo(
                log_,
                data,
                oracle,
                id_,
                subchain_,
                db_key_,
                reorg,
                tx,
                alloc)) {
            LogError()()(name_)(" database error").Flush();
        } else {

            return false;
        }
    } catch (...) {
        LogError()()(name_)(" header oracle claims existing tip ")(
            tip)(" is invalid")
            .Flush();

        return false;
    }

    return true;
}

auto SubchainStateData::do_shutdown() noexcept -> void
{
    state_ = State::shutdown;
    reorg_.Stop();
    node_p_.reset();
    api_p_.reset();
}

auto SubchainStateData::do_startup(allocator_type monotonic) noexcept -> bool
{
    if (reorg_.Start()) { return true; }

    auto me = shared_from_this();
    wallet::Progress{me}.Init();
    wallet::Rescan{me}.Init();
    get_index(me);
    wallet::Process{me}.Init();
    wallet::Scan{me}.Init();
    const auto now = Clock::now();

    for (auto& [type, time] : child_activity_) { time = now; }

    do_work(monotonic);

    return false;
}

auto SubchainStateData::get_account_targets(
    const Elements& elements,
    alloc::Default alloc) const noexcept -> Targets
{
    auto out = Targets{alloc};
    get_targets(elements, out);

    return out;
}

auto SubchainStateData::get_targets(const Elements& in, Targets& targets)
    const noexcept -> void
{
    targets.reserve(in.size());

    for (const auto& element : in.elements_20_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    for (const auto& element : in.elements_32_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    for (const auto& element : in.elements_33_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    for (const auto& element : in.elements_64_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    for (const auto& element : in.elements_65_) {
        const auto& [index, data] = element;
        targets.emplace_back(reader(data));
    }

    get_targets(in.txos_, targets);
}

auto SubchainStateData::get_targets(const TXOs& utxos, Targets& targets)
    const noexcept -> void
{
    switch (filter_type_) {
        case cfilter::Type::Basic_BCHVariant:
        case cfilter::Type::ES: {
            for (const auto& [outpoint, output] : utxos) {
                targets.emplace_back(outpoint.Bytes());
            }
        } break;
        case cfilter::Type::Basic_BIP158: {
        } break;
        case cfilter::Type::UnknownCfilter:
        default: {
            LogAbort()()(name_)(": invalid cfilter type").Abort();
        }
    }
}

auto SubchainStateData::highest_clean(
    const AsyncResults& results,
    block::Position& highestTested) noexcept -> std::optional<block::Position>
{
    const auto& [clean, dirty, sizes] = results;
    const auto haveClean = (0 < clean.size());
    const auto haveDirty = (0 < dirty.size());

    if ((false == haveClean) && haveDirty) {
        highestTested = *dirty.crbegin();

        return std::nullopt;
    } else if ((false == haveDirty) && haveClean) {
        highestTested = *clean.crbegin();

        return highestTested;
    } else if (haveClean && haveDirty) {
        highestTested = std::max(*clean.crbegin(), *dirty.crbegin());
        const auto& lowestDirty = *dirty.cbegin();

        if (auto i = clean.upper_bound(lowestDirty); clean.begin() == i) {

            return std::nullopt;
        } else {

            return *std::prev(i);
        }
    } else {

        return std::nullopt;
    }
}

auto SubchainStateData::IndexElement(
    const cfilter::Type type,
    const blockchain::crypto::Element& input,
    const Bip32Index index,
    database::ElementMap& output) const noexcept -> void
{
    log_()(name_)(" element ")(index)(" extracting filter matching patterns")
        .Flush();
    auto& list = output[index];
    const auto scripts = supported_scripts(input);

    switch (type) {
        case cfilter::Type::ES: {
            for (const auto& [sw, p, s, e, script] : scripts) {
                for (const auto& element : e) {
                    list.emplace_back(
                        space(element, list.get_allocator().resource()));
                }
            }
        } break;
        case cfilter::Type::Basic_BIP158:
        case cfilter::Type::Basic_BCHVariant: {
            for (const auto& [sw, p, s, e, script] : scripts) {
                script.Serialize(writer(list.emplace_back()));
            }
        } break;
        case cfilter::Type::UnknownCfilter:
        default: {
            LogAbort()()(name_)(": invalid cfilter type").Abort();
        }
    }
}

auto SubchainStateData::Init(std::shared_ptr<SubchainStateData> me) noexcept
    -> void
{
    signal_startup(me);
}

auto SubchainStateData::pipeline(
    const Work work,
    Message&& msg,
    allocator_type) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::pre_shutdown: {
            state_pre_shutdown(work, std::move(msg));
        } break;
        case State::shutdown: {
            // NOTE do nothing
        } break;
        default: {
            LogAbort()()(name_)(": invalid state").Abort();
        }
    }
}

auto SubchainStateData::process_prepare_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    assert_true(1_uz < body.size());

    transition_state_reorg(body[1].as<StateSequence>());
}

auto SubchainStateData::process_rescan(Message&& in) noexcept -> void
{
    start_rescan();
}

auto SubchainStateData::process_watchdog_ack(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    assert_true(1_uz < body.size());

    child_activity_.at(body[1].as<JobType>()) = Clock::now();
}

auto SubchainStateData::ProcessBlock(
    const block::Position& position,
    block::Block& block,
    allocator_type monotonic) const noexcept -> bool
{
    const auto& name = name_;
    const auto& type = filter_type_;
    const auto& node = node_;
    const auto& filters = node.FilterOracle();
    const auto& blockHash = position.hash_;
    auto keyMatches = 0_uz;
    auto txoMatches = 0_uz;
    const auto& log = api_.GetOptions().TestMode() ? LogConsole() : log_;
    const auto confirmed = [&] {
        const auto handle = element_cache_.lock_shared();
        const auto matches = match_cache_.lock_shared()->GetMatches(position);
        const auto& elements = handle->GetElements();
        auto patterns =
            std::make_pair(Patterns{monotonic}, Patterns{monotonic});
        auto& [outpoint, key] = patterns;
        outpoint.clear();
        key.clear();

        if (false == select_matches(matches, position, elements, patterns)) {
            // TODO blocks should only be queued for processing if they have
            // been previously marked by a scan or rescan operation which
            // updates the cache with the appropriate entries so it's not clear
            // why this branch can ever be reached.
            select_all(position, elements, patterns);
        }

        const auto cfilter =
            filters.LoadFilter(type, blockHash, {get_allocator(), monotonic});

        assert_true(cfilter.IsValid());

        keyMatches = key.size();
        txoMatches = outpoint.size();

        return block.Internal().FindMatches(
            api_, type, outpoint, key, log, monotonic, monotonic);
    }();
    const auto& [utxo, general] = confirmed;
    const auto& oracle = node.HeaderOracle();
    const auto header = oracle.LoadHeader(blockHash);

    assert_true(header.IsValid());
    assert_true(position == header.Position());

    handle_block_matches(position, confirmed, log, block, monotonic);
    LogConsole()(name)(" processed block ")(position).Flush();
    log()(name)(" ")(general.size())(" of ")(
        keyMatches)(" potential key matches confirmed.")
        .Flush();
    log()(name)(" ")(utxo.size())(" of ")(
        txoMatches)(" potential utxo matches confirmed.")
        .Flush();

    return true;
}

auto SubchainStateData::ProcessTransaction(
    const block::Transaction& tx,
    const Log& log,
    allocator_type monotonic) const noexcept -> void
{
    const auto matches = [&] {
        auto handle = element_cache_.lock_shared();
        const auto& elements = handle->GetElements();
        const auto targets = get_account_targets(elements, monotonic);
        const auto patterns = to_patterns(elements, monotonic);
        const auto parsed = block::ParsedPatterns{patterns, monotonic};
        const auto outpoints = translate(elements.txos_, monotonic);

        return tx.Internal().asBitcoin().FindMatches(
            api_, filter_type_, outpoints, parsed, log, monotonic, monotonic);
    }();
    const auto& [utxo, general] = matches;
    log()(name_)(" mempool transaction ")(tx.ID().asHex())(" matches ")(
        utxo.size())(" utxos and ")(general.size())(" keys")
        .Flush();
    handle_mempool_match(matches, tx, monotonic);
}

auto SubchainStateData::RescanFinished() const noexcept -> void
{
    rescan_pending_.store(false);
}

auto SubchainStateData::ReportScan(const block::Position& pos) const noexcept
    -> void
{
    log_()(name_)(" progress updated to ")(pos).Flush();
    subaccount_.Internal().SetScanProgress(pos, subchain_);
    api_.Crypto().Blockchain().Internal().ReportScan(
        chain_, owner_, account_type_, id_, subchain_, pos);
}

auto SubchainStateData::reorg_children() const noexcept -> std::size_t
{
    return 1_uz;
}

auto SubchainStateData::ReorgTarget(
    const block::Position& reorg,
    const block::Position& current) const noexcept -> block::Position
{
    return std::min(current, reorg);
}

auto SubchainStateData::Rescan(
    const block::Position best,
    const block::Height stop,
    block::Position& highestTested,
    Vector<ScanStatus>& out,
    allocator_type monotonic) const noexcept -> std::optional<block::Position>
{
    return scan(true, best, stop, highestTested, out, monotonic);
}

auto SubchainStateData::Scan(
    const block::Position best,
    const block::Height stop,
    block::Position& highestTested,
    Vector<ScanStatus>& out,
    allocator_type monotonic) const noexcept -> std::optional<block::Position>
{
    return scan(false, best, stop, highestTested, out, monotonic);
}

auto SubchainStateData::scan(
    const bool rescan,
    const block::Position best,
    const block::Height stop,
    block::Position& highestTested,
    Vector<ScanStatus>& out,
    allocator_type monotonic) const noexcept -> std::optional<block::Position>
{
    try {
        using namespace std::literals;
        const auto procedure = rescan ? "rescan"sv : "scan"sv;
        const auto& log = log_;
        const auto& name = name_;
        const auto start = Clock::now();
        const auto startHeight = highestTested.height_ + 1;
        auto atLeastOnce = std::atomic_bool{false};
        auto highestClean = std::optional<block::Position>{std::nullopt};
        auto resultMap = wallet::MatchCache::Results{get_allocator()};
        scan(
            log,
            start,
            rescan,
            best,
            stop,
            startHeight,
            procedure,
            atLeastOnce,
            highestClean,
            highestTested,
            resultMap,
            out,
            monotonic);

        if (atLeastOnce.load()) {
            if (false == resultMap.empty()) {
                match_cache_.lock()->Add(std::move(resultMap));
            }

            const auto count = out.size();
            log()(name)(" ")(procedure)(" found ")(
                count)(" new potential matches between blocks ")(
                startHeight)(" and ")(highestTested.height_)(" in ")(
                std::chrono::nanoseconds{Clock::now() - start})
                .Flush();
        } else {
            log_()(name)(" ")(procedure)(" interrupted").Flush();
        }

        return highestClean;
    } catch (...) {

        return std::nullopt;
    }
}

auto SubchainStateData::scan(
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
    allocator_type monotonic) const noexcept(false) -> void
{
    const auto elementsPerFilter = [this] {
        const auto cached = elements_per_cfilter_.load();

        if (0_uz == cached) {
            const auto chainDefault =
                params::get(chain_).CfilterBatchEstimate();

            return std::max<std::size_t>(1_uz, chainDefault);
        } else {

            return cached;
        }
    }();

    assert_true(0_uz < elementsPerFilter);

    constexpr auto GetBatchSize = [](std::size_t cfilter, std::size_t user) {
        constexpr auto cfilterWeight = 1_uz;
        constexpr auto walletWeight = 5_uz;
        constexpr auto target = 425000_uz;
        constexpr auto max = 10000_uz;

        return std::min<std::size_t>(
            std::max<std::size_t>(
                (target * (cfilterWeight * walletWeight)) /
                    ((cfilterWeight * cfilter) + (walletWeight * user)),
                1_uz),
            max);
    };
    static_assert(GetBatchSize(1, 1) == 10000);
    static_assert(GetBatchSize(25, 40) == 9444);
    static_assert(GetBatchSize(1000, 40) == 1770);
    static_assert(GetBatchSize(25, 400) == 1049);
    static_assert(GetBatchSize(1000, 400) == 708);
    static_assert(GetBatchSize(25, 4000) == 106);
    static_assert(GetBatchSize(25, 40000) == 10);
    static_assert(GetBatchSize(1000, 40000) == 10);
    static_assert(GetBatchSize(25, 400000) == 1);
    static_assert(GetBatchSize(25, 4000000) == 1);
    static_assert(GetBatchSize(10000, 4000000) == 1);
    auto elementcache = element_cache_.lock_shared();
    const auto& elements = elementcache->GetElements();
    const auto elementCount = std::max<std::size_t>(elements.size(), 1_uz);
    // NOTE attempting to scan too many filters at once causes this
    // function to take excessive time to execute, which means the Scan
    // and Rescan Actors will be unable to process new messages for an
    // extended amount of time which has many negative side effects. The
    // GetBatchSize function attempts to prevent this from happening by
    // limiting the batch size to a reasonable value based on the
    // average cfilter element count (estimated) and match set for this
    // subchain (known).
    const auto threads = choose_thread_count(elementCount);

    assert_true(0_uz < threads);

    const auto scanBatch = std::min(
        maximum_scan_, GetBatchSize(elementsPerFilter, elementCount) * threads);
    log()(name_)(" filter size: ")(elementsPerFilter)(" wallet size: ")(
        elementCount)(" batch size: ")(scanBatch)
        .Flush();
    const auto stopHeight = std::min(
        std::min<block::Height>(startHeight + scanBatch - 1, best.height_),
        stop);

    if (startHeight > stopHeight) {
        log()(name_)(" attempted to ")(procedure)(" filters from ")(
            startHeight)(" to ")(stopHeight)(" but this is impossible")
            .Flush();

        throw std::runtime_error{""};
    }

    log()(name_)(" ")(procedure)("ning filters from ")(startHeight)(" to ")(
        stopHeight)
        .Flush();
    const auto target = static_cast<std::size_t>(stopHeight - startHeight + 1);
    const auto blocks =
        node_.HeaderOracle().BestHashes(startHeight, target, monotonic);

    if (blocks.empty()) { throw std::runtime_error{""}; }

    auto filterPromise = std::promise<Vector<cfilter::GCS>>{};
    auto filterFuture = filterPromise.get_future();
    RunJob([me = shared_from_this(), &filterPromise, &blocks] {
        auto alloc = me->get_allocator();
        auto mr = alloc::MonotonicSync{alloc.resource()};
        filterPromise.set_value(me->node_.FilterOracle().LoadFilters(
            me->filter_type_, blocks, {alloc, std::addressof(mr)}));
    });
    auto selected = BlockTargets{monotonic};
    select_targets(*elementcache, blocks, elements, startHeight, selected);
    elementcache.reset();

    assert_false(selected.empty());

    auto prehash = PrehashData{
        api_,
        selected,
        name_,
        results,
        startHeight,
        std::min(threads, selected.size()),
        monotonic};
    prehash.Prepare();
    const auto havePrehash = Clock::now();
    log_()(name_)(" ")(procedure)(" calculated target hashes for ")(
        blocks.size())(" cfilters in ")(
        std::chrono::nanoseconds{havePrehash - start})
        .Flush();
    const auto cfilters = [&] {
        auto output = filterFuture.get();
        output.erase(
            std::ranges::find_if(
                output,
                [](const auto& filter) { return false == filter.IsValid(); }),
            output.end());

        return output;
    }();
    const auto haveCfilters = Clock::now();
    log_()(name_)(" ")(procedure)(" loaded cfilters in ")(
        std::chrono::nanoseconds{haveCfilters - havePrehash})
        .Flush();
    const auto cfilterCount = cfilters.size();

    assert_true(cfilterCount <= blocks.size());

    auto data = MatchResults{std::make_tuple(
        Positions{monotonic}, Positions{monotonic}, FilterMap{monotonic})};

    assert_true(0_uz < selected.size());

    prehash.Match(
        procedure, log, cfilters, atLeastOnce, results, data, monotonic);

    {
        auto handle = data.lock_shared();
        const auto& [clean, dirty, sizes] = *handle;

        if (auto size = dirty.size(); 0 < size) {
            log_()(name_)(" requesting ")(
                size)(" block hashes from block oracle")
                .Flush();
            to_block_oracle_.SendDeferred([&](const auto& positions) {
                auto work = MakeWork(node::blockoracle::Job::request_blocks);

                for (const auto& position : positions) {
                    const auto& [height, hash] = position;
                    work.AddFrame(hash);
                    out.emplace_back(ScanState::dirty, position);
                }

                return work;
            }(dirty));
        }

        highestClean = highest_clean(*handle, highestTested);

        if (false == rescan) {
            std::ranges::transform(
                sizes, std::back_inserter(filter_sizes_), [](const auto& in) {
                    return in.second;
                });

            // NOTE these statements calculate a 1000 block (or whatever
            // cfilter_size_window_ is set to) simple moving average of
            // cfilter element sizes

            while (cfilter_size_window_ < filter_sizes_.size()) {
                filter_sizes_.pop_front();
            }

            const auto totalCfilterElements = std::accumulate(
                filter_sizes_.begin(), filter_sizes_.end(), 0_uz);
            elements_per_cfilter_.store(
                std::max(1_uz, totalCfilterElements / filter_sizes_.size()));
        }
    }
}

auto SubchainStateData::select_all(
    const block::Position& block,
    const Elements& in,
    MatchesToTest& matched) const noexcept -> void
{
    const auto subchainID = block::SubchainIndex{subchain_, id_};
    auto& [outpoint, key] = matched;
    auto alloc = outpoint.get_allocator();
    const auto SelectKey = [&](const auto& all, auto& out) {
        for (const auto& [index, data] : all) {
            out.emplace_back(std::make_pair(
                std::make_pair(index, subchainID), space(reader(data), alloc)));
        }
    };
    const auto SelectTxo = [&](const auto& all, auto& out) {
        for (const auto& [op, output] : all) {
            for (const auto& txokey : output.Keys({})) {  // TODO allocator
                const auto& [id, subchain, index] = txokey;

                if (id_ != id) { continue; }
                if (subchain_ != subchain) { continue; }

                out.emplace_back(std::make_pair(
                    std::make_pair(index, subchainID),
                    space(op.Bytes(), alloc)));
            }
        }
    };

    SelectKey(in.elements_20_, key);
    SelectKey(in.elements_32_, key);
    SelectKey(in.elements_33_, key);
    SelectKey(in.elements_64_, key);
    SelectKey(in.elements_65_, key);
    SelectTxo(in.txos_, outpoint);
}

auto SubchainStateData::select_matches(
    const std::optional<wallet::MatchIndex>& matches,
    const block::Position& block,
    const Elements& in,
    MatchesToTest& matched) const noexcept -> bool
{
    const auto subchainID = block::SubchainIndex{subchain_, id_};
    auto& [outpoint, key] = matched;
    auto alloc = outpoint.get_allocator();
    const auto SelectKey =
        [&](const auto& all, const auto& selected, auto& out) {
            if (0_uz == selected.size()) { return; }

            for (const auto& [index, data] : all) {
                if (0_uz < selected.count(index)) {
                    out.emplace_back(std::make_pair(
                        std::make_pair(index, subchainID),
                        space(reader(data), alloc)));
                }
            }
        };
    const auto SelectTxo = [&](const auto& all,
                               const auto& selected,
                               auto& out) {
        if (0_uz == selected.size()) { return; }

        for (const auto& [op, output] : all) {
            if (0_uz < selected.count(op)) {
                for (const auto& txokey : output.Keys({})) {  // TODO allocator
                    const auto& [id, subchain, index] = txokey;

                    if (id_ != id) { continue; }
                    if (subchain_ != subchain) { continue; }

                    out.emplace_back(std::make_pair(
                        std::make_pair(index, subchainID),
                        space(op.Bytes(), alloc)));
                }
            }
        }
    };

    if (matches.has_value()) {
        const auto& items = matches->confirmed_match_;
        SelectKey(in.elements_20_, items.match_20_, key);
        SelectKey(in.elements_32_, items.match_32_, key);
        SelectKey(in.elements_33_, items.match_33_, key);
        SelectKey(in.elements_64_, items.match_64_, key);
        SelectKey(in.elements_65_, items.match_65_, key);
        SelectTxo(in.txos_, items.match_txo_, outpoint);

        return true;
    } else {
        // TODO this should never happen because a block is only processed when
        // a previous call to SubchainStateData::scan indicated this block had a
        // match, and the scan function should have set the matched elements in
        // the element cache. The only time this data is deleted is the call to
        // ElementCache::Forget, but that should never be called until there is
        // no possibility that blocks at or below that position will be
        // processed.
        log_()(name_)(" existing matches for block ")(block)(" not found")
            .Flush();

        return false;
    }
}

auto SubchainStateData::select_targets(
    const wallet::ElementCache& cache,
    const BlockHashes& hashes,
    const Elements& in,
    block::Height height,
    BlockTargets& targets) const noexcept -> void
{
    for (const auto& hash : hashes) {
        select_targets(cache, block::Position{height++, hash}, in, targets);
    }
}

auto SubchainStateData::select_targets(
    const wallet::ElementCache& cache,
    const block::Position& block,
    const Elements& in,
    BlockTargets& targets) const noexcept -> void
{
    constexpr auto Prepare = [](auto& pair, auto size) {
        constexpr auto Reserve = [](auto& vector, auto reserve) {
            vector.reserve(reserve);
            vector.clear();
        };
        Reserve(pair.first, size);
        Reserve(pair.second, size);
    };
    constexpr auto ChooseKey =
        [](const auto& index, const auto& data, auto& out) {
            out.first.emplace_back(index);
            out.second.emplace_back(reader(data));
        };
    constexpr auto ChooseTxo = [](const auto& outpoint, auto& out) {
        out.first.emplace_back(outpoint);
        out.second.emplace_back(outpoint.Bytes());
    };
    auto alloc = targets.get_allocator();
    auto& [hash, selected] = targets.emplace_back(std::make_pair(
        block.hash_,
        std::make_tuple(
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<Bip32Index>{alloc}, Targets{alloc}),
            std::make_pair(Vector<block::Outpoint>{alloc}, Targets{alloc}))));
    auto& [s20, s32, s33, s64, s65, stxo] = selected;
    Prepare(s20, in.elements_20_.size());
    Prepare(s32, in.elements_32_.size());
    Prepare(s33, in.elements_33_.size());
    Prepare(s64, in.elements_64_.size());
    Prepare(s65, in.elements_65_.size());
    Prepare(stxo, in.txos_.size());
    const auto matches = match_cache_.lock_shared()->GetMatches(block);

    for (const auto& [index, data] : in.elements_20_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_20_;
            const auto& yes = matches->confirmed_match_.match_20_;

            if ((0_uz == no.count(index)) && (0_uz == yes.count(index))) {
                ChooseKey(index, data, s20);
            }
        } else {
            ChooseKey(index, data, s20);
        }
    }

    for (const auto& [index, data] : in.elements_32_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_32_;
            const auto& yes = matches->confirmed_match_.match_32_;

            if ((0_uz == no.count(index)) && (0_uz == yes.count(index))) {
                ChooseKey(index, data, s32);
            }
        } else {
            ChooseKey(index, data, s32);
        }
    }

    for (const auto& [index, data] : in.elements_33_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_33_;
            const auto& yes = matches->confirmed_match_.match_33_;

            if ((0_uz == no.count(index)) && (0_uz == yes.count(index))) {
                ChooseKey(index, data, s33);
            }
        } else {
            ChooseKey(index, data, s33);
        }
    }

    for (const auto& [index, data] : in.elements_64_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_64_;
            const auto& yes = matches->confirmed_match_.match_64_;

            if ((0_uz == no.count(index)) && (0_uz == yes.count(index))) {
                ChooseKey(index, data, s64);
            }
        } else {
            ChooseKey(index, data, s64);
        }
    }

    for (const auto& [index, data] : in.elements_65_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_65_;
            const auto& yes = matches->confirmed_match_.match_65_;

            if ((0_uz == no.count(index)) && (0_uz == yes.count(index))) {
                ChooseKey(index, data, s65);
            }
        } else {
            ChooseKey(index, data, s65);
        }
    }

    for (const auto& [outpoint, output] : in.txos_) {
        if (matches.has_value()) {
            const auto& no = matches->confirmed_no_match_.match_txo_;
            const auto& yes = matches->confirmed_match_.match_txo_;

            if ((0_uz == no.count(outpoint)) && (0_uz == yes.count(outpoint))) {
                ChooseTxo(outpoint, stxo);
            }
        } else {
            ChooseTxo(outpoint, stxo);
        }
    }
}

auto SubchainStateData::start_rescan() const noexcept -> void
{
    to_scan_.Send(MakeWork(Work::do_rescan));
}

auto SubchainStateData::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::prepare_reorg: {
            process_prepare_reorg(std::move(msg));
        } break;
        case Work::watchdog_ack: {
            process_watchdog_ack(std::move(msg));
        } break;
        case Work::rescan: {
            process_rescan(std::move(msg));
        } break;
        case Work::block: {
            // NOTE ignore message
        } break;
        case Work::prepare_shutdown: {
            transition_state_pre_shutdown();
        } break;
        case Work::finish_reorg: {
            LogAbort()()(name_)(" wrong state for ")(print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::filter:
        case Work::mempool:
        case Work::start_scan:
        case Work::update:
        case Work::process:
        case Work::watchdog:
        case Work::reprocess:
        case Work::do_rescan:
        case Work::init:
        case Work::key:
        case Work::statemachine: {
            unhandled_type(work);
        }
        default: {
            unknown_type(work);
        }
    }
}

auto SubchainStateData::state_pre_shutdown(
    const Work work,
    Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::watchdog_ack:
        case Work::rescan: {
            // NOTE ignore message
        } break;
        case Work::block: {
            // NOTE ignore message
        } break;
        case Work::prepare_reorg:
        case Work::finish_reorg:
        case Work::prepare_shutdown: {
            LogAbort()()(name_)(" wrong state for ")(print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::filter:
        case Work::mempool:
        case Work::start_scan:
        case Work::update:
        case Work::process:
        case Work::watchdog:
        case Work::reprocess:
        case Work::do_rescan:
        case Work::init:
        case Work::key:
        case Work::statemachine: {
            unhandled_type(work);
        }
        default: {
            unknown_type(work);
        }
    }
}

auto SubchainStateData::state_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::prepare_reorg:
        case Work::rescan: {
            defer(std::move(msg));
        } break;
        case Work::watchdog_ack: {
            process_watchdog_ack(std::move(msg));
        } break;
        case Work::finish_reorg: {
            transition_state_normal();
        } break;
        case Work::block: {
            // NOTE ignore message
        } break;
        case Work::prepare_shutdown: {
            LogAbort()()("wrong state for ")(print(work))(" message").Abort();
        }
        case Work::shutdown:
        case Work::filter:
        case Work::mempool:
        case Work::start_scan:
        case Work::update:
        case Work::process:
        case Work::watchdog:
        case Work::reprocess:
        case Work::do_rescan:
        case Work::init:
        case Work::key:
        case Work::statemachine: {
            unhandled_type(work);
        }
        default: {
            unknown_type(work);
        }
    }
}

auto SubchainStateData::supported_scripts(const crypto::Element& element)
    const noexcept -> UnallocatedVector<ScriptForm>
{
    auto out = UnallocatedVector<ScriptForm>{};
    using enum protocol::bitcoin::base::block::script::Pattern;
    out.emplace_back(api_, element, chain_, PayToPubkey);
    out.emplace_back(api_, element, chain_, PayToPubkeyHash);
    out.emplace_back(api_, element, chain_, PayToWitnessPubkeyHash);

    return out;
}

auto SubchainStateData::to_patterns(const Elements& in, allocator_type alloc)
    const noexcept -> Patterns
{
    auto out = Patterns{alloc};
    const auto subchainID = block::SubchainIndex{subchain_, id_};
    auto cb = [&](const auto& vector) {
        for (const auto& [index, data] : vector) {
            out.emplace_back(std::make_pair(
                block::ElementIndex{index, subchainID},
                [&](const auto& source) {
                    auto pattern = Vector<std::byte>{alloc};
                    copy(reader(source), writer(pattern));

                    return pattern;
                }(data)));
        }
    };
    cb(in.elements_20_);
    cb(in.elements_32_);
    cb(in.elements_33_);
    cb(in.elements_64_);
    cb(in.elements_65_);

    return out;
}

auto SubchainStateData::TriggerRescan() const noexcept -> void
{
    if (auto rescan = rescan_pending_.exchange(true); false == rescan) {
        start_rescan();
    }
}

auto SubchainStateData::transition_state_normal() noexcept -> void
{
    state_ = State::normal;
    log_()(name_)(" transitioned to normal state ").Flush();
    trigger();
}

auto SubchainStateData::transition_state_pre_shutdown() noexcept -> void
{
    watchdog_.Cancel();
    reorg_.AcknowledgeShutdown();
    state_ = State::pre_shutdown;
    log_()(name_)(": transitioned to pre_shutdown state").Flush();
}

auto SubchainStateData::transition_state_reorg(StateSequence id) noexcept
    -> void
{
    assert_true(0_uz < id);

    if (0_uz == reorgs_.count(id)) {
        reorgs_.emplace(id);
        state_ = State::reorg;
        log_()(name_)(" ready to process reorg ")(id).Flush();
        reorg_.AcknowledgePrepareReorg(
            [this](const auto& header, const auto& lock, auto& params) {
                return do_reorg(header, lock, params);
            });
    } else {
        LogAbort()()(name_)(" reorg ")(id)(" already handled").Abort();
    }
}

auto SubchainStateData::translate(const TXOs& utxos, allocator_type alloc)
    const noexcept -> Patterns
{
    auto outpoints = Patterns{alloc};
    outpoints.reserve(utxos.size());
    outpoints.clear();

    for (const auto& [outpoint, output] : utxos) {
        const auto keys = output.Keys({});  // TODO allocator

        assert_true(0 < keys.size());
        // TODO the assertion below will not always be true in the future but
        // for now it will catch some bugs
        assert_true(1 == keys.size());

        for (const auto& key : keys) {
            const auto& [account, subchain, index] = key;

            assert_false(account.empty());
            // TODO the assertion below will not always be true in the future
            // but for now it will catch some bugs
            assert_true(account == id_);

            outpoints.emplace_back(
                block::ElementIndex{
                    static_cast<Bip32Index>(index),
                    {static_cast<crypto::Subchain>(subchain),
                     std::move(account)}},
                space(outpoint.Bytes(), outpoints.get_allocator().resource()));
        }
    }

    return outpoints;
}

auto SubchainStateData::work(allocator_type monotonic) noexcept -> bool
{
    const auto now = Clock::now();
    using namespace std::literals;
    static constexpr auto timeout = 90s;

    for (const auto& [type, time] : child_activity_) {
        const auto interval =
            std::chrono::duration_cast<std::chrono::nanoseconds>(now - time);

        if (interval > timeout) {
            LogConsole()(interval)(" elapsed since last activity from ")(
                print(type))(" job for ")(name_)
                .Flush();
        }
    }

    reset_timer(60s, watchdog_, Work::statemachine);

    return false;
}

SubchainStateData::~SubchainStateData() = default;
}  // namespace opentxs::blockchain::node::wallet
