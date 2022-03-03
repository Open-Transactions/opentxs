// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageNym.hpp"  // IWYU pragma: associated

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/HDAccount.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/StorageBlockchainAccountList.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/StorageItemHash.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/StoragePurse.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyStorage.hpp"
#include "serialization/protobuf/StorageNym.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_9(const StorageNym& input, const bool silent) -> bool
{
    OPTIONAL_SUBOBJECT(credlist, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(sentpeerrequests, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(
        incomingpeerrequests, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(sentpeerreply, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(incomingpeerreply, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(finishedpeerrequest, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(finishedpeerreply, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(
        processedpeerrequest, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(processedpeerreply, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(mailinbox, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(mailoutbox, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(threads, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(contexts, StorageNymAllowedStorageItemHash());
    OPTIONAL_SUBOBJECT(accounts, StorageNymAllowedStorageItemHash());
    CHECK_SUBOBJECTS(
        blockchainaccountindex, StorageNymAllowedBlockchainAccountList());
    CHECK_SUBOBJECTS(hdaccount, StorageNymAllowedHDAccount());
    OPTIONAL_IDENTIFIER(issuers);
    OPTIONAL_IDENTIFIER(paymentworkflow);
    OPTIONAL_IDENTIFIER(bip47);
    OPTIONAL_SUBOBJECTS(purse, StorageNymAllowedStoragePurse());

    return true;
}

auto CheckProto_10(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const StorageNym& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
