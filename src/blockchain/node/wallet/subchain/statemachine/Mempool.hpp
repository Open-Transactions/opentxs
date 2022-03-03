// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <queue>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace node
{
namespace wallet
{
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Mempool : public Job
{
public:
    using Transaction = std::shared_ptr<const block::bitcoin::Transaction>;
    using Transactions = UnallocatedVector<Transaction>;

    auto Queue(Transaction tx) noexcept -> bool;
    auto Queue(Transactions&& transactions) noexcept -> bool;
    auto Reorg(const block::Position& parent) noexcept -> void final;
    auto Run() noexcept -> bool final;

    Mempool(SubchainStateData& parent) noexcept;

    ~Mempool() override = default;

private:
    std::queue<Transaction> queue_;

    auto type() const noexcept -> const char* final;

    auto Do() noexcept -> void;

    Mempool() = delete;
    Mempool(const Mempool&) = delete;
    Mempool(Mempool&&) = delete;
    auto operator=(const Mempool&) -> Mempool& = delete;
    auto operator=(Mempool&&) -> Mempool& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
