// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/bitcoin/block/transaction/TransactionPrivate.hpp"
#include "blockchain/block/transaction/TransactionPrivate.hpp"

namespace opentxs::blockchain::bitcoin::block
{
Transaction::Transaction(blockchain::block::TransactionPrivate* imp) noexcept
    : blockchain::block::Transaction(std::move(imp))
{
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

auto Transaction::Blank() noexcept -> Transaction&
{
    static auto blank = Transaction{};

    return blank;
}

auto Transaction::Inputs() const noexcept -> std::span<const block::Input>
{
    return imp_->asBitcoinPrivate()->Inputs();
}

auto Transaction::IsGeneration() const noexcept -> bool
{
    return imp_->asBitcoinPrivate()->IsGeneration();
}

auto Transaction::Locktime() const noexcept -> std::uint32_t
{
    return imp_->asBitcoinPrivate()->Locktime();
}

auto Transaction::operator=(const Transaction& rhs) noexcept -> Transaction&
{
    blockchain::block::Transaction::operator=(rhs);

    return *this;
}

auto Transaction::operator=(Transaction&& rhs) noexcept -> Transaction&
{
    blockchain::block::Transaction::operator=(std::move(rhs));

    return *this;
}

auto Transaction::Outputs() const noexcept -> std::span<const block::Output>
{
    return imp_->asBitcoinPrivate()->Outputs();
}

auto Transaction::SegwitFlag() const noexcept -> std::byte
{
    return imp_->asBitcoinPrivate()->SegwitFlag();
}

auto Transaction::Timestamp() const noexcept -> Time
{
    return imp_->asBitcoinPrivate()->Timestamp();
}

auto Transaction::Version() const noexcept -> std::int32_t
{
    return imp_->asBitcoinPrivate()->Version();
}

auto Transaction::vBytes(blockchain::Type chain) const noexcept -> std::size_t
{
    return imp_->asBitcoinPrivate()->vBytes(chain);
}

Transaction::~Transaction() = default;
}  // namespace opentxs::blockchain::bitcoin::block
