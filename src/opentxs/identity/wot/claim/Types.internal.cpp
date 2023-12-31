// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/claim/Types.internal.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <opentxs/protobuf/ContactItemAttribute.pb.h>
#include <opentxs/protobuf/ContactSectionName.pb.h>
#include <cstdint>
#include <functional>
#include <utility>

#include "opentxs/identity/wot/claim/Attribute.hpp"    // IWYU pragma: keep
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep

namespace opentxs::identity::wot::claim
{
constexpr auto attribute_map_ = [] {
    using enum Attribute;
    using enum protobuf::ContactItemAttribute;

    return frozen::
        make_unordered_map<Attribute, protobuf::ContactItemAttribute>({
#include "opentxs/identity/wot/claim/attribute_to_contactitemattribute.inc"  // IWYU pragma: keep
        });
}();

constexpr auto sectiontype_map_ = [] {
    using enum SectionType;
    using enum protobuf::ContactSectionName;

    return frozen::
        make_unordered_map<SectionType, protobuf::ContactSectionName>({
#include "opentxs/identity/wot/claim/section_to_contactitemsection.inc"  // IWYU pragma: keep
        });
}();
}  // namespace opentxs::identity::wot::claim

namespace opentxs::protobuf
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
}  // namespace opentxs::protobuf

namespace opentxs::identity::wot::claim
{
auto translate(const identity::wot::claim::Attribute in) noexcept
    -> protobuf::ContactItemAttribute
{
    const auto& map = attribute_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::CITEMATTR_ERROR;
    }
}

auto translate(const identity::wot::claim::ClaimType in) noexcept
    -> protobuf::ContactItemType
{
    return static_cast<protobuf::ContactItemType>(
        static_cast<std::uint32_t>(in));
}

auto translate(const identity::wot::claim::SectionType in) noexcept
    -> protobuf::ContactSectionName
{
    const auto& map = identity::wot::claim::sectiontype_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::CONTACTSECTION_ERROR;
    }
}
}  // namespace opentxs::identity::wot::claim
