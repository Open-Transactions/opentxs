// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageItems.hpp"  // IWYU pragma: associated

#include "serialization/protobuf/StorageItems.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const StorageItems& input, const bool silent) -> bool
{
    OPTIONAL_IDENTIFIER(creds);
    OPTIONAL_IDENTIFIER(nyms);
    OPTIONAL_IDENTIFIER(servers);
    OPTIONAL_IDENTIFIER(units);
    OPTIONAL_IDENTIFIER(seeds);
    CHECK_EXCLUDED(contacts);
    CHECK_EXCLUDED(blockchaintransactions);
    CHECK_EXCLUDED(accounts);
    CHECK_EXCLUDED(notary);
    CHECK_EXCLUDED(master_secret);

    return true;
}
}  // namespace opentxs::proto
