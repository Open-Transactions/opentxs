// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ContactItemType

#include "opentxs/protobuf/syntax/BlockchainTransaction.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainTransaction.pb.h>
#include <opentxs/protobuf/ContactItemType.pb.h>
#include <opentxs/protobuf/ContactSectionName.pb.h>
#include <cstdint>
#include <limits>

#include "opentxs/protobuf/syntax/BlockchainTransactionInput.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainTransactionOutput.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainTransaction& input, const Log& log) -> bool
{
    for (const auto& chain : input.chain()) {
        const bool validChain = ValidContactItemType(
            {6, CONTACTSECTION_CONTRACT}, static_cast<ContactItemType>(chain));

        if (false == validChain) { FAIL_1("invalid chain"); }
    }

    CHECK_IDENTIFIER(txid);

    if (input.has_serialized()) {
        if (MIN_PLAUSIBLE_SCRIPT > input.serialized().size()) {
            FAIL_1("invalid serialized");
        }

        if (MAX_PLAUSIBLE_SCRIPT < input.serialized().size()) {
            FAIL_1("invalid serialized");
        }
    }

    OPTIONAL_SUBOBJECTS(input, BlockchainTransactionAllowedInput());
    OPTIONAL_SUBOBJECTS(output, BlockchainTransactionAllowedOutput());
    OPTIONAL_IDENTIFIER(blockhash);
    OPTIONAL_IDENTIFIERS(conflicts);

    if (MAX_TRANSACTION_MEMO_SIZE < input.memo().size()) {
        FAIL_1("invalid memo");
    }

    if (std::numeric_limits<std::uint8_t>::max() < input.segwit_flag()) {
        FAIL_2("invalid segwit flag", input.segwit_flag());
    }

    OPTIONAL_IDENTIFIER(wtxid);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
