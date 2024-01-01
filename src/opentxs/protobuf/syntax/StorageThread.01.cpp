// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageThread.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StorageThread.pb.h>
#include <opentxs/protobuf/StorageThreadItem.pb.h>  // IWYU pragma: keep

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/StorageThreadItem.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyStorage.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const StorageThread& input, const Log& log) -> bool
{
    if (!input.has_id()) { FAIL_1("missing id"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) { FAIL_1("invalid id"); }

    for (const auto& nym : input.participant()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > nym.size()) {
            FAIL_1("invalid participant");
        }
    }

    if (0 == input.participant_size()) { FAIL_1("no patricipants"); }

    OPTIONAL_SUBOBJECTS(item, StorageThreadAllowedItem());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
