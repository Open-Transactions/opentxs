// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"  // IWYU pragma: associated

#include <boost/container/container_fwd.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <compare>
#include <cstdint>
#include <future>
#include <iterator>
#include <memory>
#include <numeric>
#include <span>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "TBB.hpp"
#include "blockchain/node/wallet/subchain/ScriptForm.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/bitcoin/cfilter/GCS.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
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
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Thread.hpp"
#include "internal/util/alloc/Boost.hpp"
#include "internal/util/alloc/ThreadSafe.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"
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
class SubchainStateData::PrehashData
{
public:
    const std::size_t job_count_;

    auto Match(
        const std::string_view procedure,
        const Log& log,
        const Vector<GCS>& cfilters,
        std::atomic_bool& atLeastOnce,
        const std::size_t job,
        wallet::MatchCache::Results& results,
        MatchResults& matched,
        alloc::Default monotonic) noexcept -> void
    {
        const auto end = std::min(targets_.size(), cfilters.size());
        auto cache = std::make_tuple(
            Positions{monotonic}, Positions{monotonic}, FilterMap{monotonic});

        for (auto i = job; i < end; i += job_count_) {
            atLeastOnce.store(true);
            const auto& cfilter = cfilters.at(i);
            const auto& selected = targets_.at(i);
            const auto& data = data_.at(i);
            const auto position =
                block::Position{std::get<0>(data), selected.first};
            auto& result = results.at(position);
            match(
                procedure,
                log,
                position,
                cfilter,
                selected,
                data,
                cache,
                result,
                monotonic);
        }
        matched.modify([&](auto& out) {
            const auto& [iClean, iDirty, iSizes] = cache;
            auto& [oClean, oDirty, oSizes] = out;
            std::copy(
                iClean.begin(),
                iClean.end(),
                std::inserter(oClean, oClean.end()));
            std::copy(
                iDirty.begin(),
                iDirty.end(),
                std::inserter(oDirty, oDirty.end()));
            std::copy(
                iSizes.begin(),
                iSizes.end(),
                std::inserter(oSizes, oSizes.end()));
        });
    }
    auto Prepare(const std::size_t job) noexcept -> void
    {
        const auto end = targets_.size();

        for (auto i = job; i < end; i += job_count_) {
            hash(targets_.at(i), data_.at(i));
        }
    }

    PrehashData(
        const api::Session& api,
        const BlockTargets& targets,
        const std::string_view name,
        wallet::MatchCache::Results& results,
        block::Height start,
        std::size_t jobs,
        allocator_type alloc) noexcept
        : job_count_(jobs)
        , api_(api)
        , targets_(targets)
        , name_(name)
        , data_(alloc)
    {
        OT_ASSERT(0 < job_count_);

        data_.reserve(targets_.size());

        for (const auto& [block, elements] : targets_) {
            const auto& [e20, e32, e33, e64, e65, eTxo] = elements;
            auto& [height, data20, data32, data33, data64, data65, dataTxo] =
                data_.emplace_back();
            data20.first.reserve(e20.first.size());
            data32.first.reserve(e32.first.size());
            data33.first.reserve(e33.first.size());
            data64.first.reserve(e64.first.size());
            data65.first.reserve(e65.first.size());
            dataTxo.first.reserve(eTxo.first.size());
            height = start++;
            results[block::Position{height, block}];
        }

        OT_ASSERT(targets_.size() == data_.size());
    }

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

