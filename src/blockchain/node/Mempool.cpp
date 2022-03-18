// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "blockchain/node/Mempool.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <algorithm>
#include <chrono>
#include <queue>
#include <shared_mutex>
#include <string_view>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node
{
struct Mempool::Imp {
    using Transactions =
        UnallocatedVector<std::unique_ptr<const block::bitcoin::Transaction>>;

    auto Dump() const noexcept -> UnallocatedSet<UnallocatedCString>
    {
        auto lock = sLock{lock_};

        return active_;
    }
    auto Query(ReadView txid) const noexcept
        -> std::shared_ptr<const block::bitcoin::Transaction>
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
    auto Submit(std::unique_ptr<const block::bitcoin::Transaction> tx)
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
        auto lock = eLock{lock_};

        for (auto& tx : txns) {
            if (!tx) {
                LogError()(OT_PRETTY_CLASS())("invalid transaction").Flush();

                continue;
            }

            auto txid = Hash{tx->ID().Bytes()};
            const auto [it, added] = transactions_.try_emplace(txid, nullptr);

            if (added) { unexpired_txid_.emplace(Clock::now(), txid); }

            auto& existing = it->second;

            if (!existing) {
                existing = std::move(tx);
                notify(txid);
                active_.emplace(txid);
                unexpired_tx_.emplace(Clock::now(), std::move(txid));
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

    Imp(const api::crypto::Blockchain& crypto,
        internal::WalletDatabase& wallet,
        const network::zeromq::socket::Publish& socket,
        const Type chain) noexcept
        : crypto_(crypto)
        , wallet_(wallet)
        , chain_(chain)
        , lock_()
        , transactions_()
        , active_()
        , unexpired_txid_()
        , unexpired_tx_()
        , socket_(socket)
    {
        init();
    }

private:
    using Hash = UnallocatedCString;
    using TransactionMap = robin_hood::unordered_flat_map<
        Hash,
        std::shared_ptr<const block::bitcoin::Transaction>>;
    using Data = std::pair<Time, Hash>;
    using Cache = std::queue<Data>;

    static constexpr auto tx_limit_ = std::chrono::hours{1};
    static constexpr auto txid_limit_ = std::chrono::hours{24};

    const api::crypto::Blockchain& crypto_;
    internal::WalletDatabase& wallet_;
    const Type chain_;
    mutable std::shared_mutex lock_;
    mutable TransactionMap transactions_;
    mutable UnallocatedSet<Hash> active_;
    mutable Cache unexpired_txid_;
    mutable Cache unexpired_tx_;
    const network::zeromq::socket::Publish& socket_;

    auto notify(ReadView txid) const noexcept -> void
    {
        socket_.Send([&] {
            auto work = network::zeromq::tagged_message(
                WorkType::BlockchainMempoolUpdated);
            work.AddFrame(chain_);
            work.AddFrame(txid.data(), txid.size());

            return work;
        }());
    }

    auto init() noexcept -> void
    {
        auto transactions = Transactions{};

        for (const auto& txid : wallet_.GetUnconfirmedTransactions()) {
            if (auto tx = crypto_.LoadTransactionBitcoin(txid); tx) {
                LogVerbose()(OT_PRETTY_CLASS())(
                    "adding unconfirmed transaction ")(txid->asHex())(
                    " to mempool")
                    .Flush();
                transactions.emplace_back(std::move(tx));
            } else {
                LogError()(OT_PRETTY_CLASS())("failed to load transaction ")(
                    txid->asHex())
                    .Flush();
            }
        }

        Submit(std::move(transactions));
    }
};

Mempool::Mempool(
    const api::crypto::Blockchain& crypto,
    internal::WalletDatabase& wallet,
    const network::zeromq::socket::Publish& socket,
    const Type chain) noexcept
    : imp_(std::make_unique<Imp>(crypto, wallet, socket, chain))
{
}

auto Mempool::Dump() const noexcept -> UnallocatedSet<UnallocatedCString>
{
    return imp_->Dump();
}

auto Mempool::Heartbeat() noexcept -> void { imp_->Heartbeat(); }

auto Mempool::Query(ReadView txid) const noexcept
    -> std::shared_ptr<const block::bitcoin::Transaction>
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

auto Mempool::Submit(std::unique_ptr<const block::bitcoin::Transaction> tx)
    const noexcept -> void
{
    imp_->Submit(std::move(tx));
}

Mempool::~Mempool() = default;
}  // namespace opentxs::blockchain::node
