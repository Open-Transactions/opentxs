// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ContactItemAttribute

#include "internal/serialization/protobuf/verify/ContactItem.hpp"  // IWYU pragma: associated

#include <ContactItem.pb.h>
#include <ContactItemAttribute.pb.h>

#include "internal/serialization/protobuf/Contact.hpp"
#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const ContactItem& input, const bool silent) -> bool
{
    OPTIONAL_SUBOBJECT(id, ContactItemAllowedIdentifier());

    if (input.end() < input.start()) { FAIL_1("invalid time range"); }

    CHECK_EXISTS(type);

    if (false == input.has_value()) {
        CHECK_SUBOBJECT(commitment, ContactItemAllowedIdentifier());
    }

    for (const auto& it : input.attribute()) {
        if (!ValidContactItemAttribute(
                input.version(), static_cast<ContactItemAttribute>(it))) {
            FAIL_1("invalid attribute");
        }
    }

    if (input.has_subtype()) {
        if (3 > input.version()) { FAIL_1("Subtype present but not allowed"); }
    }

    OPTIONAL_SUBOBJECTS(superscedes, ContactItemAllowedIdentifier());

    return true;
}

auto CheckProto_1(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    if (false == CheckProto_1(input, silent)) { return false; }

    if (ClaimType::Indexed == indexed) {
        CHECK_EXISTS(id);
    } else {
        CHECK_EXCLUDED(id);
    }

    if (false == ValidContactItemType(parentVersion, input.type())) {
        FAIL_1("invalid type");
    }

    return true;
}

auto CheckProto_2(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    return CheckProto_1(input, silent, indexed, parentVersion);
}

auto CheckProto_2(const ContactItem& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    return CheckProto_1(input, silent, indexed, parentVersion);
}

auto CheckProto_3(const ContactItem& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_4(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    return CheckProto_1(input, silent, indexed, parentVersion);
}

auto CheckProto_4(const ContactItem& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_5(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    return CheckProto_1(input, silent, indexed, parentVersion);
}

auto CheckProto_5(const ContactItem& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_6(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    return CheckProto_1(input, silent, indexed, parentVersion);
}

auto CheckProto_6(const ContactItem& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_7(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_7(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_8(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_9(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_10(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_11(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_12(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_13(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_14(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_15(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_16(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_17(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_18(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_19(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const ContactItem& input,
    const bool silent,
    const ClaimType indexed,
    const ContactSectionVersion parentVersion) -> bool
{
    UNDEFINED_VERSION(20);
}
auto CheckProto_20(const ContactItem& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}

}  // namespace opentxs::proto
