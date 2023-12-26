// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/claim/Item.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Item.internal.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::identity::wot::claim
{
auto operator==(const Item& lhs, const Item& rhs) noexcept -> bool
{
    return lhs.ID() == rhs.ID();
}

auto operator<=>(const Item& lhs, const Item& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.ID() <=> rhs.ID();
}

auto swap(Item& lhs, Item& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::identity::wot::claim

namespace opentxs::identity::wot::claim
{
Item::Item(internal::Item* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

Item::Item(allocator_type alloc) noexcept
    : Item(internal::Item::Blank(alloc))
{
}

Item::Item(const Item& rhs, allocator_type alloc) noexcept
    : Item(rhs.imp_->clone(alloc))
{
}

Item::Item(Item&& rhs) noexcept
    : Item(std::exchange(rhs.imp_, nullptr))
{
}

Item::Item(Item&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto Item::Add(claim::Attribute attr) noexcept -> void { imp_->Add(attr); }

auto Item::asClaim() const noexcept -> const Claim& { return imp_->asClaim(); }

auto Item::End() const noexcept -> Time { return imp_->End(); }

auto Item::for_each_attribute(
    std::function<void(claim::Attribute)> f) const noexcept -> void
{
    return imp_->for_each_attribute(std::move(f));
}

auto Item::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Item::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
}

auto Item::HasAttribute(claim::Attribute value) const noexcept -> bool
{
    return imp_->HasAttribute(value);
}

auto Item::ID() const noexcept -> const identifier_type& { return imp_->ID(); }

auto Item::Internal() const noexcept -> const internal::Item& { return *imp_; }

auto Item::Internal() noexcept -> internal::Item& { return *imp_; }

auto Item::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Item::operator=(const Item& rhs) noexcept -> Item&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto Item::operator=(Item&& rhs) noexcept -> Item&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Item::Remove(claim::Attribute attr) noexcept -> void
{
    return imp_->Remove(attr);
}

auto Item::Section() const noexcept -> claim::SectionType
{
    return imp_->Section();
}

auto Item::Serialize(Writer&& out, bool withID) const noexcept -> bool
{
    return imp_->Serialize(std::move(out), withID);
}

auto Item::SetVersion(VersionNumber value) noexcept -> void
{
    imp_->SetVersion(value);
}

auto Item::Start() const noexcept -> Time { return imp_->Start(); }

auto Item::Subtype() const noexcept -> ReadView { return imp_->Subtype(); }

auto Item::swap(Item& rhs) noexcept -> void { pmr::swap(imp_, rhs.imp_); }

auto Item::Type() const noexcept -> claim::ClaimType { return imp_->Type(); }

auto Item::Value() const noexcept -> ReadView { return imp_->Value(); }

auto Item::Version() const noexcept -> VersionNumber { return imp_->Version(); }

Item::~Item() { pmr::destroy(imp_); }
}  // namespace opentxs::identity::wot::claim
