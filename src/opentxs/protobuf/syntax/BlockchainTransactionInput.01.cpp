// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainTransactionInput.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainTransactionInput.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/BlockchainInputWitness.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainPreviousOutput.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainTransactionOutput.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainWalletKey.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainTransactionInput& input, const Log& log) -> bool
{
    CHECK_SCRIPT(script);
    CHECK_SUBOBJECT(
        previous, BlockchainTransactionInputAllowedBlockchainPreviousOutput());
    OPTIONAL_SUBOBJECT(
        witness, BlockchainTransactionInputAllowedBlockchainInputWitness());
    OPTIONAL_SUBOBJECTS(
        key, BlockchainTransactionInputAllowedBlockchainWalletKey());
    OPTIONAL_SUBOBJECT(
        spends, BlockchainTransactionInputAllowedBlockchainTransactionOutput());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
