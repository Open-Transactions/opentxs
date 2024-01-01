// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Ciphertext.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Ciphertext.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <string>

#include "internal/util/P0330.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/SymmetricKey.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const Ciphertext& input, const Log& log, const bool nested)
    -> bool
{
    if (!input.has_mode()) { FAIL_1("missing mode"); }

    switch (input.mode()) {
        case SMODE_CHACHA20POLY1305: {
            break;
        }
        case SMODE_ERROR:
        default: {
            FAIL_2("invalid mode", input.mode());
        }
    }

    if (nested) {
        CHECK_EXCLUDED(key);
    } else {
        OPTIONAL_SUBOBJECT(key, CiphertextAllowedSymmetricKey());
    }

    static constexpr auto limit = 64_uz;

    if (1 > input.iv().size()) { FAIL_1("iv too small"); }

    if (limit < input.iv().size()) { FAIL_1("iv too large"); }

    if (limit < input.tag().size()) { FAIL_1("tag too large"); }

    if (!input.has_data()) { FAIL_1("missing data"); }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
