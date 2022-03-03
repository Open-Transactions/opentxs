// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageContactAddressIndex.hpp"  // IWYU pragma: associated

#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/ContactEnums.pb.h"
#include "serialization/protobuf/StorageContactAddressIndex.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    if (false == input.has_contact()) { FAIL_1("missing contact") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.contact().size()) {
        FAIL_2("invalid contact", input.contact())
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.contact().size()) {
        FAIL_2("invalid contact", input.contact())
    }

    const bool validChain =
        ValidContactItemType({6, CONTACTSECTION_CONTRACT}, input.chain());

    if (false == validChain) { FAIL_1("invalid type") }

    for (const auto& it : input.address()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > it.size()) {
            FAIL_2("invalid address", it)
        }

        if (MAX_PLAUSIBLE_IDENTIFIER < it.size()) {
            FAIL_2("invalid address", it)
        }
    }

    return true;
}

auto CheckProto_2(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const StorageContactAddressIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
