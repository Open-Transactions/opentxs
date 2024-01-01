// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainAddress.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainAddress.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/AsymmetricKey.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainAddress& input, const Log& log) -> bool
{
    return version_1(input, log, true, false);
}

auto version_1(
    const BlockchainAddress& input,
    const Log& log,
    const bool pubkey,
    const bool hd) -> bool
{
    if (MAX_LABEL_SIZE < input.label().size()) { FAIL_1("invalid label"); }

    OPTIONAL_IDENTIFIER(contact);
    CHECK_SUBOBJECT_VA(
        key,
        BlockchainAddressAllowedAsymmetricKey(),
        hd ? CREDTYPE_HD : CREDTYPE_LEGACY,
        pubkey ? KEYMODE_PUBLIC : KEYMODE_PRIVATE,
        KEYROLE_SIGN);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
