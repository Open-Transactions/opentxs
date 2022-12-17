// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/bitcoin/block/Input.hpp"  // IWYU pragma: associated

#include <functional>
#include <utility>

#include "blockchain/bitcoin/block/input/InputPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::bitcoin::block
{
Input::Input(InputPrivate* imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(nullptr != imp_);
}

Input::Input(allocator_type alloc) noexcept
    : Input(InputPrivate::Blank(alloc))
{
}

Input::Input(const Input& rhs, allocator_type alloc) noexcept
    : Input(rhs.imp_->clone(alloc))
{
}

Input::Input(Input&& rhs) noexcept
    : Input(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Input::Input(Input&& rhs, allocator_type alloc) noexcept
    : Input(alloc)
{
    operator=(std::move(rhs));
}

auto Input::Blank() noexcept -> Input&
{
    static auto blank = Input{};

    return blank;
}

auto Input::Coinbase() const noexcept -> ReadView { return imp_->Coinbase(); }

auto Input::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Input::Internal() const noexcept -> const internal::Input&
{
    return *imp_;
}

auto Input::Internal() noexcept -> internal::Input& { return *imp_; }

auto Input::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Input::Keys(Set<crypto::Key>& out) const noexcept -> void
{
    return imp_->Keys(out);
}

auto Input::Keys(allocator_type alloc) const noexcept -> Set<crypto::Key>
{
    return imp_->Keys(alloc);
}

auto Input::operator=(const Input& rhs) noexcept -> Input&
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

auto Input::operator=(Input&& rhs) noexcept -> Input&
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

auto Input::PreviousOutput() const noexcept
    -> const blockchain::block::Outpoint&
{
    return imp_->PreviousOutput();
}

auto Input::Print() const noexcept -> UnallocatedCString
{
    return imp_->Print();
}

auto Input::Print(allocator_type alloc) const noexcept -> CString
{
    return imp_->Print(alloc);
}

auto Input::Script() const noexcept -> const block::Script&
{
    return imp_->Script();
}

auto Input::Sequence() const noexcept -> std::uint32_t
{
    return imp_->Sequence();
}

auto Input::swap(Input& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

auto Input::Witness() const noexcept -> std::span<const WitnessItem>
{
    return imp_->Witness();
}

Input::~Input()
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
