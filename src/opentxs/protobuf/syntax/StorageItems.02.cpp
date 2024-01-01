// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageItems.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StorageItems.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_2(const StorageItems& input, const Log& log) -> bool
{
    OPTIONAL_IDENTIFIER(creds);
    OPTIONAL_IDENTIFIER(nyms);
    OPTIONAL_IDENTIFIER(servers);
    OPTIONAL_IDENTIFIER(units);
    OPTIONAL_IDENTIFIER(seeds);
    OPTIONAL_IDENTIFIER(contacts);
    CHECK_EXCLUDED(blockchaintransactions);
    CHECK_EXCLUDED(accounts);
    CHECK_EXCLUDED(notary);
    CHECK_EXCLUDED(master_secret);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
