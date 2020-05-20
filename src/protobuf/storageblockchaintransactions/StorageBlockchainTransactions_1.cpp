// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Proto.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/verify/StorageBlockchainTransactions.hpp"
#include "opentxs/protobuf/verify/VerifyStorage.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "storage blockchain transactions"

namespace opentxs
{
namespace proto
{

auto CheckProto_1(const StorageBlockchainTransactions& input, const bool silent)
    -> bool
{
    OPTIONAL_SUBOBJECTS(
        transaction, StorageBlockchainTransactionsAllowedStorageItemHash())
    CHECK_NONE(nymindex)

    return true;
}

auto CheckProto_2(const StorageBlockchainTransactions& input, const bool silent)
    -> bool
{
    OPTIONAL_SUBOBJECTS(
        transaction, StorageBlockchainTransactionsAllowedStorageItemHash())
    OPTIONAL_SUBOBJECTS(
        nymindex, StorageBlockchainTransactionsAllowedStorageContactNymIndex())

    return true;
}

auto CheckProto_3(const StorageBlockchainTransactions& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const StorageBlockchainTransactions& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const StorageBlockchainTransactions& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const StorageBlockchainTransactions& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const StorageBlockchainTransactions& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const StorageBlockchainTransactions& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const StorageBlockchainTransactions& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const StorageBlockchainTransactions& input,
    const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs