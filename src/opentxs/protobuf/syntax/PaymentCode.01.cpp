// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/PaymentCode.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PaymentCode.pb.h>

#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const PaymentCode& input, const Log& log) -> bool
{
    if (!input.has_key()) { FAIL_1("missing pubkey"); }

    if (MIN_PLAUSIBLE_KEYSIZE > input.key().size()) {
        FAIL_1("invalid pubkey");
    }

    if (!input.has_chaincode()) { FAIL_1("missing chaincode"); }

    if (MIN_PLAUSIBLE_KEYSIZE > input.chaincode().size()) {
        FAIL_1("invalid chaincode");
    }

    if (input.has_bitmessage()) {
        if (!input.has_bitmessageversion()) {
            FAIL_1("missing Bitmessage address version");
        }

        if (0xff < input.bitmessageversion()) {
            FAIL_1("invalid Bitmessage address version");
        }

        if (!input.has_bitmessagestream()) {
            FAIL_1("missing Bitmessage address stream");
        }

        if (0xff < input.bitmessagestream()) {
            FAIL_1("invalid Bitmessage address stream");
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
