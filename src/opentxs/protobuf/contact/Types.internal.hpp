// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/container_hash/hash.hpp>

#pragma once

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <frozen/unordered_map.h>
#include <cstdint>
#include <cstring>
#include <utility>
#include <variant>

#include "internal/util/P0330.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
enum ContactItemAttribute : int;
enum ContactItemType : int;
enum ContactSectionName : int;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::contact
{
using ContactSectionVersion = std::pair<std::uint32_t, ContactSectionName>;
using EnumLang = std::pair<std::uint32_t, UnallocatedCString>;

// A map of allowed section names by ContactData version
using ContactSectionMap = boost::unordered_flat_map<
    std::uint32_t,
    boost::unordered_flat_set<ContactSectionName>>;

// A map of allowed item types by ContactSection version
using ContactItemMap = boost::unordered_flat_map<
    ContactSectionVersion,
    boost::unordered_flat_set<ContactItemType>>;

// A map of allowed item attributes by ContactItem version
using ItemAttributeMap = boost::unordered_flat_map<
    std::uint32_t,
    boost::unordered_flat_set<ContactItemAttribute>>;

// Maps for converting enum values to human-readable names
using EnumTranslation = boost::unordered_flat_map<EnumLang, UnallocatedCString>;

// A map for storing relationship reciprocities
static constexpr auto RelationshipReciprocityMapSize = 19_uz;
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
}  // namespace opentxs::protobuf::contact

// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct std::hash<opentxs::protobuf::contact::ContactSectionVersion> {
    auto operator()(const opentxs::protobuf::contact::ContactSectionVersion&)
        const noexcept -> std::size_t;
};

template <>
struct std::hash<opentxs::protobuf::contact::EnumLang> {
    auto operator()(const opentxs::protobuf::contact::EnumLang&) const noexcept
        -> std::size_t;
};
// NOLINTEND(cert-dcl58-cpp)
