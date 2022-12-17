// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <span>

#include "internal/blockchain/node/Mempool.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
}  // namespace api

namespace blockchain
{

namespace database
{
class Wallet;
}  // namespace database
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class Mempool final : public internal::Mempool
{
public:
    auto Dump(alloc::Default alloc) const noexcept
        -> Set<block::TransactionHash> final;
    auto Query(const block::TransactionHash& txid, alloc::Default alloc)
        const noexcept -> block::Transaction final;
    auto Submit(const block::TransactionHash& txid) const noexcept
        -> bool final;
    auto Submit(
        std::span<const block::TransactionHash> txids,
        alloc::Default alloc) const noexcept -> Vector<bool> final;
    auto Submit(block::Transaction tx) const noexcept -> void final;

    auto Heartbeat() noexcept -> void final;

    Mempool(
        const api::Session& api,
        const api::crypto::Blockchain& crypto,
        const Type chain,
        database::Wallet& db) noexcept;
    Mempool() = delete;
    Mempool(const Mempool&) = delete;
    Mempool(Mempool&&) = delete;
    auto operator=(const Mempool&) -> Mempool& = delete;
    auto operator=(Mempool&&) -> Mempool& = delete;

    ~Mempool() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node
