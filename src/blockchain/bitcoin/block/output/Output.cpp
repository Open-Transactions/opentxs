// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::script::Pattern
// IWYU pragma: no_forward_declare opentxs::blockchain::bitcoin::block::script::Position

#include "opentxs/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: associated

#include <functional>
#include <utility>

#include "blockchain/bitcoin/block/output/OutputPrivate.hpp"
#include "internal/util/LogMacros.hpp"
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
    : Output(alloc)
{
    operator=(std::move(rhs));
}

auto Output::Blank() noexcept -> Output&
{
    static auto blank = Output{};

    return blank;
}

auto Output::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
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

auto Output::operator=(Output&& rhs) noexcept -> Output&
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

auto Output::Payee() const noexcept -> ContactID { return imp_->Payee(); }

auto Output::Payer() const noexcept -> ContactID { return imp_->Payer(); }

auto Output::Print() const noexcept -> UnallocatedCString
{
    return imp_->Print();
}

auto Output::Print(allocator_type alloc) const noexcept -> CString
{
    return imp_->Print(alloc);
}

auto Output::Script() const noexcept -> const block::Script&
{
    return imp_->Script();
}

auto Output::swap(Output& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

auto Output::Value() const noexcept -> Amount { return imp_->Value(); }

Output::~Output()
{
    if (nullptr != imp_) {
        // TODO switch to destroying delete after resolution of
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107352
        auto deleter = imp_->get_deleter();
        std::invoke(deleter);
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::bitcoin::block
