// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/KeyCredential.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/AsymmetricKey.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/KeyCredential.pb.h>
#include <stdexcept>
#include <utility>

#include "opentxs/protobuf/syntax/AsymmetricKey.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const KeyCredential& input,
    const Log& log,
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
        validAuthKey = check_version(
            authKey,
            log,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).first,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).second,
            credType,
            mode,
            KEYROLE_AUTH);
        validEncryptKey = check_version(
            encryptKey,
            log,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).first,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).second,
            credType,
            mode,
            KEYROLE_ENCRYPT);
        validSignKey = check_version(
            signKey,
            log,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).first,
            KeyCredentialAllowedAsymmetricKey().at(input.version()).second,
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
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
