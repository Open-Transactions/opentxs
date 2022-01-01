// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/SubchainStateData.hpp"  // IWYU pragma: associated

#include <chrono>
#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/Index.hpp"
#include "blockchain/node/wallet/ScriptForm.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "util/JobCounter.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
SubchainStateData::SubchainStateData(
    const api::Session& api,
    const node::internal::Network& node,
    Accounts& parent,
    const WalletDatabase& db,
    OTNymID&& owner,
    crypto::SubaccountType accountType,
    OTIdentifier&& id,
    const std::function<void(const Identifier&, const char*)>& taskFinished,
    Outstanding& jobCounter,
    const filter::Type filter,
    const Subchain subchain) noexcept
    : api_(api)
    , node_(node)
    , parent_(parent)
    , db_(db)
    , task_finished_(taskFinished)
    , name_()
    , owner_(std::move(owner))
    , account_type_(accountType)
    , id_(std::move(id))
    , subchain_(subchain)
    , filter_type_(filter)
    , db_key_(db.GetSubchainID(id_, subchain_))
    , null_position_(make_blank<block::Position>::value(api_))
    , job_counter_(jobCounter)
    , block_index_()
    , progress_(*this)
    , process_(*this, progress_)
    , rescan_(*this, process_, progress_)
    , scan_(*this, process_, rescan_)
    , mempool_(*this)
{
    OT_ASSERT(task_finished_);
    OT_ASSERT(false == owner_->empty());
    OT_ASSERT(false == id_->empty());
}

