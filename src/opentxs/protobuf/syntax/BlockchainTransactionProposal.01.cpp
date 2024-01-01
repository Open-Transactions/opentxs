// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainTransactionProposal.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainTransactionProposal.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/BlockchainTransaction.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainTransactionProposedNotification.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainTransactionProposedOutput.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainTransactionProposedSweep.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainTransactionProposal& input, const Log& log)
    -> bool
{
    CHECK_SUBOBJECTS(
        output,
        BlockchainTransactionProposalAllowedBlockchainTransactionProposedOutput());
    OPTIONAL_SUBOBJECTS(
        notification,
        BlockchainTransactionProposalAllowedBlockchainTransactionProposedNotification());
    OPTIONAL_SUBOBJECT(
        finished, BlockchainTransactionProposalAllowedBlockchainTransaction());
    OPTIONAL_SUBOBJECT(sweep, BlockchainTransactionProposalAllowedSweep());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
