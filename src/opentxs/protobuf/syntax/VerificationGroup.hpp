// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::VerificationType

#pragma once

#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class VerificationGroup;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const VerificationGroup& verificationGroup,
    const Log& log,
    const VerificationType indexed) -> bool;
auto version_2(const VerificationGroup&, const Log& log, const VerificationType)
    -> bool;
auto version_3(const VerificationGroup&, const Log& log, const VerificationType)
    -> bool;
auto version_4(const VerificationGroup&, const Log& log, const VerificationType)
    -> bool;
auto version_5(const VerificationGroup&, const Log& log, const VerificationType)
    -> bool;
auto version_6(const VerificationGroup&, const Log& log, const VerificationType)
    -> bool;
auto version_7(const VerificationGroup&, const Log& log, const VerificationType)
    -> bool;
auto version_8(const VerificationGroup&, const Log& log, const VerificationType)
    -> bool;
auto version_9(const VerificationGroup&, const Log& log, const VerificationType)
    -> bool;
auto version_10(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_11(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_12(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_13(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_14(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_15(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_16(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_17(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_18(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_19(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
auto version_20(
    const VerificationGroup&,
    const Log& log,
    const VerificationType) -> bool;
}  // namespace opentxs::protobuf::inline syntax
