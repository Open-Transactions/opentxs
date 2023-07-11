// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <variant>

#pragma once

#include <compare>
#include <cstddef>
#include <functional>

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace internal
{
class Claim;
}  // namespace internal

class Claim;
class ClaimPrivate;
}  // namespace wot
}  // namespace identity

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::identity::wot::Claim> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::identity::wot::Claim&) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::identity::wot::Claim> {
    auto operator()(
        const opentxs::identity::wot::Claim& lhs,
        const opentxs::identity::wot::Claim& rhs) const noexcept -> bool;
};
}  // namespace std

namespace opentxs::identity::wot
{
OPENTXS_EXPORT auto operator==(const Claim& lhs, const Claim& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Claim& lhs, const Claim& rhs) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Claim& lhs, Claim& rhs) noexcept -> void;
}  // namespace opentxs::identity::wot

namespace opentxs::identity::wot
{
class Claim : virtual public Allocated
{
public:
    using identifier_type = identifier::Generic;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    [[nodiscard]] auto Attributes() const noexcept
        -> UnallocatedSet<claim::Attribute>;
    [[nodiscard]] auto Attributes(alloc::Strategy alloc) const noexcept
        -> Set<claim::Attribute>;
    [[nodiscard]] auto Claimant() const noexcept -> const identifier::Nym&;
    [[nodiscard]] auto get_allocator() const noexcept -> allocator_type final;
    [[nodiscard]] auto ID() const noexcept -> const identifier_type&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Claim&;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] auto Section() const noexcept -> claim::SectionType;
    [[nodiscard]] auto Serialize(Writer&& out) const noexcept -> bool;
    [[nodiscard]] auto Start() const noexcept -> Time;
    [[nodiscard]] auto Stop() const noexcept -> Time;
    [[nodiscard]] auto Subtype() const noexcept -> ReadView;
    [[nodiscard]] auto Type() const noexcept -> claim::ClaimType;
    [[nodiscard]] auto Value() const noexcept -> ReadView;
    [[nodiscard]] auto Version() const noexcept -> VersionNumber;

    auto Add(claim::Attribute) noexcept -> void;
    [[nodiscard]] auto ChangeValue(ReadView value) noexcept -> Claim;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Claim&;
    auto Remove(claim::Attribute) noexcept -> void;
    auto swap(Claim& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Claim(ClaimPrivate* imp) noexcept;
    Claim(allocator_type alloc = {}) noexcept;
    Claim(const Claim& rhs, allocator_type alloc = {}) noexcept;
    Claim(Claim&& rhs) noexcept;
    Claim(Claim&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Claim& rhs) noexcept -> Claim&;
    auto operator=(Claim&& rhs) noexcept -> Claim&;

    ~Claim() override;

protected:
    ClaimPrivate* imp_;
};
}  // namespace opentxs::identity::wot
