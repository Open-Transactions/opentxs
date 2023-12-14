// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/claim/Types.internal.hpp"  // IWYU pragma: associated

#include <ContactItemAttribute.pb.h>
#include <ContactSectionName.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <cstdint>
#include <functional>
#include <utility>

#include "opentxs/identity/wot/claim/Attribute.hpp"    // IWYU pragma: keep
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep

namespace opentxs::identity::wot::claim
{
constexpr auto attribute_map_ = [] {
    using enum Attribute;
    using enum proto::ContactItemAttribute;

    return frozen::make_unordered_map<Attribute, proto::ContactItemAttribute>({
#include "opentxs/identity/wot/claim/attribute_to_contactitemattribute.inc"  // IWYU pragma: keep
    });
}();

constexpr auto sectiontype_map_ = [] {
    using enum SectionType;
    using enum proto::ContactSectionName;

    return frozen::make_unordered_map<SectionType, proto::ContactSectionName>({
#include "opentxs/identity/wot/claim/section_to_contactitemsection.inc"  // IWYU pragma: keep
    });
}();
}  // namespace opentxs::identity::wot::claim

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
