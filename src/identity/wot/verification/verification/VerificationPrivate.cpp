// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/wot/verification/verification/VerificationPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs::identity::wot
{
VerificationPrivate::VerificationPrivate(allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

VerificationPrivate::VerificationPrivate(
    const VerificationPrivate&,
    allocator_type alloc) noexcept
    : Allocated(alloc)
{
}

auto VerificationPrivate::Claim() const noexcept -> const ClaimID&
{
    static const auto blank = ClaimID{};

    return blank;
}

auto VerificationPrivate::ID() const noexcept -> const VerificationID&
{
    static const auto blank = VerificationID{};

    return blank;
}

auto VerificationPrivate::IsValid() const noexcept -> bool { return false; }

auto VerificationPrivate::Serialize(Writer&&) const noexcept -> bool
{
    return false;
}

auto VerificationPrivate::Start() const noexcept -> Time { return {}; }

auto VerificationPrivate::Stop() const noexcept -> Time { return {}; }

auto VerificationPrivate::Superscedes() const noexcept
    -> std::span<const VerificationID>
{
    return {};
}

auto VerificationPrivate::Value() const noexcept -> verification::Type
{
    return {};
}

auto VerificationPrivate::Version() const noexcept -> VersionNumber
{
    return {};
}
}  // namespace opentxs::identity::wot
