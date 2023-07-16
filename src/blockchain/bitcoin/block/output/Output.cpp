// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/bitcoin/block/output/OutputPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::bitcoin::block
{
Output::Output(OutputPrivate* imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(nullptr != imp_);
}

Output::Output(allocator_type alloc) noexcept
    : Output(OutputPrivate::Blank(alloc))
{
}

Output::Output(const Output& rhs, allocator_type alloc) noexcept
    : Output(rhs.imp_->clone(alloc))
{
}

Output::Output(Output&& rhs) noexcept
    : Output(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Output::Output(Output&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    if (rhs.get_allocator() == alloc) {
        swap(rhs);
    } else {
        imp_ = rhs.imp_->clone(alloc);
    }
}

auto Output::Blank() noexcept -> Output&
{
    static auto blank = Output{};

    return blank;
}

auto Output::Cashtoken() const noexcept -> const token::cashtoken::View*
{
    return imp_->Cashtoken();
}

auto Output::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Output::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
}

auto Output::Internal() const noexcept -> const internal::Output&
{
    return *imp_;
}

auto Output::Internal() noexcept -> internal::Output& { return *imp_; }

auto Output::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Output::Keys(Set<crypto::Key>& out) const noexcept -> void
{
    return imp_->Keys(out);
}

auto Output::Keys(allocator_type alloc) const noexcept -> Set<crypto::Key>
{
    return imp_->Keys(alloc);
}

auto Output::Note(const api::crypto::Blockchain& crypto) const noexcept
    -> UnallocatedCString
{
    return imp_->Note(crypto);
}

auto Output::Note(const api::crypto::Blockchain& crypto, allocator_type alloc)
    const noexcept -> CString
{
    return imp_->Note(crypto, alloc);
}

auto Output::operator=(const Output& rhs) noexcept -> Output&
{
    return copy_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Output::operator=(Output&& rhs) noexcept -> Output&
{
    return move_assign_base(*this, std::move(rhs), imp_, rhs.imp_);
}

auto Output::Payee() const noexcept -> ContactID { return imp_->Payee(); }

auto Output::Payer() const noexcept -> ContactID { return imp_->Payer(); }

auto Output::Print(const api::Crypto& crypto) const noexcept
    -> UnallocatedCString
{
    return imp_->Print(crypto);
}

auto Output::Print(const api::Crypto& crypto, allocator_type alloc)
    const noexcept -> CString
{
    return imp_->Print(crypto, alloc);
}

auto Output::Script() const noexcept -> const block::Script&
{
    return imp_->Script();
}

auto Output::swap(Output& rhs) noexcept -> void
{
    pmr_swap(*this, rhs, imp_, rhs.imp_);
}

auto Output::Value() const noexcept -> Amount { return imp_->Value(); }

Output::~Output() { pmr_delete(imp_); }
}  // namespace opentxs::blockchain::bitcoin::block
