// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::CredentialType

#pragma once

#include <opentxs/protobuf/Enums.pb.h>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class KeyCredential;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const KeyCredential& keyCredential,
    const Log& log,
    const CredentialType credType,
    const KeyMode mode) -> bool;
auto version_2(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_3(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_4(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_5(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_6(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_7(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_8(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_9(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_10(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_11(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_12(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_13(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_14(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_15(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_16(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_17(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_18(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_19(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
auto version_20(
    const KeyCredential&,
    const Log& log,
    const CredentialType,
    const KeyMode) -> bool;
}  // namespace opentxs::protobuf::inline syntax
