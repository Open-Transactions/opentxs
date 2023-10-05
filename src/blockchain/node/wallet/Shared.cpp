// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/Shared.hpp"  // IWYU pragma: associated

#include <exception>
#include <future>
#include <memory>
#include <utility>

#include "blockchain/node/spend/Imp.hpp"
#include "blockchain/node/wallet/proposals/Proposals.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Spend.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"         // IWYU pragma: keep
#include "opentxs/blockchain/block/TransactionHash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Spend.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
using namespace std::literals;

Shared::Shared(
    std::shared_ptr<const api::session::Client> api,
    std::shared_ptr<const node::Manager> node) noexcept
    : api_(*api)
    , chain_(node->Internal().Chain())
    , db_(node->Internal().DB())
    , fee_oracle_(api, node)
    , data_(api, node, db_)
{
}

auto Shared::ConstructTransaction(
    const node::Spend& spend,
    std::promise<SendOutcome>&& promise) const noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;

    if (data.proposals_.Add(spend, std::move(promise))) {
        data.to_actor_.SendDeferred(
            MakeWork(wallet::WalletJobs::statemachine),
            __FILE__,
            __LINE__,
            true);
    }
}

auto Shared::CreateSpend(const identifier::Nym& spender) const noexcept
    -> node::Spend
{
    return std::make_unique<wallet::SpendPrivate>(
               api_,
               chain_,
               spender,
               api_.Factory().PasswordPrompt("Spend "s.append(print(chain_))))
        .release();
}

auto Shared::Execute(node::Spend& spend) const noexcept -> PendingOutgoing
{
    static const auto blank = block::TransactionHash{};

    using enum SendResult;
    auto promise = std::promise<SendOutcome>{};
    auto future = promise.get_future();
    auto& internal = spend.Internal();

    try {
        internal.Finalize(
            api_.GetOptions().TestMode() ? LogConsole() : LogTrace(), {});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        promise.set_value(std::make_pair(UnspecifiedError, blank));

        return future;
    }

    if (auto error = internal.Check(); error) {
        promise.set_value(std::make_pair(*error, blank));
    } else {
        ConstructTransaction(spend, std::move(promise));
    }

    return future;
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
    const identifier::Account& node) const noexcept -> Balance
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
    const identifier::Account& subaccount,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return GetOutputs(owner, subaccount, TxoState::All, alloc);
}

auto Shared::GetOutputs(
    const identifier::Nym& owner,
    const identifier::Account& node,
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

auto Shared::Run() noexcept -> bool { return data_.lock()->proposals_.Run(); }

auto Shared::StartRescan() const noexcept -> bool
{
    return data_.lock()->to_actor_.SendDeferred(
        MakeWork(wallet::WalletJobs::rescan), __FILE__, __LINE__, true);
}

Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::wallet
