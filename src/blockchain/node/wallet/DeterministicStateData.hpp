// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

#include "blockchain/node/wallet/Index.hpp"
#include "blockchain/node/wallet/SubchainStateData.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
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
struct Network;
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
class Outstanding;
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class DeterministicStateData final : public SubchainStateData
{
public:
    const crypto::Deterministic& subaccount_;

    DeterministicStateData(
        const api::Session& api,
        const node::internal::Network& node,
        Accounts& parent,
        const WalletDatabase& db,
        const crypto::Deterministic& subaccount,
        const std::function<void(const Identifier&, const char*)>& taskFinished,
        Outstanding& jobCounter,
        const filter::Type filter,
        const Subchain subchain) noexcept;

    ~DeterministicStateData() final = default;

private:
    using MatchedTransaction = std::
        pair<std::pmr::vector<Bip32Index>, const block::bitcoin::Transaction*>;

    class Index final : public wallet::Index
    {
    public:
        auto Do(std::optional<Bip32Index> current, Bip32Index target) noexcept
            -> void final;

        Index(
            const crypto::Deterministic& subaccount,
            SubchainStateData& parent,
            Scan& scan,
            Rescan& rescan,
            Progress& progress) noexcept;

        ~Index() final = default;

    private:
        const crypto::Deterministic& subaccount_;

        auto need_index(const std::optional<Bip32Index>& current) const noexcept
            -> std::optional<Bip32Index> final;
    };

    Index index_;

    auto report_scan(const block::Position& pos) const noexcept -> void final;
    auto type() const noexcept -> std::stringstream final;

    auto get_index() noexcept -> Index& final { return index_; }
    auto handle_confirmed_matches(
        const block::bitcoin::Block& block,
        const block::Position& position,
        const block::Matches& confirmed) noexcept -> void final;
    auto handle_mempool_matches(
        const block::Matches& matches,
        std::unique_ptr<const block::bitcoin::Transaction> tx) noexcept
        -> void final;
    auto process(
        const block::Match match,
        const block::bitcoin::Transaction& tx,
        MatchedTransaction& output) noexcept -> void;

    DeterministicStateData() = delete;
    DeterministicStateData(const DeterministicStateData&) = delete;
    DeterministicStateData(DeterministicStateData&&) = delete;
    DeterministicStateData& operator=(const DeterministicStateData&) = delete;
    DeterministicStateData& operator=(DeterministicStateData&&) = delete;
};
}  // namespace opentxs::blockchain::node::wallet
