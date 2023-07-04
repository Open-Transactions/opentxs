// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/manager/Manager.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <span>
#include <string_view>
#include <utility>

#include "blockchain/node/manager/Actor.hpp"
#include "blockchain/node/manager/Shared.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/PaymentCode.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::blockchain::node::implementation
{
using namespace std::literals;

Base::Base(
    const api::Session& api,
    const Type type,
    const node::internal::Config& config,
    std::string_view seednode,
    node::Endpoints endpoints) noexcept
    : shared_(std::make_shared<manager::Shared>(
          api,
          type,
          config,
          seednode,
          std::move(endpoints)))
{
    OT_ASSERT(shared_);
}

Base::Base(
    const api::Session& api,
    const Type type,
    const node::internal::Config& config,
    std::string_view seednode) noexcept
    : Base(
          api,
          type,
          config,
          seednode,
          alloc::Default{}  // TODO allocator
      )
{
}

auto Base::AddBlock(const block::Block& block) const noexcept -> bool
{
    return shared_->AddBlock(block);
}

auto Base::AddPeer(const network::blockchain::Address& address) const noexcept
    -> bool
{
    return shared_->AddPeer(address);
}

auto Base::BlockOracle() const noexcept -> const node::BlockOracle&
{
    return shared_->BlockOracle();
}

auto Base::BroadcastTransaction(const block::Transaction& tx, const bool pushtx)
    const noexcept -> bool
{
    return shared_->BroadcastTransaction(tx, pushtx);
}

auto Base::Chain() const noexcept -> Type { return shared_->Chain(); }

auto Base::DB() const noexcept -> database::Database& { return shared_->DB(); }

auto Base::Endpoints() const noexcept -> const node::Endpoints&
{
    return shared_->Endpoints();
}

auto Base::FeeRate() const noexcept -> Amount { return shared_->FeeRate(); }

auto Base::FilterOracle() const noexcept -> const node::FilterOracle&
{
    return shared_->FilterOracle();
}

auto Base::GetBalance() const noexcept -> Balance
{
    return shared_->GetBalance();
}

auto Base::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    return shared_->GetBalance(owner);
}

auto Base::GetConfig() const noexcept -> const internal::Config&
{
    return shared_->GetConfig();
}

auto Base::GetShared() const noexcept -> std::shared_ptr<const node::Manager>
{
    return shared_->GetShared();
}

auto Base::GetTransactions() const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return shared_->GetTransactions();
}

auto Base::GetTransactions(const identifier::Nym& account) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return shared_->GetTransactions(account);
}

auto Base::GetType() const noexcept -> Type { return shared_->Chain(); }

auto Base::HeaderOracle() const noexcept -> const node::HeaderOracle&
{
    return shared_->HeaderOracle();
}

auto Base::Listen(const network::blockchain::Address& address) const noexcept
    -> bool
{
    return shared_->Listen(address);
}

auto Base::Mempool() const noexcept -> const internal::Mempool&
{
    return shared_->Mempool();
}

auto Base::Profile() const noexcept -> BlockchainProfile
{
    return shared_->Profile();
}

auto Base::SendToAddress(
    const opentxs::identifier::Nym& sender,
    std::string_view address,
    const Amount amount,
    std::string_view memo,
    std::span<const std::string_view> notify) const noexcept -> PendingOutgoing
{
    return shared_->SendToAddress(sender, address, amount, memo, notify);
}

auto Base::SendToPaymentCode(
    const opentxs::identifier::Nym& nymID,
    std::string_view recipient,
    const Amount amount,
    std::string_view memo,
    std::span<const std::string_view> notify) const noexcept -> PendingOutgoing
{
    return shared_->SendToPaymentCode(nymID, recipient, amount, memo, notify);
}

auto Base::SendToPaymentCode(
    const opentxs::identifier::Nym& nymID,
    const PaymentCode& recipient,
    const Amount amount,
    std::string_view memo,
    std::span<const PaymentCode> notify) const noexcept -> PendingOutgoing
{
    return shared_->SendToPaymentCode(nymID, recipient, amount, memo, notify);
}

auto Base::Shutdown() noexcept -> void { shared_->Shutdown(); }

auto Base::ShuttingDown() const noexcept -> bool
{
    return shared_->ShuttingDown();
}

auto Base::Start(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<node::Manager> me) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(me);
    OT_ASSERT(shared_);

    const auto& zmq = api->Network().ZeroMQ().Internal();
    const auto batchID = zmq.PreallocateBatch();
    auto* alloc = zmq.Alloc(batchID);
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr
    auto actor = boost::allocate_shared<manager::Actor>(
        alloc::PMR<manager::Actor>{alloc}, api, me, shared_, batchID);

    OT_ASSERT(actor);

    actor->Init(actor);
    shared_->Init(me);
}

auto Base::StartWallet() noexcept -> void { shared_->StartWallet(); }

auto Base::Sweep(
    const identifier::Nym& account,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept -> PendingOutgoing
{
    return shared_->Sweep(account, toAddress, notify);
}

auto Base::Sweep(
    const identifier::Nym& account,
    const identifier::Account& subaccount,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept -> PendingOutgoing
{
    return shared_->Sweep(account, subaccount, toAddress, notify);
}

auto Base::Sweep(
    const crypto::Key& key,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept -> PendingOutgoing
{
    return shared_->Sweep(key, toAddress, notify);
}

auto Base::Wallet() const noexcept -> const node::Wallet&
{
    return shared_->Wallet();
}

Base::~Base() { Shutdown(); }
}  // namespace opentxs::blockchain::node::implementation
