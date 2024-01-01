// Copyright (c) 2018-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BitcoinBlockHeaderFields.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BitcoinBlockHeaderFields.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BitcoinBlockHeaderFields& input, const Log& log) -> bool
{
    REQUIRED_SIZE(previous_header, 32);
    REQUIRED_SIZE(merkle_hash, 32);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
