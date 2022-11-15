// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::VerificationType

#pragma once

#include <cstdint>

#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class VerificationIdentity;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::proto
{
using VerificationNymMap = UnallocatedMap<UnallocatedCString, std::uint64_t>;

auto CheckProto_1(
    const VerificationIdentity& verificationIdentity,
    const bool silent,
    VerificationNymMap& map,
    const VerificationType indexed) -> bool;
auto CheckProto_2(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_3(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_4(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_5(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_6(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_7(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_8(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_9(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_10(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_11(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_12(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_13(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_14(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_15(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_16(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_17(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_18(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_19(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto CheckProto_20(
    const VerificationIdentity&,
    const bool,
    VerificationNymMap&,
    const VerificationType) -> bool;
}  // namespace opentxs::proto
