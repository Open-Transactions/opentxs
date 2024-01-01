// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainEthereumAccountData.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainEthereumAccountData.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/BlockchainImportedAccountData.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/HDPath.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainEthereumAccountData& input, const Log& log)
    -> bool
{
    CHECK_SUBOBJECT(
        imported,
        BlockchainEthereumAccountDataAllowedBlockchainImportedAccountData());
    OPTIONAL_IDENTIFIERS(incoming_txid);
    OPTIONAL_IDENTIFIERS(outgoing_txid);
    OPTIONAL_SUBOBJECT(path, BlockchainEthereumAccountDataAllowedHDPath());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
