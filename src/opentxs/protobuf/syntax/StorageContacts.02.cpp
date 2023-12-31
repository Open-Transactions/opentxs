// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/StorageContacts.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StorageContacts.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/StorageContactAddressIndex.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StorageContactNymIndex.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StorageIDList.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StorageItemHash.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyStorage.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_2(const StorageContacts& input, const Log& log) -> bool
{
    OPTIONAL_SUBOBJECTS(merge, StorageContactsAllowedList());
    OPTIONAL_SUBOBJECTS(contact, StorageContactsAllowedStorageItemHash());
    OPTIONAL_SUBOBJECTS(address, StorageContactsAllowedAddress());
    OPTIONAL_SUBOBJECTS(nym, StorageContactsAllowedStorageContactNymIndex());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
