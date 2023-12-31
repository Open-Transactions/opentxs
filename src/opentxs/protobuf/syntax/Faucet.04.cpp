// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Faucet.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactSectionName.pb.h>
#include <opentxs/protobuf/Faucet.pb.h>

#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_4(const Faucet& input, const Log& log) -> bool
{
    const bool validChain =
        ValidContactItemType({6, CONTACTSECTION_CONTRACT}, input.type());

    if (false == validChain) { FAIL_1("invalid chain"); }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.address().size()) {
        FAIL_1("invalid address");
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
