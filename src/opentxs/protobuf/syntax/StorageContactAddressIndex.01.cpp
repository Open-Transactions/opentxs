// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageContactAddressIndex.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactSectionName.pb.h>
#include <opentxs/protobuf/StorageContactAddressIndex.pb.h>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const StorageContactAddressIndex& input, const Log& log) -> bool
{
    if (false == input.has_contact()) { FAIL_1("missing contact"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.contact().size()) {
        FAIL_2("invalid contact", input.contact());
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.contact().size()) {
        FAIL_2("invalid contact", input.contact());
    }

    const bool validChain =
        ValidContactItemType({6, CONTACTSECTION_CONTRACT}, input.chain());

    if (false == validChain) { FAIL_1("invalid type"); }

    for (const auto& it : input.address()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > it.size()) {
            FAIL_2("invalid address", it);
        }

        if (MAX_PLAUSIBLE_IDENTIFIER < it.size()) {
            FAIL_2("invalid address", it);
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
