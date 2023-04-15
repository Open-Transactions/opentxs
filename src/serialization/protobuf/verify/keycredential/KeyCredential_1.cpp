// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/KeyCredential.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Enums.pb.h>
#include <KeyCredential.pb.h>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/AsymmetricKey.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(
    const KeyCredential& input,
    const bool silent,
    const CredentialType credType,
    const KeyMode mode) -> bool
{
    AsymmetricKey authKey;
    AsymmetricKey encryptKey;
    AsymmetricKey signKey;
    bool validAuthKey = false;
    bool validEncryptKey = false;
    bool validSignKey = false;

    if (!input.has_mode()) { FAIL_1("missing mode"); }

    if (input.mode() != mode) { FAIL_2("incorrect mode", input.mode()); }

    if (3 != input.key_size()) {
        FAIL_4("wrong number of keys", input.key_size(), " required: ", "3");
    }

    authKey = input.key(KEYROLE_AUTH - 1);
    encryptKey = input.key(KEYROLE_ENCRYPT - 1);
    signKey = input.key(KEYROLE_SIGN - 1);

    try {
        validAuthKey = Check(
            authKey,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).first,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).second,
            silent,
            credType,
            mode,
            KEYROLE_AUTH);
        validEncryptKey = Check(
            encryptKey,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).first,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).second,
            silent,
            credType,
            mode,
            KEYROLE_ENCRYPT);
        validSignKey = Check(
            signKey,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).first,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).second,
            silent,
            credType,
            mode,
            KEYROLE_SIGN);

        if (!validAuthKey) { FAIL_1("invalid auth key"); }

        if (!validEncryptKey) { FAIL_1("invalid encrypt key"); }

        if (!validSignKey) { FAIL_1("invalid sign key"); }
    } catch (const std::out_of_range&) {
        FAIL_2(
            "allowed asymmetric key version not defined for version",
            input.version());
    }

    return true;
}

auto CheckProto_2(
    const KeyCredential& input,
    const bool silent,
    const CredentialType type,
    const KeyMode mode) -> bool
{
    return CheckProto_1(input, silent, type, mode);
}

auto CheckProto_3(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const KeyCredential& input,
    const bool silent,
    const CredentialType,
    const KeyMode) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