    auto hash(const BlockTarget& target, BlockData& row) noexcept -> void
    {
        const auto& [block, elements] = target;
        const auto& [e20, e32, e33, e64, e65, eTxo] = elements;
        auto& [height, data20, data32, data33, data64, data65, dataTxo] = row;
        hash(block, e20, data20);
        hash(block, e32, data32);
        hash(block, e33, data33);
        hash(block, e64, data64);
        hash(block, e65, data65);
        hash(block, eTxo, dataTxo);
    }
    template <typename Input, typename Output>
    auto hash(
        const block::Hash& block,
        const std::pair<Vector<Input>, Targets>& targets,
        Output& dest) noexcept -> void
    {
        const auto key =
            blockchain::internal::BlockHashToFilterKey(block.Bytes());
        const auto& [indices, bytes] = targets;
        auto& [hashes, map] = dest;
        auto i = indices.cbegin();
        auto t = bytes.cbegin();
        auto end = indices.cend();

        for (; i < end; ++i, ++t) {
            auto& hash = hashes.emplace_back(gcs::Siphash(api_, key, *t));
            map[hash].emplace_back(&(*i));
        }

        dedup(hashes);
    }
    auto match(
        const std::string_view procedure,
        const Log& log,
        const block::Position& position,
        const GCS& cfilter,
        const BlockTarget& targets,
        const BlockData& prehashed,
        AsyncResults& cache,
        wallet::MatchCache::Index& results,
        alloc::Default monotonic) const noexcept -> void
    {
        const auto GetKeys = [&](const auto& data) {
            auto out = Set<Bip32Index>{monotonic};
            out.clear();
            const auto& [hashes, map] = data;
            const auto start = hashes.cbegin();
            const auto matches = cfilter.Internal().Match(hashes, monotonic);

            for (const auto& match : matches) {
                const auto dist = std::distance(start, match);

                OT_ASSERT(0 <= dist);

                const auto& hash = hashes.at(static_cast<std::size_t>(dist));

                for (const auto* item : map.at(hash)) { out.emplace(*item); }
            }

            return out;
        };
        const auto GetOutpoints = [&](const auto& data) {
            auto out = Set<block::Outpoint>{monotonic};
            out.clear();
            const auto& [hashes, map] = data;
            const auto start = hashes.cbegin();
            const auto matches = cfilter.Internal().Match(hashes, monotonic);

            for (const auto& match : matches) {
                const auto dist = std::distance(start, match);

                OT_ASSERT(0 <= dist);

                const auto& hash = hashes.at(static_cast<std::size_t>(dist));

                for (const auto* item : map.at(hash)) { out.emplace(*item); }
            }

            return out;
        };
        const auto GetResults = [&](const auto& cb,
                                    const auto& pre,
                                    const auto& selected,
                                    auto& clean,
                                    auto& dirty,
                                    auto& output) {
            const auto matches = cb(pre);

            for (const auto& index : selected.first) {
                if (0_uz == matches.count(index)) {
                    clean.emplace(index);
                } else {
                    dirty.emplace(index);
                }
            }

            output.first += matches.size();
            output.second += selected.first.size();
        };
        const auto& selected = targets.second;
        const auto& [height, p20, p32, p33, p64, p65, pTxo] = prehashed;
        const auto& [s20, s32, s33, s64, s65, sTxo] = selected;
        auto output = std::pair<std::size_t, std::size_t>{};
        GetResults(
            GetKeys,
            p20,
            s20,
            results.confirmed_no_match_.match_20_,
            results.confirmed_match_.match_20_,
            output);
        GetResults(
            GetKeys,
            p32,
            s32,
            results.confirmed_no_match_.match_32_,
            results.confirmed_match_.match_32_,
            output);
        GetResults(
            GetKeys,
            p33,
            s33,
            results.confirmed_no_match_.match_33_,
            results.confirmed_match_.match_33_,
            output);
        GetResults(
            GetKeys,
            p64,
            s64,
            results.confirmed_no_match_.match_64_,
            results.confirmed_match_.match_64_,
            output);
        GetResults(
            GetKeys,
            p65,
            s65,
            results.confirmed_no_match_.match_65_,
            results.confirmed_match_.match_65_,
            output);
        GetResults(
            GetOutpoints,
            pTxo,
            sTxo,
            results.confirmed_no_match_.match_txo_,
            results.confirmed_match_.match_txo_,
            output);
        const auto& [count, of] = output;
        log(OT_PRETTY_CLASS())(name_)(" GCS ")(procedure)(" for block ")(
            position)(" matched ")(count)(" of ")(of)(" target elements")
            .Flush();
        auto& [clean, dirty, sizes] = cache;

        if (0_uz == count) {
            clean.emplace(position);
        } else {
            dirty.emplace(position);
        }

        sizes.emplace(position.height_, cfilter.ElementCount());
    }
};
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
SubchainStateData::SubchainStateData(
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
    allocator_type alloc) noexcept
    : Actor(
          *api,
          LogTrace(),
          describe(subaccount, subchain, alloc),
          0ms,
          batch,
          alloc,
          [&] {
              using enum network::zeromq::socket::Direction;
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(api->Endpoints().Shutdown(), Connect);
              sub.emplace_back(fromParent, Connect);

              return sub;
          }(),
          [&] {
              using enum network::zeromq::socket::Direction;
              auto pull = network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(fromChildren, Bind);

              return pull;
          }(),
          {},
          [&] {
              using enum network::zeromq::socket::Direction;
              using enum network::zeromq::socket::Type;
              auto extra = Vector<network::zeromq::SocketData>{alloc};
              extra.emplace_back(
                  Dealer,
                  [&] {
                      auto out = Vector<network::zeromq::EndpointArg>{alloc};
                      out.emplace_back(
                          node->Internal().Endpoints().block_oracle_router_,
                          Connect);

                      return out;
                  }(),
                  false);
              extra.emplace_back(
                  Publish,
                  [&] {
                      auto out = Vector<network::zeromq::EndpointArg>{alloc};
                      out.emplace_back(toChildren, Bind);

                      return out;
                  }(),
                  false);
              extra.emplace_back(
                  Push,
                  [&] {
                      auto out = Vector<network::zeromq::EndpointArg>{alloc};
                      out.emplace_back(toScan, Connect);

                      return out;
                  }(),
                  false);

              return extra;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , api_(*api_p_)
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
{
    OT_ASSERT(false == owner_.empty());
    OT_ASSERT(false == id_.empty());
}

SubchainStateData::SubchainStateData(
    Reorg& reorg,
    const crypto::Subaccount& subaccount,
    std::shared_ptr<const api::Session> api,
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
    auto& [position, tx] = params;
    log_(OT_PRETTY_CLASS())(name_)(" processing reorg to ")(position).Flush();
    const auto tip = db_.SubchainLastScanned(db_key_);

    try {
        // TODO use position
        const auto reorg =
            oracle.Internal().CalculateReorg(data, tip, get_allocator());

        if (reorg.empty()) {
            log_(OT_PRETTY_CLASS())(name_)(
                " no action required for this subchain")
                .Flush();
            need_reorg_ = false;

            return true;
        } else {
            log_(OT_PRETTY_CLASS())(name_)(" ")(reorg.size())(
                " previously mined blocks have been invalidated")
                .Flush();
            need_reorg_ = true;
        }

        if (db_.ReorgTo(data, tx, oracle, id_, subchain_, db_key_, reorg)) {
            LogError()(OT_PRETTY_CLASS())(name_)(" database error").Flush();
        } else {

            return false;
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            name_)(" header oracle claims existing tip ")(tip)(" is invalid")
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
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid cfilter type")
                .Abort();
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
    log_(OT_PRETTY_CLASS())(name_)(" element ")(
        index)(" extracting filter matching patterns")
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
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid cfilter type")
                .Abort();
        }
    }
}

auto SubchainStateData::Init(boost::shared_ptr<SubchainStateData> me) noexcept
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
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid state").Abort();
        }
    }
}

