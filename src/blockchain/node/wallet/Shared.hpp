// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <future>
#include <memory>
#include <optional>

#include "blockchain/node/wallet/Data.hpp"
#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/wallet/FeeOracle.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Spend.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Client;
}  // namespace internal

class Client;
}  // namespace session
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
class Nym;
}  // namespace identifier

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class Wallet::Shared
{
public:
    virtual auto ConstructTransaction(
        const node::Spend& spend,
        std::promise<SendOutcome>&& promise) const noexcept -> void;
    virtual auto CreateSpend(const identifier::Nym& spender) const noexcept
        -> node::Spend;
    virtual auto Execute(node::Spend& spend) const noexcept -> PendingOutgoing;
    virtual auto FeeEstimate() const noexcept -> std::optional<Amount>;
    virtual auto GetBalance() const noexcept -> Balance;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance;
    virtual auto GetBalance(
        const identifier::Nym& owner,
        const identifier::Account& subaccount) const noexcept -> Balance;
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
        const identifier::Account& subaccount,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Account& subaccount,
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
        const node::Spend& spend,
        std::promise<SendOutcome>&& promise) const noexcept -> void final;
    auto CreateSpend(const identifier::Nym& spender) const noexcept
        -> node::Spend final;
    auto Execute(node::Spend&) const noexcept -> PendingOutgoing final;
    auto FeeEstimate() const noexcept -> std::optional<Amount> final;
    auto GetBalance() const noexcept -> Balance final;
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final;
    auto GetBalance(
        const identifier::Nym& owner,
        const identifier::Account& subaccount) const noexcept -> Balance final;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance final;
    auto GetOutputs(alloc::Default alloc) const noexcept -> Vector<UTXO> final;
    auto GetOutputs(TxoState type, alloc::Default alloc) const noexcept
        -> Vector<UTXO> final;
    auto GetOutputs(const identifier::Nym& owner, alloc::Default alloc)
        const noexcept -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Account& subaccount,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Account& subaccount,
        TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final;
    auto GetOutputs(const crypto::Key& key, TxoState type, alloc::Default alloc)
        const noexcept -> Vector<UTXO> final;
    auto GetTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<TxoTag> final;
    auto Height() const noexcept -> block::Height final;
    auto StartRescan() const noexcept -> bool final;

    auto Run() noexcept -> bool final;

    Shared(
        std::shared_ptr<const api::session::internal::Client> api,
        std::shared_ptr<const node::Manager> node) noexcept;

    ~Shared() final;

private:
    using GuardedData = libguarded::plain_guarded<wallet::Data>;

    const api::session::Client& api_;
    const blockchain::Type chain_;
    database::Wallet& db_;
    wallet::FeeOracle fee_oracle_;
    mutable GuardedData data_;
};
}  // namespace opentxs::blockchain::node::wallet
