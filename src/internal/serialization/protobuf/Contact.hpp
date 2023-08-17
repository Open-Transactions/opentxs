// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ankerl/unordered_dense.h>
#include <frozen/unordered_map.h>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <utility>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
enum ContactItemAttribute : int;
enum ContactItemType : int;
enum ContactSectionName : int;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::proto
{
using ContactSectionVersion = std::pair<uint32_t, ContactSectionName>;
using EnumLang = std::pair<uint32_t, UnallocatedCString>;
}  // namespace opentxs::proto

namespace std
{
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct hash<opentxs::proto::ContactSectionVersion> {
    auto operator()(const opentxs::proto::ContactSectionVersion&) const noexcept
        -> std::size_t;
};

template <>
struct hash<opentxs::proto::EnumLang> {
    auto operator()(const opentxs::proto::EnumLang&) const noexcept
        -> std::size_t;
};
// NOLINTEND(cert-dcl58-cpp)
}  // namespace std

namespace opentxs::proto
{
// A map of allowed section names by ContactData version
using ContactSectionMap = ankerl::unordered_dense::
    map<uint32_t, ankerl::unordered_dense::set<ContactSectionName>>;

// A map of allowed item types by ContactSection version
using ContactItemMap = ankerl::unordered_dense::
    map<ContactSectionVersion, ankerl::unordered_dense::set<ContactItemType>>;
// A map of allowed item attributes by ContactItem version
using ItemAttributeMap = ankerl::unordered_dense::
    map<uint32_t, ankerl::unordered_dense::set<ContactItemAttribute>>;
// Maps for converting enum values to human-readable names
using EnumTranslation =
    ankerl::unordered_dense::map<EnumLang, UnallocatedCString>;
// A map for storing relationship reciprocities
static constexpr auto RelationshipReciprocityMapSize = std::size_t{19};
using RelationshipReciprocity = frozen::unordered_map<
    ContactItemType,
    ContactItemType,
    RelationshipReciprocityMapSize>;

auto AllowedSectionNames() noexcept -> const ContactSectionMap&;
auto AllowedItemTypes() noexcept -> const ContactItemMap&;
auto AllowedItemAttributes() noexcept -> const ItemAttributeMap&;
auto AllowedSubtypes() noexcept -> const UnallocatedSet<ContactSectionName>&;
auto ContactSectionNames() noexcept -> const EnumTranslation&;
auto ContactItemTypes() noexcept -> const EnumTranslation&;
auto ContactItemAttributes() noexcept -> const EnumTranslation&;
auto RelationshipMap() noexcept -> const RelationshipReciprocity&;
}  // namespace opentxs::proto
