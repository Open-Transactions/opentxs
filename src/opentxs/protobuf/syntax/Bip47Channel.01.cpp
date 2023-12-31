// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Bip47Channel.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Bip47Channel.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Bip47Direction.hpp"      // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainActivity.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainDeterministicAccountData.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/HDPath.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/PaymentCode.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"  // IWYU pragma: keep

namespace opentxs::protobuf::inline syntax
{
auto version_1(const Bip47Channel& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(
        deterministic, Bip47ChannelAllowedBlockchainDeterministicAccountData());
    CHECK_SUBOBJECT(local, Bip47ChannelAllowedPaymentCode());
    CHECK_SUBOBJECT(remote, Bip47ChannelAllowedPaymentCode());
    CHECK_SUBOBJECT(incoming, Bip47ChannelAllowedBip47Direction());
    CHECK_SUBOBJECT(outgoing, Bip47ChannelAllowedBip47Direction());
    OPTIONAL_IDENTIFIER(contact);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
