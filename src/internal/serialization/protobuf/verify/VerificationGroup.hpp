// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::VerificationType

#pragma once

#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class VerificationGroup;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::proto
{
auto CheckProto_1(
    const VerificationGroup& verificationGroup,
    const bool silent,
    const VerificationType indexed) -> bool;
auto CheckProto_2(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_3(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_4(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_5(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_6(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_7(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_8(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_9(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_10(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_11(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_12(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_13(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_14(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_15(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_16(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_17(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_18(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_19(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
auto CheckProto_20(const VerificationGroup&, const bool, const VerificationType)
    -> bool;
}  // namespace opentxs::proto
