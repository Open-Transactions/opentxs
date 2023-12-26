// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <optional>
#include <string_view>

#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace claim
{
namespace internal
{
class Item;  // IWYU pragma: keep
}  // namespace internal
}  // namespace claim

class Claim;
}  // namespace wot
}  // namespace identity

namespace proto
{
class ContactItem;
}  // namespace proto

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_NO_EXPORT opentxs::identity::wot::claim::internal::Item
    : public opentxs::pmr::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept -> Item*
    {
        return pmr::default_construct<Item>(alloc::PMR<Item>{alloc});
    }

    virtual auto asClaim() const noexcept -> const Claim&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> Item*
    {
        return pmr::clone(this, alloc::PMR<Item>{alloc});
    }
    virtual auto CreateModified(
        std::optional<std::string_view> value,
        std::optional<ReadView> subtype,
        std::optional<Time> start,
        std::optional<Time> end,
        allocator_type) const noexcept -> Item*;
    virtual auto End() const noexcept -> Time;
    virtual auto for_each_attribute(
        std::function<void(claim::Attribute)>) const noexcept -> void;
    virtual auto HasAttribute(claim::Attribute) const noexcept -> bool;
    virtual auto ID() const noexcept -> const identifier::Generic&;
    virtual auto IsValid() const noexcept -> bool { return false; }
    virtual auto Section() const noexcept -> claim::SectionType;
    virtual auto Serialize(Writer&& destination, bool withID) const noexcept
        -> bool;
    virtual auto Serialize(proto::ContactItem& out, bool withID = false)
        const noexcept -> bool;
    virtual auto Start() const noexcept -> Time;
    virtual auto Subtype() const noexcept -> ReadView;
    virtual auto Type() const noexcept -> claim::ClaimType;
    virtual auto Value() const noexcept -> std::string_view;
    virtual auto Version() const noexcept -> VersionNumber;

    virtual auto Add(claim::Attribute) noexcept -> void;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }
    virtual auto Remove(claim::Attribute) noexcept -> void;
    virtual auto SetVersion(VersionNumber value) noexcept -> void;

    Item(allocator_type alloc) noexcept;
    Item() = delete;
    Item(const Item& rhs, allocator_type alloc) noexcept;
    Item(const Item&) = delete;
    Item(Item&&) = delete;
    auto operator=(const Item&) -> Item& = delete;
    auto operator=(Item&&) -> Item& = delete;

    ~Item() override = default;
};
