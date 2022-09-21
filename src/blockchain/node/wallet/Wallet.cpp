// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "internal/blockchain/node/Wallet.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <future>
#include <memory>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/Actor.hpp"
#include "blockchain/node/wallet/Shared.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(WalletJobs job) noexcept -> std::string_view
{
    try {
        using Job = WalletJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::start_wallet, "start_wallet"},
            {Job::rescan, "rescan"},
            {Job::init, "init"},
            {Job::statemachine, "statemachine"},
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

auto Wallet::ConstructTransaction(
    const proto::BlockchainTransactionProposal& tx,
    std::promise<SendOutcome>&& promise) const noexcept -> void
{
    auto shared{shared_};

    OT_ASSERT(shared);

    shared->ConstructTransaction(tx, std::move(promise));
}

auto Wallet::FeeEstimate() const noexcept -> std::optional<Amount>
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->FeeEstimate();
}

auto Wallet::GetBalance() const noexcept -> Balance
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetBalance();
}

auto Wallet::GetBalance(const crypto::Key& key) const noexcept -> Balance
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetBalance(key);
}

auto Wallet::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetBalance(owner);
}

auto Wallet::GetBalance(
    const identifier::Nym& owner,
    const identifier::Generic& subaccount) const noexcept -> Balance
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetBalance(owner, subaccount);
}

auto Wallet::GetOutputs(TxoState type, alloc::Default alloc) const noexcept
    -> Vector<UTXO>
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetOutputs(std::move(type), std::move(alloc));
}

auto Wallet::GetOutputs(alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetOutputs(std::move(alloc));
}

auto Wallet::GetOutputs(
    const crypto::Key& key,
    TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetOutputs(key, std::move(type), std::move(alloc));
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetOutputs(owner, std::move(type), std::move(alloc));
}

auto Wallet::GetOutputs(const identifier::Nym& owner, alloc::Default alloc)
    const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetOutputs(owner, std::move(alloc));
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const identifier::Generic& subaccount,
    TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetOutputs(
        owner, subaccount, std::move(type), std::move(alloc));
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const identifier::Generic& subaccount,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetOutputs(owner, subaccount, std::move(alloc));
}

auto Wallet::GetTags(const block::Outpoint& output) const noexcept
    -> UnallocatedSet<TxoTag>
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->GetTags(output);
}

auto Wallet::Height() const noexcept -> block::Height
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->Height();
}

auto Wallet::StartRescan() const noexcept -> bool
{
    auto shared{shared_};

    OT_ASSERT(shared);

    return shared->StartRescan();
}

auto Wallet::Init(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(node);

    if (node->Internal().GetConfig().disable_wallet_) {
        shared_ = boost::make_shared<Shared>();
    } else {
        const auto& asio = api->Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr
        shared_ = boost::make_shared<wallet::Shared>(api, node);
        auto actor = boost::allocate_shared<Wallet::Actor>(
            alloc::PMR<Wallet::Actor>{asio.Alloc(batchID)},
            api,
            node,
            shared_,
            batchID);

        OT_ASSERT(actor);

        actor->Init(actor);
    }

    OT_ASSERT(shared_);
}

Wallet::~Wallet() = default;
}  // namespace opentxs::blockchain::node::internal
