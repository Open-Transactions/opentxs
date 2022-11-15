// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ContactSectionName

#include "internal/serialization/protobuf/verify/ContactData.hpp"  // IWYU pragma: associated

#include <ContactData.pb.h>
#include <ContactEnums.pb.h>
#include <ContactSection.pb.h>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/ContactSection.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UnallocatedMap<ContactSectionName, uint32_t> sectionCount;

    for (const auto& it : input.section()) {
        try {
            bool validSection = Check(
                it,
                ContactDataAllowedContactSection().at(input.version()).first,
                ContactDataAllowedContactSection().at(input.version()).second,
                silent,
                indexed,
                input.version());

            if (!validSection) { FAIL_1("invalid section"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed contact section version not defined for version",
                input.version());
        }

        ContactSectionName name = it.name();

        if (sectionCount.count(name) > 0) {
            FAIL_1("duplicate section");
        } else {
            sectionCount.insert({name, 1});
        }
    }

    return true;
}

auto CheckProto_2(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    return CheckProto_1(input, silent, indexed);
}

auto CheckProto_3(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    return CheckProto_1(input, silent, indexed);
}

auto CheckProto_4(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    return CheckProto_1(input, silent, indexed);
}

auto CheckProto_5(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    return CheckProto_1(input, silent, indexed);
}

auto CheckProto_6(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    return CheckProto_1(input, silent, indexed);
}

auto CheckProto_7(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const ContactData& input,
    const bool silent,
    const ClaimType indexed) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
