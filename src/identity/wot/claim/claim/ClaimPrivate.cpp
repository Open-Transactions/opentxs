// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/wot/claim/claim/ClaimPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/Claim.hpp"

namespace opentxs::identity::wot
{
ClaimPrivate::ClaimPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

ClaimPrivate::ClaimPrivate(const ClaimPrivate&, allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

auto ClaimPrivate::Add(claim::Attribute) noexcept -> void {}

auto ClaimPrivate::Attributes() const noexcept
    -> UnallocatedSet<claim::Attribute>
{
    return {};
}

auto ClaimPrivate::Attributes(alloc::Strategy alloc) const noexcept
    -> Set<claim::Attribute>
{
    return Set<claim::Attribute>{alloc.result_};
}

auto ClaimPrivate::ChangeValue(ReadView value) noexcept -> wot::Claim
{
    return wot::Claim{get_allocator()};
}

auto ClaimPrivate::Claimant() const noexcept -> const identifier::Nym&
{
    static const auto blank = identifier::Nym{};

    return blank;
}

auto ClaimPrivate::ID() const noexcept -> const wot::Claim::identifier_type&
{
    static const auto blank = wot::Claim::identifier_type{};

    return blank;
}

auto ClaimPrivate::IsValid() const noexcept -> bool { return false; }

auto ClaimPrivate::Remove(claim::Attribute) noexcept -> void {}

auto ClaimPrivate::Section() const noexcept -> claim::SectionType { return {}; }

auto ClaimPrivate::Serialize(Writer&&) const noexcept -> bool { return false; }

auto ClaimPrivate::Start() const noexcept -> Time { return {}; }

auto ClaimPrivate::Stop() const noexcept -> Time { return {}; }

auto ClaimPrivate::Subtype() const noexcept -> ReadView { return {}; }

auto ClaimPrivate::Type() const noexcept -> claim::ClaimType { return {}; }

auto ClaimPrivate::Value() const noexcept -> ReadView { return {}; }

auto ClaimPrivate::Version() const noexcept -> VersionNumber { return {}; }
}  // namespace opentxs::identity::wot
