// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/Wallet.hpp"  // IWYU pragma: associated

#include <memory>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/Actor.hpp"
#include "blockchain/node/wallet/Shared.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"         // IWYU pragma: keep
#include "opentxs/blockchain/block/TransactionHash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Spend.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(WalletJobs job) noexcept -> std::string_view
{
    try {
        using enum WalletJobs;
        static const auto map = Map<WalletJobs, CString>{
            {shutdown, "shutdown"},
            {start_wallet, "start_wallet"},
            {rescan, "rescan"},
            {init, "init"},
            {statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid WalletJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::internal
{
Wallet::Wallet() noexcept
    : shared_()
{
}

auto Wallet::CreateSpend(const identifier::Nym& spender) const noexcept
    -> node::Spend
{
    return shared_->CreateSpend(spender);
}

auto Wallet::Execute(node::Spend& spend) const noexcept -> PendingOutgoing
{
    return shared_->Execute(spend);
}

auto Wallet::FeeEstimate() const noexcept -> std::optional<Amount>
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->FeeEstimate();
}

auto Wallet::GetBalance() const noexcept -> Balance
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetBalance();
}

auto Wallet::GetBalance(const crypto::Key& key) const noexcept -> Balance
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetBalance(key);
}

auto Wallet::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetBalance(owner);
}

auto Wallet::GetBalance(
    const identifier::Nym& owner,
    const identifier::Account& subaccount) const noexcept -> Balance
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetBalance(owner, subaccount);
}

auto Wallet::GetOutputs(TxoState type, alloc::Default alloc) const noexcept
    -> Vector<UTXO>
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetOutputs(std::move(type), std::move(alloc));
}

auto Wallet::GetOutputs(alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetOutputs(std::move(alloc));
}

auto Wallet::GetOutputs(
    const crypto::Key& key,
    TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetOutputs(key, std::move(type), std::move(alloc));
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetOutputs(owner, std::move(type), std::move(alloc));
}

auto Wallet::GetOutputs(const identifier::Nym& owner, alloc::Default alloc)
    const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetOutputs(owner, std::move(alloc));
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const identifier::Account& subaccount,
    TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetOutputs(
        owner, subaccount, std::move(type), std::move(alloc));
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const identifier::Account& subaccount,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetOutputs(owner, subaccount, std::move(alloc));
}

auto Wallet::GetTags(const block::Outpoint& output) const noexcept
    -> UnallocatedSet<TxoTag>
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->GetTags(output);
}

auto Wallet::Height() const noexcept -> block::Height
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->Height();
}

auto Wallet::StartRescan() const noexcept -> bool
{
    auto shared{shared_};

    assert_false(nullptr == shared);

    return shared->StartRescan();
}

auto Wallet::Init(
    std::shared_ptr<const api::session::internal::Client> api,
    std::shared_ptr<const node::Manager> node) noexcept -> void
{
    assert_false(nullptr == api);
    assert_false(nullptr == node);

    if (node->Internal().GetConfig().disable_wallet_) {
        shared_ = std::make_shared<Shared>();
    } else {
        const auto& asio = api->Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        shared_ = std::make_shared<wallet::Shared>(api, node);
        auto actor = std::allocate_shared<Wallet::Actor>(
            alloc::PMR<Wallet::Actor>{asio.Alloc(batchID)},
            api,
            node,
            shared_,
            batchID);

        assert_false(nullptr == actor);

        actor->Init(actor);
    }

    assert_false(nullptr == shared_);
}

Wallet::~Wallet() = default;
}  // namespace opentxs::blockchain::node::internal
