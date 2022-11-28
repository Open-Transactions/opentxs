// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ContactEnums.pb.h>  // IWYU pragma: export
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <robin_hood.h>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <utility>

#include "opentxs/util/Container.hpp"

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
using ContactSectionMap = robin_hood::
    unordered_flat_map<uint32_t, UnallocatedSet<ContactSectionName>>;

// A map of allowed item types by ContactSection version
using ContactItemMap = robin_hood::
    unordered_flat_map<ContactSectionVersion, UnallocatedSet<ContactItemType>>;
// A map of allowed item attributes by ContactItem version
using ItemAttributeMap = robin_hood::
    unordered_flat_map<uint32_t, UnallocatedSet<ContactItemAttribute>>;
// Maps for converting enum values to human-readable names
using EnumTranslation =
    robin_hood::unordered_flat_map<EnumLang, UnallocatedCString>;
// A map for storing relationship reciprocities
using RelationshipReciprocity =
    frozen::unordered_map<ContactItemType, ContactItemType, 19>;

auto AllowedSectionNames() noexcept -> const ContactSectionMap&;
auto AllowedItemTypes() noexcept -> const ContactItemMap&;
auto AllowedItemAttributes() noexcept -> const ItemAttributeMap&;
auto AllowedSubtypes() noexcept -> const UnallocatedSet<ContactSectionName>&;
auto ContactSectionNames() noexcept -> const EnumTranslation&;
auto ContactItemTypes() noexcept -> const EnumTranslation&;
auto ContactItemAttributes() noexcept -> const EnumTranslation&;
auto RelationshipMap() noexcept -> const RelationshipReciprocity&;
}  // namespace opentxs::proto
