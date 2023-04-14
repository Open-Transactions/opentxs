// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/SymmetricKey.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <Enums.pb.h>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/Ciphertext.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"
#include "internal/util/P0330.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
static constexpr auto crypto_salt_limit_ = 64_uz;

auto CheckProto_1(const SymmetricKey& input, const bool silent) -> bool
{
    try {
        const bool validKey = Check(
            input.key(),
            SymmetricKeyAllowedCiphertext().at(input.version()).first,
            SymmetricKeyAllowedCiphertext().at(input.version()).second,
            silent,
            true);

        if (!validKey) { FAIL_1("invalid encrypted key"); }
    } catch (const std::out_of_range&) {
        FAIL_2(
            "allowed ciphertext version not defined for version",
            input.version());
    }

    if (!input.has_size()) { FAIL_1("missing size"); }

    if (!input.has_type()) { FAIL_1("missing type"); }

    if (crypto_salt_limit_ < input.salt().size()) { FAIL_1("salt too large"); }

    switch (input.type()) {
        case SKEYTYPE_RAW:
        case SKEYTYPE_ECDH: {
        } break;
        case SKEYTYPE_ARGON2:
        case SKEYTYPE_ARGON2ID: {
            if (!input.has_salt()) { FAIL_1("missing salt"); }

            if (1 > input.operations()) { FAIL_1("missing operations"); }

            if (1 > input.difficulty()) { FAIL_1("missing difficulty"); }
        } break;
        case SKEYTYPE_ERROR:
        default: {
            FAIL_2("invalid type", input.type());
        }
    }

    return true;
}

auto CheckProto_2(const SymmetricKey& input, const bool silent) -> bool
{
    try {
        const bool validKey = Check(
            input.key(),
            SymmetricKeyAllowedCiphertext().at(input.version()).first,
            SymmetricKeyAllowedCiphertext().at(input.version()).second,
            silent,
            true);

        if (!validKey) { FAIL_1("invalid encrypted key"); }
    } catch (const std::out_of_range&) {
        FAIL_2(
            "allowed ciphertext version not defined for version",
            input.version());
    }

    if (!input.has_size()) { FAIL_1("missing size"); }

    if (!input.has_type()) { FAIL_1("missing type"); }

    switch (input.type()) {
        case SKEYTYPE_RAW:
        case SKEYTYPE_ECDH: {
            if (input.has_salt()) {
                FAIL_1("salt not valid for this key type");
            }
        } break;
        case SKEYTYPE_ARGON2:
        case SKEYTYPE_ARGON2ID: {
            if (!input.has_salt()) { FAIL_1("missing salt"); }

            if (1 > input.operations()) { FAIL_1("missing operations"); }

            if (crypto_salt_limit_ < input.salt().size()) {
                FAIL_1("salt too large");
            }

            if (1 > input.difficulty()) { FAIL_1("missing difficulty"); }

            if (1 > input.parallel()) { FAIL_1("missing parallel"); }
        } break;
        case SKEYTYPE_ERROR:
        default: {
            FAIL_2("invalid type", input.type());
        }
    }

    return true;
}

auto CheckProto_3(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const SymmetricKey& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
