// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/AsymmetricKey.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/AsymmetricKey.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <functional>

#include "opentxs/protobuf/syntax/Ciphertext.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/HDPath.hpp"      // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_2(
    const AsymmetricKey& input,
    const Log& log,
    const CredentialType type,
    const KeyMode mode,
    const KeyRole role) -> bool
{
    CHECK_MEMBERSHIP(type, AsymmetricKeyAllowedAsymmetricKeyType());
    CHECK_VALUE(mode, mode);
    CHECK_VALUE(role, role);
    CHECK_KEY(key);

    if (KEYMODE_PUBLIC == mode) {
        CHECK_EXCLUDED(encryptedkey);
    } else {
        CHECK_SUBOBJECT_VA(
            encryptedkey, AsymmetricKeyAllowedCiphertext(), false);
    }

    switch (input.type()) {
        case AKEYTYPE_LEGACY: {
            if (KEYROLE_ENCRYPT == input.role()) {
                CHECK_KEY(params);
            } else {
                CHECK_EXCLUDED(params);
            }
        } break;
        case AKEYTYPE_ED25519:
        case AKEYTYPE_SECP256K1:
        case AKEYTYPE_NULL:
        case AKEYTYPE_ERROR:
        default: {
            CHECK_EXCLUDED(params);
        }
    }

    switch (type) {
        case CREDTYPE_LEGACY: {
            CHECK_EXCLUDED(chaincode);
            CHECK_EXCLUDED(path);
        } break;
        case CREDTYPE_HD: {
            if (KEYMODE_PUBLIC == mode) {
                CHECK_EXCLUDED(chaincode);
                CHECK_EXCLUDED(path);
            } else {
                CHECK_SUBOBJECT_VA(
                    chaincode, AsymmetricKeyAllowedCiphertext(), false);
                CHECK_SUBOBJECT(path, AsymmetricKeyAllowedHDPath());
            }
        } break;
        case CREDTYPE_ERROR:
        default: {
            FAIL_2("incorrect or unknown type", type);
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
