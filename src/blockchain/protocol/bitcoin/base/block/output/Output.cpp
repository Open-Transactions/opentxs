// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/protocol/bitcoin/base/block/output/OutputPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
Output::Output(OutputPrivate* imp) noexcept
    : imp_(std::move(imp))
{
    assert_false(nullptr == imp_);
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
    : Output(std::exchange(rhs.imp_, nullptr))
{
}

Output::Output(Output&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto Output::Blank() noexcept -> Output&
{
    static auto blank = Output{};

    return blank;
}

auto Output::Cashtoken() const noexcept
    -> const bitcoincash::token::cashtoken::View*
{
    return imp_->Cashtoken();
}

auto Output::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Output::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
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
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto Output::operator=(Output&& rhs) noexcept -> Output&
{
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
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

auto Output::swap(Output& rhs) noexcept -> void { pmr::swap(imp_, rhs.imp_); }

auto Output::Value() const noexcept -> Amount { return imp_->Value(); }

Output::~Output() { pmr::destroy(imp_); }
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
