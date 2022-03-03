// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/AsymmetricKey.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/Ciphertext.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/HDPath.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/AsymmetricKey.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType type,
    const KeyMode mode,
    const KeyRole role) -> bool
{
    CHECK_MEMBERSHIP(type, AsymmetricKeyAllowedTypes);
    CHECK_VALUE(mode, mode);
    CHECK_VALUE(role, role);

    if (KEYMODE_PUBLIC == mode) {
        CHECK_KEY(key);
        CHECK_EXCLUDED(encryptedkey);
    } else {
        if (AKEYTYPE_LEGACY == input.type()) {
            CHECK_KEY(key);
            CHECK_EXCLUDED(encryptedkey);
        } else {
            CHECK_SUBOBJECT_VA(
                encryptedkey, AsymmetricKeyAllowedCiphertext(), false);
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
            FAIL_2("incorrect or unknown type", type)
        }
    }

    CHECK_EXCLUDED(bip32_parent);
    CHECK_EXCLUDED(params);

    return true;
}

auto CheckProto_2(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType type,
    const KeyMode mode,
    const KeyRole role) -> bool
{
    CHECK_MEMBERSHIP(type, AsymmetricKeyAllowedTypes);
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
            FAIL_2("incorrect or unknown type", type)
        }
    }

    return true;
}

auto CheckProto_3(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const AsymmetricKey& input,
    const bool silent,
    const CredentialType,
    const KeyMode,
    const KeyRole) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
