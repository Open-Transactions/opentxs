// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/block/Transaction.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/block/transaction/TransactionPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::block
{
auto operator==(const Transaction& lhs, const Transaction& rhs) noexcept -> bool
{
    return lhs.ID() == rhs.ID();
}

auto operator<=>(const Transaction& lhs, const Transaction& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.ID() <=> rhs.ID();
}

auto swap(Transaction& lhs, Transaction& rhs) noexcept -> void
{
    lhs.swap(rhs);
}
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block
{
Transaction::Transaction(TransactionPrivate* imp) noexcept
    : imp_(std::move(imp))
{
    assert_false(nullptr == imp_);
}

Transaction::Transaction(allocator_type alloc) noexcept
    : Transaction(TransactionPrivate::Blank(alloc))
{
}

Transaction::Transaction(const Transaction& rhs, allocator_type alloc) noexcept
    : Transaction(rhs.imp_->clone(alloc))
{
}

Transaction::Transaction(Transaction&& rhs) noexcept
    : Transaction(std::exchange(rhs.imp_, nullptr))
{
}

Transaction::Transaction(Transaction&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto Transaction::asBitcoin() const& noexcept
    -> const protocol::bitcoin::base::block::Transaction&
{
    return imp_->asBitcoinPublic();
}

auto Transaction::asBitcoin() & noexcept
    -> protocol::bitcoin::base::block::Transaction&
{
    return imp_->asBitcoinPublic();
}

auto Transaction::asBitcoin() && noexcept
    -> protocol::bitcoin::base::block::Transaction
{
    return std::exchange(imp_, nullptr);
}

auto Transaction::AssociatedLocalNyms(
    const api::crypto::Blockchain& crypto,
    allocator_type alloc) const noexcept -> Set<identifier::Nym>
{
    return imp_->AssociatedLocalNyms(crypto, alloc);
}

auto Transaction::AssociatedRemoteContacts(
    const api::session::Client& api,
    const identifier::Nym& nym,
    allocator_type alloc) const noexcept -> Set<identifier::Generic>
{
    return imp_->AssociatedRemoteContacts(api, nym, alloc);
}

auto Transaction::Blank() noexcept -> Transaction&
{
    static auto blank = Transaction{};

    return blank;
}

auto Transaction::BlockPosition() const noexcept -> std::optional<std::size_t>
{
    return imp_->BlockPosition();
}

auto Transaction::Chains(allocator_type alloc) const noexcept
    -> Set<blockchain::Type>
{
    return imp_->Chains(alloc);
}

auto Transaction::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Transaction::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
}

auto Transaction::Internal() const noexcept -> const internal::Transaction&
{
    return *imp_;
}

auto Transaction::Hash() const noexcept -> const TransactionHash&
{
    return imp_->Hash();
}

auto Transaction::ID() const noexcept -> const TransactionHash&
{
    return imp_->ID();
}

auto Transaction::Internal() noexcept -> internal::Transaction&
{
    return *imp_;
}

auto Transaction::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Transaction::Keys(allocator_type alloc) const noexcept -> Set<crypto::Key>
{
    return imp_->Keys(alloc);
}

auto Transaction::Memo(const api::crypto::Blockchain& crypto) const noexcept
    -> UnallocatedCString
{
    return imp_->Memo(crypto);
}

auto Transaction::Memo(
    const api::crypto::Blockchain& crypto,
    allocator_type alloc) const noexcept -> CString
{
    return imp_->Memo(crypto, alloc);
}

auto Transaction::NetBalanceChange(
    const api::crypto::Blockchain& crypto,
    const identifier::Nym& nym) const noexcept -> opentxs::Amount
{
    return imp_->NetBalanceChange(crypto, nym);
}

auto Transaction::operator=(const Transaction& rhs) noexcept -> Transaction&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto Transaction::operator=(Transaction&& rhs) noexcept -> Transaction&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Transaction::Print(const api::Crypto& crypto) const noexcept
    -> UnallocatedCString
{
    return imp_->Print(crypto);
}

auto Transaction::Print(const api::Crypto& crypto, allocator_type alloc)
    const noexcept -> CString
{
    return imp_->Print(crypto, alloc);
}

auto Transaction::swap(Transaction& rhs) noexcept -> void
{
    pmr::swap(imp_, rhs.imp_);
}

Transaction::~Transaction() { pmr::destroy(imp_); }
}  // namespace opentxs::blockchain::block
