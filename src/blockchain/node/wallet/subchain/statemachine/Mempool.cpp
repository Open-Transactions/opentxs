// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Mempool.hpp"  // IWYU pragma: associated

#include <tuple>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"

namespace opentxs::blockchain::node::wallet
{
Mempool::Mempool(SubchainStateData& parent) noexcept
    : Job(ThreadPool::General, parent)
    , queue_()
{
}

auto Mempool::Do() noexcept -> void
{
    // TODO someday keep mempool transactions around in case a future index
    // action creates a match.
}

auto Mempool::Queue(Transaction tx) noexcept -> bool
{
    return Queue(Transactions{tx});
}

auto Mempool::Queue(Transactions&& transactions) noexcept -> bool
{
    auto lock = Lock{lock_};

    for (auto& tx : transactions) { queue_.push(std::move(tx)); }

    return true;
}

auto Mempool::Reorg(const block::Position&) noexcept -> void
{
    // NOTE no action necessary
}

auto Mempool::Run() noexcept -> bool
{

    auto lock = Lock{lock_};
    const auto targets = parent_.get_account_targets();
    const auto& [elements, utxos, patterns] = targets;
    const auto parsed = block::ParsedPatterns{elements};
    const auto outpoints = [&] {
        auto out = SubchainStateData::Patterns{};
        parent_.translate(std::get<1>(targets), out);

        return out;
    }();

    while (false == queue_.empty()) {
        const auto tx = queue_.front();

        OT_ASSERT(tx);

        auto copy = tx->clone();

        OT_ASSERT(copy);

        const auto matches = copy->Internal().FindMatches(
            parent_.filter_type_, outpoints, parsed);
        parent_.handle_mempool_matches(matches, std::move(copy));
        queue_.pop();
    }

    return false;
}

auto Mempool::type() const noexcept -> const char* { return "mempool"; }
}  // namespace opentxs::blockchain::node::wallet
