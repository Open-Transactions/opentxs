// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Numbers.hpp"

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

namespace opentxs::protobuf
{
auto translate(const ContactItemAttribute in) noexcept
    -> identity::wot::claim::Attribute;
auto translate(const ContactItemType in) noexcept
    -> identity::wot::claim::ClaimType;
auto translate(const ContactSectionName in) noexcept
    -> identity::wot::claim::SectionType;
}  // namespace opentxs::protobuf

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
    -> protobuf::ContactItemAttribute;
auto translate(const identity::wot::claim::ClaimType in) noexcept
    -> protobuf::ContactItemType;
auto translate(const identity::wot::claim::SectionType in) noexcept
    -> protobuf::ContactSectionName;
}  // namespace opentxs::identity::wot::claim
