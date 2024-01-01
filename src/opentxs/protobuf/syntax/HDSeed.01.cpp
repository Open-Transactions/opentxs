// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/HDSeed.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/HDSeed.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyCrypto.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const HDSeed& input, const Log& log) -> bool
{
    OPTIONAL_SUBOBJECT(id, HDSeedAllowedIdentifier());
    CHECK_EXISTS(words);

    // 12 three-letter words
    if (47 > input.words().size()) { FAIL_1("Invalid word list"); }

    // 24 eight-letter words
    if (215 < input.words().size()) { FAIL_1("Invalid word list"); }

    if (1024 < input.passphrase().size()) { FAIL_1("Invalid passphrase"); }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
