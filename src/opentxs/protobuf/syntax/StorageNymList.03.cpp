// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageNymList.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StorageNymList.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/StorageBip47NymAddressIndex.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StorageItemHash.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyStorage.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_3(const StorageNymList& input, const Log& log) -> bool
{
    CHECK_SUBOBJECTS(nym, StorageNymListAllowedStorageItemHash());
    CHECK_IDENTIFIERS(localnymid);
    CHECK_NONE(address);
    CHECK_NONE(transaction);
    CHECK_EXCLUDED(defaultlocalnym);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
