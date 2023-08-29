// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/identity/wot/claim/Types.hpp"  // IWYU pragma: associated
#include "opentxs/identity/wot/claim/Types.hpp"   // IWYU pragma: associated

#include <ContactItemAttribute.pb.h>
#include <ContactSectionName.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <cstdint>
#include <functional>
#include <utility>

#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"          // IWYU pragma: keep
#include "opentxs/identity/IdentityType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"    // IWYU pragma: keep
#include "opentxs/identity/wot/claim/ClaimType.hpp"    // IWYU pragma: keep
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep

namespace opentxs::identity::wot::claim
{
constexpr auto attribute_map_ = [] {
    using enum Attribute;
    using enum proto::ContactItemAttribute;

    return frozen::make_unordered_map<Attribute, proto::ContactItemAttribute>({
#include "identity/wot/claim/conversions/attribute_to_contactitemattribute"  // IWYU pragma: keep
    });
}();

constexpr auto identitytype_map_ = [] {
    using enum identity::Type;
    using enum ClaimType;

    return frozen::make_unordered_map<identity::Type, ClaimType>({
#include "identity/wot/claim/conversions/identity_to_claim"  // IWYU pragma: keep
    });
}();

constexpr auto sectiontype_map_ = [] {
    using enum SectionType;
    using enum proto::ContactSectionName;

    return frozen::make_unordered_map<SectionType, proto::ContactSectionName>({
#include "identity/wot/claim/conversions/section_to_contactitemsection"  // IWYU pragma: keep
    });
}();

constexpr auto unittype_map_ = [] {
    using enum UnitType;

    return frozen::make_unordered_map<UnitType, ClaimType>({
#include "identity/wot/claim/conversions/unit_to_claim"  // IWYU pragma: keep
    });
}();
}  // namespace opentxs::identity::wot::claim

namespace opentxs::identity::wot::claim
{
auto ClaimToNym(const identity::wot::claim::ClaimType in) noexcept
    -> identity::Type
{
    static constexpr auto map =
        frozen::invert_unordered_map(identity::wot::claim::identitytype_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return identity::Type::invalid;
    }
}

auto ClaimToUnit(const identity::wot::claim::ClaimType in) noexcept -> UnitType
{
    static constexpr auto map =
        frozen::invert_unordered_map(identity::wot::claim::unittype_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return UnitType::Error;
    }
}
}  // namespace opentxs::identity::wot::claim

namespace opentxs::identity
{
auto NymToClaim(const identity::Type in) noexcept
    -> identity::wot::claim::ClaimType
{
    const auto& map = wot::claim::identitytype_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return wot::claim::ClaimType::Error;
    }
}
}  // namespace opentxs::identity

namespace opentxs::proto
{
auto translate(const ContactItemAttribute in) noexcept
    -> identity::wot::claim::Attribute
{
    static constexpr auto map =
        frozen::invert_unordered_map(identity::wot::claim::attribute_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return identity::wot::claim::Attribute::Error;
    }
}

auto translate(const ContactItemType in) noexcept
    -> identity::wot::claim::ClaimType
{
    return static_cast<identity::wot::claim::ClaimType>(static_cast<int>(in));
}

auto translate(const ContactSectionName in) noexcept
    -> identity::wot::claim::SectionType
{
    static constexpr auto map =
        frozen::invert_unordered_map(identity::wot::claim::sectiontype_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return identity::wot::claim::SectionType::Error;
    }
}
}  // namespace opentxs::proto

namespace opentxs::identity::wot::claim
{
auto translate(const identity::wot::claim::Attribute in) noexcept
    -> proto::ContactItemAttribute
{
    const auto& map = attribute_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return proto::CITEMATTR_ERROR;
    }
}

auto translate(const identity::wot::claim::ClaimType in) noexcept
    -> proto::ContactItemType
{
    return static_cast<proto::ContactItemType>(static_cast<std::uint32_t>(in));
}

auto translate(const identity::wot::claim::SectionType in) noexcept
    -> proto::ContactSectionName
{
    const auto& map = identity::wot::claim::sectiontype_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return proto::CONTACTSECTION_ERROR;
    }
}
}  // namespace opentxs::identity::wot::claim

namespace opentxs
{
auto UnitToClaim(const UnitType in) noexcept -> identity::wot::claim::ClaimType
{
    const auto& map = identity::wot::claim::unittype_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return identity::wot::claim::ClaimType::Error;
    }
}
}  // namespace opentxs
