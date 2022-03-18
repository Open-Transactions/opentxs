// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
template <class T>
class shared_ptr;
}  // namespace boost

namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace crypto
{
class Deterministic;
}  // namespace crypto

namespace node
{
namespace internal
{
struct Mempool;
struct Network;
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
class Accounts;
class Progress;
class Rescan;
class Scan;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Push;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class DeterministicStateData final : public SubchainStateData
{
public:
    const crypto::Deterministic& subaccount_;

    DeterministicStateData(
        const api::Session& api,
        const node::internal::Network& node,
        node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const crypto::Deterministic& subaccount,
        const cfilter::Type filter,
        const Subchain subchain,
        const network::zeromq::BatchID batch,
        const std::string_view parent,
        allocator_type alloc) noexcept;

    ~DeterministicStateData() final = default;

private:
    using MatchedTransaction = std::
        pair<UnallocatedVector<Bip32Index>, const block::bitcoin::Transaction*>;

    auto get_index(const SubchainStateData& me) const noexcept -> Index final;
    auto handle_confirmed_matches(
        const block::bitcoin::Block& block,
        const block::Position& position,
        const block::Matches& confirmed) const noexcept -> void final;
    auto handle_mempool_matches(
        const block::Matches& matches,
        std::unique_ptr<const block::bitcoin::Transaction> tx) const noexcept
        -> void final;
    auto process(
        const block::Match match,
        const block::bitcoin::Transaction& tx,
        MatchedTransaction& output) const noexcept -> void;
    auto ReportScan(const block::Position& pos) const noexcept -> void final;

    DeterministicStateData() = delete;
    DeterministicStateData(const DeterministicStateData&) = delete;
    DeterministicStateData(DeterministicStateData&&) = delete;
    DeterministicStateData& operator=(const DeterministicStateData&) = delete;
    DeterministicStateData& operator=(DeterministicStateData&&) = delete;
};
}  // namespace opentxs::blockchain::node::wallet
