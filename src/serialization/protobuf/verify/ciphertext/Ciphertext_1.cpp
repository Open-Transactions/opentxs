// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Ciphertext.hpp"  // IWYU pragma: associated

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/Check.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/SymmetricKey.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/Ciphertext.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const Ciphertext& input, const bool silent, const bool nested)
    -> bool
{
    if (!input.has_mode()) { FAIL_1("missing mode") }

    switch (input.mode()) {
        case SMODE_CHACHA20POLY1305: {
            break;
        }
        case SMODE_ERROR:
        default: {
            FAIL_2("invalid mode", input.mode())
        }
    }

    if (nested) {
        CHECK_EXCLUDED(key);
    } else {
        OPTIONAL_SUBOBJECT(key, CiphertextAllowedSymmetricKey());
    }

    if (!input.has_iv()) { FAIL_1("missing iv") }

    if (1 > input.iv().size()) { FAIL_1("invalid iv") }

    if (!input.has_tag()) { FAIL_1("missing tag") }

    if (1 > input.tag().size()) { FAIL_1("invalid tag") }

    if (!input.has_data()) { FAIL_1("missing data") }

    return true;
}
auto CheckProto_2(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const Ciphertext& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
