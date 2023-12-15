// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <string_view>
#include <variant>

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
namespace claim
{
namespace internal
{
class Item;
}  // namespace internal

class Item;
}  // namespace claim

class Claim;
}  // namespace wot
}  // namespace identity

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

template <>
struct OPENTXS_EXPORT std::hash<opentxs::identity::wot::claim::Item> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::identity::wot::claim::Item&) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT std::less<opentxs::identity::wot::claim::Item> {
    auto operator()(
        const opentxs::identity::wot::claim::Item& lhs,
        const opentxs::identity::wot::claim::Item& rhs) const noexcept -> bool;
};

namespace opentxs::identity::wot::claim
{
OPENTXS_EXPORT auto operator==(const Item& lhs, const Item& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Item& lhs, const Item& rhs) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Item& lhs, Item& rhs) noexcept -> void;
}  // namespace opentxs::identity::wot::claim

class OPENTXS_EXPORT opentxs::identity::wot::claim::Item
    : virtual public Allocated
{
public:
    using identifier_type = ClaimID;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    [[nodiscard]] auto asClaim() const noexcept -> const Claim&;
    [[nodiscard]] auto End() const noexcept -> Time;
    auto for_each_attribute(
        std::function<void(claim::Attribute)>) const noexcept -> void;
    [[nodiscard]] auto get_allocator() const noexcept -> allocator_type final;
    [[nodiscard]] auto HasAttribute(claim::Attribute) const noexcept -> bool;
    [[nodiscard]] auto ID() const noexcept -> const identifier_type&;
    [[nodiscard]] OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Item&;
    [[nodiscard]] auto IsValid() const noexcept -> bool;
    [[nodiscard]] auto Section() const noexcept -> claim::SectionType;
    [[nodiscard]] auto Serialize(Writer&& destination, bool withID = false)
        const noexcept -> bool;
    [[nodiscard]] auto Start() const noexcept -> Time;
    [[nodiscard]] auto Subtype() const noexcept -> ReadView;
    [[nodiscard]] auto Type() const noexcept -> claim::ClaimType;
    [[nodiscard]] auto Value() const noexcept -> std::string_view;
    [[nodiscard]] auto Version() const noexcept -> VersionNumber;

    auto Add(claim::Attribute) noexcept -> void;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function final;
    [[nodiscard]] OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Item&;
    auto Remove(claim::Attribute) noexcept -> void;
    auto SetVersion(VersionNumber) noexcept -> void;
    auto swap(Item& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Item(internal::Item* imp) noexcept;
    Item(allocator_type alloc = {}) noexcept;
    Item(const Item& rhs, allocator_type alloc = {}) noexcept;
    Item(Item&& rhs) noexcept;
    Item(Item&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Item& rhs) noexcept -> Item&;
    auto operator=(Item&& rhs) noexcept -> Item&;

    ~Item() override;

protected:
    internal::Item* imp_;
};
