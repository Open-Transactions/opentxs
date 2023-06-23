// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ContactItemAttribute

#pragma once

#include "opentxs/identity/wot/claim/Types.hpp"

#include <ContactEnums.pb.h>

#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
constexpr auto CONTACT_CONTACT_DATA_VERSION = 6;
}  // namespace opentxs

namespace opentxs::proto
{
auto translate(const ContactItemAttribute in) noexcept
    -> identity::wot::claim::Attribute;
auto translate(const ContactItemType in) noexcept
    -> identity::wot::claim::ClaimType;
auto translate(const ContactSectionName in) noexcept
    -> identity::wot::claim::SectionType;
}  // namespace opentxs::proto

namespace opentxs::identity::wot::claim
{
constexpr auto check_version(
    const VersionNumber in,
    const VersionNumber targetVersion) -> VersionNumber
{
    // Upgrade version
    if (targetVersion > in) { return targetVersion; }

    return in;
}

auto translate(const identity::wot::claim::Attribute in) noexcept
    -> proto::ContactItemAttribute;
auto translate(const identity::wot::claim::ClaimType in) noexcept
    -> proto::ContactItemType;
auto translate(const identity::wot::claim::SectionType in) noexcept
    -> proto::ContactSectionName;
}  // namespace opentxs::identity::wot::claim
