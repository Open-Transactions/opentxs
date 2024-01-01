// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/claim/Item.internal.hpp"  // IWYU pragma: associated

#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/wot/Claim.hpp"

namespace opentxs::identity::wot::claim::internal
{
Item::Item(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

Item::Item(const Item&, allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

auto Item::Add(claim::Attribute) noexcept -> void {}

auto Item::asClaim() const noexcept -> const Claim&
{
    static const auto blank = Claim{};

    return blank;
}

auto Item::CreateModified(
    std::optional<std::string_view>,
    std::optional<ReadView>,
    std::optional<Time>,
    std::optional<Time>,
    alloc::Default alloc) const noexcept -> Item*
{
    return Blank(alloc);
}

auto Item::End() const noexcept -> Time { return {}; }

auto Item::for_each_attribute(
    std::function<void(claim::Attribute)>) const noexcept -> void
{
}

auto Item::HasAttribute(claim::Attribute) const noexcept -> bool
{
    return false;
}

auto Item::ID() const noexcept -> const identifier::Generic&
{
    static const auto blank = identifier::Generic{};

    return blank;
}

auto Item::Remove(claim::Attribute) noexcept -> void {}

auto Item::Section() const noexcept -> claim::SectionType { return {}; }

auto Item::Serialize(Writer&&, bool) const noexcept -> bool { return {}; }

auto Item::Serialize(protobuf::ContactItem&, bool) const noexcept -> bool
{
    return {};
}

auto Item::SetVersion(VersionNumber) noexcept -> void {}

auto Item::Start() const noexcept -> Time { return {}; }

auto Item::Subtype() const noexcept -> ReadView { return {}; }

auto Item::Type() const noexcept -> claim::ClaimType { return {}; }

auto Item::Value() const noexcept -> std::string_view { return {}; }

auto Item::Version() const noexcept -> VersionNumber { return {}; }
}  // namespace opentxs::identity::wot::claim::internal
