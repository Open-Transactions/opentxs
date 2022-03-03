// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Verification.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/Signature.pb.h"  // IWYU pragma: keep
#include "serialization/protobuf/Verification.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(
    const Verification& input,
    const bool silent,
    const VerificationType indexed) -> bool
{
    if (VerificationType::Indexed == indexed) {
        if (!input.has_id()) { FAIL_1("missing ID") }

        if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) {
            FAIL_1("invalid ID")
        }
    } else {
        if (input.has_id()) { FAIL_1("ID field not empty") }
    }

    if (!input.has_claim()) { FAIL_1("missing claim") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.claim().size()) {
        FAIL_1("invalid claim")
    }

    if (!input.has_valid()) { FAIL_1("missing validity") }

    if (!input.has_start()) { FAIL_1("missing start time") }

    if (!input.has_end()) { FAIL_1("missing end time") }

    if (input.end() < input.start()) { FAIL_1("invalid end time") }

    if (!input.has_sig()) { FAIL_1("missing signature") }

    try {
        const bool validSignature = Check(
            input.sig(),
            VerificationAllowedSignature().at(input.version()).first,
            VerificationAllowedSignature().at(input.version()).second,
            silent,
            proto::SIGROLE_CLAIM);

        if (!validSignature) { FAIL_1("invalid signature") }
    } catch (const std::out_of_range&) {
        FAIL_2(
            "allowed signature version not defined for version",
            input.version())
    }

    return true;
}

auto CheckProto_2(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const Verification& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
