// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/NymIDSource.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/AsymmetricKey.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/NymIDSource.pb.h>
#include <opentxs/protobuf/PaymentCode.pb.h>  // IWYU pragma: keep
#include <string>

#include "opentxs/protobuf/syntax/AsymmetricKey.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/PaymentCode.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const NymIDSource& input, const Log& log) -> bool
{
    switch (input.type()) {
        case SOURCETYPE_PUBKEY: {
            CHECK_EXCLUDED(paymentcode);
            CHECK_SUBOBJECT_VA(
                key,
                NymIDSourceAllowedAsymmetricKey(),
                CREDTYPE_LEGACY,
                KEYMODE_PUBLIC,
                KEYROLE_SIGN);
        } break;
        case SOURCETYPE_BIP47: {
            CHECK_EXCLUDED(key);
            CHECK_SUBOBJECT(paymentcode, NymIDSourceAllowedPaymentCode());
        } break;
        case SOURCETYPE_ERROR:
        default:
            FAIL_2("incorrect or unknown type", input.type());
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
