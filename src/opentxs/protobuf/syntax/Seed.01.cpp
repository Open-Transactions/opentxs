// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Seed.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/Seed.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Ciphertext.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const Seed& input, const Log& log) -> bool
{
    OPTIONAL_SUBOBJECT_VA(words, SeedAllowedCiphertext(), false);
    OPTIONAL_SUBOBJECT_VA(passphrase, SeedAllowedCiphertext(), false);
    CHECK_SUBOBJECT(id, SeedAllowedIdentifier());
    CHECK_EXISTS(index);
    OPTIONAL_SUBOBJECT_VA(raw, SeedAllowedCiphertext(), false);

    switch (input.type()) {
        case SEEDTYPE_RAW: {
            if (input.has_words()) {
                FAIL_1("Unexpected words present in raw seed");
            }

            if (input.has_passphrase()) {
                FAIL_1("Unexpected passphrase present in raw seed");
            }

            if (SEEDLANG_NONE != input.lang()) {
                FAIL_1("Invalid language for raw seed");
            }
        } break;
        case SEEDTYPE_BIP39:
        case SEEDTYPE_PKT: {
            if (false == input.has_words()) { FAIL_1("Missing words"); }

            switch (input.lang()) {
                case SEEDLANG_EN: {
                    break;
                }
                case SEEDLANG_ERROR:
                case SEEDLANG_NONE:
                default: {
                    FAIL_1("Invalid language");
                }
            }
        } break;
        case SEEDTYPE_ERROR:
        default: {
            FAIL_1("Invalid type");
        }
    }

    if (false == input.has_raw()) { FAIL_1("Missing root node"); }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
