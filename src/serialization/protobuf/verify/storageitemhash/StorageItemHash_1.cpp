// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageItemHash.hpp"  // IWYU pragma: associated

#include <StorageEnums.pb.h>
#include <StorageItemHash.pb.h>

#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const StorageItemHash& input, const bool silent) -> bool
{
    if (!input.has_itemid()) { FAIL_1("missing id"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.itemid().size()) {
        FAIL_1("invalid id");
    }

    if (!input.has_hash()) { FAIL_1("missing hash"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.hash().size()) {
        FAIL_1("invalid has");
    }

    if (input.has_type() && (STORAGEHASH_ERROR != input.type())) {
        FAIL_1("unexpected type field present");
    }

    return true;
}
}  // namespace opentxs::proto
