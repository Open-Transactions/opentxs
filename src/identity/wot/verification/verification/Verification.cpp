// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/Verification.hpp"  // IWYU pragma: associated

#include <utility>

#include "identity/wot/verification/verification/VerificationPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::identity::wot
{
auto operator==(const Verification& lhs, const Verification& rhs) noexcept
    -> bool
{
    return lhs.ID() == rhs.ID();
}

auto operator<=>(const Verification& lhs, const Verification& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.ID() <=> rhs.ID();
}

auto swap(Verification& lhs, Verification& rhs) noexcept -> void
{
    lhs.swap(rhs);
}
}  // namespace opentxs::identity::wot

namespace opentxs::identity::wot
{
Verification::Verification(VerificationPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Verification::Verification(allocator_type alloc) noexcept
    : Verification(VerificationPrivate::Blank(alloc))
{
}

Verification::Verification(
    const Verification& rhs,
    allocator_type alloc) noexcept
    : Verification(rhs.imp_->clone(alloc))
{
}

Verification::Verification(Verification&& rhs) noexcept
    : Verification(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Verification::Verification(Verification&& rhs, allocator_type alloc) noexcept
    : Verification(alloc)
{
    operator=(std::move(rhs));
}

auto Verification::Claim() const noexcept -> const ClaimID&
{
    return imp_->Claim();
}

auto Verification::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Verification::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
}

auto Verification::ID() const noexcept -> const identifier_type&
{
    return imp_->ID();
}

auto Verification::Internal() const noexcept -> const internal::Verification&
{
    return *imp_;
}

auto Verification::Internal() noexcept -> internal::Verification&
{
    return *imp_;
}

auto Verification::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Verification::operator=(const Verification& rhs) noexcept -> Verification&
{
    return copy_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Verification::operator=(Verification&& rhs) noexcept -> Verification&
{
    return move_assign_base(*this, std::move(rhs), imp_, rhs.imp_);
}

auto Verification::Serialize(Writer&& out) const noexcept -> bool
{
    return imp_->Serialize(std::move(out));
}

auto Verification::Start() const noexcept -> Time { return imp_->Start(); }

auto Verification::Stop() const noexcept -> Time { return imp_->Stop(); }

auto Verification::Superscedes() const noexcept
    -> std::span<const VerificationID>
{
    return imp_->Superscedes();
}

auto Verification::swap(Verification& rhs) noexcept -> void
{
    pmr_swap(*this, rhs, imp_, rhs.imp_);
}

auto Verification::Value() const noexcept -> verification::Type
{
    return imp_->Value();
}

auto Verification::Version() const noexcept -> VersionNumber
{
    return imp_->Version();
}

Verification::~Verification() { pmr_delete(imp_); }
}  // namespace opentxs::identity::wot
