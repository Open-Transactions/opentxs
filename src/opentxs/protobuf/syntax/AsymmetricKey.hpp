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
class AsymmetricKey;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const AsymmetricKey& key,
    const Log& log,
    const CredentialType type,
    const KeyMode mode,
    const KeyRole role) -> bool;
auto version_2(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_3(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_4(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_5(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_6(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_7(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_8(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_9(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_10(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_11(
    const AsymmetricKey& key,
    const Log& log,
    const CredentialType type,
    const KeyMode mode,
    const KeyRole role) -> bool;
auto version_12(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_13(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_14(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_15(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_16(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_17(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_18(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_19(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
auto version_20(
    const AsymmetricKey&,
    const Log&,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool;
}  // namespace opentxs::protobuf::inline syntax
