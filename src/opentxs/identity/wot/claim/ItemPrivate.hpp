// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/identifier/Generic.hpp"

#pragma once

#include <functional>
#include <optional>
#include <string_view>

#include "internal/util/PMR.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/claim/Item.internal.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;  // IWYU pragma: keep
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace claim
{
class ItemPrivate;  // IWYU pragma: keep
}  // namespace claim
}  // namespace wot
}  // namespace identity

namespace proto
{
class ContactItem;
}  // namespace proto

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_NO_EXPORT opentxs::identity::wot::claim::ItemPrivate final
    : public internal::Item
{
public:
    auto asClaim() const noexcept -> const Claim& final { return claim_; }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> ItemPrivate* final
    {
        return pmr::clone(this, alloc::PMR<ItemPrivate>{alloc});
    }
    auto CreateModified(
        std::optional<std::string_view> value,
        std::optional<ReadView> subtype,
        std::optional<Time> start,
        std::optional<Time> end,
        allocator_type alloc) const noexcept -> ItemPrivate* final;
    auto End() const noexcept -> Time final;
    auto for_each_attribute(
        std::function<void(claim::Attribute)>) const noexcept -> void final;
    auto HasAttribute(claim::Attribute) const noexcept -> bool final;
    auto ID() const noexcept -> const identifier::Generic& final;
    auto IsValid() const noexcept -> bool final { return true; }
    auto Section() const noexcept -> claim::SectionType final;
    auto Serialize(Writer&& destination, bool withID) const noexcept
        -> bool final;
    auto Serialize(proto::ContactItem& out, bool withID = false) const noexcept
        -> bool final;
    auto Start() const noexcept -> Time final;
    auto Subtype() const noexcept -> ReadView final;
    auto Type() const noexcept -> claim::ClaimType final;
    auto Value() const noexcept -> std::string_view final;
    auto Version() const noexcept -> VersionNumber final;

    auto Add(claim::Attribute) noexcept -> void final;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }
    auto Remove(claim::Attribute) noexcept -> void final;
    auto SetVersion(VersionNumber value) noexcept -> void final;

    ItemPrivate(Claim claim, allocator_type alloc) noexcept;
    ItemPrivate() = delete;
    ItemPrivate(const ItemPrivate& rhs, allocator_type alloc = {}) noexcept;
    ItemPrivate(ItemPrivate&&) = delete;
    auto operator=(const ItemPrivate&) -> ItemPrivate& = delete;
    auto operator=(ItemPrivate&&) -> ItemPrivate& = delete;

    ~ItemPrivate() final;

private:
    Claim claim_;
};
