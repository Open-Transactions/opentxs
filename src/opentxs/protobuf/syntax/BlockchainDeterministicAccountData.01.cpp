// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainDeterministicAccountData.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainDeterministicAccountData.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/BlockchainAccountData.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/HDPath.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainDeterministicAccountData& input, const Log& log)
    -> bool
{
    CHECK_SUBOBJECT(
        common,
        BlockchainDeterministicAccountDataAllowedBlockchainAccountData());
    CHECK_SUBOBJECT(path, BlockchainDeterministicAccountDataAllowedHDPath());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
