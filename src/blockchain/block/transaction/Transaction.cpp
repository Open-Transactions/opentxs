// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "opentxs/blockchain/block/Transaction.hpp"  // IWYU pragma: associated

#include <functional>
#include <utility>

#include "blockchain/block/transaction/TransactionPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

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
    OT_ASSERT(nullptr != imp_);
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
    : Transaction(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Transaction::Transaction(Transaction&& rhs, allocator_type alloc) noexcept
    : Transaction(alloc)
{
    operator=(std::move(rhs));
}

auto Transaction::asBitcoin() const& noexcept
    -> const bitcoin::block::Transaction&
{
    return imp_->asBitcoinPublic();
}

auto Transaction::asBitcoin() & noexcept -> bitcoin::block::Transaction&
{
    return imp_->asBitcoinPublic();
}

auto Transaction::asBitcoin() && noexcept -> bitcoin::block::Transaction
{
    auto out = bitcoin::block::Transaction{imp_};
    imp_ = nullptr;

    return out;
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
    if (imp_ != rhs.imp_) {
        if (nullptr == imp_) {
            // NOTE moved-from state
            imp_ = rhs.imp_->clone(rhs.imp_->get_allocator());
        } else {
            auto* old{imp_};
            imp_ = rhs.imp_->clone(get_allocator());
            // TODO switch to destroying delete after resolution of
            // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107352
            auto deleter = old->get_deleter();
            std::invoke(deleter);
        }
    }

    return *this;
}

auto Transaction::operator=(Transaction&& rhs) noexcept -> Transaction&
{
    if (nullptr == imp_) {
        // NOTE moved-from state
        swap(rhs);

        return *this;
    } else if (get_allocator() == rhs.get_allocator()) {
        swap(rhs);

        return *this;
    } else {

        return operator=(rhs);
    }
}

auto Transaction::Print() const noexcept -> UnallocatedCString
{
    return imp_->Print();
}

auto Transaction::Print(allocator_type alloc) const noexcept -> CString
{
    return imp_->Print(alloc);
}

auto Transaction::swap(Transaction& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

Transaction::~Transaction()
{
    if (nullptr != imp_) {
        // TODO switch to destroying delete after resolution of
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107352
        auto deleter = imp_->get_deleter();
        std::invoke(deleter);
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::block
