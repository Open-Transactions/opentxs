// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageNym.hpp"  // IWYU pragma: associated

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/HDAccount.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/StorageBlockchainAccountList.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/StorageItemHash.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyStorage.hpp"
#include "serialization/protobuf/StorageNym.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_4(const StorageNym& input, const bool silent) -> bool
{
    OPTIONAL_SUBOBJECT(credlist, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(sentpeerrequests, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(incomingpeerrequests, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(sentpeerreply, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(incomingpeerreply, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(finishedpeerrequest, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(finishedpeerreply, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(processedpeerrequest, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(processedpeerreply, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(mailinbox, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(mailoutbox, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(threads, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(contexts, StorageNymAllowedStorageItemHash())
    OPTIONAL_SUBOBJECT(accounts, StorageNymAllowedStorageItemHash())
    CHECK_SUBOBJECTS(
        blockchainaccountindex, StorageNymAllowedBlockchainAccountList())
    CHECK_SUBOBJECTS(hdaccount, StorageNymAllowedHDAccount())
    CHECK_EXCLUDED(issuers)
    CHECK_EXCLUDED(paymentworkflow)
    CHECK_EXCLUDED(bip47)
    CHECK_NONE(purse);

    return true;
}
}  // namespace opentxs::proto
