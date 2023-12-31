// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/Enums.pb.h>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class Credential;
}  // namespace protobuf

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const Credential& serializedCred,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_2(
    const Credential& serializedCred,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_3(
    const Credential& serializedCred,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_4(
    const Credential& serializedCred,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_5(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_6(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_7(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_8(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_9(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_10(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_11(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_12(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_13(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_14(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_15(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_16(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_17(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_18(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_19(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
auto version_20(
    const Credential&,
    const Log& log,
    const KeyMode& mode = KEYMODE_ERROR,
    const CredentialRole role = CREDROLE_ERROR,
    const bool withSigs = true) -> bool;
}  // namespace opentxs::protobuf::inline syntax
