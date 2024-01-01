// Copyright (c) 2018-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainBlockHeader.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainBlockHeader.pb.h>
#include <limits>

#include "opentxs/protobuf/syntax/BitcoinBlockHeaderFields.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/BlockchainBlockLocalData.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/EthereumBlockHeaderFields.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyBlockchain.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainBlockHeader& input, const Log& log) -> bool
{
    bool bitcoin{false};
    bool ethereum{false};

    switch (input.type()) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 17:
        case 18:
        case 19:
        case std::numeric_limits<int>::max(): {
            bitcoin = true;
        } break;
        case 5:
        case 6:
        case 20:
        case 21:
        case 22:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43: {
            ethereum = true;
        } break;
        default: {
            FAIL_2("Invalid type", std::to_string(input.type()));
        }
    }

    OPTIONAL_SUBOBJECT(
        local, BlockchainBlockHeaderAllowedBlockchainBlockLocalData());

    if (bitcoin) {
        CHECK_SUBOBJECT(
            bitcoin, BlockchainBlockHeaderAllowedBitcoinBlockHeaderFields());
        CHECK_EXCLUDED(ethereum);
    } else if (ethereum) {
        CHECK_SUBOBJECT(
            ethereum, BlockchainBlockHeaderAllowedEthereumBlockHeaderFields());
        CHECK_EXCLUDED(bitcoin);
    } else {
        FAIL_1("Unknown type");
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
