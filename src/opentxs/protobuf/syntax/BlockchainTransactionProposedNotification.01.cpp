// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainTransactionProposedNotification.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainTransactionProposedNotification.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/HDPath.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/PaymentCode.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const BlockchainTransactionProposedNotification& input,
    const Log& log) -> bool
{
    CHECK_SUBOBJECT(
        sender, BlockchainTransactionProposedNotificationAllowedPaymentCode());
    CHECK_SUBOBJECT(
        path, BlockchainTransactionProposedNotificationAllowedHDPath());
    CHECK_SUBOBJECT(
        recipient,
        BlockchainTransactionProposedNotificationAllowedPaymentCode());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