auto SubchainStateData::process_prepare_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(1_uz < body.size());

    transition_state_reorg(body[1].as<StateSequence>());
}

auto SubchainStateData::process_rescan(Message&& in) noexcept -> void
{
    to_scan_.Send(MakeWork(Work::do_rescan), __FILE__, __LINE__);
}

auto SubchainStateData::process_watchdog_ack(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(1_uz < body.size());

    child_activity_.at(body[1].as<JobType>()) = Clock::now();
}

auto SubchainStateData::ProcessBlock(
    const block::Position& position,
    const block::Block& block,
    allocator_type monotonic) const noexcept -> bool
{
    const auto start = Clock::now();
    const auto& name = name_;
    const auto& type = filter_type_;
    const auto& node = node_;
    const auto& filters = node.FilterOracle();
    const auto& blockHash = position.hash_;
    auto haveTargets = Time{};
    auto haveFilter = Time{};
    auto keyMatches = 0_uz;
    auto txoMatches = 0_uz;
    const auto& log = LogTrace();
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

        haveTargets = Clock::now();
        const auto cfilter =
            filters.LoadFilter(type, blockHash, get_allocator(), monotonic);

        OT_ASSERT(cfilter.IsValid());

        haveFilter = Clock::now();
        keyMatches = key.size();
        txoMatches = outpoint.size();

        return block.Internal().FindMatches(
            api_, type, outpoint, key, log, monotonic, monotonic);
    }();
    const auto haveMatches = Clock::now();
    const auto& [utxo, general] = confirmed;
    const auto& oracle = node.HeaderOracle();
    const auto header = oracle.LoadHeader(blockHash);

    OT_ASSERT(header.IsValid());
    OT_ASSERT(position == header.Position());

    const auto haveHeader = Clock::now();
    handle_confirmed_matches(block, position, confirmed, log, monotonic);
    const auto handledMatches = Clock::now();
    LogConsole()(name)(" processed block ")(position)(" in ")(
        std::chrono::nanoseconds{Clock::now() - start})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" ")(general.size())(" of ")(
        keyMatches)(" potential key matches confirmed.")
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" ")(utxo.size())(" of ")(
        txoMatches)(" potential utxo matches confirmed.")
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to load match targets: ")(
        std::chrono::nanoseconds{haveTargets - start})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to load filter: ")(
        std::chrono::nanoseconds{haveFilter - haveTargets})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to find matches: ")(
        std::chrono::nanoseconds{haveMatches - haveFilter})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to load block header: ")(
        std::chrono::nanoseconds{haveHeader - haveMatches})
        .Flush();
    log(OT_PRETTY_CLASS())(name)(" time to handle matches: ")(
        std::chrono::nanoseconds{handledMatches - haveHeader})
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
    log(OT_PRETTY_CLASS())(name_)(" mempool transaction ")(tx.ID().asHex())(
        " matches ")(utxo.size())(" utxos and ")(general.size())(" keys")
        .Flush();
    handle_mempool_matches(matches, tx, monotonic);
}

