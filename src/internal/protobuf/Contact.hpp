// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <cstdint>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <utility>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wdeprecated-dynamic-exception-spec"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override¶"
#include "serialization/protobuf/ContactEnums.pb.h"  // IWYU pragma: export

#pragma GCC diagnostic pop

namespace opentxs
{
namespace proto
{
using ContactSectionVersion = std::pair<uint32_t, ContactSectionName>;
using EnumLang = std::pair<uint32_t, std::string>;
}  // namespace proto
}  // namespace opentxs

namespace std
{
template <>
struct hash<opentxs::proto::ContactSectionVersion> {
    auto operator()(const opentxs::proto::ContactSectionVersion&) const noexcept
        -> size_t;
};

template <>
struct hash<opentxs::proto::EnumLang> {
    auto operator()(const opentxs::proto::EnumLang&) const noexcept -> size_t;
};

}  // namespace std

namespace opentxs
{
namespace proto
{

// A map of allowed section names by ContactData version
using ContactSectionMap =
    robin_hood::unordered_flat_map<uint32_t, std::pmr::set<ContactSectionName>>;

// A map of allowed item types by ContactSection version
using ContactItemMap = robin_hood::
    unordered_flat_map<ContactSectionVersion, std::pmr::set<ContactItemType>>;
// A map of allowed item attributes by ContactItem version
using ItemAttributeMap = robin_hood::
    unordered_flat_map<uint32_t, std::pmr::set<ContactItemAttribute>>;
// Maps for converting enum values to human-readable names
using EnumTranslation = robin_hood::unordered_flat_map<EnumLang, std::string>;
// A map for storing relationship reciprocities
using RelationshipReciprocity =
    robin_hood::unordered_flat_map<ContactItemType, ContactItemType>;

auto AllowedSectionNames() noexcept -> const ContactSectionMap&;
auto AllowedItemTypes() noexcept -> const ContactItemMap&;
auto AllowedItemAttributes() noexcept -> const ItemAttributeMap&;
auto AllowedSubtypes() noexcept -> const std::pmr::set<ContactSectionName>&;
auto ContactSectionNames() noexcept -> const EnumTranslation&;
auto ContactItemTypes() noexcept -> const EnumTranslation&;
auto ContactItemAttributes() noexcept -> const EnumTranslation&;
auto RelationshipMap() noexcept -> const RelationshipReciprocity&;
}  // namespace proto
}  // namespace opentxs
