// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/VerificationOffer.hpp"  // IWYU pragma: associated

#include <Claim.pb.h>
#include <Identifier.pb.h>
#include <VerificationItem.pb.h>  // IWYU pragma: keep
#include <VerificationOffer.pb.h>

#include "internal/serialization/protobuf/verify/Claim.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerificationItem.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(1);
}

auto CheckProto_2(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const VerificationOffer& input, const bool silent) -> bool
{
    CHECK_SUBOBJECT(offeringnym, VerificationOfferAllowedIdentifier());
    CHECK_SUBOBJECT(recipientnym, VerificationOfferAllowedIdentifier());
    CHECK_SUBOBJECT(claim, VerificationOfferAllowedClaim());
    CHECK_SUBOBJECT_(
        verification,
        VerificationOfferAllowedVerificationItem(),
        silent,
        VerificationType::Normal);

    if (input.claim().nym().hash() != input.recipientnym().hash()) {
        FAIL_1("claim nym does not match recipient nym");
    }

    return true;
}

auto CheckProto_4(const VerificationOffer& input, const bool silent) -> bool
{
    return CheckProto_3(input, silent);
}

auto CheckProto_5(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const VerificationOffer& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