auto SubchainStateData::ReportScan(const block::Position& pos) const noexcept
    -> void
{
    log_(OT_PRETTY_CLASS())(name_)(" progress updated to ")(pos).Flush();
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
    allocator_type unsafe) const noexcept -> std::optional<block::Position>
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
        auto safe = alloc::ThreadSafe{unsafe.resource()};
        auto monotonic = allocator_type{std::addressof(safe)};
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
            log(OT_PRETTY_CLASS())(name)(" ")(procedure)(" found ")(
                count)(" new potential matches between blocks ")(
                startHeight)(" and ")(highestTested.height_)(" in ")(
                std::chrono::nanoseconds{Clock::now() - start})
                .Flush();
        } else {
            log_(OT_PRETTY_CLASS())(name)(" ")(procedure)(" interrupted")
                .Flush();
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

    OT_ASSERT(0_uz < elementsPerFilter);

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

    OT_ASSERT(0_uz < threads);

    const auto scanBatch = std::min(
        maximum_scan_, GetBatchSize(elementsPerFilter, elementCount) * threads);
    log(OT_PRETTY_CLASS())(name_)(" filter size: ")(
        elementsPerFilter)(" wallet size: ")(elementCount)(" batch size: ")(
        scanBatch)
        .Flush();
    const auto stopHeight = std::min(
        std::min<block::Height>(startHeight + scanBatch - 1, best.height_),
        stop);

    if (startHeight > stopHeight) {
        log(OT_PRETTY_CLASS())(name_)(" attempted to ")(
            procedure)(" filters from ")(startHeight)(" to ")(
            stopHeight)(" but this is impossible")
            .Flush();

        throw std::runtime_error{""};
    }

    log(OT_PRETTY_CLASS())(name_)(" ")(procedure)("ning filters from ")(
        startHeight)(" to ")(stopHeight)
        .Flush();
    const auto target = static_cast<std::size_t>(stopHeight - startHeight + 1);
    const auto blocks =
        node_.HeaderOracle().BestHashes(startHeight, target, monotonic);

    if (blocks.empty()) { throw std::runtime_error{""}; }

    auto filterPromise = std::promise<Vector<GCS>>{};
    auto filterFuture = filterPromise.get_future();
    tbb::fire_and_forget([&, this] {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        std::byte buf[thread_pool_monotonic_];
        auto upstream = alloc::StandardToBoost(get_allocator().resource());
        auto resource =
            alloc::BoostMonotonic(buf, sizeof(buf), std::addressof(upstream));
        auto temp = allocator_type{std::addressof(resource)};
        filterPromise.set_value(node_.FilterOracle().LoadFilters(
            filter_type_, blocks, monotonic, temp));
    });
    auto selected = BlockTargets{monotonic};
    select_targets(*elementcache, blocks, elements, startHeight, selected);
    elementcache.reset();

    OT_ASSERT(false == selected.empty());

    auto prehash = PrehashData{
        api_,
        selected,
        name_,
        results,
        startHeight,
        std::min(threads, selected.size()),
        monotonic};
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, prehash.job_count_},
        [&prehash](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) { prehash.Prepare(i); }
        });
    const auto havePrehash = Clock::now();
    log_(OT_PRETTY_CLASS())(name_)(" ")(
        procedure)(" calculated target hashes for ")(blocks.size())(
        " cfilters in ")(std::chrono::nanoseconds{havePrehash - start})
        .Flush();
    const auto cfilters = [&] {
        auto output = filterFuture.get();
        output.erase(
            std::find_if(
                output.begin(),
                output.end(),
                [](const auto& filter) { return false == filter.IsValid(); }),
            output.end());

        return output;
    }();
    const auto haveCfilters = Clock::now();
    log_(OT_PRETTY_CLASS())(name_)(" ")(procedure)(" loaded cfilters in ")(
        std::chrono::nanoseconds{haveCfilters - havePrehash})
        .Flush();
    const auto cfilterCount = cfilters.size();

    OT_ASSERT(cfilterCount <= blocks.size());

    auto data = MatchResults{std::make_tuple(
        Positions{monotonic}, Positions{monotonic}, FilterMap{monotonic})};

    OT_ASSERT(0_uz < selected.size());

    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, prehash.job_count_},
        [&](const auto& r) {
            auto resource =
                alloc::BoostMonotonic(convert_to_size(thread_pool_monotonic_));
            auto temp = allocator_type{std::addressof(resource)};

            for (auto i = r.begin(); i != r.end(); ++i) {
                prehash.Match(
                    procedure,
                    log,
                    cfilters,
                    atLeastOnce,
                    i,
                    results,
                    data,
                    temp);
            }
        });

    {
        auto handle = data.lock_shared();
        const auto& [clean, dirty, sizes] = *handle;

        if (auto size = dirty.size(); 0 < size) {
            log_(OT_PRETTY_CLASS())(name_)(" requesting ")(
                size)(" block hashes from block oracle")
                .Flush();
            to_block_oracle_.SendDeferred(
                [&](const auto& positions) {
                    auto work =
                        MakeWork(node::blockoracle::Job::request_blocks);

                    for (const auto& position : positions) {
                        const auto& [height, hash] = position;
                        work.AddFrame(hash);
                        out.emplace_back(ScanState::dirty, position);
                    }

                    return work;
                }(dirty),
                __FILE__,
                __LINE__);
        }

        highestClean = highest_clean(*handle, highestTested);

        if (false == rescan) {
            std::transform(
                sizes.begin(),
                sizes.end(),
                std::back_inserter(filter_sizes_),
                [](const auto& in) { return in.second; });

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
    const std::optional<wallet::MatchCache::Index>& matches,
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
        log_(OT_PRETTY_CLASS())(name_)(" existing matches for block ")(
            block)(" not found")
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

auto SubchainStateData::set_key_data(
    block::Transaction& tx,
    allocator_type monotonic) const noexcept -> void
{
    const auto keys = tx.asBitcoin().Keys(monotonic);
    auto data = block::KeyData{monotonic};
    const auto& api = api_.Crypto().Blockchain();

    for (const auto& key : keys) {
        data.try_emplace(
            key, api.SenderContact(key), api.RecipientContact(key));
    }

    tx.Internal().asBitcoin().SetKeyData(data);
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
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
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
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
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
            LogAbort()(OT_PRETTY_CLASS())("wrong state for ")(print(work))(
                " message")
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

auto SubchainStateData::supported_scripts(const crypto::Element& element)
    const noexcept -> UnallocatedVector<ScriptForm>
{
    auto out = UnallocatedVector<ScriptForm>{};
    using enum bitcoin::block::script::Pattern;
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

auto SubchainStateData::transition_state_normal() noexcept -> void
{
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    trigger();
}

auto SubchainStateData::transition_state_pre_shutdown() noexcept -> void
{
    watchdog_.Cancel();
    reorg_.AcknowledgeShutdown();
    state_ = State::pre_shutdown;
    log_(OT_PRETTY_CLASS())(name_)(": transitioned to pre_shutdown state")
        .Flush();
}

auto SubchainStateData::transition_state_reorg(StateSequence id) noexcept
    -> void
{
    OT_ASSERT(0_uz < id);

    if (0_uz == reorgs_.count(id)) {
        reorgs_.emplace(id);
        state_ = State::reorg;
        log_(OT_PRETTY_CLASS())(name_)(" ready to process reorg ")(id).Flush();
        reorg_.AcknowledgePrepareReorg(
            [this](const auto& header, const auto& lock, auto& params) {
                return do_reorg(header, lock, params);
            });
    } else {
        LogAbort()(OT_PRETTY_CLASS())(name_)(" reorg ")(id)(" already handled")
            .Abort();
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

        OT_ASSERT(0 < keys.size());
        // TODO the assertion below will not always be true in the future but
        // for now it will catch some bugs
        OT_ASSERT(1 == keys.size());

        for (const auto& key : keys) {
            const auto& [account, subchain, index] = key;

            OT_ASSERT(false == account.empty());
            // TODO the assertion below will not always be true in the future
            // but for now it will catch some bugs
            OT_ASSERT(account == id_);

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
