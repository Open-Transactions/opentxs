// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageItemHash.hpp"  // IWYU pragma: associated

#include <StorageItemHash.pb.h>

#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const StorageItemHash& input, const bool silent) -> bool
{
    OPTIONAL_IDENTIFIER(item_id_base58);
    CHECK_IDENTIFIER(hash);
    CHECK_EXCLUDED(type);

    return true;
}
}  // namespace opentxs::proto
