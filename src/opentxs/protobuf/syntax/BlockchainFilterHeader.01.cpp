// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainFilterHeader.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainFilterHeader.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainFilterHeader& input, const Log& log) -> bool
{
    CHECK_IDENTIFIER(header);
    CHECK_IDENTIFIER(hash);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep