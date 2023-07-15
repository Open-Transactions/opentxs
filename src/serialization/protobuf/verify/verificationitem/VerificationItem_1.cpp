// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/VerificationItem.hpp"  // IWYU pragma: associated

#include <Enums.pb.h>
#include <Signature.pb.h>  // IWYU pragma: keep
#include <VerificationItem.pb.h>

#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(
    const VerificationItem& input,
    const bool silent,
    const VerificationType indexed) -> bool
{
    if (VerificationType::Indexed == indexed) {
        CHECK_SUBOBJECT(id, VerificationItemAllowedIdentifier());
    } else {
        CHECK_EXCLUDED(id);
    }

    CHECK_SUBOBJECT(claim, VerificationItemAllowedIdentifier());
    CHECK_EXISTS(kind);

    if (input.end() < input.start()) { FAIL_1("invalid end time"); }

    CHECK_SUBOBJECT_(
        sig, VerificationItemAllowedSignature(), silent, proto::SIGROLE_CLAIM);
    OPTIONAL_SUBOBJECTS(superscedes, VerificationItemAllowedIdentifier());

    return true;
}

auto CheckProto_2(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const VerificationItem& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
