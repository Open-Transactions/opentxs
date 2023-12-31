// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageBlockchainAccountList.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactItemType.pb.h>
#include <opentxs/protobuf/ContactSectionName.pb.h>
#include <opentxs/protobuf/StorageBlockchainAccountList.pb.h>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const StorageBlockchainAccountList& input, const Log& log)
    -> bool
{
    if (false == input.has_id()) { FAIL_1("missing id"); }

    const bool validChain =
        ValidContactItemType({6, CONTACTSECTION_CONTRACT}, input.id());

    if ((false == validChain) && (input.id() != CITEMTYPE_REGTEST)) {
        FAIL_1("invalid chain");
    }

    for (const auto& it : input.list()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > it.size()) {
            FAIL_2("invalid list item", it);
        }

        if (MAX_PLAUSIBLE_IDENTIFIER < it.size()) {
            FAIL_2("invalid list item", it);
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
