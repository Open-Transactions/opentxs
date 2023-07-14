// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/identity/wot/Claim.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot
{
class ClaimPrivate : virtual public internal::Claim,
                     public opentxs::implementation::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> ClaimPrivate*
    {
        return default_construct<ClaimPrivate>(alloc::PMR<ClaimPrivate>{alloc});
    }

    [[nodiscard]] virtual auto Attributes() const noexcept
        -> UnallocatedSet<claim::Attribute>;
    [[nodiscard]] virtual auto Attributes(alloc::Strategy alloc) const noexcept
        -> Set<claim::Attribute>;
    [[nodiscard]] virtual auto Claimant() const noexcept
        -> const identifier::Nym&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> ClaimPrivate*
    {
        return pmr::clone(this, alloc::PMR<ClaimPrivate>{alloc});
    }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return make_deleter(this);
    }
    [[nodiscard]] virtual auto ID() const noexcept
        -> const wot::Claim::identifier_type&;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] virtual auto Section() const noexcept -> claim::SectionType;
    using internal::Claim::Serialize;
    [[nodiscard]] virtual auto Serialize(Writer&& out) const noexcept -> bool;
    [[nodiscard]] virtual auto Start() const noexcept -> Time;
    [[nodiscard]] virtual auto Stop() const noexcept -> Time;
    [[nodiscard]] virtual auto Subtype() const noexcept -> ReadView;
    [[nodiscard]] virtual auto Type() const noexcept -> claim::ClaimType;
    [[nodiscard]] virtual auto Value() const noexcept -> ReadView;
    [[nodiscard]] virtual auto Version() const noexcept -> VersionNumber;

    virtual auto Add(claim::Attribute) noexcept -> void;
    [[nodiscard]] virtual auto ChangeValue(ReadView value) noexcept
        -> wot::Claim;
    virtual auto Remove(claim::Attribute) noexcept -> void;

    ClaimPrivate(allocator_type alloc) noexcept;
    ClaimPrivate() = delete;
    ClaimPrivate(const ClaimPrivate& rhs, allocator_type alloc) noexcept;
    ClaimPrivate(const ClaimPrivate&) = delete;
    ClaimPrivate(ClaimPrivate&&) = delete;
    auto operator=(const ClaimPrivate&) -> ClaimPrivate& = delete;
    auto operator=(ClaimPrivate&&) -> ClaimPrivate& = delete;

    ~ClaimPrivate() override = default;
};
}  // namespace opentxs::identity::wot
