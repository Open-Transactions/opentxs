// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ContactSectionName

#include "opentxs/protobuf/syntax/ContactData.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactData.pb.h>
#include <opentxs/protobuf/ContactSection.pb.h>
#include <opentxs/protobuf/ContactSectionName.pb.h>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/ContactSection.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const ContactData& input,
    const Log& log,
    const ClaimType indexed) -> bool
{
    auto sectionCount = UnallocatedMap<ContactSectionName, std::uint32_t>{};

    for (const auto& it : input.section()) {
        try {
            const bool validSection = check_version(
                it,
                log,
                ContactDataAllowedContactSection().at(input.version()).first,
                ContactDataAllowedContactSection().at(input.version()).second,
                indexed,
                input.version());

            if (!validSection) { FAIL_1("invalid section"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed contact section version not defined for version",
                input.version());
        }

        const auto& name = it.name();

        if (sectionCount.contains(name)) {
            FAIL_1("duplicate section");
        } else {
            sectionCount.insert({name, 1});
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
