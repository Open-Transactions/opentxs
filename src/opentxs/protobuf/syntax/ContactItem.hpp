// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ClaimType

#pragma once

#include "opentxs/protobuf/contact/Types.internal.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class ContactItem;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_1(const ContactItem& contactItem, const Log& log) -> bool;
auto version_2(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_2(const ContactItem& contactItem, const Log& log) -> bool;
auto version_3(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_3(const ContactItem& contactItem, const Log& log) -> bool;
auto version_4(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_4(const ContactItem& contactItem, const Log& log) -> bool;
auto version_5(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_5(const ContactItem& contactItem, const Log& log) -> bool;
auto version_6(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_6(const ContactItem& contactItem, const Log& log) -> bool;
auto version_7(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_7(const ContactItem& contactItem, const Log& log) -> bool;
auto version_8(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_8(const ContactItem& contactItem, const Log& log) -> bool;
auto version_9(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_9(const ContactItem& contactItem, const Log& log) -> bool;
auto version_10(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_10(const ContactItem& contactItem, const Log& log) -> bool;
auto version_11(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_11(const ContactItem& contactItem, const Log& log) -> bool;
auto version_12(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_12(const ContactItem& contactItem, const Log& log) -> bool;
auto version_13(const ContactItem& contactItem, const Log& log) -> bool;
auto version_13(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_14(const ContactItem& contactItem, const Log& log) -> bool;
auto version_14(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_15(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_15(const ContactItem& contactItem, const Log& log) -> bool;
auto version_16(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_16(const ContactItem& contactItem, const Log& log) -> bool;
auto version_17(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_17(const ContactItem& contactItem, const Log& log) -> bool;
auto version_18(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_18(const ContactItem& contactItem, const Log& log) -> bool;
auto version_19(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_19(const ContactItem& contactItem, const Log& log) -> bool;
auto version_20(
    const ContactItem& contactItem,
    const Log& log,
    const ClaimType indexed,
    const contact::ContactSectionVersion parentVersion) -> bool;
auto version_20(const ContactItem& contactItem, const Log& log) -> bool;
}  // namespace opentxs::protobuf::inline syntax
