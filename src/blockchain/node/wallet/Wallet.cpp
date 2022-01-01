// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/node/wallet/Wallet.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <stdexcept>
#include <utility>

#include "core/Worker.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "util/LMDB.hpp"

namespace opentxs::factory
{
auto BlockchainWallet(
    const api::Session& api,
    const blockchain::node::internal::Network& parent,
    const blockchain::node::internal::WalletDatabase& db,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::Type chain,
    const std::string& shutdown)
    -> std::unique_ptr<blockchain::node::internal::Wallet>
{
    using ReturnType = blockchain::node::implementation::Wallet;

    return std::make_unique<ReturnType>(
        api, parent, db, mempool, chain, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::implementation
{
Wallet::Wallet(
    const api::Session& api,
    const node::internal::Network& parent,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
    const std::string& shutdown) noexcept
    : Worker(api, std::chrono::milliseconds(10))
    , parent_(parent)
    , db_(db)
    , mempool_(mempool)
    , chain_(chain)
    , task_finished_([this](const Identifier& id, const char* type) {
        auto work = MakeWork(Work::job_finished);
        work.AddFrame(id.data(), id.size());
        work.AddFrame(std::string(type));
        pipeline_.Push(std::move(work));
    })
    , enabled_(true)
    , accounts_(api, parent_, db_, chain_, task_finished_)
    , proposals_(api, parent_, db_, chain_)
{
    init_executor({
        shutdown,
        api.Endpoints().BlockchainReorg(),
        api.Endpoints().NymCreated(),
        api.Endpoints().BlockchainNewFilter(),
        api_.Crypto().Blockchain().Internal().KeyEndpoint(),
        api.Endpoints().BlockchainMempool(),
        api.Endpoints().BlockchainBlockAvailable(),
    });
    trigger_wallet();
}

auto Wallet::ConstructTransaction(
    const proto::BlockchainTransactionProposal& tx,
    std::promise<SendOutcome>&& promise) const noexcept -> void
{
    proposals_.Add(tx, std::move(promise));
    trigger();
}

auto Wallet::GetBalance() const noexcept -> Balance { return db_.GetBalance(); }

auto Wallet::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    return db_.GetBalance(owner);
}

auto Wallet::GetBalance(const identifier::Nym& owner, const Identifier& node)
    const noexcept -> Balance
{
    return db_.GetBalance(owner, node);
}

auto Wallet::GetBalance(const crypto::Key& key) const noexcept -> Balance
{
    return db_.GetBalance(key);
}

auto Wallet::GetOutputs() const noexcept -> std::pmr::vector<UTXO>
{
    return GetOutputs(TxoState::All);
}

auto Wallet::GetOutputs(TxoState type) const noexcept -> std::pmr::vector<UTXO>
{
    return db_.GetOutputs(type);
}

auto Wallet::GetOutputs(const identifier::Nym& owner) const noexcept
    -> std::pmr::vector<UTXO>
{
    return GetOutputs(owner, TxoState::All);
}

auto Wallet::GetOutputs(const identifier::Nym& owner, TxoState type)
    const noexcept -> std::pmr::vector<UTXO>
{
    return db_.GetOutputs(owner, type);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const Identifier& subaccount) const noexcept -> std::pmr::vector<UTXO>
{
    return GetOutputs(owner, subaccount, TxoState::All);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const Identifier& node,
    TxoState type) const noexcept -> std::pmr::vector<UTXO>
{
    return db_.GetOutputs(owner, node, type);
}

auto Wallet::GetOutputs(const crypto::Key& key, TxoState type) const noexcept
    -> std::pmr::vector<UTXO>
{
    return db_.GetOutputs(key, type);
}

auto Wallet::GetTags(const block::Outpoint& output) const noexcept
    -> std::pmr::set<TxoTag>
{
    return db_.GetOutputTags(output);
}

auto Wallet::Height() const noexcept -> block::Height
{
    return db_.GetWalletHeight();
}

auto Wallet::Init() noexcept -> void
{
    enabled_ = true;
    trigger_wallet();
    trigger();
}

auto Wallet::pipeline(const zmq::Message& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
            ": invalid message")
            .Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();
    const auto start = Clock::now();
    auto type = std::string{};

    switch (work) {
        case Work::shutdown: {
            type = "shutdown";
            shutdown(shutdown_promise_);
        } break;
        case Work::nym: {
            type = "nym";
            process_nym(in);
            process_wallet();
        } break;
        case Work::header: {
            type = "header";
            process_block_header(in);
        } break;
        case Work::reorg: {
            type = "reorg";
            process_reorg(in);
            // TODO ensure filter oracle has processed the reorg before running
            // state machine again
            process_wallet();
        } break;
        case Work::mempool: {
            type = "mempool";
            process_mempool(in);
        } break;
        case Work::block: {
            type = "block";
            process_block_download(in);
        } break;
        case Work::job_finished: {
            type = "job_finished";
            process_job_finished(in);
        } break;
        case Work::init_wallet: {
            type = "init_wallet";
            process_wallet();
        } break;
        case Work::key: {
            type = "key";
            process_key(in);
        } break;
        case Work::filter: {
            type = "filter";
            process_filter(in);
        } break;
        case Work::statemachine: {
            type = "statemachine";
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
                ": unhandled type")
                .Flush();

            OT_FAIL;
        }
    }

    static constexpr auto limit = std::chrono::seconds{1};
    const auto elapsed = std::chrono::nanoseconds{Clock::now() - start};

    if (elapsed > limit) {
        LogTrace()(OT_PRETTY_CLASS())(DisplayString(chain_))(": spent ")(
            elapsed)(" processing ")(type)(" signal")
            .Flush();
    }
}

auto Wallet::process_block_download(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto hash = api_.Factory().Data(body.at(2));
    accounts_.ProcessBlockAvailable(hash);
}

auto Wallet::process_block_header(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (3 >= body.size()) {
        LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
            ": invalid message")
            .Flush();

        OT_FAIL;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto position = block::Position{
        body.at(3).as<block::Height>(),
        api_.Factory().Data(body.at(2).Bytes())};
    db_.AdvanceTo(position);
}

auto Wallet::process_filter(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto type = body.at(2).as<filter::Type>();

    if (type != parent_.FilterOracleInternal().DefaultType()) { return; }

    const auto position = block::Position{
        body.at(3).as<block::Height>(), api_.Factory().Data(body.at(4))};

    if (enabled_) { accounts_.ProcessNewFilter(position); }
}

auto Wallet::process_job_finished(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto id = api_.Factory().Identifier(body.at(1));
    const auto type = std::string{body.at(2).Bytes()};
    accounts_.ProcessTaskComplete(id, type.c_str(), enabled_);
}

auto Wallet::process_key(const zmq::Message& in) noexcept -> void
{
    // TODO extract subchain id to filter which state machines get activated

    accounts_.ProcessKey();
}

auto Wallet::process_mempool(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();
    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    if (auto tx = mempool_.Query(body.at(2).Bytes()); tx) {
        accounts_.ProcessMempool(std::move(tx));
    }
}

auto Wallet::process_nym(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    accounts_.ProcessNym(body.at(1));
}

auto Wallet::process_reorg(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (6 > body.size()) {
        LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(
            ": invalid message")
            .Flush();

        OT_FAIL;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto ancestor = block::Position{
        body.at(3).as<block::Height>(),
        api_.Factory().Data(body.at(2).Bytes())};
    const auto tip = block::Position{
        body.at(5).as<block::Height>(),
        api_.Factory().Data(body.at(4).Bytes())};
    LogConsole()(DisplayString(chain_))(": processing reorg to ")(
        tip.second->asHex())(" at height ")(tip.first)
        .Flush();
    accounts_.FinishBackgroundTasks();
    auto errors = std::atomic_int{};
    {
        auto headerOracleLock =
            Lock{parent_.HeaderOracle().Internal().GetMutex()};
        auto tx = db_.StartReorg();
        accounts_.ProcessReorg(headerOracleLock, tx, errors, ancestor);
        accounts_.FinishBackgroundTasks();

        if (false == db_.FinalizeReorg(tx, ancestor)) { ++errors; }

        try {
            if (0 < errors) { throw std::runtime_error{"Prepare step failed"}; }

            if (false == tx.Finalize(true)) {

                throw std::runtime_error{"Finalize transaction failed"};
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(": ")(e.what())
                .Flush();

            OT_FAIL;
        }
    }

    try {
        if (false == db_.AdvanceTo(tip)) {

            throw std::runtime_error{"Advance chain failed"};
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(DisplayString(chain_))(" ")(e.what())
            .Flush();

        OT_FAIL;
    }

    LogConsole()(DisplayString(chain_))(": reorg to ")(tip.second->asHex())(
        " at height ")(tip.first)(" finished")
        .Flush();
}

auto Wallet::process_wallet() noexcept -> void
{
    accounts_.ProcessStateMachine(enabled_);
}

auto Wallet::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        LogDetail()("Shutting down ")(DisplayString(chain_))(" wallet").Flush();
        pipeline_.Close();
        accounts_.Shutdown();
        promise.set_value();
    }
}

auto Wallet::state_machine() noexcept -> bool
{
    if (false == running_.load()) { return false; }

    auto repeat{false};

    if (enabled_) { repeat |= proposals_.Run(); }

    return repeat;
}

auto Wallet::trigger_wallet() const noexcept -> void
{
    pipeline_.Push(MakeWork(Work::init_wallet));
}

Wallet::~Wallet() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::node::implementation
