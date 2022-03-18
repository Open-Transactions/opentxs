// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <array>
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
#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Timer.hpp"
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
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ByteLiterals.hpp"

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
            {Job::update, "update"},
            {Job::prepare_shutdown, "prepare_shutdown"},
            {Job::init, "init"},
            {Job::key, "key"},
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
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const crypto::SubaccountType accountType,
    const cfilter::Type filter,
    const Subchain subchain,
    const network::zeromq::BatchID batch,
    OTNymID&& owner,
    OTIdentifier&& id,
    const std::string_view display,
    const std::string_view parent,
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
              {CString{parent, alloc}, Direction::Connect},
          },
          {
              {fromChildren, Direction::Bind},
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
    , to_index_endpoint_(network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , to_scan_endpoint_(std::move(toScan))
    , to_rescan_endpoint_(
          network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , to_process_endpoint_(
          network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , to_progress_endpoint_(std::move(toProgress))
    , shutdown_endpoint_(parent, alloc)
    , pending_state_(State::normal)
    , state_(State::normal)
    , progress_(std::nullopt)
    , rescan_(std::nullopt)
    , index_(std::nullopt)
    , process_(std::nullopt)
    , scan_(std::nullopt)
{
    OT_ASSERT(false == owner_->empty());
    OT_ASSERT(false == id_->empty());
}

SubchainStateData::SubchainStateData(
    const api::Session& api,
    const node::internal::Network& node,
    node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const crypto::SubaccountType accountType,
    const cfilter::Type filter,
    const Subchain subchain,
    const network::zeromq::BatchID batch,
    OTNymID&& owner,
    OTIdentifier&& id,
    const std::string_view display,
    const std::string_view parent,
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
          parent,
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          network::zeromq::MakeArbitraryInproc(alloc.resource()),
          alloc)
{
}

auto SubchainStateData::ChangeState(const State state) noexcept -> bool
{
    if (auto old = pending_state_.exchange(state); old == state) {

        return true;
    }

    auto lock = lock_for_reorg(reorg_lock_);

    switch (state) {
        case State::normal: {
            OT_ASSERT(State::reorg == state_);

            transition_state_normal();
        } break;
        case State::reorg: {
            OT_ASSERT(State::normal == state_);

            transition_state_reorg();
        } break;
        case State::shutdown: {
            OT_ASSERT(State::reorg != state_);

            transition_state_shutdown();
        } break;
        default: {
            OT_FAIL;
        }
    }

    return true;
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
    log_(OT_PRETTY_CLASS())(name_)(" processing reorg to ")(
        ancestor.second->asHex())(" at height ")(ancestor.first)
        .Flush();
    const auto tip = db_.SubchainLastScanned(db_key_);
    // TODO use ancestor
    const auto& headers = node_.HeaderOracle();

    try {
        const auto reorg =
            headers.Internal().CalculateReorg(headerOracleLock, tip);

        if (0u == reorg.size()) {
            log_(OT_PRETTY_CLASS())(name_)(
                " no action required for this subchain")
                .Flush();

            return;
        } else {
            log_(OT_PRETTY_CLASS())(name_)(" ")(reorg.size())(
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
}

auto SubchainStateData::do_startup() noexcept -> void
{
    progress_.emplace(*this);
    rescan_.emplace(*this);
    index_.emplace(get_index(*this));
    process_.emplace(*this);
    scan_.emplace(*this);
    do_work();
}

auto SubchainStateData::get_account_targets(alloc::Resource* alloc)
    const noexcept -> std::tuple<Patterns, UTXOs, Targets>
{
    auto out = std::tuple<Patterns, UTXOs, Targets>{};
    auto& [elements, utxos, targets] = out;
    elements = db_.GetPatterns(db_key_, alloc);
    utxos = db_.GetUnspentOutputs(id_, subchain_, alloc);
    get_targets(elements, utxos, targets);

    return out;
}

auto SubchainStateData::get_block_targets(
    const block::Hash& id,
    const UTXOs& utxos,
    alloc::Resource* alloc) const noexcept -> std::pair<Patterns, Targets>
{
    auto out = std::pair<Patterns, Targets>{};
    auto& [elements, targets] = out;
    elements = db_.GetUntestedPatterns(db_key_, id.Bytes(), alloc);
    get_targets(elements, utxos, targets);

    return out;
}

auto SubchainStateData::get_block_targets(
    const block::Hash& id,
    Tested& tested,
    alloc::Resource* alloc) const noexcept
    -> std::tuple<Patterns, UTXOs, Targets, Patterns>
{
    auto out = std::tuple<Patterns, UTXOs, Targets, Patterns>{};
    auto& [elements, utxos, targets, outpoints] = out;
    elements = db_.GetUntestedPatterns(db_key_, id.Bytes(), alloc);
    utxos = db_.GetUnspentOutputs(id_, subchain_, alloc);
    get_targets(elements, utxos, targets, outpoints, tested);

    return out;
}

// NOTE: this version is for matching before a block is downloaded
auto SubchainStateData::get_targets(
    const Patterns& elements,
    const Vector<WalletDatabase::UTXO>& utxos,
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
    const Vector<WalletDatabase::UTXO>& utxos,
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
    log_(OT_PRETTY_CLASS())(name_)(" element ")(
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
    signal_startup(me);
}

auto SubchainStateData::pipeline(const Work work, Message&& msg) noexcept
    -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::shutdown: {
            shutdown_actor();
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
    auto buf = std::array<std::byte, 16_KiB>{};
    auto alloc = alloc::BoostMonotonic{
        buf.data(),
        buf.size(),
        alloc::standard_to_boost(get_allocator().resource())};
    auto [elements, utxos, targets, outpoints] =
        get_block_targets(blockHash, matches, &alloc);
    const auto haveTargets = Clock::now();
    const auto pFilter = filters.LoadFilter(type, blockHash);

    OT_ASSERT(pFilter);

    const auto& filter = *pFilter;
    const auto haveFilter = Clock::now();
    auto potential = node::internal::WalletDatabase::Patterns{};

    for (const auto& it : filter.Match(targets)) {
        // NOTE GCS::Match returns const_iterators to items in the input vector
        const auto pos = std::distance(targets.cbegin(), it);
        auto& [id, element] = elements.at(pos);
        potential.emplace_back(std::move(id), std::move(element));
    }

    const auto confirmed =
        block.Internal().FindMatches(type, outpoints, potential);
    const auto haveMatches = Clock::now();
    const auto& [utxo, general] = confirmed;
    const auto& oracle = node.HeaderOracle();
    const auto pHeader = oracle.LoadHeader(blockHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;

    OT_ASSERT(position == header.Position());

    handle_confirmed_matches(block, position, confirmed);
    const auto handledMatches = Clock::now();
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
    const auto updateDB = Clock::now();

    OT_ASSERT(db);  // TODO handle database errors

    log_(OT_PRETTY_CLASS())(name)(" block ")(print(position))(" processed in ")(
        std::chrono::nanoseconds{Clock::now() - start})
        .Flush();
    log_(OT_PRETTY_CLASS())(name)(" ")(general.size())(" of ")(
        potential.size())(" potential key matches confirmed.")
        .Flush();
    log_(OT_PRETTY_CLASS())(name)(" ")(utxo.size())(" of ")(outpoints.size())(
        " potential utxo matches confirmed.")
        .Flush();
    log_(OT_PRETTY_CLASS())(name)(" time to load match targets: ")(
        std::chrono::nanoseconds{haveTargets - start})
        .Flush();
    log_(OT_PRETTY_CLASS())(name)(" time to load filter: ")(
        std::chrono::nanoseconds{haveFilter - haveTargets})
        .Flush();
    log_(OT_PRETTY_CLASS())(name)(" time to find matches: ")(
        std::chrono::nanoseconds{haveMatches - haveFilter})
        .Flush();
    log_(OT_PRETTY_CLASS())(name)(" time to handle matches: ")(
        std::chrono::nanoseconds{handledMatches - haveMatches})
        .Flush();
    log_(OT_PRETTY_CLASS())(name)(" time to update database: ")(
        std::chrono::nanoseconds{updateDB - handledMatches})
        .Flush();
}

auto SubchainStateData::ProcessTransaction(
    const block::bitcoin::Transaction& tx) const noexcept -> void
{
    auto buf = std::array<std::byte, 4_KiB>{};
    auto alloc = alloc::BoostMonotonic{
        buf.data(),
        buf.size(),
        alloc::standard_to_boost(get_allocator().resource())};
    const auto targets = get_account_targets(&alloc);
    const auto& [elements, utxos, patterns] = targets;
    const auto parsed = block::ParsedPatterns{elements};
    const auto outpoints = [&] {
        auto out = SubchainStateData::Patterns{&alloc};
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

auto SubchainStateData::ReportScan(const block::Position& pos) const noexcept
    -> void
{
    api_.Crypto().Blockchain().Internal().ReportScan(
        chain_, owner_, account_type_, id_, subchain_, pos);
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

        return std::nullopt;
    }

    log_(OT_PRETTY_CLASS())(name)(" scanning filters from ")(
        startHeight)(" to ")(stopHeight)
        .Flush();
    auto* upstream = alloc::standard_to_boost(get_allocator().resource());
    // TODO adjust this once Data and GCS are allocator aware
    static constexpr auto allocBytes =
        (scan_batch_ *
         (sizeof(block::pHash) + sizeof(std::unique_ptr<const GCS>))) +
        4_KiB;
    auto alloc = alloc::BoostMonotonic{allocBytes, upstream};
    const auto targets = get_account_targets(&alloc);
    const auto target = static_cast<std::size_t>(stopHeight - startHeight + 1);
    const auto blocks = headers.BestHashes(startHeight, target, &alloc);
    const auto cfilters = filters.LoadFilters(type, blocks);

    OT_ASSERT(cfilters.size() <= blocks.size());

    auto isClean{true};

    auto b = blocks.begin();
    auto f = cfilters.begin();
    auto i = startHeight;

    for (auto end = cfilters.end(); f != end; ++f, ++b, ++i) {
        if (auto state = pending_state_.load(); state != state_) {
            LogConsole()(name)(" scan interrupted due to ")(print(state))(
                " state change")
                .Flush();

            break;
        }

        const auto& blockHash = b->get();
        const auto& pFilter = *f;
        auto testPosition = block::Position{i, blockHash};

        if (blockHash.empty()) {
            LogError()(OT_PRETTY_CLASS())(name)(" empty block hash").Flush();

            break;
        }

        if (false == bool(pFilter)) {
            LogError()(OT_PRETTY_CLASS())(name)(" filter for block ")(
                print(testPosition))(" not found ")
                .Flush();

            break;
        }

        const auto& filter = *pFilter;
        atLeastOnce = true;
        const auto hasMatches = [&] {
            const auto& [elements, utxos, patterns] = targets;
            auto matches = filter.Match(patterns);

            if (0 < matches.size()) {
                const auto [untested, retest] =
                    get_block_targets(blockHash, utxos, &alloc);
                matches = filter.Match(retest);

                if (0 < matches.size()) {
                    log_(OT_PRETTY_CLASS())(name)(" GCS scan for block ")(
                        blockHash.asHex())(" at height ")(i)(" found ")(
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
        log_(OT_PRETTY_CLASS())(name)(" scan interrupted").Flush();
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

auto SubchainStateData::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::shutdown: {
            shutdown_actor();
        } break;
        case Work::prepare_shutdown: {
            transition_state_shutdown();
        } break;
        case Work::init: {
            do_init();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::update:
        case Work::key:
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
    switch (work) {
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::shutdown: {
            LogError()(OT_PRETTY_CLASS())("wrong state for ")(print(work))(
                " message")
                .Flush();

            OT_FAIL;
        }
        case Work::filter:
        case Work::mempool:
        case Work::block:
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
    using Type = ScriptForm::Type;
    out.emplace_back(api_, element, chain_, Type::PayToPubkey);
    out.emplace_back(api_, element, chain_, Type::PayToPubkeyHash);
    out.emplace_back(api_, element, chain_, Type::PayToWitnessPubkeyHash);

    return out;
}

auto SubchainStateData::transition_state_normal() noexcept -> void
{
    disable_automatic_processing_ = false;
    auto rc = scan_->ChangeState(JobState::normal);

    OT_ASSERT(rc);

    rc = process_->ChangeState(JobState::normal);

    OT_ASSERT(rc);

    rc = index_->ChangeState(JobState::normal);

    OT_ASSERT(rc);

    rc = rescan_->ChangeState(JobState::normal);

    OT_ASSERT(rc);

    rc = progress_->ChangeState(JobState::normal);

    OT_ASSERT(rc);

    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    trigger();
}

auto SubchainStateData::transition_state_reorg() noexcept -> void
{
    disable_automatic_processing_ = true;
    auto rc = scan_->ChangeState(JobState::reorg);

    OT_ASSERT(rc);

    rc = process_->ChangeState(JobState::reorg);

    OT_ASSERT(rc);

    rc = index_->ChangeState(JobState::reorg);

    OT_ASSERT(rc);

    rc = rescan_->ChangeState(JobState::reorg);

    OT_ASSERT(rc);

    rc = progress_->ChangeState(JobState::reorg);

    OT_ASSERT(rc);

    state_ = State::reorg;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to reorg state ").Flush();
}

auto SubchainStateData::transition_state_shutdown() noexcept -> void
{
    auto rc = scan_->ChangeState(JobState::shutdown);

    OT_ASSERT(rc);

    rc = process_->ChangeState(JobState::shutdown);

    OT_ASSERT(rc);

    rc = index_->ChangeState(JobState::shutdown);

    OT_ASSERT(rc);

    rc = rescan_->ChangeState(JobState::shutdown);

    OT_ASSERT(rc);

    rc = progress_->ChangeState(JobState::shutdown);

    OT_ASSERT(rc);

    state_ = State::shutdown;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to shutdown state ").Flush();
    signal_shutdown();
}

auto SubchainStateData::translate(
    const Vector<WalletDatabase::UTXO>& utxos,
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

auto SubchainStateData::work() noexcept -> bool { return false; }
}  // namespace opentxs::blockchain::node::wallet
