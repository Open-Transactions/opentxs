// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/Claim.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/Claim.internal.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::identity::wot
{
auto operator==(const Claim& lhs, const Claim& rhs) noexcept -> bool
{
    return lhs.ID() == rhs.ID();
}

auto operator<=>(const Claim& lhs, const Claim& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.ID() <=> rhs.ID();
}

auto swap(Claim& lhs, Claim& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::identity::wot

namespace opentxs::identity::wot
{
Claim::Claim(internal::Claim* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

Claim::Claim(allocator_type alloc) noexcept
    : Claim(internal::Claim::Blank(alloc))
{
}

Claim::Claim(const Claim& rhs, allocator_type alloc) noexcept
    : Claim(rhs.imp_->clone(alloc))
{
}

Claim::Claim(Claim&& rhs) noexcept
    : Claim(std::exchange(rhs.imp_, nullptr))
{
}

Claim::Claim(Claim&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto Claim::Add(claim::Attribute attr) noexcept -> void { imp_->Add(attr); }

auto Claim::Claimant() const noexcept -> const identity::wot::Claimant&
{
    return imp_->Claimant();
}

auto Claim::CreateModified(
    std::optional<std::string_view> value,
    std::optional<ReadView> subtype,
    std::optional<Time> start,
    std::optional<Time> end,
    allocator_type alloc) const noexcept -> wot::Claim
{
    return imp_->CreateModified(value, subtype, start, end, alloc);
}

auto Claim::for_each_attribute(
    std::function<void(claim::Attribute)> f) const noexcept -> void
{
    return imp_->for_each_attribute(std::move(f));
}

auto Claim::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Claim::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
}

auto Claim::HasAttribute(claim::Attribute value) const noexcept -> bool
{
    return imp_->HasAttribute(value);
}

auto Claim::ID() const noexcept -> const identifier_type& { return imp_->ID(); }

auto Claim::Internal() const noexcept -> const internal::Claim&
{
    return *imp_;
}

auto Claim::Internal() noexcept -> internal::Claim& { return *imp_; }

auto Claim::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Claim::operator=(const Claim& rhs) noexcept -> Claim&
{
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto Claim::operator=(Claim&& rhs) noexcept -> Claim&
{
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Claim::Remove(claim::Attribute attr) noexcept -> void
{
    return imp_->Remove(attr);
}

auto Claim::Section() const noexcept -> claim::SectionType
{
    return imp_->Section();
}

auto Claim::Serialize(Writer&& out) const noexcept -> bool
{
    return imp_->Serialize(std::move(out));
}

auto Claim::SetVersion(VersionNumber value) noexcept -> void
{
    imp_->SetVersion(value);
}

auto Claim::Start() const noexcept -> Time { return imp_->Start(); }

auto Claim::Stop() const noexcept -> Time { return imp_->Stop(); }

auto Claim::Subtype() const noexcept -> ReadView { return imp_->Subtype(); }

auto Claim::swap(Claim& rhs) noexcept -> void { pmr::swap(imp_, rhs.imp_); }

auto Claim::Type() const noexcept -> claim::ClaimType { return imp_->Type(); }

auto Claim::Value() const noexcept -> ReadView { return imp_->Value(); }

auto Claim::Version() const noexcept -> VersionNumber
{
    return imp_->Version();
}

Claim::~Claim() { pmr::destroy(imp_); }
}  // namespace opentxs::identity::wot
