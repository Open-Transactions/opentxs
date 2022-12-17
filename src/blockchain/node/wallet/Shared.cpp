// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/Shared.hpp"  // IWYU pragma: associated

#include <future>
#include <memory>
#include <utility>

#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"   // IWYU pragma: keep
#include "opentxs/blockchain/block/TransactionHash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::internal
{
auto Wallet::Shared::ConstructTransaction(
    const proto::BlockchainTransactionProposal&,
    std::promise<SendOutcome>&&) const noexcept -> void
{
}

auto Wallet::Shared::FeeEstimate() const noexcept -> std::optional<Amount>
{
    return {};
}

auto Wallet::Shared::GetBalance() const noexcept -> Balance { return {}; }

auto Wallet::Shared::GetBalance(const identifier::Nym&) const noexcept
    -> Balance
{
    return {};
}

auto Wallet::Shared::GetBalance(
    const identifier::Nym&,
    const identifier::Generic&) const noexcept -> Balance
{
    return {};
}

auto Wallet::Shared::GetBalance(const crypto::Key&) const noexcept -> Balance
{
    return {};
}

auto Wallet::Shared::GetOutputs(alloc::Default) const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(TxoState, alloc::Default) const noexcept
    -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(const identifier::Nym&, alloc::Default)
    const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(
    const identifier::Nym&,
    TxoState,
    alloc::Default) const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(
    const identifier::Nym&,
    const identifier::Generic&,
    alloc::Default) const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(
    const identifier::Nym&,
    const identifier::Generic&,
    TxoState,
    alloc::Default) const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(const crypto::Key&, TxoState, alloc::Default)
    const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetTags(const block::Outpoint&) const noexcept
    -> UnallocatedSet<TxoTag>
{
    return {};
}

auto Wallet::Shared::Height() const noexcept -> block::Height { return {}; }

auto Wallet::Shared::Run() noexcept -> bool { return {}; }
}  // namespace opentxs::blockchain::node::internal

namespace opentxs::blockchain::node::wallet
{
Shared::Shared(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept
    : db_(node->Internal().DB())
    , fee_oracle_(api, node)
    , proposals_(*api, *node, db_, node->Internal().Chain())
    , to_actor_([&] {
        using Type = network::zeromq::socket::Type;
        auto out = api->Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto rc =
            out.Connect(node->Internal().Endpoints().wallet_pull_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto Shared::ConstructTransaction(
    const proto::BlockchainTransactionProposal& tx,
    std::promise<SendOutcome>&& promise) const noexcept -> void
{
    proposals_.lock()->Add(tx, std::move(promise));
    to_actor_.lock()->SendDeferred(
        MakeWork(wallet::WalletJobs::statemachine), __FILE__, __LINE__, true);
}

auto Shared::FeeEstimate() const noexcept -> std::optional<Amount>
{
    return fee_oracle_.EstimatedFee();
}

auto Shared::GetBalance() const noexcept -> Balance { return db_.GetBalance(); }

auto Shared::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    return db_.GetBalance(owner);
}

auto Shared::GetBalance(
    const identifier::Nym& owner,
    const identifier::Generic& node) const noexcept -> Balance
{
    return db_.GetBalance(owner, node);
}

auto Shared::GetBalance(const crypto::Key& key) const noexcept -> Balance
{
    return db_.GetBalance(key);
}

auto Shared::GetOutputs(alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return GetOutputs(TxoState::All, alloc);
}

auto Shared::GetOutputs(TxoState type, alloc::Default alloc) const noexcept
    -> Vector<UTXO>
{
    return db_.GetOutputs(type, alloc);
}

auto Shared::GetOutputs(const identifier::Nym& owner, alloc::Default alloc)
    const noexcept -> Vector<UTXO>
{
    return GetOutputs(owner, TxoState::All, alloc);
}

auto Shared::GetOutputs(
    const identifier::Nym& owner,
    TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return db_.GetOutputs(owner, type, alloc);
}

auto Shared::GetOutputs(
    const identifier::Nym& owner,
    const identifier::Generic& subaccount,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return GetOutputs(owner, subaccount, TxoState::All, alloc);
}

auto Shared::GetOutputs(
    const identifier::Nym& owner,
    const identifier::Generic& node,
    TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return db_.GetOutputs(owner, node, type, alloc);
}

auto Shared::GetOutputs(
    const crypto::Key& key,
    TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return db_.GetOutputs(key, type, alloc);
}

auto Shared::GetTags(const block::Outpoint& output) const noexcept
    -> UnallocatedSet<TxoTag>
{
    return db_.GetOutputTags(output);
}

auto Shared::Height() const noexcept -> block::Height
{
    return db_.GetWalletHeight();
}

auto Shared::Run() noexcept -> bool { return proposals_.lock()->Run(); }

auto Shared::StartRescan() const noexcept -> bool
{
    return to_actor_.lock()->SendDeferred(
        MakeWork(wallet::WalletJobs::rescan), __FILE__, __LINE__, true);
}

Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::wallet
