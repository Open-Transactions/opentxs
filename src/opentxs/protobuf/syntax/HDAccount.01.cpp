// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/HDAccount.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/HDAccount.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/BlockchainAddress.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainDeterministicAccountData.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainHDAccountData.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const HDAccount& input, const Log& log) -> bool
{
    OPTIONAL_SUBOBJECT(
        deterministic, HDAccountAllowedBlockchainDeterministicAccountData());
    CHECK_SUBOBJECTS(internaladdress, HDAccountAllowedBlockchainAddress());
    CHECK_SUBOBJECTS(externaladdress, HDAccountAllowedBlockchainAddress());
    OPTIONAL_SUBOBJECT(hd, HDAccountAllowedBlockchainHDAccountData());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
