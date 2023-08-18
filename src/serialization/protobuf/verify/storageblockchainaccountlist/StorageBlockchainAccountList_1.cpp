// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/StorageBlockchainAccountList.hpp"  // IWYU pragma: associated

#include <ContactItemType.pb.h>
#include <ContactSectionName.pb.h>
#include <StorageBlockchainAccountList.pb.h>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    if (false == input.has_id()) { FAIL_1("missing id"); }

    const bool validChain =
        ValidContactItemType({6, CONTACTSECTION_CONTRACT}, input.id());

    if ((false == validChain) && (input.id() != CITEMTYPE_REGTEST)) {
        FAIL_1("invalid chain");
    }

    for (const auto& it : input.list()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > it.size()) {
            FAIL_2("invalid list item", it);
        }

        if (MAX_PLAUSIBLE_IDENTIFIER < it.size()) {
            FAIL_2("invalid list item", it);
        }
    }

    return true;
}

auto CheckProto_2(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const StorageBlockchainAccountList& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
