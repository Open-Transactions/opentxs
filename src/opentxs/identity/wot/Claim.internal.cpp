// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/Claim.internal.hpp"  // IWYU pragma: associated

#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/wot/Claim.hpp"

namespace opentxs::identity::wot::internal
{
Claim::Claim(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

Claim::Claim(const Claim&, allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

auto Claim::Add(claim::Attribute) noexcept -> void {}

auto Claim::Claimant() const noexcept -> const wot::Claimant&
{
    static const auto blank = wot::Claimant{identifier::Nym{}};

    return blank;
}

auto Claim::CreateModified(
    std::optional<std::string_view>,
    std::optional<ReadView>,
    std::optional<Time>,
    std::optional<Time>,
    allocator_type alloc) const noexcept -> wot::Claim
{
    return Blank(alloc);
}

auto Claim::for_each_attribute(
    std::function<void(claim::Attribute)>) const noexcept -> void
{
}

auto Claim::HasAttribute(claim::Attribute) const noexcept -> bool
{
    return false;
}

auto Claim::ID() const noexcept -> const wot::Claim::identifier_type&
{
    static const auto blank = wot::Claim::identifier_type{};

    return blank;
}

auto Claim::IsValid() const noexcept -> bool { return false; }

auto Claim::Remove(claim::Attribute) noexcept -> void {}

auto Claim::Section() const noexcept -> claim::SectionType { return {}; }

auto Claim::Serialize(protobuf::Claim&) const noexcept -> void {}

auto Claim::Serialize(Writer&&) const noexcept -> bool { return false; }

auto Claim::SetVersion(VersionNumber) noexcept -> void {}

auto Claim::Start() const noexcept -> Time { return {}; }

auto Claim::Stop() const noexcept -> Time { return {}; }

auto Claim::Subtype() const noexcept -> ReadView { return {}; }

auto Claim::Type() const noexcept -> claim::ClaimType { return {}; }

auto Claim::Value() const noexcept -> ReadView { return {}; }

auto Claim::Version() const noexcept -> VersionNumber { return {}; }
}  // namespace opentxs::identity::wot::internal
