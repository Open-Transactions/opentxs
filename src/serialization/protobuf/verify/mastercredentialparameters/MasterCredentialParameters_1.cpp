// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/MasterCredentialParameters.hpp"  // IWYU pragma: associated

#include <MasterCredentialParameters.pb.h>
#include <NymIDSource.pb.h>  // IWYU pragma: keep
#include <SourceProof.pb.h>  // IWYU pragma: keep
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/NymIDSource.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/SourceProof.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(
    const MasterCredentialParameters& input,
    const bool silent,
    bool& expectSourceSignature) -> bool
{
    if (false == input.has_source()) { FAIL_1("missing nym id source"); }

    try {
        const bool validSource = Check(
            input.source(),
            MasterParamsAllowedNymIDSource().at(input.version()).first,
            MasterParamsAllowedNymIDSource().at(input.version()).second,
            silent);

        if (!validSource) { FAIL_1("invalid nym id source"); }
    } catch (const std::out_of_range&) {
        FAIL_2(
            "allowed nym ID source version not defined for version",
            input.version());
    }

    if (!input.has_sourceproof()) { FAIL_1("missing nym id source proof"); }

    try {
        const bool validProof = Check(
            input.sourceproof(),
            MasterParamsAllowedSourceProof().at(input.version()).first,
            MasterParamsAllowedSourceProof().at(input.version()).second,
            silent,
            expectSourceSignature);

        if (!validProof) { FAIL_1("invalid nym id source proof"); }
    } catch (const std::out_of_range&) {
        FAIL_2(
            "allowed source proof version not defined for version",
            input.version());
    }

    return true;
}

auto CheckProto_2(
    const MasterCredentialParameters& input,
    const bool silent,
    bool& expectSourceSignature) -> bool
{
    return CheckProto_1(input, silent, expectSourceSignature);
}

auto CheckProto_3(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const MasterCredentialParameters& input,
    const bool silent,
    bool&) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
