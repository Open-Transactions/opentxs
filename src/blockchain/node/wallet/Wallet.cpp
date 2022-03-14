// Copyright (c) 2010-2022 The Open-Transactions developers
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
#include <utility>

#include "core/Worker.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Factory.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::factory
{
auto BlockchainWallet(
    const api::Session& api,
    const blockchain::node::internal::Network& parent,
    const blockchain::node::internal::WalletDatabase& db,
    const blockchain::node::internal::Mempool& mempool,
    const blockchain::Type chain,
    const std::string_view shutdown)
    -> std::unique_ptr<blockchain::node::internal::Wallet>
{
    using ReturnType = blockchain::node::implementation::Wallet;

    return std::make_unique<ReturnType>(
        api, parent, db, mempool, chain, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::wallet
{
auto print(WalletJobs job) noexcept -> std::string_view
{
    try {
        using Job = WalletJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::init, "init"},
            {Job::shutdown_ready, "shutdown_ready"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid WalletJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::implementation
{
Wallet::Wallet(
    const api::Session& api,
    const node::internal::Network& parent,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
    const std::string_view shutdown,
    const CString accounts) noexcept
    : Worker(
          api,
          10ms,
          {
              {network::zeromq::socket::Type::Pair,
               {
                   {accounts, network::zeromq::socket::Direction::Bind},
               }},
          })
    , parent_(parent)
    , db_(db)
    , chain_(chain)
    , to_accounts_(pipeline_.Internal().ExtraSocket(0))
    , fee_oracle_(factory::FeeOracle(api_, chain))
    , accounts_(api, parent_, db_, mempool, chain_, accounts)
    , proposals_(api, parent_, db_, chain_)
    , shutdown_sent_(false)
{
    init_executor({
        UnallocatedCString{shutdown},
    });
}

Wallet::Wallet(
    const api::Session& api,
    const node::internal::Network& parent,
    const node::internal::WalletDatabase& db,
    const node::internal::Mempool& mempool,
    const Type chain,
    const std::string_view shutdown) noexcept
    : Wallet(
          api,
          parent,
          db,
          mempool,
          chain,
          shutdown,
          network::zeromq::MakeArbitraryInproc().c_str())
{
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

auto Wallet::GetOutputs() const noexcept -> UnallocatedVector<UTXO>
{
    return GetOutputs(TxoState::All);
}

auto Wallet::GetOutputs(TxoState type) const noexcept -> UnallocatedVector<UTXO>
{
    return db_.GetOutputs(type);
}

auto Wallet::GetOutputs(const identifier::Nym& owner) const noexcept
    -> UnallocatedVector<UTXO>
{
    return GetOutputs(owner, TxoState::All);
}

auto Wallet::GetOutputs(const identifier::Nym& owner, TxoState type)
    const noexcept -> UnallocatedVector<UTXO>
{
    return db_.GetOutputs(owner, type);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const Identifier& subaccount) const noexcept -> UnallocatedVector<UTXO>
{
    return GetOutputs(owner, subaccount, TxoState::All);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const Identifier& node,
    TxoState type) const noexcept -> UnallocatedVector<UTXO>
{
    return db_.GetOutputs(owner, node, type);
}

auto Wallet::GetOutputs(const crypto::Key& key, TxoState type) const noexcept
    -> UnallocatedVector<UTXO>
{
    return db_.GetOutputs(key, type);
}

auto Wallet::GetTags(const block::Outpoint& output) const noexcept
    -> UnallocatedSet<TxoTag>
{
    return db_.GetOutputTags(output);
}

auto Wallet::Height() const noexcept -> block::Height
{
    return db_.GetWalletHeight();
}

auto Wallet::Init() noexcept -> void
{
    accounts_.Init();
    trigger();
}

auto Wallet::pipeline(const zmq::Message& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(": invalid message")
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

    switch (work) {
        case Work::shutdown: {
            process_shutdown();
        } break;
        case Work::shutdown_ready: {
            shutdown(shutdown_promise_);
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(": unhandled type")
                .Flush();

            OT_FAIL;
        }
    }

    static constexpr auto limit = 1s;
    const auto elapsed = std::chrono::nanoseconds{Clock::now() - start};

    if (elapsed > limit) {
        LogTrace()(OT_PRETTY_CLASS())(print(chain_))(": spent ")(
            elapsed)(" processing ")(print(work))(" signal")
            .Flush();
    }
}

auto Wallet::process_shutdown() noexcept -> void
{
    if (auto previous = shutdown_sent_.exchange(true); false == previous) {
        to_accounts_.Send(MakeWork(wallet::AccountsJobs::shutdown_begin));
    }
}

auto Wallet::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        LogDetail()("Shutting down ")(print(chain_))(" wallet").Flush();
        process_shutdown();
        to_accounts_.Send(MakeWork(wallet::AccountsJobs::shutdown));
        pipeline_.Close();
        fee_oracle_.Shutdown();
        promise.set_value();
    }
}

auto Wallet::state_machine() noexcept -> bool
{
    if (false == running_.load()) { return false; }

    return proposals_.Run();
}

Wallet::~Wallet() { stop_worker().get(); }
}  // namespace opentxs::blockchain::node::implementation
