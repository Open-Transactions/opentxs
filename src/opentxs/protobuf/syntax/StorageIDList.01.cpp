// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageIDList.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StorageIDList.pb.h>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const StorageIDList& input, const Log& log) -> bool
{
    if (false == input.has_id()) { FAIL_1("missing id"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) {
        FAIL_2("invalid id", input.id());
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.id().size()) {
        FAIL_2("invalid id", input.id());
    }

    for (const auto& it : input.list()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > it.size()) {
            FAIL_2("invalid list item", it);
        }

        if (MAX_PLAUSIBLE_IDENTIFIER < it.size()) {
            FAIL_2("invalid list item", it);
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
