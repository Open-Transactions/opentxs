// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/ScriptForm.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(SubchainJobs job) noexcept -> std::string_view
{
    try {
        using Job = SubchainJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::filter, "filter"},
            {Job::mempool, "mempool"},
            {Job::block, "block"},
            {Job::reorg_begin, "reorg_begin"},
            {Job::reorg_begin_ack, "reorg_begin_ack"},
            {Job::reorg_end, "reorg_end"},
            {Job::reorg_end_ack, "reorg_end_ack"},
            {Job::startup, "startup"},
            {Job::startup, "startup"},
            {Job::update, "update"},
            {Job::init, "init"},
            {Job::key, "key"},
            {Job::shutdown_begin, "shutdown_begin"},
            {Job::shutdown_ready, "shutdown_ready"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)(": invalid SubchainJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
SubchainStateData::SubchainStateData(
    const api::Session& api,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const crypto::SubaccountType accountType,
    const cfilter::Type filter,
    const Subchain subchain,
    const network::zeromq::BatchID batch,
    OTNymID&& owner,
    OTIdentifier&& id,
    const std::string_view display,
    const std::string_view fromParent,
    const std::string_view toParent,
    CString&& fromChildren,
    CString&& toChildren,
    CString&& toScan,
    CString&& toProgress,
    allocator_type alloc) noexcept
    : Actor(
          api,
          LogTrace(),
          0ms,
          batch,
          alloc,
          {
              {CString{fromParent, alloc}, Direction::Connect},
          },
          {
              {fromChildren, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {CString{toParent, alloc}, Direction::Connect},
               }},
              {SocketType::Publish,
               {
                   {toChildren, Direction::Bind},
               }},
              {SocketType::Push,
               {
                   {toScan, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {toProgress, Direction::Connect},
               }},
          })
    , api_(api)
    , node_(node)
    , db_(db)
    , mempool_oracle_(mempool)
    , owner_(std::move(owner))
    , account_type_(accountType)
    , id_(std::move(id))
    , subchain_(subchain)
    , chain_(node_.Chain())
    , filter_type_(filter)
    , name_(describe(chain_, id_, display, subchain_, alloc))
    , db_key_(db.GetSubchainID(id_, subchain_))
    , null_position_(make_blank<block::Position>::value(api_))
    , genesis_(node_.HeaderOracle().GetPosition(0))
    , to_children_endpoint_(std::move(toChildren))
    , from_children_endpoint_(std::move(fromChildren))
    , to_index_endpoint_(network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , to_scan_endpoint_(std::move(toScan))
    , to_rescan_endpoint_(
          network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , to_process_endpoint_(
          network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , to_progress_endpoint_(std::move(toProgress))
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , to_children_(pipeline_.Internal().ExtraSocket(1))
    , to_scan_(pipeline_.Internal().ExtraSocket(2))
    , to_progress_(pipeline_.Internal().ExtraSocket(3))
    , state_(State::normal)
    , reorg_(std::nullopt)
    , progress_(std::nullopt)
    , rescan_(std::nullopt)
    , index_(std::nullopt)
    , process_(std::nullopt)
    , scan_(std::nullopt)
    , me_()
{
    OT_ASSERT(false == owner_->empty());
    OT_ASSERT(false == id_->empty());
}

SubchainStateData::SubchainStateData(
    const api::Session& api,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const crypto::SubaccountType accountType,
    const cfilter::Type filter,
    const Subchain subchain,
    const network::zeromq::BatchID batch,
    OTNymID&& owner,
    OTIdentifier&& id,
    const std::string_view display,
    const std::string_view fromParent,
    const std::string_view toParent,
    allocator_type alloc) noexcept
    : SubchainStateData(
          api,
          node,
          db,
          mempool,
          accountType,
          filter,
          subchain,
          batch,
          std::move(owner),
          std::move(id),
          display,
          fromParent,
          toParent,
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          alloc)
{
}

auto SubchainStateData::describe(
    const blockchain::Type chain,
    const Identifier& id,
    const std::string_view type,
    const Subchain subchain,
    allocator_type alloc) noexcept -> CString
{
    // TODO c++20 use allocator
    auto out = std::stringstream{};
    out << print(chain) << ' ';
    out << type;
    out << " account ";
    out << id.str();
    out << ' ';

    switch (subchain) {
        case Subchain::Internal: {
            out << "internal";
        } break;
        case Subchain::External: {
            out << "external";
        } break;
        case Subchain::Incoming: {
            out << "incoming";
        } break;
        case Subchain::Outgoing: {
            out << "outgoing";
        } break;
        case Subchain::Notification: {
            out << "notification";
        } break;
        default: {

            OT_FAIL;
        }
    }

    out << " subchain";

    return CString{alloc} + out.str().c_str();
}

auto SubchainStateData::do_reorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position ancestor) noexcept -> void
{
    LogTrace()(OT_PRETTY_CLASS())(name_)(" processing reorg to ")(
        ancestor.second->asHex())(" at height ")(ancestor.first)
        .Flush();
    const auto tip = db_.SubchainLastScanned(db_key_);
    // TODO use ancestor
    const auto& headers = node_.HeaderOracle();

    try {
        const auto reorg =
            headers.Internal().CalculateReorg(headerOracleLock, tip);

        if (0u == reorg.size()) {
            LogTrace()(OT_PRETTY_CLASS())(
                name_)(" no action required for this subchain")
                .Flush();

            return;
        } else {
            LogTrace()(OT_PRETTY_CLASS())(name_)(" ")(reorg.size())(
                " previously mined blocks have been invalidated")
                .Flush();
        }

        if (db_.ReorgTo(
                headerOracleLock,
                tx,
                headers,
                id_,
                subchain_,
                db_key_,
                reorg)) {
            scan_->ProcessReorg(ancestor);
            process_->ProcessReorg(ancestor);
            index_->ProcessReorg(ancestor);
            rescan_->ProcessReorg(ancestor);
            progress_->ProcessReorg(ancestor);
        } else {

            ++errors;
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            name_)(" header oracle claims existing tip ")(tip.second->asHex())(
            " at height ")(tip.first)(" is invalid")
            .Flush();
        ++errors;
    }
}

auto SubchainStateData::do_shutdown() noexcept -> void
{
    scan_.reset();
    process_.reset();
    index_.reset();
    rescan_.reset();
    progress_.reset();
    to_children_.Send(MakeWork(WorkType::Shutdown));
}

auto SubchainStateData::get_account_targets() const noexcept
    -> std::tuple<Patterns, UTXOs, Targets>
{
    auto out = std::tuple<Patterns, UTXOs, Targets>{};
    auto& [elements, utxos, targets] = out;
    elements = db_.GetPatterns(db_key_);
    utxos = db_.GetUnspentOutputs(id_, subchain_);
    get_targets(elements, utxos, targets);

    return out;
}

auto SubchainStateData::get_block_targets(
    const block::Hash& id,
    const UTXOs& utxos) const noexcept -> std::pair<Patterns, Targets>
{
    auto out = std::pair<Patterns, Targets>{};
    auto& [elements, targets] = out;
    elements = db_.GetUntestedPatterns(db_key_, id.Bytes());
    get_targets(elements, utxos, targets);

    return out;
}

auto SubchainStateData::get_block_targets(const block::Hash& id, Tested& tested)
    const noexcept -> std::tuple<Patterns, UTXOs, Targets, Patterns>
{
    auto out = std::tuple<Patterns, UTXOs, Targets, Patterns>{};
    auto& [elements, utxos, targets, outpoints] = out;
    elements = db_.GetUntestedPatterns(db_key_, id.Bytes());
    utxos = db_.GetUnspentOutputs(id_, subchain_);
    get_targets(elements, utxos, targets, outpoints, tested);

    return out;
}

// NOTE: this version is for matching before a block is downloaded
auto SubchainStateData::get_targets(
    const Patterns& elements,
    const UnallocatedVector<WalletDatabase::UTXO>& utxos,
    Targets& targets) const noexcept -> void
{
    targets.reserve(elements.size() + utxos.size());

    for (const auto& element : elements) {
        const auto& [id, data] = element;
        targets.emplace_back(reader(data));
    }

    switch (filter_type_) {
        case cfilter::Type::Basic_BCHVariant:
        case cfilter::Type::ES: {
            for (const auto& [outpoint, proto] : utxos) {
                targets.emplace_back(outpoint.Bytes());
            }
        } break;
        case cfilter::Type::Basic_BIP158:
        default: {
        }
    }
}

// NOTE: this version is for matching after a block is downloaded
auto SubchainStateData::get_targets(
    const Patterns& elements,
    const UnallocatedVector<WalletDatabase::UTXO>& utxos,
    Targets& targets,
    Patterns& outpoints,
    Tested& tested) const noexcept -> void
{
    targets.reserve(elements.size());
    outpoints.reserve(utxos.size());

    for (const auto& element : elements) {
        const auto& [id, data] = element;
        const auto& [index, subchain] = id;
        targets.emplace_back(reader(data));
        tested.emplace_back(index);
    }

    translate(utxos, outpoints);
}

auto SubchainStateData::IndexElement(
    const cfilter::Type type,
    const blockchain::crypto::Element& input,
    const Bip32Index index,
    WalletDatabase::ElementMap& output) const noexcept -> void
{
    LogVerbose()(OT_PRETTY_CLASS())(name_)(" element ")(
        index)(" extracting filter matching patterns")
        .Flush();
    auto& list = output[index];
    const auto scripts = supported_scripts(input);

    switch (type) {
        case cfilter::Type::ES: {
            for (const auto& [sw, p, s, e, script] : scripts) {
                for (const auto& element : e) {
                    list.emplace_back(space(element));
                }
            }
        } break;
        case cfilter::Type::Basic_BIP158:
        case cfilter::Type::Basic_BCHVariant:
        default: {
            for (const auto& [sw, p, s, e, script] : scripts) {
                script->Serialize(writer(list.emplace_back()));
            }
        }
    }
}

auto SubchainStateData::Init(boost::shared_ptr<SubchainStateData> me) noexcept
    -> void
{
    me_ = std::move(me);
    signal_startup(me_);
}

auto SubchainStateData::pipeline(const Work work, Message&& msg) noexcept
    -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::pre_reorg: {
            state_pre_reorg(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::post_reorg: {
            state_post_reorg(work, std::move(msg));
        } break;
        case State::pre_shutdown: {
            state_pre_shutdown(work, std::move(msg));
        } break;
        case State::shutdown: {
            // NOTE do not process any messages
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto SubchainStateData::ProcessBlock(
    const block::Position& position,
    const block::bitcoin::Block& block) const noexcept -> void
{
    const auto start = Clock::now();
    const auto& name = name_;
    const auto& type = filter_type_;
    const auto& node = node_;
    const auto& filters = node.FilterOracleInternal();
    const auto& blockHash = position.second.get();
    auto matches = Indices{};
    auto [elements, utxos, targets, outpoints] =
        get_block_targets(blockHash, matches);
    const auto pFilter = filters.LoadFilter(type, blockHash);

    OT_ASSERT(pFilter);

    const auto& filter = *pFilter;
    auto potential = node::internal::WalletDatabase::Patterns{};

    for (const auto& it : filter.Match(targets)) {
        // NOTE GCS::Match returns const_iterators to items in the input vector
        const auto pos = std::distance(targets.cbegin(), it);
        auto& [id, element] = elements.at(pos);
        potential.emplace_back(std::move(id), std::move(element));
    }

    const auto confirmed =
        block.Internal().FindMatches(type, outpoints, potential);
    const auto& [utxo, general] = confirmed;
    const auto& oracle = node.HeaderOracle();
    const auto pHeader = oracle.LoadHeader(blockHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;

    OT_ASSERT(position == header.Position());

    handle_confirmed_matches(block, position, confirmed);
    log_(OT_PRETTY_CLASS())(name)(" block ")(block.ID().asHex())(" at height ")(
        position.first)(" processed in ")(
        std::chrono::nanoseconds{Clock::now() - start})
        .Flush();
    log_(OT_PRETTY_CLASS())(name)(" ")(general.size())(" of ")(
        potential.size())(" potential key matches confirmed.")
        .Flush();
    log_(OT_PRETTY_CLASS())(name)(" ")(utxo.size())(" of ")(outpoints.size())(
        " potential utxo matches confirmed.")
        .Flush();
    const auto db = db_.SubchainMatchBlock(db_key_, [&] {
        auto out = UnallocatedVector<std::pair<ReadView, Indices>>{};
        out.emplace_back(position.second->Bytes(), [&] {
            auto out = Indices{};

            for (const auto& [id, element] : potential) {
                const auto& [index, subchain] = id;
                out.emplace_back(index);
            }

            return out;
        }());

        return out;
    }());

    OT_ASSERT(db);  // TODO handle database errors
}

auto SubchainStateData::ProcessTransaction(
    const block::bitcoin::Transaction& tx) const noexcept -> void
{
    const auto targets = get_account_targets();
    const auto& [elements, utxos, patterns] = targets;
    const auto parsed = block::ParsedPatterns{elements};
    const auto outpoints = [&] {
        auto out = SubchainStateData::Patterns{};
        translate(std::get<1>(targets), out);

        return out;
    }();
    auto copy = tx.clone();

    OT_ASSERT(copy);

    const auto matches =
        copy->Internal().FindMatches(filter_type_, outpoints, parsed);
    handle_mempool_matches(matches, std::move(copy));
}

auto SubchainStateData::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& ancestor) noexcept -> void
{
    do_reorg(headerOracleLock, tx, errors, ancestor);
}

auto SubchainStateData::ready_for_normal() noexcept -> void
{
    disable_automatic_processing_ = false;
    reorg_.reset();
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    to_parent_.Send(MakeWork(AccountJobs::reorg_end_ack));
    flush_cache();
}

auto SubchainStateData::ready_for_reorg() noexcept -> void
{
    state_ = State::reorg;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to reorg state ").Flush();
    to_parent_.Send(MakeWork(AccountJobs::reorg_begin_ack));
}

auto SubchainStateData::ReportScan(const block::Position& pos) const noexcept
    -> void
{
    api_.Crypto().Blockchain().Internal().ReportScan(
        node_.Chain(), owner_, account_type_, id_, subchain_, pos);
}

auto SubchainStateData::reorg_children() const noexcept -> std::size_t
{
    return 1u;
}

auto SubchainStateData::Scan(
    const block::Position best,
    const block::Height stop,
    block::Position& highestTested,
    Vector<ScanStatus>& out) const noexcept -> std::optional<block::Position>
{
    const auto& api = api_;
    const auto& name = name_;
    const auto& node = node_;
    const auto& type = filter_type_;
    const auto& headers = node.HeaderOracle();
    const auto& filters = node.FilterOracleInternal();
    const auto start = Clock::now();
    const auto startHeight = highestTested.first + 1;
    const auto stopHeight = std::min(
        std::min<block::Height>(startHeight + scan_batch_ - 1, best.first),
        stop);
    auto atLeastOnce{false};
    auto highestClean = std::optional<block::Position>{std::nullopt};

    if (startHeight > stopHeight) {
        log_(OT_PRETTY_CLASS())(name)(" attempted to scan filters from ")(
            startHeight)(" to ")(stopHeight)(" but this is impossible")
            .Flush();

        return highestClean;
    }

    log_(OT_PRETTY_CLASS())(name)(" scanning filters from ")(
        startHeight)(" to ")(stopHeight)
        .Flush();
    const auto targets = get_account_targets();
    auto blockHash = api.Factory().Data();
    auto isClean{true};
    // TODO have the filter oracle provide all filters as a single batch

    for (auto i{startHeight}; i <= stopHeight; ++i) {
        blockHash = headers.BestHash(i, best);

        if (blockHash->empty()) {
            log_(OT_PRETTY_CLASS())(name)(
                " interrupting scan due to chain reorg")
                .Flush();

            break;
        }

        auto testPosition = block::Position{i, blockHash};
        const auto pFilter = filters.LoadFilterOrResetTip(type, testPosition);

        if (false == bool(pFilter)) {
            log_(OT_PRETTY_CLASS())(name)(" filter at height ")(
                i)(" not found ")
                .Flush();

            break;
        }

        atLeastOnce = true;
        const auto hasMatches = [&] {
            const auto& [elements, utxos, patterns] = targets;
            const auto& filter = *pFilter;
            auto matches = filter.Match(patterns);

            if (0 < matches.size()) {
                const auto [untested, retest] =
                    get_block_targets(blockHash, utxos);
                matches = filter.Match(retest);

                if (0 < matches.size()) {
                    log_(OT_PRETTY_CLASS())(name)(" GCS scan for block ")(
                        blockHash->asHex())(" at height ")(i)(" found ")(
                        matches.size())(" new potential matches for the ")(
                        patterns.size())(" target elements for ")(id_)
                        .Flush();

                    return true;
                }
            }

            return false;
        }();

        if (hasMatches) {
            isClean = false;
            out.emplace_back(ScanState::dirty, testPosition);
        } else if (isClean) {
            highestClean = testPosition;
        }

        highestTested = std::move(testPosition);
    }

    if (atLeastOnce) {
        const auto count = out.size();
        log_(OT_PRETTY_CLASS())(name)(" scan found ")(
            count)(" new potential matches between blocks ")(
            startHeight)(" and ")(highestTested.first)(" in ")(
            std::chrono::nanoseconds{Clock::now() - start})
            .Flush();
    } else {
        log_(OT_PRETTY_CLASS())(name)(" scan interrupted due to missing filter")
            .Flush();
    }

    return highestClean;
}

auto SubchainStateData::set_key_data(
    block::bitcoin::Transaction& tx) const noexcept -> void
{
    const auto keys = tx.Keys();
    auto data = block::KeyData{};
    const auto& api = api_.Crypto().Blockchain();

    for (const auto& key : keys) {
        data.try_emplace(
            key, api.SenderContact(key), api.RecipientContact(key));
    }

    tx.Internal().SetKeyData(data);
}

auto SubchainStateData::startup() noexcept -> void
{
    progress_.emplace(me_);
    rescan_.emplace(me_);
    index_.emplace(get_index(me_));
    process_.emplace(me_);
    scan_.emplace(me_);
    me_.reset();
    do_work();
}

auto SubchainStateData::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(false == reorg_.has_value());

    switch (work) {
        case Work::reorg_begin: {
            transition_state_pre_reorg(std::move(msg));
        } break;
        case Work::reorg_end:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::shutdown_ready: {
            LogError()(OT_PRETTY_CLASS())("wrong state for ")(print(work))(
                " message")
                .Flush();

            OT_FAIL;
        }
        case Work::shutdown_begin: {
            transition_state_pre_shutdown(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::startup:
        case Work::update:
        case Work::init:
        case Work::key:
        case Work::statemachine:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto SubchainStateData::state_post_reorg(
    const Work work,
    Message&& msg) noexcept -> void
{
    OT_ASSERT(reorg_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_end_ack: {
            transition_state_normal(std::move(msg));
        } break;
        case Work::reorg_begin:
        case Work::reorg_begin_ack:
        case Work::reorg_end:
        case Work::shutdown_begin:
        case Work::shutdown_ready: {
            LogError()(OT_PRETTY_CLASS())("wrong state for ")(print(work))(
                " message")
                .Flush();

            OT_FAIL;
        }
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::startup:
        case Work::update:
        case Work::init:
        case Work::key:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto SubchainStateData::state_pre_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(reorg_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_begin_ack: {
            transition_state_reorg(std::move(msg));
        } break;
        case Work::reorg_begin:
        case Work::reorg_end:
        case Work::reorg_end_ack:
        case Work::shutdown_begin:
        case Work::shutdown_ready: {
            LogError()(OT_PRETTY_CLASS())("wrong state for ")(print(work))(
                " message")
                .Flush();

            OT_FAIL;
        }
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::startup:
        case Work::update:
        case Work::init:
        case Work::key:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto SubchainStateData::state_pre_shutdown(
    const Work work,
    Message&& msg) noexcept -> void
{
    OT_ASSERT(false == reorg_.has_value());

    switch (work) {
        case Work::reorg_begin:
        case Work::reorg_begin_ack:
        case Work::reorg_end:
        case Work::reorg_end_ack:
        case Work::shutdown_begin: {
            LogError()(OT_PRETTY_CLASS())("wrong state for ")(print(work))(
                " message")
                .Flush();

            OT_FAIL;
        }
        case Work::shutdown_ready: {
            transition_state_shutdown(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::startup:
        case Work::update:
        case Work::init:
        case Work::key:
        case Work::statemachine:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto SubchainStateData::state_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    OT_ASSERT(reorg_.has_value());

    switch (work) {
        case Work::shutdown:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_end: {
            transition_state_post_reorg(std::move(msg));
        } break;
        case Work::reorg_begin:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::shutdown_begin:
        case Work::shutdown_ready: {
            LogError()(OT_PRETTY_CLASS())("wrong state for ")(print(work))(
                " message")
                .Flush();

            OT_FAIL;
        }
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::startup:
        case Work::update:
        case Work::init:
        case Work::key:
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto SubchainStateData::supported_scripts(const crypto::Element& element)
    const noexcept -> UnallocatedVector<ScriptForm>
{
    auto out = UnallocatedVector<ScriptForm>{};
    const auto chain = node_.Chain();
    using Type = ScriptForm::Type;
    out.emplace_back(api_, element, chain, Type::PayToPubkey);
    out.emplace_back(api_, element, chain, Type::PayToPubkeyHash);
    out.emplace_back(api_, element, chain, Type::PayToWitnessPubkeyHash);

    return out;
}

auto SubchainStateData::transition_state_normal(Message&& in) noexcept -> void
{
    OT_ASSERT(0u < reorg_->target_);

    auto& reorg = reorg_.value();
    const auto& target = reorg.target_;
    auto& counter = reorg.done_;
    ++counter;

    if (counter < target) {
        log_(OT_PRETTY_CLASS())(name_)(" ")(counter)(" of ")(
            target)(" children finished with reorg")
            .Flush();

        return;
    } else if (counter == target) {
        log_(OT_PRETTY_CLASS())(name_)(" all ")(
            target)(" children finished with reorg")
            .Flush();
        scan_->VerifyState(Scan::State::normal);
        process_->VerifyState(Process::State::normal);
        index_->VerifyState(Index::State::normal);
        rescan_->VerifyState(Rescan::State::normal);
        progress_->VerifyState(Progress::State::normal);
        ready_for_normal();
    } else {

        OT_FAIL;
    }
}

auto SubchainStateData::transition_state_post_reorg(Message&& in) noexcept
    -> void
{
    const auto& reorg = reorg_.value();

    if (0u < reorg.target_) {
        log_(OT_PRETTY_CLASS())(name_)(" waiting for acknowledgements from ")(
            reorg_->target_)(
            " children prior to acknowledging reorg completion")
            .Flush();
        to_progress_.Send(MakeWork(SubchainJobs::reorg_end));
        state_ = State::post_reorg;
    } else {
        log_(OT_PRETTY_CLASS())(name_)(
            " no children instantiated therefore reorg may be completed "
            "immediately")
            .Flush();
        ready_for_normal();
    }
}

auto SubchainStateData::transition_state_pre_reorg(Message&& in) noexcept
    -> void
{
    disable_automatic_processing_ = true;
    reorg_.emplace(reorg_children());
    const auto& reorg = reorg_.value();

    if (0u < reorg.target_) {
        log_(OT_PRETTY_CLASS())(name_)(" waiting for acknowledgements from ")(
            reorg_->target_)(" children prior to acknowledging reorg")
            .Flush();
        to_scan_.Send(MakeWork(SubchainJobs::reorg_begin));
        state_ = State::pre_reorg;
    } else {
        log_(OT_PRETTY_CLASS())(name_)(
            " no children instantiated therefore reorg may be acknowledged "
            "immediately")
            .Flush();
        ready_for_reorg();
    }
}

auto SubchainStateData::transition_state_pre_shutdown(Message&& in) noexcept
    -> void
{
    state_ = State::pre_shutdown;
    to_scan_.Send(std::move(in));
}

auto SubchainStateData::transition_state_reorg(Message&& in) noexcept -> void
{
    OT_ASSERT(0u < reorg_->target_);

    auto& reorg = reorg_.value();
    const auto& target = reorg.target_;
    auto& counter = reorg.ready_;
    ++counter;

    if (counter < target) {
        log_(OT_PRETTY_CLASS())(name_)(" ")(counter)(" of ")(
            target)(" children ready for reorg")
            .Flush();

        return;
    } else if (counter == target) {
        log_(OT_PRETTY_CLASS())(name_)(" all ")(
            target)(" children ready for reorg")
            .Flush();
        progress_->VerifyState(Progress::State::reorg);
        rescan_->VerifyState(Rescan::State::reorg);
        index_->VerifyState(Index::State::reorg);
        process_->VerifyState(Process::State::reorg);
        scan_->VerifyState(Scan::State::reorg);
        ready_for_reorg();
    } else {

        OT_FAIL;
    }
}

auto SubchainStateData::transition_state_shutdown(Message&& in) noexcept -> void
{
    state_ = State::shutdown;
    progress_->VerifyState(Progress::State::shutdown);
    rescan_->VerifyState(Rescan::State::shutdown);
    index_->VerifyState(Index::State::shutdown);
    process_->VerifyState(Process::State::shutdown);
    scan_->VerifyState(Scan::State::shutdown);
    to_parent_.Send(std::move(in));
}

auto SubchainStateData::translate(
    const UnallocatedVector<WalletDatabase::UTXO>& utxos,
    Patterns& outpoints) const noexcept -> void
{
    for (const auto& [outpoint, output] : utxos) {
        OT_ASSERT(output);

        auto keys = output->Keys();

        OT_ASSERT(0 < keys.size());
        // TODO the assertion below will not always be true in the future but
        // for now it will catch some bugs
        OT_ASSERT(1 == keys.size());

        for (auto& key : keys) {
            const auto& [id, subchain, index] = key;
            auto account = api_.Factory().Identifier(id);

            OT_ASSERT(false == account->empty());
            // TODO the assertion below will not always be true in the future
            // but for now it will catch some bugs
            OT_ASSERT(account == id_);

            outpoints.emplace_back(
                WalletDatabase::ElementID{
                    static_cast<Bip32Index>(index),
                    {static_cast<Subchain>(subchain), std::move(account)}},
                space(outpoint.Bytes()));
        }
    }
}

auto SubchainStateData::VerifyState(const State state) const noexcept -> void
{
    OT_ASSERT(state == state_);
}

auto SubchainStateData::work() noexcept -> bool { return false; }

SubchainStateData::~SubchainStateData() { signal_shutdown(); }
}  // namespace opentxs::blockchain::node::wallet
