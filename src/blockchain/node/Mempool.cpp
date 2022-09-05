// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "blockchain/node/Mempool.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <chrono>
#include <queue>
#include <shared_mutex>
#include <string_view>
#include <utility>

#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node
{
struct Mempool::Imp {
    using Transactions =
        UnallocatedVector<std::unique_ptr<const bitcoin::block::Transaction>>;

    auto Dump() const noexcept -> UnallocatedSet<UnallocatedCString>
    {
        auto lock = sLock{lock_};

        return active_;
    }
    auto Query(ReadView txid) const noexcept
        -> std::shared_ptr<const bitcoin::block::Transaction>
    {
        auto lock = sLock{lock_};

        try {

            return transactions_.at(Hash{txid});
        } catch (...) {

            return {};
        }
    }
    auto Submit(ReadView txid) const noexcept -> bool
    {
        const auto input = UnallocatedVector<ReadView>{txid};
        const auto output = Submit(input);

        return output.front();
    }
    auto Submit(const UnallocatedVector<ReadView>& txids) const noexcept
        -> UnallocatedVector<bool>
    {
        auto output = UnallocatedVector<bool>{};
        output.reserve(txids.size());
        auto lock = eLock{lock_};

        for (const auto& txid : txids) {
            const auto [it, added] =
                transactions_.try_emplace(Hash{txid}, nullptr);

            if (added) {
                unexpired_txid_.emplace(Clock::now(), txid);
                output.emplace_back(true);
            } else {
                output.emplace_back(false);
            }
        }

        OT_ASSERT(output.size() == txids.size());

        return output;
    }
    auto Submit(std::unique_ptr<const bitcoin::block::Transaction> tx)
        const noexcept -> void
    {
        Submit([&] {
            auto out = Transactions{};
            out.emplace_back(std::move(tx));

            return out;
        }());
    }
    auto Submit(Transactions&& txns) const noexcept -> void
    {
        const auto now = Clock::now();
        auto lock = eLock{lock_};

        for (auto& tx : txns) {
            if (!tx) {
                LogError()(OT_PRETTY_CLASS())("invalid transaction").Flush();

                continue;
            }

            auto txid = Hash{tx->ID().Bytes()};
            const auto [it, added] = transactions_.try_emplace(txid, nullptr);

            if (added) { unexpired_txid_.emplace(now, txid); }

            auto& existing = it->second;

            if (!existing) {
                existing = std::move(tx);
                notify(lock, txid);
                active_.emplace(txid);
                unexpired_tx_.emplace(now, std::move(txid));
            }
        }
    }

    auto Heartbeat() noexcept -> void
    {
        const auto now = Clock::now();
        auto lock = eLock{lock_};

        while (0 < unexpired_tx_.size()) {
            const auto& [time, txid] = unexpired_tx_.front();

            if ((now - time) < tx_limit_) { break; }

            try {
                transactions_.at(txid).reset();
            } catch (...) {
            }

            active_.erase(txid);
            unexpired_tx_.pop();
        }

        while (0 < unexpired_txid_.size()) {
            const auto& [time, txid] = unexpired_txid_.front();

            if ((now - time) < txid_limit_) { break; }

            transactions_.erase(txid);
            active_.erase(txid);
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
            auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
            const auto endpoint =
                UnallocatedCString{api.Endpoints()
                                       .Internal()
                                       .Internal()
                                       .BlockchainMessageRouter()};
            const auto rc = out.Connect(endpoint.c_str());

            OT_ASSERT(rc);

            return out;
        }())
    {
        init();
    }

private:
    using Hash = UnallocatedCString;
    using TransactionMap = robin_hood::unordered_flat_map<
        Hash,
        std::shared_ptr<const bitcoin::block::Transaction>>;
    using Data = std::pair<Time, Hash>;
    using Cache = std::queue<Data>;

    static constexpr auto tx_limit_ = std::chrono::hours{2};
    static constexpr auto txid_limit_ = std::chrono::hours{24};

    const api::crypto::Blockchain& crypto_;
    database::Wallet& wallet_;
    const Type chain_;
    mutable std::shared_mutex lock_;
    mutable TransactionMap transactions_;
    mutable UnallocatedSet<Hash> active_;
    mutable Cache unexpired_txid_;
    mutable Cache unexpired_tx_;
    mutable opentxs::network::zeromq::socket::Raw to_blockchain_api_;

    auto notify(const eLock&, ReadView txid) const noexcept -> void
    {
        to_blockchain_api_.SendDeferred(
            [&] {
                auto work = network::zeromq::tagged_message(
                    WorkType::BlockchainMempoolUpdated);
                work.AddFrame(chain_);
                work.AddFrame(txid.data(), txid.size());

                return work;
            }(),
            __FILE__,
            __LINE__);
    }

    auto init() noexcept -> void
    {
        auto transactions = Transactions{};

        for (const auto& txid : wallet_.GetUnconfirmedTransactions()) {
            if (auto tx = crypto_.LoadTransactionBitcoin(txid); tx) {
                LogVerbose()(OT_PRETTY_CLASS())(
                    "adding unconfirmed transaction ")
                    .asHex(txid)(" to mempool")
                    .Flush();
                transactions.emplace_back(std::move(tx));
            } else {
                LogError()(OT_PRETTY_CLASS())("failed to load transaction ")
                    .asHex(txid)
                    .Flush();
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

auto Mempool::Dump() const noexcept -> UnallocatedSet<UnallocatedCString>
{
    return imp_->Dump();
}

auto Mempool::Heartbeat() noexcept -> void { imp_->Heartbeat(); }

auto Mempool::Query(ReadView txid) const noexcept
    -> std::shared_ptr<const bitcoin::block::Transaction>
{
    return imp_->Query(txid);
}

auto Mempool::Submit(ReadView txid) const noexcept -> bool
{
    return imp_->Submit(txid);
}

auto Mempool::Submit(const UnallocatedVector<ReadView>& txids) const noexcept
    -> UnallocatedVector<bool>
{
    return imp_->Submit(txids);
}

auto Mempool::Submit(std::unique_ptr<const bitcoin::block::Transaction> tx)
    const noexcept -> void
{
    imp_->Submit(std::move(tx));
}

Mempool::~Mempool() = default;
}  // namespace opentxs::blockchain::node
