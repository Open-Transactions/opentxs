// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainAccountData.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainAccountData.pb.h>
#include <opentxs/protobuf/ContactItemType.pb.h>
#include <opentxs/protobuf/ContactSectionName.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/BlockchainActivity.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainAccountData& input, const Log& log) -> bool
{
    CHECK_IDENTIFIER(id);

    const bool validChain =
        ValidContactItemType({6, CONTACTSECTION_CONTRACT}, input.chain());

    if ((false == validChain) && (input.chain() != CITEMTYPE_REGTEST)) {
        FAIL_1("invalid type");
    }

    OPTIONAL_SUBOBJECTS(
        unspent, BlockchainAccountDataAllowedBlockchainActivity());
    OPTIONAL_SUBOBJECTS(
        spent, BlockchainAccountDataAllowedBlockchainActivity());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
