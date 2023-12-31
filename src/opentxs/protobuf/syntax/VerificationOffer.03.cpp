// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/VerificationOffer.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Claim.pb.h>
#include <opentxs/protobuf/Identifier.pb.h>
#include <opentxs/protobuf/VerificationItem.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/VerificationOffer.pb.h>

#include "opentxs/protobuf/syntax/Claim.hpp"       // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerificationItem.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_3(const VerificationOffer& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(offeringnym, VerificationOfferAllowedIdentifier());
    CHECK_SUBOBJECT(recipientnym, VerificationOfferAllowedIdentifier());
    CHECK_SUBOBJECT(claim, VerificationOfferAllowedClaim());
    CHECK_SUBOBJECT_VA(
        verification,
        VerificationOfferAllowedVerificationItem(),
        VerificationType::Normal);

    if (input.claim().nym().hash() != input.recipientnym().hash()) {
        FAIL_1("claim nym does not match recipient nym");
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
