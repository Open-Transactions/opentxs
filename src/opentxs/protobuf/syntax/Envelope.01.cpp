// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Envelope.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/AsymmetricKey.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/Envelope.pb.h>
#include <cstdint>
#include <functional>
#include <utility>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/AsymmetricKey.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Ciphertext.hpp"     // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/TaggedKey.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
const UnallocatedMap<std::uint32_t, UnallocatedSet<AsymmetricKeyType>>
    allowed_types_{
        {1, {AKEYTYPE_LEGACY, AKEYTYPE_SECP256K1, AKEYTYPE_ED25519}},
        {2, {AKEYTYPE_LEGACY, AKEYTYPE_SECP256K1, AKEYTYPE_ED25519}},
    };

auto version_1(const Envelope& input, const Log& log) -> bool
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
            if (false == allowed_types_.at(input.version()).contains(type)) {
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
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
