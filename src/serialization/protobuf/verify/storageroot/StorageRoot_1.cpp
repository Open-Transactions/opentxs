// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageRoot.hpp"  // IWYU pragma: associated

#include "serialization/protobuf/StorageRoot.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const StorageRoot& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(items)
    CHECK_EXCLUDED(sequence)

    return true;
}
}  // namespace opentxs::proto
