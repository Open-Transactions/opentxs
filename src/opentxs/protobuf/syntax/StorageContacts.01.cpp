// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageContacts.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StorageContacts.pb.h>
#include <string>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/StorageContactAddressIndex.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StorageIDList.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StorageItemHash.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyStorage.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const StorageContacts& input, const Log& log) -> bool
{
    OPTIONAL_SUBOBJECTS(merge, StorageContactsAllowedList());
    OPTIONAL_SUBOBJECTS(contact, StorageContactsAllowedStorageItemHash());
    OPTIONAL_SUBOBJECTS(address, StorageContactsAllowedAddress());

    if (0 < input.nym().size()) {
        FAIL_2("nym index not allowed for version", input.version());
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
