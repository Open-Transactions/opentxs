// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/ScriptForm.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/JobCounter.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(SubchainJobs job) noexcept -> std::string_view
{
    try {
        static const auto map = Map<SubchainJobs, CString>{
            {SubchainJobs::shutdown, "shutdown"},
            {SubchainJobs::filter, "filter"},
            {SubchainJobs::mempool, "mempool"},
            {SubchainJobs::block, "block"},
            {SubchainJobs::job_finished, "job_finished"},
            {SubchainJobs::reorg_begin, "reorg_begin"},
            {SubchainJobs::reorg_begin_ack, "reorg_begin_ack"},
            {SubchainJobs::reorg_end, "reorg_end"},
            {SubchainJobs::reorg_end_ack, "reorg_end_ack"},
            {SubchainJobs::init, "init"},
            {SubchainJobs::key, "key"},
            {SubchainJobs::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid SubchainJobs: ")(
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
    const filter::Type filter,
    const Subchain subchain,
    const network::zeromq::BatchID batch,
    OTNymID&& owner,
    OTIdentifier&& id,
    const std::string_view shutdown,
    const std::string_view fromParent,
    const std::string_view toParent,
    allocator_type alloc) noexcept
    : Actor(
          api,
          0ms,
          batch,
          alloc,
          {
              {CString{shutdown, alloc}, Direction::Connect},
              {CString{fromParent, alloc}, Direction::Connect},
              {CString{
                   api.Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               Direction::Connect},
              {CString{api.Endpoints().BlockchainBlockAvailable(), alloc},
               Direction::Connect},
              {CString{api.Endpoints().BlockchainMempool(), alloc},
               Direction::Connect},
              {CString{api.Endpoints().BlockchainNewFilter(), alloc},
               Direction::Connect},
          },
          {},
          {},
          {
              {SocketType::Push,
               {
                   {CString{toParent}, Direction::Connect},
               }},
          })
    , api_(api)
    , node_(node)
    , db_(db)
    , mempool_oracle_(mempool)
    , task_finished_([&](const Identifier& id, const char* type) {
        auto work = MakeWork(Work::job_finished);
        work.AddFrame(id.data(), id.size());
        work.AddFrame(CString(type));  // TODO allocator
        pipeline_.Push(std::move(work));
    })
    , name_()
    , owner_(std::move(owner))
    , account_type_(accountType)
    , id_(std::move(id))
    , subchain_(subchain)
    , chain_(node_.Chain())
    , filter_type_(filter)
    , db_key_(db.GetSubchainID(id_, subchain_))
    , null_position_(make_blank<block::Position>::value(api_))
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , block_index_()
    , jobs_()
    , job_counter_(jobs_.Allocate())
    , progress_(*this)
    , process_(*this, progress_)
    , rescan_(*this, process_, progress_)
    , scan_(*this, process_, rescan_)
    , mempool_(*this)
    , state_(State::normal)
{
    OT_ASSERT(task_finished_);
    OT_ASSERT(false == owner_->empty());
    OT_ASSERT(false == id_->empty());
}

auto SubchainStateData::describe() const noexcept -> UnallocatedCString
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

auto SubchainStateData::do_shutdown() noexcept -> void
{
    get_index().Shutdown();
    mempool_.Shutdown();
    scan_.Shutdown();
    rescan_.Shutdown();
    process_.Shutdown();
}

auto SubchainStateData::finish_background_tasks() noexcept -> void
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
    const UnallocatedVector<WalletDatabase::UTXO>& utxos,
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
        default: {
            OT_FAIL;
        }
    }
}

auto SubchainStateData::process_block(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto hash = api_.Factory().Data(body.at(2));
    process_block(hash);
}

auto SubchainStateData::process_block(const block::Hash& block) noexcept -> void
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

auto SubchainStateData::process_filter(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto type = body.at(2).as<filter::Type>();

    if (type != node_.FilterOracleInternal().DefaultType()) { return; }

    const auto position = block::Position{
        body.at(3).as<block::Height>(), api_.Factory().Data(body.at(4))};
    process_filter(position);
}

auto SubchainStateData::process_filter(const block::Position& tip) noexcept
    -> void
{
    LogInsane()(OT_PRETTY_CLASS())(name_).Flush();
    rescan_.UpdateTip(tip);
    scan_.Run(tip);
}

auto SubchainStateData::process_job_finished(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto id = api_.Factory().Identifier(body.at(1));
    const auto str = CString{body.at(2).Bytes(), get_allocator()};
    process_job_finished(id, str.c_str());
}

auto SubchainStateData::process_job_finished(
    const Identifier& id,
    const char* type) noexcept -> void
{
    auto again{true};
    const auto& log = LogTrace();

    if (id != db_key_) { return; }

    log(OT_PRETTY_CLASS())(name_)(" ")(type)(" job complete").Flush();
    const auto start = Clock::now();
    static constexpr auto limit = std::chrono::minutes{1};

    while (again && ((Clock::now() - start) < limit)) { again = work(); }

    if (again) {
        log(OT_PRETTY_CLASS())(name_)(" failed to obtain lock after ")(
            std::chrono::nanoseconds{Clock::now() - start})
            .Flush();
    }
}

auto SubchainStateData::process_key(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4u < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = api_.Factory().NymID(body.at(2));

    if (owner != owner_) { return; }

    const auto id = api_.Factory().Identifier(body.at(3));

    if (id != id_) { return; }

    const auto subchain = body.at(4).as<crypto::Subchain>();

    if (subchain != subchain_) { return; }

    LogInsane()(OT_PRETTY_CLASS())(name_).Flush();
    get_index().Run();
}

auto SubchainStateData::process_mempool(Message&& in) noexcept -> void
{
    const auto body = in.Body();
    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    if (auto tx = mempool_oracle_.Query(body.at(2).Bytes()); tx) {
        process_mempool(std::move(tx));
    }
}

auto SubchainStateData::process_mempool(
    std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept -> void
{
    LogInsane()(OT_PRETTY_CLASS())(name_).Flush();

    if (mempool_.Queue(tx)) { mempool_.Run(); }
}

auto SubchainStateData::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& ancestor) noexcept -> void
{
    do_reorg(headerOracleLock, tx, errors, ancestor);
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

auto SubchainStateData::startup() noexcept -> void
{
    const_cast<UnallocatedCString&>(name_) = describe();
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

auto SubchainStateData::state_normal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::filter: {
            process_filter(std::move(msg));
        } break;
        case Work::mempool: {
            process_mempool(std::move(msg));
        } break;
        case Work::block: {
            process_block(std::move(msg));
        } break;
        case Work::job_finished: {
            process_job_finished(std::move(msg));
        } break;
        case Work::reorg_begin: {
            transition_state_reorg(std::move(msg));
        } break;
        case Work::reorg_end: {
            LogError()(OT_PRETTY_CLASS())(": wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::key: {
            process_key(std::move(msg));
        } break;
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto SubchainStateData::state_reorg(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::shutdown:
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::job_finished:
        case Work::key:
        case Work::statemachine: {
            // NOTE defer processing of non-reorg messages until after reorg is
            // complete
            pipeline_.Push(std::move(msg));
        } break;
        case Work::reorg_end: {
            transition_state_normal(std::move(msg));
        } break;
        case Work::reorg_begin: {
            LogError()(OT_PRETTY_CLASS())(": wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

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
    disable_automatic_processing_ = false;
    state_ = State::normal;
    to_parent_.Send(MakeWork(AccountsJobs::reorg_end_ack));
}

auto SubchainStateData::transition_state_reorg(Message&& in) noexcept -> void
{
    finish_background_tasks();
    disable_automatic_processing_ = true;
    state_ = State::reorg;
    to_parent_.Send(MakeWork(AccountsJobs::reorg_begin_ack));
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

auto SubchainStateData::update_scan(const block::Position& pos, bool reorg)
    const noexcept -> void
{
    if (false == reorg) { db_.SubchainSetLastScanned(db_key_, pos); }

    report_scan(pos);
}

auto SubchainStateData::work() noexcept -> bool
{
    auto again{false};
    get_index().Run();
    mempool_.Run();
    scan_.Run();
    rescan_.Run();
    again = process_.Run();

    return again;
}

SubchainStateData::~SubchainStateData()
{
    signal_shutdown();
    finish_background_tasks();
}
}  // namespace opentxs::blockchain::node::wallet
