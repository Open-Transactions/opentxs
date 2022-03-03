// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/VerificationGroup.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/VerificationIdentity.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/VerificationGroup.pb.h"
#include "serialization/protobuf/VerificationIdentity.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType indexed) -> bool
{
    VerificationNymMap nymMap;

    for (auto& it : input.identity()) {
        try {
            const bool validIdentity = Check(
                it,
                VerificationGroupAllowedIdentity().at(input.version()).first,
                VerificationGroupAllowedIdentity().at(input.version()).second,
                silent,
                nymMap,
                indexed);

            if (!validIdentity) { FAIL_2("invalid identity", it.nym()) }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed verification identity version not defined for version",
                input.version())
        }
    }

    for (auto& nym : nymMap) {
        if (nym.second > 1) { FAIL_2("duplicate identity", nym.first) }
    }

    return true;
}

auto CheckProto_2(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const VerificationGroup& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
