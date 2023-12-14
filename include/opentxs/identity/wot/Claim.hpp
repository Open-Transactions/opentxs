// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <variant>

#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
namespace wot
{
namespace internal
{
class Claim;
}  // namespace internal

class Claim;
}  // namespace wot
}  // namespace identity

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

template <>
struct OPENTXS_EXPORT std::hash<opentxs::identity::wot::Claim> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::identity::wot::Claim&) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT std::less<opentxs::identity::wot::Claim> {
    auto operator()(
        const opentxs::identity::wot::Claim& lhs,
        const opentxs::identity::wot::Claim& rhs) const noexcept -> bool;
};

namespace opentxs::identity::wot
{
OPENTXS_EXPORT auto operator==(const Claim& lhs, const Claim& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Claim& lhs, const Claim& rhs) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Claim& lhs, Claim& rhs) noexcept -> void;
}  // namespace opentxs::identity::wot

class OPENTXS_EXPORT opentxs::identity::wot::Claim : virtual public Allocated
{
public:
    using identifier_type = ClaimID;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    [[nodiscard]] auto Claimant() const noexcept -> const wot::Claimant&;
    [[nodiscard]] auto CreateModified(
        std::optional<std::string_view> value = std::nullopt,
        std::optional<ReadView> subtype = std::nullopt,
        std::optional<Time> start = std::nullopt,
        std::optional<Time> end = std::nullopt,
        allocator_type = {}) const noexcept -> Claim;
    auto for_each_attribute(
        std::function<void(claim::Attribute)>) const noexcept -> void;
    [[nodiscard]] auto get_allocator() const noexcept -> allocator_type final;
    [[nodiscard]] auto HasAttribute(claim::Attribute) const noexcept -> bool;
    [[nodiscard]] auto ID() const noexcept -> const identifier_type&;
    [[nodiscard]] OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Claim&;
    [[nodiscard]] auto IsValid() const noexcept -> bool;
    [[nodiscard]] auto Section() const noexcept -> claim::SectionType;
    [[nodiscard]] auto Serialize(Writer&& out) const noexcept -> bool;
    [[nodiscard]] auto Start() const noexcept -> Time;
    [[nodiscard]] auto Stop() const noexcept -> Time;
    [[nodiscard]] auto Subtype() const noexcept -> ReadView;
    [[nodiscard]] auto Type() const noexcept -> claim::ClaimType;
    [[nodiscard]] auto Value() const noexcept -> std::string_view;
    [[nodiscard]] auto Version() const noexcept -> VersionNumber;

    auto Add(claim::Attribute) noexcept -> void;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function final;
    [[nodiscard]] OPENTXS_NO_EXPORT auto Internal() noexcept
        -> internal::Claim&;
    auto Remove(claim::Attribute) noexcept -> void;
    auto SetVersion(VersionNumber) noexcept -> void;
    auto swap(Claim& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Claim(internal::Claim* imp) noexcept;
    Claim(allocator_type alloc = {}) noexcept;
    Claim(const Claim& rhs, allocator_type alloc = {}) noexcept;
    Claim(Claim&& rhs) noexcept;
    Claim(Claim&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Claim& rhs) noexcept -> Claim&;
    auto operator=(Claim&& rhs) noexcept -> Claim&;

    ~Claim() override;

protected:
    internal::Claim* imp_;
};
