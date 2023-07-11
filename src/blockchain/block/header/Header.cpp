// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/block/Header.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/block/header/HeaderPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/NumericHash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::block
{
auto operator==(const Header& lhs, const Header& rhs) noexcept -> bool
{
    return lhs.Hash() == rhs.Hash();
}

auto operator<=>(const Header& lhs, const Header& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.Hash() <=> rhs.Hash();
}

auto swap(Header& lhs, Header& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block
{
Header::Header(HeaderPrivate* imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(nullptr != imp_);
}

Header::Header(allocator_type alloc) noexcept
    : Header(HeaderPrivate::Blank(alloc))
{
}

Header::Header(const Header& rhs, allocator_type alloc) noexcept
    : Header(rhs.imp_->clone(alloc))
{
}

Header::Header(Header&& rhs) noexcept
    : Header(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Header::Header(Header&& rhs, allocator_type alloc) noexcept
    : Header(alloc)
{
    operator=(std::move(rhs));
}

auto Header::asBitcoin() const& noexcept -> const bitcoin::block::Header&
{
    return imp_->asBitcoinPublic();
}

auto Header::asBitcoin() & noexcept -> bitcoin::block::Header&
{
    return imp_->asBitcoinPublic();
}

auto Header::asBitcoin() && noexcept -> bitcoin::block::Header
{
    auto out = bitcoin::block::Header{imp_};
    imp_ = nullptr;

    return out;
}

auto Header::Blank() noexcept -> Header&
{
    static auto blank = Header{};

    return blank;
}

auto Header::Difficulty() const noexcept -> blockchain::Work
{
    return imp_->Difficulty();
}

auto Header::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Header::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
}

auto Header::Hash() const noexcept -> const block::Hash&
{
    return imp_->Hash();
}

auto Header::Height() const noexcept -> block::Height { return imp_->Height(); }

auto Header::IncrementalWork() const noexcept -> blockchain::Work
{
    return imp_->IncrementalWork();
}

auto Header::Internal() const noexcept -> const internal::Header&
{
    return *imp_;
}

auto Header::Internal() noexcept -> internal::Header& { return *imp_; }

auto Header::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Header::NumericHash() const noexcept -> block::NumericHash
{
    return imp_->NumericHash();
}

auto Header::operator=(const Header& rhs) noexcept -> Header&
{
    return copy_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Header::operator=(Header&& rhs) noexcept -> Header&
{
    return move_assign_base(*this, std::move(rhs), imp_, rhs.imp_);
}

auto Header::ParentHash() const noexcept -> const block::Hash&
{
    return imp_->ParentHash();
}

auto Header::ParentWork() const noexcept -> blockchain::Work
{
    return imp_->ParentWork();
}

auto Header::Position() const noexcept -> block::Position
{
    return imp_->Position();
}

auto Header::Print() const noexcept -> UnallocatedCString
{
    return imp_->Print();
}

auto Header::Print(allocator_type alloc) const noexcept -> CString
{
    return imp_->Print(alloc);
}

auto Header::Serialize(Writer&& destination, const bool bitcoinformat)
    const noexcept -> bool
{
    return imp_->Serialize(std::move(destination), bitcoinformat);
}

auto Header::swap(Header& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

auto Header::Target() const noexcept -> block::NumericHash
{
    return imp_->Target();
}

auto Header::Type() const noexcept -> blockchain::Type { return imp_->Type(); }

auto Header::Valid() const noexcept -> bool { return imp_->Valid(); }

auto Header::Work() const noexcept -> blockchain::Work { return imp_->Work(); }

Header::~Header() { pmr_delete(imp_); }
}  // namespace opentxs::blockchain::block
