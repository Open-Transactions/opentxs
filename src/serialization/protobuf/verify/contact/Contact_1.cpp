// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Contact.hpp"  // IWYU pragma: associated

#include <Contact.pb.h>
#include <ContactData.pb.h>  // IWYU pragma: keep
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/ContactData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const Contact& input, const bool silent) -> bool
{
    if (false == input.has_id()) { FAIL_1("missing id"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) {
        FAIL_2("invalid id", input.id());
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.id().size()) {
        FAIL_2("invalid id", input.id());
    }

    if (input.has_label()) {
        if (MAX_VALID_CONTACT_VALUE < input.label().size()) {
            FAIL_2("invalid label", input.id());
        }
    }

    if (input.has_contactdata()) {
        try {
            const auto validContactData = Check(
                input.contactdata(),
                ContactAllowedContactData().at(input.version()).first,
                ContactAllowedContactData().at(input.version()).second,
                silent,
                ClaimType::Normal);

            if (!validContactData) { FAIL_1("invalid contact data"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed contact data not defined for version",
                input.version());
        }
    }

    const bool merged = 0 < input.mergedto().size();
    const bool hasMerges = 0 < input.merged().size();

    if (merged && hasMerges) {
        FAIL_1("merged contact may not contain child merges");
    }

    if (merged) {
        if (MIN_PLAUSIBLE_IDENTIFIER > input.mergedto().size()) {
            FAIL_2("invalid mergedto", input.mergedto());
        }

        if (MAX_PLAUSIBLE_IDENTIFIER < input.mergedto().size()) {
            FAIL_2("invalid mergedto", input.mergedto());
        }
    }

    if (hasMerges) {
        for (const auto& merge : input.merged()) {
            if (MIN_PLAUSIBLE_IDENTIFIER > merge.size()) {
                FAIL_2("invalid merge", merge);
            }

            if (MAX_PLAUSIBLE_IDENTIFIER < merge.size()) {
                FAIL_2("invalid merge", merge);
            }
        }
    }

    return true;
}

auto CheckProto_2(const Contact& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const Contact& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_4(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const Contact& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
