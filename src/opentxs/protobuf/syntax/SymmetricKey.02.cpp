// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/SymmetricKey.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Ciphertext.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Ciphertext.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_2(const SymmetricKey& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT_VA(key, SymmetricKeyAllowedCiphertext(), true);

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
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
