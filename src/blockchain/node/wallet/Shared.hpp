// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"

#pragma once

#include <cs_plain_guarded.h>
#include <future>
#include <memory>
#include <optional>

#include "blockchain/node/wallet/spend/Proposals.hpp"
#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/wallet/FeeOracle.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Outpoint;
}  // namespace block

namespace database
{
class Wallet;
}  // namespace database

namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class Wallet::Shared
{
public:
    virtual auto ConstructTransaction(
        const proto::BlockchainTransactionProposal& tx,
        std::promise<SendOutcome>&& promise) const noexcept -> void;
    virtual auto FeeEstimate() const noexcept -> std::optional<Amount>;
    virtual auto GetBalance() const noexcept -> Balance;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance;
    virtual auto GetBalance(
        const identifier::Nym& owner,
        const identifier::Generic& subaccount) const noexcept -> Balance;
    virtual auto GetBalance(const crypto::Key& key) const noexcept -> Balance;
    virtual auto GetOutputs(alloc::Default alloc) const noexcept
        -> Vector<UTXO>;
    virtual auto GetOutputs(TxoState type, alloc::Default alloc) const noexcept
        -> Vector<UTXO>;
    virtual auto GetOutputs(const identifier::Nym& owner, alloc::Default alloc)
        const noexcept -> Vector<UTXO>;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Generic& subaccount,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Generic& subaccount,
        TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    virtual auto GetOutputs(
        const crypto::Key& key,
        TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    virtual auto GetTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<TxoTag>;
    virtual auto Height() const noexcept -> block::Height;
    virtual auto StartRescan() const noexcept -> bool { return false; }

    virtual auto Run() noexcept -> bool;

    virtual ~Shared() = default;
};
}  // namespace opentxs::blockchain::node::internal

namespace opentxs::blockchain::node::wallet
{
class Shared final : public internal::Wallet::Shared
{
public:
    auto ConstructTransaction(
        const proto::BlockchainTransactionProposal& tx,
        std::promise<SendOutcome>&& promise) const noexcept -> void final;
    auto FeeEstimate() const noexcept -> std::optional<Amount> final;
    auto GetBalance() const noexcept -> Balance final;
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final;
    auto GetBalance(
        const identifier::Nym& owner,
        const identifier::Generic& subaccount) const noexcept -> Balance final;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance final;
    auto GetOutputs(alloc::Default alloc) const noexcept
        -> Vector<Wallet::UTXO> final;
    auto GetOutputs(TxoState type, alloc::Default alloc) const noexcept
        -> Vector<Wallet::UTXO> final;
    auto GetOutputs(const identifier::Nym& owner, alloc::Default alloc)
        const noexcept -> Vector<Wallet::UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        TxoState type,
        alloc::Default alloc) const noexcept -> Vector<Wallet::UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Generic& subaccount,
        alloc::Default alloc) const noexcept -> Vector<Wallet::UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Generic& subaccount,
        TxoState type,
        alloc::Default alloc) const noexcept -> Vector<Wallet::UTXO> final;
    auto GetOutputs(const crypto::Key& key, TxoState type, alloc::Default alloc)
        const noexcept -> Vector<Wallet::UTXO> final;
    auto GetTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<TxoTag> final;
    auto Height() const noexcept -> block::Height final;
    auto StartRescan() const noexcept -> bool final;

    auto Run() noexcept -> bool final;

    Shared(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept;

    ~Shared() final;

private:
    using Proposals = libguarded::plain_guarded<wallet::Proposals>;
    using Socket = libguarded::plain_guarded<network::zeromq::socket::Raw>;

    database::Wallet& db_;
    wallet::FeeOracle fee_oracle_;
    mutable Proposals proposals_;
    mutable Socket to_actor_;
};
}  // namespace opentxs::blockchain::node::wallet
