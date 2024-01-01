// Copyright (c) 2018-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainBlockLocalData.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainBlockLocalData.pb.h>

#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainBlockLocalData& input, const Log& log) -> bool
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (input.has_work()) { CHECK_STRING_(work, 0, MAX_PLAUSIBLE_WORK); }
    if (input.has_inherit_work()) {
        CHECK_STRING_(inherit_work, 0, MAX_PLAUSIBLE_WORK);
    }
#pragma GCC diagnostic pop

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
