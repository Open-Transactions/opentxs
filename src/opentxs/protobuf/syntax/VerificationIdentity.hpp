// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::VerificationType

#pragma once

#include <cstdint>

#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class VerificationIdentity;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
using VerificationNymMap = UnallocatedMap<UnallocatedCString, std::uint64_t>;

auto version_1(
    const VerificationIdentity& verificationIdentity,
    const Log& log,
    VerificationNymMap& map,
    const VerificationType indexed) -> bool;
auto version_2(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_3(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_4(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_5(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_6(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_7(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_8(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_9(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_10(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_11(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_12(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_13(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_14(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_15(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_16(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_17(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_18(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_19(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
auto version_20(
    const VerificationIdentity&,
    const Log& log,
    VerificationNymMap&,
    const VerificationType) -> bool;
}  // namespace opentxs::protobuf::inline syntax
