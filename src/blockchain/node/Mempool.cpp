// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "blockchain/node/Mempool.hpp"  // IWYU pragma: associated

#include <boost/unordered/unordered_flat_map.hpp>
#include <chrono>
#include <compare>
#include <queue>
#include <shared_mutex>
#include <utility>

#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::blockchain::node
{
struct Mempool::Imp {
    using Transactions = Vector<blockchain::block::Transaction>;

    auto Dump(alloc::Default alloc) const noexcept
        -> Set<block::TransactionHash>
    {
        auto lock = sLock{lock_};

        return {active_, alloc};
    }
    auto Prune(const block::Block& block, alloc::Default monotonic)
        const noexcept -> void
    {
        auto lock = eLock{lock_};

        for (const auto& tx : block.get()) { expire_txid(lock, tx.ID()); }
    }
    auto Query(const block::TransactionHash& txid, alloc::Default alloc)
        const noexcept -> block::Transaction
    {
        auto lock = sLock{lock_};

        try {

            return {transactions_.at(txid), alloc};
        } catch (...) {

            return {alloc};
        }
    }
    auto Submit(const block::TransactionHash& txid) const noexcept -> bool
    {
        return Submit(
                   std::span<const block::TransactionHash>{
                       std::addressof(txid), 1_uz},
                   {})
            .front();
    }
    auto Submit(
        std::span<const block::TransactionHash> txids,
        alloc::Default alloc) const noexcept -> Vector<bool>
    {
        auto output = Vector<bool>{alloc};
        output.reserve(txids.size());
        auto lock = eLock{lock_};

        for (const auto& txid : txids) {
            const auto [it, added] =
                transactions_.try_emplace(txid, block::Transaction{});

            if (added) {
                unexpired_txid_.emplace(sClock::now(), txid);
                output.emplace_back(true);
            } else {
                output.emplace_back(false);
            }
        }

        assert_true(output.size() == txids.size());

        return output;
    }
    auto Submit(block::Transaction tx) const noexcept -> void
    {
        Submit([&] {
            auto out = Transactions{};
            out.emplace_back(std::move(tx));

            return out;
        }());
    }
    auto Submit(Transactions&& txns) const noexcept -> void
    {
        const auto now = sClock::now();
        auto lock = eLock{lock_};

        for (auto& tx : txns) {
            if (false == tx.IsValid()) {
                LogError()()("invalid transaction").Flush();

                continue;
            }

            const auto& txid = tx.ID();
            auto [it, added] =
                transactions_.try_emplace(txid, block::Transaction{});

            if (added) { unexpired_txid_.emplace(now, txid); }

            auto& existing = it->second;

            if (false == existing.IsValid()) {
                existing = std::move(tx);
                notify(lock, txid);
                active_.emplace(txid);
                unexpired_tx_.emplace(now, std::move(txid));
            }
        }
    }

    auto Heartbeat() noexcept -> void
    {
        const auto now = sClock::now();
        auto lock = eLock{lock_};

        while (false == unexpired_tx_.empty()) {
            const auto& [time, txid] = unexpired_tx_.front();

            if ((now - time) < tx_limit_) { break; }

            expire_tx(lock, txid);
            unexpired_tx_.pop();
        }

        while (false == unexpired_txid_.empty()) {
            const auto& [time, txid] = unexpired_txid_.front();

            if ((now - time) < txid_limit_) { break; }

            expire_txid(lock, txid);
            unexpired_txid_.pop();
        }
    }

    Imp(const api::Session& api,
        const api::crypto::Blockchain& crypto,
        const Type chain,
        database::Wallet& db) noexcept
        : crypto_(crypto)
        , wallet_(db)
        , chain_(chain)
        , lock_()
        , transactions_()
        , active_()
        , unexpired_txid_()
        , unexpired_tx_()
        , to_blockchain_api_([&] {
            using Type = opentxs::network::zeromq::socket::Type;
            auto out = api.Network().ZeroMQ().Context().Internal().RawSocket(
                Type::Push);
            const auto endpoint = UnallocatedCString{
                api.Endpoints().Internal().BlockchainMessageRouter()};
            const auto rc = out.Connect(endpoint.c_str());

            assert_true(rc);

            return out;
        }())
    {
        init();
    }

private:
    using TransactionMap = boost::unordered_flat_map<
        block::TransactionHash,
        block::Transaction,
        std::hash<block::TransactionHash>>;
    using Data = std::pair<sTime, block::TransactionHash>;
    using Cache = std::queue<Data>;

    static constexpr auto tx_limit_ = std::chrono::hours{2};
    static constexpr auto txid_limit_ = std::chrono::hours{24};

    const api::crypto::Blockchain& crypto_;
    database::Wallet& wallet_;
    const Type chain_;
    mutable std::shared_mutex lock_;
    mutable TransactionMap transactions_;
    mutable Set<block::TransactionHash> active_;
    mutable Cache unexpired_txid_;
    mutable Cache unexpired_tx_;
    mutable opentxs::network::zeromq::socket::Raw to_blockchain_api_;

    auto expire_tx(const eLock&, const block::TransactionHash& txid)
        const noexcept -> void
    {
        if (auto i = transactions_.find(txid); transactions_.end() != i) {
            i->second = block::Transaction::Blank();
        }

        active_.erase(txid);
    }
    auto expire_txid(const eLock&, const block::TransactionHash& txid)
        const noexcept -> void
    {
        transactions_.erase(txid);
        active_.erase(txid);
    }
    auto notify(const eLock&, const block::TransactionHash& txid) const noexcept
        -> void
    {
        to_blockchain_api_.SendDeferred([&] {
            auto work = network::zeromq::tagged_message(
                WorkType::BlockchainMempoolUpdated, true);
            work.AddFrame(chain_);
            work.AddFrame(txid.data(), txid.size());

            return work;
        }());
    }

    auto init() noexcept -> void
    {
        auto transactions = Transactions{};

        for (const auto& txid : wallet_.GetUnconfirmedTransactions()) {
            if (auto tx = crypto_.LoadTransaction(txid); tx.IsValid()) {
                LogVerbose()()("adding unconfirmed transaction ")
                    .asHex(txid)(" to mempool")
                    .Flush();
                transactions.emplace_back(std::move(tx));
            } else {
                LogError()()("failed to load transaction ").asHex(txid).Flush();
            }
        }

        Submit(std::move(transactions));
    }
};

Mempool::Mempool(
    const api::Session& api,
    const api::crypto::Blockchain& crypto,
    const Type chain,
    database::Wallet& db) noexcept
    : imp_(std::make_unique<Imp>(api, crypto, chain, db))
{
}

auto Mempool::Dump(alloc::Default alloc) const noexcept
    -> Set<block::TransactionHash>
{
    return imp_->Dump(alloc);
}

auto Mempool::Heartbeat() noexcept -> void { imp_->Heartbeat(); }

auto Mempool::Prune(const block::Block& block, alloc::Default monotonic)
    const noexcept -> void
{
    imp_->Prune(block, monotonic);
}

auto Mempool::Query(const block::TransactionHash& txid, alloc::Default alloc)
    const noexcept -> block::Transaction
{
    return imp_->Query(txid, alloc);
}

auto Mempool::Submit(const block::TransactionHash& txid) const noexcept -> bool
{
    return imp_->Submit(txid);
}

auto Mempool::Submit(
    std::span<const block::TransactionHash> txids,
    alloc::Default alloc) const noexcept -> Vector<bool>
{
    return imp_->Submit(txids, alloc);
}

auto Mempool::Submit(block::Transaction tx) const noexcept -> void
{
    imp_->Submit(std::move(tx));
}

Mempool::~Mempool() = default;
}  // namespace opentxs::blockchain::node
