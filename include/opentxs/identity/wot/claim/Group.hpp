// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/identity/wot/claim/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/identity/wot/claim/SectionType.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iosfwd>
#include <memory>

#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identity
{
namespace wot
{
namespace claim
{
class Item;
}  // namespace claim
}  // namespace wot
}  // namespace identity

namespace proto
{
class ContactSection;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::claim
{
class OPENTXS_EXPORT Group
{
public:
    using ItemMap = UnallocatedMap<OTIdentifier, std::shared_ptr<claim::Item>>;

    Group(
        const UnallocatedCString& nym,
        const claim::SectionType section,
        const claim::ClaimType type,
        const ItemMap& items);
    Group(
        const UnallocatedCString& nym,
        const claim::SectionType section,
        const std::shared_ptr<Item>& item);
    Group(const Group&) noexcept;
    Group(Group&&) noexcept;

    auto operator+(const Group& rhs) const -> Group;

    auto begin() const -> ItemMap::const_iterator;
    auto Best() const -> std::shared_ptr<Item>;
    auto Claim(const Identifier& item) const -> std::shared_ptr<Item>;
    auto HaveClaim(const Identifier& item) const -> bool;
    auto AddItem(const std::shared_ptr<Item>& item) const -> Group;
    auto AddPrimary(const std::shared_ptr<Item>& item) const -> Group;
    auto Delete(const Identifier& id) const -> Group;
    auto end() const -> ItemMap::const_iterator;
    auto Primary() const -> const Identifier&;
    auto PrimaryClaim() const -> std::shared_ptr<Item>;
    auto SerializeTo(proto::ContactSection& section, const bool withIDs = false)
        const -> bool;
    auto Size() const -> std::size_t;
    auto Type() const -> const claim::ClaimType&;

    ~Group();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Group() = delete;
    auto operator=(const Group&) -> Group& = delete;
    auto operator=(Group&&) -> Group& = delete;
};
}  // namespace opentxs::identity::wot::claim
