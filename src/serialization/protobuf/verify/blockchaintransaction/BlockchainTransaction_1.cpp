// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/BlockchainTransaction.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Contact.hpp"
#include "internal/serialization/protobuf/verify/BlockchainTransactionInput.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/BlockchainTransactionOutput.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyBlockchain.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/BlockchainTransaction.pb.h"
#include "serialization/protobuf/BlockchainTransactionInput.pb.h"
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const BlockchainTransaction& input, const bool silent) -> bool
{
    for (const auto& chain : input.chain()) {
        const bool validChain = ValidContactItemType(
            {6, CONTACTSECTION_CONTRACT}, static_cast<ContactItemType>(chain));

        if (false == validChain) { FAIL_1("invalid chain") }
    }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.txid().size()) {
        FAIL_1("invalid txid")
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.txid().size()) {
        FAIL_1("invalid txid")
    }

    if (input.has_serialized()) {
        if (MIN_PLAUSIBLE_SCRIPT > input.serialized().size()) {
            FAIL_1("invalid serialized")
        }

        if (MAX_PLAUSIBLE_SCRIPT < input.serialized().size()) {
            FAIL_1("invalid serialized")
        }
    }

    for (const auto& txin : input.input()) {
        try {
            const bool validInput = Check(
                txin,
                BlockchainTransactionAllowedInput().at(txin.version()).first,
                BlockchainTransactionAllowedInput().at(txin.version()).second,
                silent);

            if (false == validInput) { FAIL_1("invalid input") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed input version not defined for version", txin.version())
        }
    }

    for (const auto& output : input.output()) {
        try {
            const bool validOutput = Check(
                output,
                BlockchainTransactionAllowedOutput().at(input.version()).first,
                BlockchainTransactionAllowedOutput().at(input.version()).second,
                silent);

            if (false == validOutput) { FAIL_1("invalid output") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed output version not defined for version",
                input.version())
        }
    }

    if (input.has_blockhash()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > input.blockhash().size()) {
            FAIL_1("invalid blockhash")
        }

        if (MIN_PLAUSIBLE_IDENTIFIER < input.blockhash().size()) {
            FAIL_1("invalid blockhash")
        }
    }

    for (const auto& conflict : input.conflicts()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > conflict.size()) {
            FAIL_1("invalid conflict")
        }

        if (MIN_PLAUSIBLE_IDENTIFIER < conflict.size()) {
            FAIL_1("invalid conflict")
        }
    }

    if (MAX_TRANSACTION_MEMO_SIZE < input.memo().size()) {
        FAIL_1("invalid memo")
    }

    if (std::numeric_limits<std::uint8_t>::max() < input.segwit_flag()) {
        FAIL_2("invalid segwit flag", input.segwit_flag())
    }

    OPTIONAL_IDENTIFIER(wtxid);

    return true;
}

auto CheckProto_2(const BlockchainTransaction& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const BlockchainTransaction& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const BlockchainTransaction& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const BlockchainTransaction& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const BlockchainTransaction& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const BlockchainTransaction& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const BlockchainTransaction& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const BlockchainTransaction& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const BlockchainTransaction& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