auto SubchainStateData::describe() const noexcept -> std::string
{
    auto out = type();
    out << " account ";
    out << id_->str();
    out << ' ';

    switch (subchain_) {
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

    return out.str();
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
            scan_.Reorg(ancestor);
            rescan_.Reorg(ancestor);
            process_.Reorg(ancestor);
            progress_.Reorg(ancestor);
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

auto SubchainStateData::FinishBackgroundTasks() noexcept -> void
{
    job_counter_.wait_for_finished();
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
    const std::pmr::vector<WalletDatabase::UTXO>& utxos,
    Targets& targets) const noexcept -> void
{
    targets.reserve(elements.size() + utxos.size());

    for (const auto& element : elements) {
        const auto& [id, data] = element;
        targets.emplace_back(reader(data));
    }

    switch (filter_type_) {
        case filter::Type::Basic_BCHVariant:
        case filter::Type::ES: {
            for (const auto& [outpoint, proto] : utxos) {
                targets.emplace_back(outpoint.Bytes());
            }
        } break;
        case filter::Type::Basic_BIP158:
        default: {
        }
    }
}

// NOTE: this version is for matching after a block is downloaded
auto SubchainStateData::get_targets(
    const Patterns& elements,
    const std::pmr::vector<WalletDatabase::UTXO>& utxos,
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

auto SubchainStateData::index_element(
    const filter::Type type,
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
        case filter::Type::ES: {
            for (const auto& [sw, p, s, e, script] : scripts) {
                for (const auto& element : e) {
                    list.emplace_back(space(element));
                }
            }
        } break;
        case filter::Type::Basic_BIP158:
        case filter::Type::Basic_BCHVariant:
        default: {
            for (const auto& [sw, p, s, e, script] : scripts) {
                script->Serialize(writer(list.emplace_back()));
            }
        }
    }
}

auto SubchainStateData::init() noexcept -> void
{
    const_cast<std::string&>(name_) = describe();
    const auto& mempool = node_.Mempool();
    const auto txids = mempool.Dump();
    auto transactions = Transactions{};
    transactions.reserve(txids.size());

    for (const auto& txid : txids) {
        auto& tx = transactions.emplace_back(mempool.Query(txid));

        if (!tx) { transactions.pop_back(); }
    }

    mempool_.Queue(std::move(transactions));
    progress_.Init();
}

auto SubchainStateData::ProcessBlockAvailable(const block::Hash& block) noexcept
    -> void
{
    if (block_index_.Query(block)) {
        LogInsane()(OT_PRETTY_CLASS())(name_).Flush();
        auto again{true};
        const auto start = Clock::now();
        static constexpr auto limit = std::chrono::minutes{1};

        while (again && ((Clock::now() - start) < limit)) {
            again = process_.Run();
        }

        if (again) {
            LogError()(OT_PRETTY_CLASS())(
                name_)(" failed to obtain lock after ")(
                std::chrono::nanoseconds{Clock::now() - start})
                .Flush();
        }
    }
}

auto SubchainStateData::ProcessKey() noexcept -> void
{
    LogInsane()(OT_PRETTY_CLASS())(name_).Flush();
    get_index().Run();
}

auto SubchainStateData::ProcessMempool(
    std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept -> void
{
    LogInsane()(OT_PRETTY_CLASS())(name_).Flush();

    if (mempool_.Queue(tx)) { mempool_.Run(); }
}

auto SubchainStateData::ProcessNewFilter(const block::Position& tip) noexcept
    -> void
{
    LogInsane()(OT_PRETTY_CLASS())(name_).Flush();
    rescan_.UpdateTip(tip);
    scan_.Run(tip);
}

auto SubchainStateData::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& ancestor) noexcept -> bool
{
    LogInsane()(OT_PRETTY_CLASS())(name_).Flush();
    ++job_counter_;
    const auto queued =
        api_.Network().Asio().Internal().Post(ThreadPool::General, [&] {
            auto post = ScopeGuard{[this] { --job_counter_; }};

            do_reorg(headerOracleLock, tx, errors, ancestor);
        });

    if (queued) {
        LogDebug()(OT_PRETTY_CLASS())(name_)(" reorg job queued").Flush();
    } else {
        LogDebug()(OT_PRETTY_CLASS())(name_)(" failed to queue reorg job")
            .Flush();
        --job_counter_;
        ++errors;
    }

    return queued;
}

auto SubchainStateData::ProcessStateMachine(bool enabled) noexcept -> bool
{
    auto again{false};
    get_index().Run();
    mempool_.Run();

    if (enabled) {
        scan_.Run();
        rescan_.Run();
        again = process_.Run();
    }

    return again;
}

auto SubchainStateData::ProcessTaskComplete(
    const Identifier& id,
    const char* type,
    bool enabled) noexcept -> void
{
    auto again{true};
    const auto& log = LogTrace();

    if (id != db_key_) { return; }

    log(OT_PRETTY_CLASS())(name_)(" ")(type)(" job complete").Flush();
    const auto start = Clock::now();
    static constexpr auto limit = std::chrono::minutes{1};

    while (again && ((Clock::now() - start) < limit)) {
        again = ProcessStateMachine(enabled);
    }

    if (again) {
        log(OT_PRETTY_CLASS())(name_)(" failed to obtain lock after ")(
            std::chrono::nanoseconds{Clock::now() - start})
            .Flush();
    }
}

auto SubchainStateData::report_scan(const block::Position& pos) const noexcept
    -> void
{
    api_.Crypto().Blockchain().Internal().ReportScan(
        node_.Chain(), owner_, account_type_, id_, subchain_, pos);
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

auto SubchainStateData::Shutdown() noexcept -> void
{
    get_index().Shutdown();
    mempool_.Shutdown();
    scan_.Shutdown();
    rescan_.Shutdown();
    process_.Shutdown();
}

auto SubchainStateData::supported_scripts(const crypto::Element& element)
    const noexcept -> std::pmr::vector<ScriptForm>
{
    auto out = std::pmr::vector<ScriptForm>{};
    const auto chain = node_.Chain();
    using Type = ScriptForm::Type;
    out.emplace_back(api_, element, chain, Type::PayToPubkey);
    out.emplace_back(api_, element, chain, Type::PayToPubkeyHash);
    out.emplace_back(api_, element, chain, Type::PayToWitnessPubkeyHash);

    return out;
}

auto SubchainStateData::translate(
    const std::pmr::vector<WalletDatabase::UTXO>& utxos,
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
            auto account = api_.Factory().IdentifierFromBase58(id);

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

auto SubchainStateData::update_scan(const block::Position& pos, bool reorg)
    const noexcept -> void
{
    if (false == reorg) { db_.SubchainSetLastScanned(db_key_, pos); }

    report_scan(pos);
}

SubchainStateData::~SubchainStateData() { FinishBackgroundTasks(); }
}  // namespace opentxs::blockchain::node::wallet
