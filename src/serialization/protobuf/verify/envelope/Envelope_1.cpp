// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Envelope.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Enums.pb.h>
#include <Envelope.pb.h>
#include <cstdint>
#include <functional>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/AsymmetricKey.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Ciphertext.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/TaggedKey.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
const UnallocatedMap<std::uint32_t, UnallocatedSet<AsymmetricKeyType>>
    allowed_types_{
        {1, {AKEYTYPE_LEGACY, AKEYTYPE_SECP256K1, AKEYTYPE_ED25519}},
        {2, {AKEYTYPE_LEGACY, AKEYTYPE_SECP256K1, AKEYTYPE_ED25519}},
    };

auto CheckProto_1(const Envelope& input, const bool silent) -> bool
{
    CHECK_SUBOBJECTS_VA(
        dhkey,
        EnvelopeAllowedAsymmetricKey(),
        CREDTYPE_LEGACY,
        KEYMODE_PUBLIC,
        KEYROLE_ENCRYPT);
    CHECK_SUBOBJECTS(sessionkey, EnvelopeAllowedTaggedKey());
    CHECK_SUBOBJECT_VA(ciphertext, EnvelopeAllowedCiphertext(), false);

    auto dh = UnallocatedMap<AsymmetricKeyType, int>{};

    for (const auto& key : input.dhkey()) {
        const auto type = key.type();

        try {
            if (0 == allowed_types_.at(input.version()).count(type)) {
                FAIL_1("Invalid dh key type");
            }
        } catch (...) {
            FAIL_1("Unknown version");
        }

        ++dh[type];
    }

    for (const auto& [type, count] : dh) {
        if ((1 != count) && (AKEYTYPE_LEGACY != type)) {
            FAIL_1("Duplicate dh key type");
        }
    }

    return true;
}

auto CheckProto_2(const Envelope& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const Envelope& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
