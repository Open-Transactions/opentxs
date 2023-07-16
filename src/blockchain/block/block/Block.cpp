// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/block/Block.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/block/block/BlockPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::block
{
auto operator==(const Block& lhs, const Block& rhs) noexcept -> bool
{
    return lhs.Header().Hash() == rhs.Header().Hash();
}

auto operator<=>(const Block& lhs, const Block& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.Header().Hash() <=> rhs.Header().Hash();
}

auto swap(Block& lhs, Block& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block
{
Block::Block(BlockPrivate* imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(nullptr != imp_);
}

Block::Block(allocator_type alloc) noexcept
    : Block(BlockPrivate::Blank(alloc))
{
}

Block::Block(const Block& rhs, allocator_type alloc) noexcept
    : Block(rhs.imp_->clone(alloc))
{
}

Block::Block(Block&& rhs) noexcept
    : Block(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Block::Block(Block&& rhs, allocator_type alloc) noexcept
    : Block(alloc)
{
    operator=(std::move(rhs));
}

auto Block::asBitcoin() const& noexcept -> const bitcoin::block::Block&
{
    return imp_->asBitcoinPublic();
}

auto Block::asBitcoin() & noexcept -> bitcoin::block::Block&
{
    return imp_->asBitcoinPublic();
}

auto Block::asBitcoin() && noexcept -> bitcoin::block::Block
{
    auto out = bitcoin::block::Block{imp_};
    imp_ = nullptr;

    return out;
}

auto Block::Blank() noexcept -> Block&
{
    static auto blank = Block{};

    return blank;
}

auto Block::ContainsHash(const TransactionHash& hash) const noexcept -> bool
{
    return imp_->ContainsHash(hash);
}

auto Block::ContainsID(const TransactionHash& id) const noexcept -> bool
{
    return imp_->ContainsHash(id);
}

auto Block::FindByHash(const TransactionHash& hash) const noexcept
    -> const Transaction&
{
    return imp_->FindByHash(hash);
}

auto Block::FindByID(const TransactionHash& id) const noexcept
    -> const Transaction&
{
    return imp_->FindByID(id);
}

auto Block::get() const noexcept -> std::span<const Transaction>
{
    return imp_->get();
}

auto Block::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Block::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
}

auto Block::Internal() const noexcept -> const internal::Block&
{
    return *imp_;
}

auto Block::Header() const noexcept -> const block::Header&
{
    return imp_->Header();
}

auto Block::ID() const noexcept -> const block::Hash& { return imp_->ID(); }

auto Block::Internal() noexcept -> internal::Block& { return *imp_; }

auto Block::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Block::operator=(const Block& rhs) noexcept -> Block&
{
    return copy_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Block::operator=(Block&& rhs) noexcept -> Block&
{
    return move_assign_base(*this, std::move(rhs), imp_, rhs.imp_);
}

auto Block::Print(const api::Crypto& crypto) const noexcept
    -> UnallocatedCString
{
    return imp_->Print(crypto);
}

auto Block::Print(const api::Crypto& crypto, allocator_type alloc)
    const noexcept -> CString
{
    return imp_->Print(crypto, alloc);
}

auto Block::Serialize(Writer&& bytes) const noexcept -> bool
{
    return imp_->Serialize(std::move(bytes));
}

auto Block::size() const noexcept -> std::size_t { return imp_->size(); }

auto Block::swap(Block& rhs) noexcept -> void
{
    pmr_swap(*this, rhs, imp_, rhs.imp_);
}

Block::~Block() { pmr_delete(imp_); }
}  // namespace opentxs::blockchain::block
