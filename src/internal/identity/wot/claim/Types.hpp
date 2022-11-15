// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::Attribute
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::ClaimType
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::SectionType
// IWYU pragma: no_forward_declare opentxs::proto::ContactItemAttribute
// IWYU pragma: no_include "opentxs/identity/wot/claim/Attribute.hpp"
// IWYU pragma: no_include "opentxs/identity/wot/claim/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/identity/wot/claim/SectionType.hpp"

#pragma once

#include "opentxs/identity/wot/claim/Types.hpp"

#include <ContactEnums.pb.h>

#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
constexpr auto CONTACT_CONTACT_DATA_VERSION = 6;
}  // namespace opentxs

namespace opentxs
{
auto translate(const identity::wot::claim::Attribute in) noexcept
    -> proto::ContactItemAttribute;
auto translate(const identity::wot::claim::ClaimType in) noexcept
    -> proto::ContactItemType;
auto translate(const identity::wot::claim::SectionType in) noexcept
    -> proto::ContactSectionName;
auto translate(const proto::ContactItemAttribute in) noexcept
    -> identity::wot::claim::Attribute;
auto translate(const proto::ContactItemType in) noexcept
    -> identity::wot::claim::ClaimType;
auto translate(const proto::ContactSectionName in) noexcept
    -> identity::wot::claim::SectionType;
}  // namespace opentxs

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
}  // namespace opentxs::identity::wot::claim
